/*
 * mid2strm.c -- Convert MIDI data to a MIDI stream for playback using
 * the Windows midiStream API.
 *
 * Originally from Hexen II source (C) Raven Software Corp.
 * based on an old DirectX5 sample code.
 * Few bits from Doom Legacy: Copyright (C) 1998-2000 by DooM Legacy Team.
 * Multiple fixes and cleanups and adaptation into new Hammer of Thyrion
 * (uHexen2) code by O.Sezer:
 * Copyright (C) 2006-2012 O.Sezer <sezero@users.sourceforge.net>
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

#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <stdio.h>

#include "mid2strm.h"
#include "midifile.h"
#include "quakedef.h"
#include "bgmusic.h"

midi_filestate_t	mfs;

static DWORD	currenttime;
static fshandle_t	midi_fh;

static DWORD	buffer_tick_len;

static const char err_bad_midi_file[]	= "Read error on input file or file is corrupt.";
#ifdef DEBUG_BUILD
static const char err_add_event[]	= "Unable to add event to stream buffer.";
static const char err_bad_runstat[]	= "Reference to missing running status.";
static const char err_trunc_runstat[]	= "Running status message truncated";
static const char err_trunc_chan_msg[]	= "Channel message truncated";
static const char err_trunc_sysex[]	= "SysEx event truncated";
static const char err_trunc_sysex_len[]	= "SysEx event truncated (length)";
static const char err_meta_noclass[]	= "Meta event truncated (no class byte)";
static const char err_trunc_meta[]	= "Meta event truncated";
static const char err_trunc_meta_len[]	= "Meta event truncated (length)";
#endif

static int AddEventToStreamBuffer (temp_event_t *te, convert_buf_t *);
static int GetInFileData (void *dest, DWORD bytes_wanted);
static int GetTrackByte (track_state_t *ts, BYTE *b);
static int GetTrackEvent (track_state_t *ts, temp_event_t *te);
static int GetTrackVDWord (track_state_t *ts, DWORD *v);
static int RefillTrackBuffer (track_state_t *ts);
static int RewindConverter (void);

#ifndef DEBUG_BUILD
#define TRACKERR(p,msg)
#else
#define TRACKERR(p,msg)	ShowTrackError(p,msg)
static void ShowTrackError (track_state_t *ts, const char *msg)
{
	Con_Printf ("MIDI: %s\n", msg);
	Con_Printf ("Track buffer offset: %lu, total: %lu, left: %lu\n",
			(unsigned long)(ts->current_ptr - ts->start_ptr),
			ts->length, ts->left_in_buffer);
}
#endif

static int MID2STREAM_fileopen(const char *filename)
{
	FILE *handle;
	qboolean pak;
	size_t length;

	length = FS_OpenFile(filename, &handle, NULL);
	pak = file_from_pak;
	if (length == (size_t)-1)
		return -1;

	midi_fh.file = handle;
	midi_fh.start = ftell(handle);
	midi_fh.pos = 0;
	midi_fh.length = (long)length;
	midi_fh.pak = pak;
	return 0;
}

static void MID2STREAM_fileclose(void)
{
	FS_fclose(&midi_fh);
	midi_fh.file = NULL;
}

static long MID2STREAM_seek (long offset, int whence)
{
	if (FS_fseek(&midi_fh, offset, whence) != 0)
		return -1;
	return FS_ftell(&midi_fh);
}

static void MID2STREAM_readfile (void *buffer, DWORD bytes_wanted, DWORD *bytes_read)
{
	size_t nmemb = FS_fread (buffer, 1, bytes_wanted, &midi_fh);
	*bytes_read = nmemb;
}

/* ConverterInit
 *
 * Open the input file
 * Allocate and read the entire input file into memory
 * Validate the input file structure
 * Allocate the input track structures and initialize them
 * Initialize the output track structures
 *
 * Return zero on success
 */
