/*
 * midifile.h -- common MIDI constants and structures
 *
 * Compiled using widely available information from:
 * - MIDIFILE:	Tim Thompson, Michael Czeiszperger
 * - playmidi:	Copyright (C) 1994-1996 Nathan I. Laredo
 * - TiMidity:	Copyright (C) 1995 Tuukka Toivonen
 * - pmidi:	Copyright (C) 1999 Steve Ratcliffe
 * - alsa-lib:	Jaroslav Kysela <perex@perex.cz>
 *		Abramo Bagnara <abramo@alsa-project.org>
 *		Takashi Iwai <tiwai@suse.de>
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

/*
-->-- Begin unmodified copy of "MIDI-FORMAT" file by Nathan Laredo -->--

MIDI File Format
================

By Nathan Laredo, last revision: 25 August 1996

NOTE:  THIS IS NOT THE OFFICIAL MIDI FILE FORMAT SPECIFICATION.  THESE
ARE PERSONAL NOTES I WROTE WHEN I FIRST STARTED TO WRITE MY OWN MIDI
FILE READING ROUTINES.   THIS INFORMATION IS PROVIDED IN THE HOPE THAT
IT MAY SAVE SOMEONE ELSE THE AMOUNT OF RESEARCH THAT WENT INTO IT.  I
MAKE NO GUARANTEES OR WARRANTIES PERTAINING TO USEFULNESS OR ACCURACY.

-----------------------------------------------------------------------
(MThd = 0x4d546864)
	(32-bit big endian length = 6) (16-bit big endian format)
	(16-bit big endian tracks) (16-bit big-endian division)

	format is either 0 - one track, 1 - many tracks, one sequence
	or 2 - many tracks with one sequence per track.
	format 1 files should have all tempo changes in the first track

(MTrk = 0x4d54726b)
	(32-bit big endian length of track)

	(variable-length encoded ticks since previous event)
	(Event data -- see below)

	ticks and event data are repeated for length bytes.

MTrk block is repeated for as many tracks as indicated in header.
------------------------------------------------------------------------
Variable length quantities are a series of 7 bit values encoded from
msb to lsb, with the lsb bit 7 clear, all prior have bit 7 set.
------------------------------------------------------------------------
Event data is either midi data (with running status) that is to be
sent to the midi device, a sysex (two types), or a meta-event
-------------------------------------------------------------------------
A sysex event contains information to be sent directly to the midi synth.
It consists of (0xf0) (variable-length encoded length) (data after 0xf0)
or (0xf7) (variable-length encoded length) (all data to be sent)
Sysex messages may or may not be terminated with 0xf7 (EOX), EOX should
not be added automagically when messages are output to a midi synth, as
there may be a following 0xf7-type sysex after a delay that may be
required by the synth.  The 0xf7-type sysex may contain data > 0x7f so
programs shouldn't stop sending when a byte value is > 0x7f.
-------------------------------------------------------------------------
A meta event contaions information such as text, tempo, and key signature.
It consists of (0xff) (type) (variable length encoded deta length) (data)
Text is not usually null-terminated.   You must depend on the length
when reading it.

meta event types range from 0x00 to 0x7f
Type  Contents
====  ========
0x00: (16-bit big endian sequence number)
0x01: (any text)
0x02: (Copyright Message text)
0x03: (Sequence/Track Name text)
0x04: (Instrument Name text)
0x05: (Lyric text)
0x06: (Marker text)
0x07: (Cue-point text)
0x2f: End of Track -- no data for this type
0x51: (24-bit big endian tempo) -- microseconds per midi quarter note
0x54: (hour) (min) (second) (frame) (fractional-frame) - SMPTE track start
0x58: (numerator) (denominator) (clocks per metronome click)
      (32nd notes notated per midi quarter note)  -- Time Signature
      denominator is a power of two, ie 0x03 = 2^3 = 8
0x59: (sharps/flats) (major/minor flag) -- Key Signature
      if sharp key, first byte represents number of sharps
      if flat key, first byte represents negative number of flats
      the major/minor flag is 0 if minor, 1 if major.
0x7f: (Sequencer Specific data) --  I've never used this type, I ignore it.
      I have never seen it used, and If someone would let me know why it
      exists, I would appreciate it.
-----------------------------------------------------------------------------
Many thanks to the authors of the many free midi references available on
ftp.ucsd.edu that I used in creating this file format specification which
I use for my Playmidi package for Linux.

If you find any errors in this document, please email me.

Nathan Laredo -- laredo@gnu.ai.mit.edu

--
(C)1995, 1996 Nathan Laredo
This document may be freely distributed in its original form.

<----<----<----<----<-- end of MIDI-FORMAT file ----<----<----<----<----
*/

