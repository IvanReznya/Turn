#pragma once
#include "geometry/point.h"
namespace geometry {
// 1 слева от направленного AB, -1 справа, 0 на прямой.
int orientation(const Point &a, const Point &b, const Point &p);
} // namespace geometry
