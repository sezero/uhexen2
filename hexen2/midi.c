/*
	midi.c

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/midi.c,v 1.4 2005-02-04 11:28:59 sezero Exp $
*/

#ifndef PLATFORM_UNIX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <memory.h>
#include <mmreg.h>
#endif

#ifndef PLATFORM_UNIX
#include "midstuff.h"
#include "midi.h"
#endif
#include "quakedef.h"

#ifdef USE_MIDI
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

static Mix_Music *music = NULL;

void (*midi_endmusicfnc)(void);
#endif // !USE_MIDI

byte bMidiInited,bFileOpen, bPlaying, bBuffersPrepared;
byte bPaused, bLooped;
float dwVolumePercent;

#ifndef PLATFORM_UNIX
UINT uMIDIDeviceID = MIDI_MAPPER, uCallbackStatus;
int nCurrentBuffer, nEmptyBuffers;
DWORD dwBufferTickLength, dwTempoMultiplier, dwCurrentTempo, dwProgressBytes;
DWORD dwVolCache[NUM_CHANNELS];

HMIDISTRM    hStream;
CONVERTINFO  ciStreamBuffers[NUM_STREAM_BUFFERS];

// From mstrconv.c
extern INFILESTATE  ifs;

// Private to this module...
static HANDLE   hBufferReturnEvent;


static void FreeBuffers(void);


void MidiErrorMessageBox(MMRESULT mmr)
{
   char temp[1024];

   midiOutGetErrorText(mmr,temp,sizeof(temp));
	Con_Printf("%s\n",temp);
}

#endif	// !PLATFORM_UNIX



void MIDI_Play_f (void)
{
	if (Cmd_Argc () == 2)
	{
		MIDI_Play(Cmd_Argv(1));
	}
}

void MIDI_Stop_f (void)
{
	MIDI_Stop();
}

void MIDI_Pause_f (void)
{
	MIDI_Pause();
}

void MIDI_Loop_f (void)
{
#ifndef PLATFORM_UNIX
	if (Cmd_Argc () == 2)
	{
      if (strcmpi(Cmd_Argv(1),"on") == 0 || strcmpi(Cmd_Argv(1),"1") == 0) 
			MIDI_Loop(1);
		else if (strcmpi(Cmd_Argv(1),"off") == 0 || strcmpi(Cmd_Argv(1),"0") == 0) 
			MIDI_Loop(0);
		else if (strcmpi(Cmd_Argv(1),"toggle") == 0) 
			MIDI_Loop(2);
	}

	if (bLooped) Con_Printf("MIDI music will be looped\n");
	else Con_Printf("MIDI music will not be looped\n");
#endif

#ifdef USE_MIDI
	if (Cmd_Argc () == 2)
	{
      if (strcmpi(Cmd_Argv(1),"on") == 0 || strcmpi(Cmd_Argv(1),"1") == 0) 
			MIDI_Loop(1);
		else if (strcmpi(Cmd_Argv(1),"off") == 0 || strcmpi(Cmd_Argv(1),"0") == 0) 
			MIDI_Loop(0);
		else if (strcmpi(Cmd_Argv(1),"toggle") == 0) 
			MIDI_Loop(2);
	}

	if (bLooped) Con_Printf("MIDI music will be looped\n");
	else Con_Printf("MIDI music will not be looped\n");
#endif

}

void MIDI_Volume_f (void)
{
#ifndef PLATFORM_UNIX
	if (Cmd_Argc () == 2)
	{
		dwVolumePercent = atol(Cmd_Argv(1))*65535/100;
		midiOutSetVolume(hStream,(dwVolumePercent<<16)+dwVolumePercent);

/*		dwVolumePercent = atol(Cmd_Argv(1))*10;
		SetAllChannelVolumes(dwVolumePercent);*/
	}
	else
	{
		Con_Printf("MIDI volume is %d\n", dwVolumePercent/(65535/100));
	}
#endif

#ifdef USE_MIDI
	if (Cmd_Argc () == 2)
	{
		dwVolumePercent = atof(Cmd_Argv(1))*100;
		if (dwVolumePercent > 100) dwVolumePercent=100;
		if (dwVolumePercent < 0) dwVolumePercent=0;
		Mix_VolumeMusic(dwVolumePercent*1.28); /* needs to be between 0 and 128 */
	}
	else
	{
		Con_Printf("MIDI volume is %f\n", dwVolumePercent);
	}
#endif
}

