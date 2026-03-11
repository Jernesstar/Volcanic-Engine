#include "App.h"

#include "Editor.h"

u32 FRAME_W = 1280, FRAME_H = 720;

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ .Name = "Editor", .TickRate = 120 },
		{
			.Title = "Editor", .Width = 1280, .Height = 720,
			.Hidden = args.Has("--embedded")
		}
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