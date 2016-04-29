/*
 * genmodel.c
 * $Id: genmodel.c,v 1.16 2009-01-24 23:41:28 sezero Exp $
 *
 * Generates a .mdl file from a base frame, a texture bitmap,
 * and a series of frames.
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

#ifdef _MSC_VER
#pragma warning(disable: 4244)
#endif

// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "pathutil.h"
#include "scriplib.h"
#include "mathlib.h"
#include "q_endian.h"
#include "byteordr.h"
#include "loadtri.h"
#include "genmodel.h"
#include "qdir.h"

// MACROS ------------------------------------------------------------------

#define VERSION_TEXT	"1.18"

#define MAXVERTS		2048
#define MAXFRAMES		512
#define MAXSKINS		100

#define NUMVERTEXNORMALS	162

#define SKINPAGE_WIDTH	640
#define SKINPAGE_HEIGHT	480
#define SKINPAGE_SIZE	(SKINPAGE_WIDTH*SKINPAGE_HEIGHT)

// Must match definitions in genskin.c
#define SCALE_ADJUST_FACTOR	0.96
#define ENCODED_WIDTH_X		192
#define ENCODED_WIDTH_Y		475
#define ENCODED_HEIGHT_X	228
#define ENCODED_HEIGHT_Y	475

// TYPES -------------------------------------------------------------------

typedef struct {
	aliasframetype_t	type;		// single frame or group of frames
	void		*pdata;			// either a daliasframe_t or group info
	float		interval;		// only used for frames in groups
	int			numgroupframes;	// only used by group headers
	char		name[16];
} aliaspackage_t;

typedef struct {
	aliasskintype_t		type;		// single skin or group of skiins
	void		*pdata;			// either a daliasskinframe_t or group info
	float		interval;		// only used for skins in groups
	int			numgroupskins;	// only used by group headers
} aliasskinpackage_t;

typedef struct {
	int		numnormals;
	float	normals[40][3];
} vertexnormals;

typedef struct {
	vec3_t		v;
	int			lightnormalindex;
} trivert_t;

typedef struct
{
	byte	tag;
	byte	version;
	byte	encoding;
	byte	pixelBits;
	signed short	xMin;
	signed short	yMin;
	signed short	xMax;
	signed short	yMax;
	signed short	horzRes;
	signed short	vertRes;
	byte	palette[48];
	byte	pad;
	byte	planes;
	signed short	lineBytes;
	signed short	palType;
	unsigned short	vidSizeX;
	unsigned short	vidSizeY;
	byte	reserved[54];
	byte	data;
} pcx_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ClearModel(void);
static void WriteModel(void);
static void ReadModel(const char *FileName);
static void ParseScript(void);
static void LoadPCXSkin(const char *filename, byte **buffer);
static int ExtractNumber(byte *pic, int x, int y);
static int ExtractDigit(byte *pic, int x, int y);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static trivert_t	verts[MAXFRAMES][MAXVERTS];
static newmdl_t	model;

static char	file1[1024];
static char	file2[1024];
static char	skinname[1024];
static char	qbasename[1024];
static float	scale, scale_up = 1.0;

static float	ScaleWidth;
static float	ScaleHeight;

static vec3_t	mins, maxs;
static vec3_t	framesmins, framesmaxs;
static vec3_t	adjust;

static aliaspackage_t		frames[MAXFRAMES];
static aliasskinpackage_t	skins[MAXSKINS];

static vec3_t	baseverts[MAXVERTS];
static stvert_t	stverts[MAXVERTS];
static dnewtriangle_t	triangles[MAXTRIANGLES];
static int	degenerate[MAXTRIANGLES];

static char	cdpartial[256];
static char	cddir[256];

static int	framecount, skincount;
static qboolean	cdset;
static int	degeneratetris;
static int	firstframe = 1;
static float	totsize, averagesize;

static qboolean	DoOpts = false;
static qboolean	ModelReadIn = false;
static int	original_size = 0;

static vertexnormals	vnorms[MAXVERTS];

static double	avertexnormals[NUMVERTEXNORMALS][3] =
{
#include "anorms.h"
};

static trivertx_t	tarray[MAXVERTS];

static char	outname[1024];

// Must match definitions in genskin.c
static const char *DigitDefs[] =
{
	" *** *  *** * ***  * *** ",
	"   *   **    *    *    * ",
	"****     * *** *    *****",
	"****     * ***     ***** ",
	"  **  * * *  * *****   * ",
	"**** *    ****     ***** ",
	" *** *    **** *   * *** ",
	"*****    *   *   *    *  ",
	" *** *   * *** *   * *** ",
	" *** *   * ****    * *** "
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// main
//
//==========================================================================

int main(int argc, char **argv)
{
	int		i, j;
	char	path[1024], bakname[1024];

	printf("GENMODEL Version " VERSION_TEXT " (" __DATE__ ")\n\n");

	if (argc == 1) {
	_usage:
		printf("usage: genmodel [-opt] [-archive path] model.hc\n");
		exit (1);
	}

	ValidateByteorder ();

	path[0] = 0;

	myargc = argc;
	myargv = argv;

	i = CheckParm("-archive");
	if (i != 0)
	{
		if (i >= argc - 1) goto _usage;
		archive = true;
		q_strlcpy(archivedir, argv[++i], sizeof(archivedir));
		printf("Archiving source to: %s\n", archivedir);
	}
	j = CheckParm("-opt");
	if (j != 0)
		DoOpts = true;
	if (i < j) i = j;
	if (i + 1 > argc - 1)
		goto _usage;

	q_strlcpy(path, argv[i + 1], sizeof(path));
	SetQdirFromPath("");		/* (path); */

	// Init
	for (i = 0; i < 3; i++)
	{
		framesmins[i] = 9999999;
		framesmaxs[i] = -9999999;
	}
	ClearModel();

	q_strlcpy(outname, path, sizeof(outname));

	i = strlen(path);
	if (i > 4 && q_strcasecmp(&path[i - 4],".mdl") == 0)
	{
		q_strlcpy(outname, path, sizeof(outname));
		ReadModel(path);

		q_strlcpy(bakname, path, sizeof(bakname));
		bakname[i - 4] = 0;
		DefaultExtension(bakname, ".bak", sizeof(bakname));
		if (Q_rename(path, bakname))
			COM_Error("Could not rename file!\n");
	}
	else
	{
		DefaultExtension(path, ".hc", sizeof(path));
		// Load the .hc script file
		LoadScriptFile(path);

		// Parse it
		ParseScript();
	}

	WriteModel();

	return 0;
}

