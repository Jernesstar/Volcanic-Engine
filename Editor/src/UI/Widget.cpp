#include "Widget.h"

#include <iostream>
#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <angelscript.h>
#include <angelscript/add_on/scripthandle/scripthandle.h>
#include <angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <angelscript/add_on/scripthelper/scripthelper.h>
#include <angelscript/add_on/scriptmath/scriptmath.h>
#include <angelscript/add_on/scriptarray/scriptarray.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <ImGuizmo/ImGuizmo.h>

#include <clay/clay.h>

#include <Magma/Core/JSONSerializer.h>
#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Event/Events.h>
#include <VolcaniCore/Core/Input.h>

#include <Magma/Script/ScriptModule.h>
#include <Magma/Script/ScriptClass.h>
#include <Magma/Script/ScriptObject.h>

using namespace VolcaniCore;
using namespace Magma;
using namespace Magma::Script;

namespace Magma::UI {

void Traverse(Ref<Widget> parent, const rapidjson::Value& value) {
	auto id = value["Name"].GetString();
	auto type = value["Type"].GetString();

	Ref<Widget> widget = nullptr;
	if(type == "Window") {
		widget = CreateRef<Window>(id);
	}
	else if(type == "Container") {
		widget = CreateRef<Container>(id);
	}
	else if(type == "Dropdown") {
		widget = CreateRef<Dropdown>(id);
	}
	else if(type == "Button") {
		widget = CreateRef<Button>(id);
	}
	else if(type == "Image") {
		widget = CreateRef<Image>(id);
	}
	else if(type == "Text") {
		widget = CreateRef<Text>(id);
	}
	else if(type == "TextInput") {
		widget = CreateRef<TextInput>(id);
	}
	else if(type == "FileDialog") {
		widget = CreateRef<FileDialog>(id);
	}
	else if(type == "FileEditor") {
		widget = CreateRef<FileEditor>(id);
	}
	else if(type == "Gizmo") {
		widget = CreateRef<Gizmo>(id);
	}
}

void UIManager::Init() {
	int success = gladLoadGL();
	VOLCANICORE_ASSERT(success, "Glad could not load OpenGL");
	VOLCANICORE_LOG_INFO(
		"Successfully loaded OpenGL\n"
		"\tVersion: %s\n"
		"\tGPU: %s", glGetString(GL_VERSION), glGetString(GL_RENDERER));

	glEnable(GL_MULTISAMPLE);				// Smooth edges
	glEnable(GL_FRAMEBUFFER_SRGB);			// Gamma correction
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Smooth cubemap edges
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	auto window = Application::GetWindow();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable
					| ImGuiConfigFlags_ViewportsEnable
					| ImGuiConfigFlags_NavEnableKeyboard
					| ImGuiConfigFlags_NavEnableSetMousePos;
	io.DisplaySize = ImVec2(window->GetWidth(), window->GetHeight());

	ImGui_ImplGlfw_InitForOpenGL(window->GetNativeWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460 core");

	Events::RegisterListener<WindowResizedEvent>(
		[](const WindowResizedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(event.Width, event.Height);
		});

	ImGui::StyleColorsDark();

	io.IniFilename = nullptr;

	m_Root = CreateRef<Container>("Root");
}

void UIManager::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UIManager::BeginFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void UIManager::EndFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	GLFWwindow* context = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(context);
}

void UIManager::Update(TimeStep ts) {
	m_Root->Update(ts);
}

void UIManager::Render() {
	m_Root->Render();
}

void UIManager::Load(const std::string& path) {
	using namespace rapidjson;

	if(path == "") {
		VOLCANICORE_LOG_ERROR("Filename was empty");
		return;
	}

	auto name = fs::path(path).stem().stem().stem().string();
	if(!FileUtils::PathExists(path)) {
		VOLCANICORE_LOG_ERROR(
			"Could not find file with name %s", name.c_str());
		return;
	}

	Document doc;
	doc.Parse(FileUtils::ReadFile(path));

	if(doc.HasParseError()) {
		VOLCANICORE_LOG_INFO("Parsing error %i", (uint32_t)doc.GetParseError());
		return;
	}
	if(!doc.IsObject()) {
		VOLCANICORE_LOG_ERROR("File did not have root object");
		return;
	}

	const auto& root = doc["Root"];
	for(const auto& value : root.GetArray()) {
		Traverse(m_Root, value);
	}
}

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

void Root::Begin() { }

void Root::End() { }

void Window::Begin() {
	if(IsChild) {
		auto childFlags = ImGuiChildFlags_Border
						| ImGuiChildFlags_FrameStyle
						| ImGuiChildFlags_ResizeX
						| ImGuiChildFlags_ResizeY;
		auto windowFlags = ImGuiWindowFlags_NoScrollbar
						 | ImGuiWindowFlags_NoScrollWithMouse
						 | ImGuiWindowFlags_NoTitleBar
						 | ImGuiWindowFlags_NoCollapse;
		ImVec2 size((float)Width, (float)Height);

		ImGui::SetNextWindowPos({ x, y });
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Color);
		ImGui::PushID(this);
		ImGui::BeginChild("##Window", size, childFlags, windowFlags);
		ImGui::PopID();
		ImGui::PopStyleColor();
	}
	else {
		auto windowFlags = ImGuiWindowFlags_NoDocking
						 | ImGuiWindowFlags_NoTitleBar
						 | ImGuiWindowFlags_NoCollapse
						 | ImGuiWindowFlags_NoResize
						 | ImGuiWindowFlags_NoMove
						 | ImGuiWindowFlags_NoBringToFrontOnFocus
						 | ImGuiWindowFlags_NoNavFocus;

		// ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		// ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, Color);
		// ImGui::PushStyleColor(ImGuiCol_Border, BorderColor);
		if(IsRoot) {
			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			if(!x)
				x = viewport->Pos.x;
			if(!y)
				y = viewport->Pos.y;

			if(!Width)
				Width = viewport->Size.x;
			if(!Height)
				Height = viewport->Size.y;

			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 0.0f, 0.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		}
		ImGui::SetNextWindowSize({ Width, Height });
		ImGui::SetNextWindowPos({ x, y });

		if(!IsRoot)
			ImGui::PushID(this);

		ImGui::Begin("##Window", nullptr, windowFlags);
		if(AllowDock) {
			ImGuiID dockspaceID = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f),
				ImGuiDockNodeFlags_PassthruCentralNode);
		}

		if(!IsRoot)
			ImGui::PopID();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar(1 + IsRoot * 2);
	}

	State.Clicked = ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered();
	State.Held = ImGui::IsMouseDown(0) && ImGui::IsWindowHovered();
	State.Released = ImGui::IsMouseReleased(0) && ImGui::IsWindowHovered();
	State.Hovered = ImGui::IsWindowHovered();
	// State.NavFocused = ;
	State.Dragging = ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered();
}

