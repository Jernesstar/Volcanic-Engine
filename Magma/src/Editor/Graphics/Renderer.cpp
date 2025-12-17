#include "Renderer.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcanicWindow/Application.h>

#include "Platform/RendererAPI.h"

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Magma::Graphics {

static Ref<Shader> ScreenShader;
static DrawBuffer* RectBuffer;
static Ref<Shader> RectShader;

void Renderer::Init() {
#if defined(VOLCANIC_APPLE)
	RendererAPI::Create(RendererBackend::Metal);
#else
	RendererAPI::Create(RendererBackend::OpenGL);
#endif

	std::string vertexShaderStr = R"(
		#version 460 core

		layout(location = 0) out vec2 v_TexCoords;

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
			v_TexCoords = Vertices[Indices[gl_VertexID]];
			gl_Position = vec4(v_TexCoords, 0.0, 1.0);
		}
	)";

	std::string fragmentShaderStr = R"(
		#version 460 core

		uniform sampler2D u_ScreenTexture;

		layout(location = 0) in vec2 v_TexCoords;

		layout(location = 0) out vec4 FragColor;

		void main()
		{
			// FragColor = texture(u_ScreenTexture, v_TexCoords);
			FragColor = vec4(v_TexCoords, 1.0, 1.0);
		}
	)";

	ScreenShader = RendererAPI::Get()->CreateShader({ });
	List<ShaderFile> files;
	files.Emplace(ShaderFileType::Vertex, vertexShaderStr);
	files.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
	ScreenShader->SetShaderData(std::move(files));

	RectBuffer =
		RendererAPI::Get()->NewBuffer({
			.InstanceCount = 100,
			.DynamicInstances = true,
			.InstanceLayout = {
				{
					{ "PositionDimension", BufferDataType::Vec4 },
					{ "Color", BufferDataType::Vec4 }
				},
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
			// FragColor = vec4(v_Color);
			FragColor = vec4(v_TexCoords, 1.0, 1.0);
		}
	)";

	RectShader = RendererAPI::Get()->CreateShader({ });
	List<ShaderFile> files2;
	files2.Emplace(ShaderFileType::Vertex, vertexShaderStr);
	files2.Emplace(ShaderFileType::Fragment, fragmentShaderStr);
	RectShader->SetShaderData(std::move(files2));
}

void Renderer::Close() {
	ScreenShader.reset();
	RectShader.reset();
	delete RectBuffer;

	RendererAPI::Shutdown();
}

static DrawCommand* RectCommand = nullptr;

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();

	auto pass = RendererAPI::Get()->NewPass(RectBuffer);
	pass->Pipeline = RectShader;

	RectCommand = RendererAPI::Get()->NewCommand(pass);
	auto window = Application::As<WindowApplication>()->GetWindow();
	RectCommand->Clear = false;
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

	DrawQuad({ 500.0f, 50.0f, 500.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f } });
}

void Renderer::EndFrame() {
	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {
	RectBuffer->Add(DrawBufferIndex::Instance, &quad, 1);
	DrawCall& call = RectCommand->DrawCalls[0];
	call.InstanceCount++;
}

void Renderer::DrawFullscreenQuad(Ref<Framebuffer> fb, u32 attachmentIdx)
{
	if(!fb->Has(Graphics::AttachmentTarget::Color)) {
		VOLCANICORE_LOG_WARNING("Framebuffer has no color attachment");
		return;
	}

	auto att = fb->Get(Graphics::AttachmentTarget::Color, attachmentIdx);

	auto pass = RendererAPI::Get()->NewPass(nullptr);
	pass->Pipeline = ScreenShader;

	auto cmd = RendererAPI::Get()->NewCommand(pass);
	auto window = Application::As<WindowApplication>()->GetWindow();
	cmd->Clear = false;
	cmd->ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	cmd->Viewport = true;
	cmd->ViewportW = window->GetWidth();
	cmd->ViewportH = window->GetHeight();
	cmd->DepthTesting = DepthTestingMode::Off;
	cmd->Blending = BlendingMode::Greatest;
	cmd->Culling = CullingMode::Off;

	cmd->Uniforms.Set("u_ScreenTexture", AttachmentSlot{ att, 0 });

	DrawCall* call = cmd->NewCall();
	call->Primitive = DrawPrimitive::Triangle;
	call->Partition = DrawPartition::Single;
	call->VertexCount = 6;
}

}