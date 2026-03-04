#pragma once

#include "Buffer.h"

namespace VolcaniCore {

struct SearchResult {
	bool Found;
	u64 Index;

	operator bool() const { return Found; }
};

template<typename T>
class List {
public:
	List() = default;
	List(u64 size)
		: m_Buffer(size) { }
	List(const std::initializer_list<T>& list)
		: m_Buffer(list.size())
	{
		for(auto& element : list)
			Add(element);
	}
	List(List&& other)
		: m_Buffer(other.m_Buffer.GetMaxCount())
	{
		for(auto& val : other) {
			if constexpr(std::is_copy_constructible<T>())
				Add(val);
			else
				AddMove(std::forward<T&&>(val));
		}
	}
	List(const List& other)
		: m_Buffer(other.m_Buffer.GetMaxCount())
	{
		for(auto& val : other)
			Add(val);
	}

	~List() {
		Clear();
		m_Buffer.Delete();
	}

	List& operator =(const std::initializer_list<T>& list) {
		Clear();
		m_Buffer.Delete();
		m_Buffer = Buffer<T>(list.size());

		for(auto& val : list)
			Add(val);

		return *this;
	}

	List& operator =(List&& other) {
		Clear();
		m_Buffer.Delete();
		m_Buffer = Buffer<T>(other.m_Buffer.GetMaxCount());

		for(auto& val : other)
			Add(val);

		return *this;
	}

	List& operator =(const List& other) {
		Clear();
		m_Buffer.Delete();
		m_Buffer = Buffer<T>(other.m_Buffer.GetMaxCount());

		for(auto& val : other)
			Add(val);

		return *this;
	}

	operator bool() const { return Count(); }

	u64 Count() const { return m_Back - m_Front; }

	T& operator[](i64 idx) { return *At(idx); }
	const T& operator[](i64 idx) const { return *At(idx); }

	T* At(i64 idx) const {
		VOLCANICORE_ASSERT(idx >= -(i64)m_Back || idx < (i64)Count());
		auto abs = Absolute(idx);
		return m_Buffer.Get(abs);
	}

	template<typename ...Args>
	T& Emplace(Args&&... args) {
		return EmplaceAt(-1, std::forward<Args>(args)...);
	}

	template<typename ...Args>
	T& EmplaceAt(i64 idx, Args&&... args) {
		Free(idx);
		if constexpr(std::is_aggregate<T>())
			new (At(idx)) T{ std::forward<Args>(args)... };
		else
			new (At(idx)) T(std::forward<Args>(args)...);

		return *At(idx);
	}

	void AddMove(T&& element) {
		Inplace(-1, std::forward<T&&>(element));
	}

	void Add(const T& element) {
		Insert(-1, element);
	}

	void Queue(const T& element) {
		Insert(0, element);
	}

	void Push(const T& element) {
		Insert(-1, element);
	}

	T Pop() {
		return Pop(-1);
	}

	T PopFront() {
		return Pop(0);
	}

	T Pop(i64 idx) {
		VOLCANICORE_ASSERT(Count());
		T val = *At(idx);
		Remove(idx);
		auto abs = Absolute(idx);

		if(abs == m_Front)
			m_Front++;
		else if(abs == m_Back - 1)
			m_Back--;
		else
			ShiftRight(m_Front++, abs - 1, 1);

		if(m_Front == m_Back)
			m_Front = m_Back = 0;

		return val;
	}

	void Inplace(i64 idx, T&& element) {
		Free(idx);
		new (At(idx)) T(std::move(element));
	}

	void Insert(i64 idx, const T& element) {
		Free(idx);
		new (At(idx)) T(element);
	}

	template<class TPredicate>
	void ForEach(TPredicate&& func) {
		for(auto& val : *this)
			func(val);
	}

	template<typename TOut>
	List<TOut> Apply(const Func<TOut, const T&>& func) {
		List<TOut> out;
		for(auto& val : *this)
			out.Add(func(val));
		return out;
	}

	SearchResult Find(const T& target) {
		for(u64 i = 0; i < Count(); i++)
			if(target == *At(i))
				return { true, i };
		return { false, 0 };
	}

