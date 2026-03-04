#pragma once

#include <cstring>
#include <initializer_list>

#include "Core/Defines.h"
#include "Core/Assert.h"

namespace VolcaniCore {

template<typename T>
class Buffer {
public:
	Buffer() = default;
	Buffer(u64 maxCount)
		: m_MaxCount(maxCount)
	{
		if(maxCount)
			m_Data = (T*)malloc(GetMaxSize());
	}
	Buffer(Buffer&& other)
		: m_MaxCount(other.GetMaxCount()), m_Count(other.GetCount())
	{
		std::swap(m_Data, other.m_Data);
	}
	Buffer(const std::initializer_list<T>& list)
		: m_MaxCount(list.size())
	{
		if(!m_MaxCount)
			return;

		m_Data = (T*)malloc(GetMaxSize());
		Set(list.begin(), list.size());
	}
	Buffer(T* data, u64 count, u64 maxCount = 0, bool own = true)
		: m_MaxCount(maxCount), m_Count(count)
	{
		if(!m_MaxCount)
			m_MaxCount = count;

		if(own)
			m_Data = data;
		else {
			m_Data = (T*)malloc(GetMaxSize());
			memcpy(m_Data, data, GetSize());
		}
	}

	~Buffer() {
		Delete();
	}

	operator bool() const { return m_Data && m_Count; }

	Buffer& operator =(const Buffer& other) = delete;

	Buffer& operator =(Buffer&& other) {
		std::swap(m_Data, other.m_Data);
		m_MaxCount = other.m_MaxCount;
		m_Count = other.m_Count;
		return *this;
	}

	Buffer& operator =(const std::initializer_list<T>& list) {
		if(m_MaxCount < list.size()) {
			free(m_Data);
			m_MaxCount = list.size();
			m_Data = (T*)malloc(GetMaxSize());
		}

		m_Count = list.size();
		memcpy(m_Data, list.begin(), GetSize());
		return *this;
	}

	T* Get(u64 i = 0) const {
		VOLCANICORE_ASSERT((i == 0 && !m_Data) || (i < m_MaxCount));
		return m_Data + i;
	}

	u64 GetCount()		const { return m_Count; }
	u64 GetMaxCount()	const { return m_MaxCount; }
	u64 GetSize()		const { return m_Count * sizeof(T); }
	u64 GetMaxSize()	const { return m_MaxCount * sizeof(T); }

	Buffer<T> Copy() const {
		T* newData = (T*)malloc(GetSize());
		memcpy(newData, m_Data, GetSize());
		return Buffer<T>(newData, GetCount());
	}

	void Add(const T& element) {
		if(m_Count >= m_MaxCount)
			return;

		m_Data[m_Count++] = element;
	}
	void Add(const Buffer& buffer) {
		Set(buffer.Get(), buffer.GetCount(), m_Count);
	}

	void Add(const void* data, u64 count) {
		Set(data, count, m_Count);
	}

	void Set(u64 idx, const T& data) {
		Set(&data, 1, idx);
	}

	void Set(const void* data, u64 count, u64 offset = 0) {
		if(offset >= m_MaxCount)
			return;
		if(offset + count >= m_MaxCount)
			count = m_MaxCount - offset;

		memcpy(m_Data + offset, data, count * sizeof(T));

		if(offset + count > m_Count)
			m_Count = offset + count;
	}

	void Add() {
		m_Count++;
		VOLCANICORE_ASSERT(m_Count <= m_MaxCount);
	}

	void AddRange(u64 count) {
		m_Count += count;
		VOLCANICORE_ASSERT(m_Count <= m_MaxCount);
	}

	void Remove() {
		VOLCANICORE_ASSERT(m_Count > 0);
		m_Count--;
	}

	void Clear() {
		m_Count = 0;
	}

	void Delete() {
		free(m_Data);
		m_Data = nullptr;
		m_Count = 0;
		m_MaxCount = 0;
	}

