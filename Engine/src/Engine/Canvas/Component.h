#pragma once

#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>

#include "Graphics/Platform/Framebuffer.h"
#include "Graphics/Platform/Texture.h"
#include "ECS/Entity.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEngine {

// ---- Layout ----------------------------------------------------------------

enum class UIAxisDirection { Horizontal, Vertical };
enum class UIAlignment     { Start, Center, End };

// Anchors are normalized [0,1] relative to the parent (or viewport).
// { 0,0 } = top-left, { 1,1 } = bottom-right.
struct UIAnchor {
	Vec2 Min = { 0.0f, 0.0f };
	Vec2 Max = { 1.0f, 1.0f };
};

// ---- UI Components ---------------------------------------------------------

struct UIComponent {
	u8 _;

	UIComponent() = default;
	UIComponent(const UIComponent&) = default;
	virtual ~UIComponent() = default;
};

// Shared transform/bounds applied to every UI element.
struct UIRectComponent : public UIComponent {
	// Position in pixels from the top-left of the parent.
	Vec2 Position = { 0.0f, 0.0f };
	Vec2 Size     = { 100.0f, 100.0f };
	UIAnchor Anchor;
	Vec4 Color    = { 1.0f, 1.0f, 1.0f, 1.0f }; // Tint / background color

	UIRectComponent() = default;
	UIRectComponent(Vec2 pos, Vec2 size, Vec4 color = { 1,1,1,1 })
		: Position(pos), Size(size), Color(color) { }
	UIRectComponent(const UIRectComponent&) = default;
};

// Container that stacks children along an axis with optional padding & gap.
struct UILayoutComponent : public UIComponent {
	UIAxisDirection Direction = UIAxisDirection::Vertical;
	UIAlignment Alignment = UIAlignment::Start;
	Vec2 Padding = { 0.0f, 0.0f }; // x = horizontal, y = vertical
	f32 Gap = 0.0f;

	UILayoutComponent() = default;
	UILayoutComponent(UIAxisDirection dir, UIAlignment align,
					  Vec2 padding = { }, f32 gap = 0.0f)
		: Direction(dir), Alignment(align), Padding(padding), Gap(gap) { }
	UILayoutComponent(const UILayoutComponent&) = default;
};

struct UIImageComponent : public UIComponent {
	Ref<Texture> Image;
	bool PreserveAspect = false;

	UIImageComponent() = default;
	UIImageComponent(Ref<Texture> image, bool preserveAspect = false)
		: Image(image), PreserveAspect(preserveAspect) { }
	UIImageComponent(const UIImageComponent&) = default;
};

struct UITextComponent : public UIComponent {
	std::string  Content;
	f32 FontSize  = 16.0f;
	Vec4 FontColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	UIAlignment HAlign = UIAlignment::Start;

	UITextComponent() = default;
	UITextComponent(const std::string& text, f32 size = 16.0f,
					Vec4 color = { 1,1,1,1 },
					UIAlignment hAlign = UIAlignment::Start)
		: Content(text), FontSize(size), FontColor(color), HAlign(hAlign) { }
	UITextComponent(const UITextComponent&) = default;
};

struct UIButtonComponent : public UIComponent {
	Str Label;
	Vec4 NormalColor   = { 0.2f, 0.2f, 0.2f, 1.0f };
	Vec4 HoveredColor  = { 0.35f, 0.35f, 0.35f, 1.0f };
	Vec4 PressedColor  = { 0.1f, 0.1f, 0.1f, 1.0f };
	Func<void> OnClick;

	UIButtonComponent() = default;
	UIButtonComponent(const std::string& label,
					  Func<void> onClick = nullptr)
		: Label(label), OnClick(onClick) { }
	UIButtonComponent(const UIButtonComponent&) = default;
};

}