int ConverterInit (const char *filename)
{
	int	err = 1;
	DWORD	bytes_wanted, bytes_read, magic, bytes;
	UINT	idx;
	midihdr_t	header;
	track_state_t	*ts;

	currenttime = 0;

	memset (&mfs, 0, sizeof(midi_filestate_t));
	memset (&midi_fh, 0, sizeof(fshandle_t));

	if (MID2STREAM_fileopen(filename) != 0)
		return 1;

	mfs.length = midi_fh.length;

/* Read and validate MThd header, size of file header chunk
 * and the file header itself.  */
	if (GetInFileData(&magic, sizeof(DWORD)))
		goto Init_Cleanup;
	magic = (DWORD)BigLong(magic);
	if (magic == MIDI_MAGIC_RIFF) /* RMID ?? */
	{
		if (GetInFileData(&bytes, sizeof(DWORD)) != 0 || /* size */
		    GetInFileData(&magic, sizeof(DWORD)) != 0 ||
		    MIDI_MAGIC_RMID != (DWORD)BigLong(magic)  ||
		    GetInFileData(&magic, sizeof(DWORD)) != 0 ||
	/* "data" */0x64617461 != (DWORD)BigLong(magic) ||
		    GetInFileData(&bytes, sizeof(DWORD)) != 0 || /* size */
		    /* SMF must begin from here onwards: */
		    GetInFileData(&magic, sizeof(DWORD)) != 0) {
			goto Init_Cleanup;
		}
		magic = (DWORD)BigLong(magic);
	}
	if (magic != MIDI_MAGIC_MTHD)
		goto Init_Cleanup;
	if (GetInFileData(&bytes, sizeof(DWORD)))
		goto Init_Cleanup;
	if ((bytes = (DWORD)BigLong(bytes)) < sizeof(midihdr_t))
		goto Init_Cleanup;
	if (GetInFileData(&header, bytes))
		goto Init_Cleanup;

/* File header is stored in big endian (hi-lo) order. */
	mfs.format	= (DWORD) BigShort(header.format);
	mfs.numtracks	= (DWORD) BigShort(header.numtracks);
	mfs.timediv	= (DWORD) BigShort(header.timediv);

	if (mfs.format != 0 && mfs.format != 1) /* Type-2 not supported */
		goto Init_Cleanup;
	if (mfs.numtracks == 0)
		goto Init_Cleanup;
	if (mfs.format == 0 && mfs.numtracks != 1)
		goto Init_Cleanup;

/* We know how many tracks there are; allocate structures for them
 * and parse them. The parse merely looks at the MTrk signature and
 * track chunk length in order to skip to the next track header. */
	mfs.tracks = (track_state_t *) Z_Malloc(mfs.numtracks * sizeof(track_state_t), Z_MAINZONE);
	for (idx = 0, ts = mfs.tracks; idx < mfs.numtracks; ++idx, ++ts)
	{
		ts->start_ptr = (BYTE *) Z_Malloc(TRACK_BUFFER_SIZE, Z_MAINZONE);
		if (GetInFileData(&magic, sizeof(magic)))
			goto Init_Cleanup;
		if ((magic = (DWORD)BigLong(magic)) != MIDI_MAGIC_MTRK)
			goto Init_Cleanup;
		if (GetInFileData(&bytes, sizeof(bytes)))
			goto Init_Cleanup;

		bytes = (DWORD)BigLong(bytes);
		ts->length = bytes; /* Total track length */

/* Determine whether all track data will fit into a single one of our track
 * buffers. If not, we need to read in a buffer full and come back for more
 * later, saving the file offset to continue from and the amount left to read
 * in the track structure. */

		/* Save the file offset of the beginning of this track */
		ts->start_ofs = MID2STREAM_seek(0, SEEK_CUR);

		if (ts->length > TRACK_BUFFER_SIZE)
			bytes_wanted = TRACK_BUFFER_SIZE;
		else
			bytes_wanted = ts->length;

		MID2STREAM_readfile(ts->start_ptr, bytes_wanted, &bytes_read);
		if (bytes_read != bytes_wanted)
			goto Init_Cleanup;

		/* Save the number of bytes that didn't make it into the buffer */
		ts->left_on_disk = ts->length - bytes_read;
		ts->left_in_buffer = bytes_read;

		/* Save the current file offset so we can seek to it later */
		ts->nextread_ofs = MID2STREAM_seek(0, SEEK_CUR);

		/* Setup pointer to the current position in the track */
		ts->current_ptr = ts->start_ptr;
		ts->status = 0;
		ts->running_status = 0;
		ts->next_event_time = 0;

		/* Handle bozo MIDI files which contain empty track chunks */
		if (!ts->left_in_buffer && !ts->left_on_disk)
		{
			ts->status |= ITS_F_ENDOFTRK;
			continue;
		}

		/* always preread the time from each track so the mixer code can
		 * determine which track has the next event with minimum work. */
		if (GetTrackVDWord(ts, &ts->next_event_time))
			goto Init_Cleanup;

		/* Step over any unread data, advancing to the beginning of the next
		 * track's data */
		MID2STREAM_seek(ts->start_ofs + ts->length, SEEK_SET);

	}	/* End of track initialization code */

	err = 0;

Init_Cleanup:
	if (err)
	{
		Con_Printf("MIDI: %s\n", err_bad_midi_file);
		ConverterCleanup();
	}

	return err;
}


