#include "Widget.h"

#include <iostream>
#include <fstream>

#define CLAY_IMPLEMENTATION
#include <clay/clay.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcanicWindow/Application.h>
#include <VolcanicWindow/Input.h>
#include <VolcanicWindow/Events.h>

#include <Lava/Script/ScriptModule.h>
#include <Lava/Script/ScriptClass.h>
#include <Lava/Script/ScriptObject.h>

#include "Utils/JSONSerializer.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;
using namespace Magma;
using namespace Lava::Script;

namespace Magma::UI {

template<typename T>
static T Get(const rapidjson::Value& val, const std::string& key, const T& df) {
	if(val.IsNull())
		return df;
	if(val.HasMember(key))
		return val[key].Get<T>();

	return df;
}

static bool Traverse(Ref<Widget> parent, const rapidjson::Value& value) {
	if(value.IsNull()) {
		VOLCANICORE_LOG_ERROR("Value was null");
		return false;
	}
	if(!value.HasMember("Name")) {
		VOLCANICORE_LOG_ERROR("Missing property 'Name'");
		return false;
	}
	if(!value.HasMember("Type")) {
		VOLCANICORE_LOG_ERROR("Missing property 'Type'");
		return false;
	}

	std::string id = value["Name"].GetString();
	std::string type = value["Type"].GetString();

	Ref<Widget> widget = nullptr;

	if(type == "Window") {
		widget = CreateRef<Window>(id);
		auto w = widget->As<Window>();

		if(value.HasMember("IsRoot") && value["IsRoot"].IsBool())
			w->IsRoot = value["IsRoot"].GetBool();

		if(value.HasMember("IsChild") && value["IsChild"].IsBool())
			w->IsChild = value["IsChild"].GetBool();

		if(value.HasMember("AllowScroll") && value["AllowScroll"].IsBool())
			w->AllowScroll = value["AllowScroll"].GetBool();

		if(value.HasMember("Width") && value["Width"].IsString())
			if(value["Width"].GetString() == "auto")
				w->Width = parent->Width;

		if(value.HasMember("Height") && value["Height"].IsString())
			if(value["Height"].GetString() == "auto")
				w->Height = parent->Height;

		if(value.HasMember("x") && value["x"].IsString())
			if(value["x"].GetString() == "auto")
				w->x = parent->x;

		if(value.HasMember("y") && value["y"].IsString())
			if(value["y"].GetString() == "auto")
				w->y = parent->y;

		if(value.HasMember("Color") && value["Color"].IsArray()) {
			auto color = value["Color"].GetArray();
			w->Color = {
				color[0].GetFloat(),
				color[1].GetFloat(),
				color[2].GetFloat(),
				color[3].GetFloat()
			};
		}
	}
	else if(type == "Container") {
		widget = CreateRef<Container>(id);
		auto w = widget->As<Container>();

	}
	else if(type == "Dropdown") {
		widget = CreateRef<Dropdown>(id);
		auto w = widget->As<Dropdown>();

	}
	else if(type == "Button") {
		widget = CreateRef<Button>(id);
		auto w = widget->As<Button>();

		if(value.HasMember("Color") && value["Color"].IsArray()) {
			auto color = value["Color"].GetArray();
			w->Color = Vec4{
				color[0].GetFloat(),
				color[1].GetFloat(),
				color[2].GetFloat(),
				color[3].GetFloat()
			};
		}
	}
	else if(type == "Image") {
		widget = CreateRef<Image>(id);
		auto w = widget->As<Image>();

		if(value.HasMember("Asset") && value["Asset"].IsString()) {
			auto path = value["Asset"].GetString();
			Application::PushDir();
			auto image = AssetImporter::GetTexture(path);
			Application::PopDir();
			w->Asset = image;
		}
	}
	else if(type == "Text") {
		widget = CreateRef<Text>(id);
		auto w = widget->As<Text>();

		if(value.HasMember("Label") && value["Label"].IsString())
			w->Label = value["Label"].GetString();
	}
	else if(type == "TextInput") {
		widget = CreateRef<TextInput>(id);
		auto w = widget->As<TextInput>();

	}
	else if(type == "FileDialog") {
		widget = CreateRef<FileDialog>(id);
		auto w = widget->As<FileDialog>();

	}
	else if(type == "FileEditor") {
		widget = CreateRef<FileEditor>(id);
		auto w = widget->As<FileEditor>();

	}
	else if(type == "Gizmo") {
		widget = CreateRef<Gizmo>(id);
		auto w = widget->As<Gizmo>();

	}
	else {
		VOLCANICORE_LOG_WARNING("Unknown widget type '%s'", type.c_str());
		return false;
	}

	if(value.HasMember("Width") && value["Width"].IsNumber())
		widget->Width = value["Width"].GetFloat();
	if(value.HasMember("Height") && value["Height"].IsNumber())
		widget->Height = value["Height"].GetFloat();
	if(value.HasMember("x") && value["x"].IsNumber())
		widget->x = value["x"].GetFloat();
	if(value.HasMember("y") && value["y"].IsNumber())
		widget->y = value["y"].GetFloat();
	if(value.HasMember("Visible") && value["Visible"].IsBool())
		widget->Visible = value["Visible"].GetBool();
	if(value.HasMember("Enabled") && value["Enabled"].IsBool())
		widget->Enabled = value["Enabled"].GetBool();

	parent->Add(widget);

	if(!value.HasMember("Children"))
		return true;

	for(const auto& child : value["Children"].GetArray())
		if(!Traverse(widget, child))
			return false;

	return true;
}

static Ref<Widget> LoadPage(const std::string& path) {
	using namespace rapidjson;

	if(path == "") {
		VOLCANICORE_LOG_ERROR("Path was empty");
		return nullptr;
	}

	auto name = fs::path(path).stem().stem().stem().string();
	if(!FileUtils::PathExists(path)) {
		VOLCANICORE_LOG_ERROR(
			"Could not find file with name '%s'", name.c_str());
		return nullptr;
	}

	Document doc;
	doc.Parse(FileUtils::ReadFile(path));

	if(doc.HasParseError()) {
		VOLCANICORE_LOG_INFO("Parsing error %i", (uint32_t)doc.GetParseError());
		return nullptr;
	}
	if(!doc.IsObject()) {
		VOLCANICORE_LOG_ERROR("File did not have root object");
		return nullptr;
	}
	if(!doc.HasMember("Root")) {
		VOLCANICORE_LOG_ERROR("File did not have \"Root\" object");
		return nullptr;
	}
	if(!doc["Root"].IsArray()) {
		VOLCANICORE_LOG_ERROR("Root object was not an array");
		return nullptr;
	}

	auto root = CreateRef<Root>("Root");

	for(const auto& value : doc["Root"].GetArray()) {
		bool success = Traverse(root, value);
		if(!success) {
			VOLCANICORE_LOG_ERROR("Failed to load page");
			return nullptr;
		}
	}

	return root;
}

static Clay_Context* s_ClayContext;

static void HandleClayErrors(Clay_ErrorData errorData) {
	VOLCANICORE_LOG_ERROR("%s", errorData.errorText.chars);
	switch(errorData.errorType) {
		// etc
	}
}

// Example measure text function
static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, uintptr_t userData) {
	// Clay_TextElementConfig contains members such as
	// fontId, fontSize, letterSpacing etc
	// Note: Clay_String->chars is not guaranteed to be null terminated
	// This will only work for monospace fonts,
	// see the renderers/ directory for more advanced text measurement

	return (Clay_Dimensions) {
		.width = (float)text.length * (float)config->fontSize,
		.height = (float)config->fontSize
	};
}

