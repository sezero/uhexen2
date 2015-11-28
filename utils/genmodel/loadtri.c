/*
 * loadtri.c
 * $Id: loadtri.c,v 1.14 2010-02-22 22:30:33 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "pathutil.h"
#include "mathlib.h"
#include "loadtri.h"
#include "token.h"
#include "q_endian.h"

// MACROS ------------------------------------------------------------------

//#undef M_PI
//#define M_PI	3.14159265
#define MY_PI	3.14159265

// 3DS binary files
#define _3DS_MAIN3DS		0x4D4D
#define _3DS_EDIT3DS		0x3D3D
#define _3DS_EDIT_MATERIAL	0xAFFF
#define _3DS_EDIT_VIEW1		0x7001
#define _3DS_EDIT_BACKGR	0x1200
#define _3DS_EDIT_AMBIENT	0x2100
#define _3DS_EDIT_UNKNW01	0x1100
#define _3DS_EDIT_UNKNW02	0x1201
#define _3DS_EDIT_UNKNW03	0x1300
#define _3DS_EDIT_UNKNW04	0x1400
#define _3DS_EDIT_UNKNW05	0x1420
#define _3DS_EDIT_UNKNW06	0x1450
#define _3DS_EDIT_UNKNW07	0x1500
#define _3DS_EDIT_UNKNW08	0x2200
#define _3DS_EDIT_UNKNW09	0x2201
#define _3DS_EDIT_UNKNW10	0x2210
#define _3DS_EDIT_UNKNW11	0x2300
#define _3DS_EDIT_UNKNW12	0x2302
#define _3DS_EDIT_UNKNW13	0x3000
#define _3DS_EDIT_UNKNW14	0xAFFF
#define _3DS_EDIT_OBJECT	0x4000
#define _3DS_OBJ_LIGHT		0x4600
#define _3DS_OBJ_CAMERA		0x4700
#define _3DS_OBJ_UNKNWN01	0x4010
#define _3DS_OBJ_UNKNWN02	0x4012
#define _3DS_OBJ_TRIMESH	0x4100
#define _3DS_TRI_FACEL2		0x4111
#define _3DS_TRI_VISIBLE	0x4165
#define _3DS_TRI_VERTEXL	0x4110
#define _3DS_TRI_FACEL1		0x4120

// TRI binary files
#define FLOAT_START	99999.0
#define FLOAT_END	-FLOAT_START
#define TRI_MAGIC	123322

// TYPES -------------------------------------------------------------------

typedef struct
{
	float	v[3];
} vector;

typedef struct
{
	vector	n;	// normal
	vector	p;	// point
	vector	c;	// color
	float	u;	// u
	float	v;	// v
} aliaspoint_t;

typedef struct
{
	aliaspoint_t	pt[3];
} tf_triangle;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void LoadHRC(char *fileName, triangle_t **triList, int *triangleCount);
static void LoadASC(char *fileName, triangle_t **triList, int *triangleCount);
static void LoadHTR(char *fileName, triangle_t **triList, int *triangleCount);
static void LoadTRI(FILE *input, triangle_t **triList, int *triangleCount);

static void ByteSwapTri(tf_triangle *tri);

#if 0	/* unused 3DS stuff */
static void Load3DS(FILE *input, triangle_t **triList, int *triangleCount);
static void _3DSError(char *message);

static float ReadFloat(void);
static unsigned long ReadLong(void);
static unsigned short ReadShort(void);
static unsigned char ReadByte(void);
static void SkipName(void);
static void SeekTo(int position);
static unsigned long FilePosition(void);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char	InputFileName[1024];
FILE	*InputFile;
float	FixHTRRotateX = 0.0;
float	FixHTRRotateY = 0.0;
float	FixHTRRotateZ = 0.0;
float	FixHTRTranslateX = 0.0;
float	FixHTRTranslateY = 0.0;
float	FixHTRTranslateZ = 0.0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// LoadTriangleList
//
//==========================================================================

