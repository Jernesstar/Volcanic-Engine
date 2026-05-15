#include "Panels.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>

#include <Engine/Scene/Component.h>
#include <Engine/Graphics/Camera.h>

#include "Editor.h"

#undef near
#undef far

using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

// ── Helpers ───────────────────────────────────────────────────────────────────

// Wraps a component section in a collapsing header.
// The body lambda is only called when the header is open.
template<typename TComponent, typename TBody>
static void ComponentSection(const char* label,
							  ECS::Entity& entity,
							  TBody body)
{
	if(!entity.Has<TComponent>())
		return;

	ImGui::PushID(label);

	bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);

	// Right-click header → remove component
	if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("CompCtx");
	if(ImGui::BeginPopup("CompCtx")) {
		if(ImGui::MenuItem("Remove Component"))
			entity.Remove<TComponent>();
		ImGui::EndPopup();
	}

	if(open && entity.Has<TComponent>())
		body();

	ImGui::PopID();
}

// ── Per-component draw functions ──────────────────────────────────────────────

static void DrawCameraComponent(ECS::Entity& entity) {
	ComponentSection<CameraComponent>("Camera Component", entity, [&] {
		auto& comp = entity.Set<CameraComponent>();
		auto& cam  = comp.Cam;

		if(!cam) {
			if(ImGui::Button("Stereographic"))
				cam = Camera::Create(Camera::Type::Stereographic);
			ImGui::SameLine();
			if(ImGui::Button("Orthographic"))
				cam = Camera::Create(Camera::Type::Orthographic);
			ImGui::SameLine();
			if(ImGui::Button("Isometric"))
				cam = Camera::Create(Camera::Type::Isometric);
			return;
		}

		const char* typeStr =
			cam->GetType() == Camera::Type::Stereographic ? "Stereographic" :
			cam->GetType() == Camera::Type::Orthographic  ? "Orthographic"  :
			                                                 "Isometric";
		ImGui::Text("Type: %s", typeStr);
		ImGui::SameLine();
		if(ImGui::SmallButton("Switch")) {
			Camera::Type next =
				cam->GetType() == Camera::Type::Stereographic
					? Camera::Type::Orthographic
				: cam->GetType() == Camera::Type::Orthographic
					? Camera::Type::Isometric
				:   Camera::Type::Stereographic;
			cam = Camera::Create(next);
			return;
		}

		auto pos = cam->GetPosition();
		ImGui::SetNextItemWidth(180.0f);
		if(ImGui::DragFloat3("Position", &pos.x, 0.1f))
			cam->SetPosition(pos);

		auto dir = cam->GetDirection();
		ImGui::SetNextItemWidth(180.0f);
		if(ImGui::DragFloat3("Direction", &dir.x, 0.01f, -1.0f, 1.0f))
			cam->SetDirection(dir);

		uint32_t vW = cam->GetViewportWidth();
		uint32_t vH = cam->GetViewportHeight();
		uint32_t vMin = 0, vMax = 4096;
		ImGui::SetNextItemWidth(80.0f);
		bool wChanged =
			ImGui::DragScalar("Viewport W", ImGuiDataType_U32,
				&vW, 1.0f, &vMin, &vMax);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		bool hChanged =
			ImGui::DragScalar("Viewport H", ImGuiDataType_U32,
				&vH, 1.0f, &vMin, &vMax);
		if(wChanged || hChanged)
			cam->Resize(vW, vH);

		float near = cam->GetNear(), far = cam->GetFar();
		ImGui::SetNextItemWidth(80.0f);
		bool nearChanged = ImGui::DragFloat("Near", &near, 0.01f, 0.001f, 1000.0f, "%.3f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		bool farChanged  = ImGui::DragFloat("Far",  &far,  1.0f,  0.001f, 100000.0f, "%.1f");
		if(nearChanged || farChanged)
			cam->SetProjection(near, far);

		if(cam->GetType() == Camera::Type::Stereographic) {
			auto stereo = cam->As<StereographicCamera>();
			float fov = stereo->GetVerticalFOV();
			ImGui::SetNextItemWidth(80.0f);
			if(ImGui::DragFloat("FOV", &fov, 0.5f, 1.0f, 179.0f, "%.1f"))
				stereo->SetVerticalFOV(fov);
		}
		if(cam->GetType() == Camera::Type::Isometric) {
			auto iso = cam->As<IsometricCamera>();
			float dist = iso->R;
			ImGui::SetNextItemWidth(80.0f);
			if(ImGui::DragFloat("Distance", &dist, 0.5f, 1.0f, 10000.0f, "%.1f"))
				iso->SetDistance(dist);
		}
	});
}

static void DrawTagComponent(ECS::Entity& entity) {
	ComponentSection<TagComponent>("Tag Component", entity, [&] {
		auto& comp = entity.Set<TagComponent>();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("Tag", &comp.Tag);
	});
}

static void DrawTransformComponent(ECS::Entity& entity) {
	ComponentSection<TransformComponent>("Transform Component", entity, [&] {
		auto comp = entity.Get<TransformComponent>();

		glm::vec3 rotDeg = glm::degrees(comp.Rotation);

		ImGui::Text("Translation"); ImGui::SameLine(120.0f);
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("##T", glm::value_ptr(comp.Translation), 0.1f);

		ImGui::Text("Rotation");    ImGui::SameLine(120.0f);
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("##R",    glm::value_ptr(rotDeg), 0.5f, 0.0f, 360.0f, "%.2f");

		ImGui::Text("Scale");       ImGui::SameLine(120.0f);
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("##S",    glm::value_ptr(comp.Scale), 0.01f, 0.0001f, FLT_MAX, "%.3f");

		entity.Set<TransformComponent>() = {
			comp.Translation,
			glm::radians(rotDeg),
			comp.Scale
		};
	});
}

static void DrawAudioComponent(ECS::Entity& entity) {
	ComponentSection<AudioComponent>("Audio Component", entity, [&] {
		auto& comp = entity.Set<AudioComponent>();
		ImGui::Text("Asset ID: %llu", (uint64_t)comp.AudioAsset.ID);
		// TODO: hook up ContentBrowserPanel asset picker
	});
}

static void DrawMeshComponent(ECS::Entity& entity) {
	ComponentSection<MeshComponent>("Mesh Component", entity, [&] {
		auto& comp = entity.Set<MeshComponent>();
		ImGui::Text("Mesh Source: %llu",  (uint64_t)comp.MeshSourceAsset.ID);
		ImGui::Text("Material:    %llu",  (uint64_t)comp.MaterialAsset.ID);
		// TODO: hook up ContentBrowserPanel asset picker
	});
}

static void DrawSkyboxComponent(ECS::Entity& entity) {
	ComponentSection<SkyboxComponent>("Skybox Component", entity, [&] {
		auto& comp = entity.Set<SkyboxComponent>();
		ImGui::Text("Cubemap Asset: %llu", (uint64_t)comp.CubemapAsset.ID);
		// TODO: hook up ContentBrowserPanel asset picker
	});
}

static void DrawScriptComponent(ECS::Entity& entity) {
	ComponentSection<ScriptComponent>("Script Component", entity, [&] {
		auto& comp = entity.Set<ScriptComponent>();
		ImGui::Text("Module Asset: %llu", (uint64_t)comp.ModuleAsset.ID);
		if(comp.Instance)
			ImGui::Text("Class: %s",
				comp.Instance->GetClass()->Name.c_str());
		else
			ImGui::TextDisabled("(no instance)");
		// TODO: field editing, class selection
	});
}

static void DrawRigidBodyComponent(ECS::Entity& entity) {
	ComponentSection<RigidBodyComponent>("Rigid Body Component", entity, [&] {
		// auto& comp = entity.Set<RigidBodyComponent>();
		// TODO: physics not yet wired
		ImGui::TextDisabled("(physics disabled)");
	});
}

static void DrawDirectionalLightComponent(ECS::Entity& entity) {
	ComponentSection<DirectionalLightComponent>("Directional Light", entity, [&] {
		auto& c = entity.Set<DirectionalLightComponent>();
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Position",  &c.Position.x,  0.1f);
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Direction", &c.Direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Ambient",   &c.Ambient.x);
		ImGui::ColorEdit3("Diffuse",   &c.Diffuse.x);
		ImGui::ColorEdit3("Specular",  &c.Specular.x);
	});
}

