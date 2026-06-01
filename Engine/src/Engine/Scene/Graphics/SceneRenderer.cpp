#include "SceneRenderer.h"

#include "DefaultRenderPipeline.h"

#include <chrono>
#include <fstream>
#include <sstream>

namespace VolcanicEngine {

// #region agent log
static void AgentLog(const char* loc, const char* msg, const char* hyp,
	const std::string& dataJson)
{
	std::ofstream f("/home/jernesstar/Code/Work/.cursor/.cursor/debug-3c4d03.log",
		std::ios::app);
	if(!f) return;
	auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	f << "{\"sessionId\":\"3c4d03\",\"location\":\"" << loc
	  << "\",\"message\":\"" << msg << "\",\"hypothesisId\":\"" << hyp
	  << "\",\"data\":" << dataJson << ",\"timestamp\":" << ts << "}\n";
	f.flush();
}
// #endregion

void SceneRenderer::SetPipeline(Ref<RenderPipeline> pipeline) {
	// #region agent log
	{
		void* oldPtr = m_Pipeline ? m_Pipeline.get() : nullptr;
		std::ostringstream d;
		d << "{\"oldPipeline\":" << (oldPtr ? std::to_string((uintptr_t)oldPtr) : "null")
		  << ",\"newPipeline\":"
		  << (pipeline ? std::to_string((uintptr_t)pipeline.get()) : "null") << "}";
		AgentLog("SceneRenderer.cpp:SetPipeline", "set pipeline", "H2", d.str());
	}
	// #endregion
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
	// #region agent log
	{
		std::ostringstream d;
		d << "{\"pipeline\":"
		  << (m_Pipeline ? std::to_string((uintptr_t)m_Pipeline.get()) : "null")
		  << "}";
		AgentLog("SceneRenderer.cpp:OnSceneLoad", "scene load re-init", "H1", d.str());
	}
	// #endregion
}

void SceneRenderer::OnSceneClose() {
	// #region agent log
	{
		std::ostringstream d;
		d << "{\"pipeline\":"
		  << (m_Pipeline ? std::to_string((uintptr_t)m_Pipeline.get()) : "null")
		  << "}";
		AgentLog("SceneRenderer.cpp:OnSceneClose", "scene close", "H5", d.str());
	}
	// #endregion
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