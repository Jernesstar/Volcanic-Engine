#include "SceneLoader.h"

#include <bitset>

#include <angelscript/add_on/scriptarray/scriptarray.h>

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/UUID.h>
#include <Engine/Graphics/StereographicCamera.h>
#include <Engine/Graphics/OrthographicCamera.h>

#include <Engine/Core/YAMLSerializer.h>

#include <Engine/Core/BinaryWriter.h>
#include <Engine/Core/BinaryReader.h>

#include <Engine/Scene/Component.h>

#include <Engine/Core/App.h>
#include <EngineTypes/GridSet.h>
#include <EngineTypes/GridSet3D.h>
#include <EngineTypes/Timer.h>

#undef near
#undef far

using namespace VolcanicEngine;
using namespace VolcanicEngine::ECS;
using namespace VolcanicEngine::Physics;

namespace VolcanicRuntime {

template<>
BinaryReader& BinaryReader::ReadObject(glm::vec3& vec) {
	Read(vec.x);
	Read(vec.y);
	Read(vec.z);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(CameraComponent& comp) {
	uint32_t typeInt;
	Read(typeInt);
	auto type = (Camera::Type)typeInt;

	float rotation_fov;
	Read(rotation_fov);
	glm::vec3 pos, dir;
	Read(pos); Read(dir);
	uint32_t w, h;
	Read(w); Read(h);
	float near, far;
	Read(near); Read(far);
	
	comp.Cam = Camera::Create(type, rotation_fov);
	comp.Cam->SetPositionDirection(pos, dir);
	comp.Cam->SetProjection(near, far);
	comp.Cam->Resize(w, h);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(TagComponent& comp) {
	Read(comp.Tag);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(TransformComponent& comp) {
	Read(comp.Translation);
	Read(comp.Rotation);
	Read(comp.Scale);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(AudioComponent& comp) {
	uint64_t id;
	Read(id);
	comp.AudioAsset = { id, AssetType::Audio };

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(MeshComponent& comp) {
	uint64_t geometryID;
	Read(geometryID);
	comp.GeometryAsset = { geometryID, AssetType::Geometry };

	uint64_t materialID;
	Read(materialID);
	comp.MaterialAsset = { materialID, AssetType::Material };

	// MaterialInstance override block (Sprint 65) — byte-identical to the Editor
	// writer (SceneLoader.cpp) and LoadFromBytes<MaterialInstance>.
	uint64_t parentId; uint8_t parentType;
	Read(parentId); Read(parentType);
	comp.Instance.ParentAsset = { parentId, (AssetType)parentType };

	uint32_t overrideCount;
	Read(overrideCount);
	for(uint32_t i = 0; i < overrideCount; i++) {
		std::string name; uint8_t type;
		Read(name); Read(type);
		auto propType = (Graphics::ShaderPropType)type;
		Graphics::MatProp prop; prop.Type = propType;
		switch(propType) {
		case Graphics::ShaderPropType::Int:   { int32_t v; Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Float: { float v;   Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Vec2:  { glm::vec2 v; Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Vec3:  { glm::vec3 v; Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Vec4:  { glm::vec4 v; Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Mat4:  { glm::mat4 v; Read(v); prop.Value = v; break; }
		case Graphics::ShaderPropType::Texture: {
			uint64_t tid; uint8_t tt;
			Read(tid); Read(tt);
			prop.Value = Asset{ tid, (AssetType)tt };
			break;
		}
		default: break;
		}
		comp.Instance.Overrides[name] = prop;
	}

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(SkyboxComponent& comp) {
	uint64_t id;
	Read(id);
	comp.CubemapAsset = { id, AssetType::Cubemap };

	return *this;
}

static Entity s_CurrentEntity;

template<>
BinaryReader& BinaryReader::ReadObject(ScriptComponent& comp) {
	uint64_t id;
	Read(id);
	comp.ModuleAsset = { id, AssetType::Script };

	std::string className;
	Read(className);

	auto* assetManager = AssetManager::Get();
	assetManager->Load(comp.ModuleAsset);

	auto mod = assetManager->Get<ScriptModule>(comp.ModuleAsset);
	auto _class = mod->GetClass(className);
	comp.Instance = _class->Instantiate(s_CurrentEntity);

	auto obj = comp.Instance;
	for(uint32_t i = 0; i < obj->GetHandle()->GetPropertyCount(); i++) {
		int typeID;
		Read(typeID);
		if(typeID == -1)
			continue;

		ScriptField field = obj->GetProperty(i);
		std::string typeName;
		if(field.Type)
			typeName = field.Type->GetName();

		if(field.TypeID == asTYPEID_BOOL)
			Read(*field.As<bool>());
		else if(field.TypeID == asTYPEID_INT8)
			Read(*field.As<int8_t>());
		else if(field.TypeID == asTYPEID_INT16)
			Read(*field.As<int16_t>());
		else if(field.TypeID == asTYPEID_INT32)
			Read(*field.As<int32_t>());
		else if(field.TypeID == asTYPEID_INT64)
			Read(*field.As<int64_t>());
		else if(field.TypeID == asTYPEID_UINT8)
			Read(*field.As<uint8_t>());
		else if(field.TypeID == asTYPEID_UINT16)
			Read(*field.As<uint16_t>());
		else if(field.TypeID == asTYPEID_UINT32)
			Read(*field.As<uint32_t>());
		else if(field.TypeID == asTYPEID_UINT64)
			Read(*field.As<uint64_t>());
		else if(field.TypeID == asTYPEID_FLOAT)
			Read(*field.As<float>());
		else if(field.TypeID == asTYPEID_DOUBLE)
			Read(*field.As<double>());
		else if(typeName == "string")
			Read(*field.As<std::string>());
		else if(typeName == "array") {
			auto* array = field.As<CScriptArray>();
			auto subTypeID = array->GetArrayObjectType()->GetSubTypeId();
			auto* subType = ScriptEngine::Get()->GetTypeInfoById(subTypeID);
			uint64_t size = 0;
			if(subType)
				size = subType->GetSize();
			else
				size = ScriptEngine::Get()->GetSizeOfPrimitiveType(subTypeID);

			uint32_t count;
			Read(count);
			Buffer<void> data(size, count);
			ReadData(data.Get(), (uint64_t)size * count);

			array->Reserve(count);
			for(uint32_t i = 0; i < count; i++)
				array->InsertLast((char*)data.Get() + size * i);
		}
		else if(typeName == "Asset") {
			auto* asset = field.As<Asset>();

			uint64_t id;
			Read(id);
			asset->ID = id;
			uint8_t type;
			Read(type);
			asset->Type = (AssetType)type;
		}
		else if(typeName == "Vec3")
			Read(*field.As<glm::vec3>());
		else if(typeName == "GridSet") {
			auto* grid = field.As<GridSet>();
			uint32_t width;
			uint32_t height;
			Read(width);
			Read(height);
			grid->Resize(width, height);
			if(width && height)
				ReadData(grid->Get(), grid->GetCount());
		}
	}

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(RigidBodyComponent& comp) {
	uint8_t typeInt;
	Read(typeInt);
	comp.Type = (RigidBodyComponent::BodyType)typeInt;

	uint8_t shapeTypeInt;
	Read(shapeTypeInt);
	comp.Shape = (RigidBodyComponent::ShapeType)shapeTypeInt;

	Read(comp.HalfExtents);
	Read(comp.IsTrigger);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(DirectionalLightComponent& comp) {
	Read(comp.Ambient);
	Read(comp.Diffuse);
	Read(comp.Specular);
	Read(comp.Position);
	Read(comp.Direction);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(PointLightComponent& comp) {
	Read(comp.Ambient);
	Read(comp.Diffuse);
	Read(comp.Specular);
	Read(comp.Position);
	Read(comp.Constant);
	Read(comp.Linear);
	Read(comp.Quadratic);
	Read(comp.Bloom);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(SpotlightComponent& comp) {
	Read(comp.Ambient);
	Read(comp.Diffuse);
	Read(comp.Specular);
	Read(comp.Position);
	Read(comp.Direction);
	Read(comp.CutoffAngle);
	Read(comp.OuterCutoffAngle);

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(ParticleEmitterComponent& comp) {
	// Versioned format (Sprint 64, task 1.2): bail loudly on a stale binary rather
	// than reading misaligned garbage. Keep in sync with the Editor writer.
	uint32_t version;
	Read(version);
	if(version != k_EmitterFormatVersion) {
		VolcaniCore::Log::Error(
			"ParticleEmitterComponent format version {}, expected {}. "
			"Scene binary is stale — re-export from the Editor.",
			version, k_EmitterFormatVersion);
		return *this;
	}

	Read(comp.LocalOffset);
	Read(comp.LightOffset);
	Read(comp.SpawnExtents);
	Read(comp.MaxParticleCount);
	Read(comp.ParticleLifetime);
	Read(comp.SpawnInterval);
	Read(comp.Color);
	Read(comp.Size);
	Read(comp.EmitsLight);
	Read(comp.LightRadius);
	Read(comp.SpawnJitter);
	Read(comp.LightFlicker);
	Read(comp.LightFlickerSpeed);
	Read(comp.ColorStart);
	Read(comp.ColorMid);
	Read(comp.ColorEnd);

	uint64_t id;
	Read(id);
	comp.MaterialAsset = { id, AssetType::Material };

	return *this;
}

template<>
BinaryReader& BinaryReader::ReadObject(Entity& entity) {
	std::string name;
	Read(name);
	if(name != "")
		entity.SetName(name);

	s_CurrentEntity = entity;

	uint16_t bits;
	Read(bits);
	std::bitset<12> componentBits(bits);

	if(componentBits.test(0))
		Read(entity.Set<CameraComponent>());
	if(componentBits.test(1))
		Read(entity.Set<TagComponent>());
	if(componentBits.test(2))
		Read(entity.Set<TransformComponent>());
	if(componentBits.test(3))
		Read(entity.Set<AudioComponent>());
	if(componentBits.test(4))
		Read(entity.Set<MeshComponent>());
	if(componentBits.test(5))
		Read(entity.Set<SkyboxComponent>());
	if(componentBits.test(6))
		Read(entity.Set<ScriptComponent>());
	if(componentBits.test(7))
		Read(entity.Set<RigidBodyComponent>());
	if(componentBits.test(8))
		Read(entity.Set<DirectionalLightComponent>());
	if(componentBits.test(9))
		Read(entity.Set<PointLightComponent>());
	if(componentBits.test(10))
		Read(entity.Set<SpotlightComponent>());
	if(componentBits.test(11)) {
		Read(entity.Set<ParticleEmitterComponent>());
		entity.GetHandle().modified<ParticleEmitterComponent>();
	}

	return *this;
}

}

namespace VolcanicRuntime {

void SceneLoader::Load(Scene& scene, const std::string& path) {
	namespace fs = std::filesystem;

	BinaryReader reader(path);
	reader.Read(scene.Name);

	uint64_t entityCount;
	reader.Read(entityCount);

	for(uint64_t i = 0; i < entityCount; i++) {
		uint64_t id;
		reader.Read(id);
		Entity entity = scene.World3D.AddEntity(id);
		reader.Read(entity);
	}
}

void SceneLoader::Save(const Scene& scene, const std::string& path) {
	namespace fs = std::filesystem;

	BinaryWriter writer(path);

	writer.Write(scene.Name);

	scene.World3D
	.ForEach(
		[&](const Entity& entity)
		{
			// writer.Write(entity);
		});
}

}