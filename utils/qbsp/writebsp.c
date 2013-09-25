/*
 * writebsp.c
 * $Id: writebsp.c,v 1.16 2008-11-07 18:02:42 sezero Exp $
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "q_ctype.h"
#include "cmdlib.h"
#include "util_io.h"
#include "mathlib.h"
#include "q_endian.h"
#include "bspfile.h"
#include "bsp5.h"


static int		headclipnode;
static int		firstface;
int		LightValues[MAX_MAP_CLIPNODES];

//===========================================================================

/*
==================
FindFinalPlane

Used to find plane index numbers for clip nodes read from child processes
==================
*/
int FindFinalPlane (dplane_t *p)
{
	int		i;
	dplane_t	*dplane;

	for (i = 0, dplane = dplanes ; i < numplanes ; i++, dplane++)
	{
		if (p->type != dplane->type)
			continue;
		if (p->dist != dplane->dist)
			continue;
		if (p->normal[0] != dplane->normal[0])
			continue;
		if (p->normal[1] != dplane->normal[1])
			continue;
		if (p->normal[2] != dplane->normal[2])
			continue;
		return i;
	}

//
// new plane
//
	if (numplanes == MAX_MAP_PLANES)
		COM_Error ("numplanes == MAX_MAP_PLANES");
	dplane = &dplanes[numplanes];
	*dplane = *p;
	numplanes++;

	return numplanes - 1;
}


static int		planemapping[MAX_MAP_PLANES];

static void WriteNodePlanes_r (node_t *node)
{
	plane_t		*plane;
	dplane_t	*dplane;

	if (node->planenum == -1)
		return;
	if (planemapping[node->planenum] == -1)
	{	// a new plane
		planemapping[node->planenum] = numplanes;

		if (numplanes == MAX_MAP_PLANES)
			COM_Error ("numplanes == MAX_MAP_PLANES");
		plane = &planes[node->planenum];
		dplane = &dplanes[numplanes];
		dplane->normal[0] = plane->normal[0];
		dplane->normal[1] = plane->normal[1];
		dplane->normal[2] = plane->normal[2];
		dplane->dist = plane->dist;
		dplane->type = plane->type;

		numplanes++;
	}

	node->outputplanenum = planemapping[node->planenum];

	WriteNodePlanes_r (node->children[0]);
	WriteNodePlanes_r (node->children[1]);
}

/*
==================
WriteNodePlanes

==================
*/
void WriteNodePlanes (node_t *nodes)
{
	memset (planemapping,-1, sizeof(planemapping));
	WriteNodePlanes_r (nodes);
}

//===========================================================================

/*
==================
WriteClipNodes_r

==================
*/
static int WriteClipNodes_r (node_t *node)
{
	int			i, c;
	dclipnode_t	*cn;
	int			num;

// FIXME: free more stuff?
	if (node->planenum == -1)
	{
		num = node->contents;
		free (node);
		return num;
	}

// emit a clipnode
	c = numclipnodes;
	cn = &dclipnodes[numclipnodes];
	numclipnodes++;
	cn->planenum = node->outputplanenum;
	for (i = 0 ; i < 2 ; i++)
		cn->children[i] = WriteClipNodes_r(node->children[i]);

	free (node);
	return c;
}

/*
==================
WriteClipNodes

Called after the clipping hull is completed.  Generates a disk format
representation and frees the original memory.
==================
*/
void WriteClipNodes (node_t *nodes)
{
	headclipnode = numclipnodes;
	WriteClipNodes_r (nodes);
}

//===========================================================================

/*
==================
WriteLeaf
==================
*/
static void WriteLeaf (node_t *node)
{
	face_t		**fp, *f;
	dleaf_t		*leaf_p;

// emit a leaf
	leaf_p = &dleafs[numleafs];
	numleafs++;

	leaf_p->contents = node->contents;

//
// write bounding box info
//
	VectorCopy (node->mins, leaf_p->mins);
	VectorCopy (node->maxs, leaf_p->maxs);

	leaf_p->visofs = -1;	// no vis info yet

//
// write the marksurfaces
//
	leaf_p->firstmarksurface = nummarksurfaces;

	for (fp = node->markfaces ; *fp ; fp++)
	{
	// emit a marksurface
		if (nummarksurfaces == MAX_MAP_MARKSURFACES)
			COM_Error ("nummarksurfaces == MAX_MAP_MARKSURFACES");
		f = *fp;
		do
		{
			dmarksurfaces[nummarksurfaces] =  f->outputnumber;
			nummarksurfaces++;
			f = f->original;	// grab tjunction split faces
		} while (f);
	}

	leaf_p->nummarksurfaces = nummarksurfaces - leaf_p->firstmarksurface;
}


