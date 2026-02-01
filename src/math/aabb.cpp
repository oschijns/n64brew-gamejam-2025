#include "math/aabb.hpp"

using namespace jam;


bool AABB::contains(const Vec3 & point) const
{
    return
        point.x >= begin.x and point.x <= end.x and
        point.y >= begin.y and point.y <= end.y and
        point.z >= begin.z and point.z <= end.z;
}


bool AABB::overlap(const AABB & a, const AABB & b)
{
    return
        (a.begin.x <= b.end.x or b.begin.x <= a.end.x) and
        (a.begin.y <= b.end.y or b.begin.y <= a.end.y) and
        (a.begin.z <= b.end.z or b.begin.z <= a.end.z);
}


AABB & AABB::expand_with(const Vec3 & point)
{
    for (uint i = 0; i < 3; ++i)
    {
        // Reference to begin and end coordinates
        real 
            & b = begin.coords[i],
            & e = end  .coords[i];

        // Point coordinate
        const real p = point.coords[i];

        // Update bounds if necessary
        if (p < b) b = p;
        if (p > e) e = p;
    }
    return *this;
}


AABB & AABB::expand_with(const AABB & aabb)
{
    for (uint i = 0; i < 3; ++i)
    {
        // Reference to begin and end coordinates
        real 
            & b0 = begin.coords[i],
            & e0 = end  .coords[i];

        // Other AABB coordinates
        const real
            b1 = aabb.begin.coords[i],
            e1 = aabb.end  .coords[i];

        // Update bounds if necessary
        if (b1 < b0) b0 = b1;
        if (e1 > e0) e0 = e1;
    }
    return *this;
}