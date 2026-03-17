#pragma once

#include <VolcaniCore/Core/Math.h>

#include "Graphics/Camera.h"
#include "Asset/AssetManager.h"
#include "Physics/RigidBody.h"
#include "Script/ScriptObject.h"

using namespace VolcaniCore;
using namespace VolcanicEngine::Graphics;
// using namespace VolcanicEngine::Physics;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

struct Component {
	u8 _;

	Component() = default;
	Component(const Component& other) = default;
	virtual ~Component() = default;
};

// --- 3D Components

struct CameraComponent : public Component {
	Ref<Camera> Cam;

	CameraComponent() = default;
	CameraComponent(Ref<Camera> camera)
		: Cam(camera) { }
	CameraComponent(const CameraComponent& other) = default;
};

struct TagComponent : public Component {
	std::string Tag;

	TagComponent()
		: Tag("Empty Tag") { }
	TagComponent(const std::string& tag)
		: Tag(tag) { }
	TagComponent(const TagComponent& other) = default;
};

struct TransformComponent : public Component {
	Vec3 Translation = { 0.0f, 0.0f, 0.0f };
	Vec3 Rotation	 = { 0.0f, 0.0f, 0.0f };
	Vec3 Scale		 = { 1.0f, 1.0f, 1.0f };

	TransformComponent() = default;
	TransformComponent(const Vec3& t, const Vec3& r, const Vec3& s)
		: Translation(t), Rotation(r), Scale(s) { }
	TransformComponent(const Transform& t)
		: Translation(t.Translation), Rotation(t.Rotation), Scale(t.Scale) { }
	TransformComponent(const TransformComponent& other) = default;

	operator Transform() const { return { Translation, Rotation, Scale }; }
};

struct AudioComponent : public Component {
	Asset AudioAsset;

	AudioComponent() = default;
	AudioComponent(const Asset& asset)
		: AudioAsset(asset) { }
	AudioComponent(const AudioComponent& other) = default;
};

struct MeshComponent : public Component {
	Asset MeshSourceAsset;
	Asset MaterialAsset;

	MeshComponent() = default;
	MeshComponent(const Asset& source, const Asset& mat)
		: MeshSourceAsset(source), MaterialAsset(mat) { }
	MeshComponent(const MeshComponent& other) = default;
};

struct SkyboxComponent : public Component {
	Asset CubemapAsset;

	SkyboxComponent() = default;
	SkyboxComponent(const Asset& asset)
		: CubemapAsset(asset) { }
	SkyboxComponent(const SkyboxComponent& other) = default;
};

struct ScriptComponent : public Component {
	Asset ModuleAsset;
	Ref<ScriptObject> Instance;

	ScriptComponent() = default;
	ScriptComponent(const Asset& asset, Ref<ScriptObject> obj)
		: ModuleAsset(asset), Instance(obj) { }
	ScriptComponent(const ScriptComponent& other) = default;
	ScriptComponent(ScriptComponent&& other) = default;
};

struct RigidBodyComponent : public Component {
	// Ref<RigidBody> Body;

	RigidBodyComponent() = default;
	// RigidBodyComponent(Ref<RigidBody> body)
	// 	: Body(body) { }
	RigidBodyComponent(const RigidBodyComponent& other) = default;
};

struct DirectionalLightComponent : public Component {
	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;
	Vec3 Position;
	Vec3 Direction;

	DirectionalLightComponent() = default;
	DirectionalLightComponent(const Vec3& a, const Vec3& d, const Vec3& s,
							  const Vec3& pos, const Vec3& dir)
		: Ambient(a), Diffuse(d), Specular(s), Position(pos), Direction(dir) { }
	DirectionalLightComponent(const DirectionalLightComponent& other) = default;
};

struct PointLightComponent : public Component {
	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;
	Vec3 Position;
	f32 Constant;
	f32 Linear;
	f32 Quadratic;
	bool Bloom;

	PointLightComponent() = default;
	PointLightComponent(const Vec3& a, const Vec3& d, const Vec3& s,
						const Vec3& pos, f32 c, f32 l, f32 q, bool b)
		: Ambient(a), Diffuse(d), Specular(s), Position(pos),
		Constant(c), Linear(l), Quadratic(q), Bloom(b) { }
	PointLightComponent(const PointLightComponent& other) = default;
};

