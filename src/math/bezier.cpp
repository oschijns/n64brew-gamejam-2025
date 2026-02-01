#include "math/bezier.hpp"
#include "bezier.hpp"
#include "math/interpolate.hpp"
#include <libdragon.h>

using namespace jam;

Point BezierTrack::get_point(uint seg_idx, real t) const
{
    // Figure out where to read control points from
    const uint   idx = seg_idx * 3;
    const Vec3 * ptr = control_points.raw();

    // precompute powers
    const real 
        i  = 1.f - t, 
        t2 = t   * t, 
        t3 = t2  * t,
        i2 = i   * i,
        i3 = i2  * i;

    // Fetch control points
    const Vec3 
        & p0 = ptr[idx    ],
        & p1 = ptr[idx + 1],
        & p2 = ptr[idx + 2],
        & p3 = ptr[idx + 3];

    // Fetch segment data
    const SegmentData
        & s0 = segments_data[seg_idx    ],
        & s1 = segments_data[seg_idx + 1];

    // Compute the interpolated point
    // We reimplement the bezier_cubic here to avoid extra copies
    Vec3 pos;
    for (uint j = 0; j < 3; ++j)
    {
        pos.coords[j] =
            p0.coords[j] * (      i3    ) +
            p1.coords[j] * (3.f * i2 * t) +
            p2.coords[j] * (3.f * t2 * i) +
            p3.coords[j] * (      t3    );
    }
    Vec3 normal = Vec3::lerp(s0.normal, s1.normal, t).normalize();
    real width  = jam ::lerp(s0.width , s1.width , t);

    return Point(pos, normal, width);
}

jam::BezierTrack *
BezierTrack::from_blender_track_data(size_t numSegments,
                                     std::vector<jam::Vec3> points,
                                     std::vector<jam::Vec3> normals) {
  jam::BezierTrack *ret = new jam::BezierTrack(numSegments);
  assertf(ret->control_points.len() - 1 == points.size(),
          "Mismatch size between containers: %d %d", ret->control_points.len(),
          points.size());
  for (size_t i = 0; i < points.size(); i++) {
    ret->control_points[i] = points[i];
  }

  auto step = std::floor(normals.size() / points.size());
  assertf(step > 1, "Invalid step %d, %d", normals.size(), points.size());
  size_t i = 0;
  for (auto j = 0; i < normals.size(); i += step, j++) {
    ret->segments_data[j].normal = normals[i].normalize();
  }
  return ret;
}

const jam::Point jam::BezierTrack::Iterator::operator*() const
{
  int segment_id = p / 100;
  float t = (p - (segment_id * 100)) / 100.f;
  return trackData->get_point(segment_id, t);
}
jam::BezierTrack::Iterator& jam::BezierTrack::Iterator::operator++() 
{
  p += 10;
  return *this;
}  

bool jam::BezierTrack::Iterator::end()
{
  return *this == trackData->end();
}

jam::BezierTrack::Iterator& jam::BezierTrack::Iterator::reset()
{
  p = 0;
  return *this;
}
