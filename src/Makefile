# VASP Data Viewer - Views 3d data sets of molecular charge distribution
# Copyright (C) 1999 Timothy B. Terriberry (mailto:tterribe@vt.edu)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Example Makefile. This will probably require manual editing for your system.

# Location to put the executable.
TARGETDIR = ../bin
# Name of the executable.
TARGET = vaspview
# The compiler to use.
CC = gcc
# The command to use to generate dependency information.
# If this does not work, try "makedepend -f- -Y --"
MAKEDEPEND = gcc -MM
# The location of include files. Modify these to point to your X11 and GL
# include directories.
CINCLUDE = -I. -I${HOME}/include -I/usr/X11R6/include
# Extra compilation flags.
# You may get speed increases by including flags such as -O2 or -O3 or
# -ffast-math, or additional flags, depending on your system and compiler.
# The -g flag will generally include debug information.
# The flag -D__DS3_NO_3D_TEXTURES__ will disable the use of 3D textures for the
#  slice.
# Some systems claim to support 3D textures, but I cannot get them to work 
#  (even the Mesa sample program stex3d does not work correctly on my Linux 
#  laptop, for example, and I've reports of failures on SGI machines as well). 
# Until I can figure out what I'm doing wrong, __DS3_NO_3D_TEXTURES__ can be 
#  defined to prevent the use of this feature. 
# It really is a much better way of doing things, though, so you should leave 
#  it enabled unless you have problems (e.g., your texture is always just a 
#  solid color).
CFLAGS = -O2 -g
# Libraries to link with, and the location of library files. Modify the
# locations to point to your X11 and GL libraries
LIBS = -L${HOME}/lib -L/usr/X11R6/lib \
-lglut -lGLU -lGL -lXi -lXmu -lXext -lX11 -lm

# ANYTHING BELOW THIS LINE PROBABLY DOES NOT NEED EDITING

# C source file list
CSOURCES = ds3.c ds3legnd.c ds3coord.c ds3pts.c ds3bonds.c ds3slice.c\
 ds3iso.c ds3view.c ds3vasp.c ds3viewr.c glw.c glwbuttn.c glwcheck.c\
 glwcomp.c glwframe.c glwgridb.c glwlabel.c glwslide.c glwtabpn.c glwtextf.c\
 DynArray.c HashTbl.c vect.c
CHEADERS = ds3.h ds3legnd.h ds3coord.h ds3pts.h ds3bonds.h ds3slice.h\
 ds3vasp.h ds3view.h ds3viewr.h glw.h DynArray.h HashTbl.h vect.h
# Object file list
OBJS = ${CSOURCES:.c=.o}

# Rules
.c.o:
	${CC} ${CINCLUDE} ${CFLAGS} -c -o $@ $<

# Proxy target
vaspview: ${TARGETDIR}/${TARGET}

# Remove all targets
clean:
	-rm ${OBJS} ${TARGETDIR}/${TARGET}

# Make the dependency list. Run this if the source files or what they include
# changes. Only tracks dependencies on headers in this project, not libraries.
depend: ${CSOURCES} ${CHEADERS}
	${MAKEDEPEND} ${CINCLUDE} ${CFLAGS} ${CSOURCES} > $@

# Include header file dependencies.
include depend

# Make executable.
${TARGETDIR}/${TARGET}: ${OBJS}
	if [ ! -d ${TARGETDIR} ] ; then mkdir -p ${TARGETDIR} ; fi
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}
