#pragma once

#include "RendererAPI.h"

#include "Platform/OpenGL/Renderer.h"

namespace Magma::Graphics {

void RendererAPI::Create(RendererAPI::Backend backend) {
	switch(backend) {
		case RendererAPI::Backend::OpenGL:
			s_Instance = CreateRef<OpenGL::Renderer>();
			break;
		// case RendererAPI::Backend::Metal:
		// 	s_Instance = CreateRef<Metal::Renderer>();
		// 	break;
	}
}

}