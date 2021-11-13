#ifndef GRAPHICS_MATERIAL_H
#define GRAPHICS_MATERIAL_H

#include "../util/General.h"
#include "../math/Vector.h"
#include "../math/Ray.h"

#include <memory>

// TODO: Make Textured material have this as an additional option
enum class FilteringMethod {
    NEAREST_NEIGHBOR,
    BILINEAR,
};

inline double schlick_approx(const double IOR, const double cosine)
{
    double R0 = std::pow<double>((1 - IOR) / (1 + IOR), 2);
    return R0 + ((1 - R0) * std::pow<double>((1 - cosine), 5));
}

class Material;

struct HitRecord 
{
    float t;
    Vec3  point_at_t;
    Vec3  normal;
    Vec2  uv;
    Material* material_ptr;

    Vec3 tangent;
    Vec3 bitangent;
};

class Material {
public:
    virtual bool scatter(const Ray&, const HitRecord&, Vec3&, Ray&)  const = 0;
    virtual Vec3 emitted(const Vec2& uv) const;
    virtual ~Material()
    {
    }
    bool is_double_sided = false;
};

// TODO: textured
class Textured : public Material {
public:
    Textured(Color*   albedo_map, 
             Color*   normal_map,
             Color*   ao_map,
             Color*   rough_map,
             bool     is_emissive,
             uint32_t image_width,
             uint32_t image_height):
        albedo_map  (albedo_map),
        normal_map  (normal_map),
        image_width (image_width),
        is_emissive (is_emissive),
        image_height(image_height),
        ambient_occlusion_map(ao_map),
        roughness_map(rough_map)
    {
    }

    virtual Vec3 emitted(const Vec2& uv) const override;
    virtual bool scatter(const Ray& r, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;
private:
    Color* albedo_map  = nullptr;
    Color* normal_map  = nullptr;
    Color* ambient_occlusion_map = nullptr;
    Color* roughness_map         = nullptr;

    bool     is_emissive;
    uint32_t image_width;
    uint32_t image_height;
};

class Emissive : public Material {
public:
    Emissive(const Vec3& col):
        color(col)
    {
        is_double_sided = true;
    }

    virtual Vec3 emitted(const Vec2& uv) const override;
    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered)
        const override;
    Vec3 color;
};

class Lambertian : public Material {
public:
    Lambertian(const Vec3& attenuation):
        albedo(attenuation) { }

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;
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

    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;
    Vec3 albedo;
    float   fuzziness;
};

class Dielectric : public Material {
public:
    Dielectric(const float rel_ior, 
               const Vec3& albedo = Vec3({ 1.0, 1.0, 1.0 }), 
               const float fuzziness = 0.0):
        rel_ior  (rel_ior),
        albedo   (albedo),
        fuzziness(fuzziness)
    {
    }
    virtual bool scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;
    Vec3 albedo;
    float rel_ior;
    float fuzziness = 0.0;
};

#endif
