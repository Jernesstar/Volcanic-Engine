#pragma once

#include "RenderPipeline.h"

#include <Engine/Script/ScriptEngine.h>

namespace VolcanicEngine {

class ScriptPipelineContext;

class DefaultRenderPipeline : public RenderPipeline {
public:
	DefaultRenderPipeline() = default;
	~DefaultRenderPipeline() = default;

	void OnInit() override;
	void OnRender(Scene* scene) override;
	void OnResize(u32 w, u32 h) override;
	Ref<Framebuffer> GetOutput() const override;

	// // Hook registration — called by the script binding layer
	// void AddHook(PipelineStage stage, asIScriptFunction* fn);
	// void RemoveHook(PipelineStage stage, asIScriptFunction* fn);

	// // Named output access — scripts read these to composite on top
	// Ref<Framebuffer> GetBuffer(const std::string& name) const;

private:
	void ExecuteHooks(PipelineStage stage, ScriptPipelineContext* ctx);

	Ref<RenderPass> m_DepthPrepass;
	Ref<RenderPass> m_GeometryPass;
	Ref<RenderPass> m_ShadowPass;
	Ref<RenderPass> m_SkyboxPass;
	Ref<RenderPass> m_TransparencyPass;
	Ref<RenderPass> m_PostProcessPass;

	Ref<Framebuffer> m_DepthBuffer;
	Ref<Framebuffer> m_GBuffer;
	Ref<Framebuffer> m_HDRBuffer;
	Ref<Framebuffer> m_ShadowMap;

	Map<PipelineStage, List<asIScriptFunction*>> m_Hooks;
};

}