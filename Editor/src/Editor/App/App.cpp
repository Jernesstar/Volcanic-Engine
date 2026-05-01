#include "App.h"

#include <filesystem>
#include <charconv>

#define RAPIDJSON_ASSERT(x) ((void)0)
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "Editor.h"
#include "../Asset/AssetManager.h"

namespace fs = std::filesystem;

u32 FRAME_W = 1920, FRAME_H = 1080;

namespace VolcanicEditor {

EditorApp::EditorApp(const CommandLineArgs& args)
	: Application({ .Name = "Editor", .TickRate = 120 },
		{
			.Title = "Editor", .Width = FRAME_W, .Height = FRAME_H,
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