/*
==================
WriteDrawNodes_r
==================
*/
static void WriteDrawNodes_r (node_t *node)
{
	dnode_t	*n;
	int		i;

// emit a node
	if (numnodes == MAX_MAP_NODES)
		COM_Error ("numnodes == MAX_MAP_NODES");
	n = &dnodes[numnodes];
	numnodes++;

	VectorCopy (node->mins, n->mins);
	VectorCopy (node->maxs, n->maxs);

	n->planenum = node->outputplanenum;
	n->firstface = node->firstface;
	n->numfaces = node->numfaces;

//
// recursively output the other nodes
//

	for (i = 0 ; i < 2 ; i++)
	{
		if (node->children[i]->planenum == -1)
		{
			if (node->children[i]->contents == CONTENTS_SOLID)
				n->children[i] = -1;
			else
			{
				n->children[i] = -(numleafs + 1);
				WriteLeaf (node->children[i]);
			}
		}
		else
		{
			n->children[i] = numnodes;
			WriteDrawNodes_r (node->children[i]);
		}
	}
}

/*
==================
WriteDrawNodes
==================
*/
void WriteDrawNodes (node_t *headnode)
{
	int		i;
	int		start;
	dmodel_t	*bm;

#if 0
	if (headnode->contents < 0)
		COM_Error ("%s: empty model", __thisfunc__);
#endif

// emit a model
	if (nummodels == MAX_MAP_MODELS)
		COM_Error ("nummodels == MAX_MAP_MODELS");
	bm = &dmodels[nummodels];
	nummodels++;

	bm->headnode[0] = numnodes;
	bm->firstface = firstface;
	bm->numfaces = numfaces - firstface;
	firstface = numfaces;

	start = numleafs;

	if (headnode->contents < 0)
		WriteLeaf (headnode);
	else
		WriteDrawNodes_r (headnode);
	bm->visleafs = numleafs - start;

	for (i = 0 ; i < 3 ; i++)
	{
		bm->mins[i] = headnode->mins[i] + SIDESPACE + 1;	// remove the padding
		bm->maxs[i] = headnode->maxs[i] - SIDESPACE - 1;
	}
// FIXME: are all the children decendant of padded nodes?
}


/*
==================
BumpModel

Used by the clipping hull processes that only need to store headclipnode
==================
*/
void BumpModel (int hullnumber)
{
	dmodel_t	*bm;

// emit a model
	if (nummodels == MAX_MAP_MODELS)
		COM_Error ("nummodels == MAX_MAP_MODELS");
	bm = &dmodels[nummodels];
	nummodels++;

	bm->headnode[hullnumber] = headclipnode;
}

//=============================================================================

typedef struct
{
	char		identification[4];		// should be WAD2
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;			// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];			// must be null terminated
} lumpinfo_t;

typedef struct wadlist_s
{
	struct wadlist_s	*next;
	FILE		*texfile;
	wadinfo_t	wadinfo;
	lumpinfo_t	*lumpinfo;
} wadlist_t;

static wadlist_t	*wadlist = NULL;

static void CleanupName (char *in, char *out)
{
	int		i;

	for (i = 0 ; i < 16 ; i++)
	{
		if (!in[i])
			break;

		out[i] = q_toupper(in[i]);
	}

	for ( ; i < 16 ; i++)
		out[i] = 0;
}


/*
=================
TEX_InitFromWad
=================
*/
static void TEX_InitFromWad (const char *path)
{
	int			i;
	wadlist_t	*wl;

	wl = (wadlist_t *) SafeMalloc (sizeof(wadlist_t));
	wl->next = wadlist;
	wadlist = wl;

	wl->texfile = SafeOpenRead (path);
	SafeRead (wl->texfile, &wl->wadinfo, sizeof(wadinfo_t));
	if (strncmp (wl->wadinfo.identification, "WAD2", 4))
		COM_Error ("%s: %s isn't a wadfile", __thisfunc__, path);
	wl->wadinfo.numlumps = LittleLong(wl->wadinfo.numlumps);
	wl->wadinfo.infotableofs = LittleLong(wl->wadinfo.infotableofs);
	fseek (wl->texfile, wl->wadinfo.infotableofs, SEEK_SET);
	wl->lumpinfo = (lumpinfo_t *) SafeMalloc(wl->wadinfo.numlumps*sizeof(lumpinfo_t));
	SafeRead (wl->texfile, wl->lumpinfo, wl->wadinfo.numlumps*sizeof(lumpinfo_t));

	for (i = 0 ; i < wl->wadinfo.numlumps ; i++)
	{
		CleanupName (wl->lumpinfo[i].name, wl->lumpinfo[i].name);
		wl->lumpinfo[i].filepos = LittleLong(wl->lumpinfo[i].filepos);
		wl->lumpinfo[i].disksize = LittleLong(wl->lumpinfo[i].disksize);
	}
	printf ("Loaded wadfile %s\n", path);
}

