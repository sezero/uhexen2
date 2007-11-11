/*
	loadtri.h
	$Id: loadtri.h,v 1.5 2007-11-11 18:48:07 sezero Exp $
*/

#ifndef __LOADTRI_H
#define __LOADTRI_H

#define MAXTRIANGLES	2048

typedef struct
{
	vec3_t	verts[3];
} triangle_t;

void LoadTriangleList(const char *fileName, triangle_t **triList, int *triangleCount);

extern char	InputFileName[1024];

extern float	FixHTRRotateX;
extern float	FixHTRRotateY;
extern float	FixHTRRotateZ;

extern float	FixHTRTranslateX;
extern float	FixHTRTranslateY;
extern float	FixHTRTranslateZ;

#endif	/* __LOADTRI_H */

