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
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <VolcaniCore/Core/Input.h>

#include <Magma/Script/ScriptModule.h>
#include <Magma/Script/ScriptClass.h>
#include <Magma/Script/ScriptObject.h>

using namespace Magma;
using namespace Magma::UI;
using namespace Magma::Script;

namespace Magma {

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

		ImGui::SetNextWindowSize({ Width, Height });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::PushStyleColor(ImGuiCol_WindowBg, Color);
		// ImGui::PushStyleColor(ImGuiCol_Border, BorderColor);
		ImGui::PushID(this);
		ImGui::Begin("##Window", nullptr, windowFlags);
		ImGui::PopID();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
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

void WidgetManager::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->SetDefaultNamespace("Widget");

	engine->RegisterFuncdef("void WindowCallback()");
	engine->RegisterEnum("WindowFlag");
	engine->RegisterEnumValue("WindowFlag", "MenuBar", 0);
	engine->RegisterEnumValue("WindowFlag", "TitleBar", 1);
	engine->RegisterObjectType("WindowWidget", 0, asOBJ_REF | asOBJ_NOCOUNT);
	// engine->RegisterObjectMethod("WindowWidget", "void Render(WindowCallback@)",
	// 	asFUNCTION(WindowWidgetRender), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectMethod("WindowWidget", "WindowWidget@ With(WindowFlag)",
	// 	asMETHOD(WindowWidget, With), asCALL_THISCALL);
	engine->RegisterGlobalFunction("WindowWidget@ Window(string name)",
		asFUNCTION(WidgetManager::Window), asCALL_CDECL);

	engine->RegisterObjectType("Child", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectType("Image", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectType("Text", 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine->SetDefaultNamespace("");
}

void WidgetManager::BeginFrame() {

}

void WidgetManager::EndFrame() {

}

}