void MIDI_EndMusicFinished(void)
{
  printf("Music finished\n");
#ifdef USE_MIDI
  if (bLooped) {
    printf("Looping enabled\n");
    if (Mix_PlayingMusic())
      Mix_HaltMusic();

    printf("Playing again\n");
    Mix_RewindMusic();    
    Mix_FadeInMusic(music,0,2000);
    bPlaying = 1;

  }
#endif
}

// Changed to qboolean to jive with quakedef.h - DDOI
qboolean MIDI_Init(void)
{
#ifdef USE_MIDI
#ifndef SDL_SOUND
// FIXME: midi doesn't play with snd_sdl.c
  int audio_rate = 22050;
  int audio_format = AUDIO_S16;
  int audio_channels = 2;
  int audio_buffers = 4096;
#endif
  char	mididir[MAX_OSPATH];

  printf("MIDI_Init\n");

  sprintf (mididir, "%s/glhexen/midi", com_userdir);
	Sys_mkdir (mididir);

  if( COM_CheckParm("-nomidi") || COM_CheckParm("-nosound") 
      || COM_CheckParm("--nomidi") || COM_CheckParm("--nosound"))
    {
      bMidiInited = 0;
      return 0;
    }

#ifndef SDL_SOUND
// FIXME: midi doesn't play with snd_sdl.c
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {

    Con_Printf("Cannot initialize SDL_AUDIO subsystem: %s\n",SDL_GetError());
    bMidiInited = 0;
    return 0;
  }
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
    bMidiInited = 0;
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return 0;
  }
#endif

  midi_endmusicfnc = &MIDI_EndMusicFinished;
  if (midi_endmusicfnc)
    Mix_HookMusicFinished(midi_endmusicfnc);

  Cmd_AddCommand ("midi_play", MIDI_Play_f);
  Cmd_AddCommand ("midi_stop", MIDI_Stop_f);
  Cmd_AddCommand ("midi_pause", MIDI_Pause_f);
  Cmd_AddCommand ("midi_loop", MIDI_Loop_f);
  Cmd_AddCommand ("midi_volume", MIDI_Volume_f);
  
  bFileOpen = 0;
  bPlaying = 0;
  bLooped = 1;
  bPaused = 0;
  bMidiInited = 1;

#endif

#ifndef PLATFORM_UNIX	
	MMRESULT mmrRetVal;

	hBufferReturnEvent = CreateEvent(NULL,FALSE,FALSE,"Wait For Buffer Return");

	if(COM_CheckParm("-nomidi"))
	{
		bMidiInited = 0;
		return FALSE;
	}
	mmrRetVal = midiStreamOpen(&hStream,&uMIDIDeviceID,(DWORD)1,(DWORD)MidiProc,(DWORD)0,CALLBACK_FUNCTION);
	if(mmrRetVal != MMSYSERR_NOERROR )
	{
		bMidiInited = 0;
		MidiErrorMessageBox( mmrRetVal );
		return FALSE;
	}

  	Cmd_AddCommand ("midi_play", MIDI_Play_f);
  	Cmd_AddCommand ("midi_stop", MIDI_Stop_f);
  	Cmd_AddCommand ("midi_pause", MIDI_Pause_f);
  	Cmd_AddCommand ("midi_loop", MIDI_Loop_f);
  	Cmd_AddCommand ("midi_volume", MIDI_Volume_f);

	dwTempoMultiplier = 100;
	dwVolumePercent = 0xffff;
	bFileOpen = FALSE;
	bPlaying = FALSE;
	bLooped = TRUE;
	bPaused = FALSE;
	bBuffersPrepared = FALSE;
	uCallbackStatus = 0;
	bMidiInited = 1;

#endif
	return true;
}

