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

struct Material {
	Ref<Texture> Diffuse = nullptr;
	Ref<Texture> Specular = nullptr;
	Ref<Texture> Emissive = nullptr;

	Vec4 DiffuseColor = Vec4(0.0f);
	Vec4 SpecularColor = Vec4(0.0f);
	Vec4 EmissiveColor = Vec4(0.0f);
};

struct SubMesh {
	Buffer<Vertex> Vertices;
	Buffer<u32> Indices;
	u32 MaterialIndex;
};

enum class MeshType { Quad, Cube, Model };

class Mesh {
public:
	static Ref<Mesh> Create(MeshType type,
							Buffer<Vertex>&& vertices,
							Buffer<u32>&& indices,
							const Material& material = { });
	static Ref<Mesh> Create(MeshType type,
							const Material& material = { });
	static Ref<Mesh> Create(MeshType type, const Vec4& color);

public:
	const MeshType Type;
	List<SubMesh> SubMeshes;
	List<Material> Materials;

public:
	Mesh(MeshType type)
		: Type(type) { }
	~Mesh() = default;
};

}