#pragma once

#include "RenderPass.h"
#include "ScriptTexture.h"
#include "ScriptFramebuffer.h"

#include <Engine/Asset/AssetManager.h>
#include <Engine/Graphics/Renderer2D.h>

namespace VolcanicEngine {

class ScriptRenderPass {
public:
	static ScriptRenderPass* Factory(const std::string& name,
									 const std::string& shaderName)
	{
		auto shader = AssetManager::Get()->Get<Shader>(shaderName);
		auto pass = RenderPass::Create(name, shader);
		pass->SetData(Renderer2D::GetScreenBuffer());
		return new ScriptRenderPass(pass);
	}

	ScriptRenderPass(Ref<RenderPass> pass = nullptr) {
		m_Pass = pass;
	}

	void AddRef() { m_RefCount++; }
	void Release() { if(--m_RefCount == 0) delete this; }

	void SetOutput(ScriptFramebuffer* fb) {
		m_Pass->SetOutput(fb->Resolve());
	}

	void SetInputTexture(const std::string& name, ScriptTexture* tex) {
		if(!tex) return;
		u32 slot = m_NextTextureSlot++;
		if(tex->IsRawTexture()) {
			Ref<Texture> texture = tex->GetTexture();
			m_Pass->GetUniforms().Set(name, [texture, slot]() {
				return TextureSlot{ texture, slot };
			});
			return;
		}
		Ref<Attachment> att = tex->GetAttachment();
		if(!att) return;
		m_Pass->GetUniforms().Set(name, [att, slot]() {
			return AttachmentSlot{ att, slot };
		});
	}

	void SetInputInt(const std::string& name, i32 val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputFloat(const std::string& name, float val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputVec2(const std::string& name, const glm::vec2& val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputVec3(const std::string& name, const glm::vec3& val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputVec4(const std::string& name, const glm::vec4& val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputMat4(const std::string& name, const glm::mat4& val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}

	void Execute() {
		m_NextTextureSlot = 0;
		Renderer::StartPass(m_Pass);
		auto* cmd = Renderer::GetCommand();
		m_Pass->SetUniforms(cmd);

		auto* call = cmd->NewCall();
		call->VertexCount = 6;
		call->Primitive   = DrawPrimitive::Triangle;
		call->Partition   = DrawPartition::Single;

		Renderer::EndPass();
	}

	ScriptFramebuffer* GetOutput() { return m_OutputRef; }
	Ref<RenderPass> GetPass() { return m_Pass; }

private:
	int m_RefCount = 1;
	u32 m_NextTextureSlot = 0;
	Ref<RenderPass> m_Pass;
	ScriptFramebuffer* m_OutputRef = nullptr;
};

}