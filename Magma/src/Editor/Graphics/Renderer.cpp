#include "Renderer.h"

#include "Platform/RendererAPI.h"

namespace Magma::Graphics {

static DrawBufferID BaseBuffer;
static DrawPassID BasePass;

void Renderer::Init() {
#if defined(VOLCANIC_APPLE)
	RendererAPI::Create(RendererAPI::Backend::Metal);
#else
	RendererAPI::Create(RendererAPI::Backend::OpenGL);
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

	const char* vertexShaderStr = R"(
		#version 460 core

		layout(location = 0) in vec2 a_TexCoords;
		layout(location = 0) out vec2 v_TexCoords;

		void main()
		{
			v_TexCoords = a_TexCoords;

			gl_Position = vec4(2.0 * a_TexCoords - 1.0, 0.0, 1.0);
		}
	)";

	const char* fragmentShaderStr = R"(
		#version 460 core

		layout(location = 0) uniform sampler2D u_ScreenTexture;

		layout(location = 0) in vec2 v_TexCoords;

		layout(location = 0) out vec4 FragColor;

		void main()
		{
			FragColor = vec4(texture(u_ScreenTexture, v_TexCoords).rgb, 1.0);
		}
	)";

	Buffer<void> vertexShader;
	Buffer<void> fragmentShader;
	fragmentShader.Set(fragmentShaderStr, strlen(fragmentShaderStr));

	Ref<Shader> shader =
		RendererAPI::Get()->CreateShader({
			.Files = {
				{
					ShaderFileType::Vertex,
					ShaderDataType::Text,
					Buffer<void>((void*)vertexShaderStr, sizeof(char), strlen(vertexShaderStr))
				},
				{
					ShaderFileType::Fragment,
					ShaderDataType::Text,
					Buffer<void>((void*)vertexShaderStr, sizeof(char), strlen(vertexShaderStr))
				}
			}
		});

	BasePass =
		RendererAPI::Get()->NewPass({
			.Buffer = BaseBuffer,
			.Output = nullptr,
			.Pipeline = shader
		});
}

void Renderer::Close() {
	RendererAPI::Shutdown();
}

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();

}

void Renderer::EndFrame() {

	RendererAPI::Get()->EndFrame();
}

void Renderer::DrawQuad(const Quad& quad) {
	auto cmd =
		RendererAPI::Get()->NewCommand({
			.Pass = BasePass,
			.Clear = true,
			// .ClearColor = {
			// 	quad.Color.r,
			// 	quad.Color.g,
			// 	quad.Color.b,
			// 	quad.Color.a
			// },
		});
}

}