#define pragma

class Scene;

class IGameMode
{
public:
      virtual void Init(Scene* scene) = 0;
      // virtual void HandleEvent() = 0;
      // virtual void Update() = 0;
};

IGameMode* NewGameMode();
