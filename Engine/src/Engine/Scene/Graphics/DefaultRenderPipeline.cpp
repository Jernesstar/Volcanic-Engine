#include "DefaultRenderPipeline.h"
#include "ScriptPipelineContext.h"

#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer3D.h>
#include <Engine/Graphics/Renderer2D.h>

#include <Engine/Asset/AssetManager.h>
#include <Engine/Scene/Component.h>
#include <Engine/Scene/Scene.h>

using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Graphics;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

struct ParticleData {
	Vec3 Position;
	Vec3 Velocity;
	f32  Life;
};

static constexpr i32 k_EmitWorkGroup   = 64;
static constexpr i32 k_UpdateWorkGroup = 128;

void DefaultRenderPipeline::OnInit() {
	u32 w = m_Width, h = m_Height;

	m_GBuffer = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Color, w, h }, // Position  (location 0)
			{ AttachmentTarget::Color, w, h }, // Normal    (location 1)
			{ AttachmentTarget::Color, w, h }, // Albedo    (location 2)
			{ AttachmentTarget::Depth, w, h }, // Specular
		}
	});

	m_HDRBuffer = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Color, w, h },
			{ AttachmentTarget::Depth, w, h },
		}
	});

	m_ShadowMap = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Depth, 2048, 2048 },
		}
	});

	{
		FramebufferSpec bloomSpec;
		glm::ivec2 mipSize{ (i32)w, (i32)h };
		for(u32 i = 0; i < s_MipCount; i++) {
			mipSize /= 2;
			mipSize  = glm::max(mipSize, glm::ivec2(1));
			m_MipChain[i].Size = mipSize;
			bloomSpec.Attachments.Add({ AttachmentTarget::Color,
				(u32)mipSize.x, (u32)mipSize.y });
		}
		m_BloomMips = RendererAPI::CreateFramebuffer(bloomSpec);
	}

	m_OutputBuffer = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Color, w, h },
		}
	});

	auto* am = AssetManager::Get();
	m_GBufferShader    = am->Get<Shader>("GBuffer");
	m_ShadowShader     = am->Get<Shader>("ShadowDepth");
	m_LightingShader   = am->Get<Shader>("DeferredLighting");
	m_DownsampleShader = am->Get<Shader>("BloomDownsample");
	m_UpsampleShader   = am->Get<Shader>("BloomUpsample");
	m_TonemapShader    = am->Get<Shader>("Tonemap");
	// m_SkyboxShader     = am->Get<Shader>("Skybox");
	m_ParticleEmitShader   = am->Get<Shader>("ParticleEmitter");
	m_ParticleUpdateShader = am->Get<Shader>("ParticleUpdate");
	m_ParticleDrawShader   = am->Get<Shader>("Particle");

	m_GeometryPass = RenderPass::Create("GBuffer", m_GBufferShader, m_GBuffer);
	m_ShadowPass   = RenderPass::Create("Shadow", m_ShadowShader, m_ShadowMap);
	m_LightingPass = RenderPass::Create("Lighting", m_LightingShader, m_HDRBuffer);
	m_TonemapPass  = RenderPass::Create("Tonemap", m_TonemapShader);
	// m_SkyboxPass   = RenderPass::Create("Skybox", m_SkyboxShader, m_HDRBuffer);
	m_ParticleEmitPass   = RenderPass::Create("ParticleEmit",   m_ParticleEmitShader);
	m_ParticleUpdatePass = RenderPass::Create("ParticleUpdate", m_ParticleUpdateShader);
	m_ParticleDrawPass   = RenderPass::Create("ParticleDraw",   m_ParticleDrawShader,  m_HDRBuffer);

	m_GeometryPass->SetData(Renderer3D::GetMeshBuffer());
	m_ShadowPass->SetData(Renderer3D::GetMeshBuffer());
	m_LightingPass->SetData(Renderer2D::GetScreenBuffer());
	m_TonemapPass->SetData(Renderer2D::GetScreenBuffer());
	// m_SkyboxPass->SetData(Renderer3D::GetCubemapBuffer());
	m_ParticleDrawPass->SetData(Renderer2D::GetScreenBuffer());
}

