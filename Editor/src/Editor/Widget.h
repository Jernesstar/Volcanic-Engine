#pragma once

namespace Magma {

class Widget {
public:
	Widget();
	~Widget();

	static void Begin();
	static void End();
	static void Child();
	static void EndChild();

	static void Text();
	static void Image();

	static void RegisterInterface();

private:
	
};

}