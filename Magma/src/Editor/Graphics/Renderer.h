#pragma once

namespace Magma::Graphics {

class Renderer {
public:
	static void Init();
	static void Close();
	static void StartFrame();
	static void EndFrame();

	// void DrawMesh(const Mesh& mesh);
	// void DrawQuad();
};

}