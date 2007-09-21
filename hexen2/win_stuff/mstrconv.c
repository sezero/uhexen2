/*
	mstrconv.c
	$Id: mstrconv.c,v 1.23 2007-09-21 11:05:10 sezero Exp $

	Converting a MID file to a MIDI stream for
	playback using the Win32 midiStream API.
*/

#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>
#include <assert.h>
#include <stdio.h>

#include "midi.h"
#include "midstuff.h"
#include "quakedef.h"

// Global stuff which is defined in the main module
//
BOOL		bInsertTempo = FALSE;
INFILESTATE	ifs;

// Variables used by this module only
//
static DWORD	tkCurrentTime;
static byte	*MidiData;
static LONG	MidiOffset, MidiSize;

// Tracks how many malloc blocks exist. If there are any and we decide to shut
// down, we must scan for them and free them.  Malloc blocks are only created as
// temporary storgae blocks for extra parameter data associated with MIDI_SYSEX,
// MIDI_SYSEXEND, and MIDI_META events.
static DWORD	dwMallocBlocks = 0;

extern DWORD	dwBufferTickLength, dwTempoMultiplier, dwCurrentTempo;
extern DWORD	dwProgressBytes, dwVolumePercent;
extern qboolean	bLooped;

// Messages
//
static char szInitErrInFile[]	= "Read error on input file or file is corrupt.\n";
#ifdef DEBUG_BUILD
static char szEventAddErr[]	= "Unable to add event to stream buffer.";
static char gteBadRunStat[]	= "Reference to missing running status.";
static char gteRunStatMsgTrunc[]= "Running status message truncated";
static char gteChanMsgTrunc[]	= "Channel message truncated";
static char gteSysExLenTrunc[]	= "SysEx event truncated (length)";
static char gteSysExTrunc[]	= "SysEx event truncated";
static char gteMetaNoClass[]	= "Meta event truncated (no class byte)";
static char gteMetaLenTrunc[]	= "Meta event truncated (length)";
static char gteMetaTrunc[]	= "Meta event truncated";
#endif

// Prototypes
//
static int  AddEventToStreamBuffer (PTEMPEVENT pteTemp, LPCONVERTINFO);
static BOOL GetInFileData (LPVOID lpDest, DWORD cbToGet);
static BOOL GetTrackByte (PINTRACKSTATE ptsTrack, LPBYTE lpbyByte);
static BOOL GetTrackEvent (PINTRACKSTATE ptsTrack, PTEMPEVENT pteTemp);
static BOOL GetTrackVDWord (PINTRACKSTATE ptsTrack, LPDWORD lpdw);
static BOOL RefillTrackBuffer (PINTRACKSTATE ptsTrack);
static BOOL RewindConverter (void);

#ifdef DEBUG_BUILD
static void ShowTrackError (PINTRACKSTATE ptsTrack, const char *szErr)
{
	Con_Printf ("MIDI: %s\n", szErr);
	Con_Printf ("Track buffer offset: %lu, total: %lu, left: %lu\n",
			(unsigned long)(ptsTrack->pTrackCurrent - ptsTrack->pTrackStart), ptsTrack->dwTrackLength, ptsTrack->dwLeftInBuffer);
}
#endif

static LONG SetFileOffset (LONG lDistanceToMove, DWORD dwMoveMethod)
{
	LONG	SaveMidi;

	SaveMidi = MidiOffset;
	if (dwMoveMethod == FILE_BEGIN)
	{
		MidiOffset = lDistanceToMove;
	}
	else
	{
		MidiOffset += lDistanceToMove;
	}

	if (MidiOffset >= MidiSize)
	{
		MidiOffset = SaveMidi;
		return -1;
	}

	return MidiOffset;
}


static BOOL ReadFromFile (LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
	if (MidiOffset+nNumberOfBytesToRead > MidiSize)
	{
	//	Con_Printf("Bad Read (%d+%d>=%d)\n",MidiOffset,nNumberOfBytesToRead,MidiSize);
		return FALSE;
	}

	memcpy (lpBuffer, MidiData+MidiOffset, nNumberOfBytesToRead);
	MidiOffset += nNumberOfBytesToRead;
	*lpNumberOfBytesRead = nNumberOfBytesToRead;

	return TRUE;
}

