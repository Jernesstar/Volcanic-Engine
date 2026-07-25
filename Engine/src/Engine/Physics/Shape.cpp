#include "Shape.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <VolcaniCore/Core/Log.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Physics {

Ref<Shape> Shape::CreateBox(const Vec3& halfExtents) {
	// Jolt requires every half-extent to be >= the box convex radius. Clamp the
	// extents to a small floor and shrink the convex radius to fit thin boxes
	// (e.g. floor tiles) so we don't trip "Invalid convex radius".
	JPH::Vec3 he(
		std::max(halfExtents.x, 0.01f),
		std::max(halfExtents.y, 0.01f),
		std::max(halfExtents.z, 0.01f));

	float minExtent = std::min({ he.GetX(), he.GetY(), he.GetZ() });
	float convexRadius = std::min(0.05f, minExtent * 0.5f);

	JPH::BoxShapeSettings settings(he, convexRadius);
	settings.SetEmbedded();
	auto result = settings.Create();
	if(result.HasError()) {
		Log::Error("Jolt box shape: {}", result.GetError().c_str());
		return nullptr;
	}

	return CreateRef<Shape>(Type::Box, result.Get());
}

Ref<Shape> Shape::CreateSphere(float radius) {
	JPH::SphereShapeSettings settings(std::max(radius, 0.001f));
	settings.SetEmbedded();
	auto result = settings.Create();
	if(result.HasError()) {
		Log::Error("Jolt sphere shape: {}", result.GetError().c_str());
		return nullptr;
	}

	return CreateRef<Shape>(Type::Sphere, result.Get());
}

Ref<Shape> Shape::CreateCapsule(float halfHeight, float radius) {
	JPH::CapsuleShapeSettings settings(
		std::max(halfHeight, 0.001f), std::max(radius, 0.001f));
	settings.SetEmbedded();
	auto result = settings.Create();
	if(result.HasError()) {
		Log::Error("Jolt capsule shape: {}", result.GetError().c_str());
		return nullptr;
	}

	return CreateRef<Shape>(Type::Capsule, result.Get());
}

Ref<Shape> Shape::CreateMesh(const Ref<Graphics::Geometry>& geometry) {
	if(!geometry)
		return nullptr;

	JPH::VertexList vertices;
	JPH::IndexedTriangleList triangles;

	uint32_t vertexBase = 0;
	for(const auto& surface : geometry->Surfaces) {
		const auto& verts = surface.Vertices;
		const auto& indices = surface.Indices;

		for(u64 i = 0; i < verts.GetCount(); i++) {
			const auto& p = verts.Get()[i].Position;
			vertices.push_back(JPH::Float3(p.x, p.y, p.z));
		}

		for(u64 i = 0; i + 2 < indices.GetCount(); i += 3) {
			triangles.push_back(
				JPH::IndexedTriangle(
					vertexBase + indices.Get()[i + 0],
					vertexBase + indices.Get()[i + 1],
					vertexBase + indices.Get()[i + 2]));
		}

		vertexBase += (uint32_t)verts.GetCount();
	}

	if(vertices.empty() || triangles.empty()) {
		Log::Error("Jolt mesh shape: empty geometry");
		return nullptr;
	}

	JPH::MeshShapeSettings settings(std::move(vertices), std::move(triangles));
	settings.SetEmbedded();
	auto result = settings.Create();
	if(result.HasError()) {
		Log::Error("Jolt mesh shape: {}", result.GetError().c_str());
		return nullptr;
	}

	return CreateRef<Shape>(Type::Mesh, result.Get());
}

}
