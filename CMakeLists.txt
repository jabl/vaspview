# To build release version (1 order of magnitude faster!):
# mkdir build-release
# cd build-release
# CXX=g++ cmake -D CMAKE_BUILD_TYPE=Release ..

cmake_minimum_required (VERSION 2.6)

# Fortran required due to FindLAPACK module
project (tb C CXX)

find_package(GLUT)
find_package(OpenGL)
set(GLLIBS ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${X11_LIBRARIES})

#include_directories (src)
include_directories (${vaspview_SOURCE_DIR}/src)

# More compiler output
set( CMAKE_VERBOSE_MAKEFILE ON )

# OpenMP with GCC
#if (CMAKE_COMPILER_IS_GNUCXX) 
#   add_definitions (-g -msse2 -fopenmp -Wall -Wno-long-long -pedantic)
#endif ()
   
add_executable(vaspview ds3.c)

#include(FindLAPACK)
#if (LAPACK_FOUND)
#   target_link_libraries (tbcpp ${LAPACK_LIBRARIES})
#endif ()

#target_link_libraries (vaspview glut GLU GL Xi Xmu Xext X11 m ) 
target_link_libraries (vaspview ${GLLIBS} m ) 
