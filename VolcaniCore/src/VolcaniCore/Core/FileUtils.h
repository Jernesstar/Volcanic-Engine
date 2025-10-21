#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "List.h"

#undef CreateFile

namespace fs = std::filesystem;

namespace VolcaniCore {

class File {
public:
	const fs::path& Path;

public:
	File(const fs::path& path, bool clear = false);

	File& Write(const std::string& info);

	std::string Get() const;

	void Close();

private:
	std::ofstream m_Data;
};

class FileUtils {
public:
	static bool PathExists(const fs::path& path);

	static void CreateFile(const fs::path& path);

	static std::string ReadFile(const fs::path& path);

	static void WriteToFile(const fs::path& path, const std::string& info);

	static List<std::string> GetFiles(const fs::path& dir);
	static List<std::string> GetFiles(
		const fs::path& dir, const List<fs::path>& ext);
};

}