#include "Panels.h"

#include <imgui/imgui.h>

#include <VolcaniCore/Core/Application.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Platform/OpenGL/Texture.h>
#include <Engine/Graphics/Platform/OpenGL/Framebuffer.h>

#include "Editor.h"
#include "EditorRenderPipeline.h"

using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

static ImTextureID FramebufferColorID(Ref<Framebuffer> fb) {
	if(!fb)
		return (ImTextureID)0;
	auto att = fb->Get(AttachmentTarget::Color)->As<OpenGL::Attachment>();
	auto tex = att->GetRendererID();
	return (ImTextureID)(intptr_t)tex;
}

void SceneVisualizerPanel::Update(TimeStep ts) {
	auto mode = Editor::GetMode();

	if(mode == EditorMode::Edit && m_Scene) {
		m_EditorPipeline.SetSelectedEntity(Editor::GetSelected());
		m_EditorPipeline.OnRender(m_Scene);
	}
}

void SceneVisualizerPanel::Draw() {
	auto winFlags = ImGuiWindowFlags_NoScrollbar
				  | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::Begin("Scene Visualizer", &Open, winFlags);
	ImGui::PopStyleVar();
	RenderPipeline* pipeline =
		Editor::GetMode() == EditorMode::Edit
			? &m_EditorPipeline
			: App::Get()->GetSceneRenderer().GetPipeline().get();

	// ── View mode tab bar ─────────────────────────────────────────────────
	if(ImGui::BeginTabBar("VisualizerTabs", ImGuiTabBarFlags_None)) {
		if(ImGui::BeginTabItem("3D")) {
			m_ViewMode = ViewMode::World3D;
			pipeline->Render3D = true;
			pipeline->Render2D = false;
			pipeline->RenderCanvas = false;
			DrawViewport();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("2D")) {
			m_ViewMode = ViewMode::World2D;
			pipeline->Render3D = false;
			pipeline->Render2D = true;
			pipeline->RenderCanvas = false;
			DrawViewport();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Canvas")) {
			m_ViewMode = ViewMode::Canvas;
			pipeline->Render3D = false;
			pipeline->Render2D = false;
			pipeline->RenderCanvas = true;
			DrawViewport();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Composite")) {
			m_ViewMode = ViewMode::Composite;
			pipeline->Render3D = true;
			pipeline->Render2D = true;
			pipeline->RenderCanvas = true;
			DrawViewport();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void SceneVisualizerPanel::SetContext(Scene* scene) {
	m_Scene = scene;
	if(scene)
		m_EditorPipeline.OnInit();
}

void SceneVisualizerPanel::OnResize(u32 w, u32 h) {
	m_EditorPipeline.OnResize(w, h);
}

void SceneVisualizerPanel::DrawViewport() {
	ImVec2 size = ImGui::GetContentRegionAvail();

	// Pick which framebuffer to display
	Ref<Framebuffer> fb = GetActiveFramebuffer();
	ImTextureID texID = FramebufferColorID(fb);

	if(!texID) {
		// No output yet — draw a dark placeholder
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.08f, 0.08f, 0.08f, 1.0f });
		ImGui::BeginChild("##VP", size);
		ImVec2 center = {
			ImGui::GetCursorPosX() + size.x * 0.5f - 60.0f,
			ImGui::GetCursorPosY() + size.y * 0.5f - 7.0f
		};
		ImGui::SetCursorPos(center);
		ImGui::TextDisabled("(no output)");
		ImGui::EndChild();
		ImGui::PopStyleColor();
		return;
	}

	// Blit — flip UV vertically so OpenGL origin (bottom-left) maps correctly
	ImGui::Image(texID, size, { 0.0f, 1.0f }, { 1.0f, 0.0f });

	// Resize pipeline when the panel is resized
	if(size.x != m_LastSize.x || size.y != m_LastSize.y) {
		m_LastSize = size;
		OnResize((u32)size.x, (u32)size.y);
		if(App::Get() && Editor::GetMode() != EditorMode::Edit)
			App::Get()->GetSceneRenderer().GetPipeline()->OnResize(
				(u32)size.x, (u32)size.y);
	}

	// ── Play-mode overlay ────────────────────────────────────────────────
	if(Editor::GetMode() == EditorMode::Play) {
		ImVec2 overlayPos = {
			ImGui::GetItemRectMin().x + 8.0f,
			ImGui::GetItemRectMin().y + 8.0f
		};
		ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.55f);
		auto overlayFlags =
			ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoInputs
			| ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoFocusOnAppearing
			| ImGuiWindowFlags_NoNav;
		if(ImGui::Begin("##PlayOverlay", nullptr, overlayFlags)) {
			ImGui::TextColored({ 0.2f, 1.0f, 0.2f, 1.0f }, "▶  PLAYING");
		}
		ImGui::End();
	}
}

Ref<Framebuffer> SceneVisualizerPanel::GetActiveFramebuffer() const {
	auto mode = Editor::GetMode();

	if(mode == EditorMode::Edit)
		return m_EditorPipeline.GetOutput();

	// Preview / Play — read from the App's renderer
	if(App::Get())
		return App::Get()->GetSceneRenderer().GetOutput();

	return nullptr;
}

}