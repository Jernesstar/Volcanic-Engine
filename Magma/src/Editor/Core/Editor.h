#pragma once

#include <VolcaniCore/Core/CommandLineArgs.h>

using namespace VolcaniCore;

namespace Magma {

struct Component {
	std::string Name;
	std::string Path;
	bool BuildAuto;
	List<std::string> CoreDeps;
	List<std::string> EditorDeps;
};

struct LavaFlow {
	std::string Name;
	std::string Path;
	List<std::string> Components;
	List<std::string> ObjectList;
};

class Editor {
public:
	static void RegisterInterface();

public:
	enum class EditorMode {
		None, Component, Flow, Project
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
	void RenderStartScreen();
	void RenderComponentEditor();
	void RenderFlowEditor();
	void RenderProjectEditor();
};

}