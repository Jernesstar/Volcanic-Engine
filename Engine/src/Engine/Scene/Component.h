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
	Asset GeometryAsset;
	Asset MaterialAsset;

	MeshComponent() = default;
	MeshComponent(const Asset& geometry, const Asset& material)
		: GeometryAsset(geometry), MaterialAsset(material) { }
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

// Lightweight static collider used by the raycast + trigger queries in
// PhysicsSystem. Not a rigid-body dynamics body — Sprint 60 is scoped to
// raycasts and trigger volumes only. The collider is an axis-aligned box
// centred on the entity's TransformComponent, with half-extents multiplied by
// the transform scale.
struct RigidBodyComponent : public Component {
	Vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };
	bool IsTrigger = false; // Triggers are reported by overlap queries only.

	RigidBodyComponent() = default;
	RigidBodyComponent(const Vec3& halfExtents, bool isTrigger = false)
		: HalfExtents(halfExtents), IsTrigger(isTrigger) { }
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

	// Additive tint written into the HDR buffer; values > 1 push past the bloom
	// threshold so emissive particles glow. (Sprint 64)
	Vec4 Color = { 2.0f, 1.1f, 0.35f, 1.0f };
	f32 Size = 0.1f; // Billboard half-size in world units
	// When true, the emitter contributes a co-located point light into the
	// deferred lighting pass so particles light nearby scene geometry.
	bool EmitsLight = false;
	f32 LightRadius = 6.0f; // Attenuation reach of the emitter light

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

struct UIAnchor {
	
};

enum class UIAlignment {
	Start,
	Center,
	End
};

enum class UIAxisDirection {
	Vertical,
	Horizontal
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
		: Label(label) { }
	ButtonComponent(const ButtonComponent&) = default;
};

}