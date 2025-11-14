#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <functional>

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
	#define EXPORT __declspec(dllexport)
	#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
	#define EXPORT __attribute__((visibility("default")))
	#define IMPORT
#else
	#define EXPORT
	#define IMPORT
	#pragma warning Unknown dynamic link import/export semantics.
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

template<typename T>
using Ptr = std::unique_ptr<T>;

template<typename T, typename ...Args>
constexpr Ptr<T> CreatePtr(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ...Args>
constexpr Ref<T> CreateRef(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename TKey, typename TValue>
using Map = std::unordered_map<TKey, TValue>;

template<typename TKey, typename TValue>
using OMap = std::map<TKey, TValue>;

template<typename TValue1, typename TValue2>
using Pair = std::pair<TValue1, TValue2>;

template<typename TReturn, typename ...Args>
using Func = std::function<TReturn(Args...)>;