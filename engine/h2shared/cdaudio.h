/*
	cdaudio.h
	client cd audio functions

	$Id: cdaudio.h,v 1.4 2007-03-14 21:03:00 sezero Exp $
*/


#ifndef __CDAUDIO_H
#define __CDAUDIO_H

int	CDAudio_Init (void);
void	CDAudio_Play (byte track, qboolean looping);
void	CDAudio_Stop (void);
void	CDAudio_Pause (void);
void	CDAudio_Resume (void);
void	CDAudio_Shutdown (void);
void	CDAudio_Update (void);

#endif	/* __CDAUDIO_H */

