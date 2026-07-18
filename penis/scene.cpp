
#include "scene.hpp"
#include "preamble.glsl"

#include "stb_image.h"
#include "cgltf.h"
#include "dds_loader.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <assert.h>
#include <gli/gli.hpp>

#include <fstream>
#include <filesystem>
#include <span>
#include <iostream>

using namespace std;

#define ANIMDELAY 16.6667f
#define EPSILON 0.000001f

uint32_t LoadSkeleton(Scene& scene, cgltf_skin* skin);
void LoadBoneInfo(cgltf_skin* skin, vector<BoneInfo>& bones);
bool get_pose_at_time(cgltf_interpolation_type interpolationType, cgltf_accessor *input, cgltf_accessor *output, float time, void* data)
{
      if (interpolationType >= cgltf_interpolation_type_max_enum) return false;

      // Input and output should have the same count
      float tstart = 0.0f;
      float tend = 0.0f;
      int keyframe = 0;       // Defaults to first pose

      for (int i = 0; i < (int)input->count - 1; i++)
      {
            cgltf_bool r1 = cgltf_accessor_read_float(input, i, &tstart, 1);
            if (!r1) return false;

            cgltf_bool r2 = cgltf_accessor_read_float(input, i + 1, &tend, 1);
            if (!r2) return false;

            if ((tstart <= time) && (time < tend))
            {
                  keyframe = i;
                  break;
            }
      }

      

      float duration = fmaxf((tend - tstart), EPSILON);
      float t = (time - tstart)/duration;
      t = (t < 0.0f)? 0.0f : t;
      t = (t > 1.0f)? 1.0f : t;

      if (output->component_type != cgltf_component_type_r_32f) return false;

      if (output->type == cgltf_type_vec3)
      {
            switch (interpolationType)
            {
                  case cgltf_interpolation_type_step:
                  {
                        float tmp[3] = { 0.0f };
                        cgltf_accessor_read_float(output, keyframe, tmp, 3);
                        glm::vec3 v1 = {tmp[0], tmp[1], tmp[2]};
                        glm::vec3 *r = (glm::vec3*)data;

                        *r = v1;
                  } break;
                  case cgltf_interpolation_type_linear:
                  {
                        float tmp[3] = { 0.0f };
                        cgltf_accessor_read_float(output, keyframe, tmp, 3);
                        glm::vec3 v1 = {tmp[0], tmp[1], tmp[2]};
                        cgltf_accessor_read_float(output, keyframe+1, tmp, 3);
                        glm::vec3 v2 = {tmp[0], tmp[1], tmp[2]};
                        glm::vec3 *r = (glm::vec3*)data;

                        *r = glm::mix(v1, v2, t);
                  } break;
                  case cgltf_interpolation_type_cubic_spline:
                  {
                        // float tmp[3] = { 0.0f };
                        // cgltf_accessor_read_float(output, 3*keyframe+1, tmp, 3);
                        //   glm::vec3 v1 = {tmp[0], tmp[1], tmp[2]};
                        // cgltf_accessor_read_float(output, 3*keyframe+2, tmp, 3);
                        //   glm::vec3 tangent1 = {tmp[0], tmp[1], tmp[2]};
                        // cgltf_accessor_read_float(output, 3*(keyframe+1)+1, tmp, 3);
                        //   glm::vec3 v2 = {tmp[0], tmp[1], tmp[2]};
                        // cgltf_accessor_read_float(output, 3*(keyframe+1), tmp, 3);
                        //   glm::vec3 tangent2 = {tmp[0], tmp[1], tmp[2]};
                        //   glm::vec3 *r = (glm::vec3*)data;

                        // *r = Vector3CubicHermite(v1, tangent1, v2, tangent2, t);
                  } break;
                  default: break;
            }
      }
      else if (output->type == cgltf_type_vec4)
      {
            // Only v4 is for rotations, so we know it's a quaternion
            switch (interpolationType)
            {
                  case cgltf_interpolation_type_step:
                  {
                        float tmp[4] = { 0.0f };
                        cgltf_accessor_read_float(output, keyframe, tmp, 4);
                        glm::quat v1 = {tmp[3], tmp[0], tmp[1], tmp[2]};
                        glm::quat *r = (glm::quat*)data;

                        *r = v1;
                  } break;
                  case cgltf_interpolation_type_linear:
                  {
                        float tmp[4] = { 0.0f };
                        cgltf_accessor_read_float(output, keyframe, tmp, 4);
                        glm::quat v1 = {tmp[3], tmp[0], tmp[1], tmp[2]};
                        cgltf_accessor_read_float(output, keyframe+1, tmp, 4);
                        glm::quat v2 = {tmp[3], tmp[0], tmp[1], tmp[2]};
                        glm::quat *r = (glm::quat*)data;

                        *r = glm::slerp(v1, v2, t);
                  } break;
                  case cgltf_interpolation_type_cubic_spline:
                  {
                        // float tmp[4] = { 0.0f };
                        // cgltf_accessor_read_float(output, 3*keyframe+1, tmp, 4);
                        //   glm::quat v1 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                        // cgltf_accessor_read_float(output, 3*keyframe+2, tmp, 4);
                        //   glm::quat outTangent1 = {tmp[0], tmp[1], tmp[2], 0.0f};
                        // cgltf_accessor_read_float(output, 3*(keyframe+1)+1, tmp, 4);
                        //   glm::quat v2 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                        // cgltf_accessor_read_float(output, 3*(keyframe+1), tmp, 4);
                        //   glm::quat inTangent2 = {tmp[0], tmp[1], tmp[2], 0.0f};
                        //   glm::quat *r = (glm::quat*)data;

                        // v1 = QuaternionNormalize(v1);
                        // v2 = QuaternionNormalize(v2);

                        // if (Vector4DotProduct(v1, v2) < 0.0f)
                        // {
                        //     v2 = Vector4Negate(v2);
                        // }

                        // outTangent1 = Vector4Scale(outTangent1, duration);
                        // inTangent2 = Vector4Scale(inTangent2, duration);

                        // *r = QuaternionCubicHermiteSpline(v1, outTangent1, v2, inTangent2, t);
                  } break;
                  default: break;
            }
      }

      return true;
}


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

      float x = 0;
}