void UIManager::Init() {
	uint64_t size = Clay_MinMemorySize();
	Clay_Arena arena =
		Clay_CreateArenaWithCapacityAndMemory(size, malloc(size));

	auto window = Application::As<WindowApplication>()->GetWindow();
	float width = window->GetWidth();
	float height = window->GetHeight();

	// Note: screenWidth and screenHeight will need to come from your environment, Clay doesn't handle window related tasks
	Clay_Initialize(arena,
		(Clay_Dimensions) { width, height },
		(Clay_ErrorHandler) { HandleClayErrors });

	Clear();
}

void UIManager::Close() {

}

void UIManager::Update(TimeStep ts) {
	// float mouseWheelX = Input::GetMouseWheelX();
	// float mouseWheelY = Input::GetMouseWheelY();

	// // mouseover / click / touch events - needed for scrolling and debug tools
	// Clay_UpdateScrollContainers(true,
	// 	(Clay_Vector2) { mouseWheelX, mouseWheelY }, ts);

	auto window = Application::As<WindowApplication>()->GetWindow();
	float width = window->GetWidth();
	float height = window->GetHeight();

	auto mousePositionX = Input::GetMouseX();
	auto mousePositionY = Input::GetMouseY();
	auto isMouseDown = Input::MouseButtonPressed(Mouse::LeftButton);

	// support resizing
	Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
	// / click / touch events - needed for scrolling & debug tools
	Clay_SetPointerState(
		(Clay_Vector2) { mousePositionX, mousePositionY }, isMouseDown);

	Clay_BeginLayout();

	m_Root->Update(ts);
}

