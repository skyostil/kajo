cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(kajo)

# -----------------------------------------------------------------------
# Check whether we compile with CLANG.
# -----------------------------------------------------------------------
set(COMPILER_IS_CLANG FALSE)
if(CMAKE_CXX_COMPILER MATCHES "clang\\+\\+")
  set(COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
  add_definitions(-O3 -ffast-math)
  add_definitions(-std=c++0x)
  if(NOT APPLE)
    add_definitions(-march=native -mtune=native)
  else()
    add_definitions(-stdlib=libc++) # Use clang's c++ standard library
    link_libraries(-stdlib=libc++)
  endif()
  add_definitions(-Wall)
  add_definitions(-Wno-comment) # glm bug

  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                  OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_LESS 4.7)
    add_definitions(-Doverride=)
    add_definitions(-Dsteady_clock=monotonic_clock)
  endif()
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
include_directories("third_party/SimpleJSON/src")

add_subdirectory(scene)
add_subdirectory(renderer)
add_subdirectory(coordinator)
add_subdirectory(third_party/lodepng)
add_subdirectory(third_party/SimpleJSON)
