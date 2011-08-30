/*VASP Data Viewer 2 - Views 3d data sets of molecular charge distribution
  Copyright (C) 2011 Janne Blomqvist

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/

// OpenGL / GLEW / GLUT includes

#ifndef GLINC_HH
#define GLINC_HH

// Problem:

// The stdlib.h which ships with the recent versions of Visual Studio
// has a different (and conflicting) definition of the exit()
// function. It clashes with the definition in glut.h.

// Solution:

// Override the definition in glut.h with that in stdlib.h. Place the
// stdlib.h line above the glut.h line in your code.

#include <stdlib.h>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#endif
