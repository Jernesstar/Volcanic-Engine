#include "ScriptGlue.h"

#include <iostream>
#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <angelscript.h>
#include <angelscript/add_on/scripthandle/scripthandle.h>
#include <angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <angelscript/add_on/scripthelper/scripthelper.h>
#include <angelscript/add_on/scriptmath/scriptmath.h>
#include <angelscript/add_on/scriptarray/scriptarray.h>

#include <VolcaniCore/Core/Input.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Event/KeyEvents.h>
#include <VolcaniCore/Event/MouseEvents.h>

// #include <Magma/Core/AssetManager.h>
#include <Magma/Script/ScriptEngine.h>
#include <Magma/Script/ScriptModule.h>
#include <Magma/Script/ScriptClass.h>
#include <Magma/Script/ScriptObject.h>

#include "Types/GridSet.h"
#include "Types/GridSet3D.h"
#include "Types/Timer.h"

#include "App.h"

using namespace Magma;
using namespace Magma::Script;
using namespace Lava::ECS;
// using namespace Lava::Physics;

namespace Lava {

static void RegisterGlobalFunctions();
static void RegisterTypes();
static void RegisterECS();
static void RegisterPhysics();
static void RegisterEvents();

void ScriptGlue::RegisterInterface() {
	auto* engine = ScriptEngine::Get();

	RegisterStdString(engine);
	RegisterScriptHandle(engine);
	RegisterScriptMath(engine);
	RegisterScriptArray(engine, true);

	RegisterEvents();
	ScriptEngine::RegisterInterface("IApp")
		.AddMethod("void OnLoad()")
		.AddMethod("void OnClose()")
		.AddMethod("void OnUpdate(float ts)");

	// ScriptEngine::RegisterInterface("IScreen")
	// 	.AddMethod("void OnLoad()")
	// 	.AddMethod("void OnClose()")
	// 	.AddMethod("void OnUpdate(float ts)");

	RegisterGlobalFunctions();
	RegisterTypes();
	RegisterECS();

	GridSet::RegisterInterface();
	GridSet3D::RegisterInterface();
	Timer::RegisterInterface();
}

void ScriptGlue::Copy(Ref<ScriptObject> src, Ref<ScriptObject> dst) {
	// if(src->GetClass()->Name != dst->GetClass()->Name)
	// 	return;

	// for(uint32_t i = 0; i < src->GetHandle()->GetPropertyCount(); i++) {
	// 	ScriptField ours = dst->GetProperty(i);
	// 	ScriptField field = src->GetProperty(i);
	// 	if(!field.Data)
	// 		continue;
	// 	if(field.Is(ScriptQualifier::ScriptObject))
	// 		continue;

	// 	std::string fieldType;
	// 	if(field.Type)
	// 		fieldType = field.Type->GetName();

	// 	size_t size = 0;
	// 	if(field.TypeID > 0 && field.TypeID <= 11) // (Void, Double]
	// 		size = ScriptEngine::Get()->GetSizeOfPrimitiveType(field.TypeID);
	// 	else if(fieldType == "Asset")
	// 		size = sizeof(Asset);
	// 	else if(fieldType == "Vec3")
	// 		size = sizeof(Vec3);

	// 	if(size) {
	// 		void* us = dst->GetHandle()->GetAddressOfProperty(i);
	// 		void* them = src->GetHandle()->GetAddressOfProperty(i);
	// 		memcpy(us, them, size);
	// 		continue;
	// 	}

	// 	if(fieldType == "string")
	// 		*ours.As<std::string>() = *field.As<std::string>();
	// 	else if(fieldType == "array")
	// 		*ours.As<CScriptArray>() = *field.As<CScriptArray>();
	// 	else if(fieldType == "GridSet")
	// 		*ours.As<Lava::GridSet>() = *field.As<Lava::GridSet>();
	// }
}

static void print(const std::string& str) {
	std::cout << str << "\n";
}

void RegisterGlobalFunctions() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterGlobalFunction(
		"void print(const string &in)", asFUNCTION(print), asCALL_CDECL);
}

static void Vec3DefaultConstructor(Vec3* ptr) {
	new(ptr) Vec3();
}
static void Vec4DefaultConstructor(Vec4* ptr) {
	new(ptr) Vec4();
}

static void Vec3CopyConstructor(const Vec3& other, Vec3* ptr) {
	new(ptr) Vec3(other);
}
static void Vec4CopyConstructor(const Vec4& other, Vec4* ptr) {
	new(ptr) Vec4(other);
}

static void Vec3ConvConstructor(float v, Vec3* ptr) {
	new(ptr) Vec3(v);
}
static void Vec4ConvConstructor(float v, Vec4* ptr) {
	new(ptr) Vec4(v);
}

