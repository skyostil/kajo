find_package(Threads)
find_package(SDL REQUIRED)
find_package(SDL_ttf)

add_executable(
    renderer
    Image.cpp
    Image.h
    Main.cpp
    PrecalculatedScene.cpp
    PrecalculatedScene.h
    Preview.cpp
    Preview.h
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
    SurfacePoint.cpp
    SurfacePoint.h
    Util.cpp
    Util.h
)

target_link_libraries(
    renderer
    scene
    lodepng
    ${SDL_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})
