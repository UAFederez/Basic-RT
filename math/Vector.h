#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include "../util/General.h"    // random_float()

using scalar = double;

template <std::size_t N>
class Vector {
public:
    Vector()
    {
        std::fill(comp, comp + N, 0);
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

    inline Vector& operator *=(const scalar f)
    {
        for(std::size_t i = 0; i < N; i++)
            comp[i] *= f;
        return *this;
    }

    inline Vector& operator /=(const scalar f)
    {
        const scalar denom = (scalar) 1 / f;

        for(std::size_t i = 0; i < N; i++)
            comp[i] *= denom;
        return *this;
    }

    inline scalar dot(const Vector<N>& v2)
    {
        scalar result = 0.0;
        for(std::size_t i = 0; i < N; i++)
            result += comp[i] * v2[i];
        return result;
    }

    inline Vector<3> cross(const Vector<3>& v2)
    {
        return Vector<3>({
            y() * v2.z() - z() * v2.y(),
            z() * v2.x() - x() * v2.z(),
            x() * v2.y() - y() * v2.x()
        });
    }

    inline scalar x() const { return comp[0]; }
    inline scalar y() const { return comp[1]; }
    inline scalar z() const { return comp[2]; }

    inline scalar r() const { return comp[0]; }
    inline scalar g() const { return comp[1]; }
    inline scalar b() const { return comp[2]; }

    inline scalar u() const { return comp[0]; }
    inline scalar v() const { return comp[1]; }

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

    // TODO: Add range check in the future

    inline scalar& operator[](std::size_t i)       { return comp[i]; }
    inline scalar  operator[](std::size_t i) const { return comp[i]; }
private:
    scalar comp[N];
};

using Vec2 = Vector<2>;
using Vec3 = Vector<3>;

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
    return Vector<N>({ v1.x() + v2.x(), 
                       v1.y() + v2.y(), 
                       v1.z() + v2.z() });
}

template <std::size_t N>
inline Vector<N> operator-(const Vector<N>& v1, const Vector<N>& v2)
{
    return Vector<N>({ v1.x() - v2.x(), 
                       v1.y() - v2.y(), 
                       v1.z() - v2.z() });
}

template <std::size_t N>
inline Vector<N> operator*(const Vector<N>& v1, const Vector<N>& v2)
{
    return Vector<N>({ v1.x() * v2.x(), 
                       v1.y() * v2.y(), 
                       v1.z() * v2.z() });
}

template <std::size_t N>
inline Vector<N> operator/(const Vector<N>& v1, const Vector<N>& v2)
{
    return Vector<N>({ v1.x() / v2.x(), 
                       v1.y() / v2.y(), 
                       v1.z() / v2.z() });
}

template <std::size_t N>
inline Vector<N> operator*(const Vector<N>& v1, const scalar f)
{
    return Vector<N>({ v1.x() * f, v1.y() * f, v1.z() * f });
}

template <std::size_t N>
inline Vector<N> operator*(const scalar f, const Vector<N>& v1)
{
    return v1 * f;
}

template <std::size_t N>
inline Vector<N> operator/(const Vector<N>& v1, const scalar f)
{
    const float denom = 1.0 / f;
    return Vector<N>({ v1.x() * denom, v1.y() * denom, v1.z() * denom });
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
    Vec3 point;
    do {
        point = 2.0 * Vec3({ random_float(), 
                             random_float(), 
                             random_float() }) - Vec3({1.0, 1.0, 1.0});
    } while(point.magnitude_squared() >= 1.0f);
    return point;
}

inline Vec3 refract(const Vec3& incident, const Vec3& normal, const float ior)
{
    Vec3 I   = normalize(incident);
    float ndoti = dot(I, normal);
    float disc  = 1 - (ior * ior) * (1 - (ndoti * ndoti));
    if(disc < 0)
        return Vec3({0.0, 0.0, 0.0});

    return normal * (ior * ndoti - sqrt(disc)) - (I * ior);
}


#endif
