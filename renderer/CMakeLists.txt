add_executable(
    renderer
    Main.cpp
    Surface.cpp
    Surface.h
    Renderer.cpp
    Renderer.h
    Util.cpp
    Util.h
)

target_link_libraries(renderer scene lodepng)
