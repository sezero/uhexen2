
/*
 * $Id: config.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
 *
 * $Date: 2000/04/07 19:44:51 $
 * $Revision: 1.1.1.1 $
 *
 * (C) 1999 by Hyperion
 * All rights reserved
 *
 * This file is part of the MiniGL library project
 * See the file Licence.txt for more details
 *
 */

#ifndef __GLCONFIG_H
#define __GLCONFIG_H


/* This define enables clamping glColor functions
** (only the lazy programmer needs this overhead :))
*/

#define CLAMP_COLORS 1

/* This define enables some sanity-checks for vertexarrays.
**
** If disabled, make sure that the following is in "sync":
**
** - glShadeModel, glColorPointer and GL_COLOR_ARRAY
**
** - GL_TEXTURE_2D, glTexCoordPointer and
**   GL_TEXTURE_COORD_ARRAY
**
*/

//#define VA_SANITY_CHECK 1


/* define if you want to use static inline functions instead
** of macros
*/

//#define USE_MGLAPI 1

//this tells mglmacros.h not to replace the API:

#ifdef USE_MGLAPI
	#define NO_MGLMACROS 1
#else
	#undef NO_MGLMACROS
#endif


// Stack sizes of the different matrix stacks

#define MODELVIEW_STACK_SIZE    40
#define PROJECTION_STACK_SIZE  5

// Define this to make mglLockMode available
#define AUTOMATIC_LOCKING_ENABLE 1

// Define this is you don't want the ability to log GL calls
#define NLOGGING 1

// Define if you don't want debugging
#define GLNDEBUG 1

// Define if you don't want glGetError functionality
#define GL_NOERRORCHECK 1


// Define if you don't want to check if the bitmaps allocated for
// screen buffering are cybergraphics bitmaps
#define NCGXDEBUG 1

// define if you don't want to draw anything
// #define NODRAW

// Maximum number of vertices a primitive can have
// Raise this value if needed, but this *should* really be enough.

#define MGL_MAXVERTS 1024

#endif





