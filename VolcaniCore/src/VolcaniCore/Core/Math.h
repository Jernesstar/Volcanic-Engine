#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

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

constexpr float PI = 3.14159265358979328462f;
constexpr float e  = 2.71828182859f;

static constexpr float RadToDeg(float radians) {
	return (180.0f / PI) * radians;
}
static constexpr float DegToRad(float degrees) {
	return (PI / 180.0f) * degrees;
}

struct Transform {
	glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation	  = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale 	  = { 1.0f, 1.0f, 1.0f };

	glm::mat4 GetTransform() const {
		return glm::translate(glm::mat4(1.0f), Translation)
			 * glm::toMat4(glm::quat(Rotation))
			 * glm::scale(glm::mat4(1.0f), Scale);
	}

	operator glm::mat4() const { return GetTransform(); }
};

}