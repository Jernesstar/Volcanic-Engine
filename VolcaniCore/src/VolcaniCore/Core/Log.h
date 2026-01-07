#pragma once

#include <print>
#include <string>

namespace VolcaniCore {

class Log {
public:
	template<typename... Args>
	static void Info(std::format_string<Args...> fmt, Args&&... args)
	{
		std::print("[Info]: ");
		std::println(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Warning(std::format_string<Args...> fmt, Args&&... args)
	{
		std::print("[Warning]: ");
		std::println(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Error(std::format_string<Args...> fmt, Args&&... args)
	{
		std::print("[Error]: ");
		std::println(fmt, std::forward<Args>(args)...);
	}
};

}