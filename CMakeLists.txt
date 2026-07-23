cmake_minimum_required(VERSION 3.20)

project(SceneRenderer LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

find_package(OpenGL REQUIRED)

# ------------------------------------------------------------
# Jolt Physics
# ------------------------------------------------------------

# These must be configured before adding Jolt.
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)

# Configure Jolt features through Jolt's CMake options.
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE ON CACHE BOOL "" FORCE)
set(PROFILER_IN_DEBUG_AND_RELEASE ON CACHE BOOL "" FORCE)
set(ENABLE_OBJECT_STREAM ON CACHE BOOL "" FORCE)

# Do not treat warnings in third-party Jolt code as errors.
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)

add_subdirectory(
  "${THIRD_PARTY_DIR}/JoltPhysics/Build"
  "${CMAKE_CURRENT_BINARY_DIR}/JoltPhysics"
  EXCLUDE_FROM_ALL
)

# ------------------------------------------------------------
# GLFW
# ------------------------------------------------------------

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(
  "${THIRD_PARTY_DIR}/glfw"
  "${CMAKE_CURRENT_BINARY_DIR}/glfw"
  EXCLUDE_FROM_ALL
)

# ------------------------------------------------------------
# Dear ImGui
# ------------------------------------------------------------

add_library(imgui STATIC
  "${THIRD_PARTY_DIR}/imgui/imgui.cpp"
  "${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp"
  "${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp"
  "${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp"
  "${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp"
  "${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
  "${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp"
)

target_include_directories(imgui PUBLIC
  "${THIRD_PARTY_DIR}/imgui"
  "${THIRD_PARTY_DIR}/imgui/backends"
)

target_link_libraries(imgui PUBLIC
  glfw
  OpenGL::GL
)

# ------------------------------------------------------------
# Header-only libraries
# ------------------------------------------------------------

add_library(cereal INTERFACE)

target_include_directories(cereal SYSTEM INTERFACE
  "${THIRD_PARTY_DIR}/cereal/include"
)

add_library(gli INTERFACE)

target_include_directories(gli SYSTEM INTERFACE
  "${THIRD_PARTY_DIR}/gli"
)

# ------------------------------------------------------------
# SceneRenderer
# ------------------------------------------------------------

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
)

target_compile_features(SceneRenderer PRIVATE cxx_std_20)

set_target_properties(SceneRenderer PROPERTIES
  CXX_EXTENSIONS OFF
)

target_include_directories(SceneRenderer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/penis"
  "${CMAKE_CURRENT_SOURCE_DIR}/penis_common"
  "${CMAKE_CURRENT_SOURCE_DIR}/penis_jolt"

  # Jolt's headers begin at the repository root.
  "${THIRD_PARTY_DIR}/JoltPhysics"
)

target_link_libraries(SceneRenderer PRIVATE
  Jolt
  glfw
  imgui
  cereal
  gli
  OpenGL::GL
)

target_compile_definitions(SceneRenderer PRIVATE
  GLM_FORCE_CTOR_INIT
)

# Windows-only libraries used directly by your application.
if(WIN32)
  target_link_libraries(SceneRenderer PRIVATE
    gdi32
    dwmapi
  )
endif()
