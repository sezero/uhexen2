#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "byteordr.h"
#include "pathutil.h"
#include "mathlib.h"
#include "bspfile.h"

#define B2M_VERSION "0.12"

#define MAX_FACE_EDGES 100

static int verbose1 = 0, verbose2 = 0, verbose3 = 0;

//--------------

static void GetEdges(int ledge, dedge_t *edge)
{
    if (dsurfedges[ledge]>0) {
        *edge = dedges[dsurfedges[ledge]];
    } else {
        edge->v[0] = dedges[-dsurfedges[ledge]].v[1];
        edge->v[1] = dedges[-dsurfedges[ledge]].v[0];
    }
}

static void ProcessFace(int face)
{
    int n_face_edges, ledge, ledge2;
    dedge_t edges[MAX_FACE_EDGES];
    dvertex_t vert_beg[MAX_FACE_EDGES];
    dvertex_t vert_end[MAX_FACE_EDGES];
    vec3_t normal, v1, v2, v3;
    miptex_t *p_miptex;
    dmiptexlump_t *head_miptex;
    texinfo_t *p_texinfo;
    float x_off, y_off, rotation, x_scale, y_scale;
    static int gi = 0;
    char gs[12];
    char texname[18];

    n_face_edges = dfaces[face].numedges;
    if (n_face_edges < 3)
        COM_Error ("too few edges for face");
    if (n_face_edges > MAX_FACE_EDGES)
        COM_Error ("too many edges for face");

    for (ledge2 = ledge = 0; ledge < n_face_edges; ledge++,ledge2++) {
        GetEdges(ledge2 + dfaces[face].firstedge, &edges[ledge]);
        vert_beg[ledge] = dvertexes[edges[ledge].v[0]];
        vert_end[ledge] = dvertexes[edges[ledge].v[1]];
        if (verbose1)
            printf(" // edge %d, ledge %d [%4.2f %4.2f %4.2f] [%4.2f %4.2f %4.2f]\n",
                    ledge, ledge + dfaces[face].firstedge,
                    vert_beg[ledge].point[0], vert_beg[ledge].point[1], vert_beg[ledge].point[2],
                    vert_end[ledge].point[0], vert_end[ledge].point[1], vert_end[ledge].point[2]);
        if (VectorCompare(vert_beg[ledge].point, vert_end[ledge].point)) {
            ledge--;
            if (verbose1)
                printf(" // edge skiped (a) : no normal\n");
        }
    }

    VectorCopy(dplanes[dfaces[face].planenum].normal, normal);
    if (verbose1) {
        printf(" // dfaces[face].planenum = %d,  side = %d\n", dfaces[face].planenum, dfaces[face].side);
        printf(" // normal ( %4.2f %4.2f %4.2f ) type %d\n",
            normal[0], normal[1], normal[2], dplanes[dfaces[face].planenum].type);
    }
    VectorScale (normal, 2.0f, normal);
    if (!dfaces[face].side)
        VectorInverse(normal);

    for (ledge2 = 1; ledge2 < n_face_edges - 2; ledge2++) {
        VectorSubtract(vert_end[0].point, vert_beg[0].point, v1);
        VectorSubtract(vert_end[ledge2].point, vert_beg[ledge2].point, v2);
        VectorNormalize(v1);
        VectorNormalize(v2);
        if (!VectorCompare(v1, v2))
            break;
        if (verbose1)
            printf(" // edge %d skiped (b) : duplicate plane\n", ledge2);
    }

    head_miptex = (dmiptexlump_t *)dtexdata;
    p_texinfo = &texinfo[dfaces[face].texinfo];
    p_miptex = (miptex_t *)((((byte *)dtexdata))+(head_miptex->dataofs[p_texinfo->miptex]));
    strcpy(texname, p_miptex->name);
    q_strupr(texname);
    if (verbose2)
        printf(" // texinfo ( %4.2f %4.2f %4.2f %4.2f ) ( %4.2f %4.2f %4.2f %4.2f )\n",
                p_texinfo->vecs[0][0], p_texinfo->vecs[0][1], p_texinfo->vecs[0][2], p_texinfo->vecs[0][3],
                p_texinfo->vecs[0][0], p_texinfo->vecs[0][1], p_texinfo->vecs[0][2], p_texinfo->vecs[0][3]);

    //here must calc x_off y_off rotation x_scale y_scale from p_texinfo->vecs !
    x_off = y_off = rotation = 0.0f;
    x_scale = y_scale = 1.0f;

    printf(" {\n");
    printf("  ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) %s %.0f %.0f %.0f %.6f %.6f 0\n",
            vert_beg[0].point[0], vert_beg[0].point[1], vert_beg[0].point[2],
            vert_end[0].point[0], vert_end[0].point[1], vert_end[0].point[2],
            vert_end[ledge2].point[0], vert_end[ledge2].point[1], vert_end[ledge2].point[2],
            texname,
            x_off, y_off, rotation, x_scale, y_scale);

    for (ledge = 0; ledge < n_face_edges; ledge++) {
        if (ledge == 0) {
            VectorSubtract(vert_end[n_face_edges-1].point, vert_beg[n_face_edges-1].point, v1);
            VectorSubtract(vert_end[ledge].point, vert_beg[ledge].point, v2);
            VectorNormalize(v1);
            VectorNormalize(v2);
            if (VectorCompare(v1, v2)) {
                if (verbose1)
                    printf(" // edge 0 skiped (c) : duplicate plane\n");
                continue;
            }
        } else {
            VectorSubtract(vert_end[ledge-1].point, vert_beg[ledge-1].point, v1);
            VectorSubtract(vert_end[ledge].point, vert_beg[ledge].point, v2);
            VectorNormalize(v1);
            VectorNormalize(v2);
            if (VectorCompare(v1, v2)) {
                if (verbose1)
                    printf(" // edge %d skiped (d) : duplicate plane\n", ledge);
                continue;
            }
        }
        VectorAdd(vert_end[ledge].point, normal, v2);
        printf("  ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) %s 0 0 0 1.000000 1.000000 0\n",
                vert_end[ledge].point[0], vert_end[ledge].point[1], vert_end[ledge].point[2],
                vert_beg[ledge].point[0], vert_beg[ledge].point[1], vert_beg[ledge].point[2],
                v2[0], v2[1], v2[2],
                texname);
    }

    VectorAdd(vert_beg[0].point, normal, v2);
    VectorAdd(vert_end[0].point, normal, v1);
    VectorAdd(vert_end[ledge2].point, normal, v3);

    if (verbose3)
        sprintf(gs, "tx%04x", gi++);
    printf("  ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) ( %4.0f %4.0f %4.0f ) %s 0 0 0 1.000000 1.000000 0\n",
            v1[0], v1[1], v1[2],
            v2[0], v2[1], v2[2],
            v3[0], v3[1], v3[2],
            !verbose3 ? texname : gs);
    printf(" }\n");
}

