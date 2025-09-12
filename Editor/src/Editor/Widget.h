#pragma once

#include <Magma/Script/ScriptEngine.h>

using namespace Magma::Script;

namespace Magma {

class Widget {
public:
	virtual void Render(ScriptFunc func) = 0;
};

class WindowWidget {
public:
	enum class Options {
		MenuBar, TitleBar
	};

	WindowWidget* With(WindowWidget::Options option);
	void Render(ScriptFunc func);
};

class WidgetRenderer {
public:
	static void RegisterInterface();
	static void BeginFrame();
	static void EndFrame();

	static WindowWidget* Window();
	// static ChildWidget* ChildWindow();
	// static TextWidget* Text();
	// static ImageWidget* Image();
};

}