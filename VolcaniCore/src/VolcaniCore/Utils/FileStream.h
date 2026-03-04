#pragma once

#include <fstream>

namespace VolcaniCore {

class FileStream {
public:
	FileStream() = default;
	~FileStream() {
		m_Stream.close();
	}

	uint64_t GetPosition() {
		return m_Stream.tellg();
	}

	void SetPosition(u64 pos) {
		m_Stream.seekg(pos, std::ios::beg);
	}

	void Advance(u64 size) {
		SetPosition(GetPosition() + size);
	}

protected:
	std::fstream m_Stream;
};

}