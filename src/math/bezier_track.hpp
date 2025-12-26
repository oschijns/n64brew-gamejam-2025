#pragma once

/**
 * @file bezier_track.hpp
 * @brief Store track data as a chain of Bezier curves
 * 
 * BezierTrack allows to define a track as a chain of cubic Bezier curve sections.
 * Each section is defined by 4 control points, where the last point of each section
 * is the first point of the next section. The class allows to retrieve interpolated
 * points along the curve, including extra data such as normals and widths associated
 * with each section. 
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
 * Then we have 4 sections: A, B, C, D
 * Each section has 4 control points:
 * - Segment A: A0, A1, A2, B0
 * - Segment B: B0, B1, B2, C0
 * - Segment C: C0, C1, C2, D0
 * - Segment D: D0, D1, D2, A0
 * 
 * The last control point of section D (A0) is a duplicate of the first control point
 * of section A to allow proper looping.
 * 
 * And for each section, we have an associated section data which define 
 * the normal and the width at the beginning of the section.
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


    /// @brief Extra data for a track section (aka. cubic Bezier curve)
    struct SectionData
    {
    public:
        /// @brief Normal at the control point
        Vec3 normal;

        /// @brief Width at the control point
        real width;

    protected:
        /// @brief AABB enclosing the section 
        AABB aabb = AABB::invalid();

    public:
        /// @brief Default constructor
        inline SectionData(): normal(0.f, 1.f, 0.f), width(1.f)
        {}

        /// @brief Default constructor
        inline SectionData(const Vec3 & normal_, real width_):
            normal(normal_), width(width_)
        {}

        /// @brief Move constructor
        inline SectionData(Vec3 && normal_, real width_):
            normal(normal_), width(width_)
        {}

        /// @brief Default destructor
        inline ~SectionData() = default;
    };


    /// @brief Enumeration of possible errors that can be encountered when loading a track file
    enum class TrackLoadError
    {
        /// @brief No error encountered when loading the file
        OK = 0,

        /// @brief Could not open provided file
        COULD_NOT_OPEN,

        /// @brief File is empty (or almost empty)
        EMPTY_FILE,

        /// @brief Binary file use a different version than the one expected
        WRONG_VERSION,

        /// @brief No track data to read (only header)
        NO_DATA,

        /// @brief We got a section count that do not match with the size of the file
        WRONG_SECTION_COUNT,
    };


    /// @brief Store track data as a chain of cubic Bezier curves
    class BezierTrack
    {
    protected:
        /// @brief List of control points defining the Bezier curve
        /// The control points are stored in a flat list as a chain of cubic 
        /// Bezier sections. The last point of each section is the first point 
        /// of the next section. And the last control point is a duplicate of 
        /// the first to allow proper looping.
        List<Vec3> control_points;

        /// @brief List of section data associated with each control point
        List<SectionData> sections_data;


    public:
        /// @brief Create an empty Bezier curve to populate
        inline BezierTrack() = default;

        /// @brief Create a Bezier curve by specifying the number of sections
        /// @param sections_ Number of sections to allocate
        inline explicit BezierTrack(uint sections_):
            control_points(sections_ * 3 + 1),
            sections_data(sections_ + 1)
        {}

    protected:
        /// @brief Set the list to be a given size
        /// @param sections_ Number of sections to allocate
        inline void set_section_count(uint sections_)
        {
            control_points = List<Vec3>       (sections_ * 3 + 1);
            sections_data  = List<SectionData>(sections_     + 1);
        }


    public:
        /// @brief Copy constructor
        BezierTrack(const BezierTrack & track) = delete;

        /// @brief Copy operator
        BezierTrack & operator=(const BezierTrack & track) = delete;

        /// @brief Move constructor
        inline BezierTrack(BezierTrack && track):
            control_points(std::move(track.control_points)),
            sections_data (std::move(track.sections_data ))
        {}

        /// @brief Move operator
        inline BezierTrack & operator=(BezierTrack && track)
        {
            control_points = std::move(track.control_points);
            sections_data  = std::move(track.sections_data );
            return *this;
        }

        /// @brief Default destructor
        inline ~BezierTrack() = default;

        /// @brief Get the number of sections in the Bezier curve
        inline uint section_count() const { return sections_data.len() - 1; }

        /// @brief Load a bezier track from a binary file
        /// @param[in]  filepath Path to the file to load
        /// @param[out] track    The track to build from the file
        /// @return The type of error encountered when loading the file
        static TrackLoadError load_from_file(const char * filepath, BezierTrack & track);

    protected:
        /// @brief Presample a section of the track so that positions lookup are faster
        /// @param index   Index of the section to sample from
        /// @param samples Sampling storage to populate
        /// @param len     Length of the sampling storage
        void sample_section(uint index, Point * samples, uint len) const;

    public:
        /// @brief Presample a section of the track so that positions lookup are faster
        /// @tparam N The number of samples to compute
        /// @param[in]  index   Index of the section to sample from
        /// @param[out] samples Sampling storage to populate
        template<unsigned N>
        inline void sample_section(uint index, SampledSubTrack<N> & samples) const
        {
            sample_section(index, samples.points.data, N);
        }
    };


    /// @brief Sampled data for a section of the track
    template<unsigned N>
    class SampledSubTrack
    {
    protected:
        /// @brief List of points sampled from the initial track
        Array<Point, N> points;

    public:

        /// @brief Given a position in space, compute an interpolated point 
        ///        obtain the normal and width of the track at that point.
        /// @param[in]  position The position in 3D space
        /// @param[in]  index    The index of the first vertex of the segment to check
        /// @param[out] point    The point object to populate with the closest point on the track
        /// @return New index to use for the next lookup, or which previous or next sampled sub-track to look
        /// @retval   -1 if we should look in previous sub-track (car goes in reverse)
        /// @retval  N-1 if we should look in next sub-track
        /// @retval [0, N-2] Index to use for next lookup.
        /// @details Instead of looking for the closest segment, we assume that as the 
        ///          car follows the track, it will encounter the segments in sequence.
        int closest_point(const Vec3 & position, int index, Point & point) const;

    };
}
