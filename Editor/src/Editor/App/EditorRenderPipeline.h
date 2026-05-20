#pragma once

#include <Engine/Scene/Scene.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/RenderPass.h>
#include <Engine/Graphics/Platform/Framebuffer.h>
#include <Engine/Scene/Graphics/RenderPipeline.h>

#include "CameraController.h"

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

	void SetSelectedEntity(ECS::Entity entity) { m_Selected = entity; }
	void SetCamera(Ref<Camera> cam) { m_Camera = cam; }

private:
	Ref<Camera> m_Camera;
	ECS::Entity m_Selected;
	Ref<Framebuffer> m_Output;

	// Grid
	Ref<RenderPass> m_GridPass;

	// Skybox
	Ref<RenderPass> m_SkyboxPass;

	// Geometry
	Ref<RenderPass> m_GeometryPass;
	DrawCommand* m_GeometryCommand = nullptr;

	// Outlining
	Ref<RenderPass> m_MaskPass;
	Ref<RenderPass> m_OutlinePass;

	// Lines
	Ref<RenderPass> m_LinePass;
	DrawCommand* m_LineCommand = nullptr;

	// Billboards
	Ref<RenderPass> m_BillboardPass;
	DrawBuffer* m_BillboardBuffer = nullptr;
	Ref<Texture> m_CameraIcon;
	Ref<Texture> m_DirectionalLightIcon;
	Ref<Texture> m_PointLightIcon;
	Ref<Texture> m_SpotlightIcon;
	Ref<Texture> m_ParticlesIcon;

	bool m_HasCamera = false;
	bool m_HasDirectionalLight = false;
	u32 m_PointLightCount = 0;
	u32 m_SpotlightCount = 0;
	u32 m_ParticleEmitterCount = 0;
	List<std::pair<Vec3, u32>> m_Billboards;

private:
	void AddBillboard(Vec3 pos, u32 type);

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