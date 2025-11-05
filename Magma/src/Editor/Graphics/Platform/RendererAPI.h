#pragma once

#include <VolcaniCore/Core/Defines.h>

namespace Magma::Graphics {

struct DebugInfo {
	uint64_t DrawCallCount = 0;
	uint64_t IndexCount    = 0;
	uint64_t VertexCount   = 0;
	uint64_t InstanceCount = 0;
};

class RendererAPI {
public:
	enum class Backend { OpenGL, Metal };

public:
	static void Create(RendererAPI::Backend backend);
	static void Shutdown();

	static Ref<RendererAPI> Get() { return s_Instance; }

public:
	RendererAPI(RendererAPI::Backend backend)
		: m_Backend(backend) { }
	virtual ~RendererAPI() = default;

	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;
	virtual DebugInfo GetDebugInfo() = 0;

	RendererAPI::Backend GetBackend() const { return m_Backend; }

protected:
	const RendererAPI::Backend m_Backend;

protected:
	virtual void Init() = 0;
	virtual void Close() = 0;

private:
	inline static Ref<RendererAPI> s_Instance;
};

}