void LoadTriangleList(const char *fileName, triangle_t **triList, int *triangleCount)
{
	FILE	*input;

	q_strlcpy(InputFileName, fileName, sizeof(InputFileName));

	StripExtension(InputFileName);
	q_strlcat(InputFileName, ".asc", sizeof(InputFileName));
	if ((input = fopen(InputFileName, "rb")) != NULL)
	{
		fclose(input);
		LoadASC(InputFileName, triList, triangleCount);
		return;
	}

	StripExtension(InputFileName);
	q_strlcat(InputFileName, ".hrc", sizeof(InputFileName));
	if ((input = fopen(InputFileName, "rb")) != NULL)
	{
		fclose(input);
		LoadHRC(InputFileName, triList, triangleCount);
		return;
	}

	StripExtension(InputFileName);
	q_strlcat(InputFileName, ".htr", sizeof(InputFileName));
	if ((input = fopen(InputFileName, "rb")) != NULL)
	{
		fclose(input);
		LoadHTR(InputFileName, triList, triangleCount);
		return;
	}

	StripExtension(InputFileName);
	q_strlcat(InputFileName, ".tri", sizeof(InputFileName));
	if ((input = fopen(InputFileName, "rb")) != NULL)
	{
		LoadTRI(input, triList, triangleCount);
		fclose(input);
		return;
	}

	COM_Error("Could not open file '%s':\n"
		"No ASC, HRC, HTR, or TRI match.\n", fileName);
}

//==========================================================================
//
// LoadHRC
//
//==========================================================================

static void LoadHRC(char *fileName, triangle_t **triList, int *triangleCount)
{
	int		i, j;
	int		vertexCount;
	struct vList_s
	{
		float v[3];
	} *vList;
	int		triCount;
	triangle_t	*tList;
	float	scaling[3];
	float	rotation[3];
	float	translation[3];
	float	x, y, z;
	float	x2, y2, z2;
	float	rx, ry, rz;

	TK_Init();
	TK_OpenSource(fileName);
	TK_FetchRequire(TK_HRCH);
	TK_FetchRequire(TK_COLON);
	TK_FetchRequire(TK_SOFTIMAGE);
	TK_Beyond(TK_MODEL);
	TK_Beyond(TK_SCALING);
	for (i = 0; i < 3; i++)
	{
		TK_Require(TK_FLOATNUMBER);
		scaling[i] = tk_FloatNumber;
		TK_Fetch();
	}
	TK_Beyond(TK_ROTATION);
	for (i = 0; i < 3; i++)
	{
		TK_Require(TK_FLOATNUMBER);
		rotation[i] = tk_FloatNumber;
		TK_Fetch();
	}
	TK_Beyond(TK_TRANSLATION);
	for (i = 0; i < 3; i++)
	{
		TK_Require(TK_FLOATNUMBER);
		translation[i] = tk_FloatNumber;
		TK_Fetch();
	}

	rx = (float)(((rotation[0]-90.0)/360.0)*2.0*MY_PI);
	ry = (float)((rotation[1]/360.0)*2.0*MY_PI);
	rz = (float)((rotation[2]/360.0)*2.0*MY_PI);

	TK_Beyond(TK_MESH);
	TK_BeyondRequire(TK_VERTICES, TK_INTNUMBER);
	vertexCount = tk_IntNumber;
	vList = (struct vList_s *) SafeMalloc(vertexCount * sizeof(vList[0]));
	for (i = 0; i < vertexCount; i++)
	{
		TK_BeyondRequire(TK_LBRACKET, TK_INTNUMBER);
		if (tk_IntNumber != i)
		{
			COM_Error("File '%s', line %d:\nVertex index mismatch.\n",
						tk_SourceName, tk_Line);
		}
		TK_Beyond(TK_POSITION);
		// Apply the scaling, rotation, and translation in the order
		// specified in the HRC file.  This could be wrong.
		TK_Require(TK_FLOATNUMBER);
		x = tk_FloatNumber*scaling[0];
		TK_FetchRequire(TK_FLOATNUMBER);
		y = tk_FloatNumber*scaling[1];
		TK_FetchRequire(TK_FLOATNUMBER);
		z = tk_FloatNumber*scaling[2];

		y2 = (float)(y*cos(rx)+z*sin(rx));
		z2 = (float)(-y*sin(rx)+z*cos(rx));
		y = y2;
		z = z2;

		x2 = (float)(x*cos(ry)-z*sin(ry));
		z2 = (float)(x*sin(ry)+z*cos(ry));
		x = x2;
		z = z2;

		x2 = (float)(x*cos(rz)+y*sin(rz));
		y2 = (float)(-x*sin(rz)+y*cos(rz));
		x = x2;
		y = y2;

		vList[i].v[0] = x+translation[0];
		vList[i].v[1] = y+translation[1];
		vList[i].v[2] = z+translation[2];
	}
	TK_BeyondRequire(TK_POLYGONS, TK_INTNUMBER);
	triCount = tk_IntNumber;
	if (triCount >= MAXTRIANGLES)
	{
		COM_Error("Too many triangles in file %s\n", InputFileName);
	}
	*triangleCount = triCount;
	tList = (triangle_t *) SafeMalloc(MAXTRIANGLES * sizeof(triangle_t));
	*triList = tList;
	for (i = 0; i < triCount; i++)
	{
		TK_BeyondRequire(TK_LBRACKET, TK_INTNUMBER);
		if (tk_IntNumber != i)
		{
			COM_Error("File '%s', line %d:\nTriangle index mismatch.\n",
						tk_SourceName, tk_Line);
		}
		TK_BeyondRequire(TK_NODES, TK_INTNUMBER);
		if (tk_IntNumber != 3)
		{
			COM_Error("File '%s', line %d:\nBad polygon vertex count: %d.",
					tk_SourceName, tk_Line, tk_IntNumber);
		}
		for (j = 0; j < 3; j++)
		{
			TK_BeyondRequire(TK_LBRACKET, TK_INTNUMBER);
			if (tk_IntNumber != j)
			{
				COM_Error("File '%s', line %d:\nTriangle vertex index"
					" mismatch.  %d should be %d\n", tk_SourceName, tk_Line,
					tk_IntNumber, j);
			}
			TK_BeyondRequire(TK_VERTEX, TK_INTNUMBER);
			tList[i].verts[2-j][0] = vList[tk_IntNumber].v[0];
			tList[i].verts[2-j][1] = vList[tk_IntNumber].v[1];
			tList[i].verts[2-j][2] = vList[tk_IntNumber].v[2];
		}

/*		printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
			"  v2: %f, %f, %f\n", i,
			tList[i].verts[0][0],
			tList[i].verts[0][1],
			tList[i].verts[0][2],
			tList[i].verts[1][0],
			tList[i].verts[1][1],
			tList[i].verts[1][2],
			tList[i].verts[2][0],
			tList[i].verts[2][1],
			tList[i].verts[2][2]);
*/
	}
}

