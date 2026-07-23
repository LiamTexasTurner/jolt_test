#include "scene.hpp"
#include "preamble.glsl"

#include "stb_image.h"
#include "cgltf.h"

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
      diffuse_maps     = packed_freelist<DiffuseMap>(512);
      materials        = packed_freelist<Material>(512);
      meshes           = packed_freelist<Mesh>(512);
      skeletons        = packed_freelist<pSkeleton>(512);
      skinned_meshes   = packed_freelist<SkinnedMesh>(512);
      animations       = packed_freelist<Animation>(512);
      animation_graphs = packed_freelist<AnimationGraph>(512);
      transforms       = packed_freelist<Transform>(4096);
      instances        = packed_freelist<Instance>(4096);
      cameras          = packed_freelist<Camera>(32);
      skyboxes         = packed_freelist<Skybox>(32);
      ragdolls         = packed_freelist<JPH::Ragdoll*>(512);

      //defualts

      // Material default_mat;
      // default_mat.base_color[0] = 0.0f;
      // default_mat.base_color[1] = 1.0f;
      // default_mat.base_color[2] = 1.0f;
      // default_mat.base_color[3] = 1.0f;
      // default_mat.diffuse_map_ID = -1;
      // materials.insert(default_mat);
      
}

uint32_t LoadMesh(Scene& scene, MeshData& mesh_data)
{
      if(!mesh_data.skinned)
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
                        new_material.has_diffuse_map = data.has_diffuse_map;
                        new_material.diffuse_map_ID = new_diffuse_map_ID;

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
      else
      {
            SkinnedMesh mesh_result;
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
                        new_material.has_diffuse_map = data.has_diffuse_map;
                        new_material.diffuse_map_ID = new_diffuse_map_ID;

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


            string skeleton_name = filesystem::path(mesh_data.skeleton_path).stem().string();
            cout << skeleton_name << endl;
            auto [it, inserted] = scene.skeleton_skinned_mesh_map.try_emplace(filesystem::path(mesh_data.skeleton_path).stem().string());
            if(inserted)
            {
                  SkeletonData skeleton_data;
                  std::ifstream is(mesh_data.skeleton_path, ios::binary);
                  cereal::BinaryInputArchive i_archive(is);
                  i_archive(skeleton_data);

                  mesh_result.skeleton_ID = LoadSkeleton(scene, skeleton_data);
            }
            else
            {
                  mesh_result.skeleton_ID = it->second;
            }
      
            glGenVertexArrays(1, &mesh_result.mesh_VAO);
            glGenBuffers(1, &mesh_result.postion_BO);
            glGenBuffers(1, &mesh_result.tex_coord_BO);
            glGenBuffers(1, &mesh_result.normal_BO);
            glGenBuffers(1, &mesh_result.bone_IDs);
            glGenBuffers(1, &mesh_result.bone_weights);
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

            glBindBuffer(GL_ARRAY_BUFFER, mesh_result.bone_IDs);
            glBufferData(GL_ARRAY_BUFFER, mesh_data.bone_IDs.size() * sizeof(mesh_data.bone_IDs[0]), mesh_data.bone_IDs.data(), GL_STATIC_DRAW);
            glVertexAttribIPointer(SCENE_BONE_ID_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_INT, sizeof(int) * MAX_BONE_INFLUENCE, nullptr);
            glEnableVertexAttribArray(SCENE_BONE_ID_ATTRIB_LOCATION);
            
            
            glBindBuffer(GL_ARRAY_BUFFER, mesh_result.bone_weights);
            glBufferData(GL_ARRAY_BUFFER, mesh_data.bone_weights.size() * sizeof(mesh_data.bone_weights[0]), mesh_data.bone_weights.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(float) * MAX_BONE_INFLUENCE, nullptr);
            glEnableVertexAttribArray(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_result.index_BO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data.indices.size() * sizeof(uint32_t), mesh_data.indices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            
            
            
            uint32_t new_mesh_ID = scene.skinned_meshes.insert(mesh_result);

            return new_mesh_ID;
      }
}
uint32_t LoadSkeletalMesh(Scene& scene, MeshData& mesh_data)
{
      SkinnedMesh mesh_result;
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
                  new_material.has_diffuse_map = data.has_diffuse_map;
                  new_material.diffuse_map_ID = new_diffuse_map_ID;

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


      string skeleton_name = filesystem::path(mesh_data.skeleton_path).stem().string();
      cout << skeleton_name << endl;
      auto [it, inserted] = scene.skeleton_skinned_mesh_map.try_emplace(filesystem::path(mesh_data.skeleton_path).stem().string());
      if(inserted)
      {
            // SkeletonData skeleton_data;
            // std::ifstream is(mesh_data.skeleton_path, ios::binary);
            // cereal::BinaryInputArchive i_archive(is);
            // i_archive(skeleton_data);

            // mesh_result.skeleton_ID = LoadSkeleton(scene, skeleton_data);
            assert(false && "no skeleton found");
      }
      else
      {
            mesh_result.skeleton_ID = it->second;
      }
      
      glGenVertexArrays(1, &mesh_result.mesh_VAO);
      glGenBuffers(1, &mesh_result.postion_BO);
      glGenBuffers(1, &mesh_result.tex_coord_BO);
      glGenBuffers(1, &mesh_result.normal_BO);
      glGenBuffers(1, &mesh_result.bone_IDs);
      glGenBuffers(1, &mesh_result.bone_weights);
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

      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.bone_IDs);
      glBufferData(GL_ARRAY_BUFFER, mesh_data.bone_IDs.size() * sizeof(mesh_data.bone_IDs[0]), mesh_data.bone_IDs.data(), GL_STATIC_DRAW);
      glVertexAttribIPointer(SCENE_BONE_ID_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_INT, sizeof(int) * MAX_BONE_INFLUENCE, nullptr);
      glEnableVertexAttribArray(SCENE_BONE_ID_ATTRIB_LOCATION);
      
      
      glBindBuffer(GL_ARRAY_BUFFER, mesh_result.bone_weights);
      glBufferData(GL_ARRAY_BUFFER, mesh_data.bone_weights.size() * sizeof(mesh_data.bone_weights[0]), mesh_data.bone_weights.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(float) * MAX_BONE_INFLUENCE, nullptr);
      glEnableVertexAttribArray(SCENE_BONE_WEIGHTS_ATTRIB_LOCATION);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_result.index_BO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data.indices.size() * sizeof(uint32_t), mesh_data.indices.data(), GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      
      
      
      uint32_t new_mesh_ID = scene.skinned_meshes.insert(mesh_result);

      return new_mesh_ID;
}
      
      
uint32_t LoadSkeleton(Scene& scene, SkeletonData& skeleton_data)
{
      pSkeleton new_skeleton;

      new_skeleton.name = skeleton_data.name;
      new_skeleton.bone_count = skeleton_data.bone_count;
      new_skeleton.bone_info = skeleton_data.bone_info;
      new_skeleton.inv_bind_mats = skeleton_data.inv_bind_mats;
      new_skeleton.bind_pose = skeleton_data.bind_pose;

      for(BoneInfo& bone_info : new_skeleton.bone_info)
      {
            new_skeleton.bone_name_index_map.emplace(bone_info.name, bone_info.index);
      }
      

      glGenBuffers(1, &new_skeleton.inv_bind_pose_SSBO);
      

      {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.inv_bind_pose_SSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, new_skeleton.inv_bind_pose_SSBO);
            GLsizeiptr buffer_size = sizeof(glm::mat4) * new_skeleton.bone_count;
            glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, new_skeleton.inv_bind_mats.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      }
            
      
      uint32_t new_skeleton_ID = scene.skeletons.insert(new_skeleton);
      scene.skeleton_skinned_mesh_map.emplace(new_skeleton.name, new_skeleton_ID);

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
void AddSkinnedMeshInstance(Scene &scene, uint32_t skinned_mesh_ID, uint32_t *new_instance_ID, AnimationGraph anim_graph)
{
      Transform new_transform;
      new_transform.scale = glm::vec3(1.0f);

      uint32_t new_transform_ID = scene.transforms.insert(new_transform);

      Instance new_instance;
      new_instance.skinned_mesh_ID = skinned_mesh_ID;
      new_instance.transform_ID = new_transform_ID;
      new_instance.anim_graph_ID = scene.animation_graphs.insert(anim_graph);
      
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


uint32_t LoadAnimation(Scene& scene, const std::string& path)
{
      AnimationData animation_data;
      ifstream is(path, ios::binary);
      cereal::BinaryInputArchive archive(is);
      archive(animation_data);

      Animation animation;
      animation.frame_count = animation_data.frame_count;
      animation.name = animation_data.name;
      animation.frame_poses = animation_data.frame_poses;

      auto it = scene.skeleton_skinned_mesh_map.find(animation_data.skeleton_name);
      if(it != scene.skeleton_skinned_mesh_map.end())
      {
            animation.skeleton_ID = it->second;
      }
      else
      {
            assert(false && "tried to add animations to skeleton that dosent exist");
      }

      return scene.animations.insert(animation);
}

std::vector<uint32_t> LoadAnimations(Scene& scene, const std::string& path)
{
      std::vector<uint32_t> animation_IDs;
      for(const filesystem::directory_entry& entry : filesystem::directory_iterator(path))
      {
            animation_IDs.push_back(LoadAnimation(scene, entry.path().string()));
      }

      return animation_IDs;
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
