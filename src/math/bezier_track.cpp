#include "math/bezier_track.hpp"
#include "math/interpolate.hpp"

using namespace jam;


void BezierTrack::sample_segment(uint index, Point * samples, uint len) const
{
    // Figure out where to read control points from
    const uint   idx = index * 3;
    const Vec3 * ptr = control_points.raw();

    // iterate over the path at fixed weight intervals
    const real factor = 1.f / (real) (len - 1);
    for (uint i = 0; i < len; ++i)
    {
        // precompute powers
        const real 
            t  = (real) i * factor,
            v  = 1.f - t, 
            t2 = t   * t, 
            t3 = t2  * t,
            v2 = v   * v,
            v3 = v2  * v;

        // Fetch control points
        const Vec3 
            & p0 = ptr[idx    ],
            & p1 = ptr[idx + 1],
            & p2 = ptr[idx + 2],
            & p3 = ptr[idx + 3];

        // Fetch segment data
        const SegmentData
            & s0 = segments_data[index    ],
            & s1 = segments_data[index + 1];

        // Compute the interpolated point
        // We reimplement the bezier_cubic here to avoid extra copies
        Vec3 pos;
        for (uint j = 0; j < 3; ++j)
        {
            pos.coords[j] =
                p0.coords[j] * (      v3    ) +
                p1.coords[j] * (3.f * v2 * t) +
                p2.coords[j] * (3.f * t2 * v) +
                p3.coords[j] * (      t3    );
        }
        Vec3 normal = Vec3::lerp(s0.normal, s1.normal, t);
        real width  = jam ::lerp(s0.width , s1.width , t);

        samples[i] = Point(pos, normal, width);
    }
}


template<unsigned N>
Point SampledSubTrack<N>::closest_point(const Vec3 & position, uint index) const
{
    // using the index, pick two consecutive points to form as segment
    const Point 
        & pt0 = points[index    ], 
        & pt1 = points[index + 1];

    // Direction from the first point of the segment to the next one
    const Vec3 dir = pt1.position - pt0.position;

    // Get a projection scalar of the position onto the segment pt0 -> pt1
    real t = (position - pt0.position).dot(dir) / dir.mag_sqr();

    // if that projection is 
    // - lower than zero, we have moved to the previous segment
    // - higher than one, we have moved to the next segment
    
    // TODO

    // Then get the closest point
    return Point::lerp(pt0, pt1, t);
}