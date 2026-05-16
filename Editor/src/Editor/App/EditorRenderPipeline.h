#pragma once

#include <Engine/Scene/Scene.h>
#include <Engine/Graphics/RenderPass.h>
#include <Engine/Graphics/Platform/Framebuffer.h>
#include <Engine/Scene/Graphics/RenderPipeline.h>

using namespace VolcanicEngine::ECS;

namespace VolcanicEngine {

class EditorRenderPipeline : public RenderPipeline {
public:
	EditorRenderPipeline() = default;
	~EditorRenderPipeline() = default;

	void OnInit() override;
	void OnRender(Scene* scene) override;
	void OnResize(u32 w, u32 h) override;

	Ref<Framebuffer> GetOutput() const override { return m_OutputBuffer; }

	// Entity outline for selection highlight
	void SetSelectedEntity(ECS::Entity entity) { m_Selected = entity; }

private:
	ECS::Entity m_Selected;
	Ref<Framebuffer> m_OutputBuffer;

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
	void End3D();

	void Begin2D();
	void End2D();

	void BeginCanvas();
	void EndCanvas();
};

}