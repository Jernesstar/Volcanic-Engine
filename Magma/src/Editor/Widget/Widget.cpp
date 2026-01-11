#include "Widget.h"

#include <iostream>
#include <fstream>

#include <pugixml.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Backends/RmlUi_Platform_GLFW.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Input.h>
#include <VolcanicWindow/Events.h>

#include <Lava/Script/ScriptModule.h>
#include <Lava/Script/ScriptClass.h>
#include <Lava/Script/ScriptObject.h>

#include "Graphics/Platform/RendererAPI.h"
#include "Graphics/Renderer.h"
#include "Utils/JSONSerializer.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;
using namespace Magma;
using namespace Magma::Graphics;

using namespace Lava::Script;

using namespace Rml;

namespace Magma::UI {

struct CompiledBuffer {
	u32 VertexOffset = 0;
	u32 IndexOffset = 0;
	u32 VertexCount = 0;
	u32 IndexCount = 0;
	const Rml::Vertex* VertexData = nullptr;
	const i32* IndexData = nullptr;
};

class WidgetRendererInterface : public Rml::RenderInterface {
public:
	DrawBuffer* GeometryBuffer = nullptr;
	Map<UUID, CompiledBuffer> Buffers;

	Ref<Graphics::Shader> DefaultShader;
	Map<UUID, Ref<Graphics::Texture>> TextureHandles;

	Ref<Graphics::Framebuffer> FinalFramebuffer;

	bool Scissor = false;
	Rml::Rectanglei ScissorRegion;
	bool ClipMask = false;
	Matrix4f Transform = Matrix4f::Identity();

public:
	WidgetRendererInterface() {
		std::string vertexShaderStr = R"(
			#version 460 core

			uniform vec2 u_Translate;
			uniform mat4 u_Transform;
			uniform mat4 u_Projection;

			layout(location = 0) in vec2 a_Position;
			layout(location = 1) in vec4 a_Color;
			layout(location = 2) in vec2 a_TexCoord;

			layout(location = 0) out vec2 v_TexCoord;
			layout(location = 1) out vec4 v_Color;

			void main() {
				v_TexCoord = a_TexCoord;
				v_Color = a_Color;

				vec2 pos = a_Position + u_Translate;
				gl_Position = u_Projection * vec4(pos, 0.0, 1.0);
			}
		)";

		std::string fragmentShaderStr = R"(
			#version 460 core

			uniform sampler2D u_Texture;
			uniform int u_UseTexture;

			layout(location = 0) in vec2 v_TexCoord;
			layout(location = 1) in vec4 v_Color;

			layout(location = 0) out vec4 FragColor;

			void main() {
				if(u_UseTexture == 0)
					FragColor = v_Color;
				else
					FragColor = v_Color * texture(u_Texture, v_TexCoord);
			}
		)";

		DefaultShader = RendererAPI::Get()->CreateShader({ });
		VolcaniCore::List<ShaderFile> files;
		files.Emplace(ShaderFileType::Vertex, vertexShaderStr);
		files.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
		DefaultShader->SetShaderData(std::move(files));

