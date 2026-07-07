#define pragma


class Scene;
class IRenderer;
class GLFWwindow;




class IGameMode
{
public:
      virtual void Init(Scene* scene, GLFWwindow* window, IRenderer* renderer) = 0;
      virtual void HandleEvent() = 0;
      virtual void Update() = 0;

      
};

IGameMode* NewGameMode();