	void Reallocate(u64 additional) {
		Allocate(m_MaxCount + additional);
	}

	void Allocate(u64 count) {
		if(count <= m_MaxCount)
			return;

		m_MaxCount = count;
		T* newData = (T*)malloc(GetMaxSize());
		if(m_Data) {
			memcpy(newData, m_Data, GetSize());
			free(m_Data);
		}

		m_Data = newData;
	}

private:
	T* m_Data = nullptr;
	u64 m_MaxCount = 0;
	u64 m_Count = 0;
};

template<>
class Buffer<void> {
public:
	Buffer() = default;
	Buffer(u64 size, u64 maxCount)
		: m_SizeT(size), m_MaxCount(maxCount)
	{
		VOLCANICORE_ASSERT(size != 0);
		if(maxCount)
			m_Data = malloc(GetMaxSize());
	}
	Buffer(Buffer&& other)
		: m_SizeT(other.m_SizeT),
			m_MaxCount(other.GetMaxCount()), m_Count(other.GetCount())
	{
		std::swap(m_Data, other.m_Data);
	}
	Buffer(void* data, u64 size, u64 count, bool own = true)
		: m_SizeT(size), m_MaxCount(count), m_Count(count)
	{
		if(!m_MaxCount)
			m_MaxCount = count;
		if(own) {
			m_Data = data;
		} else {
			m_Data = malloc(GetMaxSize());
			memcpy(m_Data, data, GetSize());
		}
	}

	~Buffer() {
		Delete();
	}

	operator bool() const { return m_Data && m_Count; }

	Buffer& operator =(const Buffer& other) = delete;

	Buffer& operator =(Buffer&& other) {
		std::swap(m_Data, other.m_Data);
		m_SizeT = other.m_SizeT;
		m_MaxCount = other.m_MaxCount;
		m_Count = other.m_Count;
		return *this;
	}

	void* Get(u64 i = 0) const {
		VOLCANICORE_ASSERT((i == 0 && !m_Data) || (i < m_MaxCount));
		return (char*)m_Data + i;
	}

	u64 GetCount()	   const { return m_Count; }
	u64 GetMaxCount() const { return m_MaxCount; }
	u64 GetSize()	   const { return m_Count	 * m_SizeT; }
	u64 GetMaxSize()  const { return m_MaxCount * m_SizeT; }
	u64 GetSizeT() const { return m_SizeT; }

	Buffer<void> Copy() {
		void* newData = malloc(GetSize());
		memcpy(newData, m_Data, GetSize());
		return Buffer<void>(newData, m_SizeT, GetCount());
	}

	void Add(const Buffer& buffer) {
		if(m_Count + buffer.GetCount() >= m_MaxCount)
			return;

		Set(buffer.Get(), buffer.GetCount(), m_Count);
	}

	void Add(const void* data, u64 count) {
		Set(data, count, m_Count);
	}

	void Set(const void* data, u64 count, u64 offset = 0) {
		if(offset >= m_MaxCount)
			return;
		if(offset + count >= m_MaxCount)
			count = m_MaxCount - offset;

		memcpy((char*)m_Data + offset * m_SizeT, data, count * m_SizeT);

		if(offset + count > m_Count)
			m_Count = offset + count;
	}

	void Clear() {
		m_Count = 0;
	}

	void Delete() {
		free(m_Data);
		m_Data = nullptr;
		m_SizeT = 0;
		m_MaxCount = 0;
		m_Count = 0;
	}

	void Reallocate(u64 surplus) {
		m_MaxCount += surplus;

		void* newData = malloc(m_MaxCount);
		memcpy(newData, m_Data, GetSize());
		free(m_Data);
		m_Data = newData;
	}

private:
	void* m_Data = nullptr;
	u64 m_SizeT = 0;
	u64 m_MaxCount = 0;
	u64 m_Count = 0;
};

}