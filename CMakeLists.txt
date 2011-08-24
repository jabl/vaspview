# To build release version (1 order of magnitude faster!):
# mkdir build-release
# cd build-release
# CXX=g++ cmake -D CMAKE_BUILD_TYPE=Release ..

cmake_minimum_required (VERSION 2.6)

project (vaspview C CXX)

SUBDIRS (src)
find_package(GLUT)
find_package(OpenGL)
set(GLLIBS ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${X11_LIBRARIES})

# More compiler output
set( CMAKE_VERBOSE_MAKEFILE ON )

# OpenMP with GCC
#if (CMAKE_COMPILER_IS_GNUCXX) 
#   add_definitions (-g -msse2 -fopenmp -Wall -Wno-long-long -pedantic)
#endif ()
   
add_definitions (-O2 -g -std=c++0x)

