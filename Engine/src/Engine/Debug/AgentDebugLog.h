#pragma once

#include <chrono>
#include <fstream>
#include <string>

namespace VolcanicEngine::AgentDebug {

// #region agent log
inline void Log(const char* hypothesisId, const char* location,
	const char* message, const std::string& dataJson = "{}",
	const char* runId = "pre-fix")
{
	using Clock = std::chrono::system_clock;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		Clock::now().time_since_epoch()).count();
	std::ofstream out(
		"c:/Users/JMukala/Code/Work/Volcanic-Engine/debug-69cfb3.log",
		std::ios::app);
	if(!out)
		return;
	out << "{\"sessionId\":\"69cfb3\",\"runId\":\"" << runId
		<< "\",\"hypothesisId\":\"" << hypothesisId
		<< "\",\"location\":\"" << location
		<< "\",\"message\":\"" << message
		<< "\",\"data\":" << dataJson
		<< ",\"timestamp\":" << ms << "}\n";
}
// #endregion

} // namespace VolcanicEngine::AgentDebug
