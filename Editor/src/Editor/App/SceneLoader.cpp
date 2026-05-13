#include "SceneLoader.h"

#include <bitset>

#include <angelscript/sdk/add_on/scriptarray/scriptarray.h>

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/UUID.h>
#include <VolcaniCore/Utils/BinaryWriter.h>
#include <VolcaniCore/Utils/BinaryReader.h>

#include <Engine/App/App.h>
#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Script/ScriptClass.h>
#include <Engine/Script/Types/GridSet.h>
#include <Engine/Script/Types/GridSet3D.h>
#include <Engine/Script/Types/Timer.h>
#include <Engine/Scene/Component.h>

#include "../Asset/AssetManager.h"
#include "ScriptManager.h"

#undef near
#undef far

namespace VolcanicEditor {

template<>
Serializer& Serializer::Write(const Vertex& value) {
	SetOptions(Serializer::Options::ArrayOneLine);
	BeginSequence();
		Write(value.Position);
		Write(value.Normal);
		Write(value.TexCoord);
	EndSequence();
	return *this;
}

template<>
Serializer& Serializer::Write(const Asset& value) {
	BeginMapping();
		WriteKey("ID").Write((u64)value.ID);
		WriteKey("Type").Write(AssetTypeToString(value.Type));
	EndMapping();
	return *this;
}

static void DeserializeEntity(YAML::Node entityNode, World& scene);
static void SerializeEntity(YAMLSerializer& out, const Entity& entity);

void SceneLoader::EditorLoad(Scene& scene, const std::string& path) {
	YAML::Node file;
	try {
		file = YAML::LoadFile(path);
	}
	catch(YAML::ParserException e) {
		VOLCANICORE_ASSERT_ARGS(false, "Could not load file %s: %s",
								path.c_str(), e.what());
	}
	auto sceneNode = file["Scene"];

	VOLCANICORE_ASSERT(sceneNode);

	scene.Name = sceneNode["Name"].as<std::string>();

	for(auto node : sceneNode["World3D"])
		DeserializeEntity(node["Entity"], scene.World3D);
	for(auto node : sceneNode["World2D"])
		DeserializeEntity(node["Entity"], scene.World2D);
	for(auto node : sceneNode["Canvas"])
		DeserializeEntity(node["Entity"], scene.Canvas);

	Log::Info("Loaded scene {} from path {}", scene.Name, path);
	scene.World3D.ForEach([](Entity& e) { Log::Info("N: {}", e.GetName()); });
	Log::Info("Tag: {}", scene.World3D.GetEntity("Camera").Get<TagComponent>().Tag);
}

void SceneLoader::EditorSave(const Scene& scene, const std::string& path) {
	YAMLSerializer serializer;
	serializer.BeginMapping(); // File

	serializer
	.WriteKey("Scene").BeginMapping()
		.WriteKey("Name").Write(scene.Name);

	serializer.WriteKey("World3D").BeginSequence(); // World3D
	scene.World3D
	.ForEach(
		[&](const Entity& entity)
		{
			serializer.BeginMapping(); // Entity
			SerializeEntity(serializer, entity);
			serializer.EndMapping(); // Entity
		});
	serializer.EndSequence(); // World3D

	serializer.WriteKey("World2D").BeginSequence(); // World2D
	scene.World2D
	.ForEach(
		[&](const Entity& entity)
		{
			serializer.BeginMapping(); // Entity
			SerializeEntity(serializer, entity);
			serializer.EndMapping(); // Entity
		});
	serializer.EndSequence(); // World2D

	serializer.WriteKey("Canvas").BeginSequence(); // Canvas
	scene.Canvas
	.ForEach(
		[&](const Entity& entity)
		{
			serializer.BeginMapping(); // Entity
			SerializeEntity(serializer, entity);
			serializer.EndMapping(); // Entity
		});
	serializer.EndSequence(); // Canvas

	serializer.EndMapping(); // Scene

	serializer.EndMapping(); // File

	serializer.Finalize(path);
}

void SaveScript(YAMLSerializer& serializer, Ref<ScriptObject> obj) {
	serializer.WriteKey("Class").Write(obj->GetClass()->Name);
	serializer.WriteKey("Fields").BeginSequence();

	auto* handle = obj->GetHandle();
	for(u32 i = 0; i < handle->GetPropertyCount(); i++) {
		ScriptField field = obj->GetProperty(i);
		bool editorField =
			ScriptManager::FieldHasMetadata(
				obj->GetClass()->Name, field.Name, "EditorField");
		if(!editorField)
			continue;

		serializer.BeginMapping()
			.WriteKey("Field").BeginMapping();

		serializer.WriteKey("Name").Write(field.Name);

		if(field.TypeID == asTYPEID_BOOL) {
			serializer
				.WriteKey("Type").Write(std::string("bool"))
				.WriteKey("Value").Write(*field.As<bool>());
		}
		else if(field.TypeID == asTYPEID_INT8) {
			serializer
				.WriteKey("Type").Write(std::string("int8"))
				.WriteKey("Value").Write((i32)*field.As<i8>());
		}
		else if(field.TypeID == asTYPEID_INT16) {
			serializer
				.WriteKey("Type").Write(std::string("int16"))
				.WriteKey("Value").Write((i32)*field.As<i16>());
		}
		else if(field.TypeID == asTYPEID_INT32) {
			serializer
				.WriteKey("Type").Write(std::string("int32"))
				.WriteKey("Value").Write(*field.As<i32>());
		}
		else if(field.TypeID == asTYPEID_INT64) {
			serializer
				.WriteKey("Type").Write(std::string("int64"))
				.WriteKey("Value").Write(*field.As<i64>());
		}
		else if(field.TypeID == asTYPEID_UINT8) {
			serializer
				.WriteKey("Type").Write(std::string("uint8"))
				.WriteKey("Value").Write((u32)*field.As<u8>());
		}
		else if(field.TypeID == asTYPEID_UINT16) {
			serializer
				.WriteKey("Type").Write(std::string("uint16"))
				.WriteKey("Value").Write((u32)*field.As<u16>());
		}
		else if(field.TypeID == asTYPEID_UINT32) {
			serializer
				.WriteKey("Type").Write(std::string("uint32"))
				.WriteKey("Value").Write(*field.As<u32>());
		}
		else if(field.TypeID == asTYPEID_UINT64) {
			serializer
				.WriteKey("Type").Write(std::string("uint64"))
				.WriteKey("Value").Write(*field.As<u64>());
		}
		else if(field.TypeID == asTYPEID_FLOAT) {
			serializer
				.WriteKey("Type").Write(std::string("float"))
				.WriteKey("Value").Write(*field.As<f32>());
		}
		else if(field.TypeID == asTYPEID_DOUBLE) {
			serializer
				.WriteKey("Type").Write(std::string("double"))
				.WriteKey("Value").Write(*(f32*)field.As<f64>());
		}
		else if(std::string(field.Type->GetName()) == "string") {
			serializer
				.WriteKey("Type").Write(std::string("array"))
				.WriteKey("Value").Write(*field.As<std::string>());
		}
		else if(std::string(field.Type->GetName()) == "array") {
			serializer
				.WriteKey("Type").Write(std::string("array"))
				.WriteKey("Value");

			serializer.SetOptions(Serializer::Options::ArrayOneLine);
			serializer
					.BeginSequence();
			auto* data = field.As<CScriptArray>();
			for(u32 i = 0; i < data->GetSize(); i++)
				serializer
					.Write(*(u32*)data->At(i));

			serializer
				.EndSequence();
		}
		else if(std::string(field.Type->GetName()) == "Asset") {
			serializer
				.WriteKey("Type").Write(std::string("Asset"))
				.WriteKey("Value").Write(*field.As<Asset>());
		}
		else if(std::string(field.Type->GetName()) == "Vec3") {
			serializer
				.WriteKey("Type").Write(std::string("Vec3"))
				.WriteKey("Value").Write(*field.As<Vec3>());
		}
		else if(std::string(field.Type->GetName()) == "Vec2") {
			serializer
				.WriteKey("Type").Write(std::string("Vec2"))
				.WriteKey("Value").Write(*field.As<Vec2>());
		}
		else if(std::string(field.Type->GetName()) == "GridSet") {
			serializer
				.WriteKey("Type").Write(std::string("GridSet"))
				.WriteKey("Value").BeginMapping();

			auto* grid = field.As<GridSet>();
			serializer.WriteKey("Width").Write(grid->GetWidth());
			serializer.WriteKey("Height").Write(grid->GetHeight());

			serializer.WriteKey("Data")
				.BeginSequence();
			for(u32 y = 0; y < grid->GetHeight(); y++) {
				serializer.SetOptions(Serializer::Options::ArrayOneLine);
				serializer.BeginSequence();
				for(u32 x = 0; x < grid->GetWidth(); x++)
					serializer.Write(*grid->At(x, y));
				serializer.EndSequence();
			}
			serializer.EndSequence();

			serializer.EndMapping();
		}
		// Script Type
		else if(field.Is(ScriptQualifier::ScriptObject)) {
			// auto* type = field.Type;
			// for(u32 i = 0; i < type->GetPropertyCount(); i++) {
			// 	u64 offset;
			// 	type->GetProperty()
			// }
		}

			serializer.EndMapping()
		.EndMapping();
	}

	serializer
		.EndSequence();
}

void SerializeEntity(YAMLSerializer& serializer, const Entity& entity) {
	serializer.WriteKey("Entity").BeginMapping(); // Entity

	serializer.WriteKey("Name").Write(entity.GetName());
	serializer.WriteKey("ID").Write((u64)entity.GetHandle());

	serializer.WriteKey("Components")
	.BeginMapping(); // Components

	if(entity.Has<TagComponent>()) {
		auto& tag = entity.Get<TagComponent>().Tag;

		serializer.WriteKey("TagComponent")
		.BeginMapping()
			.WriteKey("Tag").Write(tag)
		.EndMapping();
	}
	if(entity.Has<CameraComponent>()) {
		auto& camera = entity.Get<CameraComponent>().Cam;
		auto type = camera->GetType();
		std::string s;
		switch(type) {
			case Camera::Type::Orthographic:
				s = "Orthographic";
				break;
			case Camera::Type::Stereographic:
				s = "Stereographic";
				break;
			case Camera::Type::Isometric:
				s = "Isometric";
				break;
		}

		serializer.WriteKey("CameraComponent")
		.BeginMapping()
			.WriteKey("Camera").BeginMapping()
			.WriteKey("Type").Write(s);

		if(type == Camera::Type::Orthographic)
			serializer
			.WriteKey("Rotation")
			.Write(camera->As<OrthographicCamera>()->GetRotation());
		if(type == Camera::Type::Stereographic)
			serializer
			.WriteKey("VerticalFOV")
			.Write(camera->As<StereographicCamera>()->GetVerticalFOV());
		else if(type == Camera::Type::Isometric)
			serializer
			.WriteKey("Distance")
			.Write(camera->As<IsometricCamera>()->R);

		serializer
			.WriteKey("Position").Write(camera->GetPosition())
			.WriteKey("Direction").Write(camera->GetDirection())
			.WriteKey("ViewportWidth").Write(camera->GetViewportWidth())
			.WriteKey("ViewportHeight").Write(camera->GetViewportHeight())
			.WriteKey("Near").Write(camera->GetNear())
			.WriteKey("Far").Write(camera->GetFar())
		.EndMapping()
		.EndMapping();
	}
	if(entity.Has<TransformComponent>()) {
		const auto& transform = entity.Get<TransformComponent>();

		serializer.WriteKey("TransformComponent")
		.BeginMapping()
			.WriteKey("Transform").BeginMapping()
				.WriteKey("Translation").Write(transform.Translation)
				.WriteKey("Rotation")	.Write(transform.Rotation)
				.WriteKey("Scale")		.Write(transform.Scale)
			.EndMapping()
		.EndMapping(); // TransformComponent
	}
	if(entity.Has<AudioComponent>()) {
		Asset asset = entity.Get<AudioComponent>().AudioAsset;
		serializer.WriteKey("AudioComponent")
		.BeginMapping()
			.WriteKey("AssetID").Write((u64)asset.ID)
		.EndMapping();
	}
	if(entity.Has<MeshComponent>()) {
		Asset meshSource = entity.Get<MeshComponent>().MeshSourceAsset;
		Asset material = entity.Get<MeshComponent>().MaterialAsset;
		serializer.WriteKey("MeshComponent")
		.BeginMapping()
			.WriteKey("MeshSourceID").Write((u64)meshSource.ID)
			.WriteKey("MaterialID").Write((u64)material.ID)
		.EndMapping();
	}
	if(entity.Has<SkyboxComponent>()) {
		Asset asset = entity.Get<SkyboxComponent>().CubemapAsset;
		serializer.WriteKey("SkyboxComponent")
		.BeginMapping()
			.WriteKey("AssetID").Write((u64)asset.ID)
		.EndMapping();
	}
	if(entity.Has<ScriptComponent>()) {
		const auto& comp = entity.Get<ScriptComponent>();
		auto obj = comp.Instance;

		serializer.WriteKey("ScriptComponent")
		.BeginMapping()
			.WriteKey("ModuleID").Write((u64)comp.ModuleAsset.ID);

		if(obj)
			SaveScript(serializer, obj);

		serializer.EndMapping();
	}
	if(entity.Has<RigidBodyComponent>()) {
		// auto body = entity.Get<RigidBodyComponent>().Body;
		// serializer.WriteKey("RigidBodyComponent")
		// .BeginMapping();

		// if(body) {
		// 	auto type = body->GetType();
		// 	auto t = type == RigidBody::Type::Static ? "Static" : "Dynamic";

		// 	serializer.WriteKey("Body")
		// 		.BeginMapping()
		// 			.WriteKey("Type").Write(t);

		// 	if(body->HasShape()) {
		// 		std::string shapeType;
		// 		switch(body->GetShape()->GetType()) {
		// 			case Shape::Type::Box:
		// 				shapeType = "Box";
		// 				break;
		// 			case Shape::Type::Sphere:
		// 				shapeType = "Sphere";
		// 				break;
		// 			case Shape::Type::Plane:
		// 				shapeType = "Plane";
		// 				break;
		// 			case Shape::Type::Capsule:
		// 				shapeType = "Capsule";
		// 				break;
		// 			case Shape::Type::Mesh:
		// 				shapeType = "Mesh";
		// 				break;
		// 		}

		// 		serializer.WriteKey("ShapeType").Write(shapeType);
		// 	}

		// 	serializer.EndMapping(); // Body
		// }
		// serializer.EndMapping(); // RigidBodyComponent
	}
	if(entity.Has<DirectionalLightComponent>()) {
		const auto& light = entity.Get<DirectionalLightComponent>();

		serializer.WriteKey("DirectionalLightComponent")
		.BeginMapping()
			.WriteKey("Light")
			.BeginMapping()
				.WriteKey("Ambient").Write(light.Ambient)
				.WriteKey("Diffuse").Write(light.Diffuse)
				.WriteKey("Specular").Write(light.Specular)
				.WriteKey("Position").Write(light.Position)
				.WriteKey("Direction").Write(light.Direction)
			.EndMapping()
		.EndMapping(); // DirectionalLightComponent
	}
	if(entity.Has<PointLightComponent>()) {
		const auto& light = entity.Get<PointLightComponent>();
		
		serializer.WriteKey("PointLightComponent")
		.BeginMapping()
			.WriteKey("Light")
			.BeginMapping()
				.WriteKey("Ambient").Write(light.Ambient)
				.WriteKey("Diffuse").Write(light.Diffuse)
				.WriteKey("Specular").Write(light.Specular)
				.WriteKey("Position").Write(light.Position)
				.WriteKey("Constant").Write(light.Constant)
				.WriteKey("Linear").Write(light.Linear)
				.WriteKey("Quadratic").Write(light.Quadratic)
				.WriteKey("Bloom").Write(light.Bloom)
			.EndMapping()
		.EndMapping(); // PointLightComponent
	}
	if(entity.Has<SpotlightComponent>()) {
		const auto& light = entity.Get<SpotlightComponent>();
	
		serializer.WriteKey("SpotlightComponent")
		.BeginMapping()
			.WriteKey("Light")
			.BeginMapping()
				.WriteKey("Ambient").Write(light.Ambient)
				.WriteKey("Diffuse").Write(light.Diffuse)
				.WriteKey("Specular").Write(light.Specular)
				.WriteKey("Position").Write(light.Position)
				.WriteKey("Direction").Write(light.Direction)
				.WriteKey("CutoffAngle").Write(light.CutoffAngle)
				.WriteKey("OuterCutoffAngle").Write(light.OuterCutoffAngle)
			.EndMapping()
		.EndMapping(); // SpotlightComponent
	}
	if(entity.Has<ParticleEmitterComponent>()) {
		const auto& system = entity.Get<ParticleEmitterComponent>();

		serializer.WriteKey("ParticleEmitterComponent")
		.BeginMapping()
			.WriteKey("Position").Write(system.Position)
			.WriteKey("MaxParticleCount").Write(system.MaxParticleCount)
			.WriteKey("ParticleLifetime").Write(system.ParticleLifetime)
			.WriteKey("SpawnInterval").Write(system.SpawnInterval)
			.WriteKey("Offset").Write(system.Offset)
			.WriteKey("MaterialID").Write((u64)system.MaterialAsset.ID)
		.EndMapping(); // ParticleEmitterComponent
	}

	serializer.EndMapping(); // Components

	serializer.EndMapping(); // Entity
}

Ref<ScriptObject> LoadScript(Entity entity, Asset asset,
	YAML::Node& scriptComponentNode)
{
	auto mod = AssetManager::Get()->Get<ScriptModule>(asset);
	if(!mod) {
		Log::Info(
			"Could not load script module %lu, needed for Entity %lu",
			(u64)asset.ID, (u64)entity.GetHandle());
		return nullptr;
	}

	if(!scriptComponentNode["Class"])
		return nullptr;

	auto className = scriptComponentNode["Class"].as<std::string>();
	auto _class = mod->GetClass(className);
	if(!_class) {
		Log::Info(
			"Could not find class '%s' in module %lu, needed for Entity %lu",
			className.c_str(), (u64)asset.ID, (u64)entity.GetHandle());
		return nullptr;
	}

	auto instance = _class->Construct();
	for(auto fieldNode : scriptComponentNode["Fields"]) {
		auto node = fieldNode["Field"];
		YAML::Node value = node["Value"];
		std::string name = node["Name"].as<std::string>();
		std::string type = node["Type"].as<std::string>();
		void* address = instance->GetProperty(name).Data;
		if(!address)
			continue;

		if(type == "bool")
			*(bool*)address = value.as<bool>();
		else if(type == "int8")
			*(i8*)address = (i8)value.as<i32>();
		else if(type == "int16")
			*(i16*)address = (i16)value.as<i32>();
		else if(type == "int32")
			*(i32*)address = value.as<i32>();
		else if(type == "int64")
			*(i64*)address = value.as<i64>();
		else if(type == "uint8")
			*(u8*)address = (u8)value.as<u32>();
		else if(type == "uint16")
			*(u16*)address = (u16)value.as<u32>();
		else if(type == "uint32")
			*(u32*)address = value.as<u32>();
		else if(type == "uint64")
			*(u64*)address = value.as<u64>();
		else if(type == "float")
			*(f32*)address = value.as<f32>();
		else if(type == "double")
			*(f64*)address = value.as<f32>();
		else if(type == "string")
			*(std::string*)address = value.as<std::string>();
		else if(type == "array") {
			auto data = value.as<List<u32>>();
			for(u32 i = 0; i < data.Count(); i++)
				((CScriptArray*)address)->InsertLast(data.At(i));
		}
		else if(type == "Asset") {
			((Asset*)address)->ID = value["ID"].as<u64>();
			((Asset*)address)->Type =
				AssetTypeFromString(value["Type"].as<std::string>());
		}
		else if(type == "Vec3")
			*(Vec3*)address = value.as<Vec3>();
		else if(type == "GridSet") {
			auto* grid = (GridSet*)address;
			auto width = value["Width"].as<u32>();
			auto height = value["Height"].as<u32>();
			grid->Resize(width, height);

			u32 x = 0, y = 0;
			for(auto row : value["Data"]) {
				for(auto val : row)
					*grid->At(x++, y) = (u8)val.as<u32>();
				x = 0;
				y++;
			}
		}
	}

	Log::Info("Loaded script class {}", className);
	return instance;
}

void DeserializeEntity(YAML::Node entityNode, World& world) {
	Entity entity = world.AddEntity();
	auto nameNode = entityNode["Name"];
	if(nameNode)
		entity.SetName(nameNode.as<std::string>());

	auto components = entityNode["Components"];
	if(!components)
		return;

	auto cameraComponentNode = components["CameraComponent"];
	if(cameraComponentNode) {
		auto cameraNode = cameraComponentNode["Camera"];
		auto pos  = cameraNode["Position"]		.as<Vec3>();
		auto dir  = cameraNode["Direction"]		.as<Vec3>();
		auto w	  = cameraNode["ViewportWidth"] .as<u32>();
		auto h	  = cameraNode["ViewportHeight"].as<u32>();
		auto near = cameraNode["Near"]			.as<f32>();
		auto far  = cameraNode["Far"]			.as<f32>();
		auto typeStr = cameraNode["Type"].as<std::string>();
		Camera::Type type;
		Ref<Camera> camera;

		if(typeStr == "Orthographic") {
			type = Camera::Type::Orthographic;
			f32 rotation = cameraNode["Rotation"].as<f32>();
			camera = CreateRef<OrthographicCamera>(rotation);
		}
		else if(typeStr == "Stereographic") {
			type = Camera::Type::Stereographic;
			f32 fov = cameraNode["VerticalFOV"].as<f32>();
			camera = CreateRef<StereographicCamera>(fov);
		}
		else if(typeStr == "Isometric") {
			type = Camera::Type::Isometric;
			f32 dist = cameraNode["Distance"].as<f32>();
			camera = CreateRef<IsometricCamera>(dist);
		}

		camera->SetPositionDirection(pos, dir);
		camera->SetProjection(near, far);
		camera->Resize(w, h);

		entity.Add<CameraComponent>(camera);
	}

	auto tagComponentNode = components["TagComponent"];
	if(tagComponentNode) {
		auto tag = tagComponentNode["Tag"].as<std::string>();
		entity.Add<TagComponent>(tag);
	}

	auto transformComponentNode = components["TransformComponent"];
	if(transformComponentNode) {
		auto transformNode = transformComponentNode["Transform"];
		entity.Add<TransformComponent>(
			Transform
			{
				.Translation = transformNode["Translation"].as<Vec3>(),
				.Rotation	 = transformNode["Rotation"].as<Vec3>(),
				.Scale		 = transformNode["Scale"].as<Vec3>()
			});
	}

	auto audioComponentNode = components["AudioComponent"];
	if(audioComponentNode) {
		auto id = audioComponentNode["AssetID"].as<u64>();
		entity.Add<AudioComponent>(Asset{ id, AssetType::Audio });
	}

	auto meshComponentNode = components["MeshComponent"];
	if(meshComponentNode) {
		auto sourceID = meshComponentNode["MeshSourceID"].as<u64>();
		auto materialID = meshComponentNode["MaterialID"].as<u64>();
		entity.Add<MeshComponent>(
			Asset{ sourceID, AssetType::Mesh },
			Asset{ materialID, AssetType::Material });
	}

	auto skyboxComponentNode = components["SkyboxComponent"];
	if(skyboxComponentNode) {
		auto id = skyboxComponentNode["AssetID"].as<u64>();
		entity.Add<SkyboxComponent>(Asset{ id, AssetType::Cubemap });
	}

	auto scriptComponentNode = components["ScriptComponent"];
	if(scriptComponentNode) {
		auto id = scriptComponentNode["ModuleID"].as<u64>();
		Asset asset = { id, AssetType::Script };

		if(!asset)
			entity.Add<ScriptComponent>();
		else {
			auto obj = LoadScript(entity, asset, scriptComponentNode);
			entity.Add<ScriptComponent>(asset, obj);
		}
	}

	auto rigidBodyComponentNode = components["RigidBodyComponent"];
	if(rigidBodyComponentNode) {
		auto rigidBodyNode = rigidBodyComponentNode["Body"];
		if(rigidBodyNode) {
			auto typeStr	   = rigidBodyNode["Type"].as<std::string>();
			auto shapeTypeStr  = rigidBodyNode["ShapeType"].as<std::string>();

			// RigidBody::Type type =
			// 	(typeStr == "Static") ? RigidBody::Type::Static
			// 						  : RigidBody::Type::Dynamic;
			// Shape::Type shapeType;
			// if(shapeTypeStr == "Box")	  shapeType = Shape::Type::Box;
			// if(shapeTypeStr == "Sphere")  shapeType = Shape::Type::Sphere;
			// if(shapeTypeStr == "Plane")	  shapeType = Shape::Type::Plane;
			// if(shapeTypeStr == "Capsule") shapeType = Shape::Type::Capsule;
			// if(shapeTypeStr == "Mesh")	  shapeType = Shape::Type::Mesh;
	
			// Ref<RigidBody> body;
			// if(shapeType == Shape::Type::Mesh)
			// 	body = RigidBody::Create(type);
			// else {
			// 	Ref<Shape> shape = Shape::Create(shapeType);
			// 	body = RigidBody::Create(type, shape);
			// }

			// entity.Add<RigidBodyComponent>(body);
		}
		else {
			// entity.Add<RigidBodyComponent>();
		}
	}

	auto directionalLightComponentNode = components["DirectionalLightComponent"];
	if(directionalLightComponentNode) {
		auto lightNode = directionalLightComponentNode["Light"];
		entity.Add<DirectionalLightComponent>(
			lightNode["Ambient"].as<Vec3>(),
			lightNode["Diffuse"].as<Vec3>(),
			lightNode["Specular"].as<Vec3>(),
			lightNode["Position"].as<Vec3>(),
			lightNode["Direction"].as<Vec3>());
	}

	auto pointLightComponentNode = components["PointLightComponent"];
	if(pointLightComponentNode) {
		auto lightNode = pointLightComponentNode["Light"];
		entity.Add<PointLightComponent>(
			lightNode["Ambient"].as<Vec3>(),
			lightNode["Diffuse"].as<Vec3>(),
			lightNode["Specular"].as<Vec3>(),
			lightNode["Position"].as<Vec3>(),
			lightNode["Constant"].as<f32>(),
			lightNode["Linear"].as<f32>(),
			lightNode["Quadratic"].as<f32>(),
			lightNode["Bloom"].as<bool>());
	}

	auto spotlightComponentNode = components["SpotlightComponent"];
	if(spotlightComponentNode) {
		auto lightNode = spotlightComponentNode["Light"];
		entity.Add<SpotlightComponent>(
			lightNode["Ambient"].as<Vec3>(),
			lightNode["Diffuse"].as<Vec3>(),
			lightNode["Specular"].as<Vec3>(),
			lightNode["Position"].as<Vec3>(),
			lightNode["Direction"].as<Vec3>(),
			lightNode["CutoffAngle"].as<f32>(),
			lightNode["OuterCutoffAngle"].as<f32>());
	}

	auto particleEmitterComponentNode = components["ParticleEmitterComponent"];
	if(particleEmitterComponentNode) {
		Asset asset
		{
			particleEmitterComponentNode["MaterialID"].as<u64>(),
			AssetType::Material
		};
		entity.Add<ParticleEmitterComponent>(
			particleEmitterComponentNode["Position"].as<Vec3>(),
			particleEmitterComponentNode["MaxParticleCount"].as<u64>(),
			particleEmitterComponentNode["ParticleLifetime"].as<f32>(),
			particleEmitterComponentNode["SpawnInterval"].as<f32>(),
			particleEmitterComponentNode["Offset"].as<f32>(),
			asset);

		// entity.GetHandle().modified<ParticleEmitterComponent>();
	}
}

}