/* GetInFileData
 *
 * Gets the requested number of bytes of data from the input file.
 * Returns 0 on success, or 1 upon short reads.
 */
static int GetInFileData (void *dest, DWORD bytes_wanted)
{
	DWORD	bytes_read;

	MID2STREAM_readfile(dest, bytes_wanted, &bytes_read);
	return (bytes_read != bytes_wanted);
}

/* ConverterCleanup
 * Free anything we ever allocated
 */
void ConverterCleanup (void)
{
	DWORD	idx;

	MID2STREAM_fileclose();
	if (mfs.tracks)
	{
		for (idx = 0; idx < mfs.numtracks; idx++)
		{
			if (mfs.tracks[idx].start_ptr)
				Z_Free(mfs.tracks[idx].start_ptr);
		}
		Z_Free(mfs.tracks);
		mfs.tracks = NULL;
	}
}

/* RewindConverter
 *
 * An adaptation of the ConverterInit() code which resets the
 * tracks without closing and opening the file.
 */
static int RewindConverter (void)
{
	DWORD	bytes_wanted, bytes_read, idx;
	int	err = 1;
	track_state_t	*ts;

	currenttime = 0;

	for (idx = 0, ts = mfs.tracks; idx < mfs.numtracks; ++idx, ++ts)
	{
/* Determine whether all track data will fit into a single one of our track
 * buffers. If not, we need to read in a buffer full and come back for more
 * later, saving the file offset to continue from and the amount left to read
 * in the track structure. */

		MID2STREAM_seek(ts->start_ofs, SEEK_SET);

		if (ts->length > TRACK_BUFFER_SIZE)
			bytes_wanted = TRACK_BUFFER_SIZE;
		else
			bytes_wanted = ts->length;

		MID2STREAM_readfile(ts->start_ptr, bytes_wanted, &bytes_read);
		if (bytes_read != bytes_wanted)
			goto Rewind_Cleanup;

		/* Save the number of bytes that didn't make it into the buffer */
		ts->left_on_disk = ts->length - bytes_read;
		ts->left_in_buffer = bytes_read;

		/* Save the current file offset so we can seek to it later */
		ts->nextread_ofs = MID2STREAM_seek(0, SEEK_CUR);

		/* Setup pointer to the current position in the track */
		ts->current_ptr = ts->start_ptr;
		ts->status = 0;
		ts->running_status = 0;
		ts->next_event_time = 0;

		/* Handle bozo MIDI files which contain empty track chunks */
		if (!ts->left_in_buffer && !ts->left_on_disk)
		{
			ts->status |= ITS_F_ENDOFTRK;
			continue;
		}

		/* always preread the time from each track so the mixer code can
		 * determine which track has the next event with minimum work. */
		if (GetTrackVDWord(ts, &ts->next_event_time))
			goto Rewind_Cleanup;

		/* Step over any unread data, advancing to the beginning of the next
		 * track's data */
		MID2STREAM_seek(ts->start_ofs + ts->length, SEEK_SET);

	}	/* End of track initialization code */

	err = 0;

Rewind_Cleanup:
	if (err)
		Con_Printf("MIDI: %s\n", err_bad_midi_file);

	return err;
}

