#pragma once


#include <glad/glad.h>
#include "opengl.h"
#include <common.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "packed_freelist.h"
#include "camera.hpp"
#include "skybox.hpp"
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>

#include <vector>
#include <span>
#include <string>
#include <map>
#include <unordered_map>

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
               has_transparecny);
      };
};

struct DiffuseMap
{
      GLuint DiffuseMapTO;
      bool has_transparency = false;
};

struct Material
{
      std::string name;

      glm::vec2 scale = glm::vec2(1.0f);
      glm::vec2 offset = glm::vec2(0.0f);
      

      GLuint albedo;
      GLuint normal;
      GLuint ORM;
      
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

      GLuint mesh_VAO;
      GLuint postion_BO;
      GLuint tex_coord_BO;
      GLuint normal_BO;
      GLuint index_BO;

      GLuint index_count;
      GLuint vertex_count;
};

struct BoneInfo
{
      int parent;
      int index;
      std::string name;
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
      GLuint bone_transform_SSBO;
      GLuint inv_bind_pose_SSBO;
      GLuint anim_pose_SSBO;
      GLuint anim_trs_SSBO;
      int bone_count;
};

struct SkinnedMesh
{
      std::vector<DrawCommand> draw_commands;
      std::vector<uint32_t> material_IDs;
      std::string Name;

      uint32_t skeleton_ID = 0;

      GLuint mesh_VAO;
      GLuint postion_BO;
      GLuint tex_coord_BO;
      GLuint normal_BO;
      GLuint bone_IDs;
      GLuint bone_weights;
      GLuint index_BO;

      GLuint index_count;
      GLuint vertex_count;      
};

struct Transform
{
      glm::vec3 scale;
      glm::vec3 rotation_origin;
      glm::quat rotation;
      glm::vec3 translation;
};

struct Instance
{
      uint32_t transform_ID = 0;
      uint32_t mesh_ID = 0;
      uint32_t skinned_mesh_ID = 0;
};

struct Opaque
{
      uint32_t instance_id = 0;
};

class Scene
{
public:
      packed_freelist<DiffuseMap> diffuse_maps;
      packed_freelist<Material> materials;
      packed_freelist<Mesh> meshes;
      packed_freelist<SkinnedMesh> skinned_meshes;
      packed_freelist<Animation> animations;
      packed_freelist<Transform> transforms;
      packed_freelist<Instance> instances;
      packed_freelist<Camera> cameras;
      packed_freelist<Skeleton> skeletons;
      packed_freelist<Skybox> skyboxes;

      uint32_t main_camera_ID = 0;;
      std::map<std::string, uint32_t> skeleton_skinned_mesh_map;

      void Init();
};

struct material_file_info
{
      std::string material_name;
      std::string file_path;
      template<class Archive>
      void serialize(Archive& ar)
      {
            ar(material_name,
               file_path);
      };
};

struct MaterialLoadData
{
      std::string diffuse_path = "";
      float scale[2] = {1.0f, 1.0f};
      float offset[2] = {0.0f, 0.0f};
      template<class Archive>
      void serialize(Archive& ar)
      {
            ar(diffuse_path,
               scale,
               offset);
      }
};

struct MeshData
{
      std::unordered_map<std::string, MaterialLoadData> material_load_map;
      std::vector<DrawCommand> draw_commands;
      std::vector<material_file_info> material_file_info;
      std::vector<float> positions;
      std::vector<float> tex_coords;
      std::vector<float> normals;
      std::vector<uint32_t> indices;
      std::vector<uint32_t> material_IDs;
      
      unsigned int index_count;
      unsigned int vertex_count;

      template<class Archive>
      void serialize(Archive& ar)      
      {
            ar(material_load_map,
               draw_commands,
               material_file_info,
               positions,
               tex_coords,
               normals,
               indices,
               material_IDs);
      };
};
      
void LoadMeshAsync(Scene& scene, MeshData& mesh_result, const std::string& filename);

uint32_t UploadMesh(Scene& scene, MeshData& mesh_data);

void LoadMeshes(Scene& scene, const std::string& filename, std::vector<uint32_t>* load_mesh_IDs);

void LoadSkinnedMeshes(Scene& scene, const std::string& filename, std::vector<uint32_t>* load_mesh_IDs);

void LoadAnimation(Scene& scene, const std::string& filename, std::vector<uint32_t>* animation_IDs);

void AddSkybox(Scene& scene, uint32_t* new_skybox_ID);

void AddMeshInstance(Scene& scene, uint32_t mesh_ID, uint32_t* new_instance_ID);

void AddSkinnedMeshInstance(Scene &scene, uint32_t skinned_mesh_ID, uint32_t *new_instance_ID);

void RemoveInstance(Scene& scene, uint32_t instance_ID);

unsigned int texture_from_file(std::string uri, const std::string &directory, bool gamma);

GLuint create_texture(char const* Filename);


