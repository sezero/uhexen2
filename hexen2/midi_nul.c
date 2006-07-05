/*
	midi_nul.c
	NULL midi driver

	$Id: midi_nul.c,v 1.2 2006-07-05 20:39:25 sezero Exp $
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