//==========================================================================
//
// ClearModel
//
//==========================================================================

static void ClearModel(void)
{
	memset(&model, 0, sizeof(model));
	model.synctype = ST_RAND; // default
	framecount = skincount = 0;

	scale = 0;
	scale_up = 1.0;

	VectorClear (adjust);
	VectorClear (mins);
	VectorClear (maxs);
	VectorClear (framesmins);
	VectorClear (framesmaxs);

	degeneratetris = 0;
	cdset = false;
	firstframe = 1;
	totsize = 0.0;
}

//==========================================================================
//
// ReadFrame
//
//==========================================================================

static void ReadFrame (FILE *FH, int framenum)
{
	int			j, k;
	trivert_t	*pframe;
	daliasframe_t	aframe;

	SafeRead (FH, &aframe, sizeof (aframe));
	SafeRead (FH, &tarray[0], model.numverts * sizeof(tarray[0]));

	pframe = verts[framenum];
	memcpy (frames[framenum].name, aframe.name, sizeof(aframe.name));
	frames[framenum].pdata = pframe;

	for (j = 0 ; j < model.numverts ; j++)
	{
		// all of these are byte values, so no need to deal with endianness
		pframe[j].lightnormalindex = tarray[j].lightnormalindex;

		for (k = 0 ; k < 3 ; k++)
		{	// scale to byte values & min/max check
//			float v;
//			byte b;

			pframe[j].v[k] = (tarray[j].v[k] * model.scale[k]) + model.scale_origin[k] + 0.001;

//			v = (pframe[j].v[k] - model.scale_origin[k]) / model.scale[k];
//			b = v;
		}
	}
}

//==========================================================================
//
// WriteFrame
//
//==========================================================================

static void WriteFrame (FILE *modelouthandle, int framenum)
{
	int			j, k;
	trivert_t	*pframe;
	daliasframe_t	aframe;
	float		v;

	pframe = verts[framenum];

	memcpy (aframe.name, frames[framenum].name, sizeof(aframe.name));

	for (j = 0 ; j < 3 ; j++)
	{
		aframe.bboxmin.v[j] = 255;
		aframe.bboxmax.v[j] = 0;
	}

	for (j = 0 ; j < model.numverts ; j++)
	{
	// all of these are byte values, so no need to deal with endianness
		tarray[j].lightnormalindex = pframe[j].lightnormalindex;

		if (tarray[j].lightnormalindex > NUMVERTEXNORMALS)
			COM_Error ("invalid lightnormalindex %d\n", tarray[j].lightnormalindex);

		for (k = 0 ; k < 3 ; k++)
		{
		// scale to byte values & min/max check
			v = (pframe[j].v[k] - model.scale_origin[k]) / model.scale[k];

			tarray[j].v[k] = v;

			if (tarray[j].v[k] < aframe.bboxmin.v[k])
			{
				aframe.bboxmin.v[k] = tarray[j].v[k];
			}
			if (tarray[j].v[k] > aframe.bboxmax.v[k])
			{
				aframe.bboxmax.v[k] = tarray[j].v[k];
			}
		}
	}

	SafeWrite (modelouthandle, &aframe, sizeof (aframe));
	SafeWrite (modelouthandle, &tarray[0], model.numverts * sizeof(tarray[0]));
}

//==========================================================================
//
// WriteGroupBBox
//
//==========================================================================

static void WriteGroupBBox (FILE *modelouthandle, int numframes, int curframe)
{
	int			i, j, k;
	daliasgroup_t	dagroup;
	trivert_t	*pframe;

	dagroup.numframes = LittleLong (numframes);

	for (i = 0 ; i < 3 ; i++)
	{
		dagroup.bboxmin.v[i] = 255;
		dagroup.bboxmax.v[i] = 0;
	}

	for (i = 0 ; i < numframes ; i++)
	{
		pframe = (trivert_t *)frames[curframe].pdata;

		for (j = 0 ; j < model.numverts ; j++)
		{
			for (k = 0 ; k < 3 ; k++)
			{
			// scale to byte values & min/max check
				tarray[j].v[k] = (pframe[j].v[k] - model.scale_origin[k]) / model.scale[k];
				if (tarray[j].v[k] < dagroup.bboxmin.v[k])
					dagroup.bboxmin.v[k] = tarray[j].v[k];
				if (tarray[j].v[k] > dagroup.bboxmax.v[k])
					dagroup.bboxmax.v[k] = tarray[j].v[k];
			}
		}

		curframe++;
	}

	SafeWrite (modelouthandle, &dagroup, sizeof(dagroup));
}

//==========================================================================
//
// WriteModelFile
//
//==========================================================================

