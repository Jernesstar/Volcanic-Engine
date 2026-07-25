#pragma once

#include <vector>

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
	// Non-virtual: components are POD-like value types stored by flecs and
	// mirrored to AngelScript via asOFFSET. A virtual dtor would add a vptr,
	// breaking the field offsets flecs/AngelScript rely on. Nothing uses
	// Component polymorphically.
	~Component() = default;
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
	Asset MaterialAsset; // parent material for this slot (== Instance.ParentAsset)

	// Per-slot material overrides. Empty by default (serializes as count 0). The
	// parent is MaterialAsset; overrides survive scene save/load (Sprint 65).
	MaterialInstance Instance;

	MeshComponent() = default;
	MeshComponent(const Asset& geometry, const Asset& material)
		: GeometryAsset(geometry), MaterialAsset(material)
	{
		Instance.ParentAsset = material;
	}
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
	// Static collider authored on an entity. Sprint 60 supports box (default)
	// and mesh (cooked from the entity's MeshComponent geometry) shapes; the
	// body is always static (no dynamics in scope). Triggers are sensors:
	// reported by overlap/trigger queries only, invisible to raycasts.
	// Backed by int so the AngelScript enum property (4 bytes) matches the C++
	// field layout. Serialized narrowed to u8.
	enum class BodyType : int { Static = 0 };
	enum class ShapeType : int { Box = 0, Sphere = 1, Capsule = 2, Mesh = 3 };

	Vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };
	bool IsTrigger = false;
	BodyType Type = BodyType::Static;
	ShapeType Shape = ShapeType::Box;

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

// Binary serialization format version for ParticleEmitterComponent. Bump on any
// change to the field set or order; the Runtime reader logs a clear error and
// bails on mismatch instead of reading misaligned data. (Sprint 64, task 1.2)
static constexpr u32 k_EmitterFormatVersion = 3;

struct ParticleEmitterComponent : public Component {
	// Emitters are transform-relative (Sprint 64). The spawn origin and light
	// origin are the entity's WorldTransform translation plus these offsets, so an
	// emitter attached to an entity tracks it when it moves. Parenting is
	// deliberately deferred — offsets only.
	Vec3 LocalOffset = { 0.0f, 0.0f, 0.0f };  // spawn origin, entity-relative
	Vec3 LightOffset = { 0.0f, 0.0f, 0.0f };  // light origin, above the spawn base

	// Per-axis half-extents of the spawn volume. Replaces the old scalar Offset (a
	// cube). Authored per emitter: a flat disc uses wide X/Z with near-zero Y, a
	// wide sheet a flat rectangle, a taller volume a box.
	Vec3 SpawnExtents = { 0.05f, 0.05f, 0.05f };

	u64 MaxParticleCount;
	f32 ParticleLifetime; // Seconds (matches the seconds-based update TimeStep)
	f32 SpawnInterval;    // Seconds between spawn cycles
	Asset MaterialAsset;

	// Additive tint written into the HDR buffer; values > 1 push past the bloom
	// threshold so emissive particles glow. Multiplies the procedural age ramp in
	// the particle shader rather than replacing it. (Sprint 64)
	Vec4 Color = { 2.0f, 1.1f, 0.35f, 1.0f };
	f32 Size = 0.1f; // Billboard PEAK half-size in world units (size ramps over age)
	// When true, the emitter contributes a co-located point light into the
	// deferred lighting pass so particles light nearby scene geometry.
	bool EmitsLight = false;
	f32 LightRadius = 6.0f; // Attenuation reach of the emitter light

	// Per-spawn positional jitter (fraction of the spawn volume). 0 = no jitter.
	// Replaces the old hardcoded spawn-scatter constant so the amount is authored.
	f32 SpawnJitter = 0.0f;