//==========================================================================
//
// LoadASC
//
//==========================================================================

static void LoadASC(char *fileName, triangle_t **triList, int *triangleCount)
{
	int		i, j;
	int		vertexCount;
	struct vList_s
	{
		float v[3];
	} *vList;
	int		triCount;
	triangle_t	*tList;
	float	x, y, z;
//	float	x2, y2, z2;
//	float	rx, ry, rz;
	qboolean goodObject;

	TK_Init();
	TK_OpenSource(fileName);

	goodObject = false;
	while (goodObject == false)
	{
		TK_Beyond(TK_C_NAMED);
		TK_Beyond(TK_OBJECT);
		TK_Beyond(TK_C_TRI);
		TK_Beyond(TK_MESH);
		TK_BeyondRequire(TK_C_VERTICES, TK_COLON);
		TK_FetchRequire(TK_INTNUMBER);
		vertexCount = tk_IntNumber;
		if (vertexCount > 0)
		{
			goodObject = true;
		}
	}
	TK_BeyondRequire(TK_C_FACES, TK_COLON);
	TK_FetchRequire(TK_INTNUMBER);
	triCount = tk_IntNumber;
	if (triCount >= MAXTRIANGLES)
	{
		COM_Error("Too many triangles in file %s\n", InputFileName);
	}
	*triangleCount = triCount;
	tList = (triangle_t *) SafeMalloc(MAXTRIANGLES * sizeof(triangle_t));
	*triList = tList;
	TK_BeyondRequire(TK_C_VERTEX, TK_LIST);

/*	rx = ((rotation[0]+90.0)/360.0)*2.0*MY_PI;
	//rx = (rotation[0]/360.0)*2.0*MY_PI;
	ry = (rotation[1]/360.0)*2.0*MY_PI;
	rz = (rotation[2]/360.0)*2.0*MY_PI;
*/
	vList = (struct vList_s *) SafeMalloc(vertexCount * sizeof(vList[0]));
	for (i = 0; i < vertexCount; i++)
	{
		TK_BeyondRequire(TK_C_VERTEX, TK_INTNUMBER);
		if (tk_IntNumber != i)
		{
			COM_Error("File '%s', line %d:\nVertex index mismatch.\n",
						tk_SourceName, tk_Line);
		}
		TK_FetchRequireFetch(TK_COLON);

		TK_BeyondRequire(TK_COLON, TK_FLOATNUMBER);
		x = tk_FloatNumber;
		TK_BeyondRequire(TK_COLON, TK_FLOATNUMBER);
		y = tk_FloatNumber;
		TK_BeyondRequire(TK_COLON, TK_FLOATNUMBER);
		z = tk_FloatNumber;

/*		x2 = x*cos(rz)+y*sin(rz);
		y2 = -x*sin(rz)+y*cos(rz);
		x = x2;
		y = y2;
		y2 = y*cos(rx)+z*sin(rx);
		z2 = -y*sin(rx)+z*cos(rx);
		y = y2;
		z = z2;
		x2 = x*cos(ry)-z*sin(ry);
		z2 = x*sin(ry)+z*cos(ry);
		x = x2;
		z = z2;
*/
		vList[i].v[0] = x;
		vList[i].v[1] = y;
		vList[i].v[2] = z;
	}
	TK_BeyondRequire(TK_C_FACE, TK_LIST);
	for (i = 0; i < triCount; i++)
	{
		TK_BeyondRequire(TK_C_FACE, TK_INTNUMBER);
		if (tk_IntNumber != i)
		{
			COM_Error("File '%s', line %d:\nTriangle index mismatch.\n",
						tk_SourceName, tk_Line);
		}
		for (j = 0; j < 3; j++)
		{
			TK_BeyondRequire(TK_IDENTIFIER, TK_COLON);
			TK_FetchRequire(TK_INTNUMBER);
			if (tk_IntNumber >= vertexCount)
			{
				COM_Error("File '%s', line %d:\nVertex number"
					" > vertexCount: %d\n", tk_SourceName, tk_Line,
					tk_IntNumber);
			}
			tList[i].verts[2-j][0] = vList[tk_IntNumber].v[0];
			tList[i].verts[2-j][1] = vList[tk_IntNumber].v[1];
			tList[i].verts[2-j][2] = vList[tk_IntNumber].v[2];
		}

/*		printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
			"  v2: %f, %f, %f\n", i,
			tList[i].verts[0][0],
			tList[i].verts[0][1],
			tList[i].verts[0][2],
			tList[i].verts[1][0],
			tList[i].verts[1][1],
			tList[i].verts[1][2],
			tList[i].verts[2][0],
			tList[i].verts[2][1],
			tList[i].verts[2][2]);
*/
	}
}

