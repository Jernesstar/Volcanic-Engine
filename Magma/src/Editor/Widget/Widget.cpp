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

#include "Graphics/Renderer.h"
#include "Utils/JSONSerializer.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;
using namespace Magma;
using namespace Magma::Graphics;

using namespace Lava::Script;

using namespace Rml;

namespace Magma::UI {

Rml::SystemInterface* s_SystemInterface = nullptr;
Rml::RenderInterface* s_RenderInterface = nullptr;

class WidgetRendererInterface : public Rml::RenderInterface {
public:
	WidgetRendererInterface() = default;
	~WidgetRendererInterface() = default;

	CompiledGeometryHandle CompileGeometry(Span<const Vertex> vertices, Span<const int> indices) override {

	}
	void RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) override {

	}
	void ReleaseGeometry(CompiledGeometryHandle geometry) override {

	}

	TextureHandle LoadTexture(Vector2i& texture_dimensions, const String& source) override {

	}
	TextureHandle GenerateTexture(Span<const byte> source, Vector2i source_dimensions) override {

	}
	void ReleaseTexture(TextureHandle texture) override {

	}

	void EnableScissorRegion(bool enable) override {

	}
	void SetScissorRegion(Rectanglei region) override {

	}

	void EnableClipMask(bool enable) override {

	}
	void RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation) override {

	}

	void SetTransform(const Matrix4f* transform) override {

	}

	LayerHandle PushLayer() override {

	}
	void CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters) override {

	}
	void PopLayer() override {

	}

	TextureHandle SaveLayerAsTexture() override {

	}

	CompiledFilterHandle SaveLayerAsMaskImage() override {

	}

	CompiledFilterHandle CompileFilter(const String& name, const Dictionary& parameters) override {

	}
	void ReleaseFilter(CompiledFilterHandle filter) override {

	}

	CompiledShaderHandle CompileShader(const String& name, const Dictionary& parameters) override {

	}
	void RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) override {

	}
	void ReleaseShader(CompiledShaderHandle shader) override {

	}
};

void WidgetManager::Init() {
	s_SystemInterface = new SystemInterface_GLFW();
	Rml::SetSystemInterface(s_SystemInterface);
	s_RenderInterface = new WidgetRendererInterface();
	Rml::SetRenderInterface(s_RenderInterface);
	Rml::Initialise();

	auto window = Application::As<WindowApplication>()->GetWindow();
	f32 width = window->GetWidth();
	f32 height = window->GetHeight();

	Rml::Context* context =
		Rml::CreateContext("main", Rml::Vector2i(width, height));

	if(!context) {
		VOLCANICORE_LOG_ERROR("Could not create RmlUI context!");
		return;
	}
}

void WidgetManager::Close() {
	m_Root.reset();

	Rml::Shutdown();

	delete s_SystemInterface;
	s_SystemInterface = nullptr;
	delete s_RenderInterface;
	s_RenderInterface = nullptr;
}

static u32 ImageIndex = 0;
static u32 TextIndex = 0;
static u32 ButtonIndex = 0;
static u32 WindowIndex = 0;

