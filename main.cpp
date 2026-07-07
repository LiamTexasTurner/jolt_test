#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "penis/scene.hpp"
#include "penis/renderer.hpp"
#include "penis/game_mode.hpp"

#include <iostream>

bool full_screen = false;

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void process_input(GLFWwindow *window);

bool first_key_press = false;

int main()
{

      glfwInit();
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);     

      GLFWmonitor* monitor = nullptr;
      const GLFWvidmode* mode = nullptr;
      GLFWwindow* window = nullptr;

      //set full screen
      if(full_screen) 
      {
            monitor = glfwGetPrimaryMonitor();
            mode = glfwGetVideoMode(monitor);
            SCR_WIDTH = mode->width;
            SCR_HEIGHT = mode->height;
            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", monitor, NULL);
      }
      else
      {
            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", monitor, NULL);
      }
            
      
      if (window == NULL)
      {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
      }
      glfwMakeContextCurrent(window);
      glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
      glfwSetCursorPosCallback(window, mouse_callback);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
      }

      glfwSwapInterval(0);
      
      Scene scene;
      scene.Init();

      IRenderer* renderer = NewRenderer();
      renderer->Init(&scene);

      //Gamemode
      IGameMode* game_mode = NewGameMode();
      game_mode->Init(&scene, renderer);
            


      while(!glfwWindowShouldClose(window))
      {
            glfwPollEvents();
            process_input(window);

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !first_key_press)
            {
                  game_mode->HandleEvent();
                  first_key_press = true;
            }

            renderer->Paint();
            
            glfwSwapBuffers(window);
      }

      
      return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
      glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
      
}

void process_input(GLFWwindow *window)
{
      if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
      {
            glfwSetWindowShouldClose(window, true);
      }
}
