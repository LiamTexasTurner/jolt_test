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
      skyboxes     = packed_freelist<Skybox>(32);
}

void LoadMeshes(Scene &scene, const string &filename, vector<uint32_t> *load_mesh_IDs)
{
      Mesh mesh_result{};
      
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

                              new_material.diffuse_map_ID = new_diffuse_map_ID;
                        }
                  }
            }
            uint32_t new_material_id = scene.materials.insert(new_material);
            new_material_IDs.push_back(new_material_id);
      }

      vector<float> positions;
      vector<float> tex_coords;
      vector<float> normals;
      vector<uint32_t> indices;

      for(int i = 0; i < data->nodes_count; i++)
      {
            cgltf_node *node = &(data->nodes[i]);
            cgltf_mesh *mesh = node->mesh;
            if(!mesh) continue;

            for(int prim_index = 0; prim_index < mesh->primitives_count; prim_index++)
            {
                  cgltf_primitive* primitive = &mesh->primitives[prim_index];

                  if(primitive->type != cgltf_primitive_type_triangles)
                  {
                        continue;
                  }

                  uint32_t base_vertex = 0;
                  uint32_t vertex_count = 0;
                  for(int attrib_index = 0; attrib_index < primitive->attributes_count; attrib_index++)
                  {
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_position)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              base_vertex = static_cast<uint32_t>(positions.size() / 3);
                              vertex_count = static_cast<uint32_t>(attribute->count);
      
                              vector<float> primitive_positions(vertex_count * 3);

                              load_attribute(attribute, 3, primitive_positions);

                              positions.insert(positions.end(), primitive_positions.begin(), primitive_positions.end());
                        }
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_texcoord)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              vector<float> prim_tex_coords(attribute->count * 2);
                              load_attribute(attribute, 2, prim_tex_coords);

                              tex_coords.insert(tex_coords.end(), prim_tex_coords.begin(), prim_tex_coords.end());     
                        }
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_normal)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              vector<float> prim_normals(attribute->count * 3);
                              load_attribute(attribute, 3, prim_normals);
                              normals.insert(normals.end(), prim_normals.begin(), prim_normals.end());
                        }

                  }

                  uint32_t first_index = static_cast<uint32_t>(indices.size());

                  if(primitive->indices)
                  {
                        cgltf_accessor* index_accessor = primitive->indices;

                        for(int index = 0; index < index_accessor->count; index++)
                        {
                              uint32_t local_index = static_cast<uint32_t>(cgltf_accessor_read_index(index_accessor, index));

                              indices.push_back(base_vertex + local_index);
                        }

                        GLDrawElementsIndirectCommand curr_draw_cmd{};

                        curr_draw_cmd.count = static_cast<GLuint>(index_accessor->count);
                        curr_draw_cmd.primCount = 1;
                        curr_draw_cmd.firstIndex = first_index;
                        curr_draw_cmd.baseVertex = 0;
                        curr_draw_cmd.baseInstance = 0;

                        mesh_result.draw_commands.push_back(curr_draw_cmd);
                  }

                  auto it = diffuse_map_cache.find(primitive->material->name);
                  if(it != diffuse_map_cache.end())
                  {
                        mesh_result.material_IDs.push_back(it->second);
                  }
            }            
            break;
      }
      if(positions.empty() || indices.empty())
      {
            cout << "glTF contains no triangle geometry" << endl;
            cgltf_free(data);
            return;
      }

      mesh_result.vertex_count = static_cast<GLuint>(positions.size() / 3);
      mesh_result.index_count = static_cast<GLuint>(indices.size());

      glGenVertexArrays(1, &mesh_result.mesh_VAO);
      glGenBuffers(1, &mesh_result.postion_BO);
      glGenBuffers(1, &mesh_result.tex_coord_BO);
      glGenBuffers(1, &mesh_result.normal_BO);
      glGenBuffers(1, &mesh_result.index_BO);

      glBindVertexArray(mesh_result.mesh_VAO);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.postion_BO);
      glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.tex_coord_BO);
      glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * sizeof(tex_coords[0]), tex_coords.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
      glEnableVertexAttribArray(SCENE_TEXCOORD_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.normal_BO);
      glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_NORMAL_ATTRIB_LOCATION);
     

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_result.index_BO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      if(data->meshes_count > 1)
      {
            cout << "!SKIPPED MESH! multiple meshes at: " << filename << endl;
      }

      cgltf_free(data);
      uint32_t new_mesh_ID = scene.meshes.insert(mesh_result);
      if(load_mesh_IDs)
      {
            load_mesh_IDs->push_back(new_mesh_ID);
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

void AddSkybox(Scene& scene, uint32_t* new_skybox_ID)
{
      Skybox new_skybox;
      *new_skybox_ID = scene.skyboxes.insert(new_skybox);
}

void RemoveInstance(Scene& scene, uint32_t instance_ID)
{
      assert(scene.instances.contains(instance_ID) && " tried to remove instance that doestn exist");
      
      uint32_t transform_ID = scene.instances[instance_ID].transform_ID;
      scene.transforms.erase(transform_ID);
      scene.instances.erase(instance_ID);
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

