#include "game_mode.hpp"
#include "scene.hpp"
#include "renderer.hpp"

#include <vector>

using namespace std;

class GameMode : public IGameMode
{
public:
      Scene* m_scene;

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene, IRenderer* renderer) override
      {
            m_scene = scene;

            m_first_update = true;

            vector<uint32_t> loaded_mesh_IDs;

            loaded_mesh_IDs.clear();
            LoadMeshes(*m_scene, "../resources/chips_2/chips_2.gltf", &loaded_mesh_IDs);
            for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            {
                  uint32_t new_instance_ID;
                  AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                  uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
                  scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            }

            loaded_mesh_IDs.clear();
            LoadMeshes(*m_scene, "../resources/level/movement_level.gltf", &loaded_mesh_IDs);
            for(uint32_t loaded_mesh_ID : loaded_mesh_IDs)
            {
                  uint32_t new_instance_ID;
                  AddInstance(*scene, loaded_mesh_ID, &new_instance_ID);
                  uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                  scene->transforms[new_instance_ID].translation = glm::vec3(0,0,0);
            }

            
            Camera main_camera(glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0,1,0), glm::vec3(1,0,0), 0.0f, 0.0f);
            m_scene->main_camera_ID = m_scene->cameras.insert((main_camera));
            // Camera mainCamera;
            // mainCamera.Eye = glm::vec3(3.0f);
            // mainCamera.Target = glm::vec3(0.0f);
            // glm::vec3 across = cross(mainCamera.Target - mainCamera.Eye, glm::vec3(0.0f, 1.0f, 0.0f));
            // mainCamera.Up = normalize(cross(across, mainCamera.Target - mainCamera.Eye));
            // mainCamera.FovY = glm::radians(70.0f);
            // m_scene->main_camera_ID = m_scene->cameras.insert(mainCamera);
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
