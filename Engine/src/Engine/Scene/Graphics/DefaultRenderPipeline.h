#pragma once

#include "RenderPipeline.h"

#include <Engine/Script/ScriptEngine.h>
#include <Engine/Graphics/Camera.h>

namespace VolcanicEngine {

class ScriptPipelineContext;

struct ParticleEmitterGPU {
	Ref<StorageBuffer> ParticleBuffer;
	Ref<StorageBuffer> FreeListBuffer;
	u64 MaxParticleCount = 0;
	f32 Timer = 0.0f;    // wrapped every spawn interval (glm::mod) — spawn timing
	f32 Elapsed = 0.0f;  // monotonic per-emitter time — drives sway/flicker phases
};

// Double-buffered grid textures for one SurfaceGridComponent. Prev/Curr hold
// consecutive tick states, lerped by TickAlpha in the shader. The grid value
// packs into the R channel of an RGBA8 texture (the only upload path the texture
// backend exposes); 256 levels is ample.
struct SurfaceGPU {
	Ref<Texture> HeightPrev;
	Ref<Texture> HeightCurr;
	u32 GridWidth = 0;
	u32 GridHeight = 0;
	u32 LastVersion = 0; // last Version we uploaded; swap+upload when it changes
	bool Initialized = false;
};

class DefaultRenderPipeline : public RenderPipeline {
public:
	DefaultRenderPipeline(
		u32 renderW = 1920, u32 renderH = 1080,
		u32 outputW = 1920, u32 outputH = 1080);
	~DefaultRenderPipeline() { ClearRenderHooks(); }

	void OnInit() override;
	void OnRender(Scene* scene, TimeStep ts) override;
	void OnClose() override;
	Ref<Framebuffer> GetOutput() const override;

	// Hook registration
	void AddRenderHook(asIScriptObject* obj);
	void RemoveRenderHook(asIScriptObject* obj);
	void ClearRenderHooks();

	// Named buffer access (for ScriptPipelineContext)
	Ref<Framebuffer> GetBuffer(const std::string& name) const;

private:
	struct RenderHook {
		asIScriptObject* Object;
		asIScriptFunction* Methods[(u64)PipelineStage::PostUI + 1];
	};

private:
	void ExecuteHooks(PipelineStage stage, ScriptPipelineContext* ctx);
	void RunBloom();
	void TickParticles(Scene* scene, TimeStep ts, Ref<Camera> cam);
	void RenderSurfaces(Scene* scene, Ref<Camera> cam);

private:
	f32 m_BloomThreshold = 0.8f;
	f32 m_FilterRadius = 0.005f;
	f32 m_Exposure = 1.0f;
	f32 m_BloomStrength = 0.04f;
	f32 m_TimeStep = 0.0f;
	f32 m_ElapsedTime = 0.0f; // monotonic; feeds u_Time to time-varying shaders

	// Tracks whether the HDR framebuffer's depth has already had the G-Buffer
	// depth blit into it this frame. The first forward transparency command
	// (surfaces, else particles) performs the blit; later ones skip it. Reset each
	// OnRender.
	bool m_HDRDepthSynced = false;

	glm::vec2 m_SubPixelOffset = { 0.0f, 0.0f };

	u32 m_RenderWidth  = 1920;
	u32 m_RenderHeight = 1080;
	u32 m_OutputWidth  = 1920;
	u32 m_OutputHeight = 1080;

	static constexpr u32 s_MipCount = 6;

	Ref<RenderPass> m_GeometryPass;
	Ref<RenderPass> m_ShadowPass;
	Ref<RenderPass> m_LightingPass;
	Ref<RenderPass> m_SkyboxPass;
	Ref<RenderPass> m_DownsamplePass;
	Ref<RenderPass> m_UpsamplePass;
	Ref<RenderPass> m_TonemapPass;
	Ref<RenderPass> m_ParticleEmitPass;
	Ref<RenderPass> m_ParticleUpdatePass;
	Ref<RenderPass> m_ParticleDrawPass;

	// One RenderPass per distinct surface shader id, built lazily the first time a
	// SurfaceGridComponent using that shader is drawn, cleared on OnClose. Surface
	// shaders are game-supplied (resolved from each surface's material), so there is
	// no single fixed pass — the pipeline holds no knowledge of any specific shader.
	Map<u64, Ref<RenderPass>> m_SurfacePasses;

	Ref<Framebuffer> m_GBuffer;
	Ref<Framebuffer> m_HDRBuffer;
	Ref<Framebuffer> m_ShadowMap;
	Ref<Framebuffer> m_BloomMips;
	Ref<Framebuffer> m_OutputBuffer;

	struct MipInfo { glm::ivec2 Size; };
	MipInfo m_MipChain[s_MipCount];

	Ref<Shader> m_GBufferShader;
	Ref<Shader> m_ShadowShader;
	Ref<Shader> m_LightingShader;
	Ref<Shader> m_DownsampleShader;
	Ref<Shader> m_UpsampleShader;
	Ref<Shader> m_TonemapShader;
	Ref<Shader> m_SkyboxShader;
	Ref<Shader> m_ParticleEmitShader;
	Ref<Shader> m_ParticleUpdateShader;
	Ref<Shader> m_ParticleDrawShader;

	Map<u64, ParticleEmitterGPU> m_ParticleState;
	Map<u64, SurfaceGPU> m_SurfaceState;

	List<RenderHook> m_RenderHooks;

	friend class ScriptPipelineContext;
};

}