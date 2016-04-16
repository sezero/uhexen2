/*
 * $Id: modes.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
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

#ifndef __MGL_MODES_H
#define __MGL_MODES_H

#ifdef __VBCC__
#pragma amiga-align
#endif

#include <Warp3D/Warp3D.h>

#ifdef __VBCC__
#pragma default-align
#endif

#define MGL_MAX_MODE 80

typedef struct
{
	GLint id; // blackbox id used for mglCreateContextID()
	GLint width,height;     // screenmode size
	GLint bit_depth;        // depth of mode
	char  mode_name[MGL_MAX_MODE]; // name for this mode
} MGLScreenMode;

typedef struct
{
	ULONG width,height,depth;
	ULONG pixel_format;
	void *base_address;
	ULONG pitch;
} MGLLockInfo;

typedef GLboolean (*MGLScreenModeCallback)(MGLScreenMode *);

#endif
