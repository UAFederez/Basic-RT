#include "Scene.h"

Scene::Scene(const std::string& file_path)
{
    read_from_file(file_path);
}

bool string_begins_with_any_of(const std::string& line,
                        const std::initializer_list<std::string>& args)
{
    for(const std::string& key : args)
    {
        if(line.find(key) == 0)
            return true;
    }
    return false;
}

void Scene::read_from_file(const std::string& file_path)
{
    std::ifstream description_file(file_path, std::ios::in);
    
    if(!description_file)
        throw std::runtime_error("[Error]" + file_path + " cannot be opened!\n");

    std::string line;

    uint32_t n_scene_params_so_far = 0;
    uint32_t n_materials_so_far    = 0;

    const std::initializer_list<std::string> scene_param_keywords = {
        "NAME", "IMG_WIDTH", "IMG_HEIGHT", "NUM_THREADS", "NUM_SAMPLES", "MAX_RDEPTH"
        "AMBIENT", "CAM_POS", "CAM_LOOK"
    };
    const std::initializer_list<std::string> material_keywords = {
        "LAMBERTIAN", "METAL", "DIELECTRIC", "TEXTURED", "EMISSIVE"
    };
    const std::initializer_list<std::string> primitive_keywords = {
        "p_SPHERE", "p_TRIANGLE", "p_RECTANGLE3D", 
    };

    while(std::getline(description_file, line))
    {
        if(line.find("#") == 0 || line.empty()) // Ignore comments
            continue;

        if(string_begins_with_any_of(line, scene_param_keywords))
        {
            read_scene_parameters(line);
            n_scene_params_so_far++;
        } else if(n_scene_params_so_far < 5)
            throw std::runtime_error("[Error] Please specify scene parameters first");

        if(string_begins_with_any_of(line, material_keywords))
        {
            n_materials_so_far++;
            read_scene_materials(line);
        }

        if(string_begins_with_any_of(line, primitive_keywords))
        {
            if(n_materials_so_far == 0)
                throw std::runtime_error("[Error] Please specify at least one material before any primitives");
            else
                read_scene_primitives(line);
        }

        // TODO: Meshes

        // Camera parameters
    }
}

