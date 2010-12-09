/*
	mididef.h
	client midi music functions

	$Id: mididef.h,v 1.12 2007-09-21 11:05:11 sezero Exp $
*/

#ifndef __MIDIDEFS_H
#define __MIDIDEFS_H

qboolean	MIDI_Init (void);
void		MIDI_Cleanup (void);
void		MIDI_Play (const char *Name);
void		MIDI_Stop (void);
void		MIDI_Pause (int mode);
void		MIDI_Loop (int mode);
void		MIDI_Update (void);

// modes for MIDI_Pause()
#define	MIDI_TOGGLE_PAUSE	0
#define	MIDI_ALWAYS_RESUME	1
#define	MIDI_ALWAYS_PAUSE	2

// modes for MIDI_Loop()
#define	MIDI_DISABLE_LOOP	0
#define	MIDI_ENABLE_LOOP	1
#define	MIDI_TOGGLE_LOOP	2

#endif	/* __MIDIDEFS_H */

