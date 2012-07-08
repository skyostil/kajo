find_package(Threads)

add_executable(
    renderer
    Main.cpp
    PrecalculatedScene.cpp
    PrecalculatedScene.h
    Random.cpp
    Random.h
    Ray.cpp
    Ray.h
    Raytracer.cpp
    Raytracer.h
    Renderer.cpp
    Renderer.h
    Shader.cpp
    Shader.h
    Surface.cpp
    Surface.h
    Util.cpp
    Util.h
)

target_link_libraries(renderer scene lodepng ${CMAKE_THREAD_LIBS_INIT})
