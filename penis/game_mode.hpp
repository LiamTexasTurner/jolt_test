#define pragma
#include <vector>

class Scene;
class IRenderer;
class Entity;


class IGameMode
{
public:
      virtual void Init(Scene* scene, IRenderer* renderer) = 0;
      virtual void HandleEvent() = 0;
      // virtual void Update() = 0;

      Scene* m_scene;
      std::vector<Entity> entities;      
};

IGameMode* NewGameMode();
