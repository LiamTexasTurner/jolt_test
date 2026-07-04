#include "scene.hpp"
#include "preamble.glsl"

#include "stb_image.h"
#include "cgltf.h"

#include <iostream>

using namespace std;

template <typename T>
void load_attribute(cgltf_accessor* attribute, int num_comp, vector<T>& dst)
{
      int n = 0;
      dst.resize(num_comp * attribute->count);
      T* buffer = (T*)attribute->buffer_view->buffer->data +
                                     attribute->buffer_view->offset/sizeof(T) + attribute->offset/sizeof(T);
      
      for (unsigned int k = 0; k < attribute->count; k++) 
      {
            for (int l = 0; l < num_comp; l++) 
            {
                  dst[num_comp*k + l] = (T)buffer[n + l];
            }
            n += (int)(attribute->stride/sizeof(T));
      }
}

void Scene::Init()
{
      diffuse_maps = packed_freelist<DiffuseMap>(512);
      materials    = packed_freelist<Material>(512);
      meshes       = packed_freelist<Mesh>(512);
      transforms   = packed_freelist<Transform>(4096);
      instances    = packed_freelist<Instance>(4096);
      cameras      = packed_freelist<Camera>(32);
}

void LoadMeshes(Scene &scene, const string &filename, vector<uint32_t> *load_mesh_IDs)
{
      cgltf_options options{};
      cgltf_data* data = nullptr;
      string directory = filename.substr(0, filename.find_last_of('/'));

      if(cgltf_parse_file(&options, filename.c_str(), &data) != cgltf_result_success)
      {
            cout << "failed to load mesh" << endl;
      }
      if(cgltf_load_buffers(&options, data, filename.c_str()) != cgltf_result_success)
      {
            cout << "failed to load mesh buffers" << endl;
      }

      int prim_count = 0;

      for(int i = 0; i < data->nodes_count; i++)
      {
            cgltf_node* node = &(data->nodes[i]);
            cgltf_mesh* mesh = node->mesh;
            if(!mesh) continue;

            for(int p = 0; p < mesh->primitives_count; p++)
            {
                  if(mesh->primitives[p].type == cgltf_primitive_type_triangles)
                  {
                        prim_count++;
                  }
            }
      }

      

      //add materials to the scene
      map<string, uint32_t> diffuse_map_cache;
      vector<uint32_t> new_material_IDs;
      for(int i = 0; i < data->materials_count; i++)
      {

            cgltf_material* mat = &data->materials[i];
            
            Material new_material;
            new_material.name = mat->name;

            if(mat->has_pbr_metallic_roughness)
            {
                  cgltf_texture_view* base_color = &mat->pbr_metallic_roughness.base_color_texture;

                  if(base_color->texture)
                  {
                        cgltf_texture* tex = base_color->texture;
                        if(tex->image)
                        {
                              cgltf_image* image = tex->image;
                              const char* uri = image->uri;

                              GLuint new_diffuse_map_TO = texture_from_file(uri, directory, false);
                              
                              DiffuseMap new_diffuse_map;
                              new_diffuse_map.DiffuseMapTO = new_diffuse_map_TO;

                              uint32_t new_diffuse_map_ID = scene.diffuse_maps.insert(new_diffuse_map);

                              diffuse_map_cache.emplace(mat->name, new_diffuse_map_ID);
                        }
                  }
            }
            uint32_t new_material_id = scene.materials.insert(new_material);
            new_material_IDs.push_back(new_material_id);
      }

      //add meshes (and prototypes) to the scene

      for(int i = 0; i < data->nodes_count; i++)
      {
            cgltf_node* node = &(data->nodes[i]);
            cgltf_mesh* mesh = node->mesh;
            if(!mesh) continue;

            
      
            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                  if(mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                  Mesh new_mesh;
                  for(unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                  {
                        if(mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)
                        {
                              cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;
                              int vert_count = (int)attribute->count;
                              new_mesh.vertex_count = vert_count;
                              vector<float> vert_positions(vert_count * 3);
                              load_attribute(attribute, 3, vert_positions);

                              GLuint new_position_BO;
                              glGenBuffers(1, &new_position_BO);
                              glBindBuffer(GL_ARRAY_BUFFER, new_position_BO);
                              glBufferData(GL_ARRAY_BUFFER, vert_positions.size() * sizeof(float), vert_positions.data(), GL_STATIC_DRAW);

                              new_mesh.postion_BO = new_position_BO;
                        }
                  }

                  //hook up VAO
                  {
                        GLuint new_mesh_VAO;
                        glGenVertexArrays(1, &new_mesh_VAO);
                        glBindVertexArray(new_mesh_VAO);
      
                        if(new_mesh.postion_BO)
                        {
                              glBindBuffer(GL_ARRAY_BUFFER, new_mesh.postion_BO);
                              glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
                              glBindBuffer(GL_ARRAY_BUFFER, 0);
                              glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);
                        }
                  }

                  uint32_t new_mesh_ID = scene.meshes.insert(new_mesh);
                  if(load_mesh_IDs)
                  {
                        load_mesh_IDs->push_back(new_mesh_ID);   
                  }
            }
            
      }
}

void AddInstance(Scene &scene, uint32_t mesh_ID, uint32_t *new_instance_ID)
{
      Transform new_transform;
      new_transform.scale = glm::vec3(1.0f);

      uint32_t new_transform_ID = scene.transforms.insert(new_transform);

      Instance new_instance;
      new_instance.mesh_ID = mesh_ID;
      new_instance.transform_ID = new_transform_ID;

      uint32_t tmp_new_instance_ID = scene.instances.insert(new_instance);
      if(new_instance_ID)
      {
            *new_instance_ID = tmp_new_instance_ID;
      }
};

unsigned int texture_from_file(std::string uri, const std::string &directory, bool gamma)
{
      std::string filename = std::string(uri);
      filename = directory + '/' + filename;

      unsigned int textureID;
      glGenTextures(1, &textureID);

      int width, height, nrComponents;
      unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
      if (data)
      {
            GLenum format;
            if (nrComponents == 1)
                  format = GL_RED;
            else if (nrComponents == 3)
                  format = GL_RGB;
            else if (nrComponents == 4)
                  format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
      }
      else
      {
            std::cout << "Texture failed to load at path: " << uri << std::endl;
            stbi_image_free(data);
      }

      return textureID;
}

