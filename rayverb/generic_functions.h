#pragma once

#include <vector>
#include <numeric>

/// You can call max_amp on an arbitrarily nested vector and get back the
/// magnitude of the value with the greatest magnitude in the vector.
template <typename T>
inline float max_amp (const T & t)
{
    return std::accumulate
    (   t.begin()
    ,   t.end()
    ,   0.0
    ,   [] (float a, const auto & b) {return std::max (a, max_amp (b));}
    );
}

/// The base case of the max_amp recursion.
template<>
inline float max_amp (const float & t)
{
    return std::fabs (t);
}

/// Recursively divide by reference.
template <typename T>
inline void div (T & ret, float f)
{
    for (auto && i : ret)
        div (i, f);
}

/// The base case of the div recursion.
template<>
inline void div (float & ret, float f)
{
    ret /= f;
}

/// Recursively multiply by reference.
template <typename T>
inline void mul (T & ret, float f)
{
    for (auto && i : ret)
        mul (i, f);
}

/// The base case of the mul recursion.
template<>
inline void mul (float & ret, float f)
{
    ret *= f;
}

/// Find the largest absolute value in an arbitarily nested vector, then
/// divide every item in the vector by that value.
template <typename T>
inline void normalize (std::vector <T> & ret)
{
    mul (ret, 1.0 / max_amp (ret));
}

/// Call binary operation u on pairs of elements from a and b, where a and b are
/// cl_floatx types.
template <typename T, typename U>
inline T elementwise (const T & a, const T & b, const U & u)
{
    T ret;
    std::transform (std::begin (a.s), std::end (a.s), std::begin (b.s), std::begin (ret.s), u);
    return ret;
}

