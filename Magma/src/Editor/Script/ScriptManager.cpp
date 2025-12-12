#include "ScriptManager.h"

#include <angelscript/add_on/scriptbuilder/scriptbuilder.h>

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/FileUtils.h>

#include "Core/Editor.h"

namespace fs = std::filesystem;

namespace Magma {

static int IncludeCallback(const char* includeStr, const char* from,
	CScriptBuilder* builder, void* param)
{
	auto includes = (List<std::string>*)param;
	std::string include = includeStr;
	Application::PushDir();

	bool found = false;
	includes->ForEach(
		[&](const std::string& path)
		{
			auto fullPath = (fs::path(path) / include).generic_string();
			if(FileUtils::PathExists(fullPath)) {
				found = true;
				builder->AddSectionFromFile(fullPath.c_str());
				return;
			}
		});

	Application::PopDir();

	if(!found) {
		VOLCANICORE_LOG_ERROR("Could not find include '%s'", includeStr);
	}

	return found ? 1 : -1;
}

asIScriptModule* ScriptManager::LoadScript(const List<std::string>& paths,
	bool metadata, bool* error, std::string name,
	const List<std::string>& includePaths)
{
	auto* engine = ScriptEngine::Get();

	if(name == "")
		name = fs::path(paths[0]).filename().stem().string();

	CScriptBuilder builder;
	int r;
	r = builder.StartNewModule(engine, name.c_str());
	if(r < 0) {
		VOLCANICORE_LOG_ERROR("StartNewModule failed for module name '%s'", name.c_str());
		if(error)
			*error = true;
		return nullptr;
	}

	builder.DefineWord("EDITOR");

	if(includePaths)
		builder.SetIncludeCallback(IncludeCallback, (void*)&includePaths);

	for(auto& path : paths) {
		r = builder.AddSectionFromFile(path.c_str());
		if(r < 0) {
			VOLCANICORE_LOG_ERROR("AddSectionFromFile failed for file '%s'", path.c_str());
			if(error)
				*error = true;
			return nullptr;
		}
	}

	r = builder.BuildModule();
	if(r < 0) {
		VOLCANICORE_LOG_ERROR("BuildModule failed for module name '%s'", name.c_str());
		if(error) *error = true;
		return nullptr;
	}


	asIScriptModule* handle = builder.GetModule();

	if(error)
		*error = false;

	if(!metadata)
		return handle;

	for(uint32_t i = 0; i < handle->GetFunctionCount(); i++) {
		asIScriptFunction* func = handle->GetFunctionByIndex(i);

	}

	for(uint32_t i = 0; i < handle->GetObjectTypeCount(); i++) {
		asITypeInfo* type = handle->GetObjectTypeByIndex(i);
		int id = type->GetTypeId();
		std::string className = type->GetName();

		auto& list = s_ClassMetadata[className];
		for(auto str : builder.GetMetadataForType(id))
			list.Add(str);

		for(uint32_t i = 0; i < type->GetPropertyCount(); i++) {
			for(auto str : builder.GetMetadataForTypeProperty(id, i)) {
				const char* name;
				type->GetProperty(i, &name);
				s_FieldMetadata[className + "::" + name].Add(str);
			}
		}

		for(uint32_t i = 0; i < type->GetMethodCount(); i++) {
			asIScriptFunction* method = type->GetMethodByIndex(i);
			for(auto str : builder.GetMetadataForTypeMethod(id, method)) {
				std::string name = method->GetName();
				s_MethodMetadata[className + "::" + name].Add(str);
			}
		}
	}

	return handle;
}

asIScriptModule* ScriptManager::LoadScript(const std::string& path,
	bool metadata, bool* error, std::string name,
	const List<std::string>& includePaths)
{
	return LoadScript({ path }, metadata, error, name, includePaths);
}
class ByteCodeWriter : public asIBinaryStream {
public:
	ByteCodeWriter(BinaryWriter* writer)
		: m_Writer(writer) { }
	~ByteCodeWriter() = default;

	int Read(void* data, uint32_t size) override {
		return 1;
	}

