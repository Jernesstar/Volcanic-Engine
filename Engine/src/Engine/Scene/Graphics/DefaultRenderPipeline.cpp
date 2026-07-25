#include "DefaultRenderPipeline.h"
#include "ScriptPipelineContext.h"

#include <unordered_map>
#include <algorithm>

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
	f32 Life;
};

static constexpr i32 k_EmitWorkGroup = 64;
static constexpr i32 k_UpdateWorkGroup = 128;

// Deferred lighting shader caps point lights at this many (u_PointLights[16] in
// DeferredLighting.glsl.frag). Kept in sync with the shader; when the scene
// exceeds it we keep the nearest lights and log once. (Sprint 64, task 3.2)
static constexpr u32 k_MaxPointLights = 16;

// Bind a material's properties to the deferred G-Buffer shader generically. A
// material's prop names ARE the shader uniform names (u_Albedo, u_AlbedoColor,
// u_HasAlbedoTexture, u_Emissive, ...), so every prop is written straight to the
// draw command. Texture props stored as an unresolved Asset are loaded through
// the AssetManager here (the generic MaterialBinder cannot reach the AssetManager
// without a circular include). No knowledge of any specific material lives here.
static void BindMaterialToGBuffer(DrawCommand* cmd, const Ref<Material>& mat) {
	// Map material props to the G-Buffer shader BY TYPE (not by name), so both the
	// authored materials (u_Albedo/u_AlbedoColor/u_Emissive) and model-imported
	// materials (DiffuseMap/DiffuseColor) resolve correctly: any texture prop
	// becomes the albedo texture, any Vec3/Vec4 the albedo colour, any float named
	// "emissive" the emissive strength. Unresolved Asset textures are loaded here.
	Vec4 albedoColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	f32 emissive = 0.0f;
	Ref<Texture> albedoTex;

	if(mat)
		for(auto& [name, prop] : mat->Props) {
			std::visit([&](auto&& v) {
				using T = std::decay_t<decltype(v)>;
				if constexpr(std::is_same_v<T, Ref<Texture>>) {
					if(v) albedoTex = v;
				}
				else if constexpr(std::is_same_v<T, Asset>) {
					if(v && v.Type == AssetType::Texture)
						albedoTex = AssetManager::Get()->Get<Texture>(v);
				}
				else if constexpr(std::is_same_v<T, Vec4>) {
					albedoColor = v;
				}
				else if constexpr(std::is_same_v<T, Vec3>) {
					albedoColor = Vec4(v, 1.0f);
				}
				else if constexpr(std::is_same_v<T, f32>) {
					if(name.find("Emissive") != Str::npos
					|| name.find("emissive") != Str::npos)
						emissive = v;
				}
			}, prop.Value);
		}

	cmd->Uniforms
	.Set("u_HasAlbedoTexture", (i32)(albedoTex ? 1 : 0))
	.Set("u_AlbedoColor", albedoColor)
	.Set("u_Emissive", emissive);
	if(albedoTex)
		cmd->Uniforms.Set("u_Albedo", TextureSlot{ albedoTex, 0 });
}

DefaultRenderPipeline::DefaultRenderPipeline(
	u32 renderW, u32 renderH, u32 outputW, u32 outputH)
	: m_RenderWidth(renderW), m_RenderHeight(renderH),
		m_OutputWidth(outputW), m_OutputHeight(outputH) { }

// Compose an entity's world transform by walking up the flecs parent chain, so
// spawned model hierarchies (a root entity with child mesh nodes) render at the
// root's placement rather than each node's local origin.
static Mat4 WorldTransform(ECS::Entity entity) {
	Mat4 m = Transform(entity.Get<TransformComponent>()).GetTransform();
	auto handle = entity.GetHandle().parent();
	while(handle.is_valid()) {
		ECS::Entity parent(handle);
		if(!parent.Has<TransformComponent>())
			break;
		m = Transform(parent.Get<TransformComponent>()).GetTransform() * m;
		handle = handle.parent();
	}
	return m;
}