void MIDI_Play(char *Name)
{

#ifndef PLATFORM_UNIX
	MMRESULT mmrRetVal;
    char Temp[100];
	char *Data;

	if (!bMidiInited)	//don't try to play if there is no midi
		return;

	sprintf(Temp, "midi/%s.mid", Name);
        MIDI_Stop();

	if (StreamBufferSetup(Temp))
	{
		Con_Printf("Couldn't load midi file %s\n",Temp);
	}
	else
	{
		bFileOpen = TRUE;

		Con_Printf("Playing midi file %s\n",Temp);

		uCallbackStatus = 0;
                        
		mmrRetVal = midiStreamRestart(hStream);

		if (mmrRetVal != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmrRetVal);
			return;
		}

		midiOutSetVolume(hStream, (dwVolumePercent<<16)+dwVolumePercent);
		bPlaying = TRUE;
	}


#endif

#ifdef USE_MIDI
	char Temp[100];
	char *Data;
	char midi_file[200];
	char midi_file_with_path[300];
	FILE *f=NULL;

	int size=0;

        //printf("MIDI_Play\n");
	if (!bMidiInited)	//don't try to play if there is no midi
		return;

	sprintf(Temp, "midi/%s.mid", Name);
	sprintf (midi_file, "glhexen/midi/%s.mid", Name);
	sprintf (midi_file_with_path, "%s/glhexen/midi/%s.mid", com_userdir,Name);
	MIDI_Stop();

	COM_FOpenFile (midi_file, &f, true);	
	if (f)
	  {
	    Con_Printf("MIDI: File %s already exists\n",midi_file);
	  }

	else {
	  Con_Printf("MIDI: File %s needs to be extracted\n",midi_file);
	  Data = (byte *)COM_LoadHunkFile2((char *)Temp, (int *)&size);
	  if (!Data) 
	    {
	      //exit(1);
	    }
	  
	  COM_WriteFileFullPath (midi_file_with_path, (void *)Data, size);
	}

	music = Mix_LoadMUS(midi_file_with_path);
	if ( music == NULL ) {
	  Con_Printf("Couldn't load %s: %s\n",
		  midi_file_with_path, SDL_GetError());
	}

	else
	{
		bFileOpen = 1;

		Con_Printf("Playing midi file %s\n",Temp);

/*		midiOutSetVolume(hStream, (dwVolumePercent<<16)+dwVolumePercent);*/
		Mix_FadeInMusic(music,0,2000);
		bPlaying = 1;
	}

#endif
}

void MIDI_Pause(void)
{
  //  printf("MIDI_Pause\n");
#ifndef PLATFORM_UNIX
	if(bPaused)
	   midiStreamRestart(hStream);
   else
      midiStreamPause(hStream);

	bPaused = !bPaused;
#endif

#ifdef USE_MIDI
	if(bPaused)
	  Mix_ResumeMusic();

	else
	  Mix_PauseMusic();

	bPaused = !bPaused;
#endif

}

void MIDI_Loop(int NewValue)
{
  printf("MIDI_Loop\n");
#ifndef PLATFORM_UNIX
  if (NewValue == 2)
    bLooped = !bLooped;
  else bLooped = NewValue;
#endif

#ifdef USE_MIDI
  if (NewValue == 2)
    bLooped = !bLooped;
  else bLooped = NewValue;

  MIDI_EndMusicFinished();
#endif
}

void ReInitMusic() {
	// called after exitting the menus and changing the music type
	// this is pretty crude, but doen't seem to break anything S.A

	if (strcmpi(bgmtype.string,"midi") == 0) {
		CDAudio_Stop();
		MIDI_Play(cl.midi_name);
	}

	if (strcmpi(bgmtype.string,"cd") == 0) {
		MIDI_Stop();
		CDAudio_Play ((byte)cl.cdtrack, true);
	}

	if (strcmpi(bgmtype.string,"none") == 0) {
		CDAudio_Stop();
		MIDI_Stop();
	}
}