namespace VolcaniCore {

template<>
BinaryWriter& BinaryWriter::WriteObject(const Vec3& vec) {
	Write(vec.x);
	Write(vec.y);
	Write(vec.z);
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const CameraComponent& comp) {
	auto camera = comp.Cam;

	Write((u32)camera->GetType());
	if(camera->GetType() == Camera::Type::Orthographic)
		Write(camera->As<OrthographicCamera>()->GetRotation());
	else if(camera->GetType() == Camera::Type::Stereographic)
		Write(camera->As<StereographicCamera>()->GetVerticalFOV());
	else if(camera->GetType() == Camera::Type::Isometric)
		Write(camera->As<IsometricCamera>()->R);

	Write(camera->GetPosition());
	Write(camera->GetDirection());
	Write(camera->GetViewportWidth());
	Write(camera->GetViewportHeight());
	Write(camera->GetNear());
	Write(camera->GetFar());

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const TagComponent& comp) {
	Write(comp.Tag);
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const TransformComponent& comp) {
	Write(comp.Translation);
	Write(comp.Rotation);
	Write(comp.Scale);
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const AudioComponent& comp) {
	Write((u64)comp.AudioAsset.ID);

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const MeshComponent& comp) {
	Write((u64)comp.MeshSourceAsset.ID);
	Write((u64)comp.MaterialAsset.ID);
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const SkyboxComponent& comp) {
	Write((u64)comp.CubemapAsset.ID);
	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const ScriptComponent& comp) {
	auto obj = comp.Instance;
	auto* handle = obj->GetHandle();

	Write((u64)comp.ModuleAsset.ID);
	Write(obj->GetClass()->Name);

	for(u32 i = 0; i < handle->GetPropertyCount(); i++) {
		ScriptField field = obj->GetProperty(i);
		bool editorField =
			VolcanicEditor::ScriptManager::FieldHasMetadata(
				comp.Instance->GetClass()->Name, field.Name, "EditorField");
		if(!editorField) {
			Write((int)-1);
			continue;
		}
		Write(field.TypeID);

		std::string typeName;
		if(field.Type)
			typeName = field.Type->GetName();

		if(field.TypeID == asTYPEID_BOOL)
			Write(*field.As<bool>());
		else if(field.TypeID == asTYPEID_INT8)
			Write(*field.As<i8>());
		else if(field.TypeID == asTYPEID_INT16)
			Write(*field.As<i16>());
		else if(field.TypeID == asTYPEID_INT32)
			Write(*field.As<i32>());
		else if(field.TypeID == asTYPEID_INT64)
			Write(*field.As<i64>());
		else if(field.TypeID == asTYPEID_UINT8)
			Write(*field.As<u8>());
		else if(field.TypeID == asTYPEID_UINT16)
			Write(*field.As<u16>());
		else if(field.TypeID == asTYPEID_UINT32)
			Write(*field.As<u32>());
		else if(field.TypeID == asTYPEID_UINT64)
			Write(*field.As<u64>());
		else if(field.TypeID == asTYPEID_FLOAT)
			Write(*field.As<f32>());
		else if(field.TypeID == asTYPEID_DOUBLE)
			Write(*field.As<f64>());
		else if(typeName == "string")
			Write(*field.As<std::string>());
		else if(typeName == "array") {
			auto* array = field.As<CScriptArray>();
			auto subTypeID = array->GetArrayObjectType()->GetSubTypeId();
			auto* subType = ScriptEngine::Get()->GetTypeInfoById(subTypeID);
			u64 size = 0;
			if(subType)
				size = subType->GetSize();
			else
				size = ScriptEngine::Get()->GetSizeOfPrimitiveType(subTypeID);

			u32 count = array->GetSize();
			Write((u32)count);
			// Works for primitive and POD types
			WriteData(array->GetBuffer(), size * count);
		}
		else if(typeName == "Asset") {
			auto asset = *field.As<Asset>();
			Write((u64)asset.ID);
			Write((u8)asset.Type);
		}
		else if(typeName == "Vec3")
			Write(*field.As<Vec3>());
		else if(typeName == "GridSet") {
			auto* grid = field.As<GridSet>();
			Write(grid->GetWidth());
			Write(grid->GetHeight());
			if(*grid)
				WriteData(grid->Get(), grid->GetCount());
		}
	}

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const RigidBodyComponent& comp) {
	// auto body = comp.Body;

	// Write((u8)body->GetType());
	// Write((u8)body->GetShape()->GetType());

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const DirectionalLightComponent& comp) {
	Write(comp.Ambient);
	Write(comp.Diffuse);
	Write(comp.Specular);
	Write(comp.Position);
	Write(comp.Direction);

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const PointLightComponent& comp) {
	Write(comp.Ambient);
	Write(comp.Diffuse);
	Write(comp.Specular);
	Write(comp.Position);
	Write(comp.Constant);
	Write(comp.Linear);
	Write(comp.Quadratic);
	Write(comp.Bloom);

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const SpotlightComponent& comp) {
	Write(comp.Ambient);
	Write(comp.Diffuse);
	Write(comp.Specular);
	Write(comp.Position);
	Write(comp.Direction);
	Write(comp.CutoffAngle);
	Write(comp.OuterCutoffAngle);

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const ParticleEmitterComponent& comp) {
	Write(comp.Position);
	Write(comp.MaxParticleCount);
	Write(comp.ParticleLifetime);
	Write(comp.SpawnInterval);
	Write(comp.Offset);
	Write((u64)comp.MaterialAsset.ID);

	return *this;
}

template<>
BinaryWriter& BinaryWriter::WriteObject(const Entity& entity) {
	Write((u64)entity.GetHandle());
	Write(entity.GetName());

	std::bitset<12> componentBits;
	componentBits |= ((u16)entity.Has<CameraComponent>()			<< 0);
	componentBits |= ((u16)entity.Has<TagComponent>()				<< 1);
	componentBits |= ((u16)entity.Has<TransformComponent>()			<< 2);
	componentBits |= ((u16)entity.Has<AudioComponent>()				<< 3);
	componentBits |= ((u16)entity.Has<MeshComponent>()				<< 4);
	componentBits |= ((u16)entity.Has<SkyboxComponent>()			<< 5);
	componentBits |= ((u16)entity.Has<ScriptComponent>()			<< 6);
	if(entity.Has<ScriptComponent>() && !entity.Get<ScriptComponent>().Instance)
		componentBits.set(6, false);
	componentBits |= ((u16)entity.Has<RigidBodyComponent>()			<< 7);
	componentBits |= ((u16)entity.Has<DirectionalLightComponent>()	<< 8);
	componentBits |= ((u16)entity.Has<PointLightComponent>()		<< 9);
	componentBits |= ((u16)entity.Has<SpotlightComponent>()			<< 10);
	componentBits |= ((u16)entity.Has<ParticleEmitterComponent>()	<< 11);

	Write((u16)componentBits.to_ulong());

	if(componentBits.test(0))
		Write(entity.Get<CameraComponent>());
	if(componentBits.test(1))
		Write(entity.Get<TagComponent>());
	if(componentBits.test(2))
		Write(entity.Get<TransformComponent>());
	if(componentBits.test(3))
		Write(entity.Get<AudioComponent>());
	if(componentBits.test(4))
		Write(entity.Get<MeshComponent>());
	if(componentBits.test(5))
		Write(entity.Get<SkyboxComponent>());
	if(componentBits.test(6))
		Write(entity.Get<ScriptComponent>());
	if(componentBits.test(7))
		Write(entity.Get<RigidBodyComponent>());
	if(componentBits.test(8))
		Write(entity.Get<DirectionalLightComponent>());
	if(componentBits.test(9))
		Write(entity.Get<PointLightComponent>());
	if(componentBits.test(10))
		Write(entity.Get<SpotlightComponent>());
	if(componentBits.test(11))
		Write(entity.Get<ParticleEmitterComponent>());

	return *this;
}

}

namespace VolcanicEditor {

void SceneLoader::RuntimeSave(const Scene& scene,
							  const std::string& projectPath,
							  const std::string& exportPath)
{
	namespace fs = std::filesystem;

	auto scenePath =
		(fs::path(projectPath) / "App" / "Scene" / scene.Name
		).string() + ".scene";
	auto binPath =
		(fs::path(exportPath) / "Scene" / scene.Name).string() + ".bin";

	BinaryWriter writer(binPath);

	writer.Write(scene.Name);

	u64 entityCount = 0;
	u64 idx = writer.GetPosition();
	writer.Advance(sizeof(u64));

	scene.World3D
	.ForEach(
		[&](const Entity& entity)
		{
			writer.Write(entity);
			entityCount++;
		});

	scene.World2D
	.ForEach(
		[&](const Entity& entity)
		{
			writer.Write(entity);
			entityCount++;
		});

	scene.Canvas
	.ForEach(
		[&](const Entity& entity)
		{
			writer.Write(entity);
			entityCount++;
		});

	writer.SetPosition(idx);
	writer.Write(entityCount);
}

}