void Window::End() {
	ImGui::End();
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
	ImGui::SetCursorPos({ x, y });
	ImGui::SetNextItemAllowOverlap();
	ImGui::PushID(this);
	ImGui::InvisibleButton("##Image", ImVec2(Width, Height));
	ImGui::PopID();
	auto* drawlist = ImGui::GetWindowDrawList();
	drawlist->AddRectFilled(
		ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
		IM_COL32(Color.r, Color.g, Color.b, Color.a), 0.0f);
}

void Button::End() {

}

void Image::Begin() {
	ImGui::SetCursorPos({ x, y });
	ImGui::Image(
		(ImTextureID)(intptr_t)Content->ID, { Width, Height },
		ImVec2(0, 1), ImVec2(1, 0));
}

void Image::End() {

}

void Text::Begin() {
	ImGui::SetCursorPos({ x, y });
	ImGui::SetWindowFontScale(Scale);
	ImGui::Text(Label.c_str());
}

void Text::End() {
	ImGui::SetWindowFontScale(1.0f);
}

void TextInput::Begin() {

}

void TextInput::End() {

}

void FileDialog::Begin() {

}

void FileDialog::End() {
	auto instance = ImGuiFileDialog::Instance();
	if(StartSelect) {
		IGFD::FileDialogConfig config;
		config.path = StartPath;
		std::string exts;
		for(auto& ext : Extensions)
			exts += "." + ext + ",";
	
		instance->OpenDialog(Title, Title, exts.c_str(), config);
		StartSelect = false;
	}

	// Returns true when an action has been taken (select or cancel)
	if(instance->Display(Title, ImGuiWindowFlags_NoCollapse, { Width, Height }))
	{
		if(instance->IsOk()) {
			std::string path = instance->GetFilePathName();
			OnSelect(path);
		}
		instance->Close();
	}
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

// 	ImVec2 size = ImGui::CalcTextSize(name.c_str());
// 	static const float tabHeight = 6.0f;
// 	float radius = closeButton * tabHeight * 0.5f;
// 	float padding = closeButton * tabHeight + 14;

// 	ImGui::SetNextItemWidth(size.x + padding);

// 	ImGui::PushID(s_Stack.Count());
// 	bool tabItem =
// 		ImGui::BeginTabItem(name.c_str(), nullptr,
// 			closeButton * ImGuiTabItemFlags_NoReorder);
// 	ImGui::PopID();

// 	ImVec2 closeButtonPos;
// 	closeButtonPos.x = ImGui::GetItemRectMax().x - 4.0f*radius;
// 	closeButtonPos.y = ImGui::GetItemRectMin().y + radius;

// 	// ImGuiTabBar* tabBar = ImGui::GetCurrentTabBar();

// 	TabState state;
// 	if(tabItem) {
// 		state.Clicked = ImGui::IsItemClicked(0);
// 		state.Hovered = ImGui::IsItemHovered();
// 		ImGui::EndTabItem();
// 	}

// 	if(closeButton) {
// 		auto closeButtonID = ImGui::GetID((int*)s_Stack.Count());
// 		state.Closed = ImGui::CloseButton(closeButtonID, closeButtonPos);
// 	}

// 	return state;
// }

}