void MIDI_Stop(void)
{
  //  printf("MIDI_Stop\n");
#ifndef PLATFORM_UNIX
	MMRESULT mmrRetVal;

   if(bFileOpen || bPlaying)// || uCallbackStatus != STATUS_CALLBACKDEAD)
   {
      bPlaying = bPaused = FALSE;
      if (uCallbackStatus != STATUS_CALLBACKDEAD && uCallbackStatus != STATUS_WAITINGFOREND)
         uCallbackStatus = STATUS_KILLCALLBACK;

      mmrRetVal = midiStreamStop(hStream);                        
		if (mmrRetVal != MMSYSERR_NOERROR)
      {
         MidiErrorMessageBox(mmrRetVal);
         return;
      }

		mmrRetVal = midiOutReset((HMIDIOUT)hStream);
		if(mmrRetVal != MMSYSERR_NOERROR)
      {
         MidiErrorMessageBox(mmrRetVal);
         return;
      }

		if(WaitForSingleObject(hBufferReturnEvent,DEBUG_CALLBACK_TIMEOUT) == WAIT_TIMEOUT)
      {
         //Con_Printf("Timed out waiting for MIDI callback\n");
         uCallbackStatus = STATUS_CALLBACKDEAD;
      }
   }

   if (uCallbackStatus == STATUS_CALLBACKDEAD)
   {
      uCallbackStatus = 0;
      if (bFileOpen)
      {
         ConverterCleanup();
         FreeBuffers();
         if (hStream)
         {
				mmrRetVal = midiStreamClose(hStream);
            if (mmrRetVal != MMSYSERR_NOERROR)
            {
               MidiErrorMessageBox(mmrRetVal);
            }
            hStream = NULL;
         }

         bFileOpen = FALSE;
      }
   }
#endif

#ifdef USE_MIDI
   if(bFileOpen || bPlaying)
   {
      Mix_HaltMusic();
      Mix_FreeMusic(music);
   }

   bPlaying = bPaused = 0;
   bFileOpen=0;

#endif
}

void MIDI_Cleanup(void)
{
  //  printf("MIDI_Cleanup\n");
#ifndef PLATFORM_UNIX
	MMRESULT mmrRetVal;

	MIDI_Stop();

	CloseHandle(hBufferReturnEvent);

	if(hStream)
	{
		mmrRetVal = midiStreamClose(hStream);
		if(mmrRetVal != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmrRetVal);
		}
		hStream = NULL;
	}
#endif

#ifdef USE_MIDI
	MIDI_Stop();
	Mix_CloseAudio();
#endif
}


/*****************************************************************************/
/* FreeBuffers()                                                             */
/*                                                                           */
/*   This function unprepares and frees all our buffers -- something we must */
/* do to work around a bug in MMYSYSTEM that prevents a device from playing  */
/* back properly unless it is closed and reopened after each stop.           */
/*****************************************************************************/
void FreeBuffers(void)
{
#ifndef PLATFORM_UNIX
   DWORD idx;
   MMRESULT mmrRetVal;

   if(bBuffersPrepared)
   {
      for(idx=0;idx<NUM_STREAM_BUFFERS;idx++)
		{
			mmrRetVal = midiOutUnprepareHeader((HMIDIOUT)hStream,&ciStreamBuffers[idx].mhBuffer,sizeof(MIDIHDR));
         if(mmrRetVal != MMSYSERR_NOERROR)
         {
            MidiErrorMessageBox(mmrRetVal);
         }
      }
         bBuffersPrepared = FALSE;
	}
    
	// Free our stream buffers...
   for(idx=0;idx<NUM_STREAM_BUFFERS;idx++)
      if(ciStreamBuffers[idx].mhBuffer.lpData)
      {
         GlobalFreePtr( ciStreamBuffers[idx].mhBuffer.lpData);
         ciStreamBuffers[idx].mhBuffer.lpData = NULL;
      }
#endif
}