		GeometryBuffer =
			RendererAPI::Get()->NewBuffer(
			{
				.IndexCount = 3'000'000,
				.DynamicIndices = false,
				.VertexCount = 1'000'000,
				.DynamicVertices = false,
				.VertexLayout =
				{
					{
						{ "a_Position", BufferDataType::Vec2 },
						{ "a_Color",	BufferDataType::UVec4 },
						{ "a_TexCoord", BufferDataType::Vec2 },
					},
					false // Instanced
				}
			});
	}

	~WidgetRendererInterface() {
		TextureHandles.clear();
		Buffers.clear();
		delete GeometryBuffer;
		DefaultShader.reset();
	}

	void BeginFrame() {
		Scissor = false;
		ClipMask = false;
	}

	void EndFrame() {

	}

	void OnReload() {
		GeometryBuffer->Clear();
	}

	CompiledGeometryHandle CompileGeometry(Span<const Rml::Vertex> vertices,
										   Span<const int> indices) override
	{
		if(GeometryBuffer->GetIndexCount() + (u32)indices.size()
		>= GeometryBuffer->Spec.IndexCount
		|| GeometryBuffer->GetVertexCount() + (u32)vertices.size()
		>= GeometryBuffer->Spec.VertexCount)
		{
			VolcaniCore::Log::Info("Reallocating geometry buffer");
			GeometryBuffer->Clear();
			for(auto& [id, buffer] : Buffers) {
				buffer.VertexOffset = GeometryBuffer->GetVertexCount();
				buffer.IndexOffset = GeometryBuffer->GetIndexCount();
				GeometryBuffer->Add(DrawBufferIndex::Index,
					buffer.IndexData, buffer.IndexCount);
				GeometryBuffer->Add(DrawBufferIndex::Vertex,
									buffer.VertexData, buffer.VertexCount);

			}
		}

		UUID id = UUID();
		auto& buffer = Buffers[id];
		buffer.IndexCount = (u32)indices.size();
		buffer.VertexCount = (u32)vertices.size();
		buffer.VertexData = vertices.data();
		buffer.IndexData = indices.data();

		buffer.VertexOffset = GeometryBuffer->GetVertexCount();
		buffer.IndexOffset = GeometryBuffer->GetIndexCount();
		GeometryBuffer->Add(DrawBufferIndex::Index,
							buffer.IndexData, buffer.IndexCount);
		GeometryBuffer->Add(DrawBufferIndex::Vertex,
							buffer.VertexData, buffer.VertexCount);

		return (CompiledGeometryHandle)id;
	}
	void RenderGeometry(CompiledGeometryHandle geomHandle, Vector2f tr,
						TextureHandle textureHandle) override
	{
		if(!geomHandle)
			return;

		UUID id = (UUID)geomHandle;
		auto& buffer = Buffers.at(id);

		auto pass = RendererAPI::Get()->NewPass(GeometryBuffer);
		pass->Pipeline = DefaultShader;

		auto* cmd = RendererAPI::Get()->NewCommand(pass);
		cmd->IndicesIndex = buffer.IndexOffset;
		cmd->VerticesIndex = buffer.VertexOffset;

		if(textureHandle) {
			UUID texID = (UUID)textureHandle;
			auto tex = TextureHandles.at(texID);
			cmd->Uniforms.Set("u_Texture", TextureSlot{ tex, 0 });
			cmd->Uniforms.Set("u_UseTexture", 1);
		}
		else
			cmd->Uniforms.Set("u_UseTexture", 0);

		if(Scissor) {
			cmd->Scissor = Scissor;
			cmd->ScissorX = (f32)ScissorRegion.Left();
			cmd->ScissorY = (f32)ScissorRegion.Top();
			cmd->ScissorW = (f32)ScissorRegion.Width();
			cmd->ScissorH = (f32)ScissorRegion.Height();
		}
 
		cmd->Uniforms.Set("u_Transform",
			Mat4
			{
				Transform[0][0], Transform[1][0], Transform[2][0], Transform[3][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[3][1],
				Transform[0][2], Transform[1][2], Transform[2][2], Transform[3][2],
				Transform[0][3], Transform[1][3], Transform[2][3], Transform[3][3],
			}
		);
		cmd->Uniforms.Set("u_Translate", Vec2{ tr.x, tr.y });

		auto window = Application::As<WindowApplication>()->GetWindow();
		auto width = (f32)window->GetWidth();
		auto height = (f32)window->GetHeight();
		auto projMat = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
		cmd->Uniforms.Set("u_Projection", projMat);

		cmd->Viewport = true;
		cmd->ViewportW = width;
		cmd->ViewportH = height;
		cmd->DepthTesting = DepthTestingMode::Off;
		cmd->Blending = BlendingMode::Greatest;
		cmd->Culling = CullingMode::Off;

		auto* call = cmd->NewCall();
		call->Primitive = DrawPrimitive::Triangle;
		call->Partition = DrawPartition::Single;
		call->IndexCount = buffer.IndexCount;
	}

	void ReleaseGeometry(CompiledGeometryHandle geomHandle) override {
		auto id = (UUID)geomHandle;
		Buffers.erase(id);
	}

	TextureHandle LoadTexture(Vector2i& texDim, const String& src) override {
		auto image = AssetImporter::LoadImage(src);

		auto tex =
			RendererAPI::Get()->CreateTexture(
			{
				.Width = image.Width,
				.Height = image.Height,
			});
		tex->SetData(image.Data.Get());

		UUID id = UUID();
		TextureHandles[id] = tex;

		return (TextureHandle)id;
	}
	TextureHandle GenerateTexture(Span<const byte> src,
								  Vector2i srcDim) override
	{
		auto tex =
			RendererAPI::Get()->CreateTexture(
			{
				.Width = (u32)srcDim.x,
				.Height = (u32)srcDim.y,
			});
		tex->SetData(src.data());

		UUID id = UUID();
		TextureHandles[id] = tex;

		return (TextureHandle)id;
	}
	void ReleaseTexture(TextureHandle texture) override {
		auto id = (UUID)texture;
		TextureHandles.erase(id);
	}

	void EnableScissorRegion(bool enable) override {
		Scissor = enable;
	}
	void SetScissorRegion(Rectanglei region) override {
		ScissorRegion = region;
	}

	void EnableClipMask(bool enable) override {
		ClipMask = enable;
	}
	void RenderToClipMask(ClipMaskOperation op, CompiledGeometryHandle geom,
						  Vector2f translation) override
	{
	}

	void SetTransform(const Matrix4f* transform) override {
		Transform = (transform ? *transform : Transform);
	}

	LayerHandle PushLayer() override {
		return 0;
	}
	void CompositeLayers(LayerHandle src, LayerHandle dst, BlendMode blendMode,
						 Span<const CompiledFilterHandle> filters) override
	{

	}
	void PopLayer() override {

	}

	TextureHandle SaveLayerAsTexture() override {
		return 0;
	}

	CompiledFilterHandle SaveLayerAsMaskImage() override {
		return 0;
	}

	CompiledFilterHandle CompileFilter(const String& name,
									   const Dictionary& parameters) override
	{
		return 0;
	}
	void ReleaseFilter(CompiledFilterHandle filter) override {

	}

	CompiledShaderHandle CompileShader(const String& name,
		const Dictionary& parameters) override
	{
		return 0;
	}
	void RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geom,
					  Vector2f tr, TextureHandle tex) override
	{

	}
	void ReleaseShader(CompiledShaderHandle shader) override {

	}
};

