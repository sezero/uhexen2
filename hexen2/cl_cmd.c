/*
	cl_cmd.c
	client command forwarding to server

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/cl_cmd.c,v 1.3 2007-02-15 07:16:10 sezero Exp $
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
Cmd_Maplist_f
Prints map filenames to the console
===========
*/
static void Cmd_Maplist_f (void)
{
	int			i, cnt, dups = 0;
	pack_t		*pak;
	searchpath_t	*search;
	char		**maplist = NULL, mappath[MAX_OSPATH];
	char	*findname, *prefix;
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

	// do two runs - first count the number of maps
	// then collect their names into maplist
scanmaps:
	cnt = 0;
	// search through the path, one element at a time
	// either "search->filename" or "search->pak" is defined
	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->pack)
		{
			pak = search->pack;

			for (i = 0; i < pak->numfiles; i++)
			{
				if (strncmp ("maps/", pak->files[i].name, 5) == 0  && 
				    strstr(pak->files[i].name, ".bsp"))
				{
					if (preLen)
					{
						if ( Q_strncasecmp(prefix, pak->files[i].name + 5, preLen) )
						{	// doesn't match the prefix. skip.
							continue;
						}
					}
					if (maplist)
					{
						size_t	len;
						int	dupl = 0, j;
						// add to our maplist
						len = strlen (pak->files[i].name + 5) - 4 + 1;
								// - ".bsp" (-4) +  "\0" (+1)
						for (j = 0 ; j < cnt ; j++)
						{
							if (!Q_strncasecmp(maplist[j], pak->files[i].name + 5, len-1))
							{
								dupl = 1;
								dups++;
								break;
							}
						}
						if (!dupl)
						{
							maplist[cnt] = malloc (len);
							Q_strlcpy ((char *)maplist[cnt] , pak->files[i].name + 5, len);
							cnt++;
						}
					}
					else
						cnt++;
				}
			}
		}
		else
		{	// element is a filename
			snprintf (mappath, sizeof(mappath), search->filename);
			Q_strlcat (mappath, "/maps", sizeof(mappath));
			findname = Sys_FindFirstFile (mappath, "*.bsp");
			while (findname)
			{
				if (preLen)
				{
					if ( Q_strncasecmp(prefix, findname, preLen) )
					{	// doesn't match the prefix. skip.
						findname = Sys_FindNextFile ();
						continue;
					}
				}
				if (maplist)
				{
					size_t	len;
					int	dupl = 0, j;
					// add to our maplist
					len = strlen(findname) - 4 + 1;
					for (j = 0 ; j < cnt ; j++)
					{
						if (!Q_strncasecmp(maplist[j], findname, len-1))
						{
							dupl = 1;
							dups++;
							break;
						}
					}
					if (!dupl)
					{
						maplist[cnt] = malloc (len);
						Q_strlcpy (maplist[cnt], findname, len);
						cnt++;
					}
				}
				else
					cnt++;
				findname = Sys_FindNextFile ();
			}
			Sys_FindClose ();
		}
	}

	if (maplist == NULL)
	{
		// after first run, we know how many maps we have
		// should I use malloc or something else
		Con_Printf ("Found %d maps:\n\n", cnt);
		if (!cnt)
			return;
		maplist = malloc(cnt * sizeof (char *));
		goto scanmaps;
	}

	// sort the list
	qsort (maplist, cnt, sizeof(char *), COM_StrCompare);
	Con_ShowList (cnt, (const char**)maplist);
	if (dups)
		Con_Printf ("\neliminated %d duplicate names\n", dups);
	Con_Printf ("\n");

	// Free memory
	for (i = 0; i < cnt; i++)
		free (maplist[i]);

	free (maplist);
}

void CL_Cmd_Init (void)
{
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer_f);

	Cmd_AddCommand ("maplist", Cmd_Maplist_f);
}

