#pragma once

#include <VolcaniCore/Core/List.h>

#include "Point.h"

using namespace VolcaniCore;

namespace Magma::Graphics {

struct Curve {
	List<Point> Points;
};

}