void Scene::read_scene_materials(const std::string& line)
{
    std::string dummy;
    std::istringstream iss(line);

    Material* material = nullptr;

    iss >> dummy;   
    if(line.find("LAMBERTIAN") == 0)
    {
        float albedo_r, albedo_g, albedo_b;

        if( !(iss >> albedo_r >> albedo_g >> albedo_b) )
            throw std::runtime_error("[Error] Please specify correct albedo RGB values");
        else
        {
            std::cout << "[INFO ] (Lambertian) "
                      << "R: "   << albedo_r 
                      << ", G: " << albedo_g 
                      << ", B: " << albedo_b << '\n';
            material = new Lambertian(Color({ albedo_r, albedo_g, albedo_b }) );
        }

    } else if(line.find("DIELECTRIC") == 0)
    {
        float albedo_r, albedo_g, albedo_b, ior;

        if( !(iss >> albedo_r >> albedo_g >> albedo_b >> ior) )
            throw std::runtime_error("[Error] Invalid dielectric parameters specified");
        else
        {
            std::cout << "[INFO ] (Dielectric) "
                      << "R: "   << albedo_r << ", "
                      << "G: "   << albedo_g << ", "
                      << "B: "   << albedo_b << ", "
                      << "ior: " << ior << '\n';
            material = new Dielectric(ior, Color({ albedo_r, albedo_g, albedo_b }));
        }
    } else if(line.find("METAL") == 0)
    {
        float albedo_r, albedo_g, albedo_b, fuzziness;

        if( !(iss >> albedo_r >> albedo_g >> albedo_b >> fuzziness) )
            throw std::runtime_error("[Error] Invalid metal parameters specified");
        else
        {
            std::cout << "[INFO ] (Metal) "
                      << "R: " << albedo_r << ", "
                      << "G: " << albedo_g << ", "
                      << "B: " << albedo_b << ", "
                      << "Fuzziness: " << fuzziness << '\n';
            material = new Metal(Color({ 1.0, 1.0, 1.0 }), 0.01);
        }
    } else if(line.find("EMISSIVE") == 0)
    {
        float color_r, color_g, color_b;
        if( !(iss >> color_r >> color_g >> color_b) )
            throw std::runtime_error("[Error] Invalid emissive parameters specified");
        else
        {
            std::cout << "[INFO ] (Emissive) "
                      << "R: " << color_r << ", "
                      << "G: " << color_g << ", "
                      << "B: " << color_b << '\n';
            material = new Emissive(Color({ color_r, color_g, color_b }) );
        }
    } else if(line.find("TEXTURED") == 0)
    {
        int32_t is_emissive_flag;
        iss >> is_emissive_flag;

        std::size_t albedo_start    = line.find("\'", 0);
        std::size_t albedo_end      = line.find("\'", albedo_start + 1);

        std::size_t normal_start    = line.find("\'", albedo_end + 1);
        std::size_t normal_end      = line.find("\'", normal_start + 1);

        std::size_t roughness_start = line.find("\'", normal_end + 1);
        std::size_t roughness_end   = line.find("\'", roughness_start + 1);

        std::size_t amb_occ_start   = line.find("\'", roughness_end + 1);
        std::size_t amb_occ_end     = line.find("\'", amb_occ_start + 1);

        std::string albedo_path    = line.substr(albedo_start + 1, albedo_end - albedo_start - 1);
        std::string normal_path    = line.substr(normal_start + 1, normal_end - normal_start - 1);
        std::string roughness_path = line.substr(roughness_start + 1, roughness_end - roughness_start - 1);
        std::string amb_occ_path   = line.substr(amb_occ_start + 1, amb_occ_end - amb_occ_start - 1);

        if(albedo_start == std::string::npos || albedo_end == std::string::npos)
            throw std::runtime_error("[ERROR] Please specify at least a color (albedo) map!\n");

        // TODO: Enforce specifying at least an albedo map
        // TODO: I/O can be multithreaded especially for large texture files
        int albedo_idx    = -1;
        int normal_idx    = -1;
        int roughness_idx = -1;
        int ao_idx        = -1;

        if(!albedo_path.empty())
        {
            std::cout << "[INFO ] Loading color map: " << albedo_path << "...\n";
            albedo_idx = load_texture_image(albedo_path);
        }
        if(!normal_path.empty())
        {
            std::cout << "[INFO ] Loading normal map: " << normal_path << "...\n";
            normal_idx = load_texture_image(normal_path);
        }
        if(!roughness_path.empty())
        {
            std::cout << "[INFO ] Loading roughness map: " << roughness_path << "...\n";
            roughness_idx = load_texture_image(roughness_path);
        }
        if(!amb_occ_path.empty())
        {
            std::cout << "[INFO ] Loading occlusion map: " << amb_occ_path << "...\n";
            ao_idx = load_texture_image(amb_occ_path);
        }

        std::cout << "[INFO ]    Albedo?    " << (albedo_idx    == -1 ? "No" : "Yes") << "\n"
                  << "[INFO ]    Normal?    " << (normal_idx    == -1 ? "No" : "Yes") << "\n"
                  << "[INFO ]    Roughness? " << (roughness_idx == -1 ? "No" : "Yes") << "\n"
                  << "[INFO ]    Occlusion? " << (ao_idx        == -1 ? "No" : "Yes") << "\n";

        Color* albedo_colors  = textures[albedo_idx].colors.data();
        Color* normal_colors  = normal_idx    != -1 ? textures[normal_idx].colors.data()   : nullptr;
        Color* rough_colors   = roughness_idx != -1 ? textures[roughness_idx].colors.data(): nullptr;
        Color* amb_occ_colors = ao_idx        != -1 ? textures[ao_idx].colors.data() : nullptr;

        material = new Textured(albedo_colors,
                                normal_colors,
                                amb_occ_colors,
                                rough_colors,
                                is_emissive_flag != 0,
                                textures[albedo_idx].width,
                                textures[albedo_idx].height);
    } else
    {
        throw std::runtime_error("[Error] Undefined parameter specified on line: \"" + line + "\"");
    }
    materials.push_back(std::unique_ptr<Material>(material));
}

