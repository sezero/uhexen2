/*
	entmap.c
	a program for remapping entity key/value pairs

	$Id: entmap.c,v 1.6 2007-05-13 12:00:12 sezero Exp $
*/

#include "util_inc.h"
#include "cmdlib.h"
#include "util_io.h"
#include "pathutil.h"
#include "mathlib.h"
#include "bspfile.h"
#include "bsp5.h"


#define	MAXTOKEN	128

static char	token[MAXTOKEN];
static qboolean	unget;
static char	*script_p;
static int		scriptline;
static FILE	*f;

static void StartTokenParsing (char *data)
{
	scriptline = 1;
	script_p = data;
	unget = false;
}

static qboolean GetToken (qboolean crossline)
{
	char	*token_p;

	if (unget)	// is a token already waiting?
	{
		unget = false;
		return true;
	}

//
// skip space
//
skipspace:
	while (*script_p <= 32)
	{
		if (!*script_p)
		{
			if (!crossline)
				Error ("Line %i is incomplete",scriptline);
			return false;
		}
		if (*script_p++ == '\n')
		{
			if (!crossline)
				Error ("Line %i is incomplete",scriptline);
			scriptline++;
		}
	}

	if (script_p[0] == '/' && script_p[1] == '/')	// comment field
	{
		if (!crossline)
			Error ("Line %i is incomplete\n",scriptline);
		while (*script_p++ != '\n')
			if (!*script_p)
			{
				if (!crossline)
					Error ("Line %i is incomplete",scriptline);
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
				Error ("EOF inside quoted token");
			*token_p++ = *script_p++;
			if (token_p == &token[MAXTOKEN])
				Error ("Token too large on line %i",scriptline);
		}
		script_p++;
	}
	else while ( *script_p > 32 )
	{
		*token_p++ = *script_p++;
		if (token_p == &token[MAXTOKEN])
			Error ("Token too large on line %i",scriptline);
	}

	*token_p = 0;

	return true;
}

static void UngetToken (void)
{
	unget = true;
}


//============================================================================

static int		num_entities;
static entity_t	entity;
static entity_t	*ent = &entity;

#if 0
static char *ValueForKey (entity_t *ent, char *key)
{
	epair_t	*ep;

	for (ep = ent->epairs ; ep ; ep = ep->next)
		if (!strcmp (ep->key, key) )
			return ep->value;
	return "";
}
#endif

/*
=================
MapEntity
=================
*/
static void MapEntity (void)
{
#if 0
	char	*class;

	class = ValueForKey (ent, "classname");

	if (!strcmp(class, "path_runcorner")
		|| !strcmp(class, "path_stand")
		|| !strcmp(class, "path_walkcorner")
		|| !strcmp(class, "path_bow") )
	{
		printf ("remapped %s\n", class);
		strcpy (class, "path_corner");
		return;
	}
#endif
}

/*
=================
ParseEpair
=================
*/
static void ParseEpair (void)
{
	epair_t	*e;

	e = malloc (sizeof(epair_t));
	memset (e, 0, sizeof(epair_t));
	e->next = ent->epairs;
	ent->epairs = e;

	strcpy (e->key, token);
	GetToken (false);
	strcpy (e->value, token);
}

/*
=================
CopyBrush

Just copy the brush to the output file
=================
*/
static qboolean CopyBrush (void)
{
	int		i, j;
	int		pts[3][3];
	char	texname[32];
	int		sofs, tofs, flags;
	int		scale[2], shift[2], rotate;

	fprintf (f,"{\n");
	while (1)
	{
	//
	// read it in the old way
	//
		GetToken (true);
		if (token[0] == '}')
			break;
		UngetToken ();

		for (i = 0 ; i < 3 ; i++)
		{
			GetToken (false);
			if (token[0] != '(')
				return false; //Error ("%s: couldn't parse", __thisfunc__);
			for (j = 0 ; j < 3 ; j++)
			{
				GetToken (false);
				pts[i][j] = atoi(token);
			}
			GetToken (false);
			if (token[0] != ')')
				return false; //Error ("%s: couldn't parse", __thisfunc__);
		}

		GetToken (false);
		strcpy (texname, token);
		GetToken (false);
		sofs = atoi(token);
		GetToken (false);
		tofs = atoi(token);
		GetToken (false);
		flags = atoi(token);

//
// convert to new definitions
//
		flags &= 7;

		shift[0] = sofs;
		shift[1] = tofs;

		rotate = 0;
		scale[0] = 1;
		scale[1] = 1;

#define	TEX_FLIPAXIS		1
#define	TEX_FLIPS		2
#define	TEX_FLIPT		4

		if (flags & TEX_FLIPAXIS)
		{
			rotate = 90;
			if ( !(flags & TEX_FLIPT) )
				scale[0] = -1;
			if (flags & TEX_FLIPS)
				scale[1] = -1;
		}
		else
		{
			if (flags & TEX_FLIPS)
				scale[0] = -1;
			if (flags & TEX_FLIPT)
				scale[1] = -1;
		}

	//
	// write it out the new way
	//
		for (i = 0 ; i < 3 ; i++)
			fprintf (f, "( %d %d %d ) ", pts[i][0], pts[i][1], pts[i][2]);
		fprintf (f, "%s %d %d %d %d %d\n", texname, shift[0], shift[1], rotate, scale[0], scale[1]);
	}

	fprintf (f,"}\n");

	return true;
}

/*
================
ParseEntity
================
*/
static int ParseEntity (void)
{
	epair_t	*ep;

	if (!GetToken (true))
		return false;

	if (strcmp (token, "{") )
		Error ("%s: { not found", __thisfunc__);

	fprintf (f,"{\n");

	ent->epairs = NULL;

	do
	{
		if (!GetToken (true))
			Error ("%s: EOF without closing brace", __thisfunc__);
		if (!strcmp (token, "}") )
			break;
		if (!strcmp (token, "{") )
		{
			if (!CopyBrush ())
				return -1;
		}
		else
			ParseEpair ();
	} while (1);

// map the epairs
	MapEntity ();

// write the epairs
	for (ep = ent->epairs ; ep ; ep = ep->next)
		fprintf (f,"\"%s\" \"%s\"\n", ep->key, ep->value);

	fprintf (f,"}\n");

	return true;
}

/*
================
Remap
================
*/
static void Remap (char *filename)
{
	char	*buf;
	char	backname[1024];
	char	workname[1024];
	int		r;

	printf ("--------------\n%s\n", filename);

	LoadFile (filename, (void **)&buf);
	strcpy (backname, filename);
	StripExtension (backname);
	strcpy (workname, backname);
	strcat (backname, ".old");
	strcat (workname, ".tmp");

	remove (workname);

	f = fopen (workname, "w");
	if (!f)
		Error ("Couldn't write to %s", backname);

	StartTokenParsing (buf);

	num_entities = 0;

	while (1)
	{
		r = ParseEntity ();
		if (r != 1)
			break;
		num_entities++;
	}

	free (buf);

	fclose (f);

	if (r == -1)
	{
		printf ("Error parsing %s\n", filename);
		return;
	}

	printf ("%5i entities\n", num_entities);
	remove (backname);
	rename (filename, backname);
	rename (workname, filename);
}


int main (int argc, char **argv)
{
	int		i;

	if (argc < 2)
		Error ("USAGE: entmap [mapfile]\nRenames the map to .old, then remaps entity values");

	for (i = 1 ; i < argc ; i++)
		Remap (argv[i]);

	exit (0);
}

