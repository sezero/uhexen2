/*
 * midi_drv.h - MIDI driver exports for BGM interface
 * $Id$
 *
 * Copyright (C) 1999-2005 Id Software, Inc.
 * Copyright (C) 2010-2012 O.Sezer <sezero@users.sourceforge.net>
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
 *
 */

#ifndef _MIDI_DRV_H_
#define _MIDI_DRV_H_

typedef qboolean (*MIDI_INIT)(void);
typedef void (*MIDI_SHUTDOWN)(void);
typedef void *(*MIDI_OPEN)(const char *);
typedef void (*MIDI_ADVANCE)(void **);
typedef void (*MIDI_REWIND)(void **);
typedef void (*MIDI_CLOSE)(void **);
typedef void (*MIDI_PAUSE)(void **);
typedef void (*MIDI_RESUME)(void **);
typedef void (*MIDI_SETVOL)(void **, float);

typedef struct midi_driver_s
{
	qboolean available;
	const char *desc;
	MIDI_INIT mididrv_initialize;
	MIDI_SHUTDOWN mididrv_shutdown;
	MIDI_OPEN mididrv_open;
	MIDI_ADVANCE mididrv_advance;
	MIDI_REWIND mididrv_rewind;
	MIDI_CLOSE mididrv_close;
	MIDI_PAUSE mididrv_pause;
	MIDI_RESUME mididrv_resume;
	MIDI_SETVOL mididrv_setvol;
	struct midi_driver_s *next;
} midi_driver_t;

#endif	/* _MIDI_DRV_H_ */

