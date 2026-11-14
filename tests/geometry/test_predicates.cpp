#include "check.h"
#include "geometry/predicates.h"
using namespace geometry;
int main() {
    check(orientation({0, 0}, {2, 0}, {1, 1}) == 1, "left turn");
    check(orientation({0, 0}, {2, 0}, {1, -1}) == -1, "right turn");
    check(orientation({0, 0}, {2, 0}, {3, 0}) == 0, "collinear outside segment");
    check(orientation({0, 0}, {0, 0}, {1, 1}) == 0, "degenerate segment");
    check(orientation({2, 0}, {0, 0}, {1, 1}) == -1, "reversed direction");
    check(orientation({0, 0}, {2, 0}, {0, 0}) == 0, "coincident point");
}
