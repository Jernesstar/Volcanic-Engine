#pragma once

#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Template.h>

#undef near
#undef far

using namespace VolcaniCore;

namespace VolcanicEngine::Graphics {

class Camera : public Derivable<Camera> {
public:
	enum class Type { Orthographic, Stereographic, Isometric };

public:
	static Ref<Camera> Create(Camera::Type type);

public:
	Camera(Camera::Type type)
		: m_Type(type) { }
	Camera(Camera::Type type, u32 width, u32 height, f32 near, f32 far)
		: m_Type(type), ViewportWidth(width), ViewportHeight(height),
		Near(near), Far(far)
	{
		VOLCANICORE_ASSERT(width != 0 && height != 0,
							"Viewport width and height must not be 0");
		VOLCANICORE_ASSERT(near != 0 && far != 0,
							"Near and far clip must not be non-zero");
	}
	virtual ~Camera() = default;

	virtual void Resize(u32 width, u32 height) {
		if(width == ViewportWidth && height == ViewportHeight)
			return;

		ViewportWidth = width ? width : ViewportWidth;
		ViewportHeight = height ? height : ViewportHeight;
		CalculateProjection();
	}
	virtual void SetProjection(f32 near, f32 far) {
		if(near == Near && far == Far)
			return;

		Near = near ? near : Near;
		Far = far ? far : Far;
		CalculateProjection();
	}

	void SetPosition(const Vec3& pos) {
		Position = pos;
		CalculateView();
	}

	void SetDirection(const Vec3& dir) {
		Direction = glm::normalize(dir);
		CalculateView();
	}

	void SetPositionDirection(const Vec3& pos, const Vec3& dir) {
		Position = pos;
		Direction = dir;
		CalculateView();
	}

	Camera::Type GetType() const { return m_Type; }

	const Vec3& GetPosition()  const { return Position; }
	const Vec3& GetDirection() const { return Direction; }
	
	u32 GetViewportWidth() const { return ViewportWidth; }
	u32 GetViewportHeight() const { return ViewportHeight; }
	f32 GetNear() const { return Near; }
	f32 GetFar() const { return Far; }

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

class OrthographicCamera : public Camera {
public:
	OrthographicCamera()
		: Camera(Camera::Type::Orthographic)
	{
		CalculateProjection();
		CalculateView();
	}

	OrthographicCamera(f32 rotation)
		: Camera(Camera::Type::Orthographic), m_Rotation(rotation)
	{
		CalculateProjection();
		CalculateView();
	}

	OrthographicCamera(u32 width, u32 height, f32 near, f32 far, f32 rot = 0.0f)
		: Camera(Camera::Type::Orthographic, width, height, near, far),
		m_Rotation(rot)
	{
		CalculateProjection();
		CalculateView();
	}
	~OrthographicCamera() = default;

	void SetRotation(f32 rotation) {
		m_Rotation = rotation;
		CalculateView();
	}

	f32 GetRotation() const { return m_Rotation; }

private:
	f32 m_Rotation;

private:
	void CalculateView() override {
		View = glm::lookAt(Position, Direction, Vec3(0.0f, 1.0f, 0.0f));
		ViewProjection = Projection * View;
	}
	void CalculateProjection() override {
		Projection = glm::ortho(-(f32)ViewportWidth  / 2.0f,
								 (f32)ViewportWidth  / 2.0f,
								-(f32)ViewportHeight / 2.0f,
								 (f32)ViewportHeight / 2.0f, Near, Far);
		ViewProjection = Projection * View;
	}
};

class StereographicCamera : public Camera {
public:
	StereographicCamera()
		: Camera(Camera::Type::Stereographic)
	{
		CalculateProjection();
		CalculateView();
	}

	StereographicCamera(f32 verticalFOV)
		: Camera(Camera::Type::Stereographic), m_VerticalFOV(verticalFOV)
	{
		CalculateProjection();
		CalculateView();
	}
	StereographicCamera(f32 fov, u32 width, u32 height, f32 near, f32 far)
		: Camera(Camera::Type::Stereographic, width, height, near, far),
			m_VerticalFOV(fov)
	{
		CalculateProjection();
		CalculateView();
	}
	~StereographicCamera() = default;

	void SetVerticalFOV(f32 verticalFOV) {
		m_VerticalFOV = verticalFOV;
		CalculateProjection();
	}
	f32 GetVerticalFOV() const { return m_VerticalFOV; }

private:
	f32 m_VerticalFOV = 45.0f;

private:
	void CalculateView() override {
		Vec3 up = { 0.0f, 1.0f, 0.0f };
		View = glm::lookAt(Position, Position + Direction, up);
		ViewProjection = Projection * View;
	}
	void CalculateProjection() override {
		Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV),
										(f32)ViewportWidth,
										(f32)ViewportHeight, Near, Far);
		ViewProjection = Projection * View;
	}
};

class IsometricCamera : public Camera {
public:
	f32 R = 45.0f;

public:
	IsometricCamera(f32 distance = 45.0f)
		: Camera(Camera::Type::Isometric), R(distance)
	{
		Near = -1000.f;
		SetDistance(R);
	}
	~IsometricCamera() = default;

	void SetDistance(f32 r) {
		R = r;
		Position =
			R * Vec3
				{
					glm::sin(glm::radians(45.0f)),
					glm::sin(glm::radians(35.264f)),
					glm::sin(glm::radians(45.0f))
				};
		Direction = -glm::normalize(Position);

		CalculateView();
		CalculateProjection();
	}

private:
	void CalculateProjection() override {
		Projection = glm::ortho(-(f32)ViewportWidth  / (R),
								 (f32)ViewportWidth  / (R),
								-(f32)ViewportHeight / (R),
								 (f32)ViewportHeight / (R), Near, Far);
		ViewProjection = Projection * View;
	}
	void CalculateView() override {
		Vec3 up = { 0.0f, 1.0f, 0.0f };
		View = glm::lookAt(Position, Position + R*Direction, up);
		ViewProjection = Projection * View;
	}
	void Resize(u32 width, u32 height) override {
		ViewportWidth = 480;
		ViewportHeight = 270;
	}
};

inline Ref<Camera> Camera::Create(Camera::Type type) {
	switch(type) {
		case Camera::Type::Orthographic:
			return CreateRef<OrthographicCamera>();
		case Camera::Type::Stereographic:
			return CreateRef<StereographicCamera>();
		case Camera::Type::Isometric:
			return CreateRef<IsometricCamera>();
		default:
			VOLCANICORE_ASSERT(false, "Unknown camera type");
			return nullptr;
	}
}

}