#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define assert(x) (void)0

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

#undef assert

#include "Defines.h"

namespace VolcaniCore {

using Vec2f = glm::vec2;
using Vec3f = glm::vec3;
using Vec4f = glm::vec4;

using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;

using Vec2 = Vec2f;
using Vec3 = Vec3f;
using Vec4 = Vec4f;

using Mat2f = glm::mat2;
using Mat3f = glm::mat3;
using Mat4f = glm::mat4;

using Mat2 = Mat2f;
using Mat3 = Mat3f;
using Mat4 = Mat4f;

constexpr f32 PI = 3.14159265358979328462f;
constexpr f32 e  = 2.71828182859f;

static constexpr f32 RadToDeg(f32 radians) {
	return (180.0f / PI) * radians;
}
static constexpr f32 DegToRad(f32 degrees) {
	return (PI / 180.0f) * degrees;
}

struct Transform {
	Vec3 Translation = { 0.0f, 0.0f, 0.0f };
	Vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	Vec3 Scale = { 1.0f, 1.0f, 1.0f };

	Mat4 GetTransform() const {
		return glm::translate(glm::mat4(1.0f), Translation)
			 * glm::toMat4(glm::quat(Rotation))
			 * glm::scale(glm::mat4(1.0f), Scale);
	}

	operator glm::mat4() const { return GetTransform(); }
};

}