/* ConvertToBuffer
 *
 * Converts MIDI data from the track buffers setup by a previous call
 * to ConverterInit().  Converts data until an error is encountered
 * or the output buffer has been filled with as much event data as possible,
 * not to exceed maxlen.
 *
 * Success/failure and the number of output bytes actually converted will
 * be returned in the convert_buf structure.
 */
int ConvertToBuffer (unsigned int flags, convert_buf_t *buf)
{
	static track_state_t	*ts, *found;
	static DWORD		status;
	static DWORD		next_time;
	static temp_event_t	tevent;

	int		err;
	DWORD		idx;

	buf->bytes_in = 0;

	if (flags & CONVERTF_RESET)
	{
		status = 0;
		memset(&tevent, 0, sizeof(struct _temp_event_s));
		ts = found = NULL;
	}

	/* If we were already done, then return with a warning */
	if (status & CONVERTF_STATUS_DONE)
	{
		if (!bgmloop)
			return CONVERTERR_DONE;

		RewindConverter();
		status = 0;
	}
	/* The caller is asking us to continue, but we're already hosed because
	 * we previously identified something as corrupt, so complain louder this
	 * time. */
	else if (status & CONVERTF_STATUS_STUCK)
	{
		return CONVERTERR_STUCK;
	}
	else if (status & CONVERTF_STATUS_GOTEVENT)
	{
		/* Turn off this bit flag */
		status ^= CONVERTF_STATUS_GOTEVENT;

		/* The following code for this case is duplicated from below,
		 * and is designed to handle a "straggler" event, should we
		 * have one left over from previous processing the last time
		 * this function was called.
		 */

		/* Don't add end of track event until we are done */
		if (tevent.shortdata[0] == MIDI_META_EVENT &&
		    tevent.shortdata[1] == MIDI_META_EOT)
		{
			if (tevent.longdata)
			{
				Z_Free(tevent.longdata);
				tevent.longdata = NULL;
			}
		}
		else if ((err = AddEventToStreamBuffer(&tevent, buf)) != CONVERTERR_NOERROR)
		{
			if (err == CONVERTERR_BUFFERFULL)
			{
			/* Do some processing and tell caller that this buffer is full */
				status |= CONVERTF_STATUS_GOTEVENT;
				return CONVERTERR_NOERROR;
			}
			else if (err == CONVERTERR_METASKIP)
			{
			/* We skip by all meta events that aren't tempo changes */
			}
			else
			{
				DEBUG_Printf("MIDI: %s\n", err_add_event);
				if (tevent.longdata)
				{
					Z_Free(tevent.longdata);
					tevent.longdata = NULL;
				}
				return err;
			}
		}
	}

	for ( ; ; )
	{
		found = NULL;
		next_time = ~(DWORD)0;	/* 0xFFFFFFFFL */

		/* Find nearest event due */
		for (idx = 0, ts = mfs.tracks; idx < mfs.numtracks; ++idx, ++ts)
		{
			if (!(ts->status & ITS_F_ENDOFTRK) &&
				ts->next_event_time < next_time)
			{
				next_time = ts->next_event_time;
				found = ts;
			}
		}

		/* None found? We must be done, so return to the caller with a smile. */
		if (!found)
		{
			status |= CONVERTF_STATUS_DONE;
			return CONVERTERR_NOERROR;
		}

		/* Ok, get the event header from that track */
		if (GetTrackEvent(found, &tevent))
		{
			/* Warn future calls that this converter is stuck
			 * at a corrupt spot and can't continue */
			status |= CONVERTF_STATUS_STUCK;
			return CONVERTERR_CORRUPT;
		}

		/* Don't add end of track event 'til we're done */
		if (tevent.shortdata[0] == MIDI_META_EVENT &&
		    tevent.shortdata[1] == MIDI_META_EOT)
		{
			if (tevent.longdata)
			{
				Z_Free(tevent.longdata);
				tevent.longdata = NULL;
			}
			continue;
		}

		if ((err = AddEventToStreamBuffer(&tevent, buf)) != CONVERTERR_NOERROR)
		{
			if (err == CONVERTERR_BUFFERFULL)
			{
			/* Do some processing and tell caller that this buffer is full */
				status |= CONVERTF_STATUS_GOTEVENT;
				return CONVERTERR_NOERROR;
			}
			else if (err == CONVERTERR_METASKIP)
			{
			/* We skip by all meta events that aren't tempo changes */
			}
			else
			{
				DEBUG_Printf("MIDI: %s\n", err_add_event);
				if (tevent.longdata)
				{
					Z_Free(tevent.longdata);
					tevent.longdata = NULL;
				}
				return err;
			}
		}
	}

	return CONVERTERR_NOERROR;	/* not reached. */
}

