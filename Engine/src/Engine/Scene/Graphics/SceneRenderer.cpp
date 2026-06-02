#include "SceneRenderer.h"

#include "DefaultRenderPipeline.h"

namespace VolcanicEngine {

void SceneRenderer::SetPipeline(Ref<RenderPipeline> pipeline) {
	if(m_Pipeline && m_Pipeline != pipeline)
		m_Pipeline->OnClose();
	m_Pipeline = pipeline;
	if(m_Pipeline)
		m_Pipeline->OnInit();
}

void SceneRenderer::UseDefaultPipeline() {
	SetPipeline(CreateRef<DefaultRenderPipeline>());
}

void SceneRenderer::UseDefaultPipeline(
	u32 renderW, u32 renderH, u32 outputW, u32 outputH)
{
	SetPipeline(CreateRef<DefaultRenderPipeline>(
		renderW, renderH, outputW, outputH));
}

void SceneRenderer::OnSceneLoad() {
}

void SceneRenderer::OnSceneClose() {
	if(m_Pipeline) {
		m_Pipeline->OnClose();
		m_Pipeline.reset();
	}
}

void SceneRenderer::Render(Scene* scene, TimeStep ts) {
	if(!m_Pipeline)
		return;
	m_Pipeline->OnRender(scene, ts);
}

Ref<Framebuffer> SceneRenderer::GetOutput() const {
	if(!m_Pipeline)
		return nullptr;
	return m_Pipeline->GetOutput();
}

}