static void ProcessModel(int model)
{
    int face;
    printf(" // total brushes: %d\n", dmodels[model].numfaces);
    for (face = 0; face < dmodels[model].numfaces; face++) {
        ProcessFace(face + dmodels[model].firstface);
    }
}

//--------------

/*
The following code: StartTokenParsing, GetToken, and ParseEpair, are
taken from map.c of QBSP source, with struct epair_s and ParseEpair()
simplified for use in here.  They are licenced under the GNU GPL v2:

Copyright (C) 1996-1997  Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define	MAXTOKEN	128

static char	token[MAXTOKEN];
static char	*script_p;
static int		scriptline;

static void StartTokenParsing (char *data)
{
	scriptline = 1;
	script_p = data;
}

static qboolean GetToken (qboolean crossline)
{
	char	*token_p;

//
// skip space
//
skipspace:
	while (*script_p <= 32)
	{
		if (!*script_p)
		{
			if (!crossline)
				COM_Error ("Line %i is incomplete",scriptline);
			return false;
		}
		if (*script_p++ == '\n')
		{
			if (!crossline)
				COM_Error ("Line %i is incomplete",scriptline);
			scriptline++;
		}
	}

	if (script_p[0] == '/' && script_p[1] == '/')	// comment field
	{
		if (!crossline)
			COM_Error ("Line %i is incomplete\n",scriptline);
		while (*script_p++ != '\n')
			if (!*script_p)
			{
				if (!crossline)
					COM_Error ("Line %i is incomplete",scriptline);
				return false;
			}
		goto skipspace;
	}

//
// copy token
//
	token_p = token;

	if (*script_p == '"')
	{
		script_p++;
		while ( *script_p != '"' )
		{
			if (!*script_p)
				COM_Error ("EOF inside quoted token");
			*token_p++ = *script_p++;
			if (token_p > &token[MAXTOKEN-1])
				COM_Error ("Token too large on line %i",scriptline);
		}
		script_p++;
	}
	else while ( *script_p > 32 )
	{
		*token_p++ = *script_p++;
		if (token_p > &token[MAXTOKEN-1])
			COM_Error ("Token too large on line %i",scriptline);
	}

	*token_p = 0;

	return true;
}

static struct epair_s
{
	char	key[MAX_KEY];
	char	value[MAX_VALUE];
} epair;

static void ParseEpair (void)
{
	if (strlen(token) >= MAX_KEY-1)
		COM_Error ("ParseEpar: token too long");
	strcpy(epair.key, token);
	GetToken (false);
	if (strlen(token) >= MAX_VALUE-1)
		COM_Error ("ParseEpar: token too long");
	strcpy(epair.value, token);
}

//--------------

static int ParseEnt(void)
{
	int model;
	int keys;

	model = -1;
	keys = 0;

	if (!GetToken (true))
		return -2;

	if (strcmp (token, "{"))
		COM_Error ("ProcessEnts: { not found");

	do {
		if (!GetToken (true))
			COM_Error ("ParseEnt: EOF without closing brace");
		if (!strcmp (token, "}") )
			break;
		ParseEpair();
		if (!keys) {
			keys++;
			printf ("{\n");
		}
		if (!strcmp(epair.key, "model")) {
			model = atoi(&epair.value[1]);
		if (verbose1 || verbose2 || verbose3)
			printf (" // \"%s\" \"%s\"\n", epair.key, epair.value);
		} else {
			printf (" \"%s\" \"%s\"\n", epair.key, epair.value);
		}
		if (!strcmp(epair.key, "classname") && !strcmp(epair.value, "worldspawn")) {
			model = 0;
		}
	} while (1);

	if (model >= 0) {
		ProcessModel (model);
	}

	if (keys) {
		printf ("}\n");
	}

	return model;
}

static void MakeMap (void)
{
	int model;
	StartTokenParsing(dentdata);
	while (1) {
		model = ParseEnt ();
		if (model == -2)
			break;
	}
}

//--------------

int main (int argc, char **argv)
{
	int		i;
	char		source[1024];
	double		start, end;

	ValidateByteorder ();

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v1")) {
			printf ("//verbose1 = true\n");
			verbose1 = 1;
		}
		else if (!strcmp(argv[i], "-v2")) {
			printf ("//verbose2 = true\n");
			verbose2 = 1;
		}
		else if (!strcmp(argv[i], "-v3")) {
			printf ("//verbose3 = true\n");
			verbose3 = 1;
		}
		else if (argv[i][0] == '-')
			COM_Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1) {
		printf ("BSP2MAP %s  Created by Janis Jagars (Disastry); modified by Pa3PyX\n", B2M_VERSION);
		COM_Error ("usage: bsp2map [-v1] [-v2] [-v3] bspfile > mapfile");
	}

	start = COM_GetTime ();

	printf ("//---------------------\n");
	strcpy (source, argv[i]);
	DefaultExtension (source, ".bsp", sizeof(source));
	printf ("//Created from %s by BSP2MAP %s\n", source, B2M_VERSION);

	LoadBSPFile (source);

	MakeMap ();

	end = COM_GetTime ();
	printf ("//%5.1f seconds elapsed\n", end-start);
	printf ("//---------------------\n");

	return 0;
}
