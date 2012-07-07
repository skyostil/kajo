cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(rayska)

if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-ffast-math)
endif()

if(MSVC)
	add_definitions(/fp:fast)
endif()

include_directories(".")
include_directories("third_party/glm")

add_subdirectory(scene)
add_subdirectory(renderer)
add_subdirectory(coordinator)
