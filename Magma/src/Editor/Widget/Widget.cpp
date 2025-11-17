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

#include "Graphics/Renderer.h"
#include "Utils/JSONSerializer.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;
using namespace Magma;
using namespace Magma::Graphics;

using namespace Lava::Script;

namespace Magma::UI {

static Clay_Context* s_ClayContext;

static void HandleClayErrors(Clay_ErrorData errorData) {
	VOLCANICORE_LOG_ERROR("%s", errorData.errorText.chars);
	switch(errorData.errorType) {
		// etc
	}
}

// Example measure text function
static Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, void* userData) {
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

void WidgetManager::Load(const std::string& path) {
	
}

void WidgetManager::Init() {
	uint64_t size = Clay_MinMemorySize();
	Clay_Arena arena =
		Clay_CreateArenaWithCapacityAndMemory(size, malloc(size));

	auto window = Application::As<WindowApplication>()->GetWindow();
	float width = window->GetWidth();
	float height = window->GetHeight();

	Clay_Initialize(arena,
		(Clay_Dimensions) { width, height },
		(Clay_ErrorHandler) { HandleClayErrors });

	Clay_SetMeasureTextFunction(MeasureText, nullptr);
}

void WidgetManager::Close() {

}

void WidgetManager::Update(TimeStep ts) {
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

	Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
	// / click / touch events - needed for scrolling & debug tools
	Clay_SetPointerState(
		(Clay_Vector2) { mousePositionX, mousePositionY }, isMouseDown);

	Clay_BeginLayout();
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
	GetRoot()->Render();

	Clay_RenderCommandArray renderCommands = Clay_EndLayout();

	for(int i = 0; i < renderCommands.length; i++) {
		Clay_RenderCommand* cmd = &renderCommands.internalArray[i];

		switch(cmd->commandType) {
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
				auto box = cmd->boundingBox;
				auto rect = cmd->renderData.rectangle;
				Renderer::DrawQuad({
					.PosX = box.x,
					.PosY = box.y,
					.Width = box.width,
					.Height = box.height,
					.Color = {
						rect.backgroundColor.r,
						rect.backgroundColor.g,
						rect.backgroundColor.b,
						rect.backgroundColor.a
					}
				});
			}
			case CLAY_RENDER_COMMAND_TYPE_BORDER: {

			}
			case CLAY_RENDER_COMMAND_TYPE_IMAGE: {

			}
			case CLAY_RENDER_COMMAND_TYPE_TEXT: {

			}
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {

			}
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {

			}
			case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
				auto data = (CustomElement*)cmd->renderData.custom.customData;
				switch(data->type) {
					case CUSTOM_ELEMENT_TYPE_GIF: {

					}
					case CUSTOM_ELEMENT_TYPE_VIDEO: {

					}
				}
			}
		}
	}
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

void Root::Begin() {
	Clay__OpenElementWithId(CLAY_ID("Root"));
	Clay__ConfigureOpenElement(
		CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
		{
			.layout = {
				.sizing = {
					CLAY_SIZING_GROW(),
					CLAY_SIZING_GROW()
				},
				.layoutDirection = CLAY_TOP_TO_BOTTOM
			},
			.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f }
		})
	);
}

void Root::End() {
	Clay__CloseElement();
}

void Window::Begin() {
	Clay__OpenElementWithId(CLAY_SID(Clay_String(ID.c_str())));
	Clay__ConfigureOpenElement(
		CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
		{
			.layout = {
				.sizing = {
					CLAY_SIZING_FIXED(Width),
					CLAY_SIZING_FIXED(Height)
				},
				// .padding = CLAY_PADDING_ALL(16),
				// .childGap = 16
			},
			.backgroundColor = { Color.r, Color.g, Color.b, Color.a }
		})
	);
}

void Window::End() {
	Clay__CloseElement();
}

void Container::Begin() {
	Clay__OpenElementWithId(CLAY_SID(Clay_String(ID.c_str())));
	Clay__ConfigureOpenElement(
		CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
		{
			.layout = {
				.sizing = {
					SizeX == SizeType::Fixed ?
						CLAY_SIZING_FIXED(Width) : CLAY_SIZING_GROW(),
					SizeY == SizeType::Fixed ?
						CLAY_SIZING_FIXED(Height) : CLAY_SIZING_GROW()
				},
				.layoutDirection =
					Layout == LayoutType::Horizontal ?
								  CLAY_LEFT_TO_RIGHT : CLAY_TOP_TO_BOTTOM,
			},
			.backgroundColor = { Color.r, Color.g, Color.b, Color.a }
		})
	);
}

void Container::End() {
	Clay__CloseElement();
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
	Clay__OpenElementWithId(CLAY_SID(Clay_String(ID.c_str())));
	Clay__ConfigureOpenElement(
		CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration,
		{
			.layout = {
				.sizing = {
					CLAY_SIZING_FIXED(Width),
					CLAY_SIZING_FIXED(Height)
				}
			},
			.image = { .imageData = (void*)Texture }
		})
	);
}

void Image::End() {
	Clay__CloseElement();
}

void Text::Begin() {
	Clay__OpenElementWithId(CLAY_SID(Clay_String(ID.c_str())));
	CLAY_TEXT(CLAY_STRING("Text"),
		CLAY_TEXT_CONFIG({
			.textColor = { Color.r, Color.g, Color.b, Color.a },
			.fontId = (u16)Font,
			.fontSize = (u16)Scale,
		})
	);
}

void Text::End() {
	Clay__CloseElement();
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

// void WidgetManager::RegisterInterface() {
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