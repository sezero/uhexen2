/*
	mid2strm.h
	$Id$

	Convert MIDI data to a MIDI stream for playback using
	the Windows midiStream API.
*/

#ifndef __MSTREAM_INCLUDED__
#define __MSTREAM_INCLUDED__

/*****************************************************************************/
/* Controls for buffer size, etc. */

#define TRACK_BUFFER_SIZE	2048
#define OUT_BUFFER_SIZE		2048	// Max stream buffer size in bytes
#define BUFFER_TIME_LENGTH	2000	// Amount to fill in milliseconds
#define NUM_STREAM_BUFFERS	5

#define DEBUG_CALLBACK_TIMEOUT	2000	// Wait 2 seconds for callback


#define MSTREAM_STOPF_NOREOPEN		0x0001

#define STREAMF_BUFFER_WARNING		0x00000002

#define CONVERTF_RESET			0x00000001

#define CONVERTF_STATUS_DONE		0x00000001
#define CONVERTF_STATUS_STUCK		0x00000002
#define CONVERTF_STATUS_GOTEVENT	0x00000004

#define CONVERTERR_NOERROR	0	// No error occured
#define CONVERTERR_CORRUPT	-101	// The input file is corrupt
// The converter has already encountered a corrupt file and cannot convert any
// more of this file -- you must reset the converter
#define CONVERTERR_STUCK	-102
#define CONVERTERR_DONE		-103	// Converter is done
#define CONVERTERR_BUFFERFULL	-104	// The buffer is full
#define CONVERTERR_METASKIP	-105	// Skipping unknown meta event

#define STATUS_KILLCALLBACK	100	// Signals that the callback should die
#define STATUS_CALLBACKDEAD	200	// Signals callback is done processing
#define STATUS_WAITINGFOREND	300	// Callback's waiting for buffers to play

#define VOLUME_BUFFER		3567	// Random value which acts as a buffer id
#define VOL_CACHE_INIT		100


/*
 * This structure is used to pass information to the ConvertToBuffer()
 * system and then internally by that function to send information about the
 * target stream buffer and current state of the conversion process to lower
 * level conversion routines internal to the MSTRCONV module.  See that source
 * file for specific details.
 */
typedef struct _ConvertInfo
{
	MIDIHDR	mhBuffer;	// Standard Windows stream buffer header
	DWORD	dwStartOffset;	// Start offset from mhStreamBuffer.lpStart
	DWORD	dwMaxLength;	// Max length to convert on this pass

	DWORD	dwBytesRecorded;// Used internally by the MSTRCONV module
	DWORD	tkStart;	// Used internally by the MSTRCONV module
	BOOL	bTimesUp;	// Used internally by the MSTRCONV module
} CONVERTINFO, *LPCONVERTINFO;


int ConverterInit (const char *szInFile);
void ConverterCleanup (void);
int ConvertToBuffer (DWORD, LPCONVERTINFO);

#endif /* __MSTREAM_INCLUDED__ */

