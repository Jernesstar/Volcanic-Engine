#include "EditorRenderPipeline.h"

#include <VolcaniCore/Core/Application.h>

#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Graphics/Renderer3D.h>
#include <Engine/Graphics/Platform/RendererAPI.h>
#include <Engine/Asset/AssetManager.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Component.h>

#include "AssetImporter.h"

using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

static constexpr u32 DEFAULT_W = 1920;
static constexpr u32 DEFAULT_H = 1080;

void EditorRenderPipeline::OnInit() {
	Application::PushDir();

	m_Output =
		RendererAPI::Get()->CreateFramebuffer({
			.Attachments = {
				{ AttachmentTarget::Color,   DEFAULT_W, DEFAULT_H },
				{ AttachmentTarget::Depth,   DEFAULT_W, DEFAULT_H },
			}
		});

	// ── Grid ──────────────────────────────────────────────────────────────
	m_GridPass =
		RenderPass::Create("Grid",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Grid.glsl.vert",
				"Editor/assets/Shaders/Grid.glsl.frag"
			}), m_Output);
	m_GridPass->SetData(Renderer2D::GetScreenBuffer());

	// ── Skybox ────────────────────────────────────────────────────────────
	// m_SkyboxPass =
	// 	RenderPass::Create("Skybox",
	// 		AssetImporter::GetShader({
	// 			"Editor/assets/Shaders/Skybox.glsl.vert",
	// 			"Editor/assets/Shaders/Skybox.glsl.frag"
	// 		}), m_Output);
	// m_SkyboxPass->SetData(Renderer3D::GetCubemapBuffer());

	// ── Geometry ──────────────────────────────────────────────────────────
	m_GeometryPass =
		RenderPass::Create("Geometry",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mesh.glsl.vert",
				"Editor/assets/Shaders/Mesh.glsl.frag"
			}), m_Output);
	m_GeometryPass->SetData(Renderer3D::GetMeshBuffer());

	// ── Mask + Outline ────────────────────────────────────────────────────
	m_MaskPass =
		RenderPass::Create("Mask",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mask.glsl.vert",
				"Editor/assets/Shaders/Mask.glsl.frag"
			}),
			RendererAPI::Get()->CreateFramebuffer({
				.Attachments = {
					{ AttachmentTarget::Color, DEFAULT_W, DEFAULT_H }
				}
			})
		);
	m_MaskPass->SetData(Renderer3D::GetMeshBuffer());

	m_OutlinePass =
		RenderPass::Create("Outline",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Outline.glsl.vert",
				"Editor/assets/Shaders/Outline.glsl.frag"
			}), m_Output);
	m_OutlinePass->SetData(Renderer2D::GetScreenBuffer());

	// ── Lines ─────────────────────────────────────────────────────────────
	m_LinePass =
		RenderPass::Create("Line",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Line.glsl.vert",
				"Editor/assets/Shaders/Line.glsl.frag"
			}), m_Output);
	m_LinePass->SetData(Renderer3D::GetLineBuffer());

	// ── Billboards ────────────────────────────────────────────────────────
	BufferLayout instanceLayout =
	{
		{
			{ "Position", BufferDataType::Vec3 }
		},
		true // Instanced
	};

	DrawBufferSpec billboardSpec =
	{
		.InstanceCount = 152,
		.VertexLayout = { },
		.InstanceLayout = instanceLayout,
	};
	m_BillboardBuffer = RendererAPI::Get()->NewBuffer(billboardSpec);

	m_BillboardPass =
		RenderPass::Create("Billboard",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Billboard.glsl.vert",
				"Editor/assets/Shaders/Billboard.glsl.frag"
			}), m_Output);
	m_BillboardPass->SetData(m_BillboardBuffer);

	m_CameraIcon           = AssetImporter::GetTexture("Editor/assets/Icons/CameraIcon.png");
	m_DirectionalLightIcon = AssetImporter::GetTexture("Editor/assets/Icons/DirectionalLightIcon.png");
	m_PointLightIcon       = AssetImporter::GetTexture("Editor/assets/Icons/PointLightIcon.png");
	m_SpotlightIcon        = AssetImporter::GetTexture("Editor/assets/Icons/SpotlightIcon.png");
	m_ParticlesIcon        = AssetImporter::GetTexture("Editor/assets/Icons/ParticlesIcon.png");

	Application::PopDir();
}

