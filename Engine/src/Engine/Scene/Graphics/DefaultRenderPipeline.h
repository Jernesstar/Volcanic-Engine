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
	f32 Timer = 0.0f;
};

class DefaultRenderPipeline : public RenderPipeline {
public:
	DefaultRenderPipeline() = default;
	~DefaultRenderPipeline() = default;

	void OnInit() override;
	void OnRender(Scene* scene) override;
	void OnResize(u32 w, u32 h) override;
	Ref<Framebuffer> GetOutput() const override;

	// Hook registration
	void AddRenderHook(asIScriptObject* obj);
	void RemoveRenderHook(asIScriptObject* obj);

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

private:
	f32 m_BloomThreshold = 0.8f;
	f32 m_FilterRadius = 0.005f;
	f32 m_Exposure = 1.0f;
	f32 m_BloomStrength = 0.04f;
	f32 m_TimeStep = 0.0f;

	glm::vec2 m_SubPixelOffset = { 0.0f, 0.0f };

	u32 m_Width  = 1920;
	u32 m_Height = 1080;

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

	List<RenderHook> m_RenderHooks;

	friend class ScriptPipelineContext;
};

}