void Scene::Init()
{
      diffuse_maps   = packed_freelist<DiffuseMap>(512);
      materials      = packed_freelist<Material>(512);
      meshes         = packed_freelist<Mesh>(512);
      skeletons      = packed_freelist<Skeleton>(512);
      skinned_meshes = packed_freelist<SkinnedMesh>(512);
      animations     = packed_freelist<Animation>(512);
      transforms     = packed_freelist<Transform>(4096);
      instances      = packed_freelist<Instance>(4096);
      cameras        = packed_freelist<Camera>(32);
      skyboxes       = packed_freelist<Skybox>(32);

      //defualts

      // Material default_mat;
      // default_mat.base_color[0] = 0.0f;
      // default_mat.base_color[1] = 1.0f;
      // default_mat.base_color[2] = 1.0f;
      // default_mat.base_color[3] = 1.0f;
      // default_mat.diffuse_map_ID = -1;
      // materials.insert(default_mat);
      
}

void LoadMeshAsync(Scene& scene, MeshData& mesh_result, const std::string& filename)
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


      for(int i = 0; i < data->materials_count; i++)
      {
            cgltf_material* mat = &data->materials[i];
            
            MaterialLoadData mat_load_data;
            
            if(mat->has_pbr_metallic_roughness)
            {
                  cgltf_texture_view* base_color = &mat->pbr_metallic_roughness.base_color_texture;
                  if(base_color->has_transform)
                  {
                        mat_load_data.scale[0] = (float)base_color->transform.scale[0];
                        mat_load_data.scale[1] = (float)base_color->transform.scale[1];

                        mat_load_data.offset[0] = (float)base_color->transform.offset[0];
                        mat_load_data.offset[1] = (float)base_color->transform.offset[1];
                  }
                  if(base_color->texture)
                  {
                        cgltf_texture* tex = base_color->texture;
                        if(tex->image)
                        {
                              cgltf_image* image = tex->image;
                              const char* uri = image->uri;
                              mat_load_data.diffuse_path = directory + "/" + filesystem::path(uri).stem().string() + ".dds";
                        }
                  }
            }

            mesh_result.material_load_map.emplace(mat->name, mat_load_data);
      }       

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
                              base_vertex = static_cast<uint32_t>(mesh_result.positions.size() / 3);
                              vertex_count = static_cast<uint32_t>(attribute->count);
                              
                              vector<float> primitive_positions(vertex_count * 3);

                              load_attribute(attribute, 3, primitive_positions);

                              mesh_result.positions.insert(mesh_result.positions.end(), primitive_positions.begin(), primitive_positions.end());
                        }
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_texcoord)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              vector<float> prim_tex_coords(attribute->count * 2);
                              load_attribute(attribute, 2, prim_tex_coords);

                              mesh_result.tex_coords.insert(mesh_result.tex_coords.end(), prim_tex_coords.begin(), prim_tex_coords.end());     
                        }
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_normal)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              vector<float> prim_normals(attribute->count * 3);
                              load_attribute(attribute, 3, prim_normals);
                              mesh_result.normals.insert(mesh_result.normals.end(), prim_normals.begin(), prim_normals.end());
                        }

                  }

                  //TODO: this should be handled by a defualt material, a case where the material
                  //does not have an albedo texture
                  uint32_t first_index = static_cast<uint32_t>(mesh_result.indices.size());
                  if(primitive->indices)
                  {
                        cgltf_accessor* index_accessor = primitive->indices;

                        for(int index = 0; index < index_accessor->count; index++)
                        {
                              uint32_t local_index = static_cast<uint32_t>(cgltf_accessor_read_index(index_accessor, index));

                              mesh_result.indices.push_back(base_vertex + local_index);
                        }

                        DrawCommand curr_draw_cmd{};
                        
                        curr_draw_cmd.count = (index_accessor->count);
                        curr_draw_cmd.primCount = 1;
                        curr_draw_cmd.firstIndex = first_index;
                        curr_draw_cmd.baseVertex = 0;
                        curr_draw_cmd.baseInstance = 0;
                        curr_draw_cmd.has_transparecny = false;
                        curr_draw_cmd.material_name = primitive->material->name;
                        mesh_result.draw_commands.push_back(curr_draw_cmd);
                  }             
            }            
            break;
      }
      if(mesh_result.positions.empty() || mesh_result.indices.empty())
      {
            cout << "glTF contains no triangle geometry" << endl;
            cgltf_free(data);
            return;
      }

      mesh_result.vertex_count = static_cast<GLuint>(mesh_result.positions.size() / 3);
      mesh_result.index_count = static_cast<GLuint>(mesh_result.indices.size());

      

      string bin_folder = directory + "/" + "bin/";

      string asset_name = filesystem::path(directory).stem().string() + ".pbin";
      
      cout << asset_name << endl;
      ofstream of(bin_folder + asset_name, ios::binary);
      cereal::BinaryOutputArchive archive(of);
      archive(mesh_result);

}

