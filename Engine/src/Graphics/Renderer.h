#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "RenderPass.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct FrameDebugInfo {
	f32 FPS;

	u64 DrawCalls = 0;
	u64 Indices   = 0;
	u64 Vertices  = 0;
	u64 Instances = 0;
};

struct FrameData {
	FrameDebugInfo Info;
};

class Renderer {
public:
	static const u64 MaxTriangles;
	static const u64 MaxIndices;
	static const u64 MaxVertices;
	static const u64 MaxInstances;

public:
	static void Init();
	static void Close();

	static void BeginFrame();
	static void EndFrame();

	static FrameData& GetFrame();

	static void StartPass(Ref<RenderPass> pass, bool pushCommand = true);
	static void EndPass();
	static Ref<RenderPass> GetPass();

	static DrawCommand* PushCommand();
	static void PopCommand();
	static DrawCommand* GetCommand();
	static DrawCommand* NewCommand(bool usePrevious = false);

	static void Clear();
	static void Resize(u32 width, u32 height);

	static void PushOptions();
	static void PopOptions(u32 count = 1);

	static void Flush();

	static FrameDebugInfo GetDebugInfo();
};

}