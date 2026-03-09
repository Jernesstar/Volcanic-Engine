#pragma once

#include <VolcaniCore/Core/Math.h>

#include "Codes.h"

using namespace VolcaniCore;

namespace VolcaniCore {

class Input {
public:
	static bool KeyPressed(Key key);
	static bool KeysPressed(Key key1, Key key2);
	static bool KeysPressed(Key key1, Key key2, Key key3);
	static bool KeysPressed(Key key1, Key key2, Key key3, Key key4);

	static void SetCursorMode(CursorMode mode);
	static CursorMode GetCursorMode();

	static bool MouseButtonPressed(Mouse mouse_button);

	static void SetMousePosition(f64 x, f64 y);

	static glm::dvec2 GetMousePosition();
	static f64 GetMouseX();
	static f64 GetMouseY();

private:
	Input() = delete;
	~Input() = delete;
};

}