uint32_t UploadMesh(Scene& scene, MeshData& mesh_data)
{
      Mesh mesh_result;
      mesh_result.vertex_count = mesh_data.vertex_count;
      mesh_result.index_count = mesh_data.index_count;
      mesh_result.draw_commands = mesh_data.draw_commands;
      mesh_result.material_IDs = mesh_data.material_IDs;
      

      for(DrawCommand& cmd : mesh_data.draw_commands)
      {
            
            auto it = mesh_data.material_load_map.find(cmd.material_name);
            if(it != mesh_data.material_load_map.end())
            {
                  MaterialLoadData& data = it->second;
                  
                  GLuint texture_ID = create_texture(data.diffuse_path.c_str());
                  
                  DiffuseMap new_diffuse_map;
                  new_diffuse_map.DiffuseMapTO = texture_ID;                  
                  new_diffuse_map.has_transparency = false;
                  uint32_t new_diffuse_map_ID = scene.diffuse_maps.insert(new_diffuse_map);
                       
                  Material new_material;
                  new_material.has_diffuse_map = true;
                  new_material.diffuse_map_ID = new_diffuse_map_ID;
                  

                  if(data.scale[0] != 1)
                  {
                        float x = 0;
                  }

                  new_material.offset = glm::vec2(data.offset[0], data.offset[1]);
                  new_material.scale = glm::vec2(data.scale[0], data.scale[1]);

                  
                  uint32_t new_material_ID = scene.materials.insert(new_material);
                  mesh_result.material_IDs.push_back(new_material_ID);

      
            }
            else
            {
                  Material new_material;
                  new_material.has_diffuse_map = false;
                  uint32_t new_material_ID = scene.materials.insert(new_material);
                  mesh_result.material_IDs.push_back(new_material_ID);
            }

      
      }
      
      glGenVertexArrays(1, &mesh_result.mesh_VAO);
      glGenBuffers(1, &mesh_result.postion_BO);
      glGenBuffers(1, &mesh_result.tex_coord_BO);
      glGenBuffers(1, &mesh_result.normal_BO);
      glGenBuffers(1, &mesh_result.index_BO);

      glBindVertexArray(mesh_result.mesh_VAO);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.postion_BO);
      glBufferData(GL_ARRAY_BUFFER, mesh_data.positions.size() * sizeof(float), mesh_data.positions.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.tex_coord_BO);
      glBufferData(GL_ARRAY_BUFFER, mesh_data.tex_coords.size() * sizeof(mesh_data.tex_coords[0]), mesh_data.tex_coords.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
      glEnableVertexAttribArray(SCENE_TEXCOORD_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.normal_BO);
      glBufferData(GL_ARRAY_BUFFER, mesh_data.normals.size() * sizeof(mesh_data.normals[0]), mesh_data.normals.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_NORMAL_ATTRIB_LOCATION);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_result.index_BO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data.indices.size() * sizeof(uint32_t), mesh_data.indices.data(), GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      uint32_t new_mesh_ID = scene.meshes.insert(mesh_result);

      return new_mesh_ID;
      
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
      map<string, uint32_t> material_map_cache;
      bool has_transparency = false;
      for(int i = 0; i < data->materials_count; i++)
      {

            cgltf_material* mat = &data->materials[i];
            
            Material new_material;
            new_material.name = mat->name;

            if(string_view("ocean_shore") == string_view(mat->name))
            {
                  float x = 0;
            }
            DiffuseMap new_diffuse_map;
            if(mat->has_pbr_metallic_roughness)
            {
                  cgltf_texture_view* base_color = &mat->pbr_metallic_roughness.base_color_texture;

                  if(base_color->has_transform)
                  {
                        new_material.scale = glm::vec2(base_color->transform.scale[0],
                                                       base_color->transform.scale[1]);
                        
                        new_material.offset = glm::vec2(base_color->transform.offset[0],
                                                        base_color->transform.offset[1]);
                        
                  }

                  if(base_color->texture)
                  {
                        cgltf_texture* tex = base_color->texture;
                        if(tex->image)
                        {
                              cgltf_image* image = tex->image;
                              const char* uri = image->uri;
                              GLuint new_diffuse_map_TO = texture_from_file(uri, directory, false);
                              new_diffuse_map.DiffuseMapTO = new_diffuse_map_TO;

                              if(mat->alpha_mode == cgltf_alpha_mode_blend ||
                                 mat->alpha_mode == cgltf_alpha_mode_mask  ||
                                 mat->pbr_metallic_roughness.base_color_factor[3] < 1.0f)
                              {
                                    new_diffuse_map.has_transparency = true;
                                    has_transparency = true;
                              }     
                        }
                        else
                        {
                              new_diffuse_map.DiffuseMapTO = -1;
                              new_diffuse_map.has_transparency = false;
                              has_transparency = false;
                        }
                  }
            }
            uint32_t new_diffuse_map_ID = scene.diffuse_maps.insert(new_diffuse_map);

            

            new_material.diffuse_map_ID = new_diffuse_map_ID;
            uint32_t new_material_ID = scene.materials.insert(new_material);
            material_map_cache.emplace(mat->name, new_material_ID);
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

                        DrawCommand curr_draw_cmd{};

                        curr_draw_cmd.count = index_accessor->count;
                        curr_draw_cmd.primCount = 1;
                        curr_draw_cmd.firstIndex = first_index;
                        curr_draw_cmd.baseVertex = 0;
                        curr_draw_cmd.baseInstance = 0;
                        curr_draw_cmd.has_transparecny = has_transparency;
                        mesh_result.draw_commands.push_back(curr_draw_cmd);

                        auto it = material_map_cache.find(primitive->material->name);
                        if(it != material_map_cache.end())
                        {
                              mesh_result.material_IDs.push_back(it->second);
                        }
                        else
                        {
                              float x = 0;
                        }                        
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

void LoadSkinnedMeshes(Scene &scene, const string &filename, vector<uint32_t> *load_mesh_IDs)
{
      SkinnedMesh skinned_mesh_result{};
      
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
      map<string, uint32_t> material_map_cache;
      bool has_transparency = false;
      for(int i = 0; i < data->materials_count; i++)
      {

            cgltf_material* mat = &data->materials[i];
            
            Material new_material;
            new_material.name = mat->name;

            if(string_view("ocean_shore") == string_view(mat->name))
            {
                  float x = 0;
            }
            DiffuseMap new_diffuse_map;
            if(mat->has_pbr_metallic_roughness)
            {
                  cgltf_texture_view* base_color = &mat->pbr_metallic_roughness.base_color_texture;

                  if(base_color->has_transform)
                  {
                        new_material.scale = glm::vec2(base_color->transform.scale[0],
                                                       base_color->transform.scale[1]);
                        
                        new_material.offset = glm::vec2(base_color->transform.offset[0],
                                                        base_color->transform.offset[1]);
                        
                  }

                  if(base_color->texture)
                  {
                        cgltf_texture* tex = base_color->texture;
                        if(tex->image)
                        {
                              cgltf_image* image = tex->image;
                              const char* uri = image->uri;
                              GLuint new_diffuse_map_TO = texture_from_file(uri, directory, false);
                              new_diffuse_map.DiffuseMapTO = new_diffuse_map_TO;

                              if(mat->alpha_mode == cgltf_alpha_mode_blend ||
                                 mat->alpha_mode == cgltf_alpha_mode_mask  ||
                                 mat->pbr_metallic_roughness.base_color_factor[3] < 1.0f)
                              {
                                    new_diffuse_map.has_transparency = true;
                                    has_transparency = true;
                              }     
                        }
                        else
                        {
                              new_diffuse_map.DiffuseMapTO = -1;
                              new_diffuse_map.has_transparency = false;
                              has_transparency = false;
                        }
                  }
            }
            uint32_t new_diffuse_map_ID = scene.diffuse_maps.insert(new_diffuse_map);

            

            new_material.diffuse_map_ID = new_diffuse_map_ID;
            uint32_t new_material_ID = scene.materials.insert(new_material);
            material_map_cache.emplace(mat->name, new_material_ID);
      }

      vector<float> positions;
      vector<float> tex_coords;
      vector<float> normals;
      vector<int>   bone_IDs;
      vector<float> bone_weights;
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
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_joints)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;                              
                              vector<unsigned char> prim_bone_IDs;
                              load_attribute(attribute, MAX_BONE_INFLUENCE, prim_bone_IDs);
                              bone_IDs.insert(bone_IDs.end(), prim_bone_IDs.begin(), prim_bone_IDs.end());
                        }
                        if(primitive->attributes[attrib_index].type == cgltf_attribute_type_weights)
                        {
                              cgltf_accessor* attribute = primitive->attributes[attrib_index].data;
                              vector<float> prim_bone_weights;
                              load_attribute(attribute, MAX_BONE_INFLUENCE, prim_bone_weights);
                              bone_weights.insert(bone_weights.end(), prim_bone_weights.begin(), prim_bone_weights.end());
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

                        DrawCommand curr_draw_cmd{};

                        curr_draw_cmd.count = index_accessor->count;
                        curr_draw_cmd.primCount = 1;
                        curr_draw_cmd.firstIndex = first_index;
                        curr_draw_cmd.baseVertex = 0;
                        curr_draw_cmd.baseInstance = 0;
                        curr_draw_cmd.has_transparecny = has_transparency;
                        skinned_mesh_result.draw_commands.push_back(curr_draw_cmd);

                        auto it = material_map_cache.find(primitive->material->name);
                        if(it != material_map_cache.end())
                        {
                              skinned_mesh_result.material_IDs.push_back(it->second);
                        }
                        else
                        {
                              float x = 0;
                        }     
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
      if(data->skins_count > 0)
      {
            cgltf_skin* skin = &data->skins[0];
            uint32_t skeleton_ID = -1;
            auto it = scene.skeleton_skinned_mesh_map.find(string(skin->name));
            if(it != scene.skeleton_skinned_mesh_map.end())
            {
                  skeleton_ID = it->second;
            }
            else
            {
                  skeleton_ID = LoadSkeleton(scene, skin);
                  
            }
            skinned_mesh_result.skeleton_ID = skeleton_ID;
      }

      skinned_mesh_result.vertex_count = static_cast<GLuint>(positions.size() / 3);
      skinned_mesh_result.index_count = static_cast<GLuint>(indices.size());

      glGenVertexArrays(1, &skinned_mesh_result.mesh_VAO);
      glGenBuffers(1, &skinned_mesh_result.postion_BO);
      glGenBuffers(1, &skinned_mesh_result.tex_coord_BO);
      glGenBuffers(1, &skinned_mesh_result.normal_BO);
      glGenBuffers(1, &skinned_mesh_result.bone_IDs);
      glGenBuffers(1, &skinned_mesh_result.bone_weights);
      glGenBuffers(1, &skinned_mesh_result.index_BO);

      glBindVertexArray(skinned_mesh_result.mesh_VAO);

      glBindBuffer(GL_ARRAY_BUFFER, skinned_mesh_result.postion_BO);
      glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, skinned_mesh_result.tex_coord_BO);
      glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * sizeof(tex_coords[0]), tex_coords.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
      glEnableVertexAttribArray(SCENE_TEXCOORD_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, skinned_mesh_result.normal_BO);
      glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
      glEnableVertexAttribArray(SCENE_NORMAL_ATTRIB_LOCATION);

      glBindBuffer(GL_ARRAY_BUFFER, skinned_mesh_result.bone_IDs);
      glBufferData(GL_ARRAY_BUFFER, bone_IDs.size() * sizeof(bone_IDs[0]), bone_IDs.data(), GL_STATIC_DRAW);
      glVertexAttribIPointer(SCENE_BONE_ID_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_INT, sizeof(int) * MAX_BONE_INFLUENCE, nullptr);
      glEnableVertexAttribArray(SCENE_BONE_ID_ATTRIB_LOCATION);
      
      
      glBindBuffer(GL_ARRAY_BUFFER, skinned_mesh_result.bone_weights);
      glBufferData(GL_ARRAY_BUFFER, bone_weights.size() * sizeof(bone_weights[0]), bone_weights.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(float) * MAX_BONE_INFLUENCE, nullptr);
      glEnableVertexAttribArray(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skinned_mesh_result.index_BO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      if(data->meshes_count > 1)
      {
            cout << "!SKIPPED MESH! multiple meshes at: " << filename << endl;
      }

      cgltf_free(data);
      uint32_t new_skinned_mesh_ID = scene.skinned_meshes.insert(skinned_mesh_result);
      if(load_mesh_IDs)
      {
            load_mesh_IDs->push_back(new_skinned_mesh_ID);
      }          
}

void LoadAnimation(Scene& scene, const std::string& filename, std::vector<uint32_t>* animation_IDs)
{
      cgltf_options options{};
      cgltf_data* data = nullptr;

      cgltf_result parse_result = cgltf_parse_file(&options, filename.c_str(), &data);
      if(cgltf_parse_file(&options, filename.c_str(), &data) != cgltf_result_success)
      {
            std::cout << "cgltf_parse_file failed: " << parse_result << std::endl;
      }
      if(cgltf_load_buffers(&options, data, filename.c_str()) != cgltf_result_success)
      {
            cout << "failed to load buffers in load_animatins()" << endl;
      }
      cgltf_result buffer_load_result = cgltf_load_buffers(&options, data, filename.c_str());
      if(buffer_load_result != cgltf_result_success)
      {
            cgltf_free(data);
            return;
      }


      int anim_count = data->animations_count;     
      vector<uint32_t> new_animation_IDs(anim_count);
      for(int i = 0; i < anim_count; i++)
      {
            new_animation_IDs[i] = scene.animations.insert(Animation{});
      }
      
      struct animation_channels
      {
            cgltf_animation_channel *translate;
            cgltf_animation_channel *rotate;
            cgltf_animation_channel *scale;
            cgltf_interpolation_type interpolationType;
      };
      
      cgltf_skin* skin = &data->skins[0];
      if(data->skins_count <= 0) return;      
      uint32_t skeleton_ID = 0;
      auto it = scene.skeleton_skinned_mesh_map.find(string(skin->name));
      if(it != scene.skeleton_skinned_mesh_map.end())
      {
            skeleton_ID = it->second;
      }
      else
      {
            LoadSkeleton(scene, skin);
      }
      
      const Skeleton* skeleton = &scene.skeletons[skeleton_ID];
      int bone_count = skeleton->bone_count;
      for(int anim_index = 0; anim_index < anim_count; anim_index++)
      {
            Animation& scene_animation = scene.animations[new_animation_IDs[anim_index]];
            scene_animation.skeleton_ID = skeleton_ID;
            cgltf_animation* anim_data = &data->animations[anim_index];

            float anim_duration = 0.0f;
            vector<animation_channels> bone_channels(bone_count);

            for(int channel_index = 0; channel_index < anim_data->channels_count; channel_index++)
            {
                  cgltf_animation_channel* channel = &anim_data->channels[channel_index];
                  int bone_index = -1;
                  for(int k = 0; k < bone_count; k++)
                  {
                        if(anim_data->channels[channel_index].target_node == skin->joints[k])
                        {
                              bone_index = k;
                              break;
                        }
                  }
                  if(bone_index == -1) continue; //because of chips ProperlyRiggedMarine being index -1??????

                  bone_channels[bone_index].interpolationType = anim_data->channels[channel_index].sampler->interpolation;

                  if(anim_data->channels[channel_index].sampler->interpolation != cgltf_interpolation_type_max_enum)
                  {
                        if(channel->target_path == cgltf_animation_path_type_translation)
                        {
                              bone_channels[bone_index].translate = &anim_data->channels[channel_index];
                        }
                        else if(channel->target_path == cgltf_animation_path_type_rotation)
                        {
                              bone_channels[bone_index].rotate = &anim_data->channels[channel_index];
                        }
                        else if(channel->target_path == cgltf_animation_path_type_scale)
                        {
                              bone_channels[bone_index].scale = &anim_data->channels[channel_index];
                        }
                  }

                  float t = 0.0f;
                  cgltf_bool r = cgltf_accessor_read_float(channel->sampler->input,
                                                           channel->sampler->input->count - 1,
                                                           &t,
                                                           1);

                  if(!r) continue;
                  anim_duration = (t > anim_duration) ? t : anim_duration;
            }
            if(anim_data->name != nullptr)
            {
                  scene_animation.name = string(anim_data->name);
            }

            int frame_count = (int)(anim_duration*1000.0f/ANIMDELAY);
            scene_animation.frame_count = frame_count;
            scene_animation.frame_poses.resize(frame_count);

            for(int j = 0; j < frame_count; j++)
            {
                  scene_animation.frame_poses[j].resize(bone_count);
                  float time = ((float) j * ANIMDELAY/1000.0f);
                  for(int k = 0; k < bone_count; k++)
                  {
                        glm::vec3 translation = glm::vec3(skin->joints[k]->translation[0],
                                                          skin->joints[k]->translation[1],
                                                          skin->joints[k]->translation[2]);
                        
                        glm::quat rotation = glm::quat(skin->joints[k]->rotation[0],
                                                       skin->joints[k]->rotation[1],
                                                       skin->joints[k]->rotation[2],
                                                       skin->joints[k]->rotation[3]);
                        
                        glm::vec3 scale = glm::vec3(skin->joints[k]->scale[0],
                                                    skin->joints[k]->scale[1],
                                                    skin->joints[k]->scale[2]);

                        if(bone_channels[k].translate)
                        {
                              get_pose_at_time(bone_channels[k].interpolationType,
                                               bone_channels[k].translate->sampler->input,
                                               bone_channels[k].translate->sampler->output,
                                               time,
                                               &translation);
                        }
                        if(bone_channels[k].rotate)
                        {
                              get_pose_at_time(bone_channels[k].interpolationType,
                                               bone_channels[k].rotate->sampler->input,
                                               bone_channels[k].rotate->sampler->output,
                                               time,
                                               &rotation);
                        }
                        if(bone_channels[k].scale)
                        {
                              get_pose_at_time(bone_channels[k].interpolationType,
                                               bone_channels[k].scale->sampler->input,
                                               bone_channels[k].scale->sampler->output,
                                               time,
                                               &scale);
                        }

                        scene_animation.frame_poses[j][k] = TRS{glm::vec4(translation, 1.0f),
                                                                glm::quat(rotation),
                                                                glm::vec4(scale, 1.0f)}; 
                  }
            }
      }
      cgltf_free(data);
}

uint32_t LoadSkeleton(Scene& scene, cgltf_skin* skin)
{
      Skeleton new_skeleton;
      LoadBoneInfo(skin, new_skeleton.bone_info);
      new_skeleton.bone_count = skin->joints_count;
      cgltf_accessor* bind_mats = skin->inverse_bind_matrices;
      new_skeleton.inv_bind_mats.resize(bind_mats->count);                  
      for(cgltf_size i = 0; i < bind_mats->count; i++)
      {
            float values[16];
            bool success = cgltf_accessor_read_float(bind_mats, i, values, 16);
            new_skeleton.inv_bind_mats[i] = glm::make_mat4(values);
      }
      new_skeleton.bind_pose.resize(skin->joints_count);
      for(int i = 0; i < skin->joints_count; i++)
      {
            cgltf_node* node = skin->joints[i];

            cgltf_float world_transform[16];
            cgltf_node_transform_world(node, world_transform);

            glm::mat4 world_mat = glm::make_mat4(world_transform);

            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;

            glm::decompose(world_mat, scale, rotation, translation, skew, perspective);

            new_skeleton.bind_pose[i].translation = glm::vec4(translation, 1.0f);
            new_skeleton.bind_pose[i].rotation = rotation;
            new_skeleton.bind_pose[i].scale = glm::vec4(scale, 1.0f);
      }

      glGenBuffers(1, &new_skeleton.inv_bind_pose_SSBO);
      glGenBuffers(1, &new_skeleton.anim_pose_SSBO);
      glGenBuffers(1, &new_skeleton.anim_trs_SSBO);
      glGenBuffers(1, &new_skeleton.bone_transform_SSBO);

      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.inv_bind_pose_SSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.inv_bind_pose_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * new_skeleton.bone_count;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, new_skeleton.inv_bind_mats.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      }
      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.anim_pose_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * new_skeleton.bone_count;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);    
            
      }

      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.anim_trs_SSBO);
            GLsizeiptr buffer_size = sizeof(TRS) * new_skeleton.bone_count;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);      
      }
      
      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.bone_transform_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * new_skeleton.bone_count;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);      
      }
      
      
      uint32_t new_skeleton_ID = scene.skeletons.insert(new_skeleton);
      scene.skeleton_skinned_mesh_map.emplace(string(skin->name), new_skeleton_ID);

      return new_skeleton_ID;
}
void LoadBoneInfo(cgltf_skin* skin, vector<BoneInfo>& bones)
{
      int bone_count = skin->joints_count;
      bones.resize(bone_count);

      for(int i = 0; i < bone_count; i++)
      {
            cgltf_node* bone_node = skin->joints[i];
            if(bone_node->name != nullptr)
            {
                  bones[i].name = string(bone_node->name);
            }
            bones[i].index = i;
            int parent_index = -1;
            for(int j = 0; j < bone_count; j++)
            {
                  if(skin->joints[j] == bone_node->parent)
                  {
                        parent_index = j;
                        break;
                  }
            }
            bones[i].parent = parent_index;
      }
}