static void WriteModelFile (FILE *modelouthandle)
{
	int			i, curframe, curskin;
	float		dist[3];
	mdl_t		modeltemp;
	newmdl_t	newmodeltemp;

// Calculate the bounding box for this model
	if (!ModelReadIn)
	{
		for (i = 0 ; i < 3 ; i++)
		{
			printf ("framesmins[%d]: %f, framesmaxs[%d]: %f\n", i, framesmins[i], i, framesmaxs[i]);
			if (fabs (framesmins[i]) > fabs (framesmaxs[i]))
				dist[i] = framesmins[i];
			else
				dist[i] = framesmaxs[i];

			model.scale[i] = (framesmaxs[i] - framesmins[i]) / 255.9;
			model.scale_origin[i] = framesmins[i];
		}

		model.boundingradius = sqrt(dist[0] * dist[0] +
						dist[1] * dist[1] +
						dist[2] * dist[2]);
	}

//
// write out the model header
//
	modeltemp.ident = LittleLong (IDPOLYHEADER);
	modeltemp.version = LittleLong (ALIAS_VERSION);
	newmodeltemp.ident = LittleLong (RAPOLYHEADER);
	newmodeltemp.version = LittleLong (ALIAS_NEWVERSION);
	newmodeltemp.boundingradius = modeltemp.boundingradius = LittleFloat (model.boundingradius);

	for (i = 0 ; i < 3 ; i++)
	{
		newmodeltemp.scale[i] = modeltemp.scale[i] = LittleFloat (model.scale[i]);
		newmodeltemp.scale_origin[i] = modeltemp.scale_origin[i] = LittleFloat (model.scale_origin[i]);
		newmodeltemp.eyeposition[i] = modeltemp.eyeposition[i] = LittleFloat (model.eyeposition[i] + adjust[i]);
	}

	newmodeltemp.flags = modeltemp.flags = LittleLong (model.flags);
	newmodeltemp.numskins = modeltemp.numskins = LittleLong (model.numskins);
	newmodeltemp.skinwidth = modeltemp.skinwidth = LittleLong (model.skinwidth);
	newmodeltemp.skinheight = modeltemp.skinheight = LittleLong (model.skinheight);
	newmodeltemp.numverts = modeltemp.numverts = LittleLong (model.numverts);
	newmodeltemp.numtris = modeltemp.numtris = LittleLong (model.numtris - degeneratetris);
	newmodeltemp.numframes = modeltemp.numframes = LittleLong (model.numframes);
	newmodeltemp.synctype = modeltemp.synctype = (synctype_t) LittleLong (model.synctype);

	if (!ModelReadIn)
	{
		averagesize = totsize / model.numtris;
	}
	else
	{
		averagesize = model.size;
	}
	newmodeltemp.size = modeltemp.size = LittleFloat (averagesize);

	// new stuff
	newmodeltemp.num_st_verts = LittleLong (model.num_st_verts);

	if (DoOpts)
	{
		SafeWrite (modelouthandle, &newmodeltemp, sizeof(newmodeltemp));
	}
	else
	{
		SafeWrite (modelouthandle, &modeltemp, sizeof(modeltemp));
	}
//
// write out the skins
//
	curskin = 0;

	for (i = 0 ; i < model.numskins ; i++)
	{
		SafeWrite (modelouthandle, &skins[curskin].type, sizeof(skins[curskin].type));
		SafeWrite (modelouthandle, skins[curskin].pdata, model.skinwidth * model.skinheight);

		curskin++;
	}

//
// write out the base model (the s & t coordinates for the vertices)
//
	for (i = 0 ; i < model.num_st_verts ; i++)
	{
		if (stverts[i].onseam == 3)
		{
			stverts[i].onseam = LittleLong (ALIAS_ONSEAM);
		}
		else
		{
			stverts[i].onseam = LittleLong (0);
		}

		stverts[i].s = LittleLong (stverts[i].s);
		stverts[i].t = LittleLong (stverts[i].t);
	}

	SafeWrite (modelouthandle, stverts, model.num_st_verts * sizeof(stverts[0]));

//
// write out the triangles
//
	if (DoOpts)
	{
		for (i = 0 ; i < model.numtris ; i++)
		{
			int			j;
			dnewtriangle_t	tri;

			if (!degenerate[i])
			{
				tri.facesfront = LittleLong (triangles[i].facesfront);

				for (j = 0 ; j < 3 ; j++)
				{
					tri.vertindex[j] = LittleShort (triangles[i].vertindex[j]);
					tri.stindex[j] = LittleShort (triangles[i].stindex[j]);
				}

				SafeWrite (modelouthandle, &tri, sizeof(tri));
			}
		}
	}
	else
	{
		for (i = 0 ; i < model.numtris ; i++)
		{
			int			j;
			dtriangle_t	tri;

			if (!degenerate[i])
			{
				tri.facesfront = LittleLong (triangles[i].facesfront);

				for (j = 0 ; j < 3 ; j++)
				{
					tri.vertindex[j] = LittleLong (triangles[i].vertindex[j]);
				}

				SafeWrite (modelouthandle,&tri,sizeof(tri));
			}
		}
	}
//
// write out the frames
//
	curframe = 0;

	for (i = 0 ; i < model.numframes ; i++)
	{
		SafeWrite (modelouthandle, &frames[curframe].type, sizeof(frames[curframe].type));

		if (frames[curframe].type == ALIAS_SINGLE)
		{
		//
		// single (non-grouped) frame
		//
			WriteFrame (modelouthandle, curframe);
			curframe++;
		}
		else
		{
			int		j, numframes, groupframe;
			float	totinterval;

			groupframe = curframe;
			curframe++;
			numframes = frames[groupframe].numgroupframes;

		//
		// set and write the group header
		//
			WriteGroupBBox (modelouthandle, numframes, curframe);

		//
		// write the interval array
		//
			totinterval = 0.0;

			for (j = 0 ; j < numframes ; j++)
			{
				daliasinterval_t	temp;

				totinterval += frames[groupframe+1+j].interval;
				temp.interval = LittleFloat (totinterval);

				SafeWrite (modelouthandle, &temp, sizeof(temp));
			}

			for (j = 0 ; j < numframes ; j++)
			{
				WriteFrame (modelouthandle, curframe);
				curframe++;
			}
		}
	}
}