	int Write(const void* data, uint32_t size) override {
		m_Writer->WriteData(data, (uint64_t)size);
		return 0;
	}

private:
	BinaryWriter* m_Writer = nullptr;
};

void ScriptManager::SaveScript(asIScriptModule* mod, BinaryWriter& writer) {
	ByteCodeWriter stream(&writer);
	mod->SaveByteCode(&stream, true);
}

void ScriptManager::RunCodeAnalysis() {
	// auto panel =
	// 	Editor::GetProjectTab()->
	// 		GetPanel("ScriptEditor")->As<ScriptEditorPanel>();

	// auto* file = panel->GetFile();
}

static asIScriptContext* s_Context = nullptr;
static asIScriptFunction* s_LastFunction = nullptr;
static asIScriptModule* s_LastModule = nullptr;
static uint32_t s_StackLevel = 0;
static bool s_Suspended = false;
static uint8_t s_Action = 0;

static void DebugLineCallback(asIScriptContext* ctx) {
	// if(ctx->GetState() != asEXECUTION_ACTIVE)
	// 	return;

	// const char* path = nullptr;
	// int lineNbr = ctx->GetLineNumber(s_StackLevel, 0, &path);
	// if(!path)
	// 	return;

	// auto panel =
	// 	Editor::GetProjectTab()->
	// 		GetPanel("ScriptEditor")->As<ScriptEditorPanel>();
	// panel->OpenFile(path, s_Suspended);

	// auto* file = panel->GetFile(path);
	// VOLCANICORE_ASSERT(file);

	// bool isBreakpoint = file->Breakpoints.contains(lineNbr);
	// if(!isBreakpoint && !s_Suspended)
	// 	return;

	// asIScriptFunction* func = ctx->GetFunction();
	// VOLCANICORE_LOG_INFO("File: %s, Function: %s, Line: %d",
	// 	path, func->GetName(), lineNbr);
	// panel->SetDebugLine(lineNbr);

	// if(!s_Suspended)
	// 	ctx->Suspend();

	// s_Suspended = true;
	// Editor::GetProjectTab()->OnPause();

	// // Having this run on a separate thread won't block
	// // the editor application from responding to events
	// // i.e pressing on the debug buttons

	// // return == free to continue execution
	// while(s_Suspended) {
	// 	// Continue
	// 	if(s_Action == 4) {
	// 		Editor::GetProjectTab()->OnResume();
	// 		s_Suspended = false;
	// 		// ctx->Execute();
	// 		return;
	// 	}
	// 	// Step out
	// 	else if(s_Action == 3 && s_StackLevel <= ctx->GetCallstackSize()) {
	// 		s_Action = 0;
	// 		return;
	// 	}
	// 	else if(s_Action == 2) { // Step into
	// 		s_Action = 0;
	// 		break;
	// 	}
	// 	// Step over
	// 	else if(s_Action == 1 && s_StackLevel < ctx->GetCallstackSize()) {
	// 		s_Action = 0;
	// 		return;
	// 	}
	// }
}

void ScriptManager::StartDebug() {
	s_Context = ScriptEngine::GetContext();
	s_Context->SetLineCallback(
		asFUNCTION(DebugLineCallback), nullptr, asCALL_CDECL);
	s_Action = 0;
}

void ScriptManager::EndDebug() {
	if(s_Context)
		s_Context->ClearLineCallback();
	s_Context = nullptr;
	s_LastModule = nullptr;
	s_LastFunction = nullptr;
	s_StackLevel = 0;
	s_Suspended = false;
}

void ScriptManager::StepOver() {
	s_Action = 1;
}

void ScriptManager::StepInto() {
	s_Action = 2;
	s_StackLevel = s_Context->GetCallstackSize();
}

void ScriptManager::StepOut() {
	s_Action = 3;
	s_StackLevel = s_Context->GetCallstackSize();
}

void ScriptManager::Continue() {
	s_Action = 4;
}

ScriptField ScriptManager::GetVariable(const std::string& name) {
	for(int i = 0; i < s_Context->GetVarCount(); i++) {
		const char* varName;
		int typeID;
		s_Context->GetVar(i, 0, &varName, &typeID);

		if(std::string(varName) == name) {
			auto* varPtr = s_Context->GetAddressOfVar(i, 0);
			auto typeInfo = ScriptEngine::Get()->GetTypeInfoById(typeID);
			return { varPtr, varName, typeID, typeInfo };
		}
	}

	return { };
}

}