static void DrawPointLightComponent(ECS::Entity& entity) {
	ComponentSection<PointLightComponent>("Point Light", entity, [&] {
		auto& c = entity.Set<PointLightComponent>();
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Position", &c.Position.x, 0.1f);
		ImGui::ColorEdit3("Ambient",  &c.Ambient.x);
		ImGui::ColorEdit3("Diffuse",  &c.Diffuse.x);
		ImGui::ColorEdit3("Specular", &c.Specular.x);
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Constant",  &c.Constant,  0.001f, 0.0f, 1.0f, "%.4f");
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Linear",    &c.Linear,    0.001f, 0.0f, 1.0f, "%.4f");
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Quadratic", &c.Quadratic, 0.001f, 0.0f, 1.0f, "%.4f");
		ImGui::Checkbox("Bloom",      &c.Bloom);
	});
}

static void DrawSpotlightComponent(ECS::Entity& entity) {
	ComponentSection<SpotlightComponent>("Spotlight", entity, [&] {
		auto& c = entity.Set<SpotlightComponent>();
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Position",  &c.Position.x,  0.1f);
		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Direction", &c.Direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Ambient",   &c.Ambient.x);
		ImGui::ColorEdit3("Diffuse",   &c.Diffuse.x);
		ImGui::ColorEdit3("Specular",  &c.Specular.x);
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Inner Cutoff",
			&c.CutoffAngle, 0.5f, 0.0f, c.OuterCutoffAngle, "%.2f");
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Outer Cutoff",
			&c.OuterCutoffAngle, 0.5f, c.CutoffAngle, 2.0f * 3.14159f, "%.2f");
	});
}

