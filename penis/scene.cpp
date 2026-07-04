#include "scene.hpp"
#include "preamble.glsl"

#include "stb_image.h"
#include "cgltf.h"

#include <iostream>

using namespace std;

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
      for(int i = 0; i < data->materials_count; i++)
      {
            cgltf_material* mat = &data->materials[i];
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
