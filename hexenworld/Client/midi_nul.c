/*
	midi_nul.c
	NULL midi driver

	$Id: midi_nul.c,v 1.4 2007-09-21 11:05:11 sezero Exp $
*/

#include "quakedef.h"

qboolean MIDI_Init (void)
{
	Con_Printf("MIDI: disabled at compile time\n");
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

