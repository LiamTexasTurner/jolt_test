cmake_minimum_required(VERSION 4.1)
project(JobSystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(JobSystem
  main.cpp
  JobSystem.cpp
)

target_compile_options(JobSystem PRIVATE
 /Zi
)


