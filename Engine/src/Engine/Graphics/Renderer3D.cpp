#include "Renderer3D.h"

#include <glm/gtc/type_ptr.hpp>

#include <VolcaniCore/Core/Assert.h>

#include "Renderer.h"
#include "Platform/RendererAPI.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

static DrawBuffer* s_MeshBuffer;
static DrawBuffer* s_LineBuffer;
static DrawBuffer* s_CubemapBuffer;

static Map<SubGeometry*, DrawCommand*> s_Geometries;
static u64 s_InstancesIndex = 0;

void Renderer3D::Init() {
	BufferLayout vertexLayout =
	{
		{
			{ "Position", BufferDataType::Vec3 },
			{ "Normal",	  BufferDataType::Vec3 },
			{ "TexCoord", BufferDataType::Vec2 },
		},
		false // Structure of arrays
	};

	BufferLayout instanceLayout =
	{
		{
			{ "Transform", BufferDataType::Mat4 }
		},
		true  // Structure of arrays
	};

	DrawBufferSpec specs =
	{
		.IndexCount = Renderer::MaxIndices,
		.VertexCount = Renderer::MaxVertices,
		.InstanceCount = Renderer::MaxInstances,
		.VertexLayout = vertexLayout,
		.InstanceLayout = instanceLayout,
	};

	BufferLayout lineLayout =
	{
		{
			{ "Position", BufferDataType::Vec3 },
			{ "Color",    BufferDataType::Vec3 },
		},
		false // Instanced
	};

	DrawBufferSpec lineSpecs =
	{
		.IndexCount = 2'000'000,
		.VertexCount = 1'000'000,
		.VertexLayout = lineLayout
	};

	f32 cubemapVertices[] =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	BufferLayout cubemapLayout =
	{
		{
			{ "Position", BufferDataType::Vec3 },
		},
		false // Instanced
	};
	DrawBufferSpec specsCubemap =
	{
		.VertexCount = 36,
		.VertexLayout = cubemapLayout,
	};

	s_MeshBuffer = RendererAPI::Get()->NewBuffer(specs);
	s_LineBuffer = RendererAPI::Get()->NewBuffer(lineSpecs);
	s_CubemapBuffer = RendererAPI::Get()->NewBuffer(specsCubemap);
	s_CubemapBuffer->Add(DrawBufferIndex::E_Vertex, cubemapVertices, 36);
}

void Renderer3D::Close() {
	delete s_MeshBuffer;
	delete s_LineBuffer;
	delete s_CubemapBuffer;
}

DrawBuffer* Renderer3D::GetMeshBuffer() {
	return s_MeshBuffer;
}

DrawBuffer* Renderer3D::GetLineBuffer() {
	return s_LineBuffer;
}

DrawBuffer* Renderer3D::GetCubemapBuffer() {
	return s_CubemapBuffer;
}

void Renderer3D::StartFrame() {
	s_MeshBuffer->Clear();
	s_LineBuffer->Clear();
}

void Renderer3D::EndFrame() {
	s_InstancesIndex = 0;
}

void Renderer3D::Begin(Ref<Camera> camera) {
	if(!camera)
		return;

	auto* command = Renderer::GetCommand();
	command->Uniforms
	.Set("u_ViewProj", camera->GetViewProjection())
	.Set("u_CameraPosition", camera->GetPosition());
}

void Renderer3D::End() {
	s_Geometries.clear();
}

void Renderer3D::DrawSkybox(Ref<Cubemap> cubemap) {
	auto* command = Renderer::NewCommand();
	auto* call = command->NewCall();

	// call->DepthMask = false;
	call->Partition = DrawPartition::Single;
	call->VertexCount = 36;

	command->Uniforms
	.Set("u_Skybox", CubemapSlot{ cubemap, 0 });
}

static void DrawSubGeometry(Ref<Geometry> root, SubGeometry& sub,
	const glm::mat4& tr, DrawCommand* cmd)
{
	DrawCommand* command;
	if(s_Geometries.count(&sub))
		command = s_Geometries[&sub];
	else {
		if(Renderer::GetPass()) {
			command = Renderer::NewCommand();
			s_Geometries[&sub] = command;
		}
		else{
			command = RendererAPI::Get()->NewCommand(cmd->Pass);
			command->Uniforms = cmd->Uniforms;
		}
	}

	auto* buffer = command->Pass->Buffer;

	if(!command->VerticesIndex) { // First time seeing this geometry this frame
		if(cmd) {
			command->IndicesIndex = s_MeshBuffer->GetIndexCount();
			command->VerticesIndex = s_MeshBuffer->GetVertexCount();
		}

		command->DepthTesting = DepthTestingMode::On;
		command->Blending = BlendingMode::Off;
		command->Culling = CullingMode::Back;

		command->IndicesIndex = s_MeshBuffer->GetIndexCount();
		command->VerticesIndex = s_MeshBuffer->GetVertexCount();
		buffer->Add(DrawBufferIndex::E_Index, sub.Indices.Get(), sub.Indices.GetCount());
		buffer->Add(DrawBufferIndex::E_Vertex, sub.Vertices.Get(), sub.Vertices.GetCount());
	}

	if(!command->DrawCalls || command->DrawCalls[-1].InstanceCount >= 10'000) {
		auto* call = command->NewCall();
		call->Partition = DrawPartition::Instanced;
		call->Primitive = DrawPrimitive::Line;
		call->InstanceOffset = s_InstancesIndex;
		s_InstancesIndex += 10'000;
	}

	auto& call = command->DrawCalls[-1];
	call.InstanceCount++;
	buffer->Add(DrawBufferIndex::E_Instance, glm::value_ptr(tr), 1);
}

void Renderer3D::DrawGeometry(Ref<Geometry> geometry, const glm::mat4& tr,
						  DrawCommand* command)
{
	for(auto& sub : geometry->Surfaces)
		DrawSubGeometry(geometry, sub, tr, command);
}

void Renderer3D::DrawQuad(Ref<Quad> quad, const glm::mat4& tr,
						  DrawCommand* command)
{
}

void Renderer3D::DrawQuad(Ref<Texture> texture, const glm::mat4& tr,
						  DrawCommand* command)
{
}

void Renderer3D::DrawQuad(const glm::vec4& color, const glm::mat4& tr,
						  DrawCommand* command)
{
}

void Renderer3D::DrawPoint(const Point& point, const glm::mat4& tr,
						   DrawCommand* command)
{
	// TODO(Implement):
}

void Renderer3D::DrawLine(const Line& line, const glm::mat4& tr,
						  DrawCommand* comand)
{
	// RendererAPI::Get()
	// ->SetBufferData(s_LineBuffer, DrawBufferIndex::Vertices, &line.P0,
	// 				1, s_LineBuffer->VerticesCount);
	// RendererAPI::Get()
	// ->SetBufferData(s_LineBuffer, DrawBufferIndex::Vertices, &line.P1,
	// 				1, s_LineBuffer->VerticesCount);

	// uint32_t indices[] =
	// {
	// 	(uint32_t)s_LineBuffer->IndicesCount,
	// 	(uint32_t)s_LineBuffer->IndicesCount + 1
	// };
	// RendererAPI::Get()
	// ->SetBufferData(s_LineBuffer, DrawBufferIndex::Indices, indices,
	// 				2, s_LineBuffer->IndicesCount);
}

void Renderer3D::DrawText(Ref<Text> text, const glm::mat4& tr,
						  DrawCommand* command)
{
	// TODO(Implement):
}

}