	SearchResult Find(const Func<bool, const T&>& func) const {
		for(u64 i = 0; i < Count(); i++)
			if(func(*At(i)))
				return { true, i };
		return { false, 0 };
	}

	SearchResult FindLast(const Func<bool, const T&>& func) const {
		for(u64 i = Count() - 1; i <= 0; i--)
			if(func(*At(i)))
				return { true, i };
		return { false, 0 };
	}

	void Reallocate(u64 additional) {
		Allocate(m_Buffer.GetMaxCount() + additional);
	}

	void Allocate(u64 maxCount) {
		if(maxCount <= m_Buffer.GetMaxCount())
			return;

		T* newData = (T*)malloc(maxCount * sizeof(T));
		for(u64 i = 0; i < Count(); i++) {
			Place(At(i), newData + i);
			Remove(i);
		}

		m_Back -= m_Front;
		m_Front = 0;
		m_Buffer.Delete();
		m_Buffer = Buffer<T>(newData, m_Back, maxCount);
	}

	void Clear() {
		for(u64 i = 0; i < Count(); i++)
			Remove(i);

		m_Front = 0;
		m_Back = 0;
	}

	const Buffer<T>& GetBuffer() const { return m_Buffer; }

	using iterator = T*;
	using const_iterator = const T*;
	iterator begin() { return m_Buffer.Get() + m_Front; }
	iterator end() { return m_Buffer.Get() + m_Back; }
	const_iterator cbegin() const { return m_Buffer.Get() + m_Front; }
	const_iterator cend()	const { return m_Buffer.Get() + m_Back; }
	const_iterator begin()	const { return cbegin(); }
	const_iterator end()	const { return cend(); }

private:
	Buffer<T> m_Buffer;
	u64 m_Front = 0, m_Back = 0;

private:
	u64 Absolute(i64 idx) const {
		if(idx < 0)
			return (u64)((i64)m_Back + idx);
		return m_Front + (u64)idx;
	}

	void Remove(i64 idx) {
		m_Buffer.Remove();
		At(idx)->~T();
	}

	void Place(T* src, T* dst) {
		if constexpr(std::is_copy_constructible<T>())
			new (dst) T(*src);
		else
			new (dst) T(std::move(*src));
	}

	void ShiftLeft(u64 beg, u64 end, u64 dx) {
		for(i64 i = (i64)beg; i <= (i64)end; i++) {
			Place(m_Buffer.Get((u64)i), m_Buffer.Get((u64)i - dx));
			m_Buffer.Get((u64)i)->~T();
		}
	}

	void ShiftRight(u64 beg, u64 end, u64 dx) {
		for(i64 i = (i64)end; i >= (i64)beg; i--) {
			Place(m_Buffer.Get((u64)i), m_Buffer.Get((u64)i + dx));
			m_Buffer.Get((u64)i)->~T();
		}
	}

	void Free(i64 idx) {
		if(!m_Buffer.GetMaxCount()) {
			m_Buffer = Buffer<T>(5);
			m_Buffer.Add();
			m_Back = 1;
			m_Front = 0;
			return;
		}

		if(Count() == m_Buffer.GetMaxCount()) {
			auto newMax = m_Buffer.GetMaxCount() + 11;
			T* newData = (T*)malloc(newMax * sizeof(T));

			u64 pos;
			if(idx < 0)
				pos = (u64)((i64)m_Back + 1 + idx);
			else
				pos = (u64)idx;

			u64 delta = 0;
			for(u64 i = 0; i < Count(); i++) {
				if(i == pos)
					delta = 1;

				Place(At(i), newData + i + delta);
				Remove(i);
			}

			m_Buffer.Delete();
			m_Buffer = Buffer<T>(newData, ++m_Back, newMax);
			return;
		}

		m_Buffer.Add();

		if(!Count()) {
			m_Back++;
			return;
		}

		auto abs = Absolute(idx);
		if(abs == m_Back - 1) {
			if(m_Back != m_Buffer.GetMaxCount())
				m_Back++;
			else
				ShiftLeft((m_Front--), m_Back - 1, 1);
		}
		else if(abs == 0) {
			if(m_Front != 0)
				m_Front--;
			else
				ShiftRight(0, (m_Back++) - 1, 1);
		}
		else
			ShiftRight(abs, (m_Back++) - 1, 1);
	}
};

}