// ConverterInit
// 
// Open the input file
// Allocate and read the entire input file into memory
// Validate the input file structure
// Allocate the input track structures and initialize them
// Initialize the output track structures
//
// Return TRUE on success
// Prints its own error message if something goes wrong
//
BOOL ConverterInit (const char *szInFile)
{
	BOOL	fRet = TRUE;
	DWORD	cbRead, dwTag, cbHeader, dwToRead;
	UINT	idx;
	MIDIFILEHDR	Header;
	PINTRACKSTATE	ptsTrack;

	tkCurrentTime = 0;

	// Initialize things we'll try to free later if we fail
	//
	memset (&ifs, 0, sizeof(INFILESTATE));
	ifs.cbFileLength = 0;
	ifs.pitsTracks = NULL;

	// Attempt to open the input and output files
	//
	MidiData = (byte *)FS_LoadHunkFile(szInFile);
	if (!MidiData)
	{
		goto Init_Cleanup;
	}

	ifs.cbFileLength = fs_filesize;
	MidiOffset = 0;
	MidiSize = ifs.cbFileLength;

// Set up to read from the memory buffer. Read and validate
// - MThd header
// - size of file header chunk
// - file header itself
//
	if ( GetInFileData(&dwTag, sizeof(DWORD)) ||
			(dwTag != MThd) ||
			GetInFileData(&cbHeader, sizeof(DWORD)) ||
			((cbHeader = DWORDSWAP(cbHeader)) < sizeof(MIDIFILEHDR)) ||
			GetInFileData(&Header, cbHeader) )
	{
		Con_Printf("MIDI: %s\n", szInitErrInFile);
		goto Init_Cleanup;
	}

// File header is stored in hi-lo order. Swap this into Intel order and save
// parameters in our native int size (32 bits)
//
	ifs.dwFormat = (DWORD)WORDSWAP(Header.wFormat);
	ifs.dwTrackCount = (DWORD)WORDSWAP(Header.wTrackCount);
	ifs.dwTimeDivision = (DWORD)WORDSWAP(Header.wTimeDivision);

// We know how many tracks there are; allocate the structures for them and parse
// them. The parse merely looks at the MTrk signature and track chunk length
// in order to skip to the next track header.
//
	ifs.pitsTracks = (PINTRACKSTATE) Z_Malloc(ifs.dwTrackCount * sizeof(INTRACKSTATE), Z_MAINZONE);

	for (idx = 0, ptsTrack = ifs.pitsTracks; idx < ifs.dwTrackCount; ++idx, ++ptsTrack)
	{
		ptsTrack->pTrackStart = (LPBYTE) Z_Malloc(TRACK_BUFFER_SIZE, Z_MAINZONE);

		if ( GetInFileData(&dwTag, sizeof(dwTag)) || (dwTag != MTrk)
				|| GetInFileData(&cbHeader, sizeof(cbHeader)) )
		{
			Con_Printf("MIDI: %s\n", szInitErrInFile);
			goto Init_Cleanup;
		}

		cbHeader = DWORDSWAP(cbHeader);
		ptsTrack->dwTrackLength = cbHeader; // Total track length

// Here we need to determine if all track data will fit into a single one of
// our track buffers.  If not, we need to read in a buffer full and come back
// for more later, saving the file offset to continue from and the amount left
// to read in the track structure.

		// Save the file offset of the beginning of this track
		ptsTrack->foTrackStart = SetFileOffset(0, FILE_CURRENT);

		if (ptsTrack->dwTrackLength > TRACK_BUFFER_SIZE)
			dwToRead = TRACK_BUFFER_SIZE;
		else
			dwToRead = ptsTrack->dwTrackLength;

		if (!ReadFromFile(ptsTrack->pTrackStart, dwToRead, &cbRead) || (cbRead != dwToRead))
		{
			Con_Printf("MIDI: %s\n", szInitErrInFile);
			goto Init_Cleanup;
		}

		// Save the number of bytes that didn't make it into the buffer
		ptsTrack->dwLeftOnDisk = ptsTrack->dwTrackLength - cbRead;
		ptsTrack->dwLeftInBuffer = cbRead;
		// Save the current file offset so we can seek to it later
		ptsTrack->foNextReadStart = SetFileOffset(0, FILE_CURRENT);

		// Setup pointer to the current position in the track
		ptsTrack->pTrackCurrent = ptsTrack->pTrackStart;
		ptsTrack->fdwTrack = 0;
		ptsTrack->byRunningStatus = 0;
		ptsTrack->tkNextEventDue = 0;

		// Handle bozo MIDI files which contain empty track chunks
		//
		if (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk)
		{
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			continue;
		}

		// We always preread the time from each track so the mixer code can
		// determine which track has the next event with a minimum of work
		//
		if ( GetTrackVDWord(ptsTrack, &ptsTrack->tkNextEventDue) )
		{
			Con_Printf("MIDI: %s\n", szInitErrInFile);
			goto Init_Cleanup;
		}

		// Step over any unread data, advancing to the beginning of the next
		// track's data
		SetFileOffset(ptsTrack->foTrackStart + ptsTrack->dwTrackLength, FILE_BEGIN);

	}	// End of track initialization code

	fRet = FALSE;

Init_Cleanup:
	if (fRet)
		ConverterCleanup();

	return (fRet);
}