void AddMeshInstance(Scene &scene, uint32_t mesh_ID, uint32_t *new_instance_ID)
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
void AddSkinnedMeshInstance(Scene &scene, uint32_t skinned_mesh_ID, uint32_t *new_instance_ID)
{
      Transform new_transform;
      new_transform.scale = glm::vec3(1.0f);

      uint32_t new_transform_ID = scene.transforms.insert(new_transform);

      Instance new_instance;
      new_instance.skinned_mesh_ID = skinned_mesh_ID;
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


      
GLuint create_texture(char const* Filename)
{
	gli::texture Texture = gli::load(Filename);
	if(Texture.empty())
		return 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());

	GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

	glm::tvec3<GLsizei> const Extent(Texture.extent());
	GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

	switch(Texture.target())
	{
	case gli::TARGET_1D:
		glTexStorage1D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
		break;
	case gli::TARGET_1D_ARRAY:
	case gli::TARGET_2D:
	case gli::TARGET_CUBE:
		glTexStorage2D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
		break;
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_3D:
	case gli::TARGET_CUBE_ARRAY:
		glTexStorage3D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Extent.y,
			Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
		break;
	default:
		assert(0);
		break;
	}

	for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
	for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		GLsizei const LayerGL = static_cast<GLsizei>(Layer);
		glm::tvec3<GLsizei> Extent(Texture.extent(Level));
		Target = gli::is_target_cube(Texture.target())
			? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
			: Target;

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		default: assert(0); break;
		}
	}
     
	return TextureName;
}
