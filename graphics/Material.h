#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H

#include "../util/General.h"
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
        point = 2.0 * Vector3(random_float(), 
                              random_float(), 
                              random_float()) - Vector3(1.0, 1.0, 1.0);
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

Vector3 refract(const Vector3& incident,
                const Vector3& normal,
                const float ior)
{
    Vector3 I   = normalize(incident);
    float ndoti = dot(I, normal);
    float disc  = 1 - (ior * ior) * (1 - (ndoti * ndoti));
    if(disc < 0)
        return Vector3(0.0, 0.0, 0.0);

    return normal * (ior * ndoti - sqrt(disc)) - (I * ior);
}

class Dielectric : public Material {
public:
    Dielectric(const float rel_ior):
        rel_ior(rel_ior)
    {
    }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 nrm = Vector3(0.0, 0.0, 0.0);
        float   ior = 0.0f;
        
        if(dot(ray_in.direction(), rec.normal) > 0) 
        {
            nrm = -rec.normal;
            ior = rel_ior;
        } else
        {
            nrm = rec.normal;
            ior = 1.0 / rel_ior;
        }

        Vector3 refracted = refract(-ray_in.direction(), nrm, ior);
        Vector3 reflected = reflect(ray_in.direction(), rec.normal);
        attenuation = Vector3(1.0, 1.0, 1.0);

        if(refracted.length_squared() == 0) 
        {
            scattered = Ray(rec.point_at_t, reflected);
            return false;
        }
        else
            scattered = Ray(rec.point_at_t, refracted);

        return true;
    }
    float   rel_ior;
};

#endif
