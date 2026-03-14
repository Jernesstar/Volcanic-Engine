#include "Renderer2D.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>

#include "Platform/RendererAPI.h"
#include "Renderer.h"

#include <Asset/AssetManager.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

static DrawBuffer* s_ScreenBuffer;

void Renderer2D::Init() {
	f32 screenCoords[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	s_ScreenBuffer =
		RendererAPI::Get()->NewBuffer(
		{
			.VertexCount = 6,
			.DynamicVertices = false,
			.VertexLayout =
			{
				{
					{ "Position", BufferDataType::Vec2 }
				},
				false
			}
		});

	s_ScreenBuffer->Add(DrawBufferIndex::E_Vertex, screenCoords, 6);
}

void Renderer2D::Close() {

}

void Renderer2D::StartFrame() {

}

void Renderer2D::EndFrame() {

}

DrawBuffer* Renderer2D::GetScreenBuffer() {
	return s_ScreenBuffer;
}

void Renderer2D::Begin(Ref<OrthographicCamera> camera) {

}

void Renderer2D::End() {

}

void Renderer2D::DrawQuad(Ref<Quad> quad, const Transform& t) {

}

void Renderer2D::DrawQuad(const glm::vec4& color, const Transform& t) {

}

void Renderer2D::DrawQuad(Ref<Texture> texture, const Transform& t) {

}

void Renderer2D::DrawText(Ref<Text> text, const Transform& t) {

}

void Renderer2D::DrawFullscreenQuad(Ref<Framebuffer> buffer,
									AttachmentTarget target)
{
	if(!buffer) {
		Log::Warning("Framebuffer is null");
		return;
	}
	if(!buffer->Has(target)) {
		Log::Warning("Framebuffer does not have needed attachment");
		return;
	}

	DrawCommand* command;
	if(Renderer::GetPass())
		command = Renderer::NewCommand(true);
	else {
		auto* pass = RendererAPI::Get()->NewPass(s_ScreenBuffer);
		pass->Pipeline = AssetManager::Get()->Get<Shader>("FullscreenQuad");
		command = RendererAPI::Get()->NewCommand(pass);
	}

	auto window = Application::GetWindow();
	command->ViewportW = window->GetWidth();
	command->ViewportH = window->GetHeight();
	command->DepthTesting = DepthTestingMode::Off;
	command->Blending = BlendingMode::Greatest;
	command->Culling = CullingMode::Off;
	command->Uniforms
	.Set("u_ScreenTexture", AttachmentSlot{ buffer->Get(target), 0 });

	auto* call = command->NewCall();
	call->VertexCount = 6;
	call->Primitive = DrawPrimitive::Triangle;
	call->Partition = DrawPartition::Single;
}

}
