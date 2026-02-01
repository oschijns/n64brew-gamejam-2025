#pragma once

#include <math.h>

namespace jam
{
    /// @brief Fixed size array structure
    /// @tparam T The type stored in the array
    /// @tparam N 
    template<typename T, unsigned N>
    struct Array
    {
    public:
        T data[N];

        /// @brief Default constructor for the data
        inline Array() = default;

        /// @brief Access operator
        inline T & operator[](unsigned index) { return data[index]; }

        /// @brief Const access operator
        inline T operator[](unsigned index) const { return data[index]; }
    };

    /// @brief List with runtime length
    /// @tparam T The type stored in the list
    template<typename T>
    struct List
    {
    protected:
        /// @brief Current size of the list
        unsigned size;

        /// @brief Pointer to the list data
        T * data;

    public:

        /// @brief Create a list with specified size
        inline List(unsigned size_): size(size_), data(new T [size_]) {}

        /// @brief Default destructor
        inline ~List() { delete[] data; }

        /// @brief Access operator
        inline T & operator[](unsigned index) { return data[index]; }

        /// @brief Const access operator
        inline T operator[](unsigned index) const { return data[index]; }

        /// @brief Get the size of the list
        inline unsigned len() const { return size; }

        /// @brief Get a raw pointer to the list data
        inline const T * raw() const { return data; }

        /// @brief Get a raw mutable pointer to the list data
        inline T * raw_mut() { return data; }
    };

    /// @brief Vector that can grow over time
    /// @tparam T The type stored in the vector
    template<typename T>
    struct Vec
    {
    protected:
        /// @brief Number of elements in the list
        unsigned size = 0;

        /// @brief Backing list with a predefined capacity
        List<T> list;

    public:
        /// @brief Create a list with specified size
        inline Vec(unsigned capacity): list(capacity) {}

        /// @brief Default destructor
        inline ~Vec() = default;

        /// @brief Append a new element to the vector
        inline void push(T && value) { list[size++] = value; }

        /// @brief Remove and return the last element from the vector
        inline T pop() { return list[--size]; }

        /// @brief Access operator
        inline T & operator[](unsigned index) { return list[index]; }

        /// @brief Const access operator
        inline T operator[](unsigned index) const { return list[index]; }

        /// @brief Get the size of the vector
        inline unsigned len() const { return size; }

        /// @brief Get the capacity of the vector
        inline unsigned capacity() const { return list.len(); }

        /// @brief Get a raw pointer to the list data
        inline const T * raw() const { return list.raw(); }

        /// @brief Get a raw mutable pointer to the list data
        inline T * raw_mut() { return list.raw_mut(); }
    };
}
