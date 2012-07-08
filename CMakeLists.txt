cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(rayska)

if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-O3 -march=native -mtune=native -ffast-math)
  add_definitions(-std=c++0x)
  add_definitions(-Wall)
  add_definitions(-Wno-comment) # glm bug
endif()

if(MSVC)
  add_definitions(/fp:fast)
endif()

add_definitions(-DGLM_FORCE_CXX11)
add_definitions(-DGLM_SWIZZLE)
add_definitions(-DUSE_SSE2)

include_directories(".")
include_directories("third_party/glm")
include_directories("third_party/lodepng")

add_subdirectory(scene)
add_subdirectory(renderer)
add_subdirectory(coordinator)
add_subdirectory(third_party/lodepng)
