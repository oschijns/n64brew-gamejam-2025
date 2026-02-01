#pragma once

/**
 * @file bezier.hpp
 * @brief Define a Bezier curve utility class
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

#include <iterator>
#include <math.h>
#include <vector>
#include "math/base.hpp"
#include "math/vec3.hpp"
#include "math/aabb.hpp"
#include "collection.hpp"

namespace jam
{
    class BezierTrack;

    /// @brief Define data specific to a Bezier curve segment point
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


    /// @brief Define a point structure for Bezier curve points
    struct Point
    {
        friend class BezierTrack;

    public:
        /// @brief Position of the point in 3D space
        Vec3 position;

        /// @brief Extra data associated with the point
        SegmentData data;

        /// @brief Default constructor
        inline Point(const Vec3 & pos, const Vec3 & normal, real width):
            position(pos), data(normal, width)
        {}

    protected:
        /// @brief Default constructor
        inline Point() {}
    };


    /// @brief Define a Bezier curve utility class
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

        /// @brief Get an interpolated point on the Bezier curve
        /// @param segment Index of the segment to sample from
        /// @param weight Weight along the segment in [0, 1]
        /// @return Interpolated point at the specified segment and weight
        Point get_point(uint segment, real weight) const;


        static jam::BezierTrack * from_blender_track_data(size_t numSegments,
                                     std::vector<jam::Vec3> points,
                                     std::vector<jam::Vec3> normals);

        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = unsigned short;

            Iterator(BezierTrack* track) : trackData(track), p(10u) { }
            Iterator(BezierTrack* track, unsigned short p_) : trackData(track), p(p_) { }
            const Point operator*() const;
            Iterator& operator++();
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.trackData == b.trackData && a.p == b.p; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); };  

            bool end();
            Iterator& reset();
        private:
            BezierTrack* trackData;
            // hundreds place is the segment id
            // remainder is the current t value
            unsigned short p;
        };

        Iterator begin() { return Iterator(this); }
        Iterator end() { return Iterator(this, segment_count() * 100); }


        struct PointIterator {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = unsigned short;

            PointIterator(BezierTrack* track, size_t p) : trackData(track), currentPoint(p) { }
            const jam::Vec3 operator*() const { return trackData->control_points[currentPoint]; };
            PointIterator& operator++() { currentPoint++; return *this; };
            friend bool operator== (const PointIterator& a, const PointIterator& b) { return a.trackData == b.trackData && a.currentPoint == b.currentPoint; };
            friend bool operator!= (const PointIterator& a, const PointIterator& b) { return !(a == b); };  

            bool end();
            Iterator& reset();
        private:
            BezierTrack* trackData;
            size_t currentPoint;
        };

        PointIterator control_points_begin() { return PointIterator(this, 0); }
        PointIterator control_points_end() { return PointIterator(this, segment_count()); }
    };
}
