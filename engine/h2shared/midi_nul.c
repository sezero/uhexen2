/*
	midi_nul.c
	NULL midi driver

	$Id$
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