#ifndef MIDIFILE_H
#define MIDIFILE_H

/* MIDI status commands */
#define	MIDICMD_NOTE_OFF		0x80
#define	MIDICMD_NOTE_ON			0x90
#define	MIDICMD_NOTE_PRESSURE		0xa0
#define	MIDICMD_KEY_PRESSURE		MIDICMD_NOTE_PRESSURE
#define	MIDICMD_KEY_AFTERTOUCH		MIDICMD_NOTE_PRESSURE
#define	MIDICMD_POLY_PRESS		MIDICMD_NOTE_PRESSURE
#define	MIDICMD_CONTROL			0xb0
#define	MIDICMD_CTRL_CHANGE		MIDICMD_CONTROL
#define	MIDICMD_CONTROLER		MIDICMD_CONTROL
#define	MIDICMD_PGM_CHANGE		0xc0
#define	MIDICMD_PATCH			MIDICMD_PGM_CHANGE
#define	MIDICMD_CHANNEL_PRESSURE	0xd0
#define	MIDICMD_CHANNEL_AFTERTOUCH	MIDICMD_CHANNEL_PRESSURE
#define	MIDICMD_PITCH_BEND		0xe0
#define	MIDICMD_PITCH_WHEEL		MIDICMD_PITCH_BEND
#define	MIDICMD_BENDER			MIDICMD_PITCH_BEND
#define	MIDICMD_SYSEX			0xf0	/* sysex (system exclusive) begin */
/* (0xf1 to 0xff from asoundef.h) */
#define	MIDICMD_MTC_QUARTER		0xf1	/* MTC quarter frame */
#define	MIDICMD_SONG_POS		0xf2
#define	MIDICMD_SONG_SELECT		0xf3
#define	MIDICMD_TUNE_REQUEST		0xf6
#define	MIDICMD_SYSEX_END		0xf7	/* end of sysex */
#define	MIDICMD_CLOCK			0xf8
#define	MIDICMD_START			0xfa
#define	MIDICMD_CONTINUE		0xfb
#define	MIDICMD_STOP			0xfc
#define	MIDICMD_SENSING			0xfe	/* active sensing */
#define	MIDICMD_RESET			0xff

