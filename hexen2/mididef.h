/*
	mididef.h
	client midi music functions

	$Id: mididef.h,v 1.6 2007-03-14 21:03:11 sezero Exp $
*/

#ifndef __MIDIDEFS_H
#define __MIDIDEFS_H

qboolean	MIDI_Init (void);
void		MIDI_Cleanup (void);
void		MIDI_Play (const char *Name);
void		MIDI_Stop (void);
void		MIDI_Pause (int mode);
void		MIDI_Loop (int NewValue);
void		MIDI_Update (void);

// modes for MIDI_Pause()
#define	MIDI_TOGGLE_PAUSE	0
#define	MIDI_ALWAYS_RESUME	1
#define	MIDI_ALWAYS_PAUSE	2

#endif	/* __MIDIDEFS_H */