void Scene::read_scene_primitives(const std::string& line)
{
    std::istringstream iss(line);
    std::string dummy; 

    iss >> dummy;   // consume label

    Mesh* mesh = new Mesh();
    if(line.find("p_SPHERE") == 0)
    {
        float   center_x, center_y, center_z, radius;
        uint32_t material_idx;
        if( !(iss >> center_x >> center_y >> center_z >> radius >> material_idx) &&
             (radius < 0.0f || material_idx > materials.size()) ) 
            throw std::runtime_error("[Error] Invalid sphere parameters specified");
        else
        {
            std::cout << "[INFO ] (Sphere) Center: (" 
                      << center_x << ", " 
                      << center_y << ", " << center_z << ") "
                      << "Radius: " << radius    << ' '
                      << "Material: " << material_idx << '\n';

            assert(material_idx < materials.size());
            mesh->add_primitive(new Sphere(Vec3({ center_x, center_y, center_z }), 
                                           radius, 
                                           materials[material_idx].get()));
        }
    }

    if(line.find("p_TRIANGLE") == 0)
    {
        float   x1, y1, z1, x2, y2, z2, x3, y3, z3; 
        uint32_t material_idx;
        if( !(iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> material_idx) ||
             (material_idx > materials.size()))
            throw std::runtime_error("[Error] Invalid triangle parameters specified");
        {
            std::printf("[INFO ] (Triangle) (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), Material: %d\n",
                         x1, y1, z1, x2 ,y2, z2, x3, y3, z3, material_idx);

            //TODO:
            //mesh->add_primitive(new Triangle(Vec3({ x1, y1, z1 }), 
            //                                 Vec3({ x2, y2, z2 }), 
            //                                 Vec3({ x3, y3, z3 }), 
            //                                 materials[material_idx]));
        }
    }
    //TODO
    if(line.find("p_RECTANGLE3D") == 0)
    {
        
    }
    meshes.push_back(std::unique_ptr<Mesh>(mesh));
}

void Scene::read_scene_parameters(const std::string& line)
{
    std::string dummy;

    std::istringstream iss(line);

    // Scene name
    iss >> dummy;
    if(line.find("NAME") == 0)
    {
        std::getline(iss, name);
        std::size_t quote_start = name.find("\"");
        std::size_t quote_end   = name.find("\"", quote_start + 1);

        if(quote_start == std::string::npos ||
                quote_end   == std::string::npos)
            std::cerr << "[WARNING] No name for this scene was specified\n";
        else
        {
            name = name.substr(quote_start + 1, quote_end - quote_start - 1);
            std::cout << "[INFO ] Scene name: " << '\'' << name << '\'' << '\n';
        }
    } 
    else if(line.find("IMG_WIDTH") == 0)
    {
        iss >> image_width;
        std::cout << "[INFO ] Width:  " << image_width << '\n';
    } 
    else if(line.find("IMG_HEIGHT") == 0)
    {
        if(!(iss >> image_height))
            throw std::runtime_error("[Error] Invalid parameter specified for IMG_HEIGHT");
        else
            std::cout << "[INFO ] Height: " << image_height << '\n';
    } 
    else if(line.find("NUM_THREADS") == 0)
    {
        if(!(iss >> num_threads))
            throw std::runtime_error("[Error] Invalid parameter specified for NUM_THREADS");
    } 
    else if(line.find("NUM_SAMPLES") == 0)
    {
        if(!(iss >> num_samples))
            throw std::runtime_error("[Error] Invalid parameter specified for NUM_SAMPLES");
    }
    else if (line.find("MAX_RDEPTH") == 0)
    {
        if(!(iss >> max_recursion_depth))
            throw std::runtime_error("[Error] Invalid parameter specified for MAX_RDEPTH");
    }
    else if (line.find("CAM_POS") == 0)
    {
        if(!(iss >> camera_pos[0] >> camera_pos[1] >> camera_pos[2]))
            throw std::runtime_error("[Error] Invalid parameter specified for CAM_POS");
    }
    else if (line.find("CAM_LOOK") == 0)
    {
        if(!(iss >> camera_look[0] >> camera_look[1] >> camera_look[2]))
            throw std::runtime_error("[Error] Invalid parameter specified for CAM_LOOK");
    }
    else if (line.find("AMBIENT") == 0)
    {
        if(!(iss >> ambient[0] >> ambient[1] >> ambient[2]))
            throw std::runtime_error("[Error] Invalid parameter specified for AMBIENT");
    }
    else // TODO
    {

    }
        
      
}

