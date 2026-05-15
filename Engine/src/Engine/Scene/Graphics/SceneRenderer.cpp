#include "SceneRenderer.h"

#include "DefaultRenderPipeline.h"

namespace VolcanicEngine {

void SceneRenderer::SetPipeline(Ref<RenderPipeline> pipeline) {
	m_Pipeline = pipeline;
	m_Pipeline->OnInit();
}

void SceneRenderer::UseDefaultPipeline() {
	SetPipeline(CreateRef<DefaultRenderPipeline>());
}

void SceneRenderer::OnSceneLoad() {
	if(m_Pipeline)
		m_Pipeline->OnInit();
}

void SceneRenderer::OnSceneClose() { }

void SceneRenderer::Update(TimeStep ts) { }

void SceneRenderer::Render(Scene* scene) {
	if(m_Pipeline)
		m_Pipeline->OnRender(scene);
}

Ref<Framebuffer> SceneRenderer::GetOutput() const {
	if(!m_Pipeline)
		return nullptr;
	return m_Pipeline->GetOutput();
}

}