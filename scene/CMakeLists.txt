add_library(
    scene STATIC
    Scene.h
    Scene.cpp
)

add_custom_command(
  TARGET scene POST_BUILD
  COMMAND ctags --c++-kinds=+p --fields=+iaS --extra=+q -R .
  WORKING_DIRECTORY .. COMMENT "Creating tags" VERBATIM)
