#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "penis/scene.hpp"
#include "penis/renderer.hpp"
#include "penis/game_mode.hpp"
#include "penis/input.hpp"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void process_input(GLFWwindow *window);

GameData game_data;

bool full_screen = false;
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

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
      glfwSetWindowUserPointer(window, &game_data);
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
      game_mode->Init(&scene, window, renderer);
            


      while(!glfwWindowShouldClose(window))
      {
            glfwPollEvents();
            game_data.mouse_dx = 0.0f;
            game_data.mouse_dy = 0.0f;      
            process_input(window);

            game_mode->Update();

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
      GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(window));

      float xpos = static_cast<float>(xposIn);
      float ypos = static_cast<float>(yposIn);

      if (gd->first_mouse)
      {
            gd->last_x = xpos;
            gd->last_y = ypos;
            gd->first_mouse = false;
      }

      gd->mouse_dx += (xpos - gd->last_x);
      gd->mouse_dy += (gd->last_y - ypos);

      gd->last_x = xpos;
      gd->last_y = ypos;
}

void process_input(GLFWwindow *window)
{
      if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
      {
            glfwSetWindowShouldClose(window, true);
      }

      GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(window));
      PlayerInput& player_input = gd->player_input;

      glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? press_key(player_input.key_inputs.W) : release_key(player_input.key_inputs.W);

      glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? press_key(player_input.key_inputs.A) : release_key(player_input.key_inputs.A);
      
      glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? press_key(player_input.key_inputs.S) : release_key(player_input.key_inputs.S);
      
      glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? press_key(player_input.key_inputs.D) : release_key(player_input.key_inputs.D);

      glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS ? press_key(player_input.key_inputs.C) : release_key(player_input.key_inputs.C);

      glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS ? press_key(player_input.key_inputs.V) : release_key(player_input.key_inputs.V);

      glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ? press_key(player_input.key_inputs.space) : release_key(player_input.key_inputs.space);

      glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? press_key(player_input.key_inputs.shift) : release_key(player_input.key_inputs.shift);
      
      glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? press_key(player_input.key_inputs.ctrl) : release_key(player_input.key_inputs.ctrl);

      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? press_key(player_input.key_inputs.rmb) : release_key(player_input.key_inputs.rmb);
     
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? press_key(player_input.key_inputs.lmb) : release_key(player_input.key_inputs.lmb);
}
