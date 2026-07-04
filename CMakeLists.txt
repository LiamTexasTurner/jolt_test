cmake_minimum_required(VERSION 4.1)
project(SceneRenderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(SceneRenderer
  main.cpp
  include/stb_image.cpp
  include/cgltf.cpp
  include/glad/glad.c

  penis/scene.cpp
  

)

target_include_directories(SceneRenderer PRIVATE
  include
)

target_link_directories(SceneRenderer PRIVATE
  include/GLFW
)

target_link_libraries(SceneRenderer PRIVATE
  glfw3
  opengl32
  gdi32
)
