#ifndef RAY_VEC3_H
#define RAY_VEC3_H

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>

class Vector3 {
    friend std::ostream& operator<<(std::ostream& os, const Vector3& v);
public:
    Vector3() {}
    Vector3(float x, float y, float z) { comp[0] = x; comp[1] = y; comp[2] = z; }
    
    inline float x() const { return comp[0]; }
    inline float y() const { return comp[1]; }
    inline float z() const { return comp[2]; }

    inline const Vector3& operator+()  const { return *this; }
    inline const Vector3  operator-()  const { return Vector3(-comp[0], -comp[1], -comp[2]); }
    inline float  operator[](int i) const { return comp[i]; }
    inline float& operator[](int i)       { return comp[i]; }

    inline Vector3& operator+=(const Vector3& v2)
    {
        comp[0] += v2.x();
        comp[1] += v2.y();
        comp[2] += v2.z();
        return *this;
    }

    inline Vector3& operator-=(const Vector3& v2)
    {
        comp[0] -= v2.x();
        comp[1] -= v2.y();
        comp[2] -= v2.z();
        return *this;
    }

    inline Vector3& operator*=(const float t)
    {
        comp[0] *= t;
        comp[1] *= t;
        comp[2] *= t;
        return *this;
    }

    inline Vector3& operator/=(const float t)
    {
        float fac = 1 / t;
        return (*this *= fac);
    }
    inline float length() const 
    { 
        return std::sqrt(length_squared()); 
    }
    inline float length_squared() const
    {
        return (comp[0] * comp[0]) + (comp[1] * comp[1]) + (comp[2] * comp[2]);
    }
    inline void normalize()
    {
        *this /= length();
    }
private:
    float comp[3] = {};
};

inline std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << "[" << v.x() << ", " << v.y() << ", " << v.z() << "]";
    return os;
}

inline Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
    return Vector3(v1.x() + v2.x(), 
                   v1.y() + v2.y(), 
                   v1.z() + v2.z());
}
inline Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
    return Vector3(v1.x() - v2.x(), 
                   v1.y() - v2.y(), 
                   v1.z() - v2.z());
}

inline Vector3 operator*(const Vector3& v1, const float t)
{
    return Vector3(v1.x() * t, v1.y() * t, v1.z() * t);
}

inline Vector3 operator*(const float t, const Vector3& v1)
{
    return v1 * t;
}

inline Vector3 operator/(const Vector3& v1, const float t)
{
    const float fac = 1 / t;
    return Vector3(v1.x() * fac, v1.y() * fac, v1.z() * fac);
}

inline Vector3 operator/(const float t, const Vector3& v1)
{
    return v1 / t;
}

inline float dot(const Vector3& v1, const Vector3& v2)
{
    return (v1.x() * v2.x()) + (v1.y() * v2.y()) + (v1.z() * v2.z());
}

inline Vector3 cross(const Vector3& v1, const Vector3& v2)
{
    return Vector3(v1.y() * v2.z() - v1.z() * v2.y(),
                   v1.z() * v2.x() - v1.x() * v2.z(),
                   v1.x() * v2.y() - v1.y() * v2.x());
}

// Component-wise multiplication
inline Vector3 comp_mul(const Vector3& v1, const Vector3& v2)
{
    return Vector3(v1.x() * v2.x(), v1.y() * v2.y(), v1.z() * v2.z());
}

inline Vector3 reflect(const Vector3& vec, const Vector3& normal)
{
    return vec - 2 * dot(vec, normal) * normal;
}

inline Vector3 normalize(const Vector3& v1)
{
    return v1 / v1.length();
}

#endif
