#include "game_mode.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "GLFW/glfw3.h"
#include "input.hpp"
#include "job_system.h"

#include <iostream>

using namespace std;

class GameMode : public IGameMode
{
public:

      std::vector<Entity> entities;      
      Scene* m_scene;
      IRenderer* m_renderer;
      GLFWwindow* m_window;


      uint32_t current_skybox;

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene, GLFWwindow* window, IRenderer* renderer) override
      {
            m_scene = scene;

            m_renderer = renderer;

            m_window = window;

            m_first_update = true;


            MeshData chips_mesh_result;
            JobSystem::Execute([&]{LoadMeshAsync(*m_scene, chips_mesh_result, "d:/OpenGLPractice/SceneRenderer/resources/chips_2/chips_2.gltf");});
            
            MeshData delta_mesh_result;
            JobSystem::Execute([&]{LoadMeshAsync(*m_scene, delta_mesh_result, "../resources/delta/delta.gltf");});
            
            JobSystem::Wait();
            
            uint32_t chips_ID = UploadMesh(*m_scene, chips_mesh_result);
            {
                  uint32_t new_instance_ID;
                  AddMeshInstance(*scene, chips_ID, &new_instance_ID);
                  uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  scene->transforms[new_instance_ID].translation = glm::vec3(0,1,0);
                  scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                  entities.emplace_back(Entity(new_instance_ID));
            }

            uint32_t delta_ID = UploadMesh(*m_scene, delta_mesh_result);
            {
                  uint32_t new_instance_ID;
                  AddMeshInstance(*scene, delta_ID, &new_instance_ID);
                  uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
                  scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                  entities.emplace_back(Entity(new_instance_ID));
            }

            // vector<uint32_t> loaded_mesh_IDs;
            // LoadMeshes(*m_scene, "../resources/delta/delta.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
            //       uint32_t new_instance_ID;
            //       AddMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //       uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //       scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
            //       scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            // }

            // LoadMeshes(*m_scene, "../resources/delta/delta.gltf", &loaded_mesh_IDs);

            // loaded_mesh_IDs.clear();
            // LoadMeshes(*m_scene, "../resources/chips_2/chips_2.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
            //       {
            //             uint32_t new_instance_ID;
            //             AddMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(1,-0.1,0);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            //             entities.emplace_back(Entity(new_instance_ID));
            //       }
                  // {
                  //       uint32_t new_instance_ID;
                  //       AddMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                  //       uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  //       scene->transforms[new_instance_ID].translation = glm::vec3(2,-0.1,0);
                  //       scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                  //       entities.emplace_back(Entity(new_instance_ID));
                  // }
                  
            //}
      
            // loaded_mesh_IDs.clear();
            // LoadSkinnedMeshes(*m_scene, "../resources/chips_2/chips_2.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
            //       {
            //             uint32_t new_instance_ID;
            //             AddSkinnedMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(0,-0.1,0);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            //             entities.emplace_back(Entity(new_instance_ID));
            //       }

            //       for(int i = 0; i < 10; i++)
            //       {
            //             uint32_t new_instance_ID;
            //             AddSkinnedMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(i,-0.1,0);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            //             entities.emplace_back(Entity(new_instance_ID));      
            //       }
            // }

            

            // loaded_mesh_IDs.clear();
            // LoadMeshes(*m_scene, "../resources/delta/delta.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
            //       uint32_t new_instance_ID;
            //       AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //       uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //       scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
            //       scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            // }

            // loaded_mesh_IDs.clear();
            // LoadMeshes(*m_scene, "../resources/level/level_big.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
            //       uint32_t new_instance_ID;
            //       AddMeshInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //       uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //       scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
            //       scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            // }


            // loaded_mesh_IDs.clear();
            // LoadMeshes(*m_scene, "../resources/follige/fern.gltf", &loaded_mesh_IDs);
            // for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            // {
                  
            //       {
            //             uint32_t new_instance_ID;
            //             AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(0,0.2,-0.2);
            //       }
            //       {
            //             uint32_t new_instance_ID;
            //             AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(0,0.2,1.2);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0));
            //       }
            // }

            // vector<uint32_t> animation_IDs;
            // animation_IDs.clear();
            // LoadAnimation(*m_scene, "../resources/chips_2/chips_2.gltf", &animation_IDs);

            

            AddSkybox(*m_scene, &current_skybox);

            
            Camera main_camera(glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0,1,0), glm::vec3(1,0,0), 0.0f, 0.0f, CamType::flying);
            m_scene->main_camera_ID = m_scene->cameras.insert((main_camera));
      }

      void HandleEvent() override
      {

            
      }

      void Update(float dt) override
      {
            GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(m_window));
            PlayerInput& player_input = gd->player_input;
            KeyInputs& key_inputs = player_input.key_inputs;
            MouseData& mouse_data = player_input.mouse_data;

            if(!gd->cursor_enabled)
            {
                  Camera& main_camera = m_scene->cameras[m_scene->main_camera_ID];
                  main_camera.UpdateFromInput(player_input, dt);
                  
                  if(key_inputs.space.pressed)
                  {
                        RemoveInstance(*m_scene, entities.back().instance_ID);      
                  }
            }
            
            if(key_inputs.tab.pressed)
            {
                  m_renderer->DrawDebugQuad();
            }
            if(key_inputs.shift.pressed)
            {
                  m_renderer->toggle_skinned();
            }
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
