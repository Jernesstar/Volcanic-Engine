#include "Renderer2D.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>

#include "Platform/RendererAPI.h"

#include "Graphics/Renderer.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

static DrawBuffer* s_ScreenBuffer;

void Renderer2D::Init() {
	float screenCoords[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	s_ScreenBuffer =
		RendererAPI::Get()->NewBuffer({
			.VertexCount = 6,
			.DynamicVertices = false,
			.InstanceCount = 100,
			.DynamicInstances = true,
			.VertexLayout =
			{
				{
					{ "Color", BufferDataType::Vec4 }
				},
				false
			},
			.InstanceLayout =
			{
				{
					{ "PositionDimension", BufferDataType::Vec4 },
					{ "Color", BufferDataType::Vec4 }
				},
				true // Instanced
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
	DrawQuad(Quad::Create(1, 1, color), t);
}

void Renderer2D::DrawQuad(Ref<Texture> texture, const Transform& t) {
	DrawQuad(Quad::Create(texture), t);
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
		// auto pipeline = ShaderLibrary::Get("Framebuffer");
		// auto* pass = RendererAPI::Get()->NewDrawPass(s_ScreenBuffer, pipeline);
		// command = RendererAPI::Get()->NewDrawCommand(pass);
	}

	// auto window = Application::GetWindow();
	// command->ViewportW = window->GetWidth();
	// command->ViewportH = window->GetHeight();
	// command->DepthTest = DepthTestingMode::Off;
	// command->Culling = CullingMode::Off;
	// command->Blending = BlendingMode::Greatest;
	// command->UniformData
	// .SetInput("u_ScreenTexture", TextureSlot{ buffer->Get(target), 0 });

	// auto& call = command->NewDrawCall();
	// call.VertexCount = 6;
	// call.Primitive = PrimitiveType::Triangle;
	// call.Partition = PartitionType::Single;
}

}