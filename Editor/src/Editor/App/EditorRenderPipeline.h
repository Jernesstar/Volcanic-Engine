#pragma once

#include <Engine/Graphics/RenderPass.h>
#include <Engine/Graphics/Platform/Framebuffer.h>
#include <Engine/Scene/Graphics/RenderPipeline.h>

namespace VolcanicEngine {

// Lightweight pipeline used only in the editor's Edit mode.
// No post-processing or shadows — just geometry drawn into an HDR colour buffer
// with a basic depth prepass. The visualizer blits the colour attachment.
class EditorRenderPipeline : public RenderPipeline {
public:
	EditorRenderPipeline() = default;
	~EditorRenderPipeline() = default;

	void OnInit() override;
	void OnRender(Scene* scene) override;
	void OnResize(u32 w, u32 h) override;

	Ref<Framebuffer> GetOutput() const override { return m_OutputBuffer; }

	// Entity outline for selection highlight
	void SetSelectedEntity(ECS::Entity entity) { m_Selected = entity; }

private:
	Ref<RenderPass> m_GeometryPass;
	Ref<RenderPass> m_SkyboxPass;
	Ref<RenderPass> m_OutlinePass;  // draws selection outline on top

	Ref<Framebuffer> m_OutputBuffer; // colour + depth

	ECS::Entity m_Selected;
};

}