#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

namespace VolcaniCore {

class BytesReader {
public:
	Buffer<u8> Bytes;

public:
	BytesReader(Buffer<u8>&& bytes)
		: Bytes(bytes) { }
	~BytesReader() = default;

	BytesReader& ReadData(void* data, u64 size) {
		memcpy(data, Bytes.Get(m_Position), size);
		m_Position += size;
		return *this;
	}

	template<typename TPrimitive>
	BytesReader& ReadRaw(TPrimitive& value) {
		ReadData((void*)&value, sizeof(TPrimitive));
		return *this;
	}

	template<typename TData>
	BytesReader& ReadObject(TData& value);

	template<typename TData>
	BytesReader& Read(TData& value) {
		if constexpr(std::is_trivial<TData>())
			ReadRaw<TData>(value);
		else
			ReadObject<TData>(value);
		return *this;
	}

	template<typename T>
	BytesReader& Read(Buffer<T>& buffer) {
		u64 count;
		Read(count);
		buffer.Allocate(count);
		buffer.AddRange(count);
		return ReadData(buffer.Get(), count * sizeof(T));
	}

	template<typename TData, typename... Args>
	BytesReader& Read(List<TData>& values, Args&&... args) {
		u64 size;
		ReadRaw<u64>(size);
		values.Allocate(size);
		for(u64 i = 0; i < size; i++)
			Read(values.Emplace(std::forward<Args>(args)...));

		return *this;
	}

	template<typename TKey, typename TValue>
	BytesReader& Read(Map<TKey, TValue>& map) {
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
	BytesReader& Read(OMap<TKey, TValue>& map) {
		u64 size;
		ReadRaw<u64>(size);

		for(u64 i = 0; i < size; i++) {
			TKey key;
			Read(key);
			Read(map[key]);
		}

		return *this;
	}

private:
	u64 m_Position = 0;
};

template<>
inline BytesReader& BytesReader::ReadObject(std::string& str) {
	u64 size;
	ReadRaw<u64>(size);
	if(size) {
		str.resize(size);
		ReadData((void*)str.data(), size);
	}

	return *this;
}

}