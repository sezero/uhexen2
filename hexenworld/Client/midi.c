/*
	midi.c

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/midi.c,v 1.2 2005-01-29 03:31:28 sezero Exp $
*/

// FIXME: midi doesnt work for HWClient at the moment

#ifndef PLATFORM_UNIX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <memory.h>
#include <mmreg.h>

#include "midstuff.h"
#include "midi.h"
#endif
#include "quakedef.h"

#ifdef USE_MIDI
#error FIXME: midi doesnt work for HWClient at the moment
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

static Mix_Music *music = NULL;
void (*midi_endmusicfnc)(void);
#endif


byte bMidiInited,bFileOpen, bPlaying, bBuffersPrepared;
byte bPaused, bLooped;
float dwVolumePercent;

#ifndef PLATFORM_UNIX
BOOL bMidiInited,bFileOpen, bPlaying, bBuffersPrepared;
BOOL bPaused, bLooped;
UINT uMIDIDeviceID = MIDI_MAPPER, uCallbackStatus;
int nCurrentBuffer, nEmptyBuffers;
DWORD dwBufferTickLength, dwTempoMultiplier, dwCurrentTempo, dwProgressBytes;
DWORD dwVolumePercent, dwVolCache[NUM_CHANNELS];

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


#endif



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
	MIDI_Pause(0);
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
}

qboolean MIDI_Init(void)
{
#ifdef USE_MIDI
  int audio_rate = 22050;
  int audio_format = AUDIO_S16;
  int audio_channels = 2;
  int audio_buffers = 4096;

  printf("MIDI_Init\n");

  if(COM_CheckParm("-nomidi"))
    {
      bMidiInited = 0;
      return 0;
    }

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

	printf("MIDI_Play\n");
	if (!bMidiInited)	//don't try to play if there is no midi
		return;

	sprintf(Temp, "midi/%s.mid", Name);
	sprintf (midi_file, "glhexen/midi/%s.mid", Name);

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
	  
	  COM_WriteFile (midi_file, (void *)Data, size);
	}

	sprintf (midi_file_with_path, "%s/glhexen/midi/%s.mid", com_gamedir,Name);
	music = Mix_LoadMUS(midi_file_with_path);
	if ( music == NULL ) {
	  Con_Printf("Couldn't load %s: %s\n",
		  Temp, SDL_GetError());
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

void MIDI_Pause(int mode)
{
  printf("MIDI_Pause\n");
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
}

void MIDI_Stop(void)
{
  printf("MIDI_Stop\n");
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
  printf("MIDI_Cleanup\n");
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

#endif
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/28 08:54:52  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.4  2002/01/02 19:38:35  phneutre
 * midi updates, though it still dosen't work at this time
 *
 * Revision 1.3  2001/12/13 14:01:59  phneutre
 * midi support
 *
 * Revision 1.2  2001/12/02 00:22:17  theoddone33
 * Initial client porting, software mode works
 *
 * Revision 1.1.1.1  2001/11/09 17:04:49  theoddone33
 * Inital import
 *
 * 
 * 7     3/27/98 6:22p Jmonroe
 * just made nomidi more clean
 * 
 * 6     3/07/98 6:20p Rjohnson
 * added a -nomidi command line parameter
 * 
 * 5     3/04/98 5:01p Rjohnson
 * Midi won't initialize if -nosound
 * 
 * 4     3/03/98 3:20p Rjohnson
 * Midi support for starting a map
 * 
 * 3     3/02/98 3:23p Rjohnson
 * Vid Updates
 * 
 * 2     2/19/98 3:42p Rjohnson
 * Midi Music - Yeah!
 * 
 * 1     2/19/98 3:23p Rjohnson
 * 
 * 3     4/24/97 3:32p Bgokey
 * 
 * 2     4/24/97 12:13p Bgokey
 * 
 * 1     2/27/97 4:04p Rjohnson
 * Initial Revision
 */
