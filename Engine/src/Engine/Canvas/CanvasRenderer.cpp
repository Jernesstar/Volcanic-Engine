#include "CanvasRenderer.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Math.h>

#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Graphics/Platform/RendererAPI.h>

#include "App/App.h"

namespace VolcanicEngine {

// ---- Helpers ---------------------------------------------------------------

// Build an orthographic projection sized to the viewport (origin = top-left).
static Mat4 OrthoForViewport(f32 w, f32 h) {
	return glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
}

// ---- RuntimeCanvasRenderer -------------------------------------------------

RuntimeCanvasRenderer::RuntimeCanvasRenderer() {
	auto window = Application::GetWindow();
	m_ViewportSize = { (f32)window->GetWidth(), (f32)window->GetHeight() };

	m_Output =
		RendererAPI::Get()->CreateFramebuffer(
			{
				.Attachments = {
					{
						AttachmentTarget::Color,
						window->GetWidth(),
						window->GetHeight()
					}
				}
			});
}

RuntimeCanvasRenderer::~RuntimeCanvasRenderer() { }

void RuntimeCanvasRenderer::Update(TimeStep ts) {

}

void RuntimeCanvasRenderer::Begin() {
	auto window = Application::GetWindow();
	m_ViewportSize = { (f32)window->GetWidth(), (f32)window->GetHeight() };

	auto* command = RendererAPI::Get()->NewCommand(nullptr);
	command->Clear = true;
	command->ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
}

// Returns { x, y, w, h } in pixel space.
Vec4 RuntimeCanvasRenderer::ResolveRect(const Entity& entity) {
	if(!entity.Has<UIRectComponent>())
		return { 0.0f, 0.0f, m_ViewportSize.x, m_ViewportSize.y };

	// auto& rect   = entity.Get<UIRectComponent>();
	// auto& anchor = rect.Anchor;

	// // Determine parent bounds – use viewport if the entity has no parent.
	// Vec2 parentPos  = { 0.0f, 0.0f };
	// Vec2 parentSize = m_ViewportSize;

	// auto parent = entity.GetParent();
	// if(parent && parent.Has<UIRectComponent>()) {
	// 	auto& parentRect = parent.Get<UIRectComponent>();
	// 	parentPos  = parentRect.Position;
	// 	parentSize = parentRect.Size;
	// }

	// // Anchor-relative origin, then offset by the element's local position.
	// Vec2 anchorOrigin = parentPos + anchor.Min * parentSize;
	// Vec2 anchorExtent = parentPos + anchor.Max * parentSize;
	// Vec2 anchoredSize = anchorExtent - anchorOrigin;

	// Vec2 resolvedPos  = anchorOrigin + rect.Position;
	// Vec2 resolvedSize = anchoredSize.x > 0 && anchoredSize.y > 0
	// 					? anchoredSize : rect.Size;

	// return { resolvedPos.x, resolvedPos.y, resolvedSize.x, resolvedSize.y };
}

void RuntimeCanvasRenderer::SubmitLayout(const Entity& entity) {
	if(!entity.Has<UILayoutComponent>() || !entity.Has<UIRectComponent>())
		return;

	// auto& layout = entity.Get<UILayoutComponent>();
	// auto& rect   = entity.Get<UIRectComponent>();
	// Vec4  bounds = ResolveRect(entity);

	// // Propagate position/size back so ResolveRect works for children.
	// rect.Position = { bounds.x, bounds.y };
	// rect.Size     = { bounds.z, bounds.w };

	// // Stack children along the chosen axis.
	// f32 cursor = (layout.Direction == UIAxisDirection::Horizontal)
	// 			   ? bounds.x + layout.Padding.x
	// 			   : bounds.y + layout.Padding.y;

	// entity.EachChild(
	// 	[&](Entity child)
	// 	{
	// 		if(!child.Has<UIRectComponent>())
	// 			return;

	// 		auto& childRect = child.Get<UIRectComponent>();

	// 		if(layout.Direction == UIAxisDirection::Horizontal) {
	// 			childRect.Position.x = cursor;
	// 			childRect.Position.y = bounds.y + layout.Padding.y;

	// 			switch(layout.Alignment) {
	// 				case UIAlignment::Center:
	// 					childRect.Position.y =
	// 						bounds.y + (bounds.w - childRect.Size.y) * 0.5f;
	// 					break;
	// 				case UIAlignment::End:
	// 					childRect.Position.y =
	// 						bounds.y + bounds.w
	// 						- childRect.Size.y - layout.Padding.y;
	// 					break;
	// 				default: break;
	// 			}

	// 			cursor += childRect.Size.x + layout.Gap;
	// 		}
	// 		else {
	// 			childRect.Position.x = bounds.x + layout.Padding.x;
	// 			childRect.Position.y = cursor;

	// 			switch(layout.Alignment) {
	// 				case UIAlignment::Center:
	// 					childRect.Position.x =
	// 						bounds.x + (bounds.z - childRect.Size.x) * 0.5f;
	// 					break;
	// 				case UIAlignment::End:
	// 					childRect.Position.x =
	// 						bounds.x + bounds.z
	// 						- childRect.Size.x - layout.Padding.x;
	// 					break;
	// 				default: break;
	// 			}

	// 			cursor += childRect.Size.y + layout.Gap;
	// 		}
	// 	});
}

void RuntimeCanvasRenderer::SubmitImage(const Entity& entity) {
	if(!entity.Has<UIImageComponent>() || !entity.Has<UIRectComponent>())
		return;

	// auto& img    = entity.Get<UIImageComponent>();
	// auto& rect   = entity.Get<UIRectComponent>();
	// Vec4  bounds = ResolveRect(entity);

	// Vec2 drawSize = { bounds.z, bounds.w };
	// if(img.PreserveAspect && img.Image) {
	// 	f32 imgAspect =
	// 		(f32)img.Image->GetWidth() / (f32)img.Image->GetHeight();
	// 	f32 boxAspect = bounds.z / bounds.w;

	// 	if(imgAspect > boxAspect)
	// 		drawSize.y = drawSize.x / imgAspect;
	// 	else
	// 		drawSize.x = drawSize.y * imgAspect;
	// }

	// Vec2 offset = { (bounds.z - drawSize.x) * 0.5f,
	// 				(bounds.w - drawSize.y) * 0.5f };

	// Transform t;
	// t.Translation = { bounds.x + offset.x + drawSize.x * 0.5f,
	// 				  bounds.y + offset.y + drawSize.y * 0.5f, 0.0f };
	// t.Scale       = { drawSize.x, drawSize.y, 1.0f };

	// auto* command = Renderer::NewCommand();
	// command->DepthTesting = DepthTestingMode::Off;
	// command->Blending     = BlendingMode::Alpha;
	// command->Culling      = CullingMode::Off;
	// command->Uniforms
	// .Set("u_ViewProj",
	// 	OrthoForViewport(m_ViewportSize.x, m_ViewportSize.y))
	// .Set("u_Color", rect.Color);

	// if(img.Image)
	// 	command->Uniforms.Set("u_Texture", TextureSlot{ img.Image, 0 });

	// Renderer2D::DrawQuad(img.Image, t);
}

void RuntimeCanvasRenderer::SubmitText(const Entity& entity) {
	if(!entity.Has<UITextComponent>() || !entity.Has<UIRectComponent>())
		return;

	// auto& text   = entity.Get<UITextComponent>();
	// Vec4  bounds = ResolveRect(entity);

	// Transform t;
	// t.Translation = { bounds.x, bounds.y, 0.0f };
	// t.Scale       = { bounds.z, bounds.w, 1.0f };

	// auto* command = Renderer::NewCommand();
	// command->DepthTesting = DepthTestingMode::Off;
	// command->Blending = BlendingMode::Greatest;
	// command->Culling = CullingMode::Off;
	// command->Uniforms
	// .Set("u_ViewProj",
	// 	OrthoForViewport(m_ViewportSize.x, m_ViewportSize.y))
	// .Set("u_Color",    text.FontColor)
	// .Set("u_FontSize", text.FontSize);

	// TODO: Pass text content + font asset once the font system is ready.
	// Renderer2D::DrawText(nullptr, t);
}

void RuntimeCanvasRenderer::SubmitButton(const Entity& entity) {
	if(!entity.Has<UIButtonComponent>() || !entity.Has<UIRectComponent>())
		return;

	// auto& btn    = entity.Get<UIButtonComponent>();
	// auto& rect   = entity.Get<UIRectComponent>();
	// Vec4  bounds = ResolveRect(entity);

	// // Draw background quad.
	// Transform t;
	// t.Translation = { bounds.x + bounds.z * 0.5f,
	// 				  bounds.y + bounds.w * 0.5f, 0.0f };
	// t.Scale       = { bounds.z, bounds.w, 1.0f };

	// auto* command = Renderer::NewCommand();
	// command->DepthTesting = DepthTestingMode::Off;
	// command->Blending     = BlendingMode::Alpha;
	// command->Culling      = CullingMode::Off;
	// command->Uniforms
	// .Set("u_ViewProj",
	// 	OrthoForViewport(m_ViewportSize.x, m_ViewportSize.y))
	// .Set("u_Color", btn.NormalColor);

	// Renderer2D::DrawQuad(btn.NormalColor, t);

	// // Draw label on top if present.
	// if(!btn.Label.empty()) {
	// 	// Reuse a synthetic UITextComponent for the label.
	// 	UITextComponent labelText;
	// 	labelText.Content   = btn.Label;
	// 	labelText.FontSize  = 14.0f;
	// 	labelText.FontColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 	labelText.HAlign    = UIAlignment::Center;

	// 	// TODO: Route through the text path once font support lands.
	// }
}

void RuntimeCanvasRenderer::Render() {

}

}