#define pragma


class Scene;
class pJolt;
class IRenderer;
class GLFWwindow;




class IGameMode
{
public:
      virtual void Init(Scene* scene, pJolt* p_jolt, GLFWwindow* window, IRenderer* renderer) = 0;
      virtual void HandleEvent() = 0;
      virtual void Update(float dt) = 0;
      virtual void PhysicsUpdate(float dt) = 0;

      
};

IGameMode* NewGameMode();