bool Scene::anything_hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
{
    HitRecord temp_rec = {};
    float closest      = t_max;
    bool hit_anything  = false;

    HitRecord bv_rec   = {};    // Not really useful since only hit/no hit matters

    for(const auto& mesh : meshes)
    {
        bool intersects_bv = false;

        // Don't bother checking with each face of the bounding box if there
        // are fewer primitives than there are faces of the bounding box
        if(mesh->primitives.size() > mesh->bounding_volume_faces.size())
        {
            for(const auto& face : mesh->bounding_volume_faces)
            {
                // Not using FLT_MAX causes farther objects to be 'cut off'
                // because a bounding box face is nearer even though the 
                // area itself may be empty enough to see the further object
                if(face->hit(r, t_min, FLT_MAX, bv_rec))  
                {
                    intersects_bv = true;
                    break;
                }
            }
        } else
            intersects_bv = true; // Go straight to intersection check against primitives

        /**
         * If so, then check hit with each of the primitives 
         * Hitting a bounding volume does not necessarily mean the ray hits a primitive 
         * if so, this is a false positive, which is fine
         **/
        if(intersects_bv)
        {
            for(const Primitive* prim : mesh->primitives)
            {
                if(prim->hit(r, t_min, closest, temp_rec))
                {
                    closest      = temp_rec.t;
                    rec          = temp_rec;
                    hit_anything = true;
                }
            }
        } // Else, no hit
    }
    return hit_anything;
}

// Keep this for now for future testing
bool Scene::anything_hit_by_ray(const Ray&  r, const float t_min, const float t_max, HitRecord&  rec) const
{
    HitRecord temp_rec = {};
    float closest      = t_max;
    bool hit_anything  = false;

    for(const auto& m : meshes)
    {
        for(const auto& obj : m->primitives)
        {
            if(obj->hit(r, t_min, closest, temp_rec))
            {
                closest      = temp_rec.t;
                rec          = temp_rec;
                hit_anything = true;
            }
        }
    }
    return hit_anything;
}

// Deallocate scene objects
Scene::~Scene() 
{
}

std::vector<Color> convert_bmp_to_vec3(uint8_t* pixel_bytes, const uint32_t tex_width, const uint32_t tex_height)
{
    const float denom  = 1 / float(256);
    uint8_t* curr_byte = pixel_bytes;

    std::vector<Color> colors = {};
    colors.reserve(tex_width * tex_height);

    for(uint32_t y = 0; y < tex_height; y++)
    {
        for(uint32_t x = 0; x < tex_width; x++)
        {
            Vec3 a = Vec3({ float(*(curr_byte + 2)) * denom,
                            float(*(curr_byte + 1)) * denom,
                            float(*(curr_byte + 0)) * denom });
            colors.push_back(a);

            curr_byte += 3;
        }
    }
    return std::move(colors);
}

int Scene::load_texture_image(const std::string& path)
{
    textures.push_back(TextureImage{});
    TextureImage& texture = textures[textures.size() - 1];

    texture.file_path = path;
    texture.pixels = read_from_bmp_file(texture.file_path.c_str(),
                                        &texture.width,
                                        &texture.height,
                                        &texture.bytes_per_pixel); 
    if(texture.pixels == nullptr)
        throw std::runtime_error("[Error] Could not read image file: " + path);

    texture.colors = convert_bmp_to_vec3(texture.pixels.get(), texture.width, texture.height);
    return textures.size() - 1;
}