static qboolean TEX_InitWads (void)
{
	int		c = 0;
	const char	*path;
	char	fullpath[1024];
	char	*mark, *tmp;

	path = ValueForKey (&entities[0], "wad");
	if (!path || !path[0])
		goto nowad;

	/* qbsp now accepts more than one wad file specified in the value
	 * of the wad key (feature requested by leileilol):  the file
	 * names must be separated by a semicolon.  spaces are allowed,
	 * quoted paths aren't allowed.
	 */
	while (*path == ';' || *path == ' ' || *path == '\t')
		path++;
	if (!path[0])
		goto nowad;

	do
	{
	/* worldcraft uses an absolute path (reported by 'leileilol'):
	 * so don't touch absolute path values, ie. the paths beginning
	 * with a '/' on unix or with a drive specified string like C:
	 * on windows. all other values with relative or no path path
	 * information are prefixed with the project path as usual.
	 */
		if (*path == '/')
			tmp = fullpath;
#  ifdef PLATFORM_WINDOWS
		/* absolute path with drive? */
		else if ( path[1] == ':' &&
			 ((path[0] >= 'A' && path[0] <= 'Z') ||
			  (path[0] >= 'a' && path[0] <= 'z')) )
			tmp = fullpath;
#  endif /* WINDOWS */
		else	/* relative path or no path. */
		{
			strcpy (fullpath, projectpath);
			tmp = strchr (fullpath, '\0');
		}
		mark = tmp;
		while (*path && *path != ';')
			*tmp++ = *path++;
		while (tmp != mark && (tmp[-1] == ' ' || tmp[-1] == '\t'))
			tmp--;
		if (tmp != mark)
		{
			*tmp = '\0';
			TEX_InitFromWad (fullpath);
			c++;
		}
		while (*path == ';' || *path == ' ' || *path == '\t')
			path++;
	} while (*path);

	if (c != 0)
		return true;

  nowad:
	texdatasize = 0;
	printf ("WARNING: no wadfile specified\n");
	return false;
}

/*
==================
LoadLump
==================
*/
static int LoadLump (char *name, byte *dest)
{
	int		i;
	wadlist_t	*wl;
	char	cname[16];

	CleanupName (name, cname);

	for (wl = wadlist; wl; wl = wl->next)
	{
		for (i = 0 ; i < wl->wadinfo.numlumps ; i++)
		{
		//	do a case insensitive search. some wadfiles
		//	doesn't have the texture name in expected case
			if (!q_strcasecmp(cname, wl->lumpinfo[i].name))
			{
				fseek (wl->texfile, wl->lumpinfo[i].filepos, SEEK_SET);
				SafeRead (wl->texfile, dest, wl->lumpinfo[i].disksize);
				return wl->lumpinfo[i].disksize;
			}
		}
	}

	printf ("WARNING: texture %s not found\n", name);
	return 0;
}


/*
==================
AddAnimatingTextures
==================
*/
static void AddAnimatingTextures (void)
{
	int		base;
	int		i, j, k;
	wadlist_t	*wl;
	char	name[32];

	base = nummiptex;

	for (i = 0 ; i < base ; i++)
	{
		if (miptex[i][0] != '+')
			continue;
		strcpy (name, miptex[i]);

		for (j = 0 ; j < 20 ; j++)
		{
			if (j < 10)
				name[1] = '0'+j;
			else
				name[1] = 'A'+j-10;	// alternate animation

		// see if this name exists in the wadfile
		// FIXME: this allows different anims to be found in different
		//	  wadfiles. would that be a problem?
			for (wl = wadlist ; wl ; wl = wl->next)
			{
				for (k = 0 ; k < wl->wadinfo.numlumps ; k++)
				{
				//	do a case insensitive search. some wadfiles
				//	doesn't have the texture name in expected case
					if (!q_strcasecmp(name, wl->lumpinfo[k].name))
					{
						FindMiptex (name);	// add to the miptex list
						goto foundit;
					}
				}
			}
			foundit: ;
		}
	}

	printf ("added %i texture frames\n", nummiptex - base);
}

/*
==================
WriteMiptex
==================
*/
static void WriteMiptex (void)
{
	int		i, len;
	byte	*data;
	dmiptexlump_t	*l;

	if (!TEX_InitWads())
		return;

	AddAnimatingTextures ();

	l = (dmiptexlump_t *)dtexdata;
	data = (byte *)&l->dataofs[nummiptex];
	l->nummiptex = nummiptex;
	for (i = 0 ; i < nummiptex ; i++)
	{
		l->dataofs[i] = data - (byte *)l;
		len = LoadLump (miptex[i], data);
		if (data + len - dtexdata >= MAX_MAP_MIPTEX)
			COM_Error ("Textures exceeded MAX_MAP_MIPTEX");
		if (!len)
			l->dataofs[i] = -1;	// didn't find the texture
		data += len;
	}

	texdatasize = data - dtexdata;
}

//===========================================================================


/*
==================
BeginBSPFile
==================
*/
void BeginBSPFile (void)
{
// edge 0 is not used, because 0 can't be negated
	numedges = 1;

// leaf 0 is common solid with no faces
	numleafs = 1;
	dleafs[0].contents = CONTENTS_SOLID;

	firstface = 0;
}


/*
==================
FinishBSPFile
==================
*/
void FinishBSPFile (void)
{
	printf ("--- FinishBSPFile ---\n");
	printf ("WriteBSPFile: %s\n", bspfilename);

	WriteMiptex ();

	PrintBSPFileSizes ();
	WriteBSPFile (bspfilename);
}

