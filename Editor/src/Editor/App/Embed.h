#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <cstdint>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Buffer.h>

class Embed {
public:
	static void Init();
	static void Close();
	static bool IsActive();

	static void SendFrame(VolcaniCore::Buffer<u8> buffer);
	inline static Func<void, const Str&> OnEvent;
};