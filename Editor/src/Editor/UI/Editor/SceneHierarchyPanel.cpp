#include "Panels.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Engine/Scene/Component.h>

#include "Editor.h"

namespace VolcanicEditor {

// ── Internal helpers ──────────────────────────────────────────────────────────

struct AddEntityOptions {
	bool pending   = false;
	bool nameError = false;
};

// Draws a single entity tree node.
// Returns true if the node was opened (caller must TreePop).
static bool DrawEntityNode(ECS::Entity& entity, ECS::Entity& selected) {
	auto flags = ImGuiTreeNodeFlags_SpanAvailWidth
			   | ImGuiTreeNodeFlags_OpenOnArrow
			   | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if(entity == selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	// Leaf entities (no children) get a bullet so the arrow doesn't appear
	// TODO: replace with entity.HasChildren() once hierarchy is wired
	flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	void* id = (void*)(uint64_t)(uint32_t)entity.GetHandle();
	const auto& name = entity.GetName();
	ImGui::TreeNodeEx(id, flags, "%s", name.empty() ? "Unnamed Entity" : name.c_str());

	bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left)
				&& !ImGui::IsItemToggledOpen();
	if(clicked) {
		selected = entity;
		Editor::SetSelected(entity);
	}

	bool deleteRequested = false;
	if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("EntityCtx");

	if(ImGui::BeginPopup("EntityCtx")) {
		if(ImGui::MenuItem("Delete")) {
			deleteRequested = true;
			if(selected == entity) {
				selected = ECS::Entity{ };
				Editor::ClearSelected();
			}
		}
		ImGui::EndPopup();
	}

	if(deleteRequested)
		entity.Kill();

	return false; // leaf nodes never need TreePop
}

// Draws the "Add Entity" popup triggered by right-clicking empty space.
// Pass the ECS::World to add the new entity into.
static void DrawAddEntityPopup(ECS::World& world, AddEntityOptions& opts) {
	if(opts.pending)
		ImGui::OpenPopup("AddEntity");

	if(!ImGui::BeginPopup("AddEntity"))
		return;

	static std::string s_Name;
	if(opts.nameError)
		ImGui::TextColored({ 1.0f, 0.3f, 0.3f, 1.0f }, "Name already in use");

	ImGui::SetNextItemWidth(200.0f);
	ImGui::InputTextWithHint("##Name", "Entity name (optional)", &s_Name);

	if(ImGui::Button("Create")
	|| ImGui::IsKeyPressed(ImGuiKey_Enter, false))
	{
		opts.nameError = false;
		if(!s_Name.empty() && world.GetEntity(s_Name).IsValid()) {
			opts.nameError = true;
		}
		else {
			if(s_Name.empty())
				world.AddEntity();
			else
				world.AddEntity(s_Name);
			s_Name.clear();
			opts.pending = false;
			opts.nameError = false;
			ImGui::CloseCurrentPopup();
		}
	}
	ImGui::SameLine();
	if(ImGui::Button("Cancel")) {
		s_Name.clear();
		opts.pending = false;
		opts.nameError = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

static void DrawWorldSection(const char* label,
							 ECS::World& world,
							 ECS::Entity& selected)
{
	static AddEntityOptions s_Opts3D, s_Opts2D, s_OtpsCanvas;
	AddEntityOptions* opts =
		(label[6] == '3') ? &s_Opts3D :
		(label[6] == '2') ? &s_Opts2D : &s_OtpsCanvas;

	bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);

	// Right-click on the section header → add entity
	if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
		opts->pending = true;

	ImGui::PushID(label);
	DrawAddEntityPopup(world, *opts);
	ImGui::PopID();

	if(!open)
		return;

	ImGui::PushID(label);

	bool anyEntity = false;
	world.ForEach(
		[&](ECS::Entity& entity) {
			anyEntity = true;
			DrawEntityNode(entity, selected);
		});

	if(!anyEntity) {
		ImGui::Indent();
		ImGui::TextDisabled("(empty - right-click header to add)");
		ImGui::Unindent();
	}

	// Click on empty window area → deselect
	if(ImGui::IsWindowHovered()
	&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
	&& !ImGui::IsAnyItemHovered())
	{
		selected = ECS::Entity{ };
		Editor::ClearSelected();
	}

	ImGui::PopID();
}

// ── Panel ─────────────────────────────────────────────────────────────────────

void SceneHierarchyPanel::Update(TimeStep ts) { }

void SceneHierarchyPanel::Draw() {
	ImGui::Begin("Scene Hierarchy", &Open);

	ECS::Entity selected = Editor::GetSelected();

	if(!m_Scene) {
		ImGui::TextDisabled("(no scene open)");
		ImGui::End();
		return;
	}

	DrawWorldSection("World 3D", m_Scene->World3D, selected);
	ImGui::Spacing();
	DrawWorldSection("World 2D", m_Scene->World2D, selected);
	ImGui::Spacing();
	DrawWorldSection("Canvas", m_Scene->Canvas, selected);

	ImGui::End();
}

}