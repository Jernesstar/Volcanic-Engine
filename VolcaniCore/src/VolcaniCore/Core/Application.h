#pragma once

#include "Time.h"
#include "Defines.h"

int main(int argc, char** argv);

namespace VolcaniCore {

struct AppSpecification {
	std::string Name;
	u32 TickRate = 0; // = 0 => Fast as possible; = n => n ticks per sec
};

class Application {
public:
	Application(const AppSpecification& spec = { "Application" });
	virtual ~Application() = default;

	virtual void OnUpdate(TimeStep ts) = 0;

protected:
	virtual void Run();
	TimePoint m_LastFrame{ Time::GetTime() };

	inline static Application* s_Instance;
	inline static AppSpecification s_Spec;

public:
	static void Close(u32 code = 0);
	static Application* Get();

	static std::string GetHomeDir();
	static std::string GetCurrentDir();
	static std::string GetLibraryDir();
	static void PushDir();
	static void PushDir(const std::string& path);
	static void PopDir();

	template<typename TDerived>
	requires std::derived_from<TDerived, Application>
	static TDerived* As() { return (TDerived*)Get(); }

private:
	static void SetCurrentDir();

	inline static std::string s_LibraryPath;
	inline static std::string s_Path;

	friend int ::main(int argc, char** argv);
};

}