#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "FileStream.h"

namespace VolcaniCore {

class BinaryReader : public FileStream {
public:
	BinaryReader(const std::string& path) {
		if(!FileUtils::PathExists(path))
			Log::Error("File does not exist: {}", path);
		m_Stream.open(path, std::ios::in | std::ios::binary);
	}

	~BinaryReader() = default;

	BinaryReader& ReadData(void* data, u64 size) {
		m_Stream.read((char*)data, size);
		return *this;
	}

	template<typename TPrimitive>
	BinaryReader& ReadRaw(TPrimitive& value) {
		ReadData((void*)&value, sizeof(TPrimitive));
		return *this;
	}

	template<typename TData>
	BinaryReader& ReadObject(TData& value);

	template<typename TData>
	BinaryReader& Read(TData& value) {
		if constexpr(std::is_trivial<TData>())
			ReadRaw<TData>(value);
		else
			ReadObject<TData>(value);
		return *this;
	}

	template<typename T>
	BinaryReader& Read(Buffer<T>& buffer) {
		u64 count;
		Read(count);
		buffer.Allocate(count);
		buffer.AddRange(count);
		return ReadData(buffer.Get(), count * sizeof(T));
	}

	template<typename TData, typename... Args>
	BinaryReader& Read(List<TData>& values, Args&&... args) {
		u64 size;
		ReadRaw<u64>(size);
		values.Allocate(size);
		for(u64 i = 0; i < size; i++)
			Read(values.Emplace(std::forward<Args>(args)...));

		return *this;
	}

	template<typename TKey, typename TValue>
	BinaryReader& Read(Map<TKey, TValue>& map) {
		u64 size;
		ReadRaw<u64>(size);
		map.reserve(size);

		for(u64 i = 0; i < size; i++) {
			TKey key;
			Read(key); Read(map[key]);
		}

		return *this;
	}

	template<typename TKey, typename TValue>
	BinaryReader& Read(OMap<TKey, TValue>& map) {
		u64 size;
		ReadRaw<u64>(size);

		for(u64 i = 0; i < size; i++) {
			TKey key;
			Read(key);
			Read(map[key]);
		}

		return *this;
	}
};

template<>
inline BinaryReader& BinaryReader::ReadObject(std::string& str) {
	u64 size;
	ReadRaw<u64>(size);
	if(size) {
		str.resize(size);
		ReadData((void*)str.data(), size);
	}

	return *this;
}

}