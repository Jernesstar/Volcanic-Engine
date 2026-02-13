#pragma once

#include <VolcaniCore/Core/List.h>

#include "Point.h"

using namespace VolcaniCore;

namespace Lava::Graphics {

struct Curve {
	List<Point> Points;
};

}