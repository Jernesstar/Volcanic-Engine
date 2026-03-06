#include "App.h"

#include "Editor.h"

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ "Editor" })
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