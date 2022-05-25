#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <cfloat>
#include <random>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <sstream>
#include <memory>

#include <SFML/Graphics.hpp>

#include "math/Vector.h"

#include "util/BitmapImage.h"
#include "util/Threading.h"
#include "util/General.h"

#include "math/Ray.h"

#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "graphics/Sphere.h"
#include "graphics/Triangle.h"
#include "graphics/Rectangle3D.h"
#include "graphics/Plane.h"
#include "graphics/Scene.h"

Color color(const Ray& r, const Scene& world, uint32_t depth)
{
    HitRecord rec = {};
    if(world.anything_hit(r, 1e-3, FLT_MAX, rec))
    {
        Ray   scattered;
        Color attenuation;
        
        if(depth > world.max_recursion_depth)
           return Color({ 0.0, 0.0, 0.0 }); 
        
        Vec3 emitted = rec.material_ptr->emitted(rec.uv);

        if(rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return emitted + attenuation * color(scattered, world, depth + 1);

        return emitted;
    }
    return world.ambient;
}

int thread_render_image_tiles(RenderThreadControl* tcb)
{
    ImageRenderInfo* image = (ImageRenderInfo*)&tcb->image;

    bool continue_to_work = true;
    while(continue_to_work)
    {
        SectionRenderInfo* current_section = NULL;

        // Check if queue is not empty,
        lock_mutex(tcb);
        if(image->section_queue_front != image->sections.size())
            current_section = &image->sections[image->section_queue_front++];
        else
            continue_to_work = false;
        unlock_mutex(tcb);

        // color the current section of the image
        if(current_section != NULL && continue_to_work)
        {
            const uint32_t bounds_x = current_section->tile_x + current_section->tile_width;
            const uint32_t bounds_y = current_section->tile_y + current_section->tile_height;
            const scalar   NS_DENOM = 1 / scalar(image->num_samples);
            const scalar   IW_DENOM = 1 / scalar(image->image_width);
            const scalar   IH_DENOM = 1 / scalar(image->image_height);

            current_section->in_progress = true;
            for(uint32_t y = current_section->tile_y; y < bounds_y; y++ )
            {
                for(uint32_t x = current_section->tile_x; x < bounds_x; x++ )
                {
                    Vec3 pixel = {};
                    for(uint32_t i = 0; i < image->num_samples; i++)
                    {
                        scalar u = scalar(x + random_scalar()) * IW_DENOM;
                        scalar v = scalar(y + random_scalar()) * IH_DENOM;

                        Ray r  = image->camera->get_ray(u, v);
                        pixel += color(r, *image->world, 0);
                    }
                    pixel *= NS_DENOM;
                    image->pixels[y * image->image_width + x] = pixel; 
                }
            }
            current_section->is_finished = true;
            current_section->in_progress = false;
            current_section = NULL;
        } else
            break;

    }
    return 0;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::printf("Usage -- Raytracer.out [description file]\n");
        return 1;
    }

    Scene scene;
    try {
        scene.read_from_file(argv[1]);
    }
    catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }

    // Image rendering description
    const uint32_t IMAGE_WIDTH  = scene.image_width;
    const uint32_t IMAGE_HEIGHT = scene.image_height;
    const uint32_t TILE_WIDTH   = scene.tile_size;
    const uint32_t TILE_HEIGHT  = scene.tile_size;

    // Camera description
    Camera main_camera(scene.camera_pos, 
                       scene.camera_look, Vec3({ 0, 1.0, 0}),
                       scene.camera_fov,
                       scalar(IMAGE_WIDTH) / scalar(IMAGE_HEIGHT));

    // Rendering thread parameters
    const uint32_t NUM_SAMPLES  = scene.num_samples;
    const uint32_t NUM_THREADS  = scene.num_threads;

    std::vector<uint8_t>   image_pixels;

    image_pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    const uint32_t WIDTH_IN_TILES  = IMAGE_WIDTH  / TILE_WIDTH;
    const uint32_t HEIGHT_IN_TILES = IMAGE_HEIGHT / TILE_HEIGHT;
    const uint32_t ADDITIONAL_H    = IMAGE_HEIGHT % TILE_HEIGHT;
    const uint32_t ADDITIONAL_W    = IMAGE_WIDTH % TILE_WIDTH;

    printf("---------------------------------\n");
    printf("[INFO ] Scene parameter:\n");
    printf("[INFO ]     Image width:            %d\n", IMAGE_WIDTH);
    printf("[INFO ]     Image height:           %d\n", IMAGE_HEIGHT);
    printf("[INFO ]     Tile width:             %d\n", TILE_WIDTH);
    printf("[INFO ]     Tile height:            %d\n", TILE_HEIGHT);
    printf("[INFO ]     Width in tiles:         %d\n", WIDTH_IN_TILES);
    printf("[INFO ]     Height in tiles:        %d\n", HEIGHT_IN_TILES);
    printf("[INFO ]     Remainder tile width    %d\n", ADDITIONAL_W);
    printf("[INFO ]     Remainder tile height:  %d\n", ADDITIONAL_H);
    printf("[INFO ]     # of samples per pixel: %d\n", NUM_SAMPLES);
    printf("[INFO ]     # of render threads:    %d\n", NUM_THREADS);
    printf("---------------------------------\n");
    
    RenderThreadControl thread_control;
    thread_control.image = {};
    thread_control.image.image_width  = scene.image_width;
    thread_control.image.image_height = scene.image_height;
    thread_control.image.num_samples  = scene.num_samples;
    thread_control.image.world        = &scene;
    thread_control.image.camera       = &main_camera;
    thread_control.image.pixels       = std::vector<Vec3>(IMAGE_WIDTH * IMAGE_HEIGHT);
    thread_control.image.section_queue_front = 0;
    thread_control.thread_stats       = std::vector<int>(scene.num_threads);

    // Prepare the work units that must be performed by the threads
    for(uint32_t tile_y = 0; tile_y < HEIGHT_IN_TILES; tile_y++)
    {
        for(uint32_t tile_x = 0; tile_x < WIDTH_IN_TILES; tile_x++)
        {
            SectionRenderInfo section = {};
            section.tile_width  = TILE_WIDTH;
            section.tile_height = TILE_HEIGHT;
            section.tile_x      = tile_x * TILE_WIDTH;
            section.tile_y      = tile_y * TILE_HEIGHT;
            section.is_finished = false;
            section.in_progress = false;

            if(tile_x == WIDTH_IN_TILES - 1)
                section.tile_width  += ADDITIONAL_W;
            if(tile_y == HEIGHT_IN_TILES - 1)
                section.tile_height += ADDITIONAL_H;

            thread_control.image.sections.push_back(section);
        }
    }
    thread_control.image.total_sections = thread_control.image.sections.size();

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::seconds;

    auto time_begin = high_resolution_clock::now();
    std::cout << "[INFO   ] Creating threads...\n";

    // Initialize threads
    auto time_render_begin = high_resolution_clock::now();
    std::vector<ThreadHandle> render_threads(scene.num_threads);

    initialize_mutex     (&thread_control);
    create_render_threads(render_threads.data(), scene.num_threads, &thread_control);

    // GUI Display
    sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Render");
    sf::Image output;
    output.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color::Black);

    bool output_done = false;    
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // Draw
        window.clear(sf::Color::Black);

        lock_mutex(&thread_control);
        for(uint32_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            for(uint32_t x = 0; x < IMAGE_WIDTH; x++)
            {
                const int ypos = IMAGE_HEIGHT - 1 - y;
                const int xpos = x;
                const Vec3* pixel = &thread_control.image.pixels[ypos * IMAGE_WIDTH + xpos];
                output.setPixel(x, y, sf::Color(255.99 * sqrt(clamp(pixel->x(), 0.0f, 1.0f)), 
                                                255.99 * sqrt(clamp(pixel->y(), 0.0f, 1.0f)), 
                                                255.99 * sqrt(clamp(pixel->z(), 0.0f, 1.0f))));
            }
        }
        unlock_mutex(&thread_control);

        sf::Texture tex;
        tex.loadFromImage(output);
        sf::Sprite sprite;
        sprite.setTexture(tex);
        window.draw(sprite);

        uint32_t num_finished = 0;
        for(const SectionRenderInfo& section : thread_control.image.sections)
        {
            if(section.in_progress)
            {
                sf::RectangleShape rect(sf::Vector2f( (uint32_t) section.tile_width, 
                                                      (uint32_t) section.tile_height ));
                rect.setFillColor(sf::Color(0, 0, 0, 0.0));
                rect.setPosition (section.tile_x , IMAGE_HEIGHT - section.tile_y - section.tile_height);
                rect.setOutlineThickness(1.0);
                rect.setOutlineColor(sf::Color(255.0, 255.0, 255.0));

                window.draw(rect);
            }
            if(section.is_finished)
                num_finished++;
        }
        
        if(num_finished == thread_control.image.sections.size() && !output_done)
        {
            auto time_render_end   = high_resolution_clock::now();
            auto time_render_total = duration<scalar>(time_render_end - time_render_begin).count();
            std::cout << "The render took " << std::fixed << std::setprecision(2)
                      << time_render_total  << " seconds.\n";
            output_done = true;
        }

        window.display();

    }
    join_render_threads(render_threads.data(), scene.num_threads);
    cleanup_threads(&thread_control, render_threads.data(), scene.num_threads);

    for(Vec3& pixel : thread_control.image.pixels)
    {
        // Because .BMP file format stores image pixel colors in BGR format
        int ir = std::min(int(255.99 * sqrt(pixel.b())), 255);
        int ig = std::min(int(255.99 * sqrt(pixel.g())), 255);
        int ib = std::min(int(255.99 * sqrt(pixel.r())), 255);

        image_pixels.push_back(ir);
        image_pixels.push_back(ig);
        image_pixels.push_back(ib);
    }

    const std::string file_name = scene.name + ".bmp";
    write_bmp_to_file(file_name.c_str(), image_pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);
    std::printf("Saving rendered image to: %s\n", file_name.c_str());
    

    return 0;
}

