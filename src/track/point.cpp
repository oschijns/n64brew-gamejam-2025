#include "math/interpolate.hpp"
#include "track/point.hpp"


using namespace jam;
using namespace jam::track;


Point lerp(const Point & pt0, const Point & pt1, real t)
{
    return Point(
        Vec3::lerp        (pt0.position, pt1.position, t),
        Vec3::normal_slerp(pt0.normal  , pt1.normal  , t),
        jam ::lerp        (pt0.width   , pt1.width   , t)
    );
}