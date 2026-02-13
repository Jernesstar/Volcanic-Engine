#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Lava::Graphics {

enum class ShaderFileType { Vertex, Fragment, Geometry, Compute, Unknown };
enum class ShaderDataType { Text, Binary };

struct ShaderFile {
	const ShaderFileType FileType;
	const ShaderDataType DataType;
	Buffer<void> Data;

	ShaderFile(ShaderFileType fileType, const std::string& data)
		: FileType(fileType), DataType(ShaderDataType::Text),
			Data(sizeof(char), data.length() + 1)
	{
		Data.Set((void*)data.c_str(), data.length() + 1);
	}

	ShaderFile(ShaderFileType fileType, Buffer<u32> data)
		: FileType(fileType), DataType(ShaderDataType::Binary),
			Data(sizeof(u32), data.GetCount())
		{
			Data.Set((void*)data.Get(), data.GetCount());
		}
};

struct ShaderSpec {

};

class Shader : public Derivable<Shader> {
public:
	const ShaderSpec Spec;

public:
	Shader(const ShaderSpec& spec)
		: Spec(spec) { };
	virtual ~Shader() = default;

	virtual void SetShaderData(List<ShaderFile>&& files) = 0;

	virtual void SetInt(const std::string& name, i32 _int) = 0;
	virtual void SetFloat(const std::string& name, f32 _float) = 0;

	virtual void SetVec2(const std::string& name, const Vec2& vec) = 0;
	virtual void SetVec3(const std::string& name, const Vec3& vec) = 0;
	virtual void SetVec4(const std::string& name, const Vec4& vec) = 0;

	virtual void SetMat2(const std::string& name, const Mat2& mat) = 0;
	virtual void SetMat3(const std::string& name, const Mat3& mat) = 0;
	virtual void SetMat4(const std::string& name, const Mat4& mat) = 0;

	virtual void SetUniformBuffer(const std::string& name, u32 binding) = 0;
	virtual void SetStorageBuffer(const std::string& name, u32 binding) = 0;
};

}