void EditorRenderPipeline::OnRender(Scene* scene) {
	if(Render3D) {
		Begin3D();

		scene->World3D.ForEach<SkyboxComponent>(
			[&](Entity& entity) { SubmitSkybox(entity); });

		scene->World3D.ForEach<CameraComponent>(
			[&](Entity& entity) { SubmitCamera3D(entity); });

		scene->World3D.ForEach<DirectionalLightComponent>(
			[&](Entity& entity) { SubmitLight3D(entity); });

		scene->World3D.ForEach<PointLightComponent>(
			[&](Entity& entity) { SubmitLight3D(entity); });

		scene->World3D.ForEach<SpotlightComponent>(
			[&](Entity& entity) { SubmitLight3D(entity); });

		scene->World3D.ForEach<ParticleEmitterComponent>(
			[&](Entity& entity) { SubmitParticles(entity); });

		scene->World3D.ForEach<MeshComponent, TransformComponent>(
			[&](Entity& entity) { SubmitGeometry(entity); });

		End3D();
	}

	if(Render2D) {
		Begin2D();
		End2D();
	}

	if(RenderCanvas) {
		BeginCanvas();
		EndCanvas();
	}
}

void EditorRenderPipeline::OnResize(u32 w, u32 h) {
	// m_Output->Resize(w, h);
}

// ── Internal helpers ──────────────────────────────────────────────────────────

void EditorRenderPipeline::AddBillboard(Vec3 pos, u32 type) {
	m_Billboards.Add({ pos, type });
}

// ── 3D ────────────────────────────────────────────────────────────────────────

void EditorRenderPipeline::Begin3D() {
	m_GeometryCommand = RendererAPI::Get()->NewCommand(m_GeometryPass->Get());
	m_GeometryCommand->Clear = true;
	m_GeometryCommand->Uniforms
	.Set("u_ViewProj", m_Camera->GetViewProjection())
	.Set("u_CameraPosition", m_Camera->GetPosition());

	m_BillboardBuffer->Clear();
	m_Billboards.Clear();

	// Always add a grid billboard at origin
	AddBillboard(Vec3(0.0f), 0);

	m_LineCommand = RendererAPI::Get()->NewCommand(m_LinePass->Get());
	m_LineCommand->DepthTesting = DepthTestingMode::On;
	m_LineCommand->Blending = BlendingMode::Greatest;
	m_LineCommand->Culling = CullingMode::Off;
	m_LineCommand->Uniforms
	.Set("u_ViewProj", m_Camera->GetViewProjection());
}

void EditorRenderPipeline::SubmitCamera3D(const Entity& entity) {
	auto camera = entity.Get<CameraComponent>().Cam;
	if(!camera)
		return;

	m_HasCamera = true;
	AddBillboard(camera->GetPosition(), 1);

	if(m_Selected != entity)
		return;

	// Draw camera frustum wireframe via line pass
	Mat4 inverse = glm::inverse(camera->GetViewProjection());

	Vec4 p0 = inverse * Vec4(-1, -1, -1, 1);
	Vec4 p1 = inverse * Vec4( 1, -1, -1, 1);
	Vec4 p2 = inverse * Vec4( 1,  1, -1, 1);
	Vec4 p3 = inverse * Vec4(-1,  1, -1, 1);
	Vec4 p4 = inverse * Vec4(-1, -1,  1, 1);
	Vec4 p5 = inverse * Vec4( 1, -1,  1, 1);
	Vec4 p6 = inverse * Vec4( 1,  1,  1, 1);
	Vec4 p7 = inverse * Vec4(-1,  1,  1, 1);

	struct Point { Vec3 Position; Vec3 Color; };
	Point points[] =
	{
		{ p0 / p0.w, Vec3(1.0f) },
		{ p1 / p1.w, Vec3(1.0f) },
		{ p2 / p2.w, Vec3(1.0f) },
		{ p3 / p3.w, Vec3(1.0f) },
		{ p4 / p4.w, Vec3(1.0f) },
		{ p5 / p5.w, Vec3(1.0f) },
		{ p6 / p6.w, Vec3(1.0f) },
		{ p7 / p7.w, Vec3(1.0f) },
		{ camera->GetPosition(), Vec3(1.0f) }
	};

	constexpr u32 indexCount = 24;
	u32 indices[indexCount] =
	{
		8, 7,  8, 6,  8, 4,  8, 5,  // Eye -> corners
		3, 2,  0, 1,  3, 0,  2, 1,  // Near plane
		7, 6,  4, 5,  7, 4,  6, 5,  // Far plane
	};

	auto* buf = m_LinePass->Get()->Buffer;
	buf->Add(DrawBufferIndex::E_Vertex, points, 9);
	buf->Add(DrawBufferIndex::E_Index, indices, indexCount);

	auto* call = m_LineCommand->NewCall();
	call->VertexCount = 9;
	call->IndexCount  = indexCount;
	call->Partition   = DrawPartition::Single;
	call->Primitive   = DrawPrimitive::Line;
}