static void OptimizeVertices(void)
{
	qboolean		vert_used[MAXVERTS];
	short			vert_replacement[MAXVERTS];
	int				i, j, k;
	trivert_t		*in;
	qboolean		Found;
	int				num_unique;

	printf("Optimizing vertices...");

	memset(vert_used, 0, sizeof(vert_used));
	memset(vert_replacement, 0, sizeof(vert_replacement)); // make static analyzers happy
	num_unique = 0;

	// search for common points among all the frames
	for (i = 0 ; i < model.numframes ; i++)
	{
		in = (trivert_t *) frames[i].pdata;

		for (j = 0; j < model.numverts; j++)
		{
			for (k = 0, Found = false; k < j; k++)
			{
			// starting from the beginning always ensures
			// vert_replacement points to the first point
			// in the array
				if (in[j].v[0] == in[k].v[0] &&
					in[j].v[1] == in[k].v[1] &&
					in[j].v[2] == in[k].v[2])
				{
					Found = true;
					vert_replacement[j] = k;
					break;
				}
			}

			if (!Found)
			{
				if (!vert_used[j])
				{
					num_unique++;
				}
				vert_used[j] = true;
			}
		}
	}

#if 0 /* this is a dead loop because of the commented out code. */
	// recompute the light normals
	for (i = 0 ; i < model.numframes ; i++)
	{
		in = (trivert_t *) frames[i].pdata;

		for (j = 0; j < model.numverts; j++)
		{
			if (!vert_used[j])
			{
				k = vert_replacement[j];

//				VectorAdd (inv[j].vnorm.normalsum, in->v[k].vnorm.normalsum, in->v[k].vnorm.normalsum);
//				in->v[k].vnorm.numnormals += in->v[j].vnorm.numnormals++;
			}
		}

/*		for (j = 0 ; j < model.numverts ; j++)
		{
			vec3_t	v;
			float	maxdot;
			int		maxdotindex;
			int		c;

			c = in->v[j].vnorm.numnormals;
			if (!c)
				COM_Error ("Vertex with no triangles attached");

			VectorScale (in->v[j].vnorm.normalsum, 1.0/c, v);
			VectorNormalize (v, v);

			maxdot = -999999.0;
			maxdotindex = -1;

			for (k = 0 ; k < NUMVERTEXNORMALS ; k++)
			{
				float	dot;

				dot = DotProduct (v, avertexnormals[k]);
				if (dot > maxdot)
				{
					maxdot = dot;
					maxdotindex = k;
				}
			}

			in->v[j].lightnormalindex = maxdotindex;
		}*/
	}
#endif

	// create substitution list
	num_unique = 0;
	for (i = 0; i < model.numverts; i++)
	{
		if (vert_used[i])
		{
			vert_replacement[i] = num_unique;
			num_unique++;
		}
		else
		{
			vert_replacement[i] = vert_replacement[vert_replacement[i]];
		}
	}

	// substitute
	for (i = 0 ; i < model.numframes ; i++)
	{
		in = (trivert_t *) frames[i].pdata;

		for (j = 0; j < model.numverts; j++)
		{
			in[vert_replacement[j]] = in[j];
		}
	}

	for (i = 0 ; i < model.numtris ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
		{
			triangles[i].vertindex[j] = vert_replacement[triangles[i].vertindex[j]];
		}
	}

/*	for (i = 0; i < numcommands; i++)
	{
		j = commands[i];
		if (!j)
			continue;

		j = abs(j);
		for (i++; j; j--, i+=3)
		{
			commands[i+2] = vert_replacement[commands[i+2]];
		}
		i--;
	}
*/
	printf("Reduced by %d\n",model.numverts - num_unique);

	model.numverts = num_unique;
}

//==========================================================================
//
// WriteModel
//
//==========================================================================

static void WriteModel (void)
{
	FILE		*modelouthandle;
//
// write the model output file
//
	if (!framecount)
	{
		printf ("no frames grabbed, no file generated\n");
		return;
	}

	if (!skincount)
		COM_Error ("frames with no skins\n");

	StripExtension (outname);
	q_strlcat(outname, ".mdl", sizeof(outname));

	if (DoOpts)
	{
		OptimizeVertices();
	}

	printf ("---------------------\n");
	printf ("writing %s:\n", outname);
	modelouthandle = SafeOpenWrite (outname);

	WriteModelFile (modelouthandle);

	printf ("%4d frame(s)\n", model.numframes);
	printf ("%4d ungrouped frame(s), including group headers\n", framecount);
	printf ("%4d skin(s)\n", model.numskins);
	printf ("%4d degenerate triangles(s) removed\n", degeneratetris);
	printf ("%4d triangles emitted\n", model.numtris - degeneratetris);
	printf ("pixels per triangle %f\n", averagesize);
	printf ("%4d numverts\n", model.numverts);

	if (ModelReadIn)
	{
		printf ("file size: %d (Saved %d)\n", (int)ftell (modelouthandle), (int)(original_size - ftell(modelouthandle)) );
	}
	else
	{
		printf ("file size: %d\n", (int)ftell (modelouthandle) );
	}
	printf ("---------------------\n");

	fclose (modelouthandle);

	ClearModel ();
}

static void ReadModel(const char *FileName)
{
	FILE	*FH;
	mdl_t	mdl;
	int		i, curframe;

	FH = fopen(FileName,"rb");
	if (!FH)
		COM_Error ("Could not open model %s\n",FileName);

	SafeRead(FH, (void *)&mdl, sizeof(mdl));

	if (mdl.ident != LittleLong (IDPOLYHEADER) ||
		mdl.version != LittleLong (ALIAS_VERSION))
	{
		COM_Error ("Invalid model version for file %s\n",FileName);
	}

	model.boundingradius = LittleFloat (mdl.boundingradius);

	for (i = 0 ; i < 3 ; i++)
	{
		model.scale[i] = LittleFloat (mdl.scale[i]);
		model.scale_origin[i] = LittleFloat (mdl.scale_origin[i]);

		// rjr - adjust will default to 0
		model.eyeposition[i] = LittleFloat (mdl.eyeposition[i] - adjust[i]);
	}

	model.flags = LittleLong (mdl.flags);
	skincount = model.numskins = LittleLong (mdl.numskins);
	model.skinwidth = LittleLong (mdl.skinwidth);
	model.skinheight = LittleLong (mdl.skinheight);
	model.numverts = LittleLong (mdl.numverts);
	model.numtris = LittleLong (mdl.numtris - degeneratetris);
	framecount = model.numframes = LittleLong (mdl.numframes);
	model.synctype = (synctype_t) LittleLong (mdl.synctype);

	model.size = LittleFloat (mdl.size);

	model.num_st_verts = model.numverts;

	// read in the skins
	for (i = 0 ; i < model.numskins ; i++)
	{
		SafeRead (FH, &skins[i].type, sizeof(skins[i].type));
		skins[i].pdata = SafeMalloc (model.skinwidth * model.skinheight);
		SafeRead (FH, skins[i].pdata, model.skinwidth * model.skinheight);
	}

	// read in the st's
	SafeRead (FH, stverts, model.num_st_verts * sizeof(stverts[0]));
	for (i = 0 ; i < model.num_st_verts ; i++)
	{
		if (stverts[i].onseam == ALIAS_ONSEAM)
		{
			stverts[i].onseam = 3;
		}
		else
		{
			stverts[i].onseam = 0;
		}

		stverts[i].s = LittleLong (stverts[i].s);
		stverts[i].t = LittleLong (stverts[i].t);
	}

	// read in the triangles
	for (i = 0 ; i < model.numtris ; i++)
	{
		int			j;
		dtriangle_t	tri;

		SafeRead (FH, &tri, sizeof(tri));

		triangles[i].facesfront = LittleLong (tri.facesfront);

		for (j = 0 ; j < 3 ; j++)
		{
			triangles[i].vertindex[j] = LittleLong (tri.vertindex[j]);
			triangles[i].stindex[j] = triangles[i].vertindex[j];
		}
	}

	// read in the frames
	curframe = 0;
	for (i = 0 ; i < model.numframes ; i++)
	{
		SafeRead (FH, &frames[curframe].type, sizeof(frames[curframe].type));

		if (frames[curframe].type == ALIAS_SINGLE)
		{	// single (non-grouped) frame
			ReadFrame (FH, curframe);
			curframe++;
		}
		else
		{
			COM_Error("group frames not implemented");

/*			int		j, numframes, groupframe;
			float	totinterval;

			groupframe = curframe;
			curframe++;
			numframes = frames[groupframe].numgroupframes;

		//
		// set and write the group header
		//
			WriteGroupBBox (modelouthandle, numframes, curframe);

		//
		// write the interval array
		//
			totinterval = 0.0;

			for (j = 0 ; j < numframes ; j++)
			{
				daliasinterval_t	temp;

				totinterval += frames[groupframe+1+j].interval;
				temp.interval = LittleFloat (totinterval);

				SafeWrite (modelouthandle, &temp, sizeof(temp));
			}

			for (j = 0 ; j < numframes ; j++)
			{
				WriteFrame (modelouthandle, curframe);
				curframe++;
			}*/
		}
	}

	ModelReadIn = true;
	original_size = ftell(FH);

	fclose(FH);
}

