add_executable(
    renderer
    Main.cpp
    Renderer.cpp
    Renderer.h
)

target_link_libraries(renderer scene lodepng)