void EditorRenderPipeline::SubmitSkybox(const Entity& entity) {
	auto& sc = entity.Get<SkyboxComponent>();
	auto* mgr = AssetManager::Get();
	mgr->Load(sc.CubemapAsset);
	auto cubemap = mgr->Get<Cubemap>(sc.CubemapAsset);
	if(!cubemap)
		return;

	auto* command = RendererAPI::Get()->NewCommand(m_SkyboxPass->Get());
	command->DepthTesting = DepthTestingMode::Off;
	command->Culling      = CullingMode::Front;
	command->Uniforms
	.Set("u_ViewProj", m_Camera->GetViewProjection())
	.Set("u_Skybox", CubemapSlot{ cubemap, 0 });

	auto* call = command->NewCall();
	call->VertexCount = 36;
	call->Partition   = DrawPartition::Single;
	call->Primitive   = DrawPrimitive::Cubemap;
}

void EditorRenderPipeline::SubmitLight3D(const Entity& entity) {
	Vec3 position;

	if(entity.Has<DirectionalLightComponent>()) {
		position = entity.Get<DirectionalLightComponent>().Position;
		m_HasDirectionalLight = true;
		AddBillboard(position, 2);
	}
	else if(entity.Has<PointLightComponent>()) {
		position = entity.Get<PointLightComponent>().Position;
		m_PointLightCount++;
		AddBillboard(position, 3);
	}
	else if(entity.Has<SpotlightComponent>()) {
		position = entity.Get<SpotlightComponent>().Position;
		m_SpotlightCount++;
		AddBillboard(position, 4);
	}
}

void EditorRenderPipeline::SubmitParticles(const Entity& entity) {
	Vec3 position = entity.Get<ParticleEmitterComponent>().Position;
	AddBillboard(position, 5);
}

void EditorRenderPipeline::SubmitGeometry(const Entity& entity) {
	auto* mgr = AssetManager::Get();
	auto& mc = entity.Get<MeshComponent>();
	auto& tc = entity.Get<TransformComponent>();

	if(!mc.GeometryAsset)
		return;

	mgr->Load(mc.GeometryAsset);
	auto geometry = mgr->Get<Geometry>(mc.GeometryAsset);
	if(!geometry)
		return;

	// Build a per-entity draw command so material uniforms differ per mesh
	auto* command = RendererAPI::Get()->NewCommand(m_GeometryPass->Get());
	command->Uniforms = m_GeometryCommand->Uniforms; // inherit camera

	Asset matAsset = mc.GetMaterialForSlot(0);
	if(matAsset) {
		mgr->Load(matAsset);
		auto mat = mgr->Get<Material>(matAsset);
		if(mat) {
			// Resolve textures and upload through MaterialBinder
			MaterialBinder::Bind(command, *mat);
		}
	}

	Transform t = tc;
	Renderer3D::DrawGeometry(geometry, t.GetTransform(), command);
}