//==========================================================================
//
// SetSkinValues
//
// Called for the base frame.
//
//==========================================================================

static void SetSkinValues (void)
{
	int			i;
	float		v;
	int			width, height, iwidth, iheight, skinwidth;
	float		basex, basey;
	float		scw, sch;

	for (i = 0 ; i < 3 ; i++)
	{
		mins[i] = 9999999;
		maxs[i] = -9999999;
	}

	for (i = 0 ; i < model.numverts ; i++)
	{
		int		j;

		stverts[i].onseam = 0;

		for (j = 0 ; j < 3 ; j++)
		{
			v = baseverts[i][j];
			if (v < mins[j])
				mins[j] = v;
			if (v > maxs[j])
				maxs[j] = v;
		}
	}

	for (i = 0 ; i < 3 ; i++)
	{
		mins[i] = floor(mins[i]);
		maxs[i] = ceil(maxs[i]);
	}

	width = maxs[0] - mins[0];
	height = maxs[2] - mins[2];

	printf("width: %i  height: %i\n",width, height);

	scw = (ScaleWidth/2)*SCALE_ADJUST_FACTOR;
	sch = ScaleHeight*SCALE_ADJUST_FACTOR;

	scale = scw/width;
	if (height*scale >= sch)
	{
		scale = sch/height;
	}

	iwidth = ceil(width*scale)+4;
	iheight = ceil(height*scale)+4;

	printf ("scale: %f\n",scale);
	printf ("iwidth: %i  iheight: %i\n",iwidth, iheight);

//
// determine which side of each triangle to map the texture to
//
	for (i = 0 ; i < model.numtris ; i++)
	{
		int		j;
		vec3_t	vtemp1, vtemp2, normal;

		VectorSubtract (baseverts[triangles[i].vertindex[0]],
				baseverts[triangles[i].vertindex[1]], vtemp1);
		VectorSubtract (baseverts[triangles[i].vertindex[2]],
				baseverts[triangles[i].vertindex[1]], vtemp2);
		CrossProduct (vtemp1, vtemp2, normal);

		if (normal[1] > 0)
		{
			basex = iwidth + 2;
			triangles[i].facesfront = 0;
		}
		else
		{
			basex = 2;
			triangles[i].facesfront = 1;
		}
		basey = 2;

		for (j = 0 ; j < 3 ; j++)
		{
			float		*pbasevert;
			stvert_t	*pstvert;

			pbasevert = baseverts[triangles[i].vertindex[j]];
			pstvert = &stverts[triangles[i].vertindex[j]];

			if (triangles[i].facesfront)
			{
				pstvert->onseam |= 1;
			}
			else
			{
				pstvert->onseam |= 2;
			}

			if ((triangles[i].facesfront) || ((pstvert->onseam & 1) == 0))
			{
			// we want the front s value for seam vertices
				pstvert->s = Q_rint((pbasevert[0] - mins[0]) * scale + basex);
				pstvert->t = Q_rint((maxs[2] - pbasevert[2]) * scale + basey);
			}
		}
	}

// make the width a multiple of 4; some hardware requires this, and it ensures
// dword alignment for each scan
	skinwidth = iwidth*2;
	model.skinwidth = (skinwidth + 3) & ~3;
	model.skinheight = iheight;

	printf ("skin width: %i (unpadded width %i)  skin height: %i\n",
			model.skinwidth, skinwidth, model.skinheight);
}