static void DrawParticleEmitterComponent(ECS::Entity& entity) {
	ComponentSection<ParticleEmitterComponent>("Particle Emitter", entity, [&] {
		auto& c = entity.Set<ParticleEmitterComponent>();

		ImGui::SetNextItemWidth(180.0f);
		ImGui::DragFloat3("Position", &c.Position.x, 0.1f);

		uint64_t pMin = 1, pMax = 100000;
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragScalar("Max Particles", ImGuiDataType_U64,
			&c.MaxParticleCount, 1.0f, &pMin, &pMax);

		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Lifetime (ms)",
			&c.ParticleLifetime, 1.0f, 1.0f, 99000.0f, "%.0f");
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Spawn Interval (ms)",
			&c.SpawnInterval, 1.0f, 1.0f, 99000.0f, "%.0f");
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Offset", &c.Offset, 0.01f, 0.0f, 1000.0f, "%.3f");

		ImGui::Text("Material: %llu", (uint64_t)c.MaterialAsset.ID);
		// TODO: hook up ContentBrowserPanel asset picker
	});
}

// ── Canvas components ─────────────────────────────────────────────────────────

static void DrawRectComponent(ECS::Entity& entity) {
	ComponentSection<RectComponent>("Rect Component", entity, [&] {
		auto& c = entity.Set<RectComponent>();
		ImGui::SetNextItemWidth(120.0f);
		ImGui::DragFloat2("Position", &c.Position.x, 0.5f);
		ImGui::SetNextItemWidth(120.0f);
		ImGui::DragFloat2("Size",     &c.Size.x,     0.5f, 0.0f, FLT_MAX);
		ImGui::ColorEdit4("Color",    &c.Color.x);
	});
}

static void DrawLayoutComponent(ECS::Entity& entity) {
	ComponentSection<LayoutComponent>("Layout Component", entity, [&] {
		auto& c = entity.Set<LayoutComponent>();

		const char* dirItems[]   = { "Vertical", "Horizontal" };
		const char* alignItems[] = { "Start", "Center", "End" };
		int dir   = (int)c.Direction;
		int align = (int)c.Alignment;

		ImGui::SetNextItemWidth(120.0f);
		if(ImGui::Combo("Direction", &dir, dirItems, 2))
			c.Direction = (UIAxisDirection)dir;
		ImGui::SetNextItemWidth(120.0f);
		if(ImGui::Combo("Alignment", &align, alignItems, 3))
			c.Alignment = (UIAlignment)align;

		ImGui::SetNextItemWidth(120.0f);
		ImGui::DragFloat2("Padding", &c.Padding.x, 0.5f, 0.0f, FLT_MAX);
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Gap", &c.Gap, 0.5f, 0.0f, FLT_MAX);
	});
}

static void DrawImageComponent(ECS::Entity& entity) {
	ComponentSection<ImageComponent>("Image Component", entity, [&] {
		auto& c = entity.Set<ImageComponent>();
		ImGui::Text("Image ID: %llu", (uint64_t)c.ImageID);
		ImGui::Checkbox("Preserve Aspect", &c.PreserveAspect);
		// TODO: hook up ContentBrowserPanel texture picker
	});
}

