set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/renderer")

find_package(Threads)
find_package(SDL REQUIRED)
find_package(SDL_ttf)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(
    renderer
    GLHelpers.cpp
    GLHelpers.h
    Image.cpp
    Image.h
    Main.cpp
    Preview.cpp
    Preview.h
    Scheduler.h
    Util.cpp
    Util.h

    # CPU renderer
    cpu/BSDF.cpp
    cpu/BSDF.h
    cpu/Light.cpp
    cpu/Light.h
    cpu/Queue.h
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
    cpu/Scheduler.cpp
    cpu/Scheduler.h
    cpu/Shader.cpp
    cpu/Shader.h
    cpu/SurfacePoint.cpp
    cpu/SurfacePoint.h

    # OpenGL renderer
    gl/BSDF.cpp
    gl/BSDF.h
    gl/Light.cpp
    gl/Light.h
    gl/Raytracer.cpp
    gl/Raytracer.h
    gl/Random.cpp
    gl/Random.h
    gl/Renderer.cpp
    gl/Renderer.h
    gl/Scene.cpp
    gl/Scene.h
    gl/Scheduler.cpp
    gl/Scheduler.h
    gl/ShaderUtil.cpp
    gl/ShaderUtil.h
    gl/SurfaceShader.cpp
    gl/SurfaceShader.h
)

target_link_libraries(
    renderer
    scene
    lodepng
    ${SDL_LIBRARY}
    ${OPENGL_LIBRARY}
    ${GLEW_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)
