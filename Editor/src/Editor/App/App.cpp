#include "App.h"

#include "Editor.h"

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Editor", 60 })
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