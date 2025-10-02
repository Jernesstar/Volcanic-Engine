#pragma once

#include "TimeUtils.h"
#include "Defines.h"
#include "Window.h"

int main(int argc, char** argv);

namespace VolcaniCore {

class Application {
public:
	Application(const WindowSpecification& spec = { "Application" });
	virtual ~Application() = default;

	static void Close();
	static Application* Get();
	static Ref<Window> GetWindow();

	static std::string GetHomeDir();
	static std::string GetCurrentDir();
	static std::string GetLibraryDir();
	static void PushDir();
	static void PushDir(const std::string& path);
	static void PopDir();

	template<typename TDerived>
	requires std::derived_from<TDerived, Application>
	static TDerived* As() { return (TDerived*)Get(); }

protected:
	virtual void OnUpdate(TimeStep ts) { }

private:
	static void Init();
	static void Run();
	static void SetCurrentDir();

	friend int ::main(int argc, char** argv);
};

}