void DefaultRenderPipeline::OnInit() {
	u32 w = m_RenderWidth, h = m_RenderHeight;

	const auto nearest = TextureSampling::Nearest;
	const auto flt = TextureFormat::Float;
	m_GBuffer = RendererAPI::CreateFramebuffer({
		{
			// Position + Normal must be float: world coords and signed normals
			// exceed [0,1] and would clamp in rgba8, corrupting lighting/shadows.
			{ AttachmentTarget::Color, w, h, nearest, flt }, // Position (loc 0)
			{ AttachmentTarget::Color, w, h, nearest, flt }, // Normal   (loc 1)
			{ AttachmentTarget::Color, w, h, nearest },      // Albedo   (loc 2)
			{ AttachmentTarget::Depth, w, h },
		}
	});

	m_HDRBuffer = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Color, w, h, nearest, flt }, // HDR (emissive > 1)
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
			mipSize = glm::max(mipSize, glm::ivec2(1));
			m_MipChain[i].Size = mipSize;
			bloomSpec.Attachments.Add({ AttachmentTarget::Color,
				(u32)mipSize.x, (u32)mipSize.y });
		}
		m_BloomMips = RendererAPI::CreateFramebuffer(bloomSpec);
	}

	m_OutputBuffer = RendererAPI::CreateFramebuffer({
		{
			{ AttachmentTarget::Color, m_OutputWidth, m_OutputHeight },
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
	m_TonemapPass  = RenderPass::Create("Tonemap", m_TonemapShader, m_OutputBuffer);
	// m_SkyboxPass   = RenderPass::Create("Skybox", m_SkyboxShader, m_HDRBuffer);
	m_DownsamplePass = RenderPass::Create("BloomDownsample", m_DownsampleShader, m_BloomMips);
	m_UpsamplePass   = RenderPass::Create("BloomUpsample", m_UpsampleShader, m_BloomMips);
	m_ParticleEmitPass   = RenderPass::Create("ParticleEmit",   m_ParticleEmitShader);
	m_ParticleUpdatePass = RenderPass::Create("ParticleUpdate", m_ParticleUpdateShader);
	m_ParticleDrawPass   = RenderPass::Create("ParticleDraw",   m_ParticleDrawShader,  m_HDRBuffer);
	// Surface passes are built lazily per game-supplied shader in RenderSurfaces.

	m_GeometryPass->SetData(Renderer3D::GetMeshBuffer());
	m_ShadowPass->SetData(Renderer3D::GetMeshBuffer());
	m_LightingPass->SetData(Renderer2D::GetScreenBuffer());
	m_TonemapPass->SetData(Renderer2D::GetScreenBuffer());
	// m_SkyboxPass->SetData(Renderer3D::GetCubemapBuffer());
	m_ParticleDrawPass->SetData(Renderer2D::GetScreenBuffer());

	Log::Info(
		"Initialized DefaultRenderPipeline (render {}x{}, output {}x{}; "
		"GBuffer {}x{}, HDR {}x{}, Output {}x{})",
		m_RenderWidth, m_RenderHeight, m_OutputWidth, m_OutputHeight,
		m_GBuffer->Get(AttachmentTarget::Color, 0)->Spec.Width,
		m_GBuffer->Get(AttachmentTarget::Color, 0)->Spec.Height,
		m_HDRBuffer->Get(AttachmentTarget::Color)->Spec.Width,
		m_HDRBuffer->Get(AttachmentTarget::Color)->Spec.Height,
		m_OutputBuffer->Get(AttachmentTarget::Color)->Spec.Width,
		m_OutputBuffer->Get(AttachmentTarget::Color)->Spec.Height);
}

void DefaultRenderPipeline::OnClose() {
	ClearRenderHooks();
	m_GBufferShader.reset();
	m_ShadowShader.reset();
	m_LightingShader.reset();
	m_DownsampleShader.reset();
	m_UpsampleShader.reset();
	m_TonemapShader.reset();
	m_SkyboxShader.reset();
	m_ParticleEmitShader.reset();
	m_ParticleUpdateShader.reset();
	m_ParticleDrawShader.reset();

	m_GeometryPass.reset();
	m_ShadowPass.reset();
	m_LightingPass.reset();
	m_TonemapPass.reset();
	m_DownsamplePass.reset();
	m_UpsamplePass.reset();
	m_ParticleEmitPass.reset();
	m_ParticleUpdatePass.reset();
	m_ParticleDrawPass.reset();
	m_SurfacePasses.clear();

	m_GBuffer.reset();
	m_HDRBuffer.reset();
	m_ShadowMap.reset();
	m_BloomMips.reset();
	m_OutputBuffer.reset();
	m_ParticleState.clear();
	m_SurfaceState.clear();
	Log::Info("DefaultRenderPipeline closed");
}

Ref<Framebuffer> DefaultRenderPipeline::GetOutput() const {
	return m_OutputBuffer;
}

Ref<Framebuffer> DefaultRenderPipeline::GetBuffer(const std::string& name) const {
	if(name == "GBuffer")   return m_GBuffer;
	if(name == "HDR")       return m_HDRBuffer;
	if(name == "ShadowMap") return m_ShadowMap;
	if(name == "Bloom")     return m_BloomMips;
	if(name == "Output")    return m_OutputBuffer;
	return nullptr;
}

static const char* s_HookMethodNames[] = {
	"void PreDepth(PipelineContext@)",
	"void PostDepth(PipelineContext@)",
	"void PreGeometry(PipelineContext@)",
	"void PostGeometry(PipelineContext@)",
	"void PreShadows(PipelineContext@)",
	"void PostShadows(PipelineContext@)",
	"void PreSkybox(PipelineContext@)",
	"void PostSkybox(PipelineContext@)",
	"void PreTransparency(PipelineContext@)",
	"void PostTransparency(PipelineContext@)",
	"void PrePostProcess(PipelineContext@)",
	"void PostPostProcess(PipelineContext@)",
	"void PreUI(PipelineContext@)",
	"void PostUI(PipelineContext@)"
};

void DefaultRenderPipeline::AddRenderHook(asIScriptObject* obj) {
	if(!obj)
		return;

	obj->AddRef();
	RenderHook hook;
	hook.Object = obj;
	auto* type = obj->GetObjectType();
	for(u32 i = 0; i <= (u32)PipelineStage::PostUI; i++)
		hook.Methods[i] = type->GetMethodByDecl(s_HookMethodNames[i]);

	m_RenderHooks.Add(hook);
}

void DefaultRenderPipeline::RemoveRenderHook(asIScriptObject* obj) {
	for(u64 i = 0; i < m_RenderHooks.Count(); i++) {
		if(m_RenderHooks[i].Object == obj) {
			obj->Release();
			m_RenderHooks.Pop(i);
			return;
		}
	}
}

void DefaultRenderPipeline::ClearRenderHooks() {
	for(u64 i = 0; i < m_RenderHooks.Count(); i++) {
		if(m_RenderHooks[i].Object)
			m_RenderHooks[i].Object->Release();
	}
	m_RenderHooks.Clear();
}

void DefaultRenderPipeline::ExecuteHooks(PipelineStage stage, ScriptPipelineContext* ctx) {
	u32 stageIdx = (u32)stage;

	for(auto& hook : m_RenderHooks) {
		auto* fn = hook.Methods[stageIdx];
		if(!fn) continue;

		ScriptFunc func{ fn, ScriptEngine::GetHookContext(), hook.Object };
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

// ── Grid surfaces ─────────────────────────────────────────────────────────────
// One quad per SurfaceGridComponent, drawn into HDR. The grid is uploaded to a
// double-buffered R-in-RGBA8 texture pair; on each new tick (Version bump) we swap
// handles (no blit) and upload the fresh field into curr. The surface's material
// names the shader; the fragment shader lerps prev/curr by TickAlpha and produces
// the look. Each distinct shader gets its own lazily-built RenderPass, cached by
// shader id. The pipeline holds no knowledge of any specific surface shader: it
// sets only engine-standard uniforms (placement, time, tick, grid textures) and
// binds every remaining material prop generically through the reflected layout.
void DefaultRenderPipeline::RenderSurfaces(Scene* scene, Ref<Camera> cam) {
	auto* am = AssetManager::Get();
	Mat4 viewProj = cam->GetViewProjection();

	scene->World3D.ForEach<SurfaceGridComponent>(
		[&](Entity& entity) {
			auto& surface = entity.Get<SurfaceGridComponent>();
			if(surface.GridWidth == 0 || surface.GridHeight == 0)
				return;

			// Resolve the material and its shader. Skip (warn once) if either is
			// missing — a surface with no shader cannot be drawn generically.
			if(!surface.MaterialAsset
			|| surface.MaterialAsset.Type != AssetType::Material) {
				static bool s_WarnedNoMat = false;
				if(!s_WarnedNoMat) {
					Log::Warning("SurfaceGridComponent has no material asset; "
						"skipping surface render.");
					s_WarnedNoMat = true;
				}
				return;
			}
			auto mat = am->Get<Material>(surface.MaterialAsset);
			if(!mat) return;

			Asset shaderAsset = mat->ShaderAsset;
			if(!shaderAsset || shaderAsset.Type != AssetType::Shader) {
				static bool s_WarnedNoShader = false;
				if(!s_WarnedNoShader) {
					Log::Warning("Surface material (id {}) resolves to no shader; "
						"skipping surface render.",
						(u64)surface.MaterialAsset.ID);
					s_WarnedNoShader = true;
				}
				return;
			}
			auto shader = am->Get<Shader>(shaderAsset);
			if(!shader) return;

			// Lazily build one pass per distinct shader id (cleared on OnClose).
			auto passIt = m_SurfacePasses.find((u64)shaderAsset.ID);
			if(passIt == m_SurfacePasses.end()) {
				auto pass = RenderPass::Create(
					"Surface", shader, m_HDRBuffer);
				passIt = m_SurfacePasses.emplace(
					(u64)shaderAsset.ID, pass).first;
			}

			u64 id = entity.GetHandle().id();
			auto& gpu = m_SurfaceState[id];

			// (Re)allocate the texture pair on first sighting or grid resize.
			if(!gpu.Initialized
			|| gpu.GridWidth != surface.GridWidth
			|| gpu.GridHeight != surface.GridHeight) {
				TextureSpec spec;
				spec.Width  = surface.GridWidth;
				spec.Height = surface.GridHeight;
				spec.Type = TextureType::RGBA;
				spec.Format = TextureFormat::Normal; // rgba8
				spec.Sampling = TextureSampling::Linear; // bilinear
				gpu.HeightPrev = RendererAPI::CreateTexture(spec);
				gpu.HeightCurr = RendererAPI::CreateTexture(spec);
				gpu.GridWidth  = surface.GridWidth;
				gpu.GridHeight = surface.GridHeight;
				gpu.LastVersion = surface.Version - 1; // force an upload below
				gpu.Initialized = true;
			}

			// Pack the grid into RGBA8 (value in R). Uploaded on every observed
			// Version increment; swapping curr->prev per step leaves prev as the
			// second-to-last state even if several ticks elapsed in one frame.
			auto upload = [&](const Ref<Texture>& tex) {
				Buffer<u8> px((u64)surface.GridWidth * surface.GridHeight * 4);
				for(u32 i = 0; i < surface.GridWidth * surface.GridHeight; i++) {
					u8 h = surface.Heights[i];
					u8 rgba[4] = { h, h, h, 255 };
					px.Add(rgba, 4);
				}
				tex->SetData(px);
			};

			if(gpu.LastVersion != surface.Version) {
				u32 diff = surface.Version - gpu.LastVersion; // wraps fine (u32)
				if(diff >= 2) {
					// Multiple ticks this frame: the last upload IS the second
					// -to-last, so make it prev, then upload latest as curr.
					std::swap(gpu.HeightPrev, gpu.HeightCurr);
					upload(gpu.HeightPrev);
					upload(gpu.HeightCurr);
				}
				else {
					// One tick: previous curr becomes prev, new field is curr.
					std::swap(gpu.HeightPrev, gpu.HeightCurr);
					upload(gpu.HeightCurr);
				}
				gpu.LastVersion = surface.Version;
			}

			// Quad world bounds. Origin is cell (0,0) centre; extend by half a
			// cell on each side so cell centres land on texel centres.
			f32 half = surface.CellSize * 0.5f;
			Vec2 minXZ = {
				surface.Origin.x - half,
				surface.Origin.z - half
			};
			Vec2 maxXZ = {
				surface.Origin.x + (surface.GridWidth  - 0.5f) * surface.CellSize,
				surface.Origin.z + (surface.GridHeight - 0.5f) * surface.CellSize
			};

			// Pre-resolve Asset-valued texture props to Ref<Texture> before the
			// generic bind (the MaterialBinder cannot reach the AssetManager). We
			// build a temporary resolved copy so the binder sees concrete textures
			// and assigns their sampler slots from the reflected layout. Slots 0/1
			// are reserved for the engine-standard grid textures below.
			Material resolved;
			resolved.ShaderAsset = mat->ShaderAsset;
			for(auto& [name, prop] : mat->Props) {
				if(std::holds_alternative<Asset>(prop.Value)) {
					Asset a = std::get<Asset>(prop.Value);
					if(a && a.Type == AssetType::Texture) {
						auto rtex = am->Get<Texture>(a);
						if(rtex)
							resolved.Props[name] =
								{ ShaderPropType::Texture, rtex };
					}
					// else: unresolved non-texture asset — drop it.
				}
				else
					resolved.Props[name] = prop;
			}

			Renderer::StartPass(passIt->second);
			{
				auto* cmd = Renderer::NewCommand();
				// Surfaces write depth and occlude correctly within HDR: the
				// shader discards uncovered texels and outputs alpha 1. Opaque
				// (Blending Off) avoids the GL_ONE,GL_ONE additive washout.
				cmd->DepthTesting = DepthTestingMode::On;
				cmd->DepthWrite = true;
				cmd->Blending = BlendingMode::Off;
				cmd->Culling = CullingMode::Off;

				// First forward command this frame: pull opaque scene depth from
				// the G-Buffer into the HDR depth buffer so the surface depth-tests
				// against walls/floor.
				if(!m_HDRDepthSynced) {
					cmd->DepthCopySrc = m_GBuffer.get();
					m_HDRDepthSynced = true;
				}

				// Engine-standard uniforms: placement, tick blend, time, and the
				// double-buffered grid textures on reserved slots 0/1.
				cmd->Uniforms
				.Set("u_ViewProj", viewProj)
				.Set("u_MinXZ", minXZ)
				.Set("u_MaxXZ", maxXZ)
				.Set("u_PlaneY", surface.PlaneY)
				.Set("u_TickAlpha", glm::clamp(surface.TickAlpha, 0.0f, 1.0f))
				.Set("u_Time", m_ElapsedTime)
				.Set("u_HeightPrev", TextureSlot{ gpu.HeightPrev, 0 })
				.Set("u_HeightCurr", TextureSlot{ gpu.HeightCurr, 1 });

				// Everything else (shading params + textures) via the generic
				// MaterialBinder against the shader's reflected layout — first real
				// caller of the reflected-layout path (Sprint 65).
				MaterialBinder::Bind(cmd, resolved, nullptr, &shader->GetLayout());

				auto* call = cmd->NewCall();
				call->VertexCount = 6;
				call->Primitive = DrawPrimitive::Triangle;
				call->Partition = DrawPartition::Single;
			}
			Renderer::EndPass();
		});
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
						{ "Life", BufferDataType::Float },
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
				gpu.Timer   += (f32)ts;
				gpu.Elapsed += (f32)ts; // monotonic, drives sway + spawn jitter

				// Spawn-rate jitter (authored amplitude): the emitter body
				// swells/thins on a slow cycle, on a different, slower frequency
				// than the light intensity flicker. Seeded per entity so a cluster
				// of emitters doesn't breathe in unison. SpawnJitter is the
				// amplitude (0 = steady). The SAME jittered interval must divide
				// and wrap the timer, or the accumulator drifts. (Sprint 64, task 3.1)
				f32 seed = (f32)(id % 997) * 0.618f;
				f32 jitter =
					1.0f + spec.SpawnJitter * sin(gpu.Elapsed * 7.3f + seed * 1.7f);
				f32 interval = glm::max(spec.SpawnInterval * jitter, 0.0001f);

				u32 toSpawn = (u32)(gpu.Timer / interval);
				gpu.Timer = glm::mod(gpu.Timer, interval);
				if(toSpawn == 0)
					return;

				u32 workGroups =
					(toSpawn + k_EmitWorkGroup - 1) / k_EmitWorkGroup;

				// Transform-relative spawn origin: emitter tracks its entity's
				// world transform (Sprint 64, task 1.1). Guard emitters that have
				// no TransformComponent (spawn at world origin + offset).
				Vec3 base = entity.Has<TransformComponent>()
					? Vec3(WorldTransform(entity)[3]) : Vec3(0.0f);
				Vec3 origin = base + spec.LocalOffset;

				auto* cmd = Renderer::NewCommand();
				cmd->Compute  = true;
				cmd->ComputeX = workGroups;
				cmd->Uniforms
				.Set("u_TimeStep", (f32)ts)
				.Set("u_ParticlesToSpawn", (i32)toSpawn)
				.Set("u_EmitterPosition", origin)
				.Set("u_ParticleLifetime", spec.ParticleLifetime)
				.Set("u_SpawnExtents", spec.SpawnExtents)
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 })
				.Set(StorageSlot{ gpu.FreeListBuffer, "", 1 });
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
				.Set("u_ElapsedTime", gpu.Elapsed) // sway phase (Sprint 64, task 2.2)
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 })
				.Set(StorageSlot{ gpu.FreeListBuffer, "", 1 });
			});
	}
	Renderer::EndPass();

	// ── Draw pass ─────────────────────────────────────────────────────────────
	// Billboarded quads rendered into the HDR buffer so emissive particles
	// feed into the bloom pass. Depth-tests against scene depth: the G-Buffer
	// depth is blit into the HDR depth by the first forward transparency
	// command (surfaces, or the first particle command below as a fallback).
	Renderer::StartPass(m_ParticleDrawPass);
	{
		Mat4 view = cam->GetView();
		Mat4 viewProj = cam->GetViewProjection();

		scene->World3D.ForEach<ParticleEmitterComponent>(
			[&](Entity& entity) {
				u64 id = entity.GetHandle().id();
				if(!m_ParticleState.count(id))
					return;

				auto& spec = entity.Get<ParticleEmitterComponent>();
				auto& gpu  = m_ParticleState[id];

				// Resolve the material texture if one is set. The emitter's
				// MaterialAsset may reference either a Texture directly or a
				// Material whose first texture prop we use.
				Ref<Texture> tex;
				if(spec.MaterialAsset) {
					if(spec.MaterialAsset.Type == AssetType::Texture)
						tex = AssetManager::Get()->Get<Texture>(spec.MaterialAsset);
					else if(spec.MaterialAsset.Type == AssetType::Material) {
						auto mat = AssetManager::Get()
							->Get<Material>(spec.MaterialAsset);
						if(mat)
							for(auto& [name, prop] : mat->Props)
								std::visit([&](auto&& v) {
									using T = std::decay_t<decltype(v)>;
									if constexpr(std::is_same_v<T, Ref<Texture>>) {
										if(v) tex = v;
									}
									else if constexpr(std::is_same_v<T, Asset>) {
										if(v && v.Type == AssetType::Texture)
											tex = AssetManager::Get()
												->Get<Texture>(v);
									}
								}, prop.Value);
					}
				}

				f32 half = spec.Size;

				auto* cmd = Renderer::NewCommand();
				// Particles test against scene depth (occluded by walls) but do
				// NOT write depth — additive sprites must not occlude each other
				// or the geometry behind them (task 2.4).
				cmd->DepthTesting = DepthTestingMode::On;
				cmd->DepthWrite = false;
				cmd->Blending = BlendingMode::Additive;
				cmd->Culling = CullingMode::Off;

				// Fallback: if no surface ran this frame, the HDR depth still holds
				// stale/cleared values, so the first particle command pulls the
				// G-Buffer depth in itself.
				if(!m_HDRDepthSynced) {
					cmd->DepthCopySrc = m_GBuffer.get();
					m_HDRDepthSynced = true;
				}

				cmd->Uniforms
				.Set("u_View", view)
				.Set("u_ViewProj", viewProj)
				.Set("u_BillboardWidth", half)
				.Set("u_BillboardHeight", half)
				// Lifetime feeds the vertex-stage age ramp (Sprint 64, task 2.3).
				// Same seconds unit as the particle's Life so t = 1 - Life/lifetime
				// is correct.
				.Set("u_ParticleLifetime", spec.ParticleLifetime)
				.Set("u_Color", spec.Color)
				// Three-stop age colour ramp (young -> mid -> old), multiplied
				// into the particle shading. Neutral white leaves Color unchanged.
				.Set("u_ColorStart", spec.ColorStart)
				.Set("u_ColorMid", spec.ColorMid)
				.Set("u_ColorEnd", spec.ColorEnd)
				.Set("u_HasTexture", (i32)(tex ? 1 : 0));

				if(tex)
					cmd->Uniforms.Set("u_Texture", TextureSlot{ tex, 0 });

				cmd->Uniforms
				.Set(StorageSlot{ gpu.ParticleBuffer, "", 0 });

				auto* call = cmd->NewCall();
				call->VertexCount = 6;
				call->InstanceCount = (u32)gpu.MaxParticleCount;
				call->Primitive = DrawPrimitive::Triangle;
				call->Partition = DrawPartition::Instanced;
			});
	}
	Renderer::EndPass();
}