// ── Resize ────────────────────────────────────────────────────────────────────

void DefaultRenderPipeline::OnResize(u32 w, u32 h) {
	// m_Width = w;
	// m_Height = h;
	// OnInit();
}

// ── GetOutput ─────────────────────────────────────────────────────────────────

Ref<Framebuffer> DefaultRenderPipeline::GetOutput() const {
	return nullptr;
}

Ref<Framebuffer> DefaultRenderPipeline::GetBuffer(const std::string& name) const {
	if(name == "GBuffer")   return m_GBuffer;
	if(name == "HDR")       return m_HDRBuffer;
	if(name == "ShadowMap") return m_ShadowMap;
	// if(name == "Bloom")     return m_BloomPingPong[1];
	return nullptr;
}

// ── Hook API ──────────────────────────────────────────────────────────────────

void DefaultRenderPipeline::AddHook(PipelineStage stage, asIScriptFunction* fn) {
	fn->AddRef();
	m_Hooks[stage].Add(fn);
}

void DefaultRenderPipeline::RemoveHook(PipelineStage stage, asIScriptFunction* fn) {
	auto& list = m_Hooks[stage];
	for(u64 i = 0; i < list.Count(); i++) {
		if(list[i] == fn) {
			list[i]->Release();
			list.Pop(i);
			return;
		}
	}
}

void DefaultRenderPipeline::ExecuteHooks(PipelineStage stage, ScriptPipelineContext* ctx) {
	for(auto* fn : m_Hooks[stage]) {
		ScriptFunc func{ fn, ScriptEngine::GetContext() };
		func.CallVoid(ctx);
	}

	// Apply any output redirection the hook requested
	if(ctx->HasRedirection()) {
		Ref<Framebuffer> target = ctx->GetRedirectedBuffer()->Resolve();
		if(stage == PipelineStage::PreGeometry)
			m_GeometryPass->SetOutput(target);
		if(stage == PipelineStage::PreShadows)
			m_ShadowPass->SetOutput(target);
	}

	if(ctx->HasSubPixelOffset())
		m_SubPixelOffset = ctx->GetSubPixelOffset();
}

