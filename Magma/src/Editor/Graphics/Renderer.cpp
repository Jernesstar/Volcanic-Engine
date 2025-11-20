#include "Renderer.h"

#include "Platform/RendererAPI.h"

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
			.IndexCount = 100,
			.VertexCount = 1000,
			.InstanceCount = 1000,
			.Vertex = {
				{ "Position", BufferDataType::Vec3 },
				{ "TexCoords", BufferDataType::Vec2 },
				{ "Color", BufferDataType::Vec4 },
				{ "Texture", BufferDataType::Int },
				{ "Normal", BufferDataType::Vec3 },
			},
			.Instance = {
				{ "Position", BufferDataType::Vec3 },
			}
		});

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

		layout(location = 0) uniform sampler2D u_ScreenTexture;

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
}

void Renderer::EndFrame() {

	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {
	Buffer<void> data(sizeof(Vec2), 4);
	data.Set(&quad.PosX, 2);
	BaseBuffer->SetData(DrawBufferIndex::Vertex, std::move(data));

	DrawCommand* cmd = RendererAPI::Get()->NewCommand(BasePass);
	cmd->Clear = true;
	cmd->Viewport = true;
	cmd->ViewportX = 0;
	cmd->ViewportY = 0;
	cmd->ViewportW = 1280;
	cmd->ViewportH = 720;

	DrawCall* call = cmd->NewCall();

	call->Primitive = DrawPrimitive::Triangle;
	call->VertexOffset = 0;
	call->VertexCount = 4;
	call->IndexOffset = 0;
	call->IndexCount = 6;
}

}