#ifndef MIDI_CAMDBASE_H
#define MIDI_CAMDBASE_H

/************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                   *
*************************************************************************
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines				*
*************************************************************************
*
* camdbase.h - CAMD library base structure
*
************************************************************************/

#ifndef EXEC_LIBRARIES_H
  #include <exec/libraries.h>
#endif

#ifndef EXEC_LISTS_H
  #include <exec/lists.h>
#endif

#ifndef EXEC_SEMAPHORES_H
  #include <exec/semaphores.h>
#endif


struct CamdBase 
{
    struct Library Lib;
    UWORD pad0;

#if 0

	/* publicly available stuff (READ ONLY) */
    ULONG Time; 	/* current time */
    WORD TickFreq;	/* ideal CAMD Tick frequency */
    WORD TickErr;	/* nanosecond error from ideal Tick length to real tick length */
			/* actual tick length is:  1/TickFreq + TickErr/1e9 */
			/* -705 < TickErr < 705 */
#endif

	/* private stuff below here */
};

#if 0
#define CAMD_TickErr_Min    -705
#define CAMD_TickErr_Max    705
#endif

#endif