static void ParseElement(Ref<Widget> parent, pugi::xml_node node) {
	if(!node)
		return;

	std::string id;
	std::string type = node.name();
	if(type == "Script")
		return;

	if(node.attribute("id"))
		id = node.attribute("id").as_string();
	else {
		u32* index = nullptr;
		if(type == "Image")
			index = &ImageIndex;
		else if(type == "Text")
			index = &TextIndex;
		else if(type == "Button")
			index = &ButtonIndex;
		else if(type == "Window")
			index = &WindowIndex;

		id = std::string(node.name()) + "-" + std::to_string((*index)++);
	}

	Ref<Widget> widget;
	if(type == "Image") {
		widget = CreateRef<Image>(id);
		auto w = widget->As<Image>();

		if(node.attribute("asset")) {
			auto path = node.attribute("asset").as_string();
			auto reg = AssetManager::GetRegistry()->As<EditorAssetRegistry>();
			w->Texture = reg->GetAssetID(path);
		}
	}
	else if(type == "Text") {
		widget = CreateRef<Text>(id);
		auto w = widget->As<Text>();

		if(node.attribute("label")) {
			w->Label = node.attribute("label").as_string();
		}
		if(node.attribute("scale")) {
			w->Scale = node.attribute("scale").as_float();
		}
		if(node.attribute("font")) {
			auto path = node.attribute("font").as_string();
			auto reg = AssetManager::GetRegistry()->As<EditorAssetRegistry>();
			w->Font = reg->GetAssetID(path);
		}
	}
	else if(type == "Button") {
		widget = CreateRef<Button>(id);
		auto w = widget->As<Button>();

		if(node.attribute("color")) {
			// w->Color = node.attribute("color").as_string();
		}
	}
	else if(type == "Window") {
		widget = CreateRef<Window>(id);
		auto w = widget->As<Window>();

	}
	else if(type == "Container") {
		widget = CreateRef<Container>(id);
		auto w = widget->As<Container>();

		if(node.attribute("width_type")) {
			std::string type = node.attribute("width_type").as_string();
			if(type == "fixed")
				w->SizeX = Container::SizeType::Fixed;
			else
				w->SizeX = Container::SizeType::Stretch;
		}
		if(node.attribute("height_type")) {
			std::string type = node.attribute("height_type").as_string();
			if(type == "fixed")
				w->SizeY = Container::SizeType::Fixed;
			else
				w->SizeY = Container::SizeType::Stretch;
		}
		if(node.attribute("padding")) {
			w->Padding = node.attribute("padding").as_uint();
		}
		if(node.attribute("childGap")) {
			w->ChildGap = node.attribute("childGap").as_uint();
		}
	}
	else
		return;

	if(node.child("Script")) {
		auto scriptNode = node.child("Script");
		std::string script = scriptNode.child_value();

		// Create ScriptModule from name
		// Load script data
		// Compile script
		// Create ScriptObject
	}

	if(node.attribute("width"))
		widget->Width = node.attribute("width").as_float();
	if(node.attribute("height"))
		widget->Height = node.attribute("height").as_float();
	if(node.attribute("visible"))
		widget->Visible = node.attribute("visible").as_bool();
	if(node.attribute("enabled"))
		widget->Enabled = node.attribute("enabled").as_bool();

	parent->Add(widget);

	for(pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
		ParseElement(widget, child);
}

void WidgetManager::Load(const std::string& path) {
	if(m_Root)
		m_Root.reset();

	TextIndex = 0;
	ImageIndex = 0;
	ButtonIndex = 0;
	WindowIndex = 0;

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());
	if(!res) {
		std::cout << "XML [" << path << "] parsed with errors, attr value: ["
				  << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << res.description() << "\n";
		std::cout << "Error offset: " << res.offset << "\n";
	}

	pugi::xml_node node = doc.child("Root");
	auto root = CreateRef<Root>(node.attribute("id").as_string());

	for(pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
		ParseElement(root, child);

	m_Root = root;
	m_RootPath = path;
	VOLCANICORE_LOG_INFO("Successfully loaded UI");
}

void WidgetManager::Reload() {
	Load(m_RootPath);
}

void WidgetManager::Update(TimeStep ts) {

	// GetRoot()->Update(ts);
}

typedef enum
{
	CUSTOM_ELEMENT_TYPE_GIF,
	CUSTOM_ELEMENT_TYPE_VIDEO
} CustomElementType;

typedef struct
{

} CustomElement_GIF;

typedef struct
{

} CustomElement_VIDEO;

typedef struct
{
	CustomElementType type;
	union {
		CustomElement_GIF gif;
		CustomElement_VIDEO video;
	} customData;
} CustomElement;

void WidgetManager::Render() {
	if(!m_Root)
		return;

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