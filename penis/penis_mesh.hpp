#pragma once

#include <common.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <span>
#include <string>
#include <map>
#include <unordered_map>

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>

struct DrawCommand
{
      std::string material_name;
      int baseVertex;
      unsigned int count;
      unsigned int primCount;
      unsigned int firstIndex;
      unsigned int baseInstance;
      bool has_transparecny = false;

      template<class Archive>
      void serialize(Archive& ar)
      {
            ar(material_name,
               baseVertex,
               count,
               primCount,
               firstIndex,
               baseInstance,
               has_transparecny
               );
      };
};

struct MaterialLoadData
{
      std::string diffuse_path = "";
      float scale[2] = {1.0f, 1.0f};
      float offset[2] = {0.0f, 0.0f};
      bool has_diffuse_map = false;
      template<class Archive>
      void serialize(Archive& ar)
      {
            ar(diffuse_path,
               scale,
               offset,
               has_diffuse_map);
      }
};

struct MeshData
{
      std::unordered_map<std::string, MaterialLoadData> material_load_map;
      std::vector<DrawCommand> draw_commands;
      std::vector<float> positions;
      std::vector<float> tex_coords;
      std::vector<float> normals;
      std::vector<float> bone_weights;
      std::vector<int>   bone_IDs;
      std::vector<uint32_t> indices;
      std::vector<uint32_t> material_IDs;

      
      unsigned int index_count;
      unsigned int vertex_count;

      bool skinned = false;
      std::string skeleton_path = "";


      template<class Archive>
      void serialize(Archive& ar)      
      {
            ar(material_load_map,
               draw_commands,
               positions,
               tex_coords,
               normals,
               bone_weights,
               bone_IDs,
               indices,
               material_IDs,
               skinned,
               skeleton_path);
      };
};

struct BoneInfo
{
      int parent;
      int index;
      std::string name;

      template<class Archive>
      void serialize(Archive& ar)
      {
            ar(parent,
               index,
               name);
      };
};

struct SkeletonData
{
      std::vector<BoneInfo> bone_info;
      std::vector<TRS> bind_pose;
      std::vector<glm::mat4> inv_bind_mats;
      std::string name;
      int bone_count;
      template<class Archive>
      void serialize(Archive& ar)      
      {
            ar(bone_info,
               bind_pose,
               inv_bind_mats,
               name,
               bone_count);
      };
      
};

struct DiffuseMap
{
      unsigned int DiffuseMapTO;
      bool has_transparency = false;
};

struct Material
{
      std::string name;

      glm::vec2 scale = glm::vec2(1.0f);
      glm::vec2 offset = glm::vec2(0.0f);
      

      unsigned int albedo;
      unsigned int normal;
      unsigned int ORM;
      
      float base_color[4];

      bool has_diffuse_map = false;
      
      uint32_t diffuse_map_ID = 0;
      uint32_t normal_map_ID = 0;
      uint32_t orm_map_ID = 0;
};

struct Mesh
{
      std::vector<DrawCommand> draw_commands;
      std::vector<uint32_t> material_IDs;
      std::string Name;

      unsigned int mesh_VAO;
      unsigned int postion_BO;
      unsigned int tex_coord_BO;
      unsigned int normal_BO;
      unsigned int index_BO;

      unsigned int index_count;
      unsigned int vertex_count;
};



struct Animation
{
      std::string name;
      std::vector<std::vector<TRS>> frame_poses;
      uint32_t skeleton_ID = 0;
      int frame_count;
};

struct Skeleton
{
      std::vector<BoneInfo> bone_info;
      std::vector<TRS> bind_pose;
      std::vector<glm::mat4> inv_bind_mats;
      std::string name;
      unsigned int bone_transform_SSBO;
      unsigned int inv_bind_pose_SSBO;
      unsigned int anim_pose_SSBO;
      unsigned int anim_trs_SSBO;
      int bone_count;      
};

struct SkinnedMesh
{
      std::vector<DrawCommand> draw_commands;
      std::vector<uint32_t> material_IDs;
      std::string Name;

      uint32_t skeleton_ID = 0;

      unsigned int mesh_VAO;
      unsigned int postion_BO;
      unsigned int tex_coord_BO;
      unsigned int normal_BO;
      unsigned int bone_IDs;
      unsigned int bone_weights;
      unsigned int index_BO;

      unsigned int index_count;
      unsigned int vertex_count;      
};
