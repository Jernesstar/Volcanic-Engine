#pragma once

#include <VolcaniCore/Core/List.h>

#include "ScriptObject.h"

namespace Magma::Script {

class ScriptModule;

class ScriptClass {
public:
	const std::string Name;

public:
	ScriptClass(const std::string& name, asITypeInfo* type);
	~ScriptClass() = default;

	bool Implements(const std::string& interfaceName);
	bool DerivesFrom(const std::string& className);

	void SetInstanceMethod(const VolcaniCore::List<std::string>& args);

	template<typename... Args>
	ScriptObject Instantiate(Args&&... args) const {
		ScriptFunc func = GetFunc();
		asIScriptObject* obj =
			func.CallReturn<asIScriptObject*>(std::forward<Args>(args)...);

		ScriptObject newObj(obj);
		newObj.m_Class = (ScriptClass*)this;
		newObj.m_Initialized = true;
		newObj.AddRef();
		return newObj;
	}

	ScriptObject Construct() const {
		asIScriptObject* obj =
			(asIScriptObject*)ScriptEngine::Get()
								->CreateUninitializedScriptObject(m_Type);

		ScriptObject newObj(obj);
		newObj.m_Class = (ScriptClass*)this;
		newObj.m_Initialized = false;
		newObj.AddRef();
		return newObj;
	}

	asIScriptFunction* GetFunction(const std::string& name) const;
	asITypeInfo* GetType() const { return m_Type; }
	const auto& GetFunctions() const { return m_Functions; }

private:
	ScriptFunc GetFunc() const;

private:
	asITypeInfo* m_Type;
	asIScriptFunction* m_Factory;
	Map<std::string, uint32_t> m_FieldMap;
	Map<std::string, asIScriptFunction*> m_Functions;

	friend class ScriptObject;
};

}