//==========================================================================
//
// LoadHTR
//
//==========================================================================

static void LoadHTR(char *fileName, triangle_t **triList, int *triangleCount)
{
	int		i, j;
	int		vertexCount;
	int		vertexNum;
	struct vList_s
	{
		float v[3];
	} *vList;
	int		triCount;
	float	origin[3];
	triangle_t	*tList;
	float	x, y, z;
	float	x2, y2, z2;
	float	rx, ry, rz;

	TK_Init();
	TK_OpenSource(fileName);

	TK_Beyond(TK_C_HEXEN);
	TK_Beyond(TK_C_TRIANGLES);
	TK_BeyondRequire(TK_C_VERSION, TK_INTNUMBER);
	if (tk_IntNumber != 1)
	{
		COM_Error("Unsupported version (%d) in file %s\n", tk_IntNumber,
							InputFileName);
	}

	// Get vertex count
	TK_BeyondRequire(TK_VERTICES, TK_INTNUMBER);
	vertexCount = tk_IntNumber;
	vList = (struct vList_s *) SafeMalloc(vertexCount * sizeof(vList[0]));

	// Get triangle count
	TK_BeyondRequire(TK_FACES, TK_INTNUMBER);
	triCount = tk_IntNumber;
	if (triCount >= MAXTRIANGLES)
	{
		COM_Error("Too many triangles in file %s\n", InputFileName);
	}
	*triangleCount = triCount;
	tList = (triangle_t *) SafeMalloc(MAXTRIANGLES * sizeof(triangle_t));
	*triList = tList;

	// Get origin
	TK_Beyond(TK_ORIGIN);
	TK_Require(TK_FLOATNUMBER);
	origin[0] = tk_FloatNumber;
	TK_FetchRequire(TK_FLOATNUMBER);
	origin[1] = tk_FloatNumber;
	TK_FetchRequire(TK_FLOATNUMBER);
	origin[2] = tk_FloatNumber;

	//rx = 90.0/360.0*2.0*MY_PI;
	rx =(float)(FixHTRRotateX/360.0*2.0*MY_PI);
	ry =(float)(FixHTRRotateY/360.0*2.0*MY_PI);
	rz =(float)(FixHTRRotateZ/360.0*2.0*MY_PI);

	// Get vertex list
	for (i = 0; i < vertexCount; i++)
	{
		TK_FetchRequire(TK_VERTEX);
		TK_FetchRequire(TK_FLOATNUMBER);
		x = tk_FloatNumber-origin[0];
		TK_FetchRequire(TK_FLOATNUMBER);
		y = tk_FloatNumber-origin[1];
		TK_FetchRequire(TK_FLOATNUMBER);
		z = tk_FloatNumber-origin[2];

		x += FixHTRTranslateX;
		y += FixHTRTranslateY;
		z += FixHTRTranslateZ;

		y2 = (float)(y*cos(rx)-z*sin(rx));
		z2 = (float)(y*sin(rx)+z*cos(rx));
		y = y2;
		z = z2;
		x2 = (float)(x*cos(ry)+z*sin(ry));
		z2 = (float)(-x*sin(ry)+z*cos(ry));
		x = x2;
		z = z2;
		x2 = (float)(x*cos(rz)-y*sin(rz));
		y2 = (float)(x*sin(rz)+y*cos(rz));
		x = x2;
		y = y2;

		vList[i].v[0] = x;
		vList[i].v[1] = y;
		vList[i].v[2] = z;
	}

	// Get face list
	for (i = 0; i < triCount; i++)
	{
		TK_FetchRequire(TK_FACE);
		TK_FetchRequire(TK_LPAREN);
		for (j = 0; j < 3; j++)
		{
			TK_FetchRequire(TK_INTNUMBER);
			vertexNum = tk_IntNumber-1;
			if (vertexNum >= vertexCount)
			{
				COM_Error("File '%s', line %d:\nVertex number"
					" >= vertexCount: %d\n", tk_SourceName, tk_Line,
					tk_IntNumber);
			}
			tList[i].verts[2-j][0] = vList[vertexNum].v[0];
			tList[i].verts[2-j][1] = vList[vertexNum].v[1];
			tList[i].verts[2-j][2] = vList[vertexNum].v[2];
		}
		TK_FetchRequire(TK_RPAREN);

/*		printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
			"  v2: %f, %f, %f\n", i,
			tList[i].verts[0][0],
			tList[i].verts[0][1],
			tList[i].verts[0][2],
			tList[i].verts[1][0],
			tList[i].verts[1][1],
			tList[i].verts[1][2],
			tList[i].verts[2][0],
			tList[i].verts[2][1],
			tList[i].verts[2][2]);
*/
	}
}