void UIManager::Render() {
	m_Root->Render();

	Clay_RenderCommandArray renderCommands = Clay_EndLayout();

	for(int i = 0; i < renderCommands.length; i++) {
		Clay_RenderCommand* cmd = &renderCommands.internalArray[i];

		switch(cmd->commandType) {
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
				Clay_Color color = cmd->renderData.rectangle.backgroundColor;
				glClearColor(color.r, color.g, color.b, color.a);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	}
}

void UIManager::Load(const std::string& path) {
	auto root = LoadPage(path);
	if(!root)
		return;

	VOLCANICORE_LOG_INFO("Loaded '%s'", path.c_str());
	m_Root = root;
	m_Path = path;
}

void UIManager::Reload() {
	if(m_Path != "")
		Load(m_Path);
}

void UIManager::Clear() {
	m_Root = CreateRef<Root>("Root");
}

void Widget::Update(TimeStep ts) {
	if(!Enabled)
		return;

	if(State) {
		if(IsNative)
			OnEvent(State);
		else if(OnEventScript.Func)
			OnEventScript.CallVoid(State);
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

void Widget::Reposition() {

}

const Clay_Color COLOR_LIGHT = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};

// Layout config is just a struct that can be declared statically, or inline
Clay_ElementDeclaration sidebarItemConfig = (Clay_ElementDeclaration) {
	.layout = {
		.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }
	},
	.backgroundColor = COLOR_ORANGE
};

void Root::Begin() {
	// Clay__OpenElementWithId(CLAY_ID("Root"));
	// Clay__ConfigureOpenElement(
	// 	CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
	// 	{
	// 		.layout = {
	// 			.sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
	// 			.padding = CLAY_PADDING_ALL(16), .childGap = 16
	// 		},
	// 		.backgroundColor = { 0, 0, 0, 0 }
	// 	})
	// );
}

void Root::End() {
	// Clay__CloseElement();
}

void Window::Begin() {
	// Clay__OpenElementWithId(CLAY_SID(Clay_String(ID.c_str())));
	// Clay__ConfigureOpenElement(
	// 	CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
	// 	{
	// 		.layout = {
	// 			.sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
	// 			.padding = CLAY_PADDING_ALL(16), .childGap = 16
	// 		},
	// 		.backgroundColor = { 10, 10, 255, 255 }
	// 	})
	// );
}

void Window::End() {
	// Clay__CloseElement();
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

void Gizmo::Begin() {

}

void Gizmo::End() {

}

// void Tab::Begin() {

// }

// void UIManager::RegisterInterface() {
// 	auto* engine = ScriptEngine::Get();

// 	engine->SetDefaultNamespace("Widget");

// 	engine->RegisterFuncdef("void WindowCallback()");
// 	engine->RegisterEnum("WindowFlag");
// 	engine->RegisterEnumValue("WindowFlag", "MenuBar", 0);
// 	engine->RegisterEnumValue("WindowFlag", "TitleBar", 1);
// 	engine->RegisterObjectType("WindowWidget", 0, asOBJ_REF | asOBJ_NOCOUNT);
// 	// engine->RegisterObjectMethod("WindowWidget", "void Render(WindowCallback@)",
// 	// 	asFUNCTION(WindowWidgetRender), asCALL_CDECL_OBJLAST);
// 	// engine->RegisterObjectMethod("WindowWidget", "WindowWidget@ With(WindowFlag)",
// 	// 	asMETHOD(WindowWidget, With), asCALL_THISCALL);
// 	engine->RegisterGlobalFunction("WindowWidget@ Window(string name)",
// 		asFUNCTION(WidgetManager::Window), asCALL_CDECL);

// 	engine->RegisterObjectType("Child", 0, asOBJ_REF | asOBJ_NOCOUNT);
// 	engine->RegisterObjectType("Image", 0, asOBJ_REF | asOBJ_NOCOUNT);
// 	engine->RegisterObjectType("Text", 0, asOBJ_REF | asOBJ_NOCOUNT);

// 	engine->SetDefaultNamespace("");
// }

}