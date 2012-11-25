find_package(Threads)
find_package(SDL REQUIRED)
find_package(SDL_ttf)

add_executable(
    renderer
    Image.cpp
    Image.h
    Main.cpp
    Preview.cpp
    Preview.h
    Util.cpp
    Util.h

    # CPU renderer
    cpu/BSDF.cpp
    cpu/BSDF.h
    cpu/Light.cpp
    cpu/Light.h
    cpu/Random.cpp
    cpu/Random.h
    cpu/Ray.cpp
    cpu/Ray.h
    cpu/Raytracer.cpp
    cpu/Raytracer.h
    cpu/Renderer.cpp
    cpu/Renderer.h
    cpu/Scene.cpp
    cpu/Scene.h
    cpu/Shader.cpp
    cpu/Shader.h
    cpu/SurfacePoint.cpp
    cpu/SurfacePoint.h
)

target_link_libraries(
    renderer
    scene
    lodepng
    ${SDL_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)