//
// GetInFileData
//
// Gets the requested number of bytes of data from the input file and returns
// a pointer to them.
// 
// Returns a pointer to the data or NULL if we'd read more than is
// there.
//
static BOOL GetInFileData (LPVOID lpDest, DWORD cbToGet)
{
	DWORD	cbRead;

	if (!ReadFromFile(lpDest, cbToGet, &cbRead) || (cbRead != cbToGet))
	{
		return (TRUE);
	}

	return (FALSE);
}


//
// ConverterCleanup
//
// Free anything we ever allocated
//
void ConverterCleanup (void)
{
	DWORD	idx;

	if (ifs.pitsTracks)
	{
		// De-allocate all our track buffers
		for (idx = 0; idx < ifs.dwTrackCount; idx++)
		{
			if (ifs.pitsTracks[idx].pTrackStart)
				Z_Free(ifs.pitsTracks[idx].pTrackStart);
		}
		Z_Free(ifs.pitsTracks);
		ifs.pitsTracks = NULL;
	}
}


/********************************************************************************/
/* RewindConverter()								*/
/*										*/
/* This little function is an adaptation of the ConverterInit() code which	*/
/* resets the tracks without closing and opening the file, thus reducing the	*/
/* time it takes to loop back to the beginning when looping.			*/
/********************************************************************************/
static BOOL RewindConverter (void)
{
	DWORD	dwToRead, cbRead, idx;
	BOOL	fRet;

	PINTRACKSTATE	ptsTrack;

	tkCurrentTime = 0;

	for (idx = 0, ptsTrack = ifs.pitsTracks; idx < ifs.dwTrackCount; ++idx, ++ptsTrack)
	{
// Here we need to determine if all track data will fit into a single one of
// our track buffers.  If not, we need to read in a buffer full and come back
// for more later, saving the file offset to continue from and the amount left
// to read in the track structure.

		SetFileOffset(ptsTrack->foTrackStart, FILE_BEGIN);

		if (ptsTrack->dwTrackLength > TRACK_BUFFER_SIZE)
			dwToRead = TRACK_BUFFER_SIZE;
		else
			dwToRead = ptsTrack->dwTrackLength;

		if (!ReadFromFile(ptsTrack->pTrackStart, dwToRead, &cbRead) || (cbRead != dwToRead))
		{
			Con_Printf("MIDI: %s\n", szInitErrInFile);
			goto Rewind_Cleanup;
		}

		// Save the number of bytes that didn't make it into the buffer
		ptsTrack->dwLeftOnDisk = ptsTrack->dwTrackLength - cbRead;
		ptsTrack->dwLeftInBuffer = cbRead;

		// Save the current file offset so we can seek to it later
		ptsTrack->foNextReadStart = SetFileOffset(0, FILE_CURRENT);

		// Setup pointer to the current position in the track
		ptsTrack->pTrackCurrent = ptsTrack->pTrackStart;
		ptsTrack->fdwTrack = 0;
		ptsTrack->byRunningStatus = 0;
		ptsTrack->tkNextEventDue = 0;

		// Handle bozo MIDI files which contain empty track chunks
		//
		if (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk)
		{
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			continue;
		}

		// We always preread the time from each track so the mixer code can
		// determine which track has the next event with a minimum of work
		//
		if (GetTrackVDWord(ptsTrack, &ptsTrack->tkNextEventDue))
		{
			Con_Printf("MIDI: %s\n", szInitErrInFile);
			goto Rewind_Cleanup;
		}

		// Step over any unread data, advancing to the beginning of the next
		// track's data
		SetFileOffset(ptsTrack->foTrackStart + ptsTrack->dwTrackLength, FILE_BEGIN);

	} // End of track initialization code

	fRet = FALSE;

Rewind_Cleanup:

	if (fRet)
		return (TRUE);

	return (FALSE);
}


