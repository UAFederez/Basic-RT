#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H

#include "../util/General.h"
#include "../math/Vector.h"
#include "../math/Ray.h"

class Material;

struct HitRecord 
{
    float t;
    Vec3  point_at_t;
    Vec3  normal;
    Vec2  uv;
    Material* material_ptr;
};

class Material {
public:
    virtual bool scatter(const Ray&, const HitRecord&, Vec3&, Ray&)  const = 0;
    virtual Vec3 emitted() const
    {
        return Vec3({0.0, 0.0, 0.0});
    }
    virtual ~Material()
    {
    }
};

class Emissive : public Material {
public:
    Emissive(const Vec3& col):
        color(col)
    {
    }

    virtual Vec3 emitted() const
    {
        return color;
    }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered)
        const override
    {
        return false;
    }
    Vec3 color;
};

class Lambertian : public Material {
public:
    Lambertian(const Vec3& attenuation):
        albedo(attenuation) { }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
    {
        Vec3 target = rec.point_at_t + rec.normal + random_in_unit_sphere();
        scattered   = Ray(rec.point_at_t, target - rec.point_at_t);
        attenuation = albedo;
        return true;
    }

    Vec3 albedo;
};

class Metal : public Material {
public:
    Metal(const Vec3& attenuation, 
          const float fuzz = 0.0f):
        albedo   (attenuation),
        fuzziness(fuzz)
    {
        fuzziness = std::min<float>(std::max<float>(0.0, fuzziness), 1.0f);
    }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
    {
        Vec3 reflected = reflect(normalize(ray_in.direction()), rec.normal);
        scattered   = Ray(rec.point_at_t, reflected + fuzziness * random_in_unit_sphere());
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }
    Vec3 albedo;
    float   fuzziness;
};

class Dielectric : public Material {
public:
    Dielectric(const float rel_ior):
        rel_ior(rel_ior)
    {
    }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
    {
        Vec3  nrm = Vec3({0.0, 0.0, 0.0});
        float ior = 0.0f;
        
        if(dot(ray_in.direction(), rec.normal) > 0) 
        {
            nrm = -rec.normal;
            ior = rel_ior;
        } else
        {
            nrm = rec.normal;
            ior = 1.0 / rel_ior;
        }

        Vec3 refracted = refract(-ray_in.direction(), nrm, ior);
        Vec3 reflected = reflect(ray_in.direction(), rec.normal);
        attenuation    = Vec3({1.0, 1.0, 1.0});

        if(refracted.magnitude_squared() == 0) 
        {
            scattered = Ray(rec.point_at_t, reflected);
            return false;
        }
        else
            scattered = Ray(rec.point_at_t, refracted);

        return true;
    }
    float rel_ior;
};

#endif
