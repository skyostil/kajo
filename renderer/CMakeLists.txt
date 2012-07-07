add_executable(
    renderer
    Main.cpp
    Surface.cpp
    Surface.h
    Ray.cpp
    Ray.h
    Renderer.cpp
    Renderer.h
    Util.cpp
    Util.h
)

target_link_libraries(renderer scene lodepng)
