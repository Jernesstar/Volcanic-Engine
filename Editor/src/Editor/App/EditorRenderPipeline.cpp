#include "EditorRenderPipeline.h"

#include <VolcaniCore/Core/Application.h>

#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer3D.h>
#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Asset/AssetManager.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Component.h>

namespace VolcanicEngine {

static constexpr u32 DEFAULT_W = 1920;
static constexpr u32 DEFAULT_H = 1080;

void EditorRenderPipeline::OnInit() {
	// auto libPath = Application::GetLibraryDir();

	// // ── Output framebuffer (colour + depth) ───────────────────────────────
	// m_OutputBuffer = RendererAPI::Get()->CreateFramebuffer({
	// 	.Attachments = {
	// 		{ AttachmentTarget::Color, DEFAULT_W, DEFAULT_H },
	// 		{ AttachmentTarget::Depth, DEFAULT_W, DEFAULT_H }
	// 	},
	// 	.EnableRead = true
	// });

	// // ── Geometry pass — opaque meshes, basic PBR/Blinn-Phong ─────────────
	// m_GeometryPass = RenderPass::Create("EditorGeometry",
	// 	AssetImporter::GetShader({
	// 		libPath + "/Engine/assets/Shaders/Mesh.glsl.vert",
	// 		libPath + "/Engine/assets/Shaders/Mesh.glsl.frag"
	// 	}), m_OutputBuffer);

	// // ── Skybox pass ────────────────────────────────────────────────────────
	// m_SkyboxPass = RenderPass::Create("EditorSkybox",
	// 	AssetImporter::GetShader({
	// 		libPath + "/Engine/assets/Shaders/Skybox.glsl.vert",
	// 		libPath + "/Engine/assets/Shaders/Skybox.glsl.frag"
	// 	}), m_OutputBuffer);

	// // ── Outline pass — stencil-based selection highlight ──────────────────
	// m_OutlinePass = RenderPass::Create("EditorOutline",
	// 	AssetImporter::GetShader({
	// 		libPath + "/Engine/assets/Shaders/Outline.glsl.vert",
	// 		libPath + "/Engine/assets/Shaders/Outline.glsl.frag"
	// 	}), m_OutputBuffer);
}

void EditorRenderPipeline::OnRender(Scene* scene) {
	// // Pull the active camera from the scene
	// Ref<Camera> activeCamera;
	// scene->World3D.ForEach<CameraComponent>(
	// 	[&](ECS::Entity& entity) {
	// 		if(!activeCamera)
	// 			activeCamera = entity.Get<CameraComponent>().Cam;
	// 	});

	// if(!activeCamera)
	// 	return;

	// // ── Skybox ─────────────────────────────────────────────────────────────
	// Renderer::StartPass(m_SkyboxPass);
	// {
	// 	scene->World3D.ForEach<SkyboxComponent>(
	// 		[&](ECS::Entity& entity) {
	// 			Renderer3D::DrawSkybox(
	// 				entity.Get<SkyboxComponent>().CubemapAsset,
	// 				activeCamera);
	// 		});
	// }
	// Renderer::EndPass();

	// // ── Geometry ───────────────────────────────────────────────────────────
	// Renderer::StartPass(m_GeometryPass);
	// {
	// 	// Submit lights
	// 	scene->World3D.ForEach<DirectionalLightComponent>(
	// 		[&](ECS::Entity& entity) {
	// 			Renderer3D::SubmitLight(entity.Get<DirectionalLightComponent>());
	// 		});
	// 	scene->World3D.ForEach<PointLightComponent>(
	// 		[&](ECS::Entity& entity) {
	// 			Renderer3D::SubmitLight(entity.Get<PointLightComponent>());
	// 		});
	// 	scene->World3D.ForEach<SpotlightComponent>(
	// 		[&](ECS::Entity& entity) {
	// 			Renderer3D::SubmitLight(entity.Get<SpotlightComponent>());
	// 		});

	// 	// Submit meshes
	// 	scene->World3D.ForEach<MeshComponent, TransformComponent>(
	// 		[&](ECS::Entity& entity) {
	// 			Renderer3D::DrawMesh(
	// 				entity.Get<MeshComponent>(),
	// 				entity.Get<TransformComponent>(),
	// 				activeCamera);
	// 		});
	// }
	// Renderer::EndPass();

	// // ── Outline (selected entity) ──────────────────────────────────────────
	// if(m_Selected && m_Selected.Has<MeshComponent, TransformComponent>()) {
	// 	Renderer::StartPass(m_OutlinePass);
	// 	{
	// 		Renderer3D::DrawMeshOutline(
	// 			m_Selected.Get<MeshComponent>(),
	// 			m_Selected.Get<TransformComponent>(),
	// 			activeCamera);
	// 	}
	// 	Renderer::EndPass();
	// }
}

void EditorRenderPipeline::OnResize(u32 w, u32 h) {
	// m_OutputBuffer->Resize(w, h);
}

}