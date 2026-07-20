#include <Windows.h>
#include <dwmapi.h>

#include <glad/glad.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#include "penis/imgui_themes.hpp"
#include "penis/scene.hpp"
#include "penis/renderer.hpp"
#include "penis/game_mode.hpp"
#include "penis/input.hpp"
#include "penis/job_system.h"


#include <vector>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_wheel_callback(GLFWwindow* window, double x_offset, double y_offset);
void process_input(GLFWwindow *window);

GameData game_data;

bool full_screen = false;
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

unsigned int PREV_SCR_WIDTH = SCR_WIDTH;
unsigned int PREV_SCR_HEIGHT = SCR_HEIGHT;


double last_time = 0.0;
double accumulator = 0.0;
double now = 0.0;
double delta_time = 0.0;

int main()
{
#ifdef _DEBUG
      _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_DEBUG);
      _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_DEBUG);
      _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif
      JobSystem::Initialize();
      glfwInit();
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Penis", monitor, NULL);
      }
      else
      {
            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Penis", monitor, NULL);
      }

      HWND hwnd = glfwGetWin32Window(window);

      // COLORREF is 0x00BBGGRR, not RGB order.
      COLORREF titleBarColor = RGB(25, 25, 25);
      
      DwmSetWindowAttribute(hwnd,
                            DWMWA_CAPTION_COLOR,
                            &titleBarColor,
                            sizeof(titleBarColor)
                            );
      int width = 0;
      int height = 0;
      int channels = 0;

      unsigned char* pixels = stbi_load("../resources/manray/manray_headset_small.png",
                                        &width,
                                        &height,
                                        &channels,
                                        4
                                        );

      if (pixels)
      {
            GLFWimage icon;
            icon.width  = width;
            icon.height = height;
            icon.pixels = pixels;

            glfwSetWindowIcon(window, 1, &icon);

            stbi_image_free(pixels);
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
      glfwSetScrollCallback(window, scroll_wheel_callback);
      glfwSetWindowUserPointer(window, &game_data);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      
      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
      }
      
      glfwSwapInterval(0);

      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
      SetDarkTheme();

      // Setup Platform/Renderer backends
      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init();
      
      Scene scene;
      scene.Init();

      IRenderer* renderer = NewRenderer();
      renderer->Init(&scene);

      renderer->Resize(SCR_WIDTH, SCR_HEIGHT);

      IGameMode* game_mode = NewGameMode();
      game_mode->Init(&scene, window, renderer);
     

      while(!glfwWindowShouldClose(window))
      {
            now = glfwGetTime();
            delta_time = now - last_time;
            last_time = now;

            glfwPollEvents();      
            process_input(window);

            game_mode->Update(delta_time);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            const ImGuiViewport* vp = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(vp->Size, ImGuiCond_Always);
            ImGui::SetNextWindowViewport(vp->ID);

            ImGui::Begin("Main", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);

            ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0, 0));                  
            ImGui::End();
                  
            ImGui::Begin("Scene");
      
            ImVec2 current_sceen_size = ImGui::GetContentRegionAvail();
            SCR_WIDTH = (int)current_sceen_size.x;
            SCR_HEIGHT = (int)current_sceen_size.y;
            if(SCR_WIDTH != PREV_SCR_WIDTH || SCR_HEIGHT != PREV_SCR_HEIGHT)
            {
                  renderer->Resize(SCR_WIDTH, SCR_HEIGHT);
                  PREV_SCR_WIDTH = SCR_WIDTH;
                  PREV_SCR_HEIGHT = SCR_HEIGHT;
            }
       
            unsigned int render_texture = renderer->Paint();

            ImGui::Image((ImTextureID)(intptr_t)render_texture,
                         ImVec2((float)SCR_WIDTH, (float)SCR_HEIGHT),
                         ImVec2(0, 1),
                         ImVec2(1, 0));
      
            ImGui::End();

            ImGui::Begin("Details");

            ImGui::PushFont(nullptr, 28.0f);

            ImGui::Text("%f", 1.0f / delta_time);

            ImGui::PopFont();

            ImGui::End();            

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
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
      if(gd->cursor_enabled) return;
      MouseData& mouse_data = gd->player_input.mouse_data;

      float xpos = static_cast<float>(xposIn);
      float ypos = static_cast<float>(yposIn);

      if (mouse_data.first_mouse)
      {
            mouse_data.last_x = xpos;
            mouse_data.last_y = ypos;
            mouse_data.first_mouse = false;
      }

      mouse_data.dx += (xpos - mouse_data.last_x);
      mouse_data.dy += (mouse_data.last_y - ypos);

      mouse_data.last_x = xpos;
      mouse_data.last_y = ypos;
}

void scroll_wheel_callback(GLFWwindow* window, double x_offset, double y_offset)
{
      GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(window));
      MouseData& mouse_data = gd->player_input.mouse_data;

      mouse_data.scroll = y_offset;
}

void process_input(GLFWwindow *window)
{

      GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(window));
      PlayerInput& player_input = gd->player_input;
      
      if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
      {
            glfwSetWindowShouldClose(window, true);
      }

      glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? press_key(player_input.key_inputs.W) : release_key(player_input.key_inputs.W);

      glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? press_key(player_input.key_inputs.A) : release_key(player_input.key_inputs.A);
      
      glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? press_key(player_input.key_inputs.S) : release_key(player_input.key_inputs.S);
      
      glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? press_key(player_input.key_inputs.D) : release_key(player_input.key_inputs.D);

      glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? press_key(player_input.key_inputs.E) : release_key(player_input.key_inputs.E);

      glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? press_key(player_input.key_inputs.Q) : release_key(player_input.key_inputs.Q);

      glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS ? press_key(player_input.key_inputs.C) : release_key(player_input.key_inputs.C);

      glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS ? press_key(player_input.key_inputs.V) : release_key(player_input.key_inputs.V);
      
      glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ? press_key(player_input.key_inputs.space) : release_key(player_input.key_inputs.space);

      glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS ? press_key(player_input.key_inputs.tab) : release_key(player_input.key_inputs.tab);

      glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? press_key(player_input.key_inputs.shift) : release_key(player_input.key_inputs.shift);
      
      glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? press_key(player_input.key_inputs.ctrl) : release_key(player_input.key_inputs.ctrl);

      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? press_key(player_input.key_inputs.rmb) : release_key(player_input.key_inputs.rmb);
     
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? press_key(player_input.key_inputs.lmb) : release_key(player_input.key_inputs.lmb);

      glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS ? press_key(player_input.key_inputs.num_4) : release_key(player_input.key_inputs.num_4);

      
      if(player_input.key_inputs.num_4.pressed)
      {
            gd->cursor_enabled = !gd->cursor_enabled;
            if(gd->cursor_enabled)
            {
                  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                  player_input.mouse_data.first_mouse = true;
            }
            else
            {
                  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);           
            }            
      }
}