/* GetTrackVDWord
 *
 * Parse a variable length DWORD from the given track. A VDWord in
 * a MIDI file is in lo-hi format and has the high bit set on every
 * byte except the last.
 *
 * Returns the DWORD in *v and zero on success; else non-zero if we
 * hit end of track. Sets ITS_F_ENDOFTRK if we hit end of track.
 */
static int GetTrackVDWord (track_state_t *ts, DWORD *v)
{
	BYTE	b;
	DWORD	val = 0;

	if (ts->status & ITS_F_ENDOFTRK)
		return 1;

	do
	{
		if (!ts->left_in_buffer && !ts->left_on_disk)
		{
			ts->status |= ITS_F_ENDOFTRK;
			return 1;
		}

		if (GetTrackByte(ts, &b))
			return 1;

		val = (val << 7) | (b & 0x7F);
	} while (b & 0x80);

	*v = val;

	return 0;
}

/* GetTrackEvent
 *
 * Fills in the event struct with the next event from the track
 *
 * te->event_time will contain the absolute tick time of the event.
 *
 * te->shortdata[0] will contain:
 * - MIDI_META_EVENT if the event is a meta event; in this case
 *   te->shortdata[1] will contain the meta class.
 * - MIDICMD_SYSEX or MIDICMD_SYSEX_END if the event is a SysEx event
 * - Otherwise, event is a channel message and te->shortdata[1] and
 *   te->shortdata[2] will contain the rest of the event.
 *
 * te->event_len will contain:
 * - The total length of the channel message in te->shortdata if the
 *   event is a channel message,
 * - The total length of the paramter data pointed to by te->longdata,
 *   otherwise.
 *
 * te->longdata will point at any additional paramters if the event is
 * a SysEx or meta event with non-zero length; else it will be NULL.
 *
 * Returns zero on success or non-zero on any kind of parse error
 * Maintains the state of the input track (i.e. ts->left_in_buffer,
 * ts->pTrackPointers, and ts->running_status).
 */