/* 7 bit controllers */
#define	MIDICTL_SUSTAIN			0x40	/* damper_pedal */
#define	MIDICTL_PORTAMENTO		0x41
#define	MIDICTL_SOSTENUTO		0x42
#define	MIDICTL_SOFT_PEDAL		0x43
#define	MIDICTL_LEGATO_FOOTSWITCH	0x44	/* general_4 */
#define	MIDICTL_HOLD2			0x45
/* (0x46 to 0x4f from asoundef.h) */
#define	MIDICTL_SC1_SOUND_VARIATION	0x46
#define	MIDICTL_SC2_TIMBRE		0x47
#define	MIDICTL_SC3_RELEASE_TIME	0x48
#define	MIDICTL_SC4_ATTACK_TIME		0x49
#define	MIDICTL_SC5_BRIGHTNESS		0x4a
#define	MIDICTL_SC6			0x4b
#define	MIDICTL_SC7			0x4c
#define	MIDICTL_SC8			0x4d
#define	MIDICTL_SC9			0x4e
#define	MIDICTL_SC10			0x4f
#define	MIDICTL_GENERAL_PURPOSE5	0x50
#define	MIDICTL_GENERAL_PURPOSE6	0x51
#define	MIDICTL_GENERAL_PURPOSE7	0x52
#define	MIDICTL_GENERAL_PURPOSE8	0x53
/* (0x54, 0x5b from asoundef.h) */
#define	MIDICTL_PORTAMENTO_CONTROL	0x54
#define	MIDICTL_REVERB_DEPTH		0x5b	/* E1 Reverb Depth */
#define	MIDICTL_TREMOLO_DEPTH		0x5c	/* E2 Tremolo Depth */
#define	MIDICTL_CHORUS_DEPTH		0x5d	/* E3 Chorus Depth */
#define	MIDICTL_DETUNE_DEPTH		0x5e	/* E4 Detune Depth */
#define	MIDICTL_PHASER_DEPTH		0x5f	/* E5 Phaser Depth */
/* parameter values */
#define	MIDICTL_DATA_INCREMENT		0x60
#define	MIDICTL_DATA_DECREMENT		0x61
/* parameter selection */
#define	MIDICTL_NONREG_PARM_LSB		0x62	/* Non-registered parameter number */
#define	MIDICTL_NONREG_PARM_MSB		0x63
#define	MIDICTL_REGIST_PARM_LSB		0x64	/* Registered parameter number */
#define	MIDICTL_REGIST_PARM_MSB		0x65
/* (0x78 to 0x7f from asoundef.h) */
#define	MIDICTL_ALL_SOUNDS_OFF		0x78
#define	MIDICTL_RESET_CONTROLLERS	0x79
#define	MIDICTL_LOCAL_CONTROL_SWITCH	0x7a
#define	MIDICTL_ALL_NOTES_OFF		0x7b
#define	MIDICTL_OMNI_OFF		0x7c
#define	MIDICTL_OMNI_ON			0x7d
#define	MIDICTL_MONO1			0x7e
#define	MIDICTL_MONO2			0x7f

/* Standard MIDI Files meta event definitions */
#define	MIDI_META_EVENT			0xff
#define	MIDI_META_SEQUENCE		0x00	/* sequence_number */
#define	MIDI_META_TEXT			0x01	/* text_event */
#define	MIDI_META_COPYRIGHT		0x02
#define	MIDI_META_TRACKNAME		0x03	/* sequence_name */
#define	MIDI_META_INSTRNAME		0x04	/* instrument_name */
#define	MIDI_META_LYRIC			0x05
#define	MIDI_META_MARKER		0x06
#define	MIDI_META_CUE_POINT		0x07

#define	MIDI_META_CHANPREFIX		0x20	/* channel_prefix */
#define	MIDI_META_EOT			0x2f	/* end_of_track */
#define	MIDI_META_TEMPO			0x51	/* set_tempo */
#define	MIDI_META_SMPTE_OFFSET		0x54	/* SMPTE track start */
#define	MIDI_META_TIME			0x58	/* time_signature */
#define	MIDI_META_KEY			0x59	/* key_signature */
#define	MIDI_META_PROP			0x7f	/* sequencer_specific */

