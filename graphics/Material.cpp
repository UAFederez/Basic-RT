#include "Material.h"

Vec3 Material::emitted(const Vec2&) const
{
    return Vec3({0.0, 0.0, 0.0});
}

Vec3 Textured::emitted(const Vec2& uv) const 
{
    if(!is_emissive)
        return Vec3({ 0.0, 0.0, 0.0 });

    Vec2 texel   = Vec2({ std::floor(uv.u() * scalar(image_width  - 1)) , 
                          std::floor(uv.v() * scalar(image_height - 1)) });
    uint32_t idx = uint32_t(texel.v() * scalar(image_width) + texel.u());
    return albedo_map[ idx ];
}

bool Textured::scatter(const Ray& r, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const 
{
    if(is_emissive)
        return false;

    // Get texel (U, V) from the UV coordinates
    Vec2 texel   = Vec2({ std::floor(rec.uv.u() * scalar(image_width  - 1)), 
                          std::floor(rec.uv.v() * scalar(image_height - 1)) });
    uint32_t idx = uint32_t(texel.v() * scalar(image_width) + texel.u());

    // Bilinear Interpolation
    Vec2 t1 = Vec2({ std::floor(rec.uv.u() * scalar(image_width  - 1)), 
                     std::floor(rec.uv.v() * scalar(image_height - 1)) });
    Vec2 t2 = Vec2({ std::ceil (rec.uv.u() * scalar(image_width  - 1)), 
                     std::floor(rec.uv.v() * scalar(image_height - 1)) });
    Vec2 t3 = Vec2({ std::floor(rec.uv.u() * scalar(image_width  - 1)), 
                     std::ceil (rec.uv.v() * scalar(image_height - 1)) });
    Vec2 t4 = Vec2({ std::ceil (rec.uv.u() * scalar(image_width  - 1)), 
                     std::ceil (rec.uv.v() * scalar(image_height - 1)) });

    int idx1 = clamp(int(t1.v() * image_width + t1.u() - 1), 0, int(image_width * image_height - 1));
    int idx2 = clamp(int(t2.v() * image_width + t2.u() + 1), 0, int(image_width * image_height - 1));
    int idx3 = clamp(int(t3.v() * image_width + t3.u() + image_width), 0, int(image_width * image_height - 1));
    int idx4 = clamp(int(t4.v() * image_width + t4.u() - image_width), 0, int(image_width * image_height - 1));

    scalar x_interp = rec.uv.u() * image_width  - t1.u();
    scalar y_interp = rec.uv.v() * image_height - t1.v();

    Vec3 tex1 = albedo_map[idx1];
    Vec3 tex2 = albedo_map[idx2];
    Vec3 tex3 = albedo_map[idx3];
    Vec3 tex4 = albedo_map[idx4];

    Vec3 a1  = (1.0 - x_interp) * tex1 + (x_interp * tex2);
    Vec3 a2  = (1.0 - x_interp) * tex3 + (x_interp * tex4);
    Vec3 att_filtered = (1.0 - y_interp) * a1  + (y_interp * a2);

    // Normal map
    Vec3 reflected    = reflect(normalize(r.direction()), rec.normal) + random_in_unit_sphere();
    Vec3 final_normal = rec.normal;

    // TODO: Option for bilinear filtering on image
    attenuation = att_filtered;

    if(normal_map != nullptr)
    {
        Vec3 texn1 = normal_map[idx1];
        Vec3 texn2 = normal_map[idx2];
        Vec3 texn3 = normal_map[idx3];
        Vec3 texn4 = normal_map[idx4];

        Vec3 n1  = (1.0 - x_interp) * texn1 + (x_interp * texn2);
        Vec3 n2  = (1.0 - x_interp) * texn3 + (x_interp * texn4);
        Vec3 norm_interp = (1.0 - y_interp) * n1  + (y_interp * n2);

        Vec3 tex_normal = norm_interp;

        // TODO: Calculate TBN
        final_normal = normalize(Vec3({
                    dot(tex_normal, Vec3({rec.tangent.x(), rec.bitangent.x(), rec.normal.x()})),
                    dot(tex_normal, Vec3({rec.tangent.y(), rec.bitangent.y(), rec.normal.y()})),
                    dot(tex_normal, Vec3({rec.tangent.z(), rec.bitangent.z(), rec.normal.z()}))
                }));

        Vec3 rough_texel     = Vec3({ 0.0, 0.0, 0.0  });
        Vec3 occlusion_texel = Vec3({ 1.0, 1.0, 1.0  });

        if(roughness_map)
            rough_texel = roughness_map[idx];
        if(ambient_occlusion_map)
            occlusion_texel = ambient_occlusion_map[idx];

        attenuation = albedo_map[idx] * occlusion_texel;
        reflected   = reflect(normalize(r.direction()), rec.normal) + (rough_texel * random_in_unit_sphere());
    }
    scattered = Ray(rec.point_at_t, reflected);
    return true;
}

Vec3 Emissive::emitted(const Vec2&) const 
{
    return color;
}

bool Emissive::scatter(const Ray&, const HitRecord&, Vec3&, Ray&)
    const 
{
    return false;
}

bool Lambertian::scatter(const Ray&, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const 
{
    Vec3 target = rec.point_at_t + rec.normal + random_in_unit_sphere();
    scattered   = Ray(rec.point_at_t, target - rec.point_at_t);
    attenuation = albedo;
    return true;
}

bool Metal::scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const 
{
    Vec3 reflected = reflect(normalize(ray_in.direction()), rec.normal);
    scattered      = Ray(rec.point_at_t, reflected + fuzziness * random_in_unit_sphere());
    attenuation    = albedo;
    return true;
}

bool Dielectric::scatter(const Ray& ray_in, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
{
    Vec3  nrm = Vec3({0.0, 0.0, 0.0});
    scalar ior = 0.0f;

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
    Vec3 reflected = reflect(ray_in.direction(), nrm);
    attenuation    = albedo;

    if(refracted.magnitude_squared() == 0) 
        scattered = Ray(rec.point_at_t, reflected);
    else
    {
        // Calculate the Fresnel effect based on Schlick's approximation
        scalar cosine          = dot(nrm, -normalize(ray_in.direction()));
        scalar reflection_prob = schlick_approx(ior, cosine);

        if(random_scalar() < reflection_prob)
            scattered = Ray(rec.point_at_t, reflected + fuzziness * random_in_unit_sphere());
        else
            scattered = Ray(rec.point_at_t, refracted + fuzziness * random_in_unit_sphere());
    }
    return true;
}
