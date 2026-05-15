#pragma once

#include "RenderPipeline.h"

namespace VolcanicEngine {

class ScriptRenderPipeline : public RenderPipeline {
public:
	ScriptRenderPipeline(asIScriptObject* obj) : m_Obj(obj) {
		m_Obj->AddRef();
		// Cache method pointers by name
		auto* type = m_Obj->GetObjectType();
		m_OnInit = type->GetMethodByDecl("void OnInit()");
		m_OnRender = type->GetMethodByDecl("void OnRender(Scene@)");
		m_OnResize = type->GetMethodByDecl("void OnResize(uint, uint)");
	}
	~ScriptRenderPipeline() { m_Obj->Release(); }

	void OnInit() override {
		auto* ctx = ScriptEngine::GetContext();
		ScriptFunc func{ m_OnInit, ctx, m_Obj };
		func.CallVoid();
	}

	void OnRender(Scene* scene) override {
		auto* ctx = ScriptEngine::GetContext();
		ScriptFunc func{ m_OnRender, ctx, m_Obj };
		func.CallVoid(scene);
	}

	void OnResize(u32 w, u32 h) override {
		if(!m_OnResize) return;
		auto* ctx = ScriptEngine::GetContext();
		ScriptFunc func{ m_OnResize, ctx, m_Obj };
		func.CallVoid(w, h);
	}

	Ref<Framebuffer> GetOutput() const override {

	}

private:
	asIScriptObject* m_Obj;
	asIScriptFunction* m_OnInit = nullptr;
	asIScriptFunction* m_OnRender = nullptr;
	asIScriptFunction* m_OnResize = nullptr;
};

}