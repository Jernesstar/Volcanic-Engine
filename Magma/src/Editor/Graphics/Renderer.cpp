#include "Renderer.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcanicWindow/Application.h>

#include "Platform/RendererAPI.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Magma::Graphics {

static DrawBuffer* BaseBuffer;
static DrawPass* BasePass;
static Ref<Shader> BaseShader;

void Renderer::Init() {
#if defined(VOLCANIC_APPLE)
	RendererAPI::Create(RendererBackend::Metal);
#else
	RendererAPI::Create(RendererBackend::OpenGL);
#endif

	BaseBuffer =
		RendererAPI::Get()->NewBuffer({
			.VertexCount = 6,
			.DynamicVertices = false,
			.VertexLayout = {
				{
					{ "TexCoords", BufferDataType::Vec2 }
				},
				false, // Dynamic
				false // Instanced
			}
		});

	float screenCoords[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	Buffer<void> buffer(sizeof(Vec2), 6);
	buffer.Set(&screenCoords[0], 6);
	BaseBuffer->SetData(DrawBufferIndex::Vertex, std::move(buffer));

	std::string vertexShaderStr = R"(
		#version 460 core

		layout(location = 0) in vec2 a_TexCoords;
		layout(location = 0) out vec2 v_TexCoords;

		void main()
		{
			v_TexCoords = a_TexCoords;

			gl_Position = vec4(2.0 * a_TexCoords - 1.0, 0.0, 1.0);
		}
	)";

	std::string fragmentShaderStr = R"(
		#version 460 core

		layout(location = 0) in vec2 v_TexCoords;

		layout(location = 0) out vec4 FragColor;

		void main()
		{
			// FragColor = vec4(texture(u_ScreenTexture, v_TexCoords).rgb, 1.0);
			FragColor = vec4(v_TexCoords, 1.0, 1.0);
		}
	)";
	
	BaseShader = RendererAPI::Get()->CreateShader({ });

	List<ShaderFile> files;
	files.Emplace(ShaderFileType::Vertex, vertexShaderStr);
	files.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
	BaseShader->SetShaderData(std::move(files));
}

void Renderer::Close() {
	RendererAPI::Shutdown();
}

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();

	BasePass = RendererAPI::Get()->NewPass(BaseBuffer);
	BasePass->Pipeline = BaseShader;
	BasePass->Output = nullptr;

	DrawCommand* cmd = RendererAPI::Get()->NewCommand(BasePass);
	auto window = Application::As<WindowApplication>()->GetWindow();
	cmd->Clear = true;
	cmd->ClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	cmd->Viewport = true;
	cmd->ViewportW = window->GetWidth();
	cmd->ViewportH = window->GetHeight();
	cmd->DepthTesting = DepthTestingMode::Off;
	cmd->Blending = BlendingMode::Greatest;
	cmd->Culling = CullingMode::Off;

	DrawCall* call = cmd->NewCall();
	call->Primitive = DrawPrimitive::Triangle;
	call->Partition = DrawPartition::Single;
	call->VertexCount = 6;
}

void Renderer::EndFrame() {

	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {

}

}