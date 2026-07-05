#pragma once

#include <glad/glad.h>
#include "opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "packed_freelist.h"

#include <vector>
#include <string>
#include <map>

struct DiffuseMap
{
      GLuint DiffuseMapTO;
};

struct Material
{
      std::string name;

      GLuint albedo;
      GLuint normal;
      GLuint ORM;
      
      float base_color[4];

      
      uint32_t diffuse_map_ID;
      uint32_t normal_map_ID;
      uint32_t orm_map_ID;
};

struct Mesh
{
      std::string Name;

      GLuint mesh_VAO;
      GLuint postion_BO;
      GLuint tex_coord_BO;
      GLuint index_BO;

      GLuint index_count;
      GLuint vertex_count;

      std::vector<GLDrawElementsIndirectCommand> draw_commands;
      std::vector<uint32_t> material_IDs;
};

struct Transform
{
      glm::vec3 scale = glm::vec3(1.0f);
      glm::vec3 rotation_origin = glm::vec3(0.0f);
      glm::quat rotation = glm::identity<glm::quat>();
      glm::vec3 translation = glm::vec3(0.0f);
};

struct Instance
{
      uint32_t mesh_ID;
      uint32_t transform_ID;
};

struct Camera
{
      glm::vec3 eye;
      glm::vec3 target;
      glm::vec3 up;

      float fov_y;
      float aspect;
      float near_z;
};
class Scene
{
public:
      packed_freelist<DiffuseMap> diffuse_maps;
      packed_freelist<Material> materials;
      packed_freelist<Mesh> meshes;
      packed_freelist<Transform> transforms;
      packed_freelist<Instance> instances;
      packed_freelist<Camera> cameras;

      uint32_t main_camera_ID;

      void Init();
};

void LoadMeshes(Scene& scene,
                const std::string& filename,
                std::vector<uint32_t>* load_mesh_IDs);

void AddInstance(Scene& scene,
                 uint32_t mesh_ID,
                 uint32_t* new_instance_ID);

unsigned int texture_from_file(std::string uri, const std::string &directory, bool gamma);