void EditorRenderPipeline::End3D() {
	Renderer3D::End();

	// Selection outline
	if(m_Selected
	&& m_Selected.Has<TransformComponent>()
	&& m_Selected.Has<MeshComponent>()
	&& m_Selected.Get<MeshComponent>().GeometryAsset)
	{
		auto& mc = m_Selected.Get<MeshComponent>();
		auto& tc = m_Selected.Get<TransformComponent>();
		auto* mgr = AssetManager::Get();

		mgr->Load(mc.GeometryAsset);
		auto geometry = mgr->Get<Geometry>(mc.GeometryAsset);

		if(geometry) {
			{
				auto* cmd = RendererAPI::Get()->NewCommand(m_MaskPass->Get());
				cmd->Clear = true;
				cmd->Uniforms
				.Set("u_ViewProj", m_Camera->GetViewProjection())
				.Set("u_Color", Vec4(1.0f));

				Transform t = tc;
				Renderer3D::DrawGeometry(geometry, t.GetTransform(), cmd);
				Renderer3D::End();
			}

			Renderer::StartPass(m_OutlinePass);
			{
				auto att = m_Output->Get(AttachmentTarget::Color);
				auto width = att->Spec.Width;
				auto height = att->Spec.Height;

				auto* cmd = Renderer::GetCommand();
				cmd->Uniforms
				.Set("u_PixelSize", 1.0f / Vec2(width, height))
				.Set("u_Color", Vec3(0.0f, 0.4f, 1.0f));

				Renderer2D::DrawFullscreenQuad(
					m_MaskPass->GetOutput(), AttachmentTarget::Color);
			}
			Renderer::EndPass();
		}
	}

	Renderer3D::End();

	// Flush billboards and grid
	for(auto& [pos, type] : m_Billboards) {
		DrawCommand* command;

		if(type == 0) {
			command = RendererAPI::Get()->NewCommand(m_GridPass->Get());
			command->Uniforms
			.Set("u_CameraPosition", m_Camera->GetPosition());
		}
		else {
			command = RendererAPI::Get()->NewCommand(m_BillboardPass->Get());
			command->Uniforms
			.Set("u_View", m_Camera->GetView())
			.Set("u_BillboardWidth",  1.0f)
			.Set("u_BillboardHeight", 1.0f);

			Ref<Texture> icon;
			if(type == 1)      icon = m_CameraIcon;
			else if(type == 2) icon = m_DirectionalLightIcon;
			else if(type == 3) icon = m_PointLightIcon;
			else if(type == 4) icon = m_SpotlightIcon;
			else if(type == 5) icon = m_ParticlesIcon;

			command->Uniforms
			.Set("u_Texture", TextureSlot{ icon, 0 });
		}

		command->DepthTesting = DepthTestingMode::On;
		command->Blending = BlendingMode::Greatest;
		command->Culling = CullingMode::Off;
		command->Uniforms
		.Set("u_ViewProj", m_Camera->GetViewProjection());

		auto* call = command->NewCall();
		call->VertexCount = 6;
		call->Primitive = DrawPrimitive::Triangle;
		call->Partition = DrawPartition::Single;

		if(type != 0) {
			call->Partition = DrawPartition::Instanced;
			call->InstanceOffset = m_BillboardBuffer->GetInstanceCount();
			call->InstanceCount = 1;
			m_BillboardBuffer->Add(DrawBufferIndex::E_Instance,
				glm::value_ptr(pos), 1);
		}
	}

	m_HasCamera = false;
	m_HasDirectionalLight = false;
	m_PointLightCount = 0;
	m_SpotlightCount = 0;
	m_ParticleEmitterCount = 0;

	Renderer::Flush();
}

// ── 2D ────────────────────────────────────────────────────────────────────────

void EditorRenderPipeline::Begin2D() { }
void EditorRenderPipeline::End2D()   { }

// ── Canvas ────────────────────────────────────────────────────────────────────

void EditorRenderPipeline::BeginCanvas() { }
void EditorRenderPipeline::EndCanvas()   { }

}