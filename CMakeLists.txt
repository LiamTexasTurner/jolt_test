cmake_minimum_required(VERSION 4.1)
project(SceneRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(SceneRenderer
  main.cpp
  include/stb_image.cpp
  include/cgltf.cpp
  include/glad/glad.c

  penis/scene.cpp
  penis/renderer.cpp
  penis/PRenderer.cpp
  penis/game_mode.cpp
  penis/shaderset.cpp
  penis/camera.cpp
  penis/entity.cpp
  penis/skybox.cpp
  penis/animation.cpp
  penis/job_system.cpp
  penis/arena.cpp
  penis/animation_graph.cpp


  c:/imgui/imgui.cpp
  c:/imgui/imgui_demo.cpp
  c:/imgui/imgui_draw.cpp
  c:/imgui/imgui_tables.cpp
  c:/imgui/imgui_widgets.cpp
  c:/imgui/backends/imgui_impl_opengl3.cpp
  c:/imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(SceneRenderer PRIVATE
  c:/gli
  include
  penis_common
  c:/cereal/include
  c:/imgui
  c:/imgui/backends
)

target_link_directories(SceneRenderer PRIVATE
  include/GLFW
  c:/imgui
  c:/imgui/backends
)

target_link_libraries(SceneRenderer PRIVATE
  glfw3
  opengl32
  gdi32
  dwmapi
)

target_compile_definitions(SceneRenderer PRIVATE
  GLM_FORCE_CTOR_INIT
)
  