//==========================================================================
//
// Load3DS
//
//==========================================================================
#if 0	/* 3DS stuff is unused. don't even know whether it's working */
static void Load3DS(FILE *input, triangle_t **triList, int *triangleCount)
{
	unsigned int i, j;
	qboolean stop;
	qboolean foundVertexList;
	unsigned int chunkType, chunkPos, chunkSize;
	unsigned int editChunkSize, editChunkPos;
	unsigned int objectChunkSize, objectChunkPos;
	unsigned int meshChunkSize, meshChunkPos;
	unsigned int vertex;
	unsigned int vertexCount;
	struct vList_s
	{
		float v[3];
	} *vList;
	unsigned int triCount;
	triangle_t	*tList;

	InputFile = input;
	if (ReadShort() != _3DS_MAIN3DS)
	{
		_3DSError("Missing 3DS main chunk header.\n");
	}
	SeekTo(16);
	if (ReadShort() != _3DS_EDIT3DS)
	{
		_3DSError("Missing 3DS edit chunk header.\n");
	}

	editChunkSize = ReadLong();
	editChunkPos = FilePosition()-6;
	stop = false;
	while (stop == false)
	{
		chunkPos = FilePosition();
		chunkType = ReadShort();
		switch (chunkType)
		{
			case _3DS_EDIT_UNKNW01:
			case _3DS_EDIT_UNKNW02:
			case _3DS_EDIT_UNKNW03:
			case _3DS_EDIT_UNKNW04:
			case _3DS_EDIT_UNKNW05:
			case _3DS_EDIT_UNKNW06:
			case _3DS_EDIT_UNKNW07:
			case _3DS_EDIT_UNKNW08:
			case _3DS_EDIT_UNKNW09:
			case _3DS_EDIT_UNKNW10:
			case _3DS_EDIT_UNKNW11:
			case _3DS_EDIT_UNKNW12:
			case _3DS_EDIT_UNKNW13:
			case _3DS_EDIT_MATERIAL:
			case _3DS_EDIT_VIEW1:
			case _3DS_EDIT_BACKGR:
			case _3DS_EDIT_AMBIENT:
				SeekTo(chunkPos+ReadLong());
				break;
			case _3DS_EDIT_OBJECT:
				stop = true;
			default:
				break;
		}
		if (FilePosition()-editChunkPos >= editChunkSize)
		{
			_3DSError("Couldn't find OBJECT chunk.\n");
		}
	}

	objectChunkSize = ReadLong();
	objectChunkPos = FilePosition()-6;
	SkipName();
	stop = false;
	while (stop == false)
	{
		chunkPos = FilePosition();
		chunkType = ReadShort();
		switch (chunkType)
		{
			case _3DS_OBJ_UNKNWN01:
			case _3DS_OBJ_UNKNWN02:
			case _3DS_OBJ_LIGHT:
			case _3DS_OBJ_CAMERA:
				SeekTo(chunkPos+ReadLong());
				break;
			case _3DS_OBJ_TRIMESH:
				stop = true;
			default:
				break;
		}
		if (FilePosition()-objectChunkPos >= objectChunkSize)
		{
			_3DSError("Couldn't find TRIMESH chunk.\n");
		}
	}

	meshChunkSize = ReadLong();
	meshChunkPos = FilePosition()-6;
	stop = false;
	foundVertexList = false;
	while (stop == false)
	{
		chunkPos = FilePosition();
		chunkType = ReadShort();
		switch (chunkType)
		{
			case _3DS_TRI_FACEL2:
			case _3DS_TRI_VISIBLE:
				SeekTo(chunkPos+ReadLong());
				break;
			case _3DS_TRI_VERTEXL:
				chunkSize = ReadLong();
				vertexCount = ReadShort();
				vList = (struct vList_s *) SafeMalloc(vertexCount * sizeof(vList[0]));
				for (i = 0; i < vertexCount; i++)
				{
					vList[i].v[0] = ReadFloat();
					vList[i].v[1] = -ReadFloat();
					vList[i].v[2] = ReadFloat();
				}
				SeekTo(chunkPos+chunkSize);
				foundVertexList = true;
				break;
			case _3DS_TRI_FACEL1:
				chunkSize = ReadLong();
				triCount = ReadShort();
				if (triCount >= MAXTRIANGLES)
				{
					COM_Error("Too many triangles in file %s\n",
								InputFileName);
				}
				*triangleCount = triCount;
				tList = (triangle_t *) SafeMalloc(MAXTRIANGLES * sizeof(triangle_t));
				*triList = tList;
				for (i = 0; i < triCount; i++)
				{
					for (j = 0; j < 3; j++)
					{
						vertex = ReadShort();
						tList[i].verts[j][0] = vList[vertex].v[0];
						tList[i].verts[j][1] = vList[vertex].v[1];
						tList[i].verts[j][2] = vList[vertex].v[2];
					}
					ReadShort(); // Skip face flags
				}
				stop = true;
				break;
			default:
				break;
		}
		if (FilePosition()-meshChunkPos >= meshChunkSize)
		{
			if (foundVertexList == false)
			{
				_3DSError("Couldn't find TRI_VERTEXL chunk.\n");
			}
			else
			{
				_3DSError("Couldn't find TRI_FACEL1 chunk.\n");
			}
		}
	}
}

