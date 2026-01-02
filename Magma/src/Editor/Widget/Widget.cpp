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
			VOLCANICORE_LOG_INFO("Reallocating geometry buffer");
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
		VOLCANICORE_LOG_INFO("LoadTexture: %s", src.c_str());

		auto reg =
			AssetManager::GetRegistry()->As<EditorAssetRegistry>();
		auto assetID = reg->GetAssetID(src);
		if(!assetID) {
			VOLCANICORE_LOG_ERROR("Could not load texture '%s'!", src.c_str());
			return (TextureHandle)0;
		}

		reg->LoadAsset(assetID);
		auto asset = reg->GetAsset(assetID)->As<ImageAsset>();

		auto tex =
			RendererAPI::Get()->CreateTexture(
			{
				.Width = asset->Width,
				.Height = asset->Height,
			});
		tex->SetData(asset->Data.Get());

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
		VOLCANICORE_LOG_INFO("EnableScissorRegion: %d", enable);
		Scissor = enable;
	}
	void SetScissorRegion(Rectanglei region) override {
		VOLCANICORE_LOG_INFO("SetScissorRegion: %d, %d, %d, %d",
							 region.Left(), region.Top(),
							 region.Width(), region.Height());
		ScissorRegion = region;
	}

	void EnableClipMask(bool enable) override {
		VOLCANICORE_LOG_INFO("EnableClipMask: %d", enable);
		ClipMask = enable;
	}
	void RenderToClipMask(ClipMaskOperation op, CompiledGeometryHandle geom,
						  Vector2f translation) override
	{
		VOLCANICORE_LOG_INFO("RenderToClipMask: %i", (int)op);
	}

	void SetTransform(const Matrix4f* transform) override {
		VOLCANICORE_LOG_INFO("SetTransform: %p", transform);
		Transform = (transform ? *transform : Transform);
	}

	LayerHandle PushLayer() override {
		VOLCANICORE_LOG_INFO("PushLayer");
		return 0;
	}
	void CompositeLayers(LayerHandle src, LayerHandle dst, BlendMode blendMode,
						 Span<const CompiledFilterHandle> filters) override
	{
		VOLCANICORE_LOG_INFO("CompositeLayers");

	}
	void PopLayer() override {
		VOLCANICORE_LOG_INFO("PopLayer");
	}

	TextureHandle SaveLayerAsTexture() override {
		VOLCANICORE_LOG_INFO("SaveLayerAsTexture");
		return 0;
	}

	CompiledFilterHandle SaveLayerAsMaskImage() override {
		VOLCANICORE_LOG_INFO("SaveLayerAsMaskImage");
		return 0;
	}

	CompiledFilterHandle CompileFilter(const String& name,
									   const Dictionary& parameters) override
	{
		VOLCANICORE_LOG_INFO("CompileFilter: %s", name.c_str());
		for(auto& [key, value] : parameters) {
			VOLCANICORE_LOG_INFO("\nParam: %s, Value: %i",
				key.c_str(), (int)value.GetType());
		}
		return 0;
	}
	void ReleaseFilter(CompiledFilterHandle filter) override {
		VOLCANICORE_LOG_INFO("ReleaseFilter");

	}

	CompiledShaderHandle CompileShader(const String& name,
		const Dictionary& parameters) override
	{
		VOLCANICORE_LOG_INFO("CompileShader: %s", name.c_str());
		for(auto& [key, value] : parameters) {
			VOLCANICORE_LOG_INFO("\nParam: %s, Value: %i",
				key.c_str(), (int)value.GetType());
		}
		return 0;
	}
	void RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) override {
		VOLCANICORE_LOG_INFO("RenderShader");

	}
	void ReleaseShader(CompiledShaderHandle shader) override {
		VOLCANICORE_LOG_INFO("ReleaseShader");

	}
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

	Rml::LoadFontFace("Magma/assets/fonts/JetBrainsMono-Bold.ttf");

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
	s_Doc = s_Context->LoadDocument(path);
	s_Doc->Show();
	m_RootPath = path;
	VOLCANICORE_LOG_INFO("Successfully loaded UI");
}

void WidgetManager::Reload() {
	if(s_Doc)
		s_Doc->Close();

	Load(m_RootPath);
	s_Doc->ReloadStyleSheet();
	s_RenderInterface->OnReload();
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

void Widget::Update(TimeStep ts) {
	if(!Enabled)
		return;

	if(State) {
		if(OnEvent.Func)
			OnEvent.CallVoid(State);
		State = { };
	}

	for(auto& animations : Animations)
		animations->Update(this, ts);
	for(auto& widget : Children)
		widget->Update(ts);
}

void Widget::Render() {
	if(!Visible)
		return;

	Begin();
	for(auto& widget : Children)
		widget->Render();
	End();
}

Widget* Widget::Add(Ref<Widget> widget) {
	Children.Add(widget);
	widget->Parent = this;
	return this;
}

void Widget::Remove(const std::string& id) {
	auto [found, i] =
		Children.Find([&](auto& w) { return w->ID == id; });
	if(found)
		Children.Pop(i);

	VOLCANICORE_LOG_WARNING("Could not remove widget '%s'", id.c_str());
}

Ref<Widget> Widget::Find(const std::string& id) {
	for(auto child : Children) {
		if(child->ID == id)
			return child;

		auto w = child->Find(id);
		if(w)
			return w;
	}

	return nullptr;
}

void Root::Begin() {

}

void Root::End() {

}

void Window::Begin() {

}

void Window::End() {

}

void Container::Begin() {

}

void Container::End() {

}

void Dropdown::Begin() {

}

void Dropdown::End() {

}

void Button::Begin() {

}

void Button::End() {
}

void Image::Begin() {

}

void Image::End() {
}

void Text::Begin() {

}

void Text::End() {

}

void TextInput::Begin() {

}

void TextInput::End() {

}

void FileDialog::Begin() {

}

void FileDialog::End() {

}

void FileEditor::Begin() {

}

void FileEditor::End() {

}

}