/********************************************************************************/
/* ConvertToBuffer()								*/
/*										*/
/* This function converts MIDI data from the track buffers setup by a		*/
/* previous call to ConverterInit().  It will convert data until an error is	*/
/* encountered or the output buffer has been filled with as much event data	*/
/* as possible, not to exceed dwMaxLength. This function can take a couple	*/
/* bit flags, passed through dwFlags. Information about the success/failure	*/
/* of this operation and the number of output bytes actually converted will	*/
/* be returned in the CONVERTINFO structure pointed at by lpciInfo.		*/
/*										*/
/********************************************************************************/
int ConvertToBuffer (DWORD dwFlags, LPCONVERTINFO lpciInfo)
{
	static INTRACKSTATE	*ptsTrack, *ptsFound;
	static DWORD		dwStatus;
	static DWORD		tkNext;
	static TEMPEVENT	teTemp;

	int	nChkErr;
	DWORD	idx;

	lpciInfo->dwBytesRecorded = 0;

	if (dwFlags & CONVERTF_RESET)
	{
		dwProgressBytes = 0;
		dwStatus = 0;
		memset(&teTemp, 0, sizeof(TEMPEVENT));
		ptsTrack = ptsFound = NULL;
	}

	// If we were already done, then return with a warning...
	if (dwStatus & CONVERTF_STATUS_DONE)
	{
		if (bLooped)
		{
			RewindConverter();
			dwProgressBytes = 0;
			dwStatus = 0;
		}
		else
		{
			return (CONVERTERR_DONE);
		}
	}
	// The caller is asking us to continue, but we're already hosed because we
	// previously identified something as corrupt, so complain louder this time.
	else if (dwStatus & CONVERTF_STATUS_STUCK)
	{
		return (CONVERTERR_STUCK);
	}
	else if (dwStatus & CONVERTF_STATUS_GOTEVENT)
	{
		// Turn off this bit flag
		dwStatus ^= CONVERTF_STATUS_GOTEVENT;

	/*
	 * The following code for this case is duplicated from below, and is
	 * designed to handle a "straggler" event, should we have one left over
	 * from previous processing the last time this function was called.
	 */

	// Don't add end of track event 'til we're done
	//
		if (teTemp.byShortData[0] == MIDI_META && teTemp.byShortData[1] == MIDI_META_EOT)
		{
			if (dwMallocBlocks)
			{
				Z_Free(teTemp.pLongData);
				dwMallocBlocks--;
			}
		}
		else if ( (nChkErr = AddEventToStreamBuffer( &teTemp, lpciInfo ))
							  != CONVERTERR_NOERROR )
		{
			if (nChkErr == CONVERTERR_BUFFERFULL)
			{
				// Do some processing and tell caller that this buffer's full
				dwStatus |= CONVERTF_STATUS_GOTEVENT;
				return (CONVERTERR_NOERROR);
			}
			else if (nChkErr == CONVERTERR_METASKIP)
			{
				// We skip by all meta events that aren't tempo changes...
			}
			else
			{
				DEBUG_Printf("MIDI: %s\n", szEventAddErr);
				if (dwMallocBlocks)
				{
					Z_Free(teTemp.pLongData);
					dwMallocBlocks--;
				}
				return (TRUE);
			}
		}
	}

	for ( ; ; )
	{
		ptsFound = NULL;
		tkNext = ~(DWORD)0;	/* 0xFFFFFFFFL */

		// Find nearest event due
		//
		for (idx = 0, ptsTrack = ifs.pitsTracks; idx < ifs.dwTrackCount; ++idx, ++ptsTrack)
		{
			if ( ( !(ptsTrack->fdwTrack & ITS_F_ENDOFTRK)) &&
				( ptsTrack->tkNextEventDue < tkNext ) )
			{
				tkNext = ptsTrack->tkNextEventDue;
				ptsFound = ptsTrack;
			}
		}

		// None found? We must be done, so return to the caller with a smile.
		//
		if (!ptsFound)
		{
			dwStatus |= CONVERTF_STATUS_DONE;
			// Need to set return buffer members properly
			return (CONVERTERR_NOERROR);
		}

		// Ok, get the event header from that track
		//
		if (GetTrackEvent(ptsFound, &teTemp))
		{
			// Warn future calls that this converter is stuck at a corrupt spot
			// and can't continue
			dwStatus |= CONVERTF_STATUS_STUCK;
			return (CONVERTERR_CORRUPT);
		}

		// Don't add end of track event 'til we're done
		//
		if (teTemp.byShortData[0] == MIDI_META && teTemp.byShortData[1] == MIDI_META_EOT)
		{
			if (dwMallocBlocks)
			{
				Z_Free(teTemp.pLongData);
				dwMallocBlocks--;
			}
			continue;
		}

		if ( (nChkErr = AddEventToStreamBuffer( &teTemp, lpciInfo ))
						     != CONVERTERR_NOERROR )
		{
			if (nChkErr == CONVERTERR_BUFFERFULL)
			{
				// Do some processing and tell somebody this buffer is full...
				dwStatus |= CONVERTF_STATUS_GOTEVENT;
				return (CONVERTERR_NOERROR);
			}
			else if (nChkErr == CONVERTERR_METASKIP)
			{
				// We skip by all meta events that aren't tempo changes...
			}
			else
			{
				DEBUG_Printf("MIDI: %s\n", szEventAddErr);
				if (dwMallocBlocks)
				{
					Z_Free(teTemp.pLongData);
					dwMallocBlocks--;
				}
				return (TRUE);
			}
		}
	}

//	return( CONVERTERR_NOERROR );	// unreachable
}