void DefaultRenderPipeline::TickParticles(Scene* scene, TimeStep ts,
	Ref<Camera> cam)
{
	Renderer::StartPass(m_ParticleEmitPass);
	{
		scene->World3D.ForEach<ParticleEmitterComponent>(
			[&](Entity& entity) {
				auto& spec = entity.Get<ParticleEmitterComponent>();
				u64 id = entity.GetHandle().id();

				// Lazily allocate GPU buffers the first time we see this entity
				if(!m_ParticleState.count(id)) {
					ParticleEmitterGPU gpu;
					gpu.MaxParticleCount = spec.MaxParticleCount;

					BufferLayout particleLayout = {
						{ "Position", BufferDataType::Vec3 },
						{ "Velocity", BufferDataType::Vec3 },
						{ "Life",     BufferDataType::Float },
					};
					BufferLayout freeListLayout = {
						{ "Indices", BufferDataType::Int },
					};

					Buffer<ParticleData> particles(spec.MaxParticleCount);
					for(u64 i = 0; i < spec.MaxParticleCount; i++)
						particles.Set(i, ParticleData{ });

					// freelist[0] = count, freelist[1..N] = indices 0..N-1
					Buffer<i32> freeList(spec.MaxParticleCount + 1);
					freeList.Set(0, (i32)spec.MaxParticleCount);
					for(u64 i = 1; i <= spec.MaxParticleCount; i++)
						freeList.Set(i, (i32)(i - 1));

					gpu.ParticleBuffer = RendererAPI::CreateStorageBuffer({
						particleLayout, spec.MaxParticleCount });
					gpu.ParticleBuffer->SetData(particles);

					gpu.FreeListBuffer = RendererAPI::CreateStorageBuffer({
						freeListLayout, spec.MaxParticleCount + 1 });
					gpu.FreeListBuffer->SetData(freeList);

					m_ParticleState[id] = std::move(gpu);
				}

				auto& gpu = m_ParticleState[id];
				gpu.Timer += (f32)ts;

				u32 toSpawn = (u32)(gpu.Timer / spec.SpawnInterval);
				gpu.Timer   = glm::mod(gpu.Timer, spec.SpawnInterval);
				if(toSpawn == 0)
					return;

				u32 workGroups =
					(toSpawn + k_EmitWorkGroup - 1) / k_EmitWorkGroup;

				auto* cmd = Renderer::NewCommand();
				cmd->Compute  = true;
				cmd->ComputeX = workGroups;
				cmd->Uniforms
				.Set("u_TimeStep",        (f32)ts)
				.Set("u_ParticlesToSpawn",(i32)toSpawn)
				.Set("u_EmitterPosition", spec.Position)
				.Set("u_ParticleLifetime",spec.ParticleLifetime)
				.Set("u_Offset",          spec.Offset)
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 })
				.Set(StorageSlot{ gpu.FreeListBuffer,  "", 1 });
			});
	}
	Renderer::EndPass();

	// ── Update pass ───────────────────────────────────────────────────────────
	Renderer::StartPass(m_ParticleUpdatePass);
	{
		scene->World3D.ForEach<ParticleEmitterComponent>(
			[&](Entity& entity) {
				u64 id = entity.GetHandle().id();
				if(!m_ParticleState.count(id))
					return;

				auto& gpu = m_ParticleState[id];
				u32 workGroups =
					((u32)gpu.MaxParticleCount + k_UpdateWorkGroup - 1)
					/ k_UpdateWorkGroup;

				auto* cmd = Renderer::NewCommand();
				cmd->Compute  = true;
				cmd->ComputeX = workGroups;
				cmd->Uniforms
				.Set("u_TimeStep", (f32)ts)
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 })
				.Set(StorageSlot{ gpu.FreeListBuffer,  "", 1 });
			});
	}
	Renderer::EndPass();

	// ── Draw pass ─────────────────────────────────────────────────────────────
	// Billboarded quads rendered into the HDR buffer so emissive particles
	// feed into the bloom pass. Depth-tests against the scene depth that was
	// blit from m_GBuffer during the lighting pass setup.
	Renderer::StartPass(m_ParticleDrawPass);
	{
		auto* sharedCmd = Renderer::GetCommand();
		sharedCmd->Uniforms
		.Set("u_View", cam->GetView())
		.Set("u_ViewProj", cam->GetViewProjection())
		.Set("u_BillboardWidth", 0.1f)
		.Set("u_BillboardHeight", 0.1f);

		scene->World3D.ForEach<ParticleEmitterComponent>(
			[&](Entity& entity) {
				u64 id = entity.GetHandle().id();
				if(!m_ParticleState.count(id))
					return;

				auto& spec = entity.Get<ParticleEmitterComponent>();
				auto& gpu  = m_ParticleState[id];

				// Resolve the material texture if one is set
				Ref<Texture> tex;
				if(spec.MaterialAsset)
					tex = AssetManager::Get()->Get<Texture>(spec.MaterialAsset);

				auto* cmd = Renderer::NewCommand();
				cmd->DepthTesting = DepthTestingMode::On;
				cmd->Blending     = BlendingMode::Additive;
				cmd->Culling      = CullingMode::Off;

				if(tex)
					cmd->Uniforms.Set("u_Texture", TextureSlot{ tex, 0 });

				cmd->Uniforms
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 });

				auto* call = cmd->NewCall();
				call->VertexCount   = 6;
				call->InstanceCount = (u32)gpu.MaxParticleCount;
				call->Primitive     = DrawPrimitive::Triangle;
				call->Partition     = DrawPartition::Instanced;
			});
	}
	Renderer::EndPass();
}

