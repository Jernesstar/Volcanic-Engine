#pragma once

#include "RenderPass.h"
#include "ScriptTexture.h"
#include "ScriptFramebuffer.h"

#include <Engine/Asset/AssetManager.h>

namespace VolcanicEngine {

class ScriptRenderPass {
public:
	static ScriptRenderPass* Factory(const std::string& name,
									 const std::string& shaderName)
	{
		auto shader = AssetManager::Get()->Get<Shader>(shaderName);
		auto pass = RenderPass::Create(name, shader);
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
		m_Pass->GetUniforms().Set(name, [tex]() {
			return TextureSlot{ tex->GetTexture(), tex->NextSlot() };
		});
	}
	void SetInputFloat(const std::string& name, float val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputVec4(const std::string& name, const glm::vec4& val) {
		m_Pass->GetUniforms().Set(name, [val]() { return val; });
	}

	ScriptFramebuffer* GetOutput() { return m_OutputRef; }
	Ref<RenderPass> GetPass() { return m_Pass; }

private:
	int m_RefCount = 1;
	Ref<RenderPass> m_Pass;
	ScriptFramebuffer* m_OutputRef = nullptr;
};
}