#include "game_mode.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "GLFW/glfw3.h"
#include "input.hpp"

#include <iostream>

using namespace std;

class GameMode : public IGameMode
{
public:

      std::vector<Entity> entities;      
      Scene* m_scene;
      IRenderer* m_renderer;
      GLFWwindow* m_window;
      

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene, GLFWwindow* window, IRenderer* renderer) override
      {
            m_scene = scene;

            m_renderer = renderer;

            m_window = window;

            m_first_update = true;

            vector<uint32_t> loaded_mesh_IDs;

            loaded_mesh_IDs.clear();
            LoadMeshes(*m_scene, "../resources/chips_2/chips_2.gltf", &loaded_mesh_IDs);
            for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            {
                  {
                        uint32_t new_instance_ID;
                        AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                        uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                        scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
                        scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                        entities.emplace_back(Entity(new_instance_ID));
                  }
                  {
                        uint32_t new_instance_ID;
                        AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                        uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                        scene->transforms[new_instance_ID].translation = glm::vec3(2,0,0);
                        scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                        entities.emplace_back(Entity(new_instance_ID));
                  }
                  
            }

            loaded_mesh_IDs.clear();
            LoadMeshes(*m_scene, "../resources/level/level_big.gltf", &loaded_mesh_IDs);
            for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            {
                  uint32_t new_instance_ID;
                  AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                  uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
            }

            
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

            Camera& main_camera = m_scene->cameras[m_scene->main_camera_ID];
            main_camera.UpdateFromInput(player_input, dt);
                  
            if(key_inputs.space.pressed)
            {
                  RemoveInstance(*m_scene, entities.back().instance_ID);      
            }
            if(key_inputs.tab.pressed)
            {
                  m_renderer->DrawDebugQuad();
            }
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
