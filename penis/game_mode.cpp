#include "game_mode.hpp"
#include "scene.hpp"

#include <vector>

using namespace std;

class GameMode : public IGameMode
{
public:
      Scene* m_scene;

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene) override
      {
            m_scene = scene;

            m_first_update = true;

            vector<uint32_t> loaded_mesh_IDs;

            loaded_mesh_IDs.clear();
            LoadMeshes(*m_scene, "../resources/chips_2/chips_2.gltf", &loaded_mesh_IDs);
            
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
