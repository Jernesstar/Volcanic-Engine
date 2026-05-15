#include "DefaultRenderPipeline.h"
// #include "ScriptPipelineContext.h"

using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

Ref<Framebuffer> DefaultRenderPipeline::GetOutput() const {

}

void DefaultRenderPipeline::OnInit() {

}

void DefaultRenderPipeline::OnRender(Scene* scene) {
	// auto ctx = ScriptPipelineContext::Factory(this, scene);

	// --- Depth Prepass ---
	// ExecuteHooks(PipelineStage::PreDepthPrepass, ctx);
	// Renderer::StartPass(m_depthPrepass);
	// scene->ForEach<MeshComponent, TransformComponent>([](auto& mesh, auto& tr) {
	// 	Renderer3D::DrawMesh(/* ... */);
	// });
	// Renderer::EndPass();
	// ExecuteHooks(PipelineStage::PostDepthPrepass, ctx);

	// // --- Geometry (G-Buffer fill) ---
	// ExecuteHooks(PipelineStage::PreGeometry, ctx);
	// Renderer::StartPass(m_geometryPass);
	// scene->ForEach<MeshComponent, TransformComponent>([](auto& mesh, auto& tr) {
	// 	Renderer3D::DrawMesh(/* ... */);
	// });
	// Renderer::EndPass();
	// ExecuteHooks(PipelineStage::PostGeometry, ctx);

	// // ... shadows, skybox, transparency, post-process, UI ...
}

void DefaultRenderPipeline::OnResize(u32 w, u32 h) {

}

void DefaultRenderPipeline::ExecuteHooks(PipelineStage stage, ScriptPipelineContext* ctx) {
	for(auto* f : m_Hooks[stage]) {
		ScriptFunc func{ f, ScriptEngine::GetContext() };
		func.CallVoid(ctx);
	}
}

}