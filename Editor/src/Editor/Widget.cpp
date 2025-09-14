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

#include "UI/UI.h"

using namespace Magma;
using namespace Magma::UI;
using namespace Magma::Script;

namespace Magma {

static List<WindowWidget> s_Windows;

static void WindowWidgetRender(asIScriptFunction* ptr, WindowWidget* w) {
	auto* context = ScriptEngine::GetContext();
	ScriptFunc func = { ptr, context, nullptr };
	w->Render(func);
	ptr->Release();
}

void WidgetRenderer::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->SetDefaultNamespace("Widget");

	engine->RegisterFuncdef("void WindowCallback()");
	engine->RegisterEnum("WindowFlag");
	engine->RegisterEnumValue("WindowFlag", "MenuBar", 0);
	engine->RegisterEnumValue("WindowFlag", "TitleBar", 1);
	engine->RegisterObjectType("WindowWidget", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod("WindowWidget", "void Render(WindowCallback@)",
		asFUNCTION(WindowWidgetRender), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("WindowWidget", "WindowWidget@ With(WindowFlag)",
		asMETHOD(WindowWidget, With), asCALL_THISCALL);
	engine->RegisterGlobalFunction("WindowWidget@ Window(string name)",
		asFUNCTION(WidgetRenderer::Window), asCALL_CDECL);

	engine->RegisterObjectType("Child", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectType("Image", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectType("Text", 0, asOBJ_REF | asOBJ_NOCOUNT);

	engine->SetDefaultNamespace("");
}

void WidgetRenderer::BeginFrame() {

}

void WidgetRenderer::EndFrame() {
	s_Windows.Clear();
}

void WindowWidget::Render(ScriptFunc func) {
	ImGui::PushID(s_Windows.Count());
	ImGui::Begin(Name.c_str());

	func.CallVoid();

	ImGui::End();
	ImGui::PopID();
}

WindowWidget* WindowWidget::With(WindowWidget::Options option) {

	return this;
}

WindowWidget* WidgetRenderer::Window(const std::string& name) {
	return &s_Windows.Emplace(name);
}

}