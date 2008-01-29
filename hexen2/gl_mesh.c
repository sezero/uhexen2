/*
	gl_mesh.c
	triangle model functions

	$Id: gl_mesh.c,v 1.20 2008-01-29 10:03:12 sezero Exp $
*/

#include "quakedef.h"

/*
=================================================================

ALIAS MODEL DISPLAY LIST GENERATION

=================================================================
*/

static int		used[8192];

// the command list holds counts and s/t values that are valid for
// every frame
static int		commands[8192];
static int		numcommands;

// all frames will have their vertexes rearranged and expanded
// so they are in the order expected by the command list
static int		vertexorder[8192];
static int		numorder;

static int		stripverts[128];
static int		striptris[128];
static int		stripstverts[128];
static int		stripcount;

/*
================
StripLength
================
*/
static int StripLength (int starttri, int startv)
{
	int			m1, m2;
	int			st1, st2;
	int			j;
	mtriangle_t	*last, *check;
	int			k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripstverts[0] = last->stindex[(startv)%3];

	stripverts[1] = last->vertindex[(startv+1)%3];
	stripstverts[1] = last->stindex[(startv+1)%3];

	stripverts[2] = last->vertindex[(startv+2)%3];
	stripstverts[2] = last->stindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+2)%3];
	st1 = last->stindex[(startv+2)%3];
	m2 = last->vertindex[(startv+1)%3];
	st2 = last->stindex[(startv+1)%3];

	// look for a matching triangle
nexttri:
	for (j = starttri+1, check = &triangles[starttri+1]; j < pheader->numtris; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k = 0; k < 3; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->stindex[k] != st1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;
			if (check->stindex[ (k+1)%3 ] != st2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
			{
				m2 = check->vertindex[ (k+2)%3 ];
				st2 = check->stindex[ (k+2)%3 ];
			}
			else
			{
				m1 = check->vertindex[ (k+2)%3 ];
				st1 = check->stindex[ (k+2)%3 ];
			}

			stripverts[stripcount+2] = check->vertindex[ (k+2)%3 ];
			stripstverts[stripcount+2] = check->stindex[ (k+2)%3 ];
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}

done:
	// clear the temp used flags
	for (j = starttri+1; j < pheader->numtris; j++)
	{
		if (used[j] == 2)
			used[j] = 0;
	}

	return stripcount;
}

/*
===========
FanLength
===========
*/
static int FanLength (int starttri, int startv)
{
	int		m1, m2;
	int		st1, st2;
	int		j;
	mtriangle_t	*last, *check;
	int		k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripstverts[0] = last->stindex[(startv)%3];

	stripverts[1] = last->vertindex[(startv+1)%3];
	stripstverts[1] = last->stindex[(startv+1)%3];

	stripverts[2] = last->vertindex[(startv+2)%3];
	stripstverts[2] = last->stindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+0)%3];
	st1 = last->stindex[(startv+2)%3];
	m2 = last->vertindex[(startv+2)%3];
	st2 = last->stindex[(startv+1)%3];

	// look for a matching triangle
nexttri:
	for (j = starttri+1, check = &triangles[starttri+1]; j < pheader->numtris; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k = 0; k < 3; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->stindex[k] != st1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;
			if (check->stindex[ (k+1)%3 ] != st2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertindex[ (k+2)%3 ];
			st2 = check->stindex[ (k+2)%3 ];

			stripverts[stripcount+2] = m2;
			stripstverts[stripcount+2] = st2;
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}

done:
	// clear the temp used flags
	for (j = starttri+1; j < pheader->numtris; j++)
	{
		if (used[j] == 2)
			used[j] = 0;
	}

	return stripcount;
}