static void Vec3InitConstructor(float r, float g, float b, Vec3* ptr) {
	new(ptr) Vec3(r, g, b);
}
static void Vec4InitConstructor(float r, float g, float b, float a, Vec4* ptr) {
	new(ptr) Vec4(r, g, b, a);
}

static void Vec3ListConstructor(float* list, Vec3* ptr) {
	new(ptr) Vec3(list[0], list[1], list[2]);
}
static void Vec4ListConstructor(float* list, Vec4* ptr) {
	new(ptr) Vec4(list[0], list[1], list[2], list[3]);
}

static Vec3 AddVec3(const Vec3& v1, const Vec3& v2) {
	return v1 + v2;
}

static Vec3 SubVec3(const Vec3& v1, const Vec3& v2) {
	return v1 - v2;
}

static Vec3 NegateVec3(const Vec3& dest) {
	return -dest;
}

static Vec3 MultiplyVec3(float r, const Vec3& dest) {
	return r * dest;
}

static Vec3 FloatDivideVec3(float r, const Vec3& dest) {
	return r / dest;
}

static Vec3 FloatDividedByVec3(float r, const Vec3& dest) {
	return dest / r;
}

void RegisterTypes() {
	auto* engine = ScriptEngine::Get();

	// engine->SetDefaultNamespace("Math");
	engine->RegisterObjectType("Vec3", sizeof(Vec3),
		asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS
		| asGetTypeTraits<Vec3>());
	engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT,
		"void f()", asFUNCTION(Vec3DefaultConstructor), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT,
		"void f(const Vec3 &in)", asFUNCTION(Vec3CopyConstructor),
		asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT,
		"void f(float)", asFUNCTION(Vec3ConvConstructor), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT,
		"void f(float, float, float)", asFUNCTION(Vec3InitConstructor),
		asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("Vec3", asBEHAVE_LIST_CONSTRUCT,
		"void f(const int &in) { float, float, float }",
		asFUNCTION(Vec3ListConstructor), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectProperty("Vec3", "float x", asOFFSET(Vec3, x));
	engine->RegisterObjectProperty("Vec3", "float y", asOFFSET(Vec3, y));
	engine->RegisterObjectProperty("Vec3", "float z", asOFFSET(Vec3, z));

	engine->RegisterObjectMethod("Vec3", "Vec3 &opAddAssign(const Vec3 &in)",
		asMETHODPR(Vec3, operator+=, (const Vec3 &), Vec3&), asCALL_THISCALL);
	engine->RegisterObjectMethod("Vec3", "Vec3 &opSubAssign(const Vec3 &in)",
		asMETHODPR(Vec3, operator-=, (const Vec3 &), Vec3&), asCALL_THISCALL);
	engine->RegisterObjectMethod("Vec3", "Vec3 opAdd(const Vec3 &in) const",
		asFUNCTION(AddVec3), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opSub(const Vec3 &in) const",
		asFUNCTION(SubVec3), asCALL_CDECL_OBJFIRST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opNeg() const",
		asFUNCTION(NegateVec3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opMul(float r) const",
		asFUNCTION(MultiplyVec3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opMul_r(float r) const",
		asFUNCTION(MultiplyVec3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opDiv(float r) const",
		asFUNCTION(FloatDivideVec3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("Vec3", "Vec3 opDiv_r(float r) const",
		asFUNCTION(FloatDividedByVec3), asCALL_CDECL_OBJLAST);

	engine->RegisterGlobalFunction("float radians(float deg)",
		asFUNCTIONPR(glm::radians, (float), float), asCALL_CDECL);
	engine->RegisterGlobalFunction("Vec3 radians(const Vec3 &in)",
		asFUNCTIONPR(glm::radians, (const Vec3&), Vec3), asCALL_CDECL);
	engine->RegisterGlobalFunction("float degrees(float deg)",
		asFUNCTIONPR(glm::degrees, (float), float), asCALL_CDECL);
	engine->RegisterGlobalFunction("Vec3 degrees(const Vec3 &in)",
		asFUNCTIONPR(glm::degrees, (const Vec3&), Vec3), asCALL_CDECL);
	engine->RegisterGlobalFunction("Vec3 sin(const Vec3 &in)",
		asFUNCTIONPR(glm::sin, (const Vec3&), Vec3), asCALL_CDECL);
	engine->RegisterGlobalFunction("Vec3 cos(const Vec3 &in)",
		asFUNCTIONPR(glm::cos, (const Vec3&), Vec3), asCALL_CDECL);
	engine->RegisterGlobalFunction("Vec3 normalize(const Vec3 &in)",
		asFUNCTIONPR(glm::normalize, (const Vec3&), Vec3), asCALL_CDECL);

	// engine->SetDefaultNamespace("");
}

static KeyPressedEvent* KeyPressedEventCast(KeyEvent* event) {
	if(event->Type != EventType::KeyPressed)
		return nullptr;
	return static_cast<KeyPressedEvent*>(event);
}

static KeyReleasedEvent* KeyReleasedEventCast(KeyEvent* event) {
	if(event->Type != EventType::KeyReleased)
		return nullptr;
	return static_cast<KeyReleasedEvent*>(event);
}

static KeyCharEvent* KeyCharacterEventCast(KeyEvent* event) {
	if(event->Type != EventType::KeyChar)
		return nullptr;
	return static_cast<KeyCharEvent*>(event);
}

static MouseButtonPressedEvent* MousePressedEventCast(MouseEvent* event) {
	if(event->Type != EventType::MouseButtonPressed)
		return nullptr;
	return static_cast<MouseButtonPressedEvent*>(event);
}

void RegisterEvents() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterEnum("Mouse");
	engine->RegisterEnumValue("Mouse", "Left", 0);
	engine->RegisterEnumValue("Mouse", "Right", 1);
	engine->RegisterEnumValue("Mouse", "Middle", 2);

	engine->RegisterEnum("Key");
	engine->RegisterEnumValue("Key", "A", 65);
	engine->RegisterEnumValue("Key", "B", 66);
	engine->RegisterEnumValue("Key", "C", 67);
	engine->RegisterEnumValue("Key", "D", 68);
	engine->RegisterEnumValue("Key", "E", 69);
	engine->RegisterEnumValue("Key", "F", 70);
	engine->RegisterEnumValue("Key", "G", 71);
	engine->RegisterEnumValue("Key", "H", 72);
	engine->RegisterEnumValue("Key", "I", 73);
	engine->RegisterEnumValue("Key", "J", 74);
	engine->RegisterEnumValue("Key", "K", 75);
	engine->RegisterEnumValue("Key", "L", 76);
	engine->RegisterEnumValue("Key", "M", 77);
	engine->RegisterEnumValue("Key", "N", 78);
	engine->RegisterEnumValue("Key", "O", 79);
	engine->RegisterEnumValue("Key", "P", 80);
	engine->RegisterEnumValue("Key", "Q", 81);
	engine->RegisterEnumValue("Key", "R", 82);
	engine->RegisterEnumValue("Key", "S", 83);
	engine->RegisterEnumValue("Key", "T", 84);
	engine->RegisterEnumValue("Key", "U", 85);
	engine->RegisterEnumValue("Key", "V", 86);
	engine->RegisterEnumValue("Key", "W", 87);
	engine->RegisterEnumValue("Key", "X", 88);
	engine->RegisterEnumValue("Key", "Y", 89);
	engine->RegisterEnumValue("Key", "Z", 90);

	engine->RegisterEnumValue("Key", "Space", 32);
	engine->RegisterEnumValue("Key", "Ctrl", 224 + 230);
	engine->RegisterEnumValue("Key", "Shift", 225 + 229);
	engine->RegisterEnumValue("Key", "Enter", 257);

	engine->RegisterEnumValue("Key", "Right", 262);
	engine->RegisterEnumValue("Key", "Left",  263);
	engine->RegisterEnumValue("Key", "Down",  264);
	engine->RegisterEnumValue("Key", "Up",    265);

	engine->SetDefaultNamespace("Input");
	engine->RegisterGlobalFunction("bool KeyPressed(Key key)",
		asFUNCTION(Input::KeyPressed), asCALL_CDECL);
	engine->RegisterGlobalFunction("bool MousePressed(Mouse button)",
		asFUNCTION(Input::MouseButtonPressed), asCALL_CDECL);
	engine->SetDefaultNamespace("");

	engine->RegisterEnum("EventType");
	engine->RegisterEnumValue("EventType", "KeyPressed",	0);
	engine->RegisterEnumValue("EventType", "KeyReleased",	1);
	engine->RegisterEnumValue("EventType", "KeyCharacter",	2);
	engine->RegisterEnumValue("EventType", "MouseMoved",	3);
	engine->RegisterEnumValue("EventType", "MouseScrolled", 4);
	engine->RegisterEnumValue("EventType", "MousePressed",	5);
	engine->RegisterEnumValue("EventType", "MouseReleased", 6);

	engine->RegisterObjectType("KeyEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty("KeyEvent", "const EventType Type",
		asOFFSET(KeyEvent, Type));
	engine->RegisterObjectProperty("KeyEvent", "const Key Key",
		asOFFSET(KeyEvent, Key));

	engine->RegisterObjectType("KeyPressedEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty("KeyPressedEvent", "const EventType Type",
		asOFFSET(KeyPressedEvent, Type));
	engine->RegisterObjectProperty("KeyPressedEvent", "const Key Key",
		asOFFSET(KeyPressedEvent, Key));
	engine->RegisterObjectProperty("KeyPressedEvent", "const bool IsRepeat",
		asOFFSET(KeyPressedEvent, IsRepeat));

	engine->RegisterObjectType("KeyReleasedEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty("KeyReleasedEvent", "const EventType Type",
		asOFFSET(KeyReleasedEvent, Type));
	engine->RegisterObjectProperty("KeyReleasedEvent", "const Key Key",
		asOFFSET(KeyReleasedEvent, Key));

	engine->RegisterObjectType("KeyCharacterEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty("KeyCharacterEvent", "const EventType Type",
		asOFFSET(KeyCharEvent, Type));
	engine->RegisterObjectProperty("KeyCharacterEvent", "const Key Key",
		asOFFSET(KeyCharEvent, Key));
	engine->RegisterObjectMethod("KeyCharacterEvent", "string get_Char() const property",
		asMETHOD(KeyCharEvent, ToString), asCALL_THISCALL);

	engine->RegisterObjectMethod("KeyEvent", "KeyPressedEvent@ opCast()",
		asFUNCTION(KeyPressedEventCast), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("KeyEvent", "KeyReleasedEvent@ opCast()",
		asFUNCTION(KeyReleasedEventCast), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("KeyEvent", "KeyCharacterEvent@ opCast()",
		asFUNCTION(KeyCharacterEventCast), asCALL_CDECL_OBJLAST);

	engine->RegisterObjectType("MouseEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	// engine->RegisterObjectProperty("MouseEvent", "const EventType Type",
	// 	asOFFSET(MouseEvent, Type));

	// engine->RegisterObjectType("MousePressedEvent", 0, asOBJ_REF | asOBJ_NOCOUNT);
	// engine->RegisterObjectProperty("MousePressedEvent", "const EventType Type",
	// 	asOFFSET(MouseButtonPressedEvent, Type));
	// engine->RegisterObjectProperty("MousePressedEvent", "const Mouse Button",
	// 	asOFFSET(MouseButtonPressedEvent, Button));

}

// static void AssetDefaultCtor(Asset* ptr) {
// 	new (ptr) Asset{ };
// }

// static void AssetInitCtor(uint64_t id, AssetType type, Asset* ptr) {
// 	new (ptr) Asset{ id, type };
// }

// static uint64_t GetAssetID(Asset* asset) {
// 	return (uint64_t)asset->ID;
// }

// static bool AssetIsValid(Asset* asset) {
// 	return AssetManager::Get()->IsValid(*asset);
// }

// static bool AssetIsLoaded(Asset* asset) {
// 	return AssetManager::Get()->IsLoaded(*asset);
// }

// static std::string AssetGetName(Asset* asset) {
// 	return AssetManager::Get()->GetAssetName(*asset);
// }

// static Sound* GetSound(Asset asset, AssetManager* manager) {
// 	manager->Load(asset);
// 	return manager->Get<Sound>(asset).get();
// }

// static void PlaySound(Sound* sound) {
// 	sound->Play();
// }

// static Component* EntityAddComponent(const std::string& name, Entity* entity) {

// }

// static const Component* EntityGetComponent(const std::string& name, const Entity* entity) {

// }

// static Component* EntitySetComponent(const std::string& name, Entity* entity) {

// }

void RegisterECS() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterObjectType("Entity", sizeof(Entity),
		asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLINTS |
		asGetTypeTraits<Entity>());
	engine->RegisterObjectMethod("Entity", "string get_Name() const property",
		asMETHOD(Entity, GetName), asCALL_THISCALL);
	engine->RegisterObjectMethod("Entity", "void set_Name(const string &in) property",
		asMETHOD(Entity, SetName), asCALL_THISCALL);
	engine->RegisterObjectMethod("Entity", "bool get_IsAlive() const property",
		asMETHOD(Entity, IsAlive), asCALL_THISCALL);
	engine->RegisterObjectMethod("Entity", "bool get_IsValid() const property",
		asMETHOD(Entity, IsValid), asCALL_THISCALL);
	engine->RegisterObjectMethod("Entity", "void Kill()",
		asMETHOD(Entity, Kill), asCALL_THISCALL);

	// engine->RegisterObjectMethod("Entity", "bool Has(const string &in) const",
	// 	asMETHODPR(Entity, Has, (const std::string& name) const, bool),
	// 	asCALL_THISCALL);

	// engine->RegisterObjectMethod("Entity",
	// 	"Component@ Add(const string &in)",
	// 	asFUNCTION(EntityAddComponent), asCALL_CDECL_OBJLAST);

	// engine->RegisterObjectMethod("Entity",
	// 	"const Component@ Get(const string &in) const",
	// 	asFUNCTION(EntityGetComponent), asCALL_CDECL_OBJLAST);

	// engine->RegisterObjectMethod("Entity",
	// 	"Component@ Set(const string &in)",
	// 	asFUNCTION(EntitySetComponent), asCALL_CDECL_OBJLAST);
}

}