/********************************************************************************/
/* GetTrackVDWord								*/
/*										*/
/* Attempts to parse a variable length DWORD from the given track. A VDWord	*/
/* in a MIDI file								*/
/*  (a) is in lo-hi format							*/
/*  (b) has the high bit set on every byte except the last			*/
/*										*/
/* Returns the DWORD in *lpdw and TRUE on success; else				*/
/* FALSE if we hit end of track first. Sets ITS_F_ENDOFTRK			*/
/* if we hit end of track.							*/
/********************************************************************************/
static BOOL GetTrackVDWord (PINTRACKSTATE ptsTrack, LPDWORD lpdw)
{
	BYTE	byByte;
	DWORD	dw = 0;

	if (ptsTrack->fdwTrack & ITS_F_ENDOFTRK)
		return (TRUE);

	do
	{
		if (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk)
		{
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			return (TRUE);
		}

		if (GetTrackByte(ptsTrack, &byByte))
			return (TRUE);

		dw = ( dw << 7 ) | ( byByte & 0x7F );
	} while (byByte & 0x80);

	*lpdw = dw;

	return (FALSE);
}


/********************************************************************************/
/* GetTrackEvent								*/
/*										*/
/* Fills in the event struct with the next event from the track			*/
/*										*/
/* pteTemp->tkEvent will contain the absolute tick time of the event		*/
/* pteTemp->byShortData[0] will contain						*/
/*  MIDI_META if the event is a meta event;					*/
/*   in this case pteTemp->byShortData[1] will contain the meta class		*/
/*  MIDI_SYSEX or MIDI_SYSEXEND if the event is a SysEx event			*/
/*  Otherwise, the event is a channel message and pteTemp->byShortData[1]	*/
/*   and pteTemp->byShortData[2] will contain the rest of the event.		*/
/*										*/
/* pteTemp->dwEventLength will contain						*/
/*  The total length of the channel message in pteTemp->byShortData if		*/
/*   the event is a channel message						*/
/*  The total length of the paramter data pointed to by				*/
/*   pteTemp->pLongData otherwise						*/
/*										*/
/* pteTemp->pLongData will point at any additional paramters if the		*/
/*  event is a SysEx or meta event with non-zero length; else			*/
/*  it will contain NULL							*/
/*										*/
/* Returns FALSE on success or TRUE on any kind of parse error			*/
/* Prints its own error message ONLY in the debug version			*/
/*										*/
/* Maintains the state of the input track (i.e. ptsTrack->dwLeftInBuffer,	*/
/* ptsTrack->pTrackPointers, and ptsTrack->byRunningStatus).			*/
/********************************************************************************/
static BOOL GetTrackEvent (INTRACKSTATE *ptsTrack, PTEMPEVENT pteTemp)
{
	DWORD	idx;
	BYTE	byByte;
	UINT	dwEventLength;

	// Clear out the temporary event structure to get rid of old data...
	memset(pteTemp, 0, sizeof(TEMPEVENT));

	// Already at end of track? There's nothing to read.
	//
	if ( (ptsTrack->fdwTrack & ITS_F_ENDOFTRK) ||
	     (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk) )
		return (TRUE);

	// Get the first byte, which determines the type of event.
	//
	if (GetTrackByte(ptsTrack, &byByte))
		return (TRUE);

	// If the high bit is not set, then this is a channel message
	// which uses the status byte from the last channel message
	// we saw. NOTE: We do not clear running status across SysEx or
	// meta events even though the spec says to because there are
	// actually files out there which contain that sequence of data.
	//
	if ( !( byByte & 0x80 ) )
	{
		// No previous status byte? We're hosed.
		if (!ptsTrack->byRunningStatus)
		{
			TRACKERR(ptsTrack, gteBadRunStat);
			return (TRUE);
		}

		pteTemp->byShortData[0] = ptsTrack->byRunningStatus;
		pteTemp->byShortData[1] = byByte;

		byByte = pteTemp->byShortData[0] & 0xF0;
		pteTemp->dwEventLength = 2;

		// Only program change and channel pressure events are 2 bytes long;
		// the rest are 3 and need another byte
		//
		if ( (byByte != MIDI_PRGMCHANGE) && (byByte != MIDI_CHANPRESS) )
		{
			if (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk)
			{
				TRACKERR(ptsTrack, gteRunStatMsgTrunc);
				ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
				return (TRUE);
			}

			if (GetTrackByte(ptsTrack, &pteTemp->byShortData[2]))
				return( TRUE );
			++pteTemp->dwEventLength;
		}
	}
	else if ((byByte & 0xF0) != MIDI_SYSEX)
	{
		// Not running status, not in SysEx range - must be
		// normal channel message (0x80-0xEF)
		//
		pteTemp->byShortData[0] = byByte;
		ptsTrack->byRunningStatus = byByte;

		// Strip off channel and just keep message type
		//
		byByte &= 0xF0;

		dwEventLength = (byByte == MIDI_PRGMCHANGE || byByte == MIDI_CHANPRESS) ? 1 : 2;
		pteTemp->dwEventLength = dwEventLength + 1;

		if ((ptsTrack->dwLeftInBuffer + ptsTrack->dwLeftOnDisk) < dwEventLength)
		{
			TRACKERR(ptsTrack, gteChanMsgTrunc);
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			return (TRUE);
		}

		if (GetTrackByte(ptsTrack, &pteTemp->byShortData[1]))
			return (TRUE);

		if (dwEventLength == 2)
		{
			if (GetTrackByte( ptsTrack, &pteTemp->byShortData[2]))
				return (TRUE);
		}
	}
	else if ((byByte == MIDI_SYSEX) || (byByte == MIDI_SYSEXEND))
	{
		// One of the SysEx types. (They are the same as far as we're concerned;
		// there is only a semantic difference in how the data would actually
		// get sent when the file is played. We must take care to put the proper
		// event type back on the output track, however.)
		//
		// Parse the general format of:
		//  BYTE    bEvent (MIDI_SYSEX or MIDI_SYSEXEND)
		//  VDWORD  cbParms
		//  BYTE    abParms[cbParms]
		//
		pteTemp->byShortData[0] = byByte;
		if (GetTrackVDWord(ptsTrack, &pteTemp->dwEventLength))
		{
			TRACKERR(ptsTrack, gteSysExLenTrunc);
			return (TRUE);
		}

		if ((ptsTrack->dwLeftInBuffer + ptsTrack->dwLeftOnDisk) < pteTemp->dwEventLength)
		{
			TRACKERR(ptsTrack, gteSysExTrunc);
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			return (TRUE);
		}

		// Malloc a temporary memory block to hold the parameter data
		pteTemp->pLongData = (LPBYTE) Z_Malloc(pteTemp->dwEventLength, Z_MAINZONE);
		// Copy from the input buffer to the parameter data buffer
		for (idx = 0; idx < pteTemp->dwEventLength; idx++)
		{
			if (GetTrackByte(ptsTrack, pteTemp->pLongData + idx))
			{
				TRACKERR(ptsTrack, gteSysExTrunc);
				return (TRUE);
			}
		}
		// Increment our counter, which tells the program to look around for
		// a malloc block to free, should it need to exit or reset before the
		// block would normally be freed
		dwMallocBlocks++;
	}
	else if (byByte == MIDI_META)
	{
		// It's a meta event. Parse the general form:
		//  BYTE    bEvent  (MIDI_META)
		//  BYTE    bClass
		//  VDWORD  cbParms
		//  BYTE    abParms[cbParms]
		//
		pteTemp->byShortData[0] = byByte;

		if (!ptsTrack->dwLeftInBuffer && !ptsTrack->dwLeftOnDisk)
		{
			TRACKERR(ptsTrack, gteMetaNoClass);
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
			return (TRUE);
		}

		if (GetTrackByte(ptsTrack, &pteTemp->byShortData[1]))
			return (TRUE);

		if (GetTrackVDWord(ptsTrack, &pteTemp->dwEventLength))
		{
			TRACKERR(ptsTrack, gteMetaLenTrunc);
			return (TRUE);
		}

		// NOTE: It's perfectly valid to have a meta with no data
		// In this case, dwEventLength == 0 and pLongData == NULL
		//
		if (pteTemp->dwEventLength)
		{
			if ((ptsTrack->dwLeftInBuffer + ptsTrack->dwLeftOnDisk) < pteTemp->dwEventLength)
			{
				TRACKERR(ptsTrack, gteMetaTrunc);
				ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
				return (TRUE);
			}

			// Malloc a temporary memory block to hold the parameter data
			pteTemp->pLongData = (LPBYTE) Z_Malloc(pteTemp->dwEventLength, Z_MAINZONE);
			// Copy from the input buffer to the parameter data buffer
			for (idx = 0; idx < pteTemp->dwEventLength; idx++)
			{
				if (GetTrackByte(ptsTrack, pteTemp->pLongData + idx))
				{
					TRACKERR(ptsTrack, gteMetaTrunc);
					return (TRUE);
				}
			}
			// Increment our counter, which tells the program to look around for
			// a malloc block to free, should it need to exit or reset before the
			// block would normally be freed
			dwMallocBlocks++;
		}

		if (pteTemp->byShortData[1] == MIDI_META_EOT)
			ptsTrack->fdwTrack |= ITS_F_ENDOFTRK;
	}
	else
	{
		// Messages in this range are system messages and aren't supposed to
		// be in a normal MIDI file. If they are, we've either misparsed or the
		// authoring software is stupid.
		//
		return (TRUE);
	}

	// Event time was already stored as the current track time
	//
	pteTemp->tkEvent = ptsTrack->tkNextEventDue;

	// Now update to the next event time. The code above MUST properly
	// maintain the end of track flag in case the end of track meta is
	// missing.  NOTE: This code is a continuation of the track event
	// time pre-read which is done at the end of track initialization.
	//
	if ( !(ptsTrack->fdwTrack & ITS_F_ENDOFTRK) )
	{
		DWORD	tkDelta;

		if (GetTrackVDWord(ptsTrack, &tkDelta))
			return (TRUE);

		ptsTrack->tkNextEventDue += tkDelta;
	}

	return (FALSE);
}


