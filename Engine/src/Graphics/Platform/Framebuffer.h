#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Lava::Graphics {

enum class AttachmentTarget { Color, Depth, Stencil };

struct AttachmentSpec {
	AttachmentTarget Target;
	u32 Width = 0, Height = 0;
};

class Attachment : public Derivable<Attachment> {
public:
	const AttachmentSpec Spec;

public:
	Attachment(AttachmentSpec spec)
		: Spec(spec) { }
	virtual ~Attachment() = default;
};

struct FramebufferSpec {
	List<AttachmentSpec> Attachments;
};

class Framebuffer : public Derivable<Framebuffer> {
public:
	const FramebufferSpec Spec;

public:
	Framebuffer(const FramebufferSpec& spec)
		: Spec(spec) { }
	virtual ~Framebuffer() = default;

	virtual bool Has(AttachmentTarget target) const = 0;
	virtual void Attach(AttachmentTarget target, u32 idx, u32 dst) = 0;
	virtual Ref<Attachment> Get(AttachmentTarget target, u32 idx = 0) const = 0;
};

}