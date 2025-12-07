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
};

class WidgetRendererInterface : public Rml::RenderInterface {
public:
	DrawBuffer* GeometryBuffer;
	VolcaniCore::List<CompiledBuffer> Buffers;

	Ref<Graphics::Shader> DefaultShader;
	bool Scissor = false;
	Rml::Rectanglei ScissorRegion;
	bool ClipMask = false;
	Matrix4f Transform = Matrix4f::Identity();

	VolcaniCore::List<Ref<Graphics::Texture>> TextureHandles;

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

			layout(location = 0) out vec2 v_FragTexCoord;
			layout(location = 1) out vec4 v_FragColor;

			void main() {
				v_FragTexCoord = a_TexCoord;
				v_FragColor = a_Color;

				vec2 pos = a_Position + u_Translate;
				vec4 outPos = u_Projection * u_Transform * vec4(pos, 0.0, 1.0);

				gl_Position = outPos;
			}
		)";

		std::string fragmentShaderStr = R"(
			#version 460 core

			uniform sampler2D u_Texture;
			uniform int u_UseTexture;

			layout(location = 0) in vec2 a_FragTexCoord;
			layout(location = 1) in vec4 a_FragColor;

			layout(location = 0) out vec4 FragColor;

			void main() {
				if(u_UseTexture == 0) {
					FragColor = a_FragColor;
					return;
				}

				vec4 texColor = texture(u_Texture, a_FragTexCoord);
				FragColor = a_FragColor * texColor;
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
				.IndexCount = 1'000'000,
				.DynamicIndices = false,
				.VertexCount = 10'000'000,
				.DynamicVertices = false,
				.VertexLayout =
				{
					{
						{ "a_Position", BufferDataType::Vec2 },
						{ "a_Color",	BufferDataType::Vec4 },
						{ "a_TexCoord", BufferDataType::Vec2 },
					},
					true, // Dynamic
					false // Instanced
				},
			});
	}

	~WidgetRendererInterface() {
		TextureHandles.Clear();

		delete GeometryBuffer;
	}

	void BeginFrame() {
		VOLCANICORE_LOG_INFO("WidgetRendererInterface::BeginFrame");

		Scissor = false;
		ClipMask = false;
	}

	void EndFrame() {
		VOLCANICORE_LOG_INFO("WidgetRendererInterface::EndFrame");

	}

	CompiledGeometryHandle CompileGeometry(Span<const Rml::Vertex> vertices,
										   Span<const int> indices) override
	{
		VOLCANICORE_LOG_INFO("CompileGeometry");

		auto& buffer = Buffers.Emplace();
		buffer.VertexOffset = GeometryBuffer->GetVertexCount();
		buffer.IndexOffset = GeometryBuffer->GetIndexCount();
		buffer.VertexCount = vertices.size();
		buffer.IndexCount = indices.size();

		GeometryBuffer->Add(DrawBufferIndex::Vertex,
			vertices.data(), vertices.size());
		GeometryBuffer->Add(DrawBufferIndex::Index,
			indices.data(), indices.size());

		return (CompiledGeometryHandle)Buffers.Count();
	}
	void RenderGeometry(CompiledGeometryHandle geomHandle, Vector2f translation,
						TextureHandle textureHandle) override
	{
		VOLCANICORE_LOG_INFO("RenderGeometry");

		auto bufferIdx = (u64)geomHandle;
		auto& buffer = Buffers[bufferIdx - 1];

		auto pass = RendererAPI::Get()->NewPass(GeometryBuffer);
		pass->Pipeline = DefaultShader;
		auto cmd = RendererAPI::Get()->NewCommand(pass);

		cmd->VerticesIndex = buffer.VertexOffset;
		cmd->IndicesIndex = buffer.IndexOffset;

		if(textureHandle) {
			auto texIdx = (u64)textureHandle;
			auto tex = TextureHandles[texIdx - 1];
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
			Mat4{
				Transform[0][0], Transform[1][0], Transform[2][0], Transform[3][0],
				Transform[0][1], Transform[1][1], Transform[2][1], Transform[3][1],
				Transform[0][2], Transform[1][2], Transform[2][2], Transform[3][2],
				Transform[0][3], Transform[1][3], Transform[2][3], Transform[3][3],
			}
		);
		cmd->Uniforms.Set("u_Translate", Vec2{ translation.x, translation.y });

		auto window = Application::As<WindowApplication>()->GetWindow();
		auto width = (f32)window->GetWidth();
		auto height = (f32)window->GetHeight();
		auto projMat =
			Rml::Matrix4f::ProjectOrtho(0, width, height, 0, -10000, 10000);
		cmd->Uniforms.Set("u_Projection",
			Mat4{
				projMat[0][0], projMat[1][0], projMat[2][0], projMat[3][0],
				projMat[0][1], projMat[1][1], projMat[2][1], projMat[3][1],
				projMat[0][2], projMat[1][2], projMat[2][2], projMat[3][2],
				projMat[0][3], projMat[1][3], projMat[2][3], projMat[3][3],
			}
		);

		auto* call = cmd->NewCall();
		call->Primitive = DrawPrimitive::Triangle;
		call->Partition = DrawPartition::Single;
		call->IndexCount = buffer.IndexCount;
	}

	void ReleaseGeometry(CompiledGeometryHandle geomHandle) override {
		VOLCANICORE_LOG_INFO("ReleaseGeometry");

		auto bufferIdx = (u64)geomHandle;
		Buffers.Pop(bufferIdx - 1);
	}

	TextureHandle LoadTexture(Vector2i& texDim, const String& src) override {
		VOLCANICORE_LOG_INFO("LoadTexture: %s", src.c_str());

		auto reg =
			AssetManager::GetRegistry()->As<EditorAssetRegistry>();
		auto id = reg->GetAssetID(src);
		if(!id) {
			VOLCANICORE_LOG_ERROR("Could not load texture '%s'!", src.c_str());
			return (TextureHandle)0;
		}

		reg->LoadAsset(id);
		auto asset = reg->GetAsset(id)->As<ImageAsset>();

		auto tex =
			RendererAPI::Get()->CreateTexture(
			{
				.Width = asset->Width,
				.Height = asset->Height,
			});
		tex->SetData(asset->Data.Get());
		TextureHandles.Add(tex);

		return (TextureHandle)TextureHandles.Count();
	}
	TextureHandle GenerateTexture(Span<const byte> src,
								  Vector2i srcDim) override
	{
		VOLCANICORE_LOG_INFO("GenerateTexture");

		auto tex =
			RendererAPI::Get()->CreateTexture(
			{
				.Width = (u32)srcDim.x,
				.Height = (u32)srcDim.y,
			});
		tex->SetData(src.data());
		TextureHandles.Add(tex);

		return (TextureHandle)TextureHandles.Count();
	}
	void ReleaseTexture(TextureHandle texture) override {
		VOLCANICORE_LOG_INFO("ReleaseTexture");

		u64 id = (u64)texture;
		if(id == 0 || id > TextureHandles.Count())
			return;

		TextureHandles.Pop(id - 1);
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

struct TestDataModel {
	bool ShowText = true;
	Rml::String Animal = "Dog";
};

static TestDataModel TestData;

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

	TestData = { };
	if(Rml::DataModelConstructor model =
		s_Context->CreateDataModel("animals"))
	{
		model.Bind("show_text", &TestData.ShowText);
		model.Bind("animal", &TestData.Animal);
	}

	s_Doc =
		s_Context->LoadDocument("Magma/assets/UI/test.rml");
	s_Doc->Show();

	Rml::Element* element = s_Doc->GetElementById("world");
	// element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
	element->SetInnerRML(reinterpret_cast<const char*>("Hello, World!"));
	element->SetProperty("font-size", "1.5em");

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
}

void WidgetManager::Close() {
	m_Root.reset();

	Rml::Shutdown();

	delete s_SystemInterface;
	s_SystemInterface = nullptr;
	delete s_RenderInterface;
	s_RenderInterface = nullptr;
}

void WidgetManager::Load(const std::string& path) {
	if(m_Root)
		m_Root.reset();

	m_RootPath = path;
	VOLCANICORE_LOG_INFO("Successfully loaded UI");
}

void WidgetManager::Reload() {
	Load(m_RootPath);
}

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