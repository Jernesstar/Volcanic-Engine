#include "UIRenderer.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/List.h>

#include <VolcaniCore/Event/Events.h>

using namespace VolcaniCore;

namespace Magma::UI {

enum class UIType {
	Window,
	ChildWindow,
	DummyWindow,
	MenuBar,
	Menu,
	TabBar,
	Tab
};

static List<UIType> s_Stack;

static ImVec2 CalcPosition(UIElement* element) {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	float width = viewport->Size.x;
	float height = viewport->Size.y;
	float x = viewport->Pos.x;
	float y = viewport->Pos.y;

	float alignX = 0;
	float alignY = 0;
	switch(element->xAlignment) {
		case XAlignment::Center:
			alignX = width / 2;
			break;
		case XAlignment::Right:
			alignX = width;
			x = -x;
			break;
	}
	switch(element->yAlignment) {
		case YAlignment::Center:
			alignY = height / 2;
			break;
		case YAlignment::Bottom:
			alignY = height;
			y = -y;
			break;
	}

	return ImVec2{ x + alignX + element->x, y + alignY + element->y };
}

UIState UIRenderer::DrawWindow(UI::Window& window) {

	return {
		ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(),
		ImGui::IsWindowHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

static void (*ButtonFunction)(Ref<UIElement>, ImVec2);

static void ButtonText(Ref<UIElement> element, ImVec2 dim) {
	std::string text = element->As<Text>()->Content;
	ImGui::Button(text.c_str(), dim);
}

static void ButtonImage(Ref<UIElement> element, ImVec2 dim) {
	if(!element->As<Image>()->Content)
		return;

	auto tex = element->As<Image>()->Content;
	auto id = (ImTextureID)(intptr_t)tex->ID;
	ImGui::ImageButton("##Image", id, dim, ImVec2(0, 1), ImVec2(1, 0));
}

UIState UIRenderer::DrawButton(UI::Button& button) {
	if(button.Display->GetType() == UIElementType::Image)
		ButtonFunction = ButtonImage;
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, button.Color);
		ButtonFunction = ButtonText;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
	if(button.UsePosition)
		ImGui::SetCursorPos(ImVec2(button.x, button.y));
	ButtonFunction(button.Display, ImVec2(button.Width, button.Height));
	ImGui::PopStyleVar();

	if(button.Display->GetType() == UIElementType::Text)
		ImGui::PopStyleColor();

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(),
		ImGui::IsMouseDown(0) && ImGui::IsItemHovered(),
	};
}

UIState UIRenderer::DrawImage(UI::Image& image) {
	auto texture = image.Content;

	ImVec2 dim = ImVec2(image.Width, image.Height);
	if(image.UsePosition)
		ImGui::SetCursorPos(ImVec2(image.x, image.y));
	ImGui::Image((ImTextureID)(intptr_t)texture->ID, dim, ImVec2(0, 1), ImVec2(1, 0));

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

UIState UIRenderer::DrawText(UI::Text& text) {
	if(text.Content == "")
		return { };

	ImVec2 size = ImGui::CalcTextSize(text.Content.c_str());
	text.Width = size.x;
	text.Height = size.y;

	ImGui::PushStyleColor(ImGuiCol_Text, text.Color);
	ImGui::SetCursorPos(ImVec2(text.x, text.y));
	ImGui::Text(text.Content.c_str());
	ImGui::PopStyleColor();

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

UIState UIRenderer::DrawTextInput(TextInput& textInput) {
	// char input[textInput.GetMaxCharCount()]{""};
	char input[255]{ "" };

	ImGui::PushID(&textInput);
	ImGui::InputText("##TextInput", input, sizeof(input));
	textInput.Text = std::string(input);
	ImGui::PopID();

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

UIState UIRenderer::DrawDropdown(Dropdown& dropdown) {
	if(ImGui::BeginCombo("##Combo", dropdown.GetSelected().c_str()))
	{
		for(uint32_t i = 0; i < dropdown.Options.Count(); i++) {
			auto item = dropdown.Options[i];
			bool isSelected = dropdown.CurrentItem == i;
			if(ImGui::Selectable(item.c_str(), isSelected))
				dropdown.CurrentItem = i;
			if(isSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	return {
		// ImGui::IsItemClicked(),
		// ImGui::IsItemHovered(),
		// ImGui::IsMouseReleased(0),
		// ImGui::IsMouseDown(0),
	};
}

static Map<std::string, FileDialog> s_Dialogs;

void UIRenderer::DrawFileDialog(FileDialog& dialog) {
	if(dialog.Title == "")
		return;
	if(s_Dialogs.contains(dialog.Title))
		return;

	auto instance = ImGuiFileDialog::Instance();
	IGFD::FileDialogConfig config;
	config.path = dialog.StartPath;
	std::string exts;
	for(auto& ext : dialog.Extensions)
		exts += "." + ext + ",";

	instance->OpenDialog(dialog.Title, dialog.Title, exts.c_str(), config);
	s_Dialogs.emplace(dialog.Title, dialog);
}

UIState UIRenderer::DrawMenuBar(const std::string& name) {
	ImGui::BeginMainMenuBar();

	s_Stack.Add(UIType::MenuBar);

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

UIState UIRenderer::DrawMenu(const std::string& name) {
	ImGui::BeginMenu(name.c_str());

	s_Stack.Add(UIType::Menu);

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

UIState UIRenderer::DrawTabBar(const std::string& name) {
	s_Stack.Add(UIType::TabBar);

	ImGui::BeginTabBar(name.c_str(), ImGuiTabBarFlags_Reorderable);

	return {
		ImGui::IsItemClicked(),
		ImGui::IsItemHovered(),
		ImGui::IsMouseReleased(0),
		ImGui::IsMouseDown(0),
	};
}

TabState UIRenderer::DrawTab(const std::string& name, bool closeButton) {
	s_Stack.Add(UIType::Tab);

	ImVec2 size = ImGui::CalcTextSize(name.c_str());
	static const float tabHeight = 6.0f;
	float radius = closeButton * tabHeight * 0.5f;
	float padding = closeButton * tabHeight + 14;

	ImGui::SetNextItemWidth(size.x + padding);

	ImGui::PushID(s_Stack.Count());
	bool tabItem =
		ImGui::BeginTabItem(name.c_str(), nullptr,
			closeButton * ImGuiTabItemFlags_NoReorder);
	ImGui::PopID();

	ImVec2 closeButtonPos;
	closeButtonPos.x = ImGui::GetItemRectMax().x - 4.0f*radius;
	closeButtonPos.y = ImGui::GetItemRectMin().y + radius;

	// ImGuiTabBar* tabBar = ImGui::GetCurrentTabBar();

	TabState state;
	if(tabItem) {
		state.Clicked = ImGui::IsItemClicked(0);
		state.Hovered = ImGui::IsItemHovered();
		ImGui::EndTabItem();
	}

	if(closeButton) {
		auto closeButtonID = ImGui::GetID((int*)s_Stack.Count());
		state.Closed = ImGui::CloseButton(closeButtonID, closeButtonPos);
	}

	return state;
}

void UIRenderer::ShowPopupLabel(const std::string& str) {
	ImGui::SetItemTooltip(str.c_str());
}

void UIRenderer::Pop(uint64_t count) {
	count = count ? count : s_Stack.Count();
	while(count--) {
		auto type = s_Stack.Pop();

		switch(type) {
			case UIType::Window:
				ImGui::End();
				break;
			case UIType::ChildWindow:
				ImGui::EndChild();
				break;
			case UIType::DummyWindow:
				break;
			case UIType::MenuBar:
				ImGui::EndMainMenuBar();
				break;
			case UIType::Menu:
				ImGui::EndMenu();
				break;
			case UIType::TabBar:
				ImGui::EndTabBar();
				break;
		}
	}
}

void UIRenderer::BeginFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}

void UIRenderer::EndFrame() {
	Pop(0);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	GLFWwindow* context = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(context);
}

void UIRenderer::Init() {
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

	Events::RegisterListener<KeyPressedEvent>(
		[](KeyPressedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureKeyboard;
		});
	Events::RegisterListener<KeyReleasedEvent>(
		[](KeyReleasedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureKeyboard;
		});
	Events::RegisterListener<MouseButtonPressedEvent>(
		[](MouseButtonPressedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureMouse;
		});
	Events::RegisterListener<MouseButtonReleasedEvent>(
		[](MouseButtonReleasedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureMouse;
		});
	Events::RegisterListener<MouseScrolledEvent>(
		[](MouseScrolledEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureMouse;
		});
	Events::RegisterListener<MouseMovedEvent>(
		[](MouseMovedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			// event.Handled = io.WantCaptureMouse;
		});
	Events::RegisterListener<WindowResizedEvent>(
		[](const WindowResizedEvent& event)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(event.Width, event.Height);
		});

	ImGui::StyleColorsDark();
}

void UIRenderer::Close() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

}