static void _3DSError(char *message)
{
	COM_Error(message);
}

static float ReadFloat(void)
{
	float	t;

	fread(&t, sizeof(float), 1, InputFile);
	return t;
}

static unsigned long ReadLong(void)
{
	unsigned long	t;

	fread(&t, sizeof(unsigned long), 1, InputFile);
	return t;
}

static unsigned short ReadShort(void)
{
	unsigned short	t;

	fread(&t, sizeof(unsigned short), 1, InputFile);
	return t;
}

static unsigned char ReadByte(void)
{
	unsigned char	t;

	fread(&t, sizeof(unsigned char), 1, InputFile);
	return t;
}

static void SkipName(void)
{
	int		i;
	int		c;

	for (i = 0, c = 1; i < 12 && c != 0; i++)
	{
		c = ReadByte();
	}
}

static void SeekTo(int position)
{
	fseek(InputFile, position, SEEK_SET);
}

static unsigned long FilePosition(void)
{
	return ftell(InputFile);
}
#endif	// end of unused 3DS stuff

//==========================================================================
//
// LoadTRI
//
//==========================================================================

static void LoadTRI(FILE *input, triangle_t **triList, int *triangleCount)
{
	int		i, j, k;
	char	text[256];
	int		count;
	int		magic;
	tf_triangle	tri;
	triangle_t	*ptri;
	int		exitpattern;
	float		t;
	union {
		float	_f;
		int	_i;
	} start;

	t = -FLOAT_START;
	*((unsigned char *)&exitpattern + 0) = *((unsigned char *)&t + 3);
	*((unsigned char *)&exitpattern + 1) = *((unsigned char *)&t + 2);
	*((unsigned char *)&exitpattern + 2) = *((unsigned char *)&t + 1);
	*((unsigned char *)&exitpattern + 3) = *((unsigned char *)&t + 0);

	fread(&magic, sizeof(int), 1, input);
	if (BigLong(magic) != TRI_MAGIC)
	{
		COM_Error("Bad .TRI file: %s\n", InputFileName);
	}

	ptri = (triangle_t *) SafeMalloc(MAXTRIANGLES * sizeof(triangle_t));
	*triList = ptri;

	count = 0; // make static analyzers happy
	while (feof(input) == 0)
	{
		fread(&start._f, sizeof(float), 1, input);
		start._i = BigLong(start._i);

		if (start._i != exitpattern)
		{
			if (start._f == FLOAT_START)
			{ // Start of an object or group of objects
				i = -1;
				do
				{
					++i;
					fread(&(text[i]), sizeof(char), 1, input);
				} while (text[i] != '\0');
				//fprintf(stdout,"OBJECT START: %s\n", text);

				fread(&count, sizeof(int), 1, input);
				count = BigLong(count);
				if (count != 0)
				{
					//fprintf(stdout,"NUMBER OF TRIANGLES: %d\n", count);
					i = -1;
					do
					{
						++i;
						fread(&(text[i]), sizeof( char ), 1, input);
					} while (text[i] != '\0');
					//fprintf(stdout,"  Object texture name: '%s'\n", text);
				}
			}
			else if (start._f == FLOAT_END)
			{
				i = -1;
				do
				{
					++i;
					fread(&(text[i]), sizeof(char), 1, input);
				} while (text[i] != '\0');
				//fprintf(stdout,"OBJECT END: %s\n", text);
				continue;
			}
		}

		// Read the triangles
		for (i = 0; i < count; ++i)
		{
			fread(&tri, sizeof(tf_triangle), 1, input);
			ByteSwapTri(&tri);
			for (j = 0; j < 3; j++)
			{
				for (k = 0; k < 3; k++)
				{
					ptri->verts[j][k] = tri.pt[j].p.v[k];
				}
			}

/*			printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
				"  v2: %f, %f, %f\n", i,
				ptri->verts[0][0],
				ptri->verts[0][1],
				ptri->verts[0][2],
				ptri->verts[1][0],
				ptri->verts[1][1],
				ptri->verts[1][2],
				ptri->verts[2][0],
				ptri->verts[2][1],
				ptri->verts[2][2]);
*/
			ptri++;
			if ((ptri - *triList) >= MAXTRIANGLES)
			{
				COM_Error("Error: too many triangles; increase MAXTRIANGLES\n");
			}
		}
	}
	*triangleCount = ptri - *triList;
}

//==========================================================================
//
// ByteSwapTri
//
//==========================================================================

static void ByteSwapTri(tf_triangle *tri)
{
	int		i;

	for (i = 0; i < (int) sizeof(tf_triangle)/4; i++)
	{
		((int *)tri)[i] = BigLong(((int *)tri)[i]);
	}
}

