#pragma once

#include <VolcaniCore/Core/CommandLineArgs.h>

using namespace VolcaniCore;

namespace Magma {

class Editor {
public:
	static void RegisterInterface();

public:
	enum class EditorMode {
		None, Flow, Project
	} Mode = EditorMode::None;

public:
	Editor() { s_Instance = this; }
	~Editor() = default;

	void Open();
	void Close();

	void Load(const CommandLineArgs& args);
	void Update(TimeStep ts);
	void Render();

private:
	inline static Editor* s_Instance;

private:
	void LoadHomeScreen();
	void LoadFlowEditor();
	void LoadProjectEditor();
};

}