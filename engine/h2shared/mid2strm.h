/*
 * mid2strm.h -- Convert MIDI data to a MIDI stream for playback using
 * the Windows midiStream API.
 *
 * Originally from Hexen II source (C) Raven Software Corp.
 * based on an old DirectX5 sample code.
 * Few bits from Doom Legacy: Copyright (C) 1998-2000 by DooM Legacy Team.
 * Multiple fixes and cleanups and adaptation into new Hammer of Thyrion
 * (uHexen2) code by O.Sezer:
 * Copyright (C) 2006-2011 O.Sezer <sezero@users.sourceforge.net>
 *
 * $Id$
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

#ifndef MID2STRM_H
#define MID2STRM_H

#define TRACK_BUFFER_SIZE	2048
#define OUT_BUFFER_SIZE		2048	/* max stream buffer size in bytes */
#define BUFFER_TIME_LENGTH	2000	/* amount to fill, in milliseconds */

#define NUM_STREAM_BUFFERS	5

#define DEBUG_CALLBACK_TIMEOUT	2000	/* wait 2 seconds for callback */


#define CONVERTF_RESET			0x00000001

#define CONVERTF_STATUS_DONE		0x00000001
#define CONVERTF_STATUS_STUCK		0x00000002
#define CONVERTF_STATUS_GOTEVENT	0x00000004

#define CONVERTERR_NOERROR	0	/* no errors */
#define CONVERTERR_CORRUPT	-101	/* input file is corrupt */
#define CONVERTERR_STUCK	-102	/* converter has already encountered a corrupt */
						/* file and can not convert any more
						 * of it. you must reset the converter. */
#define CONVERTERR_DONE		-103	/* converter is done */
#define CONVERTERR_BUFFERFULL	-104	/* the buffer is full */
#define CONVERTERR_METASKIP	-105	/* skipping unknown meta event */

#define STATUS_KILLCALLBACK	100	/* signals that the callback should die */
#define STATUS_CALLBACKDEAD	200	/* signals callback is done processing */
#define STATUS_WAITINGFOREND	300	/* callback is waiting for buffers to play */

#define VOL_CACHE_INIT		100


/* Temporary event structure which stores event data
 * until we're ready to dump it into a stream buffer */
typedef struct _temp_event_s
{
	DWORD	event_time;	/* absolute tick time of event */
	BYTE	shortdata[4];	/* event type and parameters if channel msg */
	DWORD	event_len;	/* length of data which follows if meta or sysex */
	BYTE	*longdata;	/* -> event data if applicable */
} temp_event_t;


/* state of a track open for read */
typedef struct _track_state_s
{
	DWORD	status;		/* track status */
	DWORD	length;		/* total bytes in track */
	DWORD	left_in_buffer;	/* bytes left unread in track buffer */
	BYTE	*start_ptr;	/* -> start of track data buffer */
	BYTE	*current_ptr;	/* -> next byte to read in buffer */
	DWORD	next_event_time;/* absolute time of next event in track */
	BYTE	running_status;	/* running status from last channel msg */

	DWORD	start_ofs;	/* start of track. for walking the file */
	DWORD	nextread_ofs;	/* file offset of next read from disk */
	DWORD	left_on_disk;	/* bytes left unread on disk */
} track_state_t;

#define ITS_F_ENDOFTRK	0x00000001


/* state of the input MIDI file */
typedef struct _midi_filestate_s
{
	DWORD	length;		/* total bytes in file */
	DWORD	timediv;	/* original time division */
	DWORD	format;		/* original format */
	DWORD	numtracks;	/* tracks count */
	track_state_t *tracks;	/* -> array of tracks in this file */
} midi_filestate_t;

extern midi_filestate_t	mfs;


/* convert_buf_t is used to pass information to the ConvertToBuffer ()
 * and then internally by that function to send information about the
 * target stream buffer and current state of the conversion process to
 * lower level conversion routines. */
typedef struct _convert_buf_s
{
	MIDIHDR	mh;		/* windows stream buffer header */
	BOOL	prepared;	/* midiOutPrepareHeader() done? */
	DWORD	start_ofs;	/* start offset from mhBuffer.lpData */
	DWORD	maxlen;		/* max length to convert on this pass */

	DWORD	bytes_in;	/* number of bytes put into this buf. */
	DWORD	starttime;
	BOOL	times_up;
} convert_buf_t;

int ConverterInit (const char *filename);
void ConverterCleanup (void);
int ConvertToBuffer (unsigned int flags, convert_buf_t *buf);

#endif /* MID2STRM_H */

