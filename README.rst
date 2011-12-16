==========
Vaspview 2
==========

This is an experiment to modernize the old `vaspview
<http://vaspview.sf.net>`_ application for viewing VASP charge density
files.

Installation
============

Dependencies
------------

Vaspview 2 requires the following tools and libraries for building:

- A C++ compiler.

- cmake.

- GLEW.

- GLUT.

- OpenGL libraries (GL and GLU).

On Ubuntu 10.04 you can install everything required by installing the packages

build-essential cmake libglew1.5-dev freeglut3-dev libgl1-mesa-dev
libglu1-mesa-dev libxi-dev libxmu-dev

On Redhat Enterprise Linux 5 + EPEL repo, you need (at least)

gcc-c++ cmake glew-devel freeglut-devel libXmu-devel

After that, make a directory for building, run cmake and make, e.g.

mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make

Assuming everything is successful, the binary will be found in
BUILD_DIRECTORY/src/vaspview .


Development Projects
====================

DONE
----

- Previously vaspview used manual setting up of GL function pointers
  on Windows, with only lowest common denominator fallback on
  non-Windows platforms. This has been replaced with the GLEW
  library. This provides an easy way to set up all the entry points,
  as well as an easy way to query for available extensions at runtime
  rather than compile-time.

- As a side-effect of the above, 3D textures are now enabled on all
  current hw, leading to MASSIVE speedup when moving the slice plane
  around.

- Use GL_FLOAT instead of GL_DOUBLE for vertices and normals. It's
  accurate enough, and on everything but the latest generation
  hardware (e.g. NVIDIA Fermi and later) doubles are an order of
  magnitude slower.

- Added pad fields so that the vertex structure is a multiple of 32
  bytes in size (previously 24 with GL_FLOAT, 48 with GL_DOUBLE); this
  is supposedly better on Ati according to
  http://www.opengl.org/wiki/Vertex_Buffer_Object

- Switch implementation language to C++98, allowing the replacement of
  CDynArray with std::string and std::vector, as appropriate, and
  replacing CHashTable with std::map (std::unordered_map in C++2011
  would be even better, but it doesn't matter performance-wise). Also,
  switching object allocation and destruction to new/delete and
  classes with constructors and destructors simplifies the code a bit.

- Optionally use Vertex Buffer Objects (VBO) for uploading isosurface
  data to the GPU. Time spent in ds3ViewIsoDrawNode dropped from 38%
  to 11%, nice!

- Use CMAKE for building.

- Enable support for compressed 3D textures. Tried to do this, but
  image quality was bad so it had to be removed.

- Removed old EXT_paletted_textures code path, as no current hardware
  supports paletted textures:
  http://www.opengl.org/registry/specs/EXT/paletted_texture.txt
  . NVIDIA hardware as of the NV4x generation (2004) does not support
  this, ATI/AMD and Intel never really supported it. Managed to test
  this code path on an old Geforce 2MX, but it didn't work there
  either, maybe a bug in the codepath, or a bug in the driver; in any
  case it doesn't matter as the codepath is now removed.

- Removed old 2D slice texture generation-on-CPU codepath, as all
  current hardware supports EXT_texture3D (part of OpenGL 1.2 core),
  and the fallback was unusably slow and buggy. Tested on GeForce 2MX
  and Radeon 9200, both ~10 year old HW in 2011, and both support 3D
  textures.

- Fix a bunch of bugs wrt using uninitialized memory.

TODO
====

- Triangle stripping? http://tomsdxfaq.blogspot.com/ claims stripping
  is an outdated optimization, as is
  http://home.comcast.net/~tom_forsyth/blog.wiki.html#Strippers (same
  guy, I think). OTOH rendering in vertex buffer ordering seems more
  important. http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html


- CUDA experiment: Move the marching cubes algorithm to the GPU. CUDA
  SDK has an example implementing marching cubes, integrate that?
  Currently this is the biggest performance bottleneck, up to 46%
  spent in DS3IsoSurface::isoMake() when moving the isosurface value
  slider back and forth.

- Replace vect.hh/vect.cc with Eigen?

- Someone could test, and contribute, OS X and Windows support
  (*wink*). The code itself should all be relatively platform neutral
  (thanks to GLEW), so mainly some patches to the cmake build scripts
  would be needed. There is also an old src/win32/winmain.c from the
  original vasputil, this could perhaps be resurrected.

- VBO rendering in batches to prevent exhausting GPU memory on older
  cards. Batch size should be about the size of the pre-T&L cache size
  on the GPU, which on slightly older cards (Geforce 6800) is
  apparently about ~64000 vertices. 

- Supposedly element rendering is faster if the indices are
  GL_UNSIGNED_SHORT rather than GL_UNSIGNED_INT. Since it's easy to
  have more than 2**16 vertices, this would imply batching and sorting
  the vertices etc.

- http://www.sci.utah.edu/~bavoil/opengl/vbo/batching/

- Replace builtin GLUT-based widget toolkit with Qt?

- Better use of RAII, per 'good' C++ style. This is, unfortunately,
  tedious as currently object ownership and lifetimes are not very
  clear.