class UIElementWrapper {
public:
	Rml::Element* Element = nullptr;

};

Rml::SystemInterface* s_SystemInterface = nullptr;
WidgetRendererInterface* s_RenderInterface = nullptr;
Rml::Context* s_Context = nullptr;

static ElementDocument* s_Doc = nullptr;

void WidgetManager::Init() {
	s_SystemInterface = new SystemInterface_GLFW();
	Rml::SetSystemInterface(s_SystemInterface);
	s_RenderInterface = new WidgetRendererInterface();
	Rml::SetRenderInterface(s_RenderInterface);
	Rml::Initialise();

	auto window = Application::As<WindowApplication>()->GetWindow();
	f32 width = window->GetWidth();
	f32 height = window->GetHeight();

	s_Context =
		Rml::CreateContext("main", Rml::Vector2i(width, height));

	VOLCANICORE_ASSERT(s_Context, "Could not create RmlUI context!");

	Application::PushDir();
	Rml::LoadFontFace("Magma/assets/fonts/JetBrainsMono-Bold.ttf");
	Application::PopDir();

	Events::RegisterListener<WindowResizedEvent>(
		[](WindowResizedEvent& e)
		{
			s_Context->SetDimensions({ e.Width, e.Height });
		});
	Events::RegisterListener<MouseMovedEvent>(
		[](MouseMovedEvent& e)
		{
			s_Context->ProcessMouseMove(e.x, e.y, 0);
		});
	Events::RegisterListener<MouseScrolledEvent>(
		[](MouseScrolledEvent& e)
		{
			s_Context->ProcessMouseWheel((int)e.ScrollY, 0);
		});
	Events::RegisterListener<MouseButtonPressedEvent>(
		[](MouseButtonPressedEvent& e)
		{
			s_Context->ProcessMouseButtonDown(e.Button, 0);
		});
	Events::RegisterListener<MouseButtonReleasedEvent>(
		[](MouseButtonReleasedEvent& e)
		{
			s_Context->ProcessMouseButtonUp(e.Button, 0);
		});
	Events::RegisterListener<KeyPressedEvent>(
		[](KeyPressedEvent& e)
		{
			s_Context->ProcessKeyDown(RmlGLFW::ConvertKey((int)e.Key), 0);
		});
	Events::RegisterListener<KeyReleasedEvent>(
		[](KeyReleasedEvent& e)
		{
			s_Context->ProcessKeyUp(RmlGLFW::ConvertKey((int)e.Key), 0);
		});
	Events::RegisterListener<KeyCharEvent>(
		[](KeyCharEvent& e)
		{
			s_Context->ProcessTextInput(e.Char);
		});
}

void WidgetManager::Close() {
	Rml::Shutdown();

	delete s_SystemInterface;
	s_SystemInterface = nullptr;
	delete s_RenderInterface;
	s_RenderInterface = nullptr;
}

void WidgetManager::Load(const std::string& path) {
	s_Context->UnloadAllDocuments();
	Application::PushDir();
	s_Doc = s_Context->LoadDocument(path);
	Application::PopDir();
	s_Doc->Show();
	s_RenderInterface->OnReload();
	m_RootPath = path;
}

void WidgetManager::Reload() {
	if(s_Doc)
		s_Doc->Close();

	Load(m_RootPath);
	s_Doc->ReloadStyleSheet();
}

Rml::ElementDocument* WidgetManager::GetDocument() { return s_Doc; }

void WidgetManager::Update(TimeStep ts) {
	s_Context->Update();
}

void WidgetManager::Render() {
	s_RenderInterface->BeginFrame();
	s_Context->Render();
	s_RenderInterface->EndFrame();
}

}