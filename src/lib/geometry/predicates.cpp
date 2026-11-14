#include "geometry/predicates.h"
namespace geometry {
int orientation(const Point &a, const Point &b, const Point &p) {
    double determinant = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    return (determinant > 0) - (determinant < 0);
}
} // namespace geometry
