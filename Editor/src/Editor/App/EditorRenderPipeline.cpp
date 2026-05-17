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
				{ AttachmentTarget::Color, DEFAULT_W, DEFAULT_H }
			}
		});

	GridPass =
		RenderPass::Create("Grid",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Grid.glsl.vert",
				"Editor/assets/Shaders/Grid.glsl.frag"
			}), m_Output);
	GridPass->SetData(Renderer2D::GetScreenBuffer());

	BufferLayout instanceLayout =
	{
		{
			{ "Position", BufferDataType::Vec3 }
		},
		true // Structure of arrays, aka. Instanced
	};

	DrawBufferSpec specs
	{
		.InstanceCount = 152,
		.VertexLayout = { },
		.InstanceLayout = instanceLayout,
	};
	BillboardBuffer = RendererAPI::Get()->NewBuffer(specs);

	BillboardPass =
		RenderPass::Create("Billboard",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Billboard.glsl.vert",
				"Editor/assets/Shaders/Billboard.glsl.frag"
			}), m_Output);
	BillboardPass->SetData(BillboardBuffer);

	CameraIcon =
		AssetImporter::GetTexture("Editor/assets/Icons/CameraIcon.png");
	DirectionalLightIcon =
		AssetImporter::GetTexture("Editor/assets/Icons/DirectionalLightIcon.png");
	PointLightIcon =
		AssetImporter::GetTexture("Editor/assets/Icons/PointLightIcon.png");
	SpotlightIcon =
		AssetImporter::GetTexture("Editor/assets/Icons/SpotlightIcon.png");
	ParticlesIcon =
		AssetImporter::GetTexture("Editor/assets/Icons/ParticlesIcon.png");

	GeometryPass =
		RenderPass::Create("Geometry",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mesh.glsl.vert",
				"Editor/assets/Shaders/Mesh.glsl.frag"
			}), m_Output);
	GeometryPass->SetData(Renderer3D::GetMeshBuffer());

	MaskPass =
		RenderPass::Create("Mask",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mask.glsl.vert",
				"Editor/assets/Shaders/Mask.glsl.frag"
			}),
			RendererAPI::Get()->CreateFramebuffer({
				{
					{ AttachmentTarget::Color, DEFAULT_W, DEFAULT_H }
				}
			})
		);
	MaskPass->SetData(Renderer3D::GetMeshBuffer());

	OutlinePass =
		RenderPass::Create("Outline",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Outline.glsl.vert",
				"Editor/assets/Shaders/Outline.glsl.frag"
			}), m_Output);
	OutlinePass->SetData(Renderer2D::GetScreenBuffer());

	LinePass =
		RenderPass::Create("Line",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Line.glsl.vert",
				"Editor/assets/Shaders/Line.glsl.frag"
			}), m_Output);
	LinePass->SetData(Renderer3D::GetLineBuffer());

	Application::PopDir();
}

