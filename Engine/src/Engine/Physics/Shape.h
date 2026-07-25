#pragma once

#include <cstdint>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Math.h>

#include <Engine/Graphics/Geometry.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Physics {

// Thin wrapper around a Jolt collision shape. Static-body colliders are built
// from these: axis-aligned boxes for blocky geometry, cooked meshes for
// arbitrary geometry, plus cheap sphere/capsule primitives.
class Shape {
public:
	enum class Type : uint8_t {
		Box = 0,
		Sphere = 1,
		Capsule = 2,
		Mesh = 3,
	};

public:
	static Ref<Shape> CreateBox(const Vec3& halfExtents);
	static Ref<Shape> CreateSphere(float radius);
	static Ref<Shape> CreateCapsule(float halfHeight, float radius);

	// Cook a static triangle-mesh shape from a graphics Geometry's surfaces.
	static Ref<Shape> CreateMesh(const Ref<Graphics::Geometry>& geometry);

public:
	Shape(Type type, JPH::RefConst<JPH::Shape> shape)
		: m_Type(type), m_Shape(std::move(shape)) { }
	~Shape() = default;

	Type GetType() const { return m_Type; }
	bool IsValid() const { return m_Shape != nullptr; }

	const JPH::RefConst<JPH::Shape>& Get() const { return m_Shape; }

private:
	Type m_Type;
	JPH::RefConst<JPH::Shape> m_Shape;
};

}
