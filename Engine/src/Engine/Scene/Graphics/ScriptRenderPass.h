#pragma once

#include "RenderPass.h"
#include "ScriptFramebuffer.h"

namespace VolcanicEngine {

class ScriptRenderPass {
public:
	static ScriptRenderPass* Factory(const std::string& name,
									 const std::string& shaderPath)
	{
		// auto shader = AssetManager::Load<Shader>(shaderPath);
		// auto pass = RenderPass::Create(name, shader);
		// return new ScriptRenderPass(pass);
	}

	void AddRef() { m_refCount++; }
	void Release() { if(--m_refCount == 0) delete this; }

	void SetOutput(ScriptFramebuffer* fb) {
		m_pass->SetOutput(fb->Resolve());
	}

	void SetInputTexture(const std::string& name, ScriptTexture* tex) {
		m_pass->GetUniforms().Set(name, [tex]() {
			return TextureSlot{ tex->GetTexture(), tex->NextSlot() };
		});
	}
	void SetInputFloat(const std::string& name, float val) {
		m_pass->GetUniforms().Set(name, [val]() { return val; });
	}
	void SetInputVec4(const std::string& name, const glm::vec4& val) {
		m_pass->GetUniforms().Set(name, [val]() { return val; });
	}

	ScriptFramebuffer* GetOutput() { return m_outputRef; }
	Ref<RenderPass> GetPass() { return m_pass; }

private:
	int m_refCount = 1;
	Ref<RenderPass> m_pass;
	ScriptFramebuffer* m_outputRef = nullptr;
};
}