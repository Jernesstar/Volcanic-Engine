#include "App.h"

#include <filesystem>
#include <charconv>

#define RAPIDJSON_ASSERT(x) ((void)0)
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <Engine/Audio/AudioEngine.h>

#include "Editor.h"
#include "../Asset/AssetManager.h"

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application(
		{ .Name = "Editor", .TickRate = 60 },
		{ .Title = "Editor" }
	)
{
	VolcanicEngine::Audio::AudioEngine::Init();
	Editor::Init(args);
}

EditorApp::~EditorApp() {
	Editor::Close();
	VolcanicEngine::Audio::AudioEngine::Shutdown();
}

void EditorApp::OnUpdate(TimeStep ts) {
	Editor::Update(ts);
	Editor::Render();
}

}