void DefaultRenderPipeline::OnRender(Scene* scene, TimeStep ts) {
	m_ElapsedTime += (f32)ts; // monotonic clock for hardcoded flicker (Sprint 64)

	// Re-arm the once-per-frame G-Buffer→HDR depth blit (task 2.4). The first
	// forward transparency command that runs this frame does the copy.
	m_HDRDepthSynced = false;

	ScriptPipelineContext* ctx =
		ScriptPipelineContext::Factory(this, scene);

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

	// Particle emitters that emit light contribute a co-located point light into
	// the deferred lighting pass (Unity-style approximation): the whole emitter
	// lights nearby geometry rather than each particle individually. (Sprint 64)
	scene->World3D.ForEach<ParticleEmitterComponent>(
		[&](ECS::Entity& entity) {
			auto& spec = entity.Get<ParticleEmitterComponent>();
			if(!spec.EmitsLight)
				return;

			// Emitter-light flicker (light intensity): three mutually NON-harmonic
			// sines (11.3/19.7/31.1 base rates) so there's no periodic beat, seeded
			// per entity id so neighbouring emitters flicker independently. Applied
			// uniformly to Ambient/Diffuse/Specular. LightFlicker is the amplitude
			// (0 = steady) and LightFlickerSpeed scales the rates. (Sprint 64, task 3.1)
			f32 t = m_ElapsedTime * spec.LightFlickerSpeed;
			u64 id = entity.GetHandle().id();
			f32 seed = (f32)(id % 997) * 0.618f;
			f32 flicker = 1.0f + spec.LightFlicker * (
				  0.55f * sin(t * 11.3f + seed)
				+ 0.30f * sin(t * 19.7f + seed * 2.1f)
				+ 0.15f * sin(t * 31.1f + seed * 3.7f));

			f32 r = glm::max(spec.LightRadius, 0.5f);
			// Light colour follows the emitter tint (the pooled light reads as the
			// emitter's overall colour), but particle tints are authored in HDR
			// (e.g. 3.2 for bloom-bright sparks) — used raw as light intensity they
			// overexpose the whole level. Normalize to unit peak so the tint sets
			// the hue and LightRadius/flicker set the energy. (task 3.3)
			Vec3 tint = Vec3(spec.Color) * spec.Color.w;
			f32 peak = glm::max(tint.x, glm::max(tint.y, tint.z));
			if(peak > 1.0f)
				tint /= peak;
			Vec3 color = tint * 0.9f * flicker;

			// Light origin = world transform + LightOffset, slightly above the
			// spawn base so the emitter isn't lit from below. (task 1.1 / 3.3)
			Vec3 base = entity.Has<TransformComponent>()
				? Vec3(WorldTransform(entity)[3]) : Vec3(0.0f);

			PointLightComponent light;
			light.Position  = base + spec.LightOffset;
			light.Ambient   = color * 0.05f;
			light.Diffuse   = color;
			light.Specular  = color * 0.5f;
			light.Constant  = 1.0f;
			light.Linear    = 2.0f / r;   // ~near-zero attenuation around radius r
			light.Quadratic = 1.0f / (r * r);
			// Particles already write emissive HDR that blooms directly; this
			// light only contributes scene lighting. Keep it out of the bloom
			// bright-pass so the emitter glow isn't double-counted. (task 3.3)
			light.Bloom     = false;
			pointLights.Add(light);
		});

	scene->World3D.ForEach<SkyboxComponent>(
		[&](ECS::Entity& entity) {
			auto asset = entity.Get<SkyboxComponent>().CubemapAsset;
			if(asset && asset.Type == AssetType::Cubemap)
				skybox = AssetManager::Get()->Get<Cubemap>(asset);
		});

	if(!mainCamera) {
		ctx->Release();
		return;
	}

	mainCamera->Resize(m_RenderWidth, m_RenderHeight);

	// ── Shadow pass ───────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreShadows, ctx);

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

		scene->World3D.ForEach<MeshComponent, TransformComponent>(
			[&](ECS::Entity& entity) {
				auto& mesh = entity.Get<MeshComponent>();
				auto& tr = entity.Get<TransformComponent>();
				auto geo = AssetManager::Get()->Get<Geometry>(mesh.GeometryAsset);
				if(geo)
					Renderer3D::DrawGeometry(geo, WorldTransform(entity), cmd);
			});
		Renderer3D::End();
	}
	Renderer::EndPass();

	ExecuteHooks(PipelineStage::PostShadows, ctx);

	// ── Depth prepass ────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreDepthPrepass, ctx);

	ExecuteHooks(PipelineStage::PostDepthPrepass, ctx);

	// ── Geometry pass (G-Buffer fill) ────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreGeometry, ctx);

	Renderer::StartPass(m_GeometryPass);
	{
		Renderer::Clear();

		Mat4 viewProj = mainCamera->GetViewProjection();

		// Batch meshes by material: material uniforms live on the command, so we
		// use one command per distinct material and append every mesh of that
		// material as its own instanced draw call. This keeps the command count
		// (and GL state changes) small even for scenes with hundreds of meshes.
		// Safe because s_Commands is pre-allocated and does not reallocate.
		std::unordered_map<u64, DrawCommand*> matCommands;
		scene->World3D.ForEach<MeshComponent, TransformComponent>(
			[&](ECS::Entity& entity) {
				auto& mesh = entity.Get<MeshComponent>();
				auto& tr = entity.Get<TransformComponent>();

				auto geo = AssetManager::Get()->Get<Geometry>(mesh.GeometryAsset);
				if(!geo) return;

				u64 matKey = (u64)mesh.MaterialAsset.ID;
				DrawCommand* cmd;
				auto it = matCommands.find(matKey);
				if(it != matCommands.end())
					cmd = it->second;
				else {
					cmd = Renderer::NewCommand();
					cmd->Uniforms.Set("u_ViewProj", viewProj);
					auto mat = AssetManager::Get()->Get<Material>(mesh.MaterialAsset);
					BindMaterialToGBuffer(cmd, mat);
					matCommands[matKey] = cmd;
				}

				Renderer3D::DrawGeometry(geo, WorldTransform(entity), cmd);
			});

		// DrawGeometry sets alpha blending (Greatest) on the command, which is
		// wrong for the G-Buffer: g_Albedo.a carries emissive strength (0 for
		// most surfaces), so SRC_ALPHA blending would discard the albedo writes
		// entirely. G-Buffer attachments hold data, not colors — never blend.
		for(auto& [key, mcmd] : matCommands)
			mcmd->Blending = BlendingMode::Off;
	}
	Renderer::EndPass();

	ExecuteHooks(PipelineStage::PostGeometry, ctx);

	// ── Skybox ────────────────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PreSkybox, ctx);

	if(skybox && m_SkyboxPass) {
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

	ExecuteHooks(PipelineStage::PostSkybox, ctx);

	// ── Lighting pass (deferred) ──────────────────────────────────────────────

	Renderer::StartPass(m_LightingPass);
	{
		Renderer::Clear();
		auto* cmd = Renderer::GetCommand();
		cmd->DepthTesting = DepthTestingMode::Off;
		// Fullscreen lighting pass must not clobber the HDR depth buffer that
		// the forward surface/particle passes depth-test against (task 2.4). Its
		// Renderer::Clear() above is queued as a separate command and stays
		// ordered before the transparency passes' depth blit.
		cmd->DepthWrite = false;
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
		cmd->Uniforms.Set("u_DirLightCount",
			(i32)(dirLights.Count() < 4 ? dirLights.Count() : 4));

		// Point light budget (Sprint 64, task 3.2): the deferred lighting shader
		// only has k_MaxPointLights slots. When the scene has more (many emitter
		// lights + authored lights), keep the ones NEAREST the camera and
		// log once — lights past the cap were previously dropped silently.
		bool over = pointLights.Count() > k_MaxPointLights;
		if(over) {
			Vec3 camPos = mainCamera->GetPosition();
			std::sort(pointLights.begin(), pointLights.end(),
				[&](const PointLightComponent& a, const PointLightComponent& b) {
					Vec3 da = a.Position - camPos, db = b.Position - camPos;
					return glm::dot(da, da) < glm::dot(db, db); // squared distance
				});
		}
		// Latched: warn once when we go over, re-arm when back under so a recurring
		// problem is reported again but the log isn't spammed every frame.
		static bool s_CapWarned = false;
		if(over && !s_CapWarned) {
			Log::Warning("Point light count {} exceeds cap {}; dropping {} "
				"farthest lights.", pointLights.Count(), k_MaxPointLights,
				pointLights.Count() - k_MaxPointLights);
			s_CapWarned = true;
		}
		else if(!over)
			s_CapWarned = false;

		u32 pointCount = (u32)glm::min<u64>(pointLights.Count(), k_MaxPointLights);
		for(u32 i = 0; i < pointCount; i++) {
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
		cmd->Uniforms.Set("u_PointLightCount", (i32)pointCount);

		auto* call = cmd->NewCall();
		call->VertexCount = 6;
		call->Primitive = DrawPrimitive::Triangle;
		call->Partition = DrawPartition::Single;
	}
	Renderer::EndPass();

	// ── Transparency pass (forward, additive over HDR) ────────────────────────

	ExecuteHooks(PipelineStage::PreTransparency, ctx);
	// Surfaces first: they write depth and emissive body into HDR; particles then
	// draw over them (lifted above the plane so they don't z-fight).
	RenderSurfaces(scene, mainCamera);
	TickParticles(scene, ts, mainCamera);
	ExecuteHooks(PipelineStage::PostTransparency, ctx);

	// ── Bloom (compute) ───────────────────────────────────────────────────────

	ExecuteHooks(PipelineStage::PrePostProcess, ctx);
	RunBloom();
	ExecuteHooks(PipelineStage::PostPostProcess, ctx);

	// ── Tonemap + final blit ──────────────────────────────────────────────────

	if(!ctx->IsBlitSuppressed()) {
		ExecuteHooks(PipelineStage::PreUI, ctx);

		Renderer::StartPass(m_TonemapPass);
		{
			auto* cmd = Renderer::GetCommand();
			cmd->DepthTesting = DepthTestingMode::Off;
			cmd->Uniforms
			.Set("u_HDR",
				AttachmentSlot{ m_HDRBuffer->Get(AttachmentTarget::Color), 0 })
			.Set("u_Bloom",
				AttachmentSlot{
					m_BloomMips->Get(AttachmentTarget::Color, 0), 1
				})
			.Set("u_Exposure", m_Exposure)
			.Set("u_BloomStrength", m_BloomStrength)
			.Set("u_SubPixelOffset", m_SubPixelOffset)
			.Set("u_SrcWidth", (f32)m_RenderWidth)
			.Set("u_SrcHeight", (f32)m_RenderHeight);

			auto* call = cmd->NewCall();
			call->VertexCount = 6;
			call->Primitive   = DrawPrimitive::Triangle;
			call->Partition   = DrawPartition::Single;
		}
		Renderer::EndPass();

		ExecuteHooks(PipelineStage::PostUI, ctx);
	}

	Renderer::Flush();
	ctx->Release();
}

// ── Bloom compute passes ──────────────────────────────────────────────────────

void DefaultRenderPipeline::RunBloom() {
	// ── Downsample pass ───────────────────────────────────────────────────────
	// Mip 0: HDR scene → mip chain[0]  (threshold applied here)
	// Mip i: chain[i-1] → chain[i]
	Renderer::StartPass(m_DownsamplePass);
	{
		glm::vec2 srcRes{ (f32)m_RenderWidth, (f32)m_RenderHeight };
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