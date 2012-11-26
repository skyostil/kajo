# - Find GLEW
# This module finds if GLEW is installed and determines where
# the include files and libraries are.
#
# This code sets the following variables:
#
#  GLEW_INCLUDE_PATH
#  GLEW_LIBRARY
#
#  GLEW_FOUND = is GLEW usable on system?

IF(GLEW_FOUND)
   # Already in cache, be silent
   SET(GLEW_FIND_QUIETLY TRUE)
ENDIF(GLEW_FOUND)

FIND_PATH(
        GLEW_INCLUDE_PATH glew.h
        PATHS /usr/include /usr/local/include /usr/pkg/include
        PATH_SUFFIXES GL GLEW
)

FIND_LIBRARY(
        GLEW_LIBRARY NAMES GLEW GLEW32 GLEW32S
        PATHS ${GLEW_INCLUDE_PATH}/../lib /usr/lib /usr/local/lib /usr/pkg/lib
        PATH_SUFFIXES lib lib64
)

IF(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
  SET(GLEW_INCLUDE_PATH "${GLEW_INCLUDE_PATH}")
  SET(GLEW_LIBRARY "${GLEW_LIBRARY}")
  SET(GLEW_FOUND TRUE)
ENDIF(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)

MARK_AS_ADVANCED(
  GLEW_INCLUDE_PATH
  GLEW_LIBRARY
)
