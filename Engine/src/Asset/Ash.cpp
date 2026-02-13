#include "Ash.h"

#include "AssetManager.h"

#include <Lava/Script/ScriptEngine.h>

using namespace Lava::Script;

namespace Lava {

static void AshRegisterInterface();

void Ash::Init() {
	AshRegisterInterface();
}

void Ash::Shutdown() {

}

void Ash::BeginFrame() {

}

void Ash::EndFrame() {

}

void Ash::OnUpdate(TimeStep ts) {

}

void Ash::OnEvent(uint32_t event) {
	AshEvent e = (AshEvent)event;
}

static void AssetDefaultCtor(Asset* ptr) {
	new (ptr) Asset{ };
}

void AshRegisterInterface() {
	auto* engine = ScriptEngine::Get();

	engine->RegisterEnum("AssetType");
	engine->RegisterEnumValue("AssetType", "None",    0);
	engine->RegisterEnumValue("AssetType", "Mesh",	  1);
	engine->RegisterEnumValue("AssetType", "Texture", 2);
	engine->RegisterEnumValue("AssetType", "Cubemap", 3);
	engine->RegisterEnumValue("AssetType", "Font",	  4);
	engine->RegisterEnumValue("AssetType", "Audio",	  5);
	engine->RegisterEnumValue("AssetType", "Script",  6);
	engine->RegisterEnumValue("AssetType", "Shader",  7);
	engine->RegisterEnumValue("AssetType", "Custom",  8);

	// engine->RegisterObjectType("Asset", sizeof(Asset),
	// 	asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Asset>());
	// engine->RegisterObjectBehaviour("Asset", asBEHAVE_CONSTRUCT,
	// 	"void f()", asFUNCTION(AssetDefaultCtor), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectBehaviour("Asset", asBEHAVE_CONSTRUCT,
	// 	"void f(uint64, AssetType)", asFUNCTION(AssetInitCtor),
	// 	asCALL_CDECL_OBJLAST);
}

// static Sound* GetSound(Asset asset, AssetManager* manager) {
// 	manager->Load(asset);
// 	return manager->Get<Sound>(asset).get();
// }

// static void PlaySound(Sound* sound) {
// 	sound->Play();
// }

void RegisterAssetManager() {
	// engine->RegisterObjectProperty("Asset", "const AssetType Type",
	// 	asOFFSET(Asset, Type));
	// engine->RegisterObjectProperty("Asset", "const bool Primary",
	// 	asOFFSET(Asset, Primary));
	// engine->RegisterObjectMethod("Asset", "uint64 get_ID() const property",
	// 	asFUNCTION(GetAssetID), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectMethod("Asset", "bool get_IsValid() const property",
	// 	asFUNCTION(AssetIsValid), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectMethod("Asset", "bool get_IsLoaded() const property",
	// 	asFUNCTION(AssetIsLoaded), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectMethod("Asset", "string get_Name() const property",
	// 	asFUNCTION(AssetGetName), asCALL_CDECL_OBJLAST);

	// engine->RegisterObjectType("AssetManagerClass", 0,
	// 	asOBJ_REF | asOBJ_NOHANDLE);
	// engine->RegisterObjectMethod("AssetManagerClass", "bool Load(Asset)",
	// 	asMETHOD(AssetManager, Load), asCALL_THISCALL);
	// engine->RegisterObjectMethod("AssetManagerClass", "bool Unload(Asset)",
	// 	asMETHOD(AssetManager, Unload), asCALL_THISCALL);
	// engine->RegisterObjectMethod("AssetManagerClass",
	// 	"Asset GetNamedAsset(const string &in)",
	// 	asMETHOD(AssetManager, GetNamedAsset), asCALL_THISCALL);
	// engine->RegisterObjectMethod("AssetManagerClass",
	// 	"Asset GetNativeAsset(const string &in)",
	// 	asMETHOD(AssetManager, GetNativeAsset), asCALL_THISCALL);

	// engine->RegisterObjectType("Sound", 0, asOBJ_REF | asOBJ_NOCOUNT);
	// engine->RegisterObjectMethod("Sound", "void Play(float volume)",
	// 	asMETHOD(Sound, Play), asCALL_THISCALL);
	// engine->RegisterObjectMethod("Sound", "void Play()",
	// 	asFUNCTION(PlaySound), asCALL_CDECL_OBJLAST);
	// engine->RegisterObjectMethod("AssetManagerClass",
	// 	"Sound@ GetSound(Asset)", asFUNCTION(GetSound), asCALL_CDECL_OBJLAST);
}

}