/*****************************************************************************/
/* StreamBufferSetup()                                                       */
/*                                                                           */
/*    This function uses the filename stored in the global character array to*/
/* open a MIDI file. Then it goes tabout converting at least the first part of*/
/* that file into a midiStream buffer for playback.                          */
/*****************************************************************************/
BOOL StreamBufferSetup(char *Name)
{
#ifndef PLATFORM_UNIX
   int nChkErr;
   BOOL bFoundEnd = FALSE;
   DWORD dwConvertFlag, idx;
   MMRESULT mmrRetVal;
   MIDIPROPTIMEDIV mptd;

   if(!hStream)
	{
		mmrRetVal = midiStreamOpen(&hStream,&uMIDIDeviceID,(DWORD)1,(DWORD)MidiProc,(DWORD)0,CALLBACK_FUNCTION);
      if(mmrRetVal != MMSYSERR_NOERROR)
      {
         MidiErrorMessageBox(mmrRetVal);
         return(TRUE);
      }
	}

   for(idx=0;idx<NUM_STREAM_BUFFERS;idx++)
   {
      ciStreamBuffers[idx].mhBuffer.dwBufferLength = OUT_BUFFER_SIZE;
		ciStreamBuffers[idx].mhBuffer.lpData = GlobalAllocPtr(GHND,OUT_BUFFER_SIZE);
      if(ciStreamBuffers[idx].mhBuffer.lpData == NULL)
      {
         // Buffers we already allocated will be killed by WM_DESTROY
         // after we fail on the create by returning with -1
         return(-1);
      }
   }
   if(ConverterInit(Name))
      return(TRUE);

   // Initialize the volume cache array to some pre-defined value
   for(idx=0;idx<NUM_CHANNELS;idx++)
      dwVolCache[idx] = VOL_CACHE_INIT;

   mptd.cbStruct = sizeof(mptd);
   mptd.dwTimeDiv = ifs.dwTimeDivision;

	mmrRetVal = midiStreamProperty(hStream,(LPBYTE)&mptd,MIDIPROP_SET | MIDIPROP_TIMEDIV);
   if(mmrRetVal != MMSYSERR_NOERROR)
   {
      MidiErrorMessageBox(mmrRetVal);
      ConverterCleanup();
      return(TRUE);
   }

   nEmptyBuffers = 0;
   dwConvertFlag = CONVERTF_RESET;

   for(nCurrentBuffer=0;nCurrentBuffer<NUM_STREAM_BUFFERS;nCurrentBuffer++)
   {
      // Tell the converter to convert up to one entire buffer's length of output
      // data. Also, set a flag so it knows to reset any saved state variables it
      // may keep from call to call.
      ciStreamBuffers[nCurrentBuffer].dwStartOffset = 0;
      ciStreamBuffers[nCurrentBuffer].dwMaxLength = OUT_BUFFER_SIZE;
      ciStreamBuffers[nCurrentBuffer].tkStart = 0;
      ciStreamBuffers[nCurrentBuffer].bTimesUp = FALSE;

		nChkErr = ConvertToBuffer(dwConvertFlag,&ciStreamBuffers[nCurrentBuffer]);
      if (nChkErr != CONVERTERR_NOERROR)
      {
         if (nChkErr == CONVERTERR_DONE)
         {
            bFoundEnd = TRUE;
         }
         else
         {
            DebugPrint("Initial conversion pass failed");
            ConverterCleanup();
            return(TRUE);
         }
      }
      ciStreamBuffers[nCurrentBuffer].mhBuffer.dwBytesRecorded = ciStreamBuffers[nCurrentBuffer].dwBytesRecorded;

      if(!bBuffersPrepared)
		{
	      mmrRetVal = midiOutPrepareHeader((HMIDIOUT)hStream,&ciStreamBuffers[nCurrentBuffer].mhBuffer,sizeof(MIDIHDR));
         if(mmrRetVal != MMSYSERR_NOERROR)
         {
            MidiErrorMessageBox(mmrRetVal);
            ConverterCleanup();
            return(TRUE);
         }
		}

		mmrRetVal = midiStreamOut(hStream,&ciStreamBuffers[nCurrentBuffer].mhBuffer,sizeof(MIDIHDR));
      if(mmrRetVal != MMSYSERR_NOERROR)
      {
         MidiErrorMessageBox(mmrRetVal);
         break;
      }
      dwConvertFlag = 0;

      if(bFoundEnd)
         break;
   }

   bBuffersPrepared = TRUE;
   nCurrentBuffer = 0;
#endif 
   return(false);
}


