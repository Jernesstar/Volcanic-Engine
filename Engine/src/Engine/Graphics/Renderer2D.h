#pragma once

#include <VolcaniCore/Core/Math.h>

#include "Platform/RendererAPI.h"
#include "Platform/Texture.h"
#include "Platform/Framebuffer.h"

#include "Camera.h"
#include "Quad.h"
#include "Text.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

class Renderer2D {
public:
	static void StartFrame();
	static void EndFrame();
	static DrawBuffer* GetScreenBuffer();

	static void Begin(Ref<OrthographicCamera> camera);
	static void End();

	static void DrawQuad(Ref<Quad> texture, const Transform& t = { });
	static void DrawQuad(Ref<Texture> quad, const Transform& t = { });
	static void DrawQuad(const glm::vec4& color, const Transform& t = { });

	static void DrawText(Ref<Text> text, const Transform& t = { });

	static void DrawFullscreenQuad(Ref<Framebuffer> buffer,
		AttachmentTarget target = AttachmentTarget::Color);

private:
	static void Init();
	static void Close();

	friend class Renderer;
};

}