void DefaultRenderPipeline::OnRender(Scene* scene) {
	auto ctx = Ref<ScriptPipelineContext>(
		ScriptPipelineContext::Factory(this, scene));

	// ── Collect scene data ────────────────────────────────────────────────────

	Ref<Camera> mainCamera;
	List<DirectionalLightComponent> dirLights;
	List<PointLightComponent> pointLights;
	Ref<Cubemap> skybox;

	scene->World3D.ForEach<CameraComponent>(
		[&](ECS::Entity& entity) {
			if(!mainCamera)
				mainCamera = entity.Get<CameraComponent>().Cam;
		});

	scene->World3D.ForEach<DirectionalLightComponent>(
		[&](ECS::Entity& entity) {
			dirLights.Add(entity.Get<DirectionalLightComponent>());
		});

	scene->World3D.ForEach<PointLightComponent>(
		[&](ECS::Entity& entity) {
			pointLights.Add(entity.Get<PointLightComponent>());
		});

	scene->World3D.ForEach<SkyboxComponent>(
		[&](ECS::Entity& entity) {
			auto asset = entity.Get<SkyboxComponent>().CubemapAsset;
			skybox = AssetManager::Get()->Get<Cubemap>(asset);
		});

	if(!mainCamera)
		return;

	// ── Shadow pass ───────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreShadows, ctx.get());

	// Build a simple orthographic light-space matrix from the first dir light
	Mat4 lightSpaceMatrix{ 1.0f };
	if(dirLights.Count()) {
		auto& dl = dirLights[0];
		auto lightCam = OrthographicCamera(40, 40, 1.0f, 100.0f);
		lightCam.SetPosition(dl.Position);
		lightCam.SetDirection(dl.Direction);
		lightSpaceMatrix = lightCam.GetViewProjection();
	}

	Renderer::StartPass(m_ShadowPass);
	{
		Renderer::Clear();

		auto* cmd = Renderer::GetCommand();
		cmd->Uniforms.Set("u_LightSpaceMatrix", lightSpaceMatrix);

		Renderer3D::Begin(mainCamera);
		scene->World3D.ForEach<MeshComponent, TransformComponent>(
			[&](ECS::Entity& entity) {
				auto& mesh = entity.Get<MeshComponent>();
				auto& tr = entity.Get<TransformComponent>();
				auto geo = AssetManager::Get()->Get<Geometry>(mesh.GeometryAsset);
				if(geo)
					Renderer3D::DrawGeometry(geo, Transform(tr).GetTransform());
			});
		Renderer3D::End();
	}
	Renderer::EndPass();

	ExecuteHooks(PipelineStage::PostShadows, ctx.get());

	// ── Depth prepass ────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreDepthPrepass, ctx.get());

	ExecuteHooks(PipelineStage::PostDepthPrepass, ctx.get());

	// ── Geometry pass (G-Buffer fill) ────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreGeometry, ctx.get());

	Renderer::StartPass(m_GeometryPass);
	{
		Renderer::Clear();

		auto* cmd = Renderer::GetCommand();
		cmd->Uniforms.Set("u_ViewProj", mainCamera->GetViewProjection());

		scene->World3D.ForEach<MeshComponent, TransformComponent>(
			[&](ECS::Entity& entity) {
				auto& mesh = entity.Get<MeshComponent>();
				auto& tr   = entity.Get<TransformComponent>();

				auto geo = AssetManager::Get()->Get<Geometry>(mesh.GeometryAsset);
				if(!geo) return;

				// Bind material for this draw command
				auto matAsset = mesh.GetMaterialForSlot(0);
				auto mat = AssetManager::Get()->Get<Material>(matAsset);
				if(mat)
					MaterialBinder::Bind(cmd, *mat);

				Renderer3D::DrawGeometry(geo, Transform(tr).GetTransform(), cmd);
			});
	}
	Renderer::EndPass();

	ExecuteHooks(PipelineStage::PostGeometry, ctx.get());

	// ── Skybox ────────────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreSkybox, ctx.get());

	if(skybox) {
		Renderer::StartPass(m_SkyboxPass);
		{
			auto* cmd = Renderer::GetCommand();
			// Remove translation from view so skybox stays centered
			Mat4 skyView = Mat4(Mat3(mainCamera->GetView()));
			cmd->Uniforms
			.Set("u_View", skyView)
			.Set("u_Projection", mainCamera->GetProjection())
			.Set("u_Skybox", CubemapSlot{ skybox, 0 });
			cmd->DepthTesting = DepthTestingMode::On;

			auto* call = cmd->NewCall();
			call->VertexCount = 36;
			call->Primitive = DrawPrimitive::Triangle;
			call->Partition = DrawPartition::Single;
		}
		Renderer::EndPass();
	}

	ExecuteHooks(PipelineStage::PostSkybox, ctx.get());

	// ── Lighting pass (deferred) ──────────────────────────────────────────────

	Renderer::StartPass(m_LightingPass);
	{
		Renderer::Clear();
		auto* cmd = Renderer::GetCommand();
		cmd->DepthTesting = DepthTestingMode::Off;
		cmd->Blending = BlendingMode::Off;

		// G-Buffer inputs
		cmd->Uniforms
		.Set("u_GPosition", AttachmentSlot{ m_GBuffer->Get(AttachmentTarget::Color, 0), 0 })
		.Set("u_GNormal", AttachmentSlot{ m_GBuffer->Get(AttachmentTarget::Color, 1), 1 })
		.Set("u_GAlbedo", AttachmentSlot{ m_GBuffer->Get(AttachmentTarget::Color, 2), 2 })
		.Set("u_ShadowMap", AttachmentSlot{ m_ShadowMap->Get(AttachmentTarget::Depth),  3 })
		.Set("u_LightSpaceMatrix", lightSpaceMatrix)
		.Set("u_CameraPos", mainCamera->GetPosition());

		// Directional lights (up to 4)
		for(u32 i = 0; i < dirLights.Count() && i < 4; i++) {
			auto& dl = dirLights[i];
			std::string s = "u_DirLights[" + std::to_string(i) + "]";
			cmd->Uniforms
			.Set(s + ".Direction", dl.Direction)
			.Set(s + ".Ambient",   dl.Ambient)
			.Set(s + ".Diffuse",   dl.Diffuse)
			.Set(s + ".Specular",  dl.Specular);
		}
		cmd->Uniforms.Set("u_DirLightCount", (i32)dirLights.Count());

		// Point lights (up to 16)
		for(u32 i = 0; i < pointLights.Count() && i < 16; i++) {
			auto& pl = pointLights[i];
			std::string s = "u_PointLights[" + std::to_string(i) + "]";
			cmd->Uniforms
			.Set(s + ".Position",  pl.Position)
			.Set(s + ".Ambient",   pl.Ambient)
			.Set(s + ".Diffuse",   pl.Diffuse)
			.Set(s + ".Specular",  pl.Specular)
			.Set(s + ".Constant",  pl.Constant)
			.Set(s + ".Linear",    pl.Linear)
			.Set(s + ".Quadratic", pl.Quadratic);
		}
		cmd->Uniforms.Set("u_PointLightCount", (i32)pointLights.Count());

		auto* call = cmd->NewCall();
		call->VertexCount = 6;
		call->Primitive   = DrawPrimitive::Triangle;
		call->Partition   = DrawPartition::Single;
	}
	Renderer::EndPass();

	// ── Transparency pass (forward, additive over HDR) ────────────────────────

	f32 ts = scene->World3D.GetNative().delta_time();
	ExecuteHooks(PipelineStage::PreTransparency, ctx.get());
	TickParticles(scene, ts, mainCamera);
	ExecuteHooks(PipelineStage::PostTransparency, ctx.get());

	// ── Bloom (compute) ───────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PrePostProcess, ctx.get());
	RunBloom();
	ExecuteHooks(PipelineStage::PostPostProcess, ctx.get());

	// ── Tonemap + final blit ──────────────────────────────────────────────────

	if(!ctx->IsBlitSuppressed()) {
		ExecuteHooks(PipelineStage::PreUI, ctx.get());

		Renderer::StartPass(m_TonemapPass);
		{
			auto* cmd = Renderer::GetCommand();
			cmd->DepthTesting = DepthTestingMode::Off;
			cmd->Uniforms
			// mip 0 is the smallest; mip s_MipCount-1 is the largest (half-res)
			// the final accumulated bloom lives in mip index s_MipCount-1
			.Set("u_HDR",
				AttachmentSlot{ m_HDRBuffer->Get(AttachmentTarget::Color), 0 })
			.Set("u_Bloom",
				AttachmentSlot{
					m_BloomMips->Get(AttachmentTarget::Color, s_MipCount - 1), 1
				})
			.Set("u_Exposure", m_Exposure)
			.Set("u_BloomStrength", m_BloomStrength)
			.Set("u_SubPixelOffset", m_SubPixelOffset);

			auto* call = cmd->NewCall();
			call->VertexCount = 6;
			call->Primitive   = DrawPrimitive::Triangle;
			call->Partition   = DrawPartition::Single;
		}
		Renderer::EndPass();

		ExecuteHooks(PipelineStage::PostUI, ctx.get());
	}

	Renderer::Flush();
}