static void DrawTextComponent(ECS::Entity& entity) {
	ComponentSection<TextComponent>("Text Component", entity, [&] {
		auto& c = entity.Set<TextComponent>();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("Content", &c.Content);
		ImGui::SetNextItemWidth(80.0f);
		ImGui::DragFloat("Font Size", &c.FontSize, 0.5f, 4.0f, 256.0f, "%.1f");
		ImGui::ColorEdit4("Font Color", &c.FontColor.x);

		const char* alignItems[] = { "Start", "Center", "End" };
		int align = (int)c.HAlign;
		ImGui::SetNextItemWidth(120.0f);
		if(ImGui::Combo("H Align", &align, alignItems, 3))
			c.HAlign = (UIAlignment)align;
	});
}

static void DrawButtonComponent(ECS::Entity& entity) {
	ComponentSection<ButtonComponent>("Button Component", entity, [&] {
		auto& c = entity.Set<ButtonComponent>();
		ImGui::InputText("Label", &c.Label);
		ImGui::ColorEdit4("Normal",  &c.NormalColor.x);
		ImGui::ColorEdit4("Hovered", &c.HoveredColor.x);
		ImGui::ColorEdit4("Pressed", &c.PressedColor.x);
	});
}

// ── Add-component menu ────────────────────────────────────────────────────────

static void DrawAddComponentMenu(ECS::Entity& entity) {
	if(!ImGui::Button("Add Component"))
		return;
	ImGui::OpenPopup("AddComp");
}

static void DrawAddComponentPopup(ECS::Entity& entity) {
	if(!ImGui::BeginPopup("AddComp"))
		return;

#define ADD_IF_MISSING(T, Label) \
	if(!entity.Has<T>() && ImGui::MenuItem(Label)) entity.Add<T>();
	
	ImGui::SeparatorText("3D");
	ADD_IF_MISSING(CameraComponent,           "Camera")
	ADD_IF_MISSING(TagComponent,              "Tag")
	ADD_IF_MISSING(TransformComponent,        "Transform")
	ADD_IF_MISSING(AudioComponent,            "Audio")
	ADD_IF_MISSING(MeshComponent,             "Mesh")
	ADD_IF_MISSING(SkyboxComponent,           "Skybox")
	ADD_IF_MISSING(ScriptComponent,           "Script")
	ADD_IF_MISSING(RigidBodyComponent,        "Rigid Body")
	ADD_IF_MISSING(DirectionalLightComponent, "Directional Light")
	ADD_IF_MISSING(PointLightComponent,       "Point Light")
	ADD_IF_MISSING(SpotlightComponent,        "Spotlight")
	ADD_IF_MISSING(ParticleEmitterComponent,  "Particle Emitter")

	ImGui::SeparatorText("2D");

	ImGui::SeparatorText("Canvas");
	ADD_IF_MISSING(RectComponent,   "Rect")
	ADD_IF_MISSING(LayoutComponent, "Layout")
	ADD_IF_MISSING(ImageComponent,  "Image")
	ADD_IF_MISSING(TextComponent,   "Text")
	ADD_IF_MISSING(ButtonComponent, "Button")

#undef ADD_IF_MISSING

	ImGui::EndPopup();
}

// ── Panel ─────────────────────────────────────────────────────────────────────

void ComponentEditorPanel::Update(TimeStep ts) {
	// Keep context in sync with the global selection
	ECS::Entity sel = Editor::GetSelected();
	if(sel != m_Context)
		m_Context = sel;
}

void ComponentEditorPanel::Draw() {
	ImGui::Begin("Component Editor", &Open);

	if(!m_Context || !m_Context.IsValid()) {
		ImGui::TextDisabled("(no entity selected)");
		ImGui::End();
		return;
	}

	// Entity name header
	const auto& name = m_Context.GetName();
	ImGui::SeparatorText(name.empty() ? "Unnamed Entity" : name.c_str());
	ImGui::Spacing();

	// ── 3D components ──────────────────────────────────────────────────────
	DrawCameraComponent(m_Context);
	DrawTagComponent(m_Context);
	DrawTransformComponent(m_Context);
	DrawAudioComponent(m_Context);
	DrawMeshComponent(m_Context);
	DrawSkyboxComponent(m_Context);
	DrawScriptComponent(m_Context);
	DrawRigidBodyComponent(m_Context);
	DrawDirectionalLightComponent(m_Context);
	DrawPointLightComponent(m_Context);
	DrawSpotlightComponent(m_Context);
	DrawParticleEmitterComponent(m_Context);

	// ── Canvas components ──────────────────────────────────────────────────
	DrawRectComponent(m_Context);
	DrawLayoutComponent(m_Context);
	DrawImageComponent(m_Context);
	DrawTextComponent(m_Context);
	DrawButtonComponent(m_Context);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawAddComponentMenu(m_Context);
	DrawAddComponentPopup(m_Context);

	ImGui::End();
}

}