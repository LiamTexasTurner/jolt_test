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

#include "penis_mesh.hpp"
#include "animation_graph.hpp"


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
      packed_freelist<Transform> transforms;
      packed_freelist<Instance> instances;
      
      packed_freelist<DiffuseMap> diffuse_maps;
      packed_freelist<Material> materials;
      
      packed_freelist<Mesh> meshes;
      
      packed_freelist<SkinnedMesh> skinned_meshes;
      packed_freelist<Skeleton> skeletons;
      packed_freelist<Animation> animations;      
      packed_freelist<AnimationGraph> animation_graphs;

      packed_freelist<Camera> cameras;
      packed_freelist<Skybox> skyboxes;

      uint32_t main_camera_ID = 0;;
      std::map<std::string, uint32_t> skeleton_skinned_mesh_map;

      void Init();
      void InitSkinnedMeshBuffers();
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

uint32_t LoadMesh(Scene& scene, MeshData& mesh_data);

uint32_t LoadSkeletalMesh(Scene& scene, MeshData& mesh_data, AnimationGraph anim_graph);

uint32_t LoadSkeleton(Scene& scene, SkeletonData& skeleton_data);

uint32_t LoadAnimation(Scene& scene, const std::string& path);

std::vector<uint32_t> LoadAnimations(Scene& scene, const std::string& path);

void AddSkybox(Scene& scene, uint32_t* new_skybox_ID);

void AddMeshInstance(Scene& scene, uint32_t mesh_ID, uint32_t* new_instance_ID);

void AddSkinnedMeshInstance(Scene &scene, uint32_t skinned_mesh_ID, uint32_t *new_instance_ID);

void RemoveInstance(Scene& scene, uint32_t instance_ID);

unsigned int texture_from_file(std::string uri, const std::string &directory, bool gamma);

GLuint create_texture(char const* Filename);