//
// GetTrackByte
//
// Retrieve the next byte from the track buffer, refilling the buffer from
// disk if necessary.
//
static BOOL GetTrackByte (PINTRACKSTATE ptsTrack, LPBYTE lpbyByte)
{
	if (!ptsTrack->dwLeftInBuffer)
	{
		if (RefillTrackBuffer(ptsTrack))
			return (TRUE);
	}

	*lpbyByte = *ptsTrack->pTrackCurrent++;
	ptsTrack->dwLeftInBuffer--;
	return (FALSE);
}


//
// RefillTrackBuffer()
//
// This function attempts to read in a buffer-full of data for a MIDI track.
//
static BOOL RefillTrackBuffer (PINTRACKSTATE ptsTrack)
{
	DWORD	dwBytesRead, dwResult;
	BOOL	bResult;

	if (ptsTrack->dwLeftOnDisk)
	{
		ptsTrack->pTrackCurrent = ptsTrack->pTrackStart;

		// Seek to the proper place in the file, indicated by
		// ptsTrack->foNextReadStart and read in the remaining data,
		// up to a maximum of the buffer size.
		dwResult = (DWORD) SetFileOffset((LONG)(ptsTrack->foNextReadStart), FILE_BEGIN);
		if (dwResult == (DWORD)(-1))
		{
			Con_Printf("MIDI: Unable to seek to track buffer location in RefillTrackBuffer()!!\n");
			return (TRUE);
		}

		if (ptsTrack->dwLeftOnDisk > TRACK_BUFFER_SIZE)
			ptsTrack->dwLeftInBuffer = TRACK_BUFFER_SIZE;
		else
			ptsTrack->dwLeftInBuffer = ptsTrack->dwLeftOnDisk;

		bResult = ReadFromFile(ptsTrack->pTrackStart, ptsTrack->dwLeftInBuffer, &dwBytesRead);

		ptsTrack->dwLeftOnDisk -= dwBytesRead;
		ptsTrack->foNextReadStart = dwResult + dwBytesRead;
		ptsTrack->dwLeftInBuffer = dwBytesRead;

		if (!bResult || (bResult && !dwBytesRead) || (bResult && dwBytesRead != ptsTrack->dwLeftInBuffer))
		{
			Con_Printf("MIDI: Read operation failed prematurely!!\n");
			ptsTrack->dwLeftInBuffer = dwBytesRead;
			return (TRUE);
		}
		else
			return (FALSE);
	}

	return (TRUE);
}