void EditorRenderPipeline::OnRender(Scene* scene) {
	if(Render3D) {
		Begin3D();

		scene->World3D.
			ForEach<CameraComponent>(
				[&](Entity& entity)
				{
					SubmitCamera3D(entity);
				}
			);
		scene->World3D.
			ForEach<SkyboxComponent>(
				[&](Entity& entity)
				{
					SubmitSkybox(entity);
				}
			);

		scene->World3D.
			ForEach<DirectionalLightComponent>(
				[&](Entity& entity)
				{
					SubmitLight3D(entity);
				}
			);
		scene->World3D.
			ForEach<PointLightComponent>(
				[&](Entity& entity)
				{
					SubmitLight3D(entity);
				}
			);
		scene->World3D.
			ForEach<SpotlightComponent>(
				[&](Entity& entity)
				{
					SubmitLight3D(entity);
				}
			);

		scene->World3D.
			ForEach<ParticleEmitterComponent>(
				[&](Entity& entity)
				{
					SubmitParticles(entity);
				}
			);

		scene->World3D.
			ForEach<MeshComponent, TransformComponent>(
				[&](Entity& entity)
				{
					SubmitGeometry(entity);
				}
			);

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

void EditorRenderPipeline::Begin3D() {
	GeometryCommand = RendererAPI::Get()->NewCommand(GeometryPass->Get());
	GeometryCommand->Clear = true;
	GeometryCommand->ClearColor = { 1.0f, 1.0f, 0.0f, 1.0f };
	GeometryCommand->Uniforms
	.Set("u_ViewProj", m_Camera->GetViewProjection())
	.Set("u_CameraPosition", m_Camera->GetPosition());

	BillboardBuffer->Clear();
	Billboards.Clear();

	Vec3 pos = m_Camera->GetPosition();
	Vec3 dir = m_Camera->GetDirection();
	Vec3 planePos = Vec3(0.0f);
	Vec3 planeNormal = Vec3(0.0f, 1.0f, 0.0f);
	// float t;
	// if(glm::intersectRayPlane(pos, dir, planePos, planeNormal, t))
	// 	AddBillboard(pos + t * dir, 0);
		AddBillboard(Vec3(0.0f), 0);

	LineCommand = RendererAPI::Get()->NewCommand(LinePass->Get());
	LineCommand->DepthTesting = DepthTestingMode::On;
	LineCommand->Blending = BlendingMode::Greatest;
	LineCommand->Culling = CullingMode::Off;
	LineCommand->Uniforms
	.Set("u_ViewProj", m_Camera->GetViewProjection());
}

void EditorRenderPipeline::SubmitCamera3D(const Entity& entity) {
	auto camera = entity.Get<CameraComponent>().Cam;
	if(!camera)
		return;

	HasCamera = true;
	AddBillboard(camera->GetPosition(), 1);

	if(Selected != entity)
		return;

	Mat4 inverse = glm::inverse(camera->GetViewProjection());

	Vec4 p0 = inverse * Vec4(-1, -1, -1, 1); // Front bottom left
	Vec4 p1 = inverse * Vec4( 1, -1, -1, 1); // Front bottom right
	Vec4 p2 = inverse * Vec4( 1,  1, -1, 1); // Front top right
	Vec4 p3 = inverse * Vec4(-1,  1, -1, 1); // Front top left
	Vec4 p4 = inverse * Vec4(-1, -1,  1, 1); // Back bottom left
	Vec4 p5 = inverse * Vec4( 1, -1,  1, 1); // Back bottom right
	Vec4 p6 = inverse * Vec4( 1,  1,  1, 1); // Back top right
	Vec4 p7 = inverse * Vec4(-1,  1,  1, 1); // Back top left

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
		8, 7, // Left Top
		8, 6, // Right Top
		8, 4, // Left Bottom
		8, 5, // Right Bottom

		3, 2, // Front Top
		0, 1, // Front Bottom
		3, 0, // Front Left
		2, 1, // Front Right

		7, 6, // Back Top
		4, 5, // Back Bottom
		7, 4, // Back Left
		6, 5, // Back Right
	};

	auto* buffer = LinePass->Get()->Buffer;
	buffer->Add(DrawBufferIndex::E_Vertex, points, 9);
	buffer->Add(DrawBufferIndex::E_Index, indices, indexCount);

	auto* call = LineCommand->NewCall();
	call->VertexCount = 9;
	call->IndexCount = indexCount;
	call->Partition = DrawPartition::Single;
	call->Primitive = DrawPrimitive::Line;
}

void EditorRenderPipeline::SubmitSkybox(const Entity& entity) {

}

void EditorRenderPipeline::SubmitLight3D(const Entity& entity) {
	Vec3 position;

	if(entity.Has<DirectionalLightComponent>()) {
		position = entity.Get<DirectionalLightComponent>().Position;
		HasDirectionalLight = true;
		AddBillboard(position, 2);
	}
	else if(entity.Has<PointLightComponent>()) {
		position = entity.Get<PointLightComponent>().Position;
		PointLightCount++;
		AddBillboard(position, 3);
	}
	else if(entity.Has<SpotlightComponent>()) {
		position = entity.Get<SpotlightComponent>().Position;
		SpotlightCount++;
		AddBillboard(position, 4);
	}
}

void EditorRenderPipeline::SubmitParticles(const Entity& entity) {
	Vec3 position = entity.Get<ParticleEmitterComponent>().Position;
	AddBillboard(position, 5);
}

void EditorRenderPipeline::SubmitGeometry(const Entity& entity) {

}

void EditorRenderPipeline::AddBillboard(Vec3 pos, u32 type) {
	Vec3 cameraPos = m_Camera->GetPosition();
	Billboards.Add({ pos, type });
}

void EditorRenderPipeline::End3D() {
	Renderer3D::End();

	auto* assetManager = AssetManager::Get();
	// if(Selected
	// && Selected.Has<TransformComponent>()
	// && Selected.Has<MeshComponent>()
	// && Selected.Get<MeshComponent>().MeshSourceAsset)
	// {
	// 	auto& tc = Selected.Get<TransformComponent>();
	// 	auto& mc = Selected.Get<MeshComponent>();

	// 	auto mesh = assetManager->Get<Mesh>(mc.MeshSourceAsset);
	// 	{
	// 		auto* command = RendererAPI::Get()->NewCommand(MaskPass->Get());
	// 		command->Clear = true;
	// 		command->Uniforms
	// 		.Set("u_ViewProj",
	// 			GeometryCommand->Uniforms.Mat4Uniforms["u_ViewProj"]);
	// 		command->Uniforms
	// 		.Set("u_Color", Vec4(1.0f));

	// 		// Renderer3D::DrawMesh(mesh, tc, command);
	// 	}

	// 	Renderer::StartPass(OutlinePass);
	// 	{
	// 		auto att = m_Output->Get(AttachmentTarget::Color, 0);
	// 		auto width = att->Spec.Width;
	// 		auto height = att->Spec.Height;

	// 		auto* command = Renderer::GetCommand();
	// 		command->Uniforms
	// 		.Set("u_PixelSize", 1.0f / glm::vec2(width, height));
	// 		command->Uniforms
	// 		.Set("u_Color", Vec3(0.0f, 0.0f, 1.0f));

	// 		auto mask = MaskPass->GetOutput();
	// 		Renderer2D::DrawFullscreenQuad(mask, AttachmentTarget::Color);
	// 	}
	// 	Renderer::EndPass();
	// }

	// Renderer3D::End();

	for(auto [pos, type] : Billboards) {
		DrawCommand* command;
		if(type == 0) {
			command = RendererAPI::Get()->NewCommand(GridPass->Get());
			command->Uniforms
			.Set("u_CameraPosition", m_Camera->GetPosition());
		}
		else {
			command = RendererAPI::Get()->NewCommand(BillboardPass->Get());
			command->Uniforms
			.Set("u_View", m_Camera->GetView())
			.Set("u_BillboardWidth", 1.0f)
			.Set("u_BillboardHeight", 1.0f);

			Ref<Texture> icon;
			if(type == 1)
				icon = CameraIcon;
			else if(type == 2)
				icon = DirectionalLightIcon;
			else if(type == 3)
				icon = PointLightIcon;
			else if(type == 4)
				icon = SpotlightIcon;
			else if(type == 5)
				icon = ParticlesIcon;

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
		call->Partition = DrawPartition::Single;
		call->Primitive = DrawPrimitive::Triangle;

		if(type != 0) {
			call->Partition = DrawPartition::Instanced;
			call->InstanceOffset = BillboardBuffer->GetInstanceCount();
			call->InstanceCount = 1;
			BillboardBuffer->Add(DrawBufferIndex::E_Instance, glm::value_ptr(pos), 1);
		}
	}

	HasCamera = false;
	HasDirectionalLight = false;
	PointLightCount = 0;
	SpotlightCount = 0;
}

void EditorRenderPipeline::Begin2D() {

}

void EditorRenderPipeline::End2D() {
	
}

void EditorRenderPipeline::BeginCanvas() {

}

void EditorRenderPipeline::EndCanvas() {

}

}