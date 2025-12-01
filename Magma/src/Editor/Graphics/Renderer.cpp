#include "Renderer.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcanicWindow/Application.h>

#include "Platform/RendererAPI.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Magma::Graphics {

static DrawBuffer* ScreenBuffer;
static DrawPass* ScreenPass;

static DrawBuffer* RectBuffer;
static DrawPass* RectPass;

void Renderer::Init() {
#if defined(VOLCANIC_APPLE)
	RendererAPI::Create(RendererBackend::Metal);
#else
	RendererAPI::Create(RendererBackend::OpenGL);
#endif

	ScreenBuffer =
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

	ScreenBuffer->Add(DrawBufferIndex::Vertex, screenCoords, 6);

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

	Ref<Shader> shader;

	shader = RendererAPI::Get()->CreateShader({ });
	List<ShaderFile> files;
	files.Emplace(ShaderFileType::Vertex, vertexShaderStr);
	files.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
	shader->SetShaderData(std::move(files));

	ScreenPass = RendererAPI::Get()->NewPass(ScreenBuffer);
	ScreenPass->Pipeline = shader;
	ScreenPass->Output = nullptr;

	RectBuffer =
		RendererAPI::Get()->NewBuffer({
			.InstanceCount = 100,
			.DynamicInstances = true,
			.InstanceLayout = {
				{
					{ "PositionDimension", BufferDataType::Vec4 },
					{ "Color", BufferDataType::Vec4 }
				},
				true, // Dynamic
				true // Instanced
			}
		});

	vertexShaderStr = R"(
		#version 460 core

		layout(location = 1) uniform vec2 u_ScreenSize;

		layout(location = 0) in vec4 a_PositionDimension;
		layout(location = 1) in vec4 a_Color;

		layout(location = 0) out vec2 v_TexCoords;
		layout(location = 1) out vec4 v_Color;

		const vec2 Vertices[4] =
			vec2[4](
				vec2(-1.0f, -1.0f),
				vec2( 1.0f, -1.0f),
				vec2( 1.0f,  1.0f),
				vec2(-1.0f,  1.0f)
			);
		const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

		void main()
		{
			vec2 pos = 2.0 * (a_PositionDimension.xy / u_ScreenSize.xy) - 1.0;
			vec2 dim = a_PositionDimension.zw / u_ScreenSize.xy;
			vec2 vertex = Vertices[Indices[gl_VertexID]];

			vec2 finalPos = pos + vertex * dim;
			gl_Position = vec4(finalPos, 0.0, 1.0);

			v_TexCoords = (vertex + 1.0) / 2.0;
			v_Color = a_Color;
		}
	)";

	fragmentShaderStr = R"(
		#version 460 core

		layout(location = 0) in vec2 v_TexCoords;
		layout(location = 1) in vec4 v_Color;

		layout(location = 0) out vec4 FragColor;

		void main()
		{
			FragColor = vec4(v_Color);
		}
	)";

	shader = RendererAPI::Get()->CreateShader({ });
	List<ShaderFile> files2;
	files2.Emplace(ShaderFileType::Vertex, vertexShaderStr);
	files2.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
	shader->SetShaderData(std::move(files2));

	RectPass = RendererAPI::Get()->NewPass(RectBuffer);
	RectPass->Pipeline = shader;
	RectPass->Output = nullptr;
}

void Renderer::Close() {
	RendererAPI::Shutdown();
}

static DrawCommand* RectCommand = nullptr;

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();

	RectCommand = RendererAPI::Get()->NewCommand(RectPass);
	auto window = Application::As<WindowApplication>()->GetWindow();
	RectCommand->Clear = true;
	RectCommand->ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	RectCommand->Viewport = true;
	RectCommand->ViewportW = window->GetWidth();
	RectCommand->ViewportH = window->GetHeight();
	RectCommand->DepthTesting = DepthTestingMode::Off;
	RectCommand->Blending = BlendingMode::Greatest;
	RectCommand->Culling = CullingMode::Off;

	RectCommand->Uniforms
	.Set("u_ScreenSize", Vec2(window->GetWidth(), window->GetHeight()));

	DrawCall* call = RectCommand->NewCall();
	call->Primitive = DrawPrimitive::Triangle;
	call->Partition = DrawPartition::Instanced;
	call->VertexCount = 6;
}

void Renderer::EndFrame() {
	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {
	RectBuffer->Add(DrawBufferIndex::Instance, &quad, 1);
	DrawCall& call = RectCommand->DrawCalls[0];
	call.InstanceCount++;
}

}