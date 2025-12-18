#pragma once

/**
 * @file bezier_track.hpp
 * @brief Store track data as a chain of Bezier curves
 * 
 * BezierTrack allows to define a track as a chain of cubic Bezier curve segments.
 * Each segment is defined by 4 control points, where the last point of each segment
 * is the first point of the next segment. The class allows to retrieve interpolated
 * points along the curve, including extra data such as normals and widths associated
 * with each segment. 
 * 
 * Assuming we have a circuit defined as this:
 * 
 *       D2 -- A0 -- A1
 *      /             \
 *     D1              A2
 *     |               |
 *     D0              B0
 *     |               |
 *     C2              B1
 *      \             /
 *       C1 -- C0 -- B2
 * 
 * Then we have 4 segments: A, B, C, D
 * Each segment has 4 control points:
 * - Segment A: A0, A1, A2, B0
 * - Segment B: B0, B1, B2, C0
 * - Segment C: C0, C1, C2, D0
 * - Segment D: D0, D1, D2, A0
 * 
 * The last control point of segment D (A0) is a duplicate of the first control point
 * of segment A to allow proper looping.
 * 
 * And for each segment, we have an associated segment data which define 
 * the normal and the width at the beginning of the segment.
 * 
 * references:
 * - https://en.wikipedia.org/wiki/B%C3%A9zier_curve
 * - https://youtu.be/aVwxzDHniEw
 */

#include <math.h>
#include "math/base.hpp"
#include "math/interpolate.hpp"
#include "math/vec3.hpp"
#include "math/aabb.hpp"
#include "collection.hpp"

namespace jam
{
    template<unsigned N>
    class SampledSubTrack;

    class BezierTrack;


    /// @brief Point sampled from the track
    struct Point
    {
        friend class BezierTrack;

    public:
        /// @brief Position of the point in 3D space
        Vec3 position;

        /// @brief Normal at the control point
        Vec3 normal;

        /// @brief Width at the control point
        real width;

        /// @brief Default constructor
        inline Point(const Vec3 & pos, const Vec3 & normal_, real width_):
            position(pos), normal(normal_), width(width_)
        {}

    protected:
        /// @brief Default constructor
        inline Point() {}

    public:
        /// @brief Linera interpolation between two points
        /// @param pt0 First point
        /// @param pt1 Second point
        /// @param t   Interpolation weight
        /// @return The interpolation of the two points
        static inline Point lerp(const Point & pt0, const Point & pt1, real t)
        {
            return Point(
                Vec3::lerp(pt0.position, pt1.position, t),
                Vec3::normal_slerp(pt0.normal, pt1.normal, t),
                jam::lerp(pt0.width, pt1.width, t)
            );
        }

    };


    /// @brief Extra data for a track segment (aka. cubic Bezier curve)
    struct SegmentData
    {
    public:
        /// @brief Normal at the control point
        Vec3 normal;

        /// @brief Width at the control point
        real width;

    protected:
        /// @brief AABB enclosing the segment 
        AABB aabb = AABB::invalid();

    public:
        /// @brief Default constructor
        inline SegmentData(): normal(0.f, 1.f, 0.f), width(1.f)
        {}

        /// @brief Default constructor
        inline SegmentData(const Vec3 & normal_, real width_):
            normal(normal_), width(width_)
        {}

        /// @brief Move constructor
        inline SegmentData(Vec3 && normal_, real width_):
            normal(normal_), width(width_)
        {}

        /// @brief Default destructor
        inline ~SegmentData() = default;
    };


    /// @brief Store track data as a chain of cubic Bezier curves
    class BezierTrack
    {
    protected:
        /// @brief List of control points defining the Bezier curve
        /// The control points are stored in a flat list as a chain of cubic 
        /// Bezier segments. The last point of each segment is the first point 
        /// of the next segment. And the last control point is a duplicate of 
        /// the first to allow proper looping.
        List<Vec3> control_points;

        /// @brief List of segment data associated with each control point
        List<SegmentData> segments_data;


    public:
        /// @brief Create a Bezier curve by specifying the number of segments
        /// @param segments_ Number of segments to allocate
        inline BezierTrack(uint segments_):
            control_points(segments_ * 3 + 1),
            segments_data(segments_ + 1)
        {}

        /// @brief Default destructor
        inline ~BezierTrack() = default;

        /// @brief Get the number of segments in the Bezier curve
        inline uint segment_count() const { return segments_data.len() - 1; }

    protected:
        /// @brief Presample a segment of the track so that positions lookup are faster
        /// @param index   Index of the segment to sample from
        /// @param samples Sampling storage to populate
        /// @param len     Length of the sampling storage
        void sample_segment(uint index, Point * samples, uint len) const;

    public:
        /// @brief Presample a segment of the track so that positions lookup are faster
        /// @tparam N The number of samples to compute
        /// @param[in]  index   Index of the segment to sample from
        /// @param[out] samples Sampling storage to populate
        template<unsigned N>
        inline void sample_segment(uint index, SampledSubTrack<N> & samples) const
        {
            sample_segment(index, samples.points.data, N);
        }
    };


    /// @brief Sampled data for a segment of the track
    template<unsigned N>
    class SampledSubTrack
    {
    protected:
        /// @brief List of points sampled from the initial track
        Array<Point, N> points;

    public:

        /// @brief Given a position in space, compute an interpolated point 
        ///        obtain the normal and width of the track at that point.
        /// @param position The position in 3D space
        /// @param index    The index of the first vertex of the segment to check
        /// @return Closest point on the track
        /// @details Instead of looking for the closest segment, we assume that 
        ///          as the car follows the track, it will encounter the segments 
        ///          in sequence.
        Point closest_point(const Vec3 & position, uint index) const;

    };
}
