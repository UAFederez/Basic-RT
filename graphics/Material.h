#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H

#include "../math/Vector3.h"
#include "../math/Ray.h"

class Material;

struct HitRecord 
{
    float     t;
    Vector3   point_at_t;
    Vector3   normal;
    Material* material_ptr;
};

Vector3 random_in_unit_sphere()
{
    Vector3 point;
    do {
        point = 2.0 * Vector3(drand48(), drand48(), drand48()) - Vector3(1.0, 1.0, 1.0);
    } while(point.length_squared() >= 1.0f);
    return point;
}

class Material {
public:
    virtual bool scatter(const Ray&, const HitRecord&, Vector3&, Ray&)  const = 0;
    virtual ~Material()
    {
    }
};

class Lambertian : public Material {
public:
    Lambertian(const Vector3& attenuation):
        albedo(attenuation) { }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 target = rec.point_at_t + rec.normal + random_in_unit_sphere();
        scattered   = Ray(rec.point_at_t, target - rec.point_at_t);
        attenuation = albedo;
        return true;
    }

    Vector3 albedo;
};

class Metal : public Material {
public:
    Metal(const Vector3& attenuation, 
          const float fuzz = 0.0f):
        albedo   (attenuation),
        fuzziness(fuzz)
    {
        fuzziness = std::min<float>(std::max<float>(0.0, fuzziness), 1.0f);
    }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 reflected = reflect(normalize(ray_in.direction()), rec.normal);
        scattered   = Ray(rec.point_at_t, reflected + fuzziness * random_in_unit_sphere());
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }
    Vector3 albedo;
    float   fuzziness;
};

#endif
