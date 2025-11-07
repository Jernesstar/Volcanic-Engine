#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma::Graphics {

enum class AttachmentTarget { Color, Depth, Stencil };

class Attachment : public Derivable<Attachment> {
public:
	const AttachmentTarget Target;

public:
	Attachment(AttachmentTarget target)
		: Target(target) { }
	virtual ~Attachment() = default;
};

class Framebuffer : public Derivable<Framebuffer> {
public:
	Framebuffer(u32 width, u32 height)
		: m_Width(width), m_Height(height) { }
	virtual ~Framebuffer() = default;

	virtual bool Has(AttachmentTarget target) const = 0;
	virtual void Add(AttachmentTarget target, Ref<Attachment> att) = 0;
	virtual void Attach(AttachmentTarget target, u32 idx, u32 dst) = 0;
	virtual Ref<Attachment> Get(AttachmentTarget target, u32 idx = 0) const = 0;

	virtual u32 GetWidth() const { return m_Width; }
	virtual u32 GetHeight() const { return m_Height; }

protected:
	u32 m_Width, m_Height;
};

struct FramebufferSpecification {

};

}