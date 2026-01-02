#pragma once


/**
 * @file track/model.hpp
 * @brief Store track data as a chain of Bezier curves
 * 
 * track::Model allows to define a track as a chain of cubic Bezier curve sections.
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
 * - Section A: A0, A1, A2, B0
 * - Section B: B0, B1, B2, C0
 * - Section C: C0, C1, C2, D0
 * - Section D: D0, D1, D2, A0
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


#include "math/base.hpp"
#include "math/vec3.hpp"
#include "math/aabb.hpp"
#include "collection.hpp"
#include "track/point.hpp"


namespace jam::track
{
    class Model;

    /// @brief Extra data for a track section (aka. cubic Bezier curve)
    struct SectionData
    {
        friend class Model;

    protected:
        /// @brief Normal at the control point
        Vec3 normal;

        /// @brief Width at the control point
        real width = 1.f;

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
    enum class LoadError: uint
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
    class Model
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
        inline Model() = default;

        /// @brief Create a Bezier curve by specifying the number of sections
        /// @param sections_ Number of sections to allocate
        inline explicit Model(uint sections_):
            control_points(sections_ * 3 + 1),
             sections_data(sections_     + 1)
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
        Model(const Model & track) = delete;

        /// @brief Copy operator
        void operator=(const Model & track) = delete;

        /// @brief Move constructor
        inline Model(Model && track):
            control_points(std::move(track.control_points)),
            sections_data (std::move(track.sections_data ))
        {}

        /// @brief Move operator
        inline Model & operator=(Model && track)
        {
            control_points = std::move(track.control_points);
            sections_data  = std::move(track.sections_data );
            return *this;
        }

        /// @brief Default destructor
        inline ~Model() = default;

        /// @brief Load a bezier track from a binary file
        /// @param[in]  filepath Path to the file to load
        /// @param[out] track    The track to build from the file
        /// @return The type of error encountered when loading the file
        static LoadError load_from_file(const char * filepath, Model & track);

    public:
        /// @brief Get the number of sections in the Bezier curve
        inline uint section_count() const { return sections_data.len() - 1; }

        /// @brief Presample a section of the track so that positions lookup are faster
        /// @param index   Index of the section to sample from
        /// @param samples Sampling storage to populate
        void sample_section(uint index, List<Point> & samples) const;
    };
}