static int GetTrackEvent (track_state_t *ts, temp_event_t *te)
{
	DWORD	idx;
	BYTE	b;
	UINT	event_len;

	/* Clear out the temporary event structure to get rid of old data */
	memset(te, 0, sizeof(struct _temp_event_s));

	/* Already at end of track? There's nothing to read. */
	if (ts->status & ITS_F_ENDOFTRK)
		return 1;
	if (!ts->left_in_buffer && !ts->left_on_disk)
		return 1;

	/* Get the first byte, which determines the type of event. */
	if (GetTrackByte(ts, &b))
		return 1;

	/* If the high bit is not set, then this is a channel message
	 * which uses the status byte from the last channel message
	 * we saw. NOTE: We do not clear running status across SysEx or
	 * meta events even though the spec says to because there are
	 * actually files out there which contain that sequence of data.
	 */
	if ( !(b & 0x80))
	{
		/* No previous status byte? We're hosed. */
		if (!ts->running_status)
		{
			TRACKERR(ts, err_bad_runstat);
			return 1;
		}

		te->shortdata[0] = ts->running_status;
		te->shortdata[1] = b;

		b = te->shortdata[0] & 0xF0;
		te->event_len = 2;

		/* Only program change and channel pressure events are
		 * 2 bytes long.  the rest are 3 and need another byte. */
		if (b != MIDICMD_PGM_CHANGE && b != MIDICMD_CHANNEL_PRESSURE)
		{
			if (!ts->left_in_buffer && !ts->left_on_disk)
			{
				TRACKERR(ts, err_trunc_runstat);
				ts->status |= ITS_F_ENDOFTRK;
				return 1;
			}

			if (GetTrackByte(ts, &te->shortdata[2]))
				return 1;
			++te->event_len;
		}
	}
	else if ((b & 0xF0) != MIDICMD_SYSEX)
	{
		/* Not running status, not in SysEx range - must be
		 * normal channel message (0x80-0xEF)  */
		te->shortdata[0] = b;
		ts->running_status = b;

		/* Strip off channel and just keep message type */
		b &= 0xF0;

		event_len = (b == MIDICMD_PGM_CHANGE || b == MIDICMD_CHANNEL_PRESSURE) ? 1 : 2;
		te->event_len = event_len + 1;

		if ((ts->left_in_buffer + ts->left_on_disk) < event_len)
		{
			TRACKERR(ts, err_trunc_chan_msg);
			ts->status |= ITS_F_ENDOFTRK;
			return 1;
		}

		if (GetTrackByte(ts, &te->shortdata[1]))
			return 1;

		if (event_len == 2)
		{
			if (GetTrackByte(ts, &te->shortdata[2]))
				return 1;
		}
	}
	else if (b == MIDICMD_SYSEX || b == MIDICMD_SYSEX_END)
	{
		/* One of the SysEx types. (They are the same as far as we're
		 * concerned; there is only a semantic difference in how the
		 * data would actually get sent when the file is played.
		 * We must take care to put the proper event type back on the
		 * output track, however.)
		 *
		 * Parse the general format of:
		 *	BYTE	event (MIDICMD_SYSEX or MIDICMD_SYSEX_END)
		 *	VDWORD	num_parms
		 *	BYTE	ab_parms[num_parms]
		 */
		te->shortdata[0] = b;
		if (GetTrackVDWord(ts, &te->event_len))
		{
			TRACKERR(ts, err_trunc_sysex_len);
			return 1;
		}

		if ((ts->left_in_buffer + ts->left_on_disk) < te->event_len)
		{
			TRACKERR(ts, err_trunc_sysex);
			ts->status |= ITS_F_ENDOFTRK;
			return 1;
		}

		te->longdata = (BYTE *) Z_Malloc(te->event_len, Z_MAINZONE);
		for (idx = 0; idx < te->event_len; idx++)
		{
			if (GetTrackByte(ts, te->longdata + idx))
			{
				Z_Free(te->longdata);
				te->longdata = NULL;
				TRACKERR(ts, err_trunc_sysex);
				return 1;
			}
		}
	}
	else if (b == MIDI_META_EVENT)
	{
		/* It's a meta event. Parse the general form:
		 *	BYTE	event (MIDI_META_EVENT)
		 *	BYTE	class
		 *	VDWORD	num_parms
		 *	BYTE	ab_parms[num_parms]
		 */
		te->shortdata[0] = b;

		if (!ts->left_in_buffer && !ts->left_on_disk)
		{
			TRACKERR(ts, err_meta_noclass);
			ts->status |= ITS_F_ENDOFTRK;
			return 1;
		}

		if (GetTrackByte(ts, &te->shortdata[1]))
			return 1;

		if (GetTrackVDWord(ts, &te->event_len))
		{
			TRACKERR(ts, err_trunc_meta_len);
			return 1;
		}

		/* NOTE: It's perfectly valid to have a meta with no data
		 * In this case, event_len == 0 and longdata == NULL */
		if (te->event_len)
		{
			if ((ts->left_in_buffer + ts->left_on_disk) < te->event_len)
			{
				TRACKERR(ts, err_trunc_meta);
				ts->status |= ITS_F_ENDOFTRK;
				return 1;
			}

			te->longdata = (BYTE *) Z_Malloc(te->event_len, Z_MAINZONE);
			for (idx = 0; idx < te->event_len; idx++)
			{
				if (GetTrackByte(ts, te->longdata + idx))
				{
					Z_Free(te->longdata);
					te->longdata = NULL;
					TRACKERR(ts, err_trunc_meta);
					return 1;
				}
			}
		}

		if (te->shortdata[1] == MIDI_META_EOT)
			ts->status |= ITS_F_ENDOFTRK;
	}
	else
	{
		/* Messages in this range are system messages and aren't
		 * supposed to be in a normal MIDI file. If they are, we
		 * have either misparsed or the authoring software is stupid.
		 */
		return 1;
	}

	/* Event time was already stored as the current track time */
	te->event_time = ts->next_event_time;

	/* Now update to the next event time. The code above MUST properly
	 * maintain the end of track flag in case the end of track meta is
	 * missing.  NOTE: This code is a continuation of the track event
	 * time pre-read which is done at the end of track initialization.
	 */
	if ( !(ts->status & ITS_F_ENDOFTRK))
	{
		DWORD	delta_time;

		if (GetTrackVDWord(ts, &delta_time))
			return 1;

		ts->next_event_time += delta_time;
	}

	return 0;
}