#ifndef PLATFORM_UNIX
/*****************************************************************************/
/* MidiProc()                                                                */
/*                                                                           */
/*   This is the callback handler which continually refills MIDI data buffers*/
/* as they're returned to us from the audio subsystem.                       */
/*****************************************************************************/
void CALLBACK MidiProc(HMIDIIN hMidi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
   static int nWaitingBuffers = 0;
   MIDIEVENT *pme;
   MIDIHDR *pmh;
   MMRESULT mmrRetVal;
   int nChkErr;

   switch(uMsg)
   {
      case MOM_DONE:
         if(uCallbackStatus == STATUS_CALLBACKDEAD)
		   {
            return;
			}

         nEmptyBuffers++;

         if(uCallbackStatus == STATUS_WAITINGFOREND)
         {
            if(nEmptyBuffers < NUM_STREAM_BUFFERS)
            {
               return;
            }
            else
            {
               uCallbackStatus = STATUS_CALLBACKDEAD;
				   MIDI_Stop();
				   SetEvent(hBufferReturnEvent);
               return;
            }
         }

         // This flag is set whenever the callback is waiting for all buffers to
         // come back.
         if(uCallbackStatus == STATUS_KILLCALLBACK)
         {
            // Count NUM_STREAM_BUFFERS-1 being returned for the last time
            if(nEmptyBuffers < NUM_STREAM_BUFFERS)
            {
               return;
            }
            // Then send a stop message when we get the last buffer back...
            else
            {
               // Change the status to callback dead
               uCallbackStatus = STATUS_CALLBACKDEAD;
	   			SetEvent( hBufferReturnEvent );
               return;
            }
         }

         dwProgressBytes += ciStreamBuffers[nCurrentBuffer].mhBuffer.dwBytesRecorded;

			///////////////////////////////////////////////////////////////////////////////
			// Fill an available buffer with audio data again...

		   if(bPlaying && nEmptyBuffers)
         {
            ciStreamBuffers[nCurrentBuffer].dwStartOffset = 0;
				ciStreamBuffers[nCurrentBuffer].dwMaxLength = OUT_BUFFER_SIZE;
				ciStreamBuffers[nCurrentBuffer].tkStart = 0;
				ciStreamBuffers[nCurrentBuffer].dwBytesRecorded = 0;
				ciStreamBuffers[nCurrentBuffer].bTimesUp = FALSE;

				nChkErr = ConvertToBuffer(0,&ciStreamBuffers[nCurrentBuffer]);
				if(nChkErr != CONVERTERR_NOERROR)
            {
               if(nChkErr == CONVERTERR_DONE)
               {
                  // Don't include this one in the count
		            nWaitingBuffers = NUM_STREAM_BUFFERS - 1;
				      uCallbackStatus = STATUS_WAITINGFOREND;
						return;
					}
					else
					{
						Con_Printf( "MidiProc() conversion pass failed!" );
						ConverterCleanup();
						return;
					}
            }

				ciStreamBuffers[nCurrentBuffer].mhBuffer.dwBytesRecorded = ciStreamBuffers[nCurrentBuffer].dwBytesRecorded;

				mmrRetVal = midiStreamOut(hStream,&ciStreamBuffers[nCurrentBuffer].mhBuffer,sizeof(MIDIHDR));
				if(mmrRetVal != MMSYSERR_NOERROR )
            {
					MidiErrorMessageBox( mmrRetVal );
					ConverterCleanup();
					return;
            }
				nCurrentBuffer = ( nCurrentBuffer + 1 ) % NUM_STREAM_BUFFERS;
				nEmptyBuffers--;
			}
			break;

		case MOM_POSITIONCB:
			pmh = (MIDIHDR *)dwParam1;
			pme = (MIDIEVENT *)(pmh->lpData + pmh->dwOffset);
			if (MIDIEVENT_TYPE( pme->dwEvent ) == MIDI_CTRLCHANGE)
         {
            if (MIDIEVENT_DATA1( pme->dwEvent ) == MIDICTRL_VOLUME_LSB)
            {
               DebugPrint( "Got an LSB volume event" );
               break;
            }
            if (MIDIEVENT_DATA1( pme->dwEvent ) != MIDICTRL_VOLUME)
               break;

				// Mask off the channel number and cache the volume data byte
				dwVolCache[ MIDIEVENT_CHANNEL( pme->dwEvent )] = MIDIEVENT_VOLUME( pme->dwEvent );
				// Post a message so that the main program knows to counteract
				// the effects of the volume event in the stream with its own
				// generated event which reflects the proper trackbar position.
/*				PostMessage( hWndMain, WM_MSTREAM_UPDATEVOLUME,MIDIEVENT_CHANNEL( pme->dwEvent ), 0L );*/

			}
			break;

		default:
			break;
   }

	return;
}