/* some more MIDI controller numbers (from asoundef.h) */
#define	MIDICTL_MSB_BANK		0x00	/* bank selection */
#define	MIDICTL_MSB_MODWHEEL		0x01	/* modulation */
#define	MIDICTL_MSB_BREATH		0x02
#define	MIDICTL_MSB_FOOT		0x04
#define	MIDICTL_MSB_PORTAMENTO_TIME	0x05
#define	MIDICTL_MSB_DATA_ENTRY		0x06
#define	MIDICTL_MSB_MAIN_VOLUME		0x07
#define	MIDICTL_MSB_BALANCE		0x08
#define	MIDICTL_MSB_PAN			0x0a	/* panpot */
#define	MIDICTL_MSB_EXPRESSION		0x0b
#define	MIDICTL_MSB_EFFECT1		0x0c
#define	MIDICTL_MSB_EFFECT2		0x0d
#define	MIDICTL_MSB_GENERAL_PURPOSE1	0x10
#define	MIDICTL_MSB_GENERAL_PURPOSE2	0x11
#define	MIDICTL_MSB_GENERAL_PURPOSE3	0x12
#define	MIDICTL_MSB_GENERAL_PURPOSE4	0x13
#define	MIDICTL_LSB_BANK		0x20	/* bank selection */
#define	MIDICTL_LSB_MODWHEEL		0x21	/* modulation */
#define	MIDICTL_LSB_BREATH		0x22
#define	MIDICTL_LSB_FOOT		0x24
#define	MIDICTL_LSB_PORTAMENTO_TIME	0x25
#define	MIDICTL_LSB_DATA_ENTRY		0x26
#define	MIDICTL_LSB_MAIN_VOLUME		0x27
#define	MIDICTL_LSB_BALANCE		0x28
#define	MIDICTL_LSB_PAN			0x2a	/* panpot */
#define	MIDICTL_LSB_EXPRESSION		0x2b
#define	MIDICTL_LSB_EFFECT1		0x2c
#define	MIDICTL_LSB_EFFECT2		0x2d
#define	MIDICTL_LSB_GENERAL_PURPOSE1	0x30
#define	MIDICTL_LSB_GENERAL_PURPOSE2	0x31
#define	MIDICTL_LSB_GENERAL_PURPOSE3	0x32
#define	MIDICTL_LSB_GENERAL_PURPOSE4	0x33

/* miscellaneous definitions */
#define	MIDI_LOWERBYTE(x)		((unsigned char)(x & 0xff))
#define	MIDI_UPPERBYTE(x)		((unsigned char)((x & 0xff00)>>8))

/* header magic */
#define	MIDI_MAGIC_MTHD			0x4d546864	/* MIDI header magic "MThd" */
#define	MIDI_MAGIC_MTRK			0x4d54726b	/* Track header magic "MTrk" */
/* Non-standard MIDI file formats */
#define	MIDI_MAGIC_CTMF			0x43544d46	/* Creative Music File (CMF) */
#define	MIDI_MAGIC_RMID			0x524d4944	/* "RMID" (Microsoft's RMI) */
#define	MIDI_MAGIC_RIFF			0x52494646	/* "RIFF", for Microsoft RMID */

/* Constants for MIDI v1.0. (from asoundef.h) */
#define	MIDI_CHANNELS			16	/* Number of channels per port/cable. */
#define	MIDI_GM_DRUM_CHANNEL		(10-1)	/* Channel number for GM drums. */


/* Pack these structures to byte granularity, because they are data
 * stored in midi files.  With GCC, use __attribute__((__packed__)).
 * With MS Visual C and others, a #pragma pack() equivalent is needed.
 */
#if !defined(__GNUC__)
#define __midi_packed
#else
#define __midi_packed	__attribute__((__packed__))
#endif

#if !defined(__GNUC__)
#pragma pack(push,1)
#endif

struct _midichunk_s
{
	int	tag;	/* MThd or MTrk */
	int	len;	/* for MThd: must be 6, i.e. sizeof midihdr_t */
			/* for MTrk: specifies the size of the track. */
} __midi_packed;

struct _midihdr_s
{
	short	format;
	short	numtracks;
	short	timediv;
} __midi_packed;

typedef struct _midichunk_s	midichunk_t;
typedef struct _midihdr_s	midihdr_t;

#if !defined(__GNUC__)
#pragma pack(pop)
#endif

typedef int _check_midichunk_size[(sizeof(struct _midichunk_s) == 8) * 2 - 1];
typedef int _check_midi_header_size[(sizeof(struct _midihdr_s) == 6) * 2 - 1];

#endif	/* MIDIFILE_H */