// ── Bloom compute passes ──────────────────────────────────────────────────────

void DefaultRenderPipeline::RunBloom() {
	// ── Downsample pass ───────────────────────────────────────────────────────
	// Mip 0: HDR scene → mip chain[0]  (threshold applied here)
	// Mip i: chain[i-1] → chain[i]
	Renderer::StartPass(m_DownsamplePass);
	{
		glm::vec2 srcRes{ (f32)m_Width, (f32)m_Height };
		auto srcAttachment = m_HDRBuffer->Get(AttachmentTarget::Color);

		for(u32 i = 0; i < s_MipCount; i++) {
			auto& mip = m_MipChain[i];

			auto* cmd = Renderer::NewCommand();
			cmd->Compute = true;
			cmd->ComputeX = (mip.Size.x + 7) / 8;
			cmd->ComputeY = (mip.Size.y + 7) / 8;
			cmd->ComputeZ = 1;
			cmd->Uniforms
			.Set("u_SrcResolution", srcRes)
			.Set("u_Threshold", m_BloomThreshold)
			.Set("u_IsMipZero", (i32)(i == 0));

			if(i == 0)
				cmd->Uniforms.Set("u_SrcTexture",
					AttachmentSlot{ srcAttachment, 0 });
			else
				cmd->Uniforms.Set("u_SrcTexture",
					AttachmentSlot{
						m_BloomMips->Get(AttachmentTarget::Color, i - 1), 0 });

			// Image binding for write output
			cmd->Outputs.Add({ AttachmentTarget::Color, i });

			srcRes = glm::vec2(mip.Size);
		}
	}
	Renderer::EndPass();

	// ── Upsample pass ─────────────────────────────────────────────────────────
	// Walk back up the chain, additively accumulating into each larger mip.
	// chain[s_MipCount-1] already contains the smallest level; we blend each
	// level into the next larger one, mirroring the demo's Upsample loop.
	Renderer::StartPass(m_UpsamplePass);
	{
		for(i32 i = (i32)s_MipCount - 1; i > 0; i--) {
			auto& srcMip = m_MipChain[i];
			auto& dstMip = m_MipChain[i - 1];

			auto* cmd = Renderer::NewCommand();
			cmd->Compute = true;
			cmd->ComputeX = (dstMip.Size.x + 7) / 8;
			cmd->ComputeY = (dstMip.Size.y + 7) / 8;
			cmd->ComputeZ = 1;
			cmd->Uniforms
			.Set("u_SrcTexture",
				 AttachmentSlot{
					m_BloomMips->Get(AttachmentTarget::Color, i), 0 })
			.Set("u_FilterRadius", m_FilterRadius)
			.Set("u_SrcResolution", glm::vec2(srcMip.Size));

			// Read-modify-write into the destination mip
			cmd->Outputs.Add({ AttachmentTarget::Color, (u32)(i - 1) });
		}
	}
	Renderer::EndPass();
}

}