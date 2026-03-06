#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

namespace VolcaniCore {

class Log {
public:
	static void Init(bool file = false);

	template<typename... Args>
	static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		spdlog::info(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Warning(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		spdlog::warn(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		spdlog::error(fmt, std::forward<Args>(args)...);
	}
};

}