//
// AddEventToStreamBuffer
//
// Put the given event into the given stream buffer at the given location
// pteTemp must point to an event filled out in accordance with the
// description given in GetTrackEvent
//
// Returns FALSE on sucess or TRUE on an error condition
// Handles its own error notification by displaying to the appropriate
// output device (either our debugging window, or the screen).
//
static int AddEventToStreamBuffer (PTEMPEVENT pteTemp, CONVERTINFO *lpciInfo)
{
	DWORD	tkDelta;
	MIDIEVENT	*pmeEvent;

	pmeEvent = (MIDIEVENT *)(lpciInfo->mhBuffer.lpData + lpciInfo->dwStartOffset + lpciInfo->dwBytesRecorded);

	// When we see a new, empty buffer, set the start time on it...
	if (!lpciInfo->dwBytesRecorded)
		lpciInfo->tkStart = tkCurrentTime;

	// Use the above set start time to figure out how much longer we should fill
	// this buffer before officially declaring it as "full"
	if (tkCurrentTime - lpciInfo->tkStart > dwBufferTickLength)
	{
		if (lpciInfo->bTimesUp)
		{
			lpciInfo->bTimesUp = FALSE;
			return (CONVERTERR_BUFFERFULL);
		}
		else
		{
			lpciInfo->bTimesUp = TRUE;
		}
	}

	// Delta time is absolute event time minus absolute time
	// already gone by on this track
	tkDelta = pteTemp->tkEvent - tkCurrentTime;

	// Event time is now current time on this track
	tkCurrentTime = pteTemp->tkEvent;

	if (bInsertTempo)
	{
		bInsertTempo = FALSE;

		if (lpciInfo->dwMaxLength - lpciInfo->dwBytesRecorded < 3 * sizeof(DWORD))
		{
			// Cleanup from our write operation
			return (CONVERTERR_BUFFERFULL);
		}

		if (dwCurrentTempo)
		{
			pmeEvent->dwDeltaTime = 0;
			pmeEvent->dwStreamID = 0;
			pmeEvent->dwEvent = (dwCurrentTempo * 100) / dwTempoMultiplier;
			pmeEvent->dwEvent |= (((DWORD)MEVT_TEMPO ) << 24) | MEVT_F_SHORT;

			lpciInfo->dwBytesRecorded += 3 * sizeof(DWORD);
			pmeEvent += 3 * sizeof(DWORD);
		}
	}

	if (pteTemp->byShortData[0] < MIDI_SYSEX)
	{
		// Channel message. We know how long it is, just copy it.
		// Need 3 DWORD's: delta-t, stream-ID, event
		if (lpciInfo->dwMaxLength-lpciInfo->dwBytesRecorded < 3*sizeof(DWORD))
		{
			// Cleanup from our write operation
			return (CONVERTERR_BUFFERFULL);
		}

		pmeEvent->dwDeltaTime = tkDelta;
		pmeEvent->dwStreamID = 0;
		pmeEvent->dwEvent = ( pteTemp->byShortData[0] )
					| ( ((DWORD)pteTemp->byShortData[1] ) << 8 )
					| ( ((DWORD)pteTemp->byShortData[2] ) << 16 )
					| MEVT_F_SHORT;

		if ( ((pteTemp->byShortData[0] & 0xF0) == MIDI_CTRLCHANGE)
			&& (pteTemp->byShortData[1] == MIDICTRL_VOLUME) )
		{
			// If this is a volume change, generate a callback so we can grab
			// the new volume for our cache
			pmeEvent->dwEvent |= MEVT_F_CALLBACK;
		}
		lpciInfo->dwBytesRecorded += 3 *sizeof(DWORD);
	}
	else if (pteTemp->byShortData[0] == MIDI_SYSEX || pteTemp->byShortData[0] == MIDI_SYSEXEND)
	{
		DEBUG_Printf("%s: Ignoring SysEx event.\n", __thisfunc__);
		if (dwMallocBlocks)
		{
			Z_Free(pteTemp->pLongData);
			dwMallocBlocks--;
		}
	}
	else
	{
		// Better be a meta event.
		//  BYTE    byEvent
		//  BYTE    byEventType
		//  VDWORD  dwEventLength
		//  BYTE    pLongEventData[dwEventLength]
		//
		assert(pteTemp->byShortData[0] == MIDI_META);

		// The only meta-event we care about is change tempo
		//
		if (pteTemp->byShortData[1] != MIDI_META_TEMPO)
		{
			if (dwMallocBlocks)
			{
				Z_Free(pteTemp->pLongData);
				dwMallocBlocks--;
			}
			return (CONVERTERR_METASKIP);
		}

		// We should have three bytes of parameter data...
		assert(pteTemp->dwEventLength == 3);

		// Need 3 DWORD's: delta-t, stream-ID, event data
		if (lpciInfo->dwMaxLength - lpciInfo->dwBytesRecorded < 3 *sizeof(DWORD))
		{
			// Cleanup the temporary event if necessary and return
			if (dwMallocBlocks)
			{
				Z_Free(pteTemp->pLongData);
				dwMallocBlocks--;
			}
			return (CONVERTERR_BUFFERFULL);
		}

		pmeEvent->dwDeltaTime = tkDelta;
		pmeEvent->dwStreamID = 0;

		// Note: this is backwards from above because we're converting a single
		//       data value from hi-lo to lo-hi format...
		pmeEvent->dwEvent = ( pteTemp->pLongData[2] )
					| ( ((DWORD)pteTemp->pLongData[1] ) << 8 )
					| ( ((DWORD)pteTemp->pLongData[0] ) << 16 );

		/* This next step has absolutely nothing to do with the conversion of a
		 * MIDI file to a stream, it's simply put here to add the functionality
		 * of the tempo slider. If you don't need this, be sure to remove the
		 * next two lines.
		 */
		dwCurrentTempo = pmeEvent->dwEvent;
		pmeEvent->dwEvent = (pmeEvent->dwEvent * 100 ) / dwTempoMultiplier;

		pmeEvent->dwEvent |= ( ((DWORD)MEVT_TEMPO ) << 24 ) | MEVT_F_SHORT;

		dwBufferTickLength = (ifs.dwTimeDivision * 1000 * BUFFER_TIME_LENGTH) / dwCurrentTempo;
		DEBUG_Printf("%s: dwBufferTickLength = %lu\n", __thisfunc__, dwBufferTickLength);

		if (dwMallocBlocks)
		{
			Z_Free(pteTemp->pLongData);
			dwMallocBlocks--;
		}
		lpciInfo->dwBytesRecorded += 3 *sizeof(DWORD);
	}

	return (FALSE);
}

