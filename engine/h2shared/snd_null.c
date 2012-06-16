/*
 * snd_null.c -- include this instead of all the other snd_* files
 * to have no sound code whatsoever
 * $Id: snd_null.c,v 1.4 2007-07-17 16:10:09 sezero Exp $
 *
 * Copyright (C) 1996-1997 Id Software, Inc.
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

#include "quakedef.h"

cvar_t		bgmvolume = {"bgmvolume", "1", CVAR_ARCHIVE};
cvar_t		bgmtype = {"bgmtype", "cd", CVAR_ARCHIVE};	// cd or midi
cvar_t		sfxvolume = {"volume", "0.7", CVAR_ARCHIVE};
cvar_t		precache = {"precache", "1", CVAR_NONE};

 
void S_Init (void)
{
	Cvar_RegisterVariable(&precache);
	Con_Printf("SOUND: disabled at compile time\n");
}

void S_Shutdown (void)
{
}

void S_TouchSound (const char *sample)
{
}

void S_ClearBuffer (void)
{
}

void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
}

void S_StopSound (int entnum, int entchannel)
{
}

sfx_t *S_PrecacheSound (const char *name)
{
	return NULL;
}

void S_ClearPrecache (void)
{
}

void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{	
}

void S_UpdateSoundPos (int entnum, int entchannel, vec3_t origin)
{	
}

void S_StopAllSounds (qboolean clear)
{
}

void S_BeginPrecaching (void)
{
}

void S_EndPrecaching (void)
{
}

void S_ExtraUpdate (void)
{
}

void S_LocalSound (const char *s)
{
}

void S_BlockSound (void)
{
}

void S_UnblockSound (void)
{
}

