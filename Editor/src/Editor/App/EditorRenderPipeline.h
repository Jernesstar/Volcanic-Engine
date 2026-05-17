#pragma once

#include <Engine/Scene/Scene.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/RenderPass.h>
#include <Engine/Graphics/Platform/Framebuffer.h>
#include <Engine/Scene/Graphics/RenderPipeline.h>

using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::ECS;

namespace VolcanicEditor {

class EditorRenderPipeline : public RenderPipeline {
public:
	EditorRenderPipeline() = default;
	~EditorRenderPipeline() = default;

	void OnInit() override;
	void OnRender(Scene* scene) override;
	void OnResize(u32 w, u32 h) override;

	Ref<Framebuffer> GetOutput() const override { return m_Output; }

	// Entity outline for selection highlight
	void SetSelectedEntity(ECS::Entity entity) { m_Selected = entity; }
	void SetCamera(Ref<Camera> cam) { m_Camera = cam; }

private:
	Ref<Camera> m_Camera;
	ECS::Entity m_Selected;
	Ref<Framebuffer> m_Output;

	Entity Selected;
	bool Hovered = false;

	// Grid
	Ref<RenderPass> GridPass;

	// Outlining
	Ref<RenderPass> MaskPass;
	Ref<RenderPass> OutlinePass;

	// Lines
	Ref<RenderPass> LinePass;
	DrawCommand* LineCommand;

	// Billboards
	Ref<RenderPass> BillboardPass;
	DrawBuffer* BillboardBuffer;
	Ref<Texture> DirectionalLightIcon;
	Ref<Texture> PointLightIcon;
	Ref<Texture> SpotlightIcon;
	Ref<Texture> CameraIcon;
	Ref<Texture> ParticlesIcon;

	Ref<RenderPass> GeometryPass;
	DrawCommand* GeometryCommand;

	bool HasCamera = false;
	bool HasDirectionalLight = false;
	u32 PointLightCount = 0;
	u32 SpotlightCount = 0;
	u32 ParticleEmitterCount = 0;
	List<std::pair<Vec3, u32>> Billboards;

private:
	void Begin3D();
	void SubmitCamera3D(const Entity& entity);
	void SubmitSkybox(const Entity& entity);
	void SubmitLight3D(const Entity& entity);
	void SubmitParticles(const Entity& entity);
	void SubmitGeometry(const Entity& entity);
	void AddBillboard(Vec3 position, u32 type);
	void End3D();

	void Begin2D();
	void End2D();

	void BeginCanvas();
	void EndCanvas();
};

}