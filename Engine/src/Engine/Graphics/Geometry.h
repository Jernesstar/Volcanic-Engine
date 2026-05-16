#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

struct Vertex {
	Vec3 Position;
	Vec3 Normal;
	Vec2 TexCoord;
};

struct SubGeometry {
	Buffer<Vertex> Vertices;
	Buffer<u32> Indices;
	u32 SurfaceSlot; // which material slot this surface maps to
};

enum class GeometryType { Quad, Cube, Sphere, Capsule, Model };

class Geometry {
public:
	static Ref<Geometry> Create(GeometryType type,
							Buffer<Vertex>&& vertices,
							Buffer<u32>&& indices);
	static Ref<Geometry> Create(GeometryType type);

public:
	const GeometryType Type;
	List<SubGeometry> Surfaces;

public:
	Geometry(GeometryType type)
		: Type(type) { }
	~Geometry() = default;
};

}