/****************************************************************************/
/* SetAllChannelVolumes()                                                   */
/*                                                                          */
/*   Given a percent in tenths of a percent, sets volume on all channels to */
/* reflect the new value.                                                   */
/****************************************************************************/
void SetAllChannelVolumes(DWORD dwVolumePercent)
{
   DWORD dwEvent, dwStatus, dwVol, idx;
   MMRESULT mmrRetVal;

/*   if(!bPlaying)
      return;*/

   for(idx=0,dwStatus=MIDI_CTRLCHANGE;idx<NUM_CHANNELS;idx++,dwStatus++)
   {
      dwVol = ( dwVolCache[idx] * dwVolumePercent ) / 1000;
      dwEvent = dwStatus | ((DWORD)MIDICTRL_VOLUME << 8) | ((DWORD)dwVol << 16);
		mmrRetVal = midiOutShortMsg((HMIDIOUT)hStream, dwEvent);
      if(mmrRetVal != MMSYSERR_NOERROR )
      {
         MidiErrorMessageBox(mmrRetVal);
         return;
      }
   }
}


/****************************************************************************/
/* SetChannelVolume()                                                       */
/*                                                                          */
/*   Given a percent in tenths of a percent, sets volume on a specified     */
/* channel to reflect the new value.                                        */
/****************************************************************************/
void SetChannelVolume(DWORD dwChannel, DWORD dwVolumePercent)
{
   DWORD dwEvent, dwVol;
   MMRESULT mmrRetVal;

   if(!bPlaying)
      return;

   dwVol = (dwVolCache[dwChannel] * dwVolumePercent) / 1000;
   dwEvent = MIDI_CTRLCHANGE | dwChannel | ((DWORD)MIDICTRL_VOLUME << 8) | ((DWORD)dwVol << 16);

	mmrRetVal = midiOutShortMsg((HMIDIOUT)hStream, dwEvent);
   if(mmrRetVal != MMSYSERR_NOERROR )
   {
      MidiErrorMessageBox( mmrRetVal );
      return;
   }
}
#endif	// !PLATFORM_UNIX
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/12/18 13:20:37  sezero
 * make the music automatically restart when changed in the options menu
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:05:13  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.8  2002/01/04 13:50:06  phneutre
 * music looping fix
 *
 * Revision 1.7  2002/01/02 14:25:59  phneutre
 * force -nomidi if -nosound is set
 *
 * Revision 1.6  2002/01/01 00:55:21  phneutre
 * fixed compilation problem when -DUSEMIDI is not set
 *
 * Revision 1.5  2001/12/15 14:35:27  phneutre
 * more midi stuff (loop and volume)
 *
 * Revision 1.4  2001/12/13 17:28:38  phneutre
 * use ~/.aot/ instead of data1/ to create meshes or extract temp files
 *
 * Revision 1.3  2001/12/13 16:48:55  phneutre
 * create glhexen/midi subdir to copy .mid files
 *
 * Revision 1.2  2001/12/11 19:17:50  phneutre
 * initial support for MIDI music (requieres SDL_mixer, see Makefile
 *
 * 3     2/12/98 12:26a Jmonroe
 * finished -nomidi to not try to play
 * 
 * 2     1/15/98 10:04p Jmonroe
 * added stub mpack menu stuff
 * 
 * 3     4/24/97 3:32p Bgokey
 * 
 * 2     4/24/97 12:13p Bgokey
 * 
 * 1     2/27/97 4:04p Rjohnson
 * Initial Revision
 */
