/*
	midi_nul.c
	NULL midi driver

	$Id$
*/

#include "quakedef.h"

qboolean MIDI_Init (void)
{
	Con_Printf("MIDI_DRV: disabled at compile time.\n");
	return false;
}

void MIDI_Play(const char *Name)
{
}

void MIDI_Pause(int mode)
{
}

void MIDI_Loop(int mode)
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

