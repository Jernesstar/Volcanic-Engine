#pragma once

#include <glad/glad.h>

#include "Platform/Shader.h"

using namespace VolcanicEngine;

namespace OpenGL {

static u32 GetShaderType(Graphics::ShaderFileType type) {
	switch(type) {
		case Graphics::ShaderFileType::Vertex:   return GL_VERTEX_SHADER;
		case Graphics::ShaderFileType::Fragment: return GL_FRAGMENT_SHADER;
		case Graphics::ShaderFileType::Geometry: return GL_GEOMETRY_SHADER;
		case Graphics::ShaderFileType::Compute:  return GL_COMPUTE_SHADER;
	}

	return 0;
}

class Shader : public Graphics::Shader {
public:
	Shader(const Graphics::ShaderSpec& spec)
		: Graphics::Shader(spec)
	{
		m_ProgramID = glCreateProgram();
	}
	~Shader() {
		glDeleteProgram(m_ProgramID);
	}

	void SetShaderData(List<Graphics::ShaderFile>&& files) override {
		List<u32> ids(files.Count());

		for(const auto& shader : files) {
			u32 type = GetShaderType(shader.FileType);
			u32 shaderID = glCreateShader(type);

			if(shader.DataType == Graphics::ShaderDataType::Text) {
				const char* address = (const char*)shader.Data.Get();
				glShaderSource(shaderID, 1, &address, nullptr);
				glCompileShader(shaderID);
			}
			else if(shader.DataType == Graphics::ShaderDataType::Binary) {
				// Expects u8, so here we use Count = GetMaxSize = GetMaxCount * 4
				glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V,
					shader.Data.Get(), (GLsizei)shader.Data.GetMaxSize());
				glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			}

			int result;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);

			if(result == GL_FALSE) {
				int length;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
				if(length) {
					char* message = new char[length];
					glGetShaderInfoLog(shaderID, length, &length, message);
					Log::Error("A compile error was detected \n{}", message);
				}
			}

			glAttachShader(m_ProgramID, shaderID);
			ids.Add(shaderID);
		}

		glLinkProgram(m_ProgramID);
		int result;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &result);

		if(result == GL_FALSE) {
			GLint length;
			glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &length);

			if(length) {
				char* message = new char[length];
				glGetProgramInfoLog(m_ProgramID, length, &length, message);
				Log::Error("A linking error was detected \n{}", message);
			}

			glDeleteProgram(m_ProgramID);
		}

		for(auto& shaderID : ids) {
			glDetachShader(m_ProgramID, shaderID);
			glDeleteShader(shaderID);
		}
	}

	void SetInt(const std::string& name, i32 _int) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, _int);
	}

	void SetFloat(const std::string& name, f32 _float) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1f(location, _float);
	}

	void SetVec2(const std::string& name, const Vec2& vec) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2f(location, vec.x, vec.y);
	}

	void SetVec3(const std::string& name, const Vec3& vec) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3f(location, vec.x, vec.y, vec.z);
	}

	void SetVec4(const std::string& name, const Vec4& vec) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
	}

	void SetMat2(const std::string& name, const Mat2& mat) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void SetMat3(const std::string& name, const Mat3& mat) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void SetMat4(const std::string& name, const Mat4& mat) override {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void SetUniformBuffer(const std::string& name, u32 binding) override {
		GLint location = glGetUniformBlockIndex(m_ProgramID, name.c_str());
		glUniformBlockBinding(m_ProgramID, location, binding);
	}
	void SetStorageBuffer(const std::string& name, u32 binding) override {
		GLint location =
			glGetProgramResourceIndex(m_ProgramID,
				GL_SHADER_STORAGE_BLOCK, name.c_str());
		glShaderStorageBlockBinding(m_ProgramID, location, binding);
	}

	void Bind() const {	glUseProgram(m_ProgramID); }
	void Unbind() const { glUseProgram(0); }

	void Lock() { glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); }
	void Compute(u32 x, u32 y, u32 z) const {
		glDispatchCompute(x, y, z);
	}

private:
	u32 m_ProgramID;
};

}