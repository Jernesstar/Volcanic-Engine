#include "Renderer.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Assert.h>
#include <VolcaniCore/Core/Defines.h>

#include "RendererAPI.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

namespace VolcanicEngine::Graphics {

const u64 Renderer::MaxTriangles = 100'000;
const u64 Renderer::MaxIndices   = MaxTriangles * 6;
const u64 Renderer::MaxVertices  = MaxTriangles * 3;
const u64 Renderer::MaxInstances = MaxTriangles * 4;

static FrameData s_Frame;
static Ref<RenderPass> s_RenderPass;
static DrawCommand* s_Command;

static bool s_OptionsValid = false;

void Renderer::Init() {
	s_Frame = { };

	RendererAPI::Create(RendererBackend::OpenGL);
	Renderer2D::Init();
	Renderer3D::Init();
}

void Renderer::Close() {
	Renderer3D::Close();
	Renderer2D::Close();
}

void Renderer::BeginFrame() {
	RendererAPI::Get()->BeginFrame();
	Renderer2D::StartFrame();
	Renderer3D::StartFrame();
}

void Renderer::EndFrame() {
	Renderer3D::EndFrame();
	Renderer2D::EndFrame();
	RendererAPI::Get()->EndFrame();
}

void Renderer::StartPass(Ref<RenderPass> pass, bool pushCommand) {
	s_RenderPass = pass;
	if(pushCommand)
		PushCommand();
}

void Renderer::EndPass() {
	s_Command = nullptr;
	s_RenderPass = nullptr;
}

Ref<RenderPass> Renderer::GetPass() {
	return s_RenderPass;
}

DrawCommand* Renderer::PushCommand() {
	s_Command = NewCommand();
	return s_Command;
}

void Renderer::PopCommand() {
	if(!s_Command)
		return;

	s_RenderPass->SetUniforms(s_Command);
	s_Command = nullptr;
}

DrawCommand* Renderer::GetCommand() {
	return s_Command;
}

DrawCommand* Renderer::NewCommand(bool usePrevious) {
	if(usePrevious && s_Command && !s_Command->DrawCalls)
		return s_Command;

	return RendererAPI::Get()->NewCommand(s_RenderPass->Get());
}

void Renderer::Clear() {
	if(!s_Command) {
		auto command = RendererAPI::Get()->NewCommand(nullptr);
		command->Clear = true;
		command->ClearColor = { 1.0f, 0.0f, 0.0f, 1.0f };
		Renderer::Flush();
	}
	else {
		GetCommand()->Clear = true;
		GetCommand()->ClearColor = { 1.0f, 0.0f, 1.0f, 1.0f };
	}
}

void Renderer::Resize(uint32_t width, uint32_t height) {
	s_Command->ViewportW = width;
	s_Command->ViewportH = height;
}

void Renderer::PushOptions() {
	s_OptionsValid = true;
}

void Renderer::PopOptions(uint32_t count) {
	s_OptionsValid = false;
}

void Renderer::Flush() {
	s_Command = nullptr;
	RendererAPI::Get()->EndFrame();
}

FrameDebugInfo Renderer::GetDebugInfo() {
	return s_Frame.Info;
}

FrameData& Renderer::GetFrame() {
	return s_Frame;
}

}
