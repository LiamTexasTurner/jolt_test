#define pragma

class Scene;
class IRenderer;

class IGameMode
{
public:
      virtual void Init(Scene* scene, IRenderer* renderer) = 0;
      virtual void HandleEvent() = 0;
      // virtual void Update() = 0;
};

IGameMode* NewGameMode();
