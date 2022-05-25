#pragma once
#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include <cassert>
#include <array>
#include <iostream>
#include "../util/General.h"    // random_scalar()

// For MSVC support
const scalar k_PI = 3.14159265358979323846264338327950288;

template <std::size_t N>
class Vector {
    template <std::size_t D>
    friend std::ostream& operator<<(std::ostream& os, const Vector<D>& v);

public:
    Vector()
    {
        std::fill(comp, comp + N, static_cast<scalar>(0));
    }
    Vector(const std::array<scalar, N>& val)
    {
        std::copy(val.begin(), val.end(), comp);
    }

    std::size_t size() const { return N; }

    inline Vector& operator+=(const Vector<N>& v2)
    {
        for(std::size_t i = 0; i < N; i++)
            comp[i] += v2[i];
        return *this;
    }

    inline Vector& operator-=(const Vector<N>& v2)
    {
        for(std::size_t i = 0; i < N; i++)
            comp[i] -= v2[i];
        return *this;
    }

    inline Vector operator-() const
    {
        return (*this * -1);
    }

    inline Vector& operator*=(const scalar f)
    {
        for(std::size_t i = 0; i < N; i++)
            comp[i] *= f;
        return *this;
    }

    inline Vector& operator/=(const scalar f)
    {
        const scalar denom = (scalar) 1 / f;

        for(std::size_t i = 0; i < N; i++)
            comp[i] *= denom;
        return *this;
    }

    inline scalar dot(const Vector<N>& v2) const
    {
        scalar result = 0.0;
        for(std::size_t i = 0; i < N; i++)
            result += comp[i] * v2[i];
        return result;
    }

    inline Vector<3> cross(const Vector<3>& v2) const
    {
        return Vector<3>({
            y() * v2.z() - z() * v2.y(),
            z() * v2.x() - x() * v2.z(),
            x() * v2.y() - y() * v2.x()
        });
    }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 1, scalar>::type x() const { return comp[0]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 2, scalar>::type y() const { return comp[1]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 3, scalar>::type z() const { return comp[2]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 1, scalar>::type r() const { return comp[0]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 2, scalar>::type g() const { return comp[1]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 3, scalar>::type b() const { return comp[2]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 1, scalar>::type u() const { return comp[0]; }

    template <std::size_t D = N>
    inline typename std::enable_if< D >= 2, scalar>::type v() const { return comp[1]; }

    inline scalar magnitude_squared() const
    {
        scalar result = 0.0f;
        for(std::size_t i = 0; i < N; i++)
            result += comp[i] * comp[i];
        return result;
    }

    inline scalar magnitude() const
    {
        return sqrt(magnitude_squared());
    }

    inline Vector& normalize()
    {
        *this /= magnitude();
        return *this;
    }

    inline scalar* begin() { return &comp[0]; }
    inline scalar* end()   { return &comp[N]; }

    inline const scalar* begin() const { return &comp[0]; }
    inline const scalar* end()   const { return &comp[N]; }

    // TODO: Add range check version in the future

    inline scalar& operator[](std::size_t i)       { return comp[i]; }
    inline scalar  operator[](std::size_t i) const { return comp[i]; }
private:
    scalar comp[N];
};

using Vec2  = Vector<2>;
using Vec3  = Vector<3>;
using Color = Vector<3>;

inline Vector<3> cross(const Vector<3>& v1, const Vector<3>& v2)
{
    return Vector<3>({
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
    });
}

template <std::size_t N>
inline scalar dot(const Vector<N>& v1, const Vector<N>& v2)
{
    scalar result = 0.0;
    for(std::size_t i = 0; i < N; i++)
        result += v1[i] * v2[i];
    return result;
}


template <std::size_t N>
inline Vector<N> operator+(const Vector<N>& v1, const Vector<N>& v2)
{
    Vector<N> result = v1;
    for(std::size_t i = 0; i < N; i++)
        result[i] += v2[i];
    return result;
}

template <std::size_t N>
inline Vector<N> operator-(const Vector<N>& v1, const Vector<N>& v2)
{
    return v1 + (-v2);
}

template <std::size_t N>
inline Vector<N> operator*(const Vector<N>& v1, const Vector<N>& v2)
{
    Vector<N> result = v1;
    for(std::size_t i = 0; i < N; i++)
        result[i] *= v2[i];
    return result;
}

template <std::size_t N>
inline Vector<N> operator/(const Vector<N>& v1, const Vector<N>& v2)
{
    return v1 * (1.0 / v2);
}

template <std::size_t N>
inline Vector<N> operator*(const Vector<N>& v1, const scalar f)
{
    Vector<N> result = v1;
    for(std::size_t i = 0; i < N; i++)
        result[i] *= f;
    return result;
}

template <std::size_t N>
inline Vector<N> operator*(const scalar f, const Vector<N>& v1)
{
    return v1 * f;
}

template <std::size_t N>
inline Vector<N> operator/(const Vector<N>& v1, const scalar f)
{
    const scalar denom = 1.0 / f;
    return v1 * denom;
}

template <std::size_t N>
inline Vector<N> operator/(const scalar f, const Vector<N>& v1)
{
    return v1 / f;
}

template <std::size_t N>
inline Vector<N> normalize(const Vector<N>& v)
{
    return v / v.magnitude();
}

template <std::size_t N>
inline Vector<N> reflect(const Vector<N>& vec, const Vector<N>& normal)
{
    return vec - 2 * dot(vec, normal) * normal;
}

inline Vec3 random_in_unit_sphere()
{
    const scalar rand_theta = random_scalar(0.0, 2 * k_PI);
    const scalar rand_phi   = random_scalar(0.0, 2 * k_PI);
    
    return Vec3({
        cosf(rand_theta) * sinf(rand_phi),
        sinf(rand_theta) * sinf(rand_phi),
        cosf(rand_phi),
    });
}

inline Vec3 refract(const Vec3& incident, const Vec3& normal, const scalar ior)
{
    Vec3 I      = normalize(incident);
    scalar ndoti = dot(I, normal);
    scalar disc  = 1 - (ior * ior) * (1 - (ndoti * ndoti));
    if(disc < 0)
        return Vec3({0.0, 0.0, 0.0});

    return normal * (ior * ndoti - sqrt(disc)) - (I * ior);
}

template <std::size_t N>
std::ostream& operator<<(std::ostream& os, const Vector<N>& v)
{
    os << "[";
    for(std::size_t i = 0; i < N; i++)
        std::cout << v[i] << (i == N - 1 ? "]" : ", ");
    return os;
}

#endif
