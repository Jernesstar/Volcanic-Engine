#include "SceneRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Window/Input.h>
#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Graphics/Renderer3D.h>
#include <Engine/Graphics/StereographicCamera.h>
#include <Engine/Graphics/Platform/RendererAPI.h>

#include "App/Editor.h"
#include "Asset/AssetManager.h"
#include "Asset/AssetImporter.h"

namespace VolcanicEditor {

EditorSceneRenderer::EditorSceneRenderer() {
	Application::PushDir();

	auto camera = CreateRef<StereographicCamera>(75.0f);
	camera->SetPosition({ 0.0f, 1.0f, 15.0f });
	camera->Resize(1920, 1080);
	camera->SetProjection(0.001f, 10'000.0f);
	m_Controller.SetCamera(camera);
	m_Controller.TranslationSpeed = 25.0f;

	auto window = Application::GetWindow();
	m_Output =
		RendererAPI::Get()->CreateFramebuffer({
			{
				{ AttachmentTarget::Color, window->GetWidth(), window->GetHeight() }
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

	MeshPass =
		RenderPass::Create("Mesh",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mesh.glsl.vert",
				"Editor/assets/Shaders/Mesh.glsl.frag"
			}), m_Output);
	MeshPass->SetData(Renderer3D::GetMeshBuffer());

	MaskPass =
		RenderPass::Create("Mask",
			AssetImporter::GetShader({
				"Editor/assets/Shaders/Mask.glsl.vert",
				"Editor/assets/Shaders/Mask.glsl.frag"
			}),
			RendererAPI::Get()->CreateFramebuffer({
				{
					{ AttachmentTarget::Color, window->GetWidth(), window->GetHeight() }
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

EditorSceneRenderer::~EditorSceneRenderer() {

}

void EditorSceneRenderer::Update(TimeStep ts) {
	if(Hovered)
		m_Controller.OnUpdate(ts);
}

// void EditorSceneRenderer::SetContext(SceneVisualizerPanel* panel) {
// 	RootPanel = panel;
// 	Renderer3D::End();
// 	Renderer2D::End();
// 	Renderer3D::GetMeshBuffer()->Clear();
// 	// Renderer3D::GetCubemapBuffer()->Clear();
// 	Renderer3D::GetLineBuffer()->Clear();
// }

void EditorSceneRenderer::AddBillboard(const glm::vec3& pos, uint32_t type) {
	glm::vec3 cameraPos = m_Controller.GetCamera()->GetPosition();
	Billboards.Add({ pos, type });

	// float distance = glm::distance(cameraPos, pos);

	// // Put them in the list farthest to closest
	// auto [found, i] =
	// 	Billboards.FindLast( // The minimal distance pair with greater distance
	// 		[=](const std::pair<glm::vec3, uint32_t>& pair) -> bool
	// 		{
	// 			return distance < glm::distance(pair.first, cameraPos);
	// 		});

	// if(!found) // Farthest thing
	// 	Billboards.Insert(0, { pos, type });
	// else
	// 	Billboards.Insert(i + 1, { pos, type });
}

void EditorSceneRenderer::Begin() {
	auto camera = m_Controller.GetCamera();

	{
		MeshCommand = RendererAPI::Get()->NewCommand(MeshPass->Get());
		MeshCommand->Clear = true;
		MeshCommand->Uniforms
		.Set("u_ViewProj", camera->GetViewProjection())
		.Set("u_CameraPosition", camera->GetPosition());
	}

	BillboardBuffer->Clear();
	Billboards.Clear();

	glm::vec3 pos = camera->GetPosition();
	glm::vec3 dir = camera->GetDirection();
	glm::vec3 planePos = glm::vec3(0.0f);
	glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
	// float t;
	// if(glm::intersectRayPlane(pos, dir, planePos, planeNormal, t))
	// 	AddBillboard(pos + t * dir, 0);
		AddBillboard(glm::vec3(0.0f), 0);

	LineCommand = RendererAPI::Get()->NewCommand(LinePass->Get());
	LineCommand->DepthTesting = DepthTestingMode::On;
	LineCommand->Blending = BlendingMode::Greatest;
	LineCommand->Culling = CullingMode::Off;
	LineCommand->Uniforms
	.Set("u_ViewProj", m_Controller.GetCamera()->GetViewProjection());
}

void EditorSceneRenderer::SubmitCamera(const Entity& entity) {
	auto camera = entity.Get<CameraComponent>().Cam;
	if(!camera)
		return;

	HasCamera = true;
	AddBillboard(camera->GetPosition(), 1);

	if(Selected != entity)
		return;

	glm::mat4 inverse = glm::inverse(camera->GetViewProjection());

	glm::vec4 p0 = inverse * glm::vec4(-1, -1, -1, 1); // Front bottom left
	glm::vec4 p1 = inverse * glm::vec4( 1, -1, -1, 1); // Front bottom right
	glm::vec4 p2 = inverse * glm::vec4( 1,  1, -1, 1); // Front top right
	glm::vec4 p3 = inverse * glm::vec4(-1,  1, -1, 1); // Front top left
	glm::vec4 p4 = inverse * glm::vec4(-1, -1,  1, 1); // Back bottom left
	glm::vec4 p5 = inverse * glm::vec4( 1, -1,  1, 1); // Back bottom right
	glm::vec4 p6 = inverse * glm::vec4( 1,  1,  1, 1); // Back top right
	glm::vec4 p7 = inverse * glm::vec4(-1,  1,  1, 1); // Back top left

	Point points[] =
	{
		{ p0 / p0.w, glm::vec3(1.0f) },
		{ p1 / p1.w, glm::vec3(1.0f) },
		{ p2 / p2.w, glm::vec3(1.0f) },
		{ p3 / p3.w, glm::vec3(1.0f) },
		{ p4 / p4.w, glm::vec3(1.0f) },
		{ p5 / p5.w, glm::vec3(1.0f) },
		{ p6 / p6.w, glm::vec3(1.0f) },
		{ p7 / p7.w, glm::vec3(1.0f) },
		{ camera->GetPosition(), glm::vec3(1.0f) }
	};

	constexpr uint32_t indexCount = 24;
	uint32_t indices[indexCount] =
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

void EditorSceneRenderer::SubmitSkybox(const Entity& entity) {
	auto& sc = entity.Get<SkyboxComponent>();
	auto* assetManager = AssetManager::Get();
	// auto cubemap = assetManager->Get<Cubemap>(sc.CubemapAsset);

	// MeshCommand->Uniforms
	// .Set("u_Skybox", CubemapSlot{ cubemap });
}

void EditorSceneRenderer::SubmitLight(const Entity& entity) {
	glm::vec3 position;

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

void EditorSceneRenderer::SubmitParticles(const Entity& entity) {
	glm::vec3 position = entity.Get<ParticleEmitterComponent>().Position;
	AddBillboard(position, 5);
}

void EditorSceneRenderer::SubmitMesh(const Entity& entity) {
	auto* assetManager = AssetManager::Get();
	auto& tc = entity.Get<TransformComponent>();
	auto& mc = entity.Get<MeshComponent>();

	if(!mc.MeshSourceAsset)
		return;

	auto mesh = assetManager->Get<Mesh>(mc.MeshSourceAsset);

	if(!mc.MaterialAsset.ID) {
		Renderer::StartPass(MeshPass);
		{
			Renderer3D::DrawMesh(mesh, tc);
		}
		Renderer::EndPass();
		return;
	}

	if(!mc.MaterialAsset)
		return;

	auto material = assetManager->Get<DrawUniforms>(mc.MaterialAsset);

	DrawCommand* command = RendererAPI::Get()->NewCommand(MeshPass->Get());
	command->Uniforms = *material;

	Renderer3D::DrawMesh(mesh, tc, command);
}

void EditorSceneRenderer::Render() {
	Renderer3D::End();

	auto* assetManager = AssetManager::Get();
	if(Selected
	&& Selected.Has<TransformComponent>()
	&& Selected.Has<MeshComponent>()
	&& Selected.Get<MeshComponent>().MeshSourceAsset)
	{
		auto& tc = Selected.Get<TransformComponent>();
		auto& mc = Selected.Get<MeshComponent>();

		auto mesh = assetManager->Get<Mesh>(mc.MeshSourceAsset);
		{
			auto* command = RendererAPI::Get()->NewCommand(MaskPass->Get());
			command->Clear = true;
			command->Uniforms
			.Set("u_ViewProj",
				MeshCommand->Uniforms.Mat4Uniforms["u_ViewProj"]);
			command->Uniforms
			.Set("u_Color", glm::vec4(1.0f));

			Renderer3D::DrawMesh(mesh, tc, command);
		}

		Renderer::StartPass(OutlinePass);
		{
			auto att = m_Output->Get(AttachmentTarget::Color, 0);
			auto width = att->Spec.Width;
			auto height = att->Spec.Height;

			auto* command = Renderer::GetCommand();
			command->Uniforms
			.Set("u_PixelSize", 1.0f / glm::vec2(width, height));
			command->Uniforms
			.Set("u_Color", glm::vec3(0.0f, 0.0f, 1.0f));

			auto mask = MaskPass->GetOutput();
			Renderer2D::DrawFullscreenQuad(mask, AttachmentTarget::Color);
		}
		Renderer::EndPass();
	}

	Renderer3D::End();

	auto camera = m_Controller.GetCamera();
	for(auto [pos, type] : Billboards) {
		DrawCommand* command;
		if(type == 0) {
			command = RendererAPI::Get()->NewCommand(GridPass->Get());
			command->Uniforms
			.Set("u_CameraPosition", camera->GetPosition());
		}
		else {
			command = RendererAPI::Get()->NewCommand(BillboardPass->Get());
			command->Uniforms
			.Set("u_View", camera->GetView())
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
		.Set("u_ViewProj", camera->GetViewProjection());

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

// #ifdef MAGMA_PHYSICS
#if false
	auto* scene = RootPanel->GetPhysicsWorld().Get();
	const PxRenderBuffer& rb = scene->getRenderBuffer();
	if(rb.getNbLines()) {
		List<Point> points(rb.getNbLines() * 2);
		List<uint32_t> indices(rb.getNbLines() * 2);
	
		for(uint32_t i = 0; i < rb.getNbLines(); i++) {
			const PxDebugLine& l = rb.getLines()[i];
			Point p0 = { { l.pos0.x, l.pos0.y, l.pos0.z }, glm::vec3(1.0f) };
			Point p1 = { { l.pos1.x, l.pos1.y, l.pos1.z }, glm::vec3(1.0f) };
			points.Add(p0);
			points.Add(p1);
			indices.Add(2*i);
			indices.Add(2*i + 1);
		}

		auto* buffer = LinePass->Get()->BufferData;
		RendererAPI::Get()
		->SetBufferData(buffer, DrawBufferIndex::Indices,
			indices.GetBuffer().Get(), indices.Count(), 24);
		RendererAPI::Get()
		->SetBufferData(buffer, DrawBufferIndex::Vertices,
			points.GetBuffer().Get(), points.Count(), 9);

		auto& call = LineCommand->NewDrawCall();
		call.IndexStart = 24;
		call.IndexCount = indices.Count();
		call.VertexStart = 9;
		call.VertexCount = points.Count();
		call.Partition = PartitionType::Single;
		call.Primitive = PrimitiveType::Line;
	}
#endif

	Renderer::Flush();

	HasCamera = false;
	HasDirectionalLight = false;
	PointLightCount = 0;
	SpotlightCount = 0;
}

}