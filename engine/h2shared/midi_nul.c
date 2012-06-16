/*
 * midi_nul.c -- NULL midi driver
 * $Id$
 *
 * Copyright (C) 2006-2012 O.Sezer <sezero@users.sourceforge.net>
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
#include "bgmusic.h"
#include "midi_drv.h"

qboolean MIDI_Init (void)
{
	/* don't bother doing BGM_RegisterMidiDRV() */
	Con_Printf("MIDI_DRV: disabled at compile time.\n");
	return false;
}

void MIDI_Cleanup(void)
{
}

