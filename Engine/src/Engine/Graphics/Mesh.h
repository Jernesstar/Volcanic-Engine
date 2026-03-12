#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include "Texture.h"

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct Vertex {
	Vec3 Position;
	Vec3 Normal;
	Vec2 TexCoord;
};

struct SubMesh {
	Buffer<Vertex> Vertices;
	Buffer<u32> Indices;
	u32 MaterialIndex;
};

struct SubMaterial {
	Ref<Texture> Diffuse = nullptr;
	Ref<Texture> Specular = nullptr;
	Ref<Texture> Emissive = nullptr;

	glm::vec4 DiffuseColor = glm::vec4(0.0f);
	glm::vec4 SpecularColor = glm::vec4(0.0f);
	glm::vec4 EmissiveColor = glm::vec4(0.0f);
};

enum class MeshType { Quad, Cube, Model };

class Mesh {
public:
	static Ref<Mesh> Create(MeshType type,
							Buffer<Vertex>&& vertices,
							Buffer<u32>&& indices);
	static Ref<Mesh> Create(MeshType type);

public:
	const MeshType Type;
	List<SubMesh> SubMeshes;
	List<SubMaterial> Materials;

public:
	Mesh(MeshType type)
		: Type(type) { }
	~Mesh() = default;
};

}