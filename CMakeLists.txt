# To build release version:
# mkdir build-release
# cd build-release
# cmake -D CMAKE_BUILD_TYPE=Release ..

cmake_minimum_required (VERSION 2.4)

# For GLEW module
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

project (vaspview C CXX)

SUBDIRS (src)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
set(GLLIBS ${GLUT_LIBRARY} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} ${X11_LIBRARIES})

# More compiler output
set( CMAKE_VERBOSE_MAKEFILE ON )

if (CMAKE_COMPILER_IS_GNUCXX) 
   add_definitions (-g -Wall -Wno-long-long -pedantic -fno-math-errno)
endif ()
