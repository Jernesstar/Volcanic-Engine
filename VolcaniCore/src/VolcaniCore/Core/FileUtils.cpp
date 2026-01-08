#include "FileUtils.h"

#include <filesystem>
#include <fstream>

#include "Core/Assert.h"

namespace VolcaniCore {

File::File(const fs::path& path, bool clear)
	: Path(path)
{
	m_Data.open(path.string().c_str(), std::ios::out | (clear ? std::ios::trunc : std::ios::app));
}

File& File::Write(const std::string& data) {
	m_Data << data.c_str() << "\n";
	return *this;
}

std::string File::Get() const {
	return FileUtils::ReadFile(Path);
}

void File::Close() {
	m_Data.close();
}

bool FileUtils::PathExists(const fs::path& path) {
	return fs::exists(path.c_str());
}

void FileUtils::CreateFile(const fs::path& path) {
	std::ofstream file{ path.c_str() };
}

std::string FileUtils::ReadFile(const fs::path& filePath) {
	std::ifstream in(filePath, std::ios::in);
	VOLCANICORE_ASSERT_ARGS(in, "Could not open file: {}", filePath.string());

	std::stringstream buffer;
	buffer << in.rdbuf();
	return buffer.str();
}

void FileUtils::WriteToFile(const fs::path& path, const std::string& info) {
	std::ofstream fout(path.c_str());
	fout << info.c_str();
}

List<std::string> FileUtils::GetFiles(const fs::path& dir) {
	List<std::string> files;

	if(fs::is_directory(dir))
		for(auto p : fs::directory_iterator(dir))
			files.Add(p.path().string());

	return files;
}

List<std::string> FileUtils::GetFiles(
	const fs::path& dir, const List<fs::path>& extensions)
{
	List<std::string> files;
	if(fs::is_directory(dir))
		for(auto p : fs::directory_iterator(dir))
			for(auto& ext : extensions)
				if(p.path().extension().string() == ext)
					files.Add(p.path().string());

	return files;
}

}