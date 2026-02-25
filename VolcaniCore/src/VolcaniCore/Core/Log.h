#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace VolcaniCore {

class Log {
public:
	static void Init() {
		auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic.txt");
		spdlog::set_default_logger(logger);
	}

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