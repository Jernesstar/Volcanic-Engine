#include "ScriptEngine.h"

#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Script {

static asIScriptEngine* s_Engine;

static List<asIScriptContext*> s_Contexts;
static asIScriptContext* s_HookContext = nullptr;

static void MessageCallback(const asSMessageInfo *msg, void *param) {
	const char* type = "ERR ";
	if(msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if(msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	Log::Info("{0} ({1}, {2}) : {3}: {4}",
		msg->section, msg->row, msg->col, type, msg->message);
}

void ScriptEngine::Init() {
	s_Engine = asCreateScriptEngine();
	s_Engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
	for(uint32_t i = 0; i < 5; i++)
		s_Contexts.Add(s_Engine->CreateContext());
}

void ScriptEngine::Shutdown() {
	if(s_HookContext) {
		s_HookContext->Release();
		s_HookContext = nullptr;
	}
	for(asIScriptContext* context : s_Contexts)
		context->Release();
	s_Contexts.Clear();
	if(s_Engine)
		s_Engine->ShutDownAndRelease();
}

asIScriptEngine* ScriptEngine::Get() {
	return s_Engine;
}

void ScriptEngine::RegisterSingleton(const std::string& className,
									 const std::string& instanceName,
									 void* instance)
{
	s_Engine->RegisterObjectType(
		className.c_str(), 0, asOBJ_REF | asOBJ_NOHANDLE);

	s_Engine->RegisterGlobalProperty(
		(className + " " + instanceName).c_str(), instance);
}

asIScriptContext* ScriptEngine::GetContext() {
	for(auto* context : s_Contexts) {
		if(context == s_HookContext)
			continue;
		if(context->GetState() == asEXECUTION_ACTIVE)
			continue;
		context->Unprepare();
		return context;
	}

	auto* newContext = s_Engine->CreateContext();
	s_Contexts.Add(newContext);
	return newContext;
}

asIScriptContext* ScriptEngine::GetHookContext() {
	if(!s_HookContext)
		s_HookContext = s_Engine->CreateContext();
	s_HookContext->Unprepare();
	return s_HookContext;
}

InterfaceBuilder ScriptEngine::RegisterInterface(const std::string& name) {
	s_Engine->RegisterInterface(name.c_str());

	return InterfaceBuilder{ name };
}

InterfaceBuilder& InterfaceBuilder::AddMethod(const std::string& decl) {
	s_Engine->RegisterInterfaceMethod(Name.c_str(), decl.c_str());
	return *this;
}

}