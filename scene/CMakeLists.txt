add_library(
    scene STATIC
    Parser.h
    Parser.cpp
    Scene.h
    Scene.cpp
)

target_link_libraries(
    scene
    simplejson
)

if(!APPLE)
  add_custom_command(
    TARGET scene POST_BUILD
    COMMAND ctags --c++-kinds=+p --fields=+iaS --extra=+q -R .
    WORKING_DIRECTORY .. COMMENT "Creating tags" VERBATIM)
endif()
