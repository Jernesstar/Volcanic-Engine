#pragma once

#include "UIElement.h"

using namespace VolcaniCore;

namespace Magma::UI {

class FileDialog : public UIElement {
public:
	std::string Title;
	std::string StartPath;
	bool OpenDir = false;
	List<std::string> Extensions;
	Func<void, std::string&> OnSelect;

public:
	FileDialog()
		: UIElement(UIElementType::Button) { }

	void Draw() override;
};

}