	// Emitter-light flicker amplitude (0 = off/steady). When > 0 the emitter
	// light's intensity is modulated by a fixed non-harmonic pattern each frame so
	// the pool of light shimmers. LightFlickerSpeed scales that pattern's rate.
	f32 LightFlicker = 0.0f;
	f32 LightFlickerSpeed = 1.0f;

	// Three-stop age colour ramp (young -> mid -> old), multiplied into the
	// particle shading. Neutral white defaults leave the base Color unchanged; a
	// game authors a warm ramp for one effect, a cool ramp for another, etc.
	Vec3 ColorStart = { 1.0f, 1.0f, 1.0f };
	Vec3 ColorMid   = { 1.0f, 1.0f, 1.0f };
	Vec3 ColorEnd   = { 1.0f, 1.0f, 1.0f };

	ParticleEmitterComponent() = default;
	ParticleEmitterComponent(u64 max, f32 lifetime, f32 spawnInterval,
		const Vec3& spawnExtents, const Asset& asset)
		: SpawnExtents(spawnExtents), MaxParticleCount(max),
		ParticleLifetime(lifetime), SpawnInterval(spawnInterval),
		MaterialAsset(asset) { }
	ParticleEmitterComponent(const ParticleEmitterComponent& other) = default;
};

// Grid-based animated surface. A flat row-major u8 cell grid (0..255) drives ONE
// quad shaded entirely in the fragment stage — coverage, colour and edges come
// from the grid, never from per-cell geometry. A gameplay system (script) mutates
// the grid and bumps Version once per fixed tick. The render pipeline owns the
// double-buffered GPU height textures (keyed by entity id) and, whenever Version
// changes, swaps prev<-curr and uploads the new field, then draws the quad
// spanning [Origin, Origin + (GridWidth,GridHeight)*CellSize] on the XZ plane at
// height PlaneY, sampling the lerp(prev, curr, TickAlpha) grid value in the
// fragment shader. All look/shading is supplied by the entity's material and its
// shader — this component is purely the field, placement, and tick state.
//
// Not a POD asOFFSET mirror: the std::vector makes this a ref-typed script
// object with explicit accessor methods (registered in ScriptGlue), so scripts
// never touch the buffer layout directly.
struct SurfaceGridComponent : public Component {
	u32 GridWidth = 0;
	u32 GridHeight = 0;
	std::vector<u8> Heights; // row-major, size GridWidth*GridHeight

	// World placement of the quad. Origin is the world XZ of cell (0,0)'s CENTRE;
	// the quad is inset/extended by half a cell so cell centres map to texel
	// centres under bilinear sampling.
	Vec3 Origin = { 0.0f, 0.0f, 0.0f };
	f32 CellSize = 1.0f;
	f32 PlaneY = 0.05f; // world height of the surface plane

	// Inter-tick blend: the fractional progress of the gameplay tick accumulator,
	// pushed from the same accumulator that drives the tick (no second timer).
	f32 TickAlpha = 0.0f;

	// Bumped once per tick by the driving system. The pipeline compares against its
	// cached value to decide when to swap+upload (handles multi-tick catch-up:
	// each tick bumps Version and the pipeline swaps per observed increment).
	u32 Version = 0;

	// The material naming the shader that renders this surface. All shading
	// parameters live in the material's props (bound generically via reflection),
	// so no look-specific fields belong on this engine component.
	Asset MaterialAsset;

	SurfaceGridComponent() = default;
	SurfaceGridComponent(const SurfaceGridComponent& other) = default;

	void Allocate(u32 w, u32 h) {
		GridWidth = w; GridHeight = h;
		Heights.assign((size_t)w * h, 0);
	}
	u8 At(u32 x, u32 y) const {
		if(x >= GridWidth || y >= GridHeight) return 0;
		return Heights[(size_t)y * GridWidth + x];
	}
	void Set(u32 x, u32 y, u8 v) {
		if(x >= GridWidth || y >= GridHeight) return;
		Heights[(size_t)y * GridWidth + x] = v;
	}
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