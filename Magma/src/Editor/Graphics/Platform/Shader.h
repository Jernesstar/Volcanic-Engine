#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Template.h>

using namespace VolcaniCore;

namespace Magma::Graphics {

enum class ShaderFileType { Vertex, Fragment, Geometry, Compute, Unknown };
enum class ShaderDataType { Text, Binary };

struct ShaderFile {
	const ShaderFileType FileType;
	const ShaderDataType DataType;
	Buffer<void> Data;
};

class Shader : public Derivable<Shader> {
public:
	Shader() = default;
	virtual ~Shader() = default;

	virtual void SetInt(const std::string& name, i32 _int) = 0;
	virtual void SetFloat(const std::string& name, f32 _float) = 0;

	virtual void SetVec2(const std::string& name, const Vec2& vec) = 0;
	virtual void SetVec3(const std::string& name, const Vec3& vec) = 0;
	virtual void SetVec4(const std::string& name, const Vec4& vec) = 0;

	virtual void SetMat2(const std::string& name, const Mat2& mat) = 0;
	virtual void SetMat3(const std::string& name, const Mat3& mat) = 0;
	virtual void SetMat4(const std::string& name, const Mat4& mat) = 0;

	virtual void SetUniformBuffer(const std::string& name, uint32_t binding) = 0;
	virtual void SetStorageBuffer(const std::string& name, uint32_t binding) = 0;
};

struct ShaderSpecification {
	List<ShaderFile> Files;
};

}