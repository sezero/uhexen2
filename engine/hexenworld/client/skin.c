/*
	skin.c
	skin loading

	$Id$
*/

#include "quakedef.h"

cvar_t		baseskin = {"baseskin", "base", CVAR_NONE};
cvar_t		noskins = {"noskins", "1", CVAR_NONE};

static char	allskins[128];
#define	MAX_CACHED_SKINS	128
static skin_t	skins[MAX_CACHED_SKINS];
static int		numskins;


/*
================
Skin_Find

Determines the best skin for the given scoreboard
slot, and sets scoreboard->skin
================
*/
void Skin_Find (player_info_t *sc)
{
	skin_t		*skin;
	int			i;
	char		name[128];
	const char	*s;

	if (allskins[0])
		strcpy (name, allskins);
	else
	{
		s = Info_ValueForKey (sc->userinfo, "skin");
		if (s[0])
			strcpy (name, s);
		else
			strcpy (name, baseskin.string);
	}

	if (strstr (name, "..") || *name == '.')
		strcpy (name, "base");

	COM_StripExtension (name, name, sizeof(name));

	for (i = 0; i < numskins; i++)
	{
		if (!strcmp (name, skins[i].name))
		{
			sc->skin = &skins[i];
			Skin_Cache (sc->skin);
			return;
		}
	}

	if (numskins == MAX_CACHED_SKINS)
	{	// ran out of spots, so flush everything
		Skin_Skins_f ();
		return;
	}

	skin = &skins[numskins];
	sc->skin = skin;
	numskins++;

	memset (skin, 0, sizeof(*skin));
	strcpy (skin->name, name);
}


/*
==========
Skin_Cache

Returns a pointer to the skin bitmap, or NULL to use the default
==========
*/
byte	*Skin_Cache (skin_t *skin)
{
	char	name[1024];
	byte	*raw;
	byte	*out, *pix;
	pcx_t	*pcx;
	int		x, y;
	int		dataByte;
	int		runLength;

	if (cls.downloadtype == dl_skin)
		return NULL;		// use base until downloaded

	if (noskins.integer == 1)	// JACK: So NOSKINS > 1 will show skins, but
		return NULL;		//	 not download new ones.

	if (skin->failedload)
		return NULL;

	out = (byte *) Cache_Check (&skin->cache);
	if (out)
		return out;

//
// load the pic from disk
//
	q_snprintf (name, sizeof(name), "skins/%s.pcx", skin->name);
	raw = FS_LoadTempFile (name, NULL);
	if (!raw)
	{
		Con_Printf ("Couldn't load skin %s\n", name);
		q_snprintf (name, sizeof(name), "skins/%s.pcx", baseskin.string);
		raw = FS_LoadTempFile (name, NULL);
		if (!raw)
		{
			skin->failedload = true;
			return NULL;
		}
	}

//
// parse the PCX file
//
	pcx = (pcx_t *)raw;
	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 320
		|| pcx->ymax >= 200)
	{
		skin->failedload = true;
		Con_Printf ("Bad skin %s\n", name);
		return NULL;
	}

	out = (byte *) Cache_Alloc (&skin->cache, 320*200, skin->name);
	if (!out)
		Sys_Error ("%s: couldn't allocate", __thisfunc__);

	pix = out;
	memset (out, 0, 320*200);

	for (y = 0; y < pcx->ymax; y++, pix += 320)
	{
		for (x = 0; x <= pcx->xmax ; )
		{
			if ((size_t)(raw - (byte*)pcx) > fs_filesize)
			{
				Cache_Free (&skin->cache);
				skin->failedload = true;
				Con_Printf ("Skin %s was malformed.  You should delete it.\n", name);
				return NULL;
			}
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				if ((size_t)(raw - (byte*)pcx) > fs_filesize)
				{
					Cache_Free (&skin->cache);
					skin->failedload = true;
					Con_Printf ("Skin %s was malformed.  You should delete it.\n", name);
					return NULL;
				}
				dataByte = *raw++;
			}
			else
				runLength = 1;

			// skin sanity check
			if (runLength + x > pcx->xmax + 2)
			{
				Cache_Free (&skin->cache);
				skin->failedload = true;
				Con_Printf ("Skin %s was malformed.  You should delete it.\n", name);
				return NULL;
			}
			while (runLength-- > 0)
				pix[x++] = dataByte;
		}
	}

	if ((size_t)(raw - (byte *)pcx) > fs_filesize)
	{
		Cache_Free (&skin->cache);
		skin->failedload = true;
		Con_Printf ("Skin %s was malformed.  You should delete it.\n", name);
		return NULL;
	}

	skin->failedload = false;

	return out;
}


/*
=================
Skin_NextDownload
=================
*/
void Skin_NextDownload (void)
{
	player_info_t	*sc;
	int		i;

	if (cls.downloadnumber == 0)
		Con_Printf ("Checking skins...\n");
	cls.downloadtype = dl_skin;

	for ( ; cls.downloadnumber != MAX_CLIENTS; cls.downloadnumber++)
	{
		sc = &cl.players[cls.downloadnumber];
		if (!sc->name[0])
			continue;
		Skin_Find (sc);
		if (noskins.integer)
			continue;
		if (!CL_CheckOrDownloadFile(va("skins/%s.pcx", sc->skin->name)))
			return;		// started a download
	}

	cls.downloadtype = dl_none;

	// now load them in for real
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		sc = &cl.players[i];
		if (!sc->name[0])
			continue;
		Skin_Cache (sc->skin);
#ifdef GLQUAKE
		sc->skin = NULL;
#endif
	}

	if (cls.state != ca_active)
	{	// get next signon phase
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, va("begin %i", cl.servercount));
		Cache_Report ();	// print remaining memory
	}
}


/*
==========
Skin_Skins_f

Refind all skins, downloading if needed.
==========
*/
void	Skin_Skins_f (void)
{
	int		i;

	for (i = 0; i < numskins; i++)
	{
		if (skins[i].cache.data)
			Cache_Free (&skins[i].cache);
	}
	numskins = 0;

	if (cls.state == ca_disconnected)
	{
		Con_Printf("%s: no connection to a server.\n", __thisfunc__);
		return;
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_skin;
	Skin_NextDownload ();
}


/*
==========
Skin_AllSkins_f

Sets all skins to one specific one
==========
*/
void	Skin_AllSkins_f (void)
{
	strcpy (allskins, Cmd_Argv(1));

	Skin_Skins_f ();
}

