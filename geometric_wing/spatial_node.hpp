#pragma once

namespace super_core::geometric {

struct Point2D {
    float x, y;

    bool operator<(const Point2D& other) const {
        float d1 = (x * x) + (y * y);
        float d2 = (other.x * other.x) + (other.y * other.y);
        return d1 < d2;
    }
};

struct BoundingBox {
    Point2D min_p;
    Point2D max_p;
};

} // namespace super_core::geometric
