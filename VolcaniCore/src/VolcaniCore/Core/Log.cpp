#include "Log.h"

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Application.h"

namespace VolcaniCore {

void Log::Init(bool file) {
	if(file) {
		auto path = Application::GetLibraryDir() + "/logs/VolcanicEngine.txt";
		auto logger = spdlog::basic_logger_mt("basic_logger", path);
		spdlog::set_default_logger(logger);
	}
	else {
		auto logger = spdlog::stdout_logger_mt("basic_logger");
		spdlog::set_default_logger(logger);
	}
}

}