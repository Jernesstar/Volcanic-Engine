#include <VolcaniCore/Core/CommandLineArgs.h>
#include <VolcaniCore/Core/TimeUtils.h>

using namespace VolcaniCore;

namespace VolcanicEditor {

class Editor {
public:
	static void Init(const CommandLineArgs& args);
	static void Close();

	static void Update(TimeStep ts);
	static void Render();

	static void OpenProject(const std::string& path);
	static void NewProject(const std::string& path);
	static void SaveProject();

	static void NewScene(const std::string& name);
	static void OpenScene(const std::string& name);
	static void SaveScene(const std::string& name);

	static void NewCanvas(const std::string& name);
	static void OpenCanvas(const std::string& name);
	static void SaveCanvas(const std::string& name);
};

}