cmake_minimum_required(VERSION 4.1)
project(SceneRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure Jolt before adding it.
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)

add_subdirectory(
  C:/JoltPhysics/Build
  ${CMAKE_BINARY_DIR}/JoltPhysics
)

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
  penis/penis_jolt.cpp
  penis_jolt/debug_renderer.cpp

  c:/imgui/imgui.cpp
  c:/imgui/imgui_demo.cpp
  c:/imgui/imgui_draw.cpp
  c:/imgui/imgui_tables.cpp
  c:/imgui/imgui_widgets.cpp
  c:/imgui/backends/imgui_impl_opengl3.cpp
  c:/imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(SceneRenderer PRIVATE
  include  
  penis_common
  c:/gli  
  c:/cereal/include
  c:/imgui
  c:/imgui/backends
  C:/JoltPhysics
)

target_link_directories(SceneRenderer PRIVATE
  libs
  include/GLFW
  c:/imgui
  c:/imgui/backends
)

target_link_libraries(SceneRenderer PRIVATE
  Jolt
  glfw3
  opengl32
  gdi32
  dwmapi
)

target_compile_definitions(SceneRenderer PRIVATE
  GLM_FORCE_CTOR_INIT
  JPH_DEBUG_RENDERER
  JPH_PROFILE_ENABLED
  JPH_OBJECT_STREAM
)
  