/*
================
BuildTris

Generate a list of trifans or strips
for the model, which holds for all frames
================
*/
static void BuildTris (void)
{
	int		i, j, k;
	int		startv;
	float	s, t;
	int		len, bestlen, besttype;
	int		bestverts[1024];
	int		besttris[1024];
	int		beststverts[1024];
	int		type;

	//
	// build tristrips
	//
	numorder = 0;
	numcommands = 0;
	memset (used, 0, sizeof(used));
	for (i = 0; i < pheader->numtris; i++)
	{
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;

		bestlen = 0;
		besttype = 0;
		for (type = 0 ; type < 2 ; type++)
		{
			for (startv = 0; startv < 3; startv++)
			{
				if (type == 1)
					len = StripLength (i, startv);
				else
					len = FanLength (i, startv);
				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j = 0; j < bestlen+2; j++)
					{
						beststverts[j] = stripstverts[j];
						bestverts[j] = stripverts[j];
					}
					for (j = 0; j < bestlen; j++)
						besttris[j] = striptris[j];
				}
			}
		}

		// mark the tris on the best strip as used
		for (j = 0; j < bestlen; j++)
			used[besttris[j]] = 1;

		if (besttype == 1)
			commands[numcommands++] = (bestlen+2);
		else
			commands[numcommands++] = -(bestlen+2);

		for (j = 0; j < bestlen+2; j++)
		{
			int		tmp;

			// emit a vertex into the reorder buffer
			k = bestverts[j];
			vertexorder[numorder++] = k;

			k = beststverts[j];

			// emit s/t coords into the commands stream
			s = stverts[k].s;
			t = stverts[k].t;

			if (!triangles[besttris[0]].facesfront && stverts[k].onseam)
				s += pheader->skinwidth / 2;	// on back side
			s = (s + 0.5) / pheader->skinwidth;
			t = (t + 0.5) / pheader->skinheight;

		//	*(float *)&commands[numcommands++] = s;
		//	*(float *)&commands[numcommands++] = t;
			// NOTE: 4 == sizeof(int)
			//	   == sizeof(float)
			memcpy (&tmp, &s, 4);
			commands[numcommands++] = tmp;
			memcpy (&tmp, &t, 4);
			commands[numcommands++] = tmp;
		}
	}

	commands[numcommands++] = 0;		// end of list marker
	DEBUG_Printf ("%3i tri %3i vert %3i cmd\n", pheader->numtris, numorder, numcommands);
}


/*
================
GL_MakeAliasModelDisplayLists
================
*/
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr)
{
	int		i, j;
	int		*cmds;
	trivertx_t	*verts;
#if DO_MESH_CACHE
	char	cache[MAX_OSPATH];
	FILE	*f;
	size_t	len;

	//
	// look for a cached version
	//
	q_snprintf (cache, sizeof(cache), "%s/glhexen/", fs_userdir);
	len = strlen(cache);
	COM_StripExtension (m->name + (sizeof("models/") - 1), cache + len, sizeof(cache) - len);
	q_strlcat (cache, ".ms2", sizeof(cache));

	f = fopen (cache, "rb");
	if (f)
	{
		fread (&numcommands, 4, 1, f);
		fread (&numorder, 4, 1, f);
		fread (&commands, numcommands * sizeof(commands[0]), 1, f);
		fread (&vertexorder, numorder * sizeof(vertexorder[0]), 1, f);
		fclose (f);
	}
	else
	{
#endif
		//
		// build it from scratch
		//
		DEBUG_Printf ("meshing %s...\n", m->name);
		BuildTris ();		// trifans or lists

		//
		// save out the cached version in user's directory
		//
#if DO_MESH_CACHE
		f = fopen (cache, "wb");
		if (!f)
		{
			char gldir[MAX_OSPATH];

			q_snprintf (gldir, sizeof(gldir), "%s/glhexen", fs_userdir);
			Sys_mkdir (gldir);
			q_snprintf (gldir, sizeof(gldir), "%s/glhexen/boss", fs_userdir);
			Sys_mkdir (gldir);
			q_snprintf (gldir, sizeof(gldir), "%s/glhexen/puzzle", fs_userdir);
			Sys_mkdir (gldir);
			f = fopen (cache, "wb");
		}

		if (f)
		{
			fwrite (&numcommands, 4, 1, f);
			fwrite (&numorder, 4, 1, f);
			fwrite (&commands, numcommands * sizeof(commands[0]), 1, f);
			fwrite (&vertexorder, numorder * sizeof(vertexorder[0]), 1, f);
			fclose (f);
		}
	}
#endif

	// save the data out

	hdr->poseverts = numorder;

	cmds = (int *) Hunk_AllocName (numcommands * 4, "cmds");
	hdr->commands = (byte *)cmds - (byte *)hdr;
	memcpy (cmds, commands, numcommands * 4);

	verts = (trivertx_t *) Hunk_AllocName (hdr->numposes * hdr->poseverts * sizeof(trivertx_t), "verts");
	hdr->posedata = (byte *)verts - (byte *)hdr;
	for (i = 0; i < hdr->numposes; i++)
	{
		for (j = 0; j < numorder; j++)
			*verts++ = poseverts[i][vertexorder[j]];
	}
}

