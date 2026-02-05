#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Math.h>

#include <Platform/RendererAPI.h>

using namespace VolcaniCore;

namespace Magma::Graphics {

struct Quad {
	f32 PosX, PosY, Width, Height;
	Vec4 Color;
};

class Renderer {
public:
	static void Init();
	static void Close();
	static void BeginFrame();
	static void EndFrame();

	static void DrawQuad(const Quad&);
	static void DrawFullscreenQuad(Ref<Framebuffer> fb, u32 attachmentIdx = 0);
};

}