/*
=================
Cmd_Base
=================
*/
static void Cmd_Base (void)
{
	int		i, j, k;
	triangle_t	*ptri;
	byte	*pskinbitmap;

	GetToken(false);
	q_strlcpy(qbasename, token, sizeof(qbasename));

	//q_snprintf(file1, sizeof(file1), "%s/%s.tri", cdpartial, token);
	//ExpandPathAndArchive (file1);

	q_snprintf(file1, sizeof(file1), "%s/%s", cddir, token);

	// Extract the scaling information from a skin page
	GetToken(false);
	q_snprintf(file2, sizeof(file2), "%s/%s.pcx", cddir, token);
	LoadPCXSkin(file2, &pskinbitmap);
	ScaleWidth = (float)ExtractNumber(pskinbitmap, ENCODED_WIDTH_X, ENCODED_WIDTH_Y);
	ScaleHeight = (float)ExtractNumber(pskinbitmap, ENCODED_HEIGHT_X, ENCODED_HEIGHT_Y);
	free(pskinbitmap);

//
// load the base triangles
//
	LoadTriangleList (file1, &ptri, &model.numtris);
	printf("Number of triangles (including degenerate triangles): %d\n", model.numtris);

//
// run through all the base triangles, storing each unique vertex in the
// base vertex list and setting the indirect triangles to point to the base
// vertices
//
	for (i = 0; i < model.numtris; i++)
	{
		if (VectorCompare(ptri[i].verts[0], ptri[i].verts[1])
			|| VectorCompare(ptri[i].verts[1], ptri[i].verts[2])
			|| VectorCompare(ptri[i].verts[2], ptri[i].verts[0]))
		{
			degeneratetris++;
			degenerate[i] = 1;
		}
		else
		{
			degenerate[i] = 0;
		}

		for (j = 0; j < 3; j++)
		{
			for (k = 0; k < model.numverts; k++)
			{
				if (VectorCompare(ptri[i].verts[j], baseverts[k]))
				{
					// already in the base vertex list
					break;
				}
			}

			if (k == model.numverts)
			{
				// new vertex
				VectorCopy(ptri[i].verts[j], baseverts[model.numverts]);
				model.numverts++;
				model.num_st_verts = model.numverts;
			}

			triangles[i].vertindex[j] = k;
			// Unoptimized vertexes parellel st vertexes
			triangles[i].stindex[j] = k;
		}
	}

	printf("Number of vertices: %i\n", model.numverts);

	printf("Extracted scaling info: width %d, height %d\n",
			(int)ScaleWidth, (int)ScaleHeight);

//
// calculate s & t for each vertex, and set the skin width and height
//
	SetSkinValues ();
}


/*
===============
Cmd_Skin
===============
*/
static void Cmd_Skin (void)
{
	//byte	*ppal;
	byte	*pskinbitmap;
	byte	*ptemp1, *ptemp2;
	int		i;
	float	scw, sch;

	GetToken (false);
	q_strlcpy(skinname, token, sizeof(skinname));

	//q_snprintf(file1, sizeof(file1), "%s/%s.lbm", cdpartial, token);
	//ExpandPathAndArchive (file1);

	q_snprintf(file1, sizeof(file1), "%s/%s.pcx", cddir, token);

	if (TokenAvailable ())
	{
		GetToken (false);
		skins[skincount].interval = atof (token);
		if (skins[skincount].interval <= 0.0)
			COM_Error ("Non-positive interval");
	}
	else
	{
		skins[skincount].interval = 0.1F;
	}

	//LoadFile(file1, &pskinbitmap);
	LoadPCXSkin(file1, &pskinbitmap);

	scw = (float)ExtractNumber(pskinbitmap, ENCODED_WIDTH_X, ENCODED_WIDTH_Y);
	sch = (float)ExtractNumber(pskinbitmap, ENCODED_HEIGHT_X, ENCODED_HEIGHT_Y);
	if (ScaleWidth != scw || ScaleHeight != sch)
	{
		COM_Error("Conflicting scale values in %s.\nBase info: %d, %d\n"
			"Skin info: %d, %d", file1, (int)ScaleWidth, (int)ScaleHeight,
			(int)scw, (int)sch);
	}

	skins[skincount].pdata = SafeMalloc (model.skinwidth * model.skinheight);

	// Copy skinwidth*skinheight, since PCXs are always
	// loaded as 640x480 bitmaps
	ptemp1 = (byte *) skins[skincount].pdata;
	ptemp2 = pskinbitmap;
	for (i = 0; i < model.skinheight; i++)
	{
		memcpy(ptemp1, ptemp2, model.skinwidth);
		ptemp1 += model.skinwidth;
		ptemp2 += 640;
	}

	skincount++;

	if (skincount > MAXSKINS)
		COM_Error ("Too many skins; increase MAXSKINS");
}

//==========================================================================
// ExtractNumber

static int ExtractNumber(byte *pic, int x, int y)
{
	return ExtractDigit(pic, x, y)*100 + ExtractDigit(pic, x+6, y)*10 + ExtractDigit(pic, x+12, y);
}

//==========================================================================
// ExtractDigit

static int ExtractDigit(byte *pic, int x, int y)
{
	int		i;
	int		r, c;
	char	digString[32];
	char	*buffer;
	byte	backColor;

	backColor = pic[(SKINPAGE_HEIGHT-1)*SKINPAGE_WIDTH];
	buffer = digString;
	for (r = 0; r < 5; r++)
	{
		for (c = 0; c < 5; c++)
		{
			*buffer++ = (pic[(y+r)*SKINPAGE_WIDTH+x+c] == backColor) ? ' ' : '*';
		}
	}
	*buffer = '\0';
	for (i = 0; i < 10; i++)
	{
		if (strcmp(DigitDefs[i], digString) == 0)
		{
			return i;
		}
	}
	COM_Error("Unable to extract scaling info from skin PCX.");
	return -1;
}


