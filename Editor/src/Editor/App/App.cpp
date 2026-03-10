#include "App.h"

#include "Editor.h"

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ .Name = "Editor", .TickRate = 60 },
		{ .Title = "Editor", .Width = 1280, .Height = 720, .Hidden = args.Has("--embedded") }
	)
{
	Editor::Init(args);
}

EditorApp::~EditorApp() {
	Editor::Close();
}

void EditorApp::OnUpdate(TimeStep ts) {
	Editor::Update(ts);
	Editor::Render();
}

}