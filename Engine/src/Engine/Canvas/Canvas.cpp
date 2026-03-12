#include "Canvas.h"

#include "CanvasRenderer.h"

using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

Canvas::Canvas(const std::string& name)
	: Name(name) { }

void Canvas::OnUpdate(TimeStep ts) {
	EntityWorld.OnUpdate(ts);

	// auto object = element->ScriptInstance;
	// if(!object)
	// 	return;

	// object->Call("OnUpdate", (f32)ts);

	// UIState state = element->GetState();
	// if(state.Clicked)
	// 	object->Call("OnClick");
	// if(state.Hovered)
	// 	object->Call("OnHover");
	// if(state.MouseUp)
	// 	object->Call("OnMouseUp");
	// if(state.MouseDown)
	// 	object->Call("OnMouseDown");
}

void Canvas::OnRender(CanvasRenderer& renderer) {
	auto& world = EntityWorld.GetNative();

	renderer.Begin();

	world.query_builder()
	.with<UILayoutComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitLayout(Entity{ id });
		});

	world.query_builder()
	.with<UIImageComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitImage(Entity{ id });
		});

	world.query_builder()
	.with<UITextComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitText(Entity{ id });
		});

	world.query_builder()
	.with<UIButtonComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitButton(Entity{ id });
		});

	renderer.Render();
}

}