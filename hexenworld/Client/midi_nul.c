/*
	midi_sdl.c
	NULL midi driver

	$Id: midi_nul.c,v 1.1 2006-06-29 23:02:02 sezero Exp $
*/

#include "quakedef.h"

qboolean MIDI_Init (void)
{
	Con_Printf("MIDI: disabled at compile time\n");
	return false;
}

void MIDI_Play(char *Name)
{
}

void MIDI_Pause(int mode)
{
}

void MIDI_Loop(int NewValue)
{
}

void MIDI_Stop(void)
{
}

void MIDI_Cleanup(void)
{
}

void MIDI_Update(void)
{
}

