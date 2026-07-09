#pragma once

#include <glad/glad.h>
#include "opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "packed_freelist.h"
#include "camera.hpp"
#include "skybox.hpp"

#include <vector>
#include <span>
#include <string>
#include <map>

struct DiffuseMap
{
      GLuint DiffuseMapTO = -1;
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
      
      uint32_t diffuse_map_ID;
      uint32_t normal_map_ID;
      uint32_t orm_map_ID;
};

struct DrawCommand
{
      GLDrawElementsIndirectCommand gl_draw_ele_cmd;
      bool has_transparecny;
};

struct Mesh
{
      std::string Name;

      GLuint mesh_VAO;
      GLuint postion_BO;
      GLuint tex_coord_BO;
      GLuint normal_BO;
      GLuint index_BO;

      GLuint index_count;
      GLuint vertex_count;

      std::vector<DrawCommand> draw_commands;
      std::vector<uint32_t> material_IDs;
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
      uint32_t mesh_ID;
      uint32_t transform_ID;
};

struct Opaque
{
      uint32_t instance_id;
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
      packed_freelist<Skybox> skyboxes;

      uint32_t main_camera_ID;

      void Init();
};

void LoadMeshes(Scene& scene,
                const std::string& filename,
                std::vector<uint32_t>* load_mesh_IDs);

void AddSkybox(Scene& scene, uint32_t* new_skybox_ID);

void AddInstance(Scene& scene,
                 uint32_t mesh_ID,
                 uint32_t* new_instance_ID);

void RemoveInstance(Scene& scene, uint32_t instance_ID);

unsigned int texture_from_file(std::string uri, const std::string &directory, bool gamma);