/*
===============
GrabFrame
===============
*/
static void GrabFrame (const char *frame, int isgroup)
{
	triangle_t		*ptri;
	int				i, j;
	trivert_t		*ptrivert;
	int				numtris;

	//q_snprintf(file1, sizeof(file1), "%s/%s.tri", cdpartial, frame);
	//ExpandPathAndArchive (file1);

	q_snprintf(file1, sizeof(file1), "%s/%s", cddir, frame);

	printf ("grabbing %s\n", file1);
	frames[framecount].interval = 0.1F;
	q_strlcpy(frames[framecount].name, frame, sizeof(frames[0].name));

//
// load the frame
//
	LoadTriangleList (file1, &ptri, &numtris);

	if (numtris != model.numtris)
	{
		COM_Error("Number of triangles doesn't match\n"
			"Base frame: %d, frame %s: %d\n",
			model.numtris, frame, numtris);
	}

// set the intervals
	if (isgroup && TokenAvailable ())
	{
		GetToken (false);
		frames[framecount].interval = atof (token);
		if (frames[framecount].interval <= 0.0)
			COM_Error ("Non-positive interval %s %f", token, frames[framecount].interval);
	}
	else
	{
		frames[framecount].interval = 0.1F;
	}

//
// allocate storage for the frame's vertices
//
	ptrivert = verts[framecount];

	frames[framecount].pdata = ptrivert;
	frames[framecount].type = ALIAS_SINGLE;

	for (i = 0 ; i < model.numverts ; i++)
	{
		vnorms[i].numnormals = 0;
	}

//
// store the frame's vertices in the same order as the base. This assumes the
// triangles and vertices in this frame are in exactly the same order as in the
// base
//
	for (i = 0 ; i < numtris ; i++)
	{
		vec3_t	vtemp1, vtemp2, normal;
		float	ftemp;

		if (degenerate[i])
			continue;

		if (firstframe)
		{
			VectorSubtract (ptri[i].verts[0], ptri[i].verts[1], vtemp1);
			VectorSubtract (ptri[i].verts[2], ptri[i].verts[1], vtemp2);
			VectorScale (vtemp1, scale_up, vtemp1);
			VectorScale (vtemp2, scale_up, vtemp2);
			CrossProduct (vtemp1, vtemp2, normal);

			totsize += sqrt (normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]) / 2.0;
		}

		VectorSubtract (ptri[i].verts[0], ptri[i].verts[1], vtemp1);
		VectorSubtract (ptri[i].verts[2], ptri[i].verts[1], vtemp2);
		CrossProduct (vtemp1, vtemp2, normal);

		VectorNormalize (normal);

	// rotate the normal so the model faces down the positive x axis
		ftemp = normal[0];
		normal[0] = -normal[1];
		normal[1] = ftemp;

		for (j = 0 ; j < 3 ; j++)
		{
			int		k;
			int		vertindex;

			vertindex = triangles[i].vertindex[j];

		// rotate the vertices so the model faces down the positive x axis
		// also adjust the vertices to the desired origin

			ptrivert[vertindex].v[0] = ((-ptri[i].verts[j][1]) * scale_up) + adjust[0];
			ptrivert[vertindex].v[1] = (ptri[i].verts[j][0] * scale_up) + adjust[1];
			ptrivert[vertindex].v[2] = (ptri[i].verts[j][2] * scale_up) + adjust[2];

			for (k = 0 ; k < 3 ; k++)
			{
				if (ptrivert[vertindex].v[k] < framesmins[k])
					framesmins[k] = ptrivert[vertindex].v[k];

				if (ptrivert[vertindex].v[k] > framesmaxs[k])
					framesmaxs[k] = ptrivert[vertindex].v[k];
			}

			VectorCopy (normal, vnorms[vertindex].normals[vnorms[vertindex].numnormals] );

			vnorms[vertindex].numnormals++;
		}
	}

//
// calculate the vertex normals, match them to the template list, and store the
// index of the best match
//
	for (i = 0 ; i < model.numverts ; i++)
	{
		int		l;
		vec3_t	v;
		float	maxdot;
		int		maxdotindex;

		if (vnorms[i].numnormals > 0)
		{
			for (l = 0 ; l < 3 ; l++)
			{
				int		m;

				v[l] = 0;

				for (m = 0 ; m < vnorms[i].numnormals ; m++)
				{
					v[l] += vnorms[i].normals[m][l];
				}

				v[l] /= vnorms[i].numnormals;
			}
		}
		else
		{
			COM_Error ("Vertex with no non-degenerate triangles attached");
		}

		VectorNormalize (v);

		maxdot = -999999.0;
		maxdotindex = -1;

		for (l = 0 ; l < NUMVERTEXNORMALS ; l++)
		{
			float	dot;

			dot = DotProduct (v, avertexnormals[l]);
			if (dot > maxdot)
			{
				maxdot = dot;
				maxdotindex = l;
			}
		}

		ptrivert[i].lightnormalindex = maxdotindex;
	}

	framecount++;

	if (framecount >= MAXFRAMES)
		COM_Error ("Too many frames; increase MAXFRAMES");

	free (ptri);
	firstframe = 0;
}


/*
===============
Cmd_Frame	
===============
*/
static void Cmd_Frame (int isgroup)
{
	while (TokenAvailable())
	{
		GetToken (false);
		GrabFrame (token, isgroup);

		if (!isgroup)
			model.numframes++;
	}
}

/*
===============
Cmd_SkinGroupStart
===============
*/
static void Cmd_SkinGroupStart (void)
{
	int			groupskin;

	groupskin = skincount++;
	if (skincount >= MAXFRAMES)
		COM_Error ("Too many skins; increase MAXSKINS");

	skins[groupskin].type = ALIAS_SKIN_GROUP;
	skins[groupskin].numgroupskins = 0;

	while (1)
	{
		GetToken (true);
		if (endofscript)
			COM_Error ("End of file during group");

		if (!strcmp (token, "$skin"))
		{
			Cmd_Skin ();
			skins[groupskin].numgroupskins++;
		}
		else if (!strcmp (token, "$skingroupend"))
		{
			break;
		}
		else
		{
			COM_Error ("$skin or $skingroupend expected\n");
		}
	}

	if (skins[groupskin].numgroupskins == 0)
		COM_Error ("Empty group\n");
}

/*
===============
Cmd_FrameGroupStart
===============
*/
static void Cmd_FrameGroupStart (void)
{
	int			groupframe;

	groupframe = framecount++;
	if (framecount >= MAXFRAMES)
		COM_Error ("Too many frames; increase MAXFRAMES");

	frames[groupframe].type = ALIAS_GROUP;
	frames[groupframe].numgroupframes = 0;

	while (1)
	{
		GetToken (true);
		if (endofscript)
			COM_Error ("End of file during group");

		if (!strcmp (token, "$frame"))
		{
			Cmd_Frame (1);
		}
		else if (!strcmp (token, "$framegroupend"))
		{
			break;
		}
		else
		{
			COM_Error ("$frame or $framegroupend expected\n");
		}
	}

	frames[groupframe].numgroupframes += framecount - groupframe - 1;

	if (frames[groupframe].numgroupframes == 0)
		COM_Error ("Empty group\n");
}

