#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Template.h>

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

class Camera : public Derivable<Camera> {
public:
	enum class Type { Ortho, Stereo };

public:
	static Ref<Camera> Create(Camera::Type type);
	static Ref<Camera> Create(Camera::Type type, f32 fovOrRotation);

public:
	Camera(Camera::Type type);
	Camera(Camera::Type type, u32 width, u32 height, f32 near, f32 far);
	virtual ~Camera() = default;

	virtual void Resize(u32 width, u32 height);
	virtual void SetProjection(f32 near, f32 far);

	void SetPosition(const Vec3& pos);
	void SetDirection(const Vec3& dir);
	void SetPositionDirection(const Vec3& pos, const Vec3& dir);

	Camera::Type GetType() const { return m_Type; }

	const Vec3& GetPosition()  const { return Position; }
	const Vec3& GetDirection() const { return Direction; }
	
	u32 GetViewportWidth() const { return ViewportWidth; }
	u32 GetViewportHeight() const { return ViewportHeight; }
	f32 GetNear() const { return Near; }
	f32 GetFar()	const { return Far; }

	const Mat4& GetView()           const { return View; }
	const Mat4& GetProjection()     const { return Projection; }
	const Mat4& GetViewProjection() const { return ViewProjection; }

protected:
	Vec3 Position = { 0.0f, 0.0f, 0.0f };
	Vec3 Direction = { 0.0f, 0.0f, -1.0f };

	u32 ViewportWidth  = 800;
	u32 ViewportHeight = 600;
	f32 Near = 0.001f;
	f32 Far  = 1000.0f;

	Mat4 Projection{ 1.0f };
	Mat4 View{ 1.0f };
	Mat4 ViewProjection{ 1.0f };

protected:
	virtual void CalculateView() = 0;
	virtual void CalculateProjection() = 0;

private:
	const Camera::Type m_Type;
};

}