struct SpotlightComponent : public Component {
	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;
	Vec3 Position;
	Vec3 Direction;
	f32 CutoffAngle;
	f32 OuterCutoffAngle;

	SpotlightComponent() = default;
	SpotlightComponent(const Vec3& a, const Vec3& d, const Vec3& s,
						const Vec3& pos, const Vec3& dir,
						f32 inner, f32 outer)
		: Ambient(a), Diffuse(d), Specular(s), Position(pos), Direction(dir),
		CutoffAngle(inner), OuterCutoffAngle(outer) { }
	SpotlightComponent(const SpotlightComponent& other) = default;
};

struct ParticleEmitterComponent : public Component {
	Vec3 Position;
	u64 MaxParticleCount;
	f32 ParticleLifetime; // In milliseconds
	f32 SpawnInterval; // In milliseconds
	f32 Offset;
	Asset MaterialAsset;

	ParticleEmitterComponent() = default;
	ParticleEmitterComponent(const Vec3& pos, u64 max, f32 lifetime,
		f32 spawnInterval, f32 offset, const Asset& asset)
		: Position(pos), MaxParticleCount(max), ParticleLifetime(lifetime),
		SpawnInterval(spawnInterval), Offset(offset), MaterialAsset(asset) { }
	ParticleEmitterComponent(const ParticleEmitterComponent& other) = default;
};

// --- 2D Components

struct PointLight2DComponent : public Component {

};

// --- Canvas components ---

struct CanvasScriptComponent : public Component {
	UUID ScriptID;
	Ref<ScriptObject> Instance;

	CanvasScriptComponent() = default;
	CanvasScriptComponent(UUID id, Ref<ScriptObject> obj)
		: ScriptID(id), Instance(obj) { }
	CanvasScriptComponent(const CanvasScriptComponent& other) = default;
	CanvasScriptComponent(CanvasScriptComponent&& other) = default;
};

struct RectComponent : public Component {
	Vec2 Position = { 0.0f, 0.0f };
	Vec2 Size     = { 100.0f, 100.0f };
	UIAnchor Anchor;
	Vec4 Color    = { 1.0f, 1.0f, 1.0f, 1.0f };

	RectComponent() = default;
	RectComponent(Vec2 pos, Vec2 size, Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f })
		: Position(pos), Size(size), Color(color) { }
	RectComponent(const RectComponent&) = default;
};

struct LayoutComponent : public Component {
	UIAxisDirection Direction = UIAxisDirection::Vertical;
	UIAlignment Alignment = UIAlignment::Start;
	Vec2 Padding = { 0.0f, 0.0f };
	f32 Gap = 0.0f;

	LayoutComponent() = default;
	LayoutComponent(UIAxisDirection dir, UIAlignment align,
					  Vec2 padding = { }, f32 gap = 0.0f)
		: Direction(dir), Alignment(align), Padding(padding), Gap(gap) { }
	LayoutComponent(const LayoutComponent&) = default;
};

struct ImageComponent : public Component {
	UUID ImageID;
	Ref<Texture> Image;
	bool PreserveAspect = false;

	ImageComponent() = default;
	ImageComponent(Ref<Texture> image, bool preserveAspect = false)
		: Image(image), PreserveAspect(preserveAspect) { }
	ImageComponent(const ImageComponent&) = default;
};

struct TextComponent : public Component {
	std::string  Content;
	f32 FontSize  = 16.0f;
	Vec4 FontColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	UIAlignment HAlign = UIAlignment::Start;

	TextComponent() = default;
	TextComponent(const std::string& text, f32 size = 16.0f,
					Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f },
					UIAlignment hAlign = UIAlignment::Start)
		: Content(text), FontSize(size), FontColor(color), HAlign(hAlign) { }
	TextComponent(const TextComponent&) = default;
};

struct ButtonComponent : public Component {
	Str Label;
	Vec4 NormalColor   = { 0.2f, 0.2f, 0.2f, 1.0f };
	Vec4 HoveredColor  = { 0.35f, 0.35f, 0.35f, 1.0f };
	Vec4 PressedColor  = { 0.1f, 0.1f, 0.1f, 1.0f };

	ButtonComponent() = default;
	ButtonComponent(const std::string& label,
					  Func<void> onClick = nullptr)
		: Label(label), OnClick(onClick) { }
	ButtonComponent(const ButtonComponent&) = default;
};

}