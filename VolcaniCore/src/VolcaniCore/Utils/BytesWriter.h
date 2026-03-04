#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

namespace VolcaniCore {

class BytesWriter {
public:
	Buffer<u8> Bytes;

public:
	BytesWriter(u64 size)
		: Bytes(size * 2) { }
	~BytesWriter() = default;

	BytesWriter& WriteData(const void* data, u64 byteCount) {
		Bytes.Add(data, byteCount);
		return *this;
	}

	template<typename TPrimitive>
	BytesWriter& WriteRaw(const TPrimitive& value) {
		WriteData((void*)&value, sizeof(TPrimitive));
		return *this;
	}

	template<typename TData>
	BytesWriter& WriteObject(const TData& value);

	template<typename TData>
	BytesWriter& Write(const TData& value) {
		if constexpr(std::is_trivial<TData>())
			WriteRaw<TData>(value);
		else
			WriteObject<TData>(value);
		return *this;
	}

	template<typename T>
	BytesWriter& Write(const Buffer<T>& buffer) {
		Write(buffer.GetCount());
		return WriteData(buffer.Get(), buffer.GetSize());
	}

	template<typename TData>
	BytesWriter& Write(const List<TData>& values) {
		WriteRaw<u64>(values.Count());
		for(const auto& val : values)
			Write(val);

		return *this;
	}

	template<typename TKey, typename TValue>
	BytesWriter& Write(const Map<TKey, TValue>& map) {
		WriteRaw<u64>(map.size());
		for(auto& [key, val] : map) {
			Write(key);
			Write(val);
		}

		return *this;
	}

	template<typename TKey, typename TValue>
	BytesWriter& Write(const OMap<TKey, TValue>& map) {
		WriteRaw<u64>(map.size());
		for(auto& [key, val] : map) {
			Write(key);
			Write(val);
		}

		return *this;
	}
};

template<>
inline BytesWriter& BytesWriter::WriteObject(const std::string& str) {
	Write((u64)str.size());
	if(str.size())
		WriteData((void*)str.data(), str.size());

	return *this;
}

}