/*
=================
Cmd_Origin
=================
*/
static void Cmd_Origin (void)
{

// rotate points into frame of reference so model points down the positive x
// axis
	GetToken (false);
	adjust[1] = -atof (token);

	GetToken (false);
	adjust[0] = atof (token);

	GetToken (false);
	adjust[2] = -atof (token);
}

/*
=================
Cmd_Eyeposition
=================
*/
static void Cmd_Eyeposition (void)
{

// rotate points into frame of reference so model points down the positive x
// axis
	GetToken (false);
	model.eyeposition[1] = atof (token);

	GetToken (false);
	model.eyeposition[0] = -atof (token);

	GetToken (false);
	model.eyeposition[2] = atof (token);
}

/*
=================
Cmd_ScaleUp
=================
*/
static void Cmd_ScaleUp (void)
{
	GetToken (false);
	scale_up = atof (token);
}

/*
=================
Cmd_Flags
=================
*/
static void Cmd_Flags (void)
{
	GetToken (false);
	model.flags = atoi (token);
}

/*
=================
Cmd_RotateHTR
=================
*/
static void Cmd_RotateHTR(void)
{
	GetToken(false);
	FixHTRRotateX = atof(token);

	GetToken(false);
	FixHTRRotateY = atof(token);

	GetToken(false);
	FixHTRRotateZ = atof(token);
}

/*
=================
Cmd_TranslateHTR
=================
*/
static void Cmd_TranslateHTR(void)
{
	GetToken(false);
	FixHTRTranslateX = atof(token);

	GetToken(false);
	FixHTRTranslateY = atof(token);

	GetToken(false);
	FixHTRTranslateZ = atof(token);
}

/*
=================
Cmd_Modelname
=================
*/
static void Cmd_Modelname (void)
{
	WriteModel ();
	GetToken (false);
	q_strlcpy(outname, token, sizeof(outname));
}

/*
===============
ParseScript
===============
*/
static void ParseScript (void)
{
	while (1)
	{
		do
		{	// look for a line starting with a $ command
			GetToken (true);
			if (endofscript)
				return;
			if (token[0] == '$')
				break;
			while (TokenAvailable())
				GetToken (false);
		} while (1);

		if (!strcmp (token, "$modelname"))
		{
			Cmd_Modelname ();
		}
		else if (!strcmp (token, "$base"))
		{
			Cmd_Base ();
		}
		else if (!strcmp (token, "$cd"))
		{
			if (cdset)
				COM_Error ("Two $cd in one model");
			cdset = true;
			GetToken (false);
			q_strlcpy (cdpartial, token, sizeof(cdpartial));
			q_strlcpy (cddir, ExpandPath(token), sizeof(cddir));
		}
		else if (!strcmp (token, "$sync"))
		{
			model.synctype = ST_SYNC;
		}
		else if (!strcmp (token, "$origin"))
		{
			Cmd_Origin ();
		}
		else if (!strcmp (token, "$eyeposition"))
		{
			Cmd_Eyeposition ();
		}
		else if (!strcmp (token, "$scale"))
		{
			Cmd_ScaleUp ();
		}
		else if (!strcmp (token, "$flags"))
		{
			Cmd_Flags ();
		}
		else if (!strcmp (token, "$rotatehtr"))
		{
			Cmd_RotateHTR();
		}
		else if (!strcmp (token, "$translatehtr"))
		{
			Cmd_TranslateHTR();
		}
		else if (!strcmp (token, "$frame"))
		{
			Cmd_Frame (0);
		}
		else if (!strcmp (token, "$skin"))
		{
			Cmd_Skin ();
			model.numskins++;
		}
		else if (!strcmp (token, "$framegroupstart"))
		{
			Cmd_FrameGroupStart ();
			model.numframes++;
		}
		else if (!strcmp (token, "$skingroupstart"))
		{
			Cmd_SkinGroupStart ();
			model.numskins++;
		}
		else
		{
			COM_Error ("bad command %s\n", token);
		}
	}
}

//==========================================================================
//
// LoadPCXSkin
//
//==========================================================================

static void LoadPCXSkin(const char *filename, byte **buffer)
{
	int		i;
	void		*buf;
	pcx_t		*pcx;
	int		w, h;
	int		count;
	byte	controlByte, repeatByte;
	byte	*src, *dst;
	byte	length;

	// Load file
	LoadFile(filename, &buf);
	pcx = (pcx_t *) buf;

	// Check for a valid PCX header
	w = pcx->xMax - pcx->xMin + 1;
	h = pcx->yMax - pcx->yMin + 1;
	if (pcx->tag != 10 || pcx->version != 5
		|| pcx->encoding != 1 || pcx->pixelBits != 8
		|| pcx->planes != 1 || pcx->lineBytes != w
		|| w != SKINPAGE_WIDTH || h != SKINPAGE_HEIGHT)
	{
		printf("->tag = %d [10]\n", pcx->tag);
		printf("->version = %d [5]\n", pcx->version);
		printf("->encoding = %d [1]\n", pcx->encoding);
		printf("->pixelBits = %d [8]\n", pcx->pixelBits);
		printf("->planes = %d [1]\n", pcx->planes);
		printf("->lineBytes = %d\n", pcx->lineBytes);
		printf("width = %d [640]\n", w);
		printf("height = %d [480]\n", h);
		COM_Error("Bad PCX skin file.\n");
	}

	// Allocate page
	dst = (byte *) SafeMalloc(SKINPAGE_SIZE);
	*buffer = dst;

	// Decompress
	for (src = &pcx->data, i = 0; i < SKINPAGE_HEIGHT;
		//dst += SKINPAGE_WIDTH,
		i++)
	{
		count = 0;
		while (count < SKINPAGE_WIDTH)
		{
			controlByte = *src++;
			if ((controlByte&0xc0) == 0xc0)
			{	// Repeat run
				length = controlByte&0x3f;
				repeatByte = *src++;
				memset(dst, repeatByte, length);
				dst += length;
				count += length;
			}
			else
			{
				*dst++ = controlByte;
				count++;
			}
		}
		if (count > SKINPAGE_WIDTH)
		{
			COM_Error("PCX decompression overflow.\n");
		}
	}

	free (buf);
}

