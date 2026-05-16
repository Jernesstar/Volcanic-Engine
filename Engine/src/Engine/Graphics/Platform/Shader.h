#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

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

enum class ShaderPropType { Int, Float, Vec2, Vec3, Vec4, Mat4, Texture, Sampler };

struct ShaderPropDecl {
	std::string Name;
	ShaderPropType Type;
	u32 Binding;       // descriptor set binding index
	u32 Set;
};

struct ShaderLayout {
	List<ShaderPropDecl> Uniforms;
	List<ShaderPropDecl> Samplers;
};

}

#include <VolcaniCore/Utils/BytesWriter.h>
#include <VolcaniCore/Utils/BytesReader.h>

namespace VolcaniCore {

template<>
inline BytesWriter& BytesWriter::WriteObject(const VolcanicEngine::Graphics::ShaderPropDecl& decl) {
	Write(decl.Name);
	Write((u32)decl.Type);
	Write(decl.Binding);
	Write(decl.Set);
	return *this;
}

template<>
inline BytesReader& BytesReader::ReadObject(VolcanicEngine::Graphics::ShaderPropDecl& decl) {
	Read(decl.Name);
	u32 type;
	Read(type);
	decl.Type = (VolcanicEngine::Graphics::ShaderPropType)type;
	Read(decl.Binding);
	Read(decl.Set);
	return *this;
}

template<>
inline BytesWriter& BytesWriter::WriteObject(const VolcanicEngine::Graphics::ShaderLayout& layout) {
	Write(layout.Uniforms);
	Write(layout.Samplers);
	return *this;
}

template<>
inline BytesReader& BytesReader::ReadObject(VolcanicEngine::Graphics::ShaderLayout& layout) {
	Read(layout.Uniforms);
	Read(layout.Samplers);
	return *this;
}

}

namespace VolcanicEngine::Graphics {

struct ShaderSpec {

};

class Shader : public Derivable<Shader> {
public:
	const ShaderSpec Spec;

public:
	Shader(const ShaderSpec& spec)
		: Spec(spec) { };
	virtual ~Shader() = default;

	virtual void SetShaderData(List<ShaderFile>&& files,
							   const ShaderLayout& layout) = 0;

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

	const ShaderLayout& GetLayout() const { return m_Layout; }

protected:
	ShaderLayout m_Layout;
};

}