/* GetTrackByte
 *
 * Retrieve the next byte from the track buffer, refilling the buffer
 * from disk if necessary.
 */
static int GetTrackByte (track_state_t *ts, BYTE *b)
{
	if (!ts->left_in_buffer)
	{
		if (RefillTrackBuffer(ts))
			return 1;
	}

	*b = *ts->current_ptr++;
	ts->left_in_buffer--;
	return 0;
}

/* RefillTrackBuffer
 *
 * Attempts to read in a buffer-full of data for a MIDI track.
 */
static int RefillTrackBuffer (track_state_t *ts)
{
	long	ofs;
	DWORD	bytes_wanted, bytes_read;

	if (!ts->left_on_disk)
		return 1;

	ts->current_ptr = ts->start_ptr;

	/* Seek to the proper place in the file, indicated by
	 * ts->nextread_ofs and read in the remaining data, up
	 * to a maximum of the buffer size. */
	ofs = MID2STREAM_seek((long)ts->nextread_ofs, SEEK_SET);
	if (ofs == -1)
	{
		Con_Printf("MIDI: Unable to seek to track buffer location!\n");
		return 1;
	}

	if (ts->left_on_disk > TRACK_BUFFER_SIZE)
		bytes_wanted = TRACK_BUFFER_SIZE;
	else
		bytes_wanted = ts->left_on_disk;

	MID2STREAM_readfile(ts->start_ptr, bytes_wanted, &bytes_read);

	ts->left_on_disk -= bytes_read;
	ts->nextread_ofs = (DWORD)ofs + bytes_read;
	ts->left_in_buffer = bytes_read;

	if (!bytes_read || bytes_read != bytes_wanted)
	{
		Con_Printf("MIDI: Read failed prematurely!\n");
		return 1;
	}

	return 0;
}

/* AddEventToStreamBuffer
 *
 * Put the given event into the given stream buffer at the given location
 * te must point to an event filled out in accordance with the description
 * given in GetTrackEvent()
 *
 * Returns zero on sucess, non-zero on error.
 */
