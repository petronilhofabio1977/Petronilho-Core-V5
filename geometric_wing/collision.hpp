#pragma once
#include "geometric_wing/spatial_node.hpp"

namespace super_core::geometric {

inline bool check_collision(const BoundingBox& a, const BoundingBox& b) {
    return (a.min_p.x <= b.max_p.x && a.max_p.x >= b.min_p.x) &&
           (a.min_p.y <= b.max_p.y && a.max_p.y >= b.min_p.y);
}

} // namespace super_core::geometric
