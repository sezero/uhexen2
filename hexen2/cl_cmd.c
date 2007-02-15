/*
	cl_cmd.c
	client command forwarding to server

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/cl_cmd.c,v 1.4 2007-02-15 19:24:37 sezero Exp $
*/

#include "quakedef.h"
#include "pakfile.h"	/* for the maplist cmd */

/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer (void)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	SZ_Print (&cls.message, Cmd_Argv(0));

	if (Cmd_Argc() > 1)
	{
		SZ_Print (&cls.message, " ");
		SZ_Print (&cls.message, Cmd_Args());
	}
}

// This is the command variant of the above. The only difference
// is that it doesn't forward the first argument, which is "cmd"
void Cmd_ForwardToServer_f (void)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return;		// not really connected

	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte (&cls.message, clc_stringcmd);
		SZ_Print (&cls.message, Cmd_Args());
	}
}

/*
===========
processMapname

Callback for Cmd_Maplist_f. Returns 0 if a name is skipped, the
current number of names added to the list if the name is added,
or -1 upon failures.
===========
*/
#define MAX_NUMMAPS	256	/* max number of maps to list */
static int		map_count = 0;
static char		*maplist[MAX_NUMMAPS];

static int processMapname (const char *mapname, const char *partial, size_t len_partial, qboolean from_pak)
{
	size_t			len;
	int			j;
	char	cur_name[MAX_QPATH];

	if (map_count >= MAX_NUMMAPS)
	{
		Con_Printf ("WARNING: reached maximum number of maps to list\n");
		return -1;
	}

	if ( len_partial )
	{
		if ( Q_strncasecmp(partial, mapname, len_partial) )
			return 0;	// doesn't match the prefix. skip.
	}

	Q_strlcpy (cur_name, mapname, sizeof(cur_name));
	len = strlen(cur_name) - 4;	// ".bsp" : 4
	if ( from_pak )
	{
		if ( strcmp(cur_name + len, ".bsp") )
			return 0;
	}

	cur_name[len] = 0;
	if ( !cur_name[0] )
		return 0;

	for (j = 0; j < map_count; j++)
	{
		if ( !Q_strcasecmp(maplist[j], mapname) )
			return 0;	// duplicated name. skip.
	}

	// add to the maplist
	maplist[map_count] = malloc (len+1);
	if (maplist[map_count] == NULL)
	{
		Con_Printf ("WARNING: Failed allocating memory for maplist\n");
		return -1;
	}

	Q_strlcpy (maplist[map_count], mapname, len+1);
	return (++map_count);
}

/*
===========
Cmd_Maplist_f
Prints map filenames to the console
===========
*/
static void Cmd_Maplist_f (void)
{
	searchpath_t	*search;
	char		*prefix;
	size_t		preLen;

	if (Cmd_Argc() > 1)
	{
		prefix = Cmd_Argv(1);
		preLen = strlen(prefix);
	}
	else
	{
		preLen = 0;
		prefix = NULL;
	}

	// search through the path, one element at a time
	// either "search->filename" or "search->pak" is defined
	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->pack)
		{
			int			i;

			for (i = 0; i < search->pack->numfiles; i++)
			{
				if ( strncmp("maps/", search->pack->files[i].name, 5) )
					continue;
				if ( processMapname(search->pack->files[i].name + 5, prefix, preLen, true) < 0 )
					goto done;
			}
		}
		else
		{	// element is a filename
			char		*findname;

			findname = Sys_FindFirstFile (va("%s/maps",search->filename), "*.bsp");
			while (findname)
			{
				if ( processMapname(findname, prefix, preLen, false) < 0 )
				{
					Sys_FindClose ();
					goto done;
				}
				findname = Sys_FindNextFile ();
			}
			Sys_FindClose ();
		}
	}

done:
	if (!map_count)
	{
		Con_Printf ("No maps found.\n\n");
		return;
	}
	else
	{
		Con_Printf ("Found %d maps:\n\n", map_count);
	}

	// sort the list
	qsort (maplist, map_count, sizeof(char *), COM_StrCompare);
	Con_ShowList (map_count, (const char**)maplist);
	Con_Printf ("\n");

	// free the memory and zero map_count
	while (map_count)
	{
		free (maplist[--map_count]);
	}
}

void CL_Cmd_Init (void)
{
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer_f);

	Cmd_AddCommand ("maplist", Cmd_Maplist_f);
}

