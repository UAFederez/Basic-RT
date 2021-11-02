#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <cfloat>

#include "util/ProgressBar.h"
#include "util/BitmapImage.h"

#include "math/Vector3.h"
#include "math/Ray.h"

#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "graphics/Sphere.h"
#include "graphics/Triangle.h"
#include "graphics/Plane.h"
#include "graphics/World.h"

Vector3 color(const Ray& r, const World& world, int depth)
{
    HitRecord rec = {};
    if(world.hit_anything(r, 1e-3, FLT_MAX, rec))
    {
        Ray     scattered;
        Vector3 attenuation;
        
        if(depth < 50 && rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return comp_mul(attenuation, color(scattered, world, depth + 1));
        else
            return Vector3(0.0, 0.0, 0.0);
    }
    Vector3 unit_dir = normalize(r.direction());
    float t = 0.5 * (unit_dir.y() + 1.0f);
    return (1.0 - t) * Vector3(1.0, 1.0, 1.0) + t * Vector3(0.5, 0.7, 1.0);
}
int main()
{
    srand(time(0));
    const int IMAGE_WIDTH  = 1280;
    const int IMAGE_HEIGHT = 720;
    const int NUM_SAMPLES  = 1;
    const float NS_DENOM   = 1 / float(NUM_SAMPLES);

    std::vector<uint8_t>   pixels;
    std::vector<Material*> materials;

    pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    // Scene description
    World scene = {};
    scene.objects.push_back(new Plane (Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 
                                       new Metal(Vector3(0.8, 0.8, 0.8), 0.0)));
    scene.objects.push_back(new Plane (Vector3(0.0, 0.0,-4), Vector3(0.0, 0.0, 1.0), 
                                       new Metal(Vector3(0.8, 0.8, 0.8), 0.05)));
    scene.objects.push_back(new Sphere(Vector3( 0.0, 1.5, 0.0), 1.5, 
                                       new Dielectric(1.5)));

    float ring_radius = 2.0f;
    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < (9 + j) * (j + 1); i++)
        {
            Material* mat;
            float mat_prob = float(rand()) / float(RAND_MAX);
            if(mat_prob < 0.40)
            {
                mat = new Lambertian(Vector3(0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                             0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                             0.5 + float(rand())/float(RAND_MAX) * 0.5));
            } else if(mat_prob < 0.80)
            {
                mat = new Metal(Vector3(0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                        0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                        0.5 + float(rand())/float(RAND_MAX) * 0.5),
                                        0.1);
            } else
                mat = new Dielectric(1.5);

            const float theta  = (360 / ((9 + j) * (j + 1))) * i;
            const float x_pos = cos(theta * M_PI / 180.0f) * ring_radius;
            const float z_pos = sin(theta * M_PI / 180.0f) * ring_radius;

            materials.push_back(mat);
            scene.objects.push_back(new Sphere(Vector3(x_pos, 0.5, z_pos), 0.5, mat));

        }
        ring_radius *= 2;
    }

    // Camera description
    const float view_rot = 90.0f;
    const float dist     = 8.0f;
    Camera main_camera(Vector3(cos(view_rot * M_PI / 180) * dist, 3, 
                               sin(view_rot * M_PI / 180) * dist), 
                       Vector3( 0,1.0, 0),
                       Vector3( 0,1.0, 0),
                       30,
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // For measuring the current progress
    Progress progress = {};
    progress.width    = IMAGE_WIDTH;
    progress.height   = IMAGE_HEIGHT;
    progress.finished = 0;

    ThreadInfo thread_info;
    create_thread(&progress, &thread_info);

    for( int row = 0; row < IMAGE_HEIGHT; row++ )
    {
        for( int col = 0; col < IMAGE_WIDTH; col++ )
        {
            Vector3 pixel_color = {};
            for(int i = 0; i < NUM_SAMPLES; i++)
            {
                float u = float(col + float(rand())/float(RAND_MAX)) / float(IMAGE_WIDTH);
                float v = float(row + float(rand())/float(RAND_MAX)) / float(IMAGE_HEIGHT);

                Ray r  = main_camera.get_ray(u, v);
                pixel_color += color(r, scene, 0);
            }
            pixel_color *= NS_DENOM;
            pixel_color  = Vector3(sqrt(pixel_color.x()), sqrt(pixel_color.y()), sqrt(pixel_color.z()));

            uint8_t image_red   = uint8_t(255.99 * pixel_color.x());
            uint8_t image_green = uint8_t(255.99 * pixel_color.y());
            uint8_t image_blue  = uint8_t(255.99 * pixel_color.z());

            pixels.push_back(image_blue);
            pixels.push_back(image_green);
            pixels.push_back(image_red);
            progress.finished++;
        }
    }

    join_thread(&thread_info);

    write_bmp_to_file("output.bmp", pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);
    // Cleanup
    for(Material* mat : materials)
        delete mat;
    return 0;
}
