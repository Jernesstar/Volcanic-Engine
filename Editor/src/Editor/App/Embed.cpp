#include "Embed.h"

#include <stdexcept>
#include <string>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

#ifdef _WIN32
  static HANDLE s_FramePipe = INVALID_HANDLE_VALUE;
  static HANDLE s_InputPipe = INVALID_HANDLE_VALUE;
#else
  static i32 s_FrameFd = -1;
  static i32 s_InputFd = -1;
#endif

static void WriteFrame(const u8* data, u32 size);
static void InputLoop();

static std::atomic<bool> s_Running = true;
static std::thread s_InputThread;

void Embed::Init() {
#ifdef _WIN32
	s_FramePipe = CreateNamedPipeA(
		"\\\\.\\pipe\\volcanic_frames",
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1, FRAME_W * FRAME_H * 4 + 4, 0, 0, nullptr
	);
	if(s_FramePipe == INVALID_HANDLE_VALUE)
		throw std::runtime_error("CreateNamedPipe (frames) failed");

	s_InputPipe = CreateNamedPipeA(
		"\\\\.\\pipe\\volcanic_input",
		PIPE_ACCESS_INBOUND,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1, 0, 4096, 0, nullptr
	);
	if(s_InputPipe == INVALID_HANDLE_VALUE)
		throw std::runtime_error("CreateNamedPipe (input) failed");

	ConnectNamedPipe(s_FramePipe, nullptr);
	ConnectNamedPipe(s_InputPipe, nullptr);
#else
	auto MakeFifo = [](const char* path) { mkfifo(path, 0666); };
	MakeFifo("/tmp/volcanic_frames");
	MakeFifo("/tmp/volcanic_input");

	s_FrameFd = open("/tmp/volcanic_frames", O_WRONLY);
	if(s_FrameFd < 0)
		throw std::runtime_error("open volcanic_frames failed");

	s_InputFd = open("/tmp/volcanic_input", O_RDONLY);
	if(s_InputFd < 0)
		throw std::runtime_error("open volcanic_input failed");
#endif

	s_InputThread = std::thread(InputLoop);
}

void Embed::Close() {
	s_Running = false;
	if(s_InputThread.joinable())
		s_InputThread.join();

#ifdef _WIN32
	CloseHandle(s_FramePipe);
	CloseHandle(s_InputPipe);
#else
	close(s_FrameFd);
	close(s_InputFd);
#endif
}

bool Embed::IsActive() {
	return s_Running;
}

void Embed::SendFrame(VolcaniCore::Buffer<u8> buffer) {
	WriteFrame(buffer.Get(), (u32)buffer.GetSize());
}

void WriteFrame(const u8* data, u32 size) {
#ifdef _WIN32
	DWORD written;
	WriteFile(s_FramePipe, &size, sizeof(size), &written, nullptr);
	WriteFile(s_FramePipe, data,  size,         &written, nullptr);
#else
	write(s_FrameFd, &size, sizeof(size));
	write(s_FrameFd, data,  size);
#endif
}

void InputLoop() {
	while(s_Running) {
		uint32_t len = 0;

#ifdef _WIN32
		DWORD bytesRead;
		if(!ReadFile(s_InputPipe, &len, sizeof(len), &bytesRead, nullptr) || bytesRead != sizeof(len))
			break;
#else
		if(read(s_InputFd, &len, sizeof(len)) != sizeof(len))
			break;
#endif

		if(len == 0 || len > 65536)
			continue;
		std::string json(len, '\0');

#ifdef _WIN32
		if(!ReadFile(s_InputPipe, json.data(), len, &bytesRead, nullptr))
			break;
#else
		if(read(s_InputFd, json.data(), len) != (ssize_t)len)
			break;
#endif

		Embed::OnEvent(json);
	}
}