static int AddEventToStreamBuffer (temp_event_t *te, convert_buf_t *buf)
{
	DWORD	delta_time;
	MIDIEVENT	*me;

	me = (MIDIEVENT *)(buf->mh.lpData + buf->start_ofs + buf->bytes_in);

	/* When we see a new, empty buffer, set the start time on it */
	if (!buf->bytes_in)
		buf->starttime = currenttime;

	/* Use the above set start time to figure out how much longer
	 * we should fill this buffer before declaring it as "full" */
	if (currenttime - buf->starttime > buffer_tick_len)
	{
		if (buf->times_up)
		{
			buf->times_up = FALSE;
			return CONVERTERR_BUFFERFULL;
		}

		buf->times_up = TRUE;
	}

	/* Delta time is absolute event time minus absolute time
	 * already gone by on this track */
	delta_time = te->event_time - currenttime;
	/* Event time is now current time on this track */
	currenttime = te->event_time;

	if (te->shortdata[0] < MIDICMD_SYSEX)
	{
		/* Channel message. Need 3 DWORD's: delta-t, stream-ID, event */
		if (buf->maxlen - buf->bytes_in < 3*sizeof(DWORD))
			return CONVERTERR_BUFFERFULL;

		me->dwDeltaTime = delta_time;
		me->dwStreamID = 0;
		me->dwEvent = (te->shortdata[0])
					| (((DWORD)te->shortdata[1]) <<  8)
					| (((DWORD)te->shortdata[2]) << 16)
					| MEVT_F_SHORT;

		if ((te->shortdata[0] & 0xF0) == MIDICMD_CONTROL &&
		    te->shortdata[1] == MIDICTL_MSB_MAIN_VOLUME)
		{
			/* If this is a volume change, generate a callback
			 * so we can grab the new volume for our cache. */
			me->dwEvent |= MEVT_F_CALLBACK;
		}
		buf->bytes_in += 3 *sizeof(DWORD);
	}
	else if (te->shortdata[0] == MIDICMD_SYSEX ||
		 te->shortdata[0] == MIDICMD_SYSEX_END)
	{
		DEBUG_Printf("%s: Ignoring SysEx event.\n", __thisfunc__);
		if (te->longdata)
		{
			Z_Free(te->longdata);
			te->longdata = NULL;
		}
	}
	else
	{
		/* Better be a meta event.
		 *	BYTE	event
		 *	BYTE	type
		 *	VDWORD	len
		 *	BYTE	longdata[len]
		 */
		assert(te->shortdata[0] == MIDI_META_EVENT);

		/* The only meta-event we care about is change tempo */
		if (te->shortdata[1] != MIDI_META_TEMPO)
		{
			if (te->longdata)
			{
				Z_Free(te->longdata);
				te->longdata = NULL;
			}
			return CONVERTERR_METASKIP;
		}

		/* We should have three bytes of parameter data */
		assert(te->event_len == 3);
		/* Need 3 DWORD's: delta-t, stream-ID, event data */
		if (buf->maxlen - buf->bytes_in < 3 *sizeof(DWORD))
		{
			if (te->longdata)
			{
				Z_Free(te->longdata);
				te->longdata = NULL;
			}
			return CONVERTERR_BUFFERFULL;
		}

		me->dwDeltaTime = delta_time;
		me->dwStreamID = 0;

		/* Note: this is backwards from above because we're converting
		 *	 a single data value from hi-lo to lo-hi format... */
		me->dwEvent = (te->longdata[2])
					| (((DWORD)te->longdata[1]) << 8 )
					| (((DWORD)te->longdata[0]) << 16);

		buffer_tick_len = (mfs.timediv * 1000 * BUFFER_TIME_LENGTH)
					/ me->dwEvent /* == current tempo */;
		DEBUG_Printf("%s: buffer tick length: %lu\n", __thisfunc__, buffer_tick_len);
		me->dwEvent |= (((DWORD)MEVT_TEMPO) << 24) | MEVT_F_SHORT;

		if (te->longdata)
		{
			Z_Free(te->longdata);
			te->longdata = NULL;
		}
		buf->bytes_in += 3 *sizeof(DWORD);
	}

	return 0;
}

