#include "Scene.h"

#include "PhysicsSystem.h"
#include "ScriptSystem.h"

#include "SceneRenderer.h"

using namespace VolcanicEngine::ECS;
// using namespace VolcanicEngine::Physics;

namespace VolcanicEngine {

Scene::Scene(const std::string& name)
	: Name(name) { }

void Scene::OnUpdate(TimeStep ts) {
	World3D.OnUpdate(ts);
	World2D.OnUpdate(ts);
	Canvas.OnUpdate(ts);
}

void Scene::OnRender(SceneRenderer& renderer) {
	auto& world = World3D.GetNative();

	renderer.Begin();

	world.query_builder()
	.with<CameraComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitCamera(Entity{ id });
		});

	world.query_builder()
	.with<SkyboxComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitSkybox(Entity{ id });
		});

	world.query_builder()
	.with<DirectionalLightComponent>().or_()
	.with<PointLightComponent>().or_()
	.with<SpotlightComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitLight(Entity{ id });
		});

	world.query_builder()
	.with<ParticleEmitterComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitParticles(Entity{ id });
		});

	world.query_builder()
	.with<MeshComponent>().and_().with<TransformComponent>()
	.build()
	.each(
		[&](flecs::entity id)
		{
			renderer.SubmitMesh(Entity{ id });
		});

	// world.query_builder()
	// .with<LayoutComponent>()
	// .build()
	// .each(
	// 	[&](flecs::entity id)
	// 	{
	// 		renderer.SubmitLayout(Entity{ id });
	// 	});

	// world.query_builder()
	// .with<ImageComponent>()
	// .build()
	// .each(
	// 	[&](flecs::entity id)
	// 	{
	// 		renderer.SubmitImage(Entity{ id });
	// 	});

	// world.query_builder()
	// .with<TextComponent>()
	// .build()
	// .each(
	// 	[&](flecs::entity id)
	// 	{
	// 		renderer.SubmitText(Entity{ id });
	// 	});

	// world.query_builder()
	// .with<ButtonComponent>()
	// .build()
	// .each(
	// 	[&](flecs::entity id)
	// 	{
	// 		renderer.SubmitButton(Entity{ id });
	// 	});

	renderer.Render();
}

void Scene::RegisterSystems() {
	World3D.Add<PhysicsSystem>();
	World3D.Add<ScriptSystem>();

	for(auto phase : { flecs::PreUpdate, flecs::OnUpdate, flecs::PostUpdate }) {
		Phase ourPhase;
		if(phase == flecs::PreUpdate)
			ourPhase = Phase::PreUpdate;
		if(phase == flecs::OnUpdate)
			ourPhase = Phase::OnUpdate;
		if(phase == flecs::PostUpdate)
			ourPhase = Phase::PostUpdate;

		World3D.GetNative()
		.system<RigidBodyComponent>()
		.kind(phase)
		.run(
			[=, this](flecs::iter& it)
			{
				while(it.next()) {
					auto sys = World3D.Get<PhysicsSystem>();
					if(!sys)
						continue;

					for(auto i : it) {
						Entity entity{ it.entity(i) };
						sys->Run(entity, it.delta_time(), ourPhase);
					}
				}
			});
	}

	for(auto phase : { flecs::PreUpdate }) {
		Phase ourPhase;
		if(phase == flecs::PreUpdate)
			ourPhase = Phase::PreUpdate;
		if(phase == flecs::OnUpdate)
			ourPhase = Phase::OnUpdate;
		if(phase == flecs::PostUpdate)
			ourPhase = Phase::PostUpdate;

		World3D.GetNative()
		.system<ScriptComponent>()
		.kind(phase)
		.run(
			[=, this](flecs::iter& it)
			{
				while(it.next()) {
					auto sys = World3D.Get<ScriptSystem>();
					if(!sys)
						continue;

					for(auto i : it) {
						Entity entity{ it.entity(i) };
						sys->Run(entity, it.delta_time(), ourPhase);
					}
				}
			});
	}

	World3D.GetNative()
	.system("ScriptUpdate")
	.kind(flecs::OnUpdate)
	.run(
		[=, this](flecs::iter& it)
		{
			auto sys = World3D.Get<ScriptSystem>();
			if(!sys)
				return;
			sys->Update(it.delta_time());
		});

	World3D.GetNative()
	.system("PhysicsUpdate")
	.kind(flecs::OnUpdate)
	.run(
		[=, this](flecs::iter& it)
		{
			auto sys = World3D.Get<PhysicsSystem>();
			if(!sys)
				return;
			sys->Update(it.delta_time());
		});

	World3D.GetNative()
	.observer()
	.with<RigidBodyComponent>()
	.event(flecs::Monitor)
	.each(
		[=, this](flecs::iter& it, size_t i)
		{
			auto sys = World3D.Get<PhysicsSystem>();
			if(!sys)
				return;

			Entity entity{ it.entity(i) };
			if(it.event() == flecs::OnAdd)
				sys->OnComponentAdd(entity);
			else if(it.event() == flecs::OnSet)
				sys->OnComponentSet(entity);
			else if(it.event() == flecs::OnRemove)
				sys->OnComponentRemove(entity);
		});

	World3D.GetNative()
	.observer()
	.with<ScriptComponent>()
	.event(flecs::Monitor)
	.each(
		[=, this](flecs::iter& it, size_t i)
		{
			auto sys = World3D.Get<ScriptSystem>();
			if(!sys)
				return;

			Entity entity{ it.entity(i) };
			if(it.event() == flecs::OnAdd)
				sys->OnComponentAdd(entity);
			else if(it.event() == flecs::OnSet)
				sys->OnComponentSet(entity);
			else if(it.event() == flecs::OnRemove)
				sys->OnComponentRemove(entity);
		});
}

void Scene::UnregisterSystems() {
	World3D.Remove<ScriptSystem>();
	World3D.Remove<PhysicsSystem>();
}

}