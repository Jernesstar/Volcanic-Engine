#include "Panels.h"

#include <imgui/imgui.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Window/Input.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Platform/OpenGL/Texture.h>
#include <Engine/Graphics/Platform/OpenGL/Framebuffer.h>

#include "Editor.h"
#include "EditorRenderPipeline.h"

using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

static ImTextureID FramebufferColorID(Ref<Framebuffer> fb) {
	if(!fb)
		return (ImTextureID)0;
	auto att = fb->Get(AttachmentTarget::Color);
	if(!att)
		return (ImTextureID)0;
	auto glAtt = att->As<OpenGL::Attachment>();
	auto tex = glAtt->GetRendererID();
	return (ImTextureID)(intptr_t)tex;
}

void SceneVisualizerPanel::Update(TimeStep ts) {
	auto mode = Editor::GetMode();

	if(m_Hovered || Input::GetCursorMode() == CursorMode::Locked)
		m_Controller.OnUpdate(ts);

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

	m_Hovered = ImGui::IsWindowHovered();

	// ── View mode tab bar ─────────────────────────────────────────────────
	if(ImGui::BeginTabBar("VisualizerTabs", ImGuiTabBarFlags_None)) {
		if(Editor::GetMode() != EditorMode::Edit
		&& ImGui::BeginTabItem("Composite"))
		{
			m_ViewMode = ViewMode::Composite;
			pipeline->Render3D = true;
			pipeline->Render2D = true;
			pipeline->RenderCanvas = true;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("3D")) {
			m_ViewMode = ViewMode::World3D;
			pipeline->Render3D = true;
			pipeline->Render2D = false;
			pipeline->RenderCanvas = false;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("2D")) {
			m_ViewMode = ViewMode::World2D;
			pipeline->Render3D = false;
			pipeline->Render2D = true;
			pipeline->RenderCanvas = false;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Canvas")) {
			m_ViewMode = ViewMode::Canvas;
			pipeline->Render3D = false;
			pipeline->Render2D = false;
			pipeline->RenderCanvas = true;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	DrawViewport();

	ImGui::End();
}

void SceneVisualizerPanel::SetContext(Scene* scene) {
	m_Scene = scene;
	auto camera = CreateRef<StereographicCamera>(75.0f);
	camera->SetPosition({ 0.0f, 1.0f, 15.0f });
	camera->Resize(1920, 1080);
	camera->SetProjection(0.001f, 10'000.0f);
	m_Controller.SetCamera(camera);
	m_Controller.TranslationSpeed = 25.0f;

	if(scene) {
		m_EditorPipeline.OnInit();
		m_EditorPipeline.Render3D = false;
		m_EditorPipeline.Render2D = false;
		m_EditorPipeline.RenderCanvas = false;
		m_EditorPipeline.SetCamera(camera);
	}
}

void SceneVisualizerPanel::OnResize(u32 w, u32 h) {

}

void SceneVisualizerPanel::DrawViewport() {
	ImVec2 availSize = ImGui::GetContentRegionAvail();

	Ref<Framebuffer> fb = GetActiveFramebuffer();
	ImTextureID texID = FramebufferColorID(fb);

	// 1. Fill the background with a Unity-like grey color
	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.0f });
	ImGui::BeginChild("##ViewportContainer", availSize,
		ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

	if(!texID) {
		ImVec2 center = {
			ImGui::GetCursorPosX() + availSize.x * 0.5f - 60.0f,
			ImGui::GetCursorPosY() + availSize.y * 0.5f - 7.0f
		};
		ImGui::SetCursorPos(center);
		ImGui::TextDisabled("(no output)");
		ImGui::EndChild();
		ImGui::PopStyleColor();
		return;
	}

	float targetWidth = 1920.0f;
	float targetHeight = 1080.0f;
	float targetAspect = targetWidth / targetHeight;
	float availAspect = availSize.x / availSize.y;

	ImVec2 imageSize;

	// 3. Calculate aspect ratio fitting
	if (availAspect > targetAspect) {
		// Window is wider than the target aspect ratio (Pillarbox)
		imageSize.y = availSize.y;
		imageSize.x = availSize.y * targetAspect;
	} else {
		// Window is taller than the target aspect ratio (Letterbox)
		imageSize.x = availSize.x;
		imageSize.y = availSize.x / targetAspect;
	}

	// 4. Center the image inside the child window
	ImVec2 imagePos = {
		(availSize.x - imageSize.x) * 0.5f,
		(availSize.y - imageSize.y) * 0.5f
	};
	ImGui::SetCursorPos(imagePos);
	// 5. Draw the scaled image
	ImGui::Image(texID, imageSize, { 0.0f, 1.0f }, { 1.0f, 0.0f });
	m_Hovered = ImGui::IsItemHovered();

	ImGui::EndChild();
	ImGui::PopStyleColor();

	// ── Play-mode overlay ────────────────────────────────────────────────
	if(Editor::GetMode() == EditorMode::Play || Editor::GetMode() == EditorMode::Pause) {
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
			if(Editor::GetMode() == EditorMode::Play)
				ImGui::TextColored({ 0.2f, 1.0f, 0.2f, 1.0f }, "PLAYING");
			else if(Editor::GetMode() == EditorMode::Pause)
				ImGui::TextColored({ 1.0f, 1.0f, 0.2f, 1.0f }, "PAUSED");
		}
		ImGui::End();
	}

	ImVec2 overlayPos = {
		ImGui::GetItemRectMax().x - 120.0f,
		ImGui::GetItemRectMin().y + 10.0f
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
	if(ImGui::Begin("##DebugOverlay", nullptr, overlayFlags)) {
		auto info = Renderer::GetDebugInfo();
		ImGui::Text("FPS: %0.1f", info.FPS);
		ImGui::Text("Draw Calls: %li", info.DrawCalls);
		ImGui::Text("Indices: %li", info.Indices);
		ImGui::Text("Vertices: %li", info.Vertices);
		ImGui::Text("Instances: %li", info.Instances);
	}
	ImGui::End();
}

Ref<Framebuffer> SceneVisualizerPanel::GetActiveFramebuffer() const {
	auto mode = Editor::GetMode();
	if(mode == EditorMode::Edit)
		return m_EditorPipeline.GetOutput();
	if(App::Get())
		return App::Get()->GetSceneRenderer().GetOutput();

	return nullptr;
}

}