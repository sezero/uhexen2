/*
	midi_sdl.c
	midiplay via SDL_mixer

	$Id: midi_sdl.c,v 1.1 2005-02-05 16:27:09 sezero Exp $
*/

#include "quakedef.h"

#ifdef USE_MIDI
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

static Mix_Music *music = NULL;
int audio_wasinit = 0;

void (*midi_endmusicfnc)(void);
#endif	// USE_MIDI

byte bMidiInited,bFileOpen, bPlaying, bBuffersPrepared;
byte bPaused, bLooped;
extern cvar_t bgmvolume;
float bgm_volume_old = -1.0f;


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

#ifndef USE_MIDI
// placeholders
void MIDI_Loop_f (void) {}
void MIDI_EndMusicFinished(void) {}
qboolean MIDI_Init(void) { return true; }
void MIDI_Play(char *Name) {}
void MIDI_Pause(void) {}
void MIDI_Loop(int NewValue) {}
void MIDI_Stop(void) {}
void MIDI_Cleanup(void) { Con_Printf("MIDI_Cleanup\n"); }
void MIDI_UpdateVolume(void) {}
#else
// actual functions
void MIDI_Loop_f (void)
{
	if (Cmd_Argc () == 2)
	{
		if (strcmpi(Cmd_Argv(1),"on") == 0 || strcmpi(Cmd_Argv(1),"1") == 0)
			MIDI_Loop(1);
		else if (strcmpi(Cmd_Argv(1),"off") == 0 || strcmpi(Cmd_Argv(1),"0") == 0)
			MIDI_Loop(0);
		else if (strcmpi(Cmd_Argv(1),"toggle") == 0)
			MIDI_Loop(2);
	}

	if (bLooped)
		Con_Printf("MIDI music will be looped\n");
	else
		Con_Printf("MIDI music will not be looped\n");
}

static void MIDI_SetVolume(float volume_frac)
{
	if (!bMidiInited)
		return;

	volume_frac = (volume_frac >= 0.0f) ? volume_frac : 0.0f;
	volume_frac = (volume_frac <= 1.0f) ? volume_frac : 1.0f;
	Mix_VolumeMusic(volume_frac*128); /* needs to be between 0 and 128 */
}

void MIDI_UpdateVolume(void)
{
	if (bgmvolume.value != bgm_volume_old) {
		bgm_volume_old = bgmvolume.value;
		MIDI_SetVolume(bgm_volume_old);
	}
}

void MIDI_EndMusicFinished(void)
{
	printf("Music finished\n");
	if (bLooped) {
		printf("Looping enabled\n");
		if (Mix_PlayingMusic())
			Mix_HaltMusic();

		printf("Playing again\n");
		Mix_RewindMusic();
		Mix_FadeInMusic(music,0,2000);
		bPlaying = 1;
	}
}

qboolean MIDI_Init(void)
{
#warning FIXME: midi doesnt play with snd_sdl.c
	int audio_rate = 22050;
	int audio_format = AUDIO_S16;
	int audio_channels = 2;
	int audio_buffers = 4096;
	char	mididir[MAX_OSPATH];

	if (COM_CheckParm("-nomidi") || COM_CheckParm("-nosound")
	   || COM_CheckParm("--nomidi") || COM_CheckParm("--nosound")) {

		bMidiInited = 0;
		return 0;
	}

	Con_Printf("MIDI_Init:\n");
	sprintf (mididir, "%s/.midi", com_userdir);
	Sys_mkdir (mididir);

	// Try initing the audio subsys if it hasn't been already
	audio_wasinit = SDL_WasInit(SDL_INIT_AUDIO);
	if (audio_wasinit == 0) {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
			Con_Printf("Cannot initialize SDL_AUDIO subsystem: %s\n",SDL_GetError());
			bMidiInited = 0;
			return 0;
		} else {
			Con_Printf("Audio subsystem opened for SDL_mixer.\n");
		}
	}
	// Someone else (-> snd_sdl.c) opened it already. Don' try.
	// But in this case, the following Mix_OpenAudio will fail anyway...

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		bMidiInited = 0;
		Con_Printf("SDL_mixer: open audio failed: %s\n", SDL_GetError());
		return 0;
	}

	midi_endmusicfnc = &MIDI_EndMusicFinished;
	if (midi_endmusicfnc)
		Mix_HookMusicFinished(midi_endmusicfnc);

	Cmd_AddCommand ("midi_play", MIDI_Play_f);
	Cmd_AddCommand ("midi_stop", MIDI_Stop_f);
	Cmd_AddCommand ("midi_pause", MIDI_Pause_f);
	Cmd_AddCommand ("midi_loop", MIDI_Loop_f);

	bFileOpen = 0;
	bPlaying = 0;
	bLooped = 1;
	bPaused = 0;
	bMidiInited = 1;

	return true;
}

void MIDI_Play(char *Name)
{
	char Temp[MAX_OSPATH];
	char *Data;
	char midi_file_with_path[MAX_OSPATH];
	FILE *f=NULL;

	int size=0;

	if (!bMidiInited)	//don't try to play if there is no midi
		return;

	//printf("MIDI_Play\n");
	sprintf(Temp, "midi/%s.mid", Name);
	sprintf (midi_file_with_path, ".midi/%s.mid", Name); // without userdir for now
	MIDI_Stop();

	COM_FOpenFile (midi_file_with_path, &f, true);
	if (f) {
		Sys_Printf("MIDI: File %s already exists\n",Temp);
		fclose(f);
	} else {
		Sys_Printf("MIDI: File %s needs to be extracted\n",Temp);
		Data = (byte *)COM_LoadHunkFile((char *)Temp);
		if (!Data) {
			Con_Printf("musicfile %s not found, not playing\n", Temp);
			return;
		}
		size = com_filesize;
		COM_WriteFile (midi_file_with_path, (void *)Data, size);
	}
	// now with full userdir path included, for SDL_mixer
	sprintf (midi_file_with_path, "%s/.midi/%s.mid", com_userdir, Name);
	music = Mix_LoadMUS(midi_file_with_path);
	if ( music == NULL ) {
		Sys_Printf("Couldn't load %s: %s\n", midi_file_with_path, SDL_GetError());
	} else {
		bFileOpen = 1;
		Con_Printf("Playing midi file %s\n",Temp);
		Mix_FadeInMusic(music,0,2000);
		bPlaying = 1;
	}
}

void MIDI_Pause(void)
{
	if (!bPlaying)
		return;

	//  printf("MIDI_Pause\n");
	if(bPaused)
		Mix_ResumeMusic();
	else
		Mix_PauseMusic();

	bPaused = !bPaused;
}

void MIDI_Loop(int NewValue)
{
	printf("MIDI_Loop\n");
	if (NewValue == 2)
		bLooped = !bLooped;
	else
		bLooped = NewValue;

	MIDI_EndMusicFinished();
}

void MIDI_Stop(void)
{
	// printf("MIDI_Stop\n");
	if (!bMidiInited)	//Just to be safe
		return;

	if(bFileOpen || bPlaying) {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
	}

	bPlaying = bPaused = 0;
	bFileOpen=0;
}

void MIDI_Cleanup(void)
{
	Con_Printf("MIDI_Cleanup\n");
	if ( bMidiInited == 1 ) {
		MIDI_Stop();
		Con_Printf("Closing SDL_mixer for midi music.\n");
		Mix_CloseAudio();
		// I'd better do this here...
		if (audio_wasinit == 0) {
			Con_Printf("Closing Audio subsystem for SDL_mixer.\n");
			SDL_CloseAudio();
		}
	}
}
#endif

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


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/02/05 16:21:13  sezero
 * killed Com_LoadHunkFile2()  [from HexenWorld]
 *
 * Revision 1.4  2005/02/05 16:20:14  sezero
 * fix possible path length overflows
 *
 * Revision 1.3  2005/02/05 16:18:25  sezero
 * added midi volume control (partially from Pa3PyX)
 *
 * Revision 1.2  2005/02/05 16:17:29  sezero
 * - Midi file paths cleanup. these should be leftovers
 *   from times when gamedir and userdir were the same.
 * - Killed Com_WriteFileFullPath(), not used anymore.
 * - Replaced some Con_Printf() with Sys_Printf().
 *
 * Revision 1.1  2005/02/05 16:16:06  sezero
 * separate win32 and linux versions of midi files. too much mess otherwise.
 *
 *
 * 2005/02/04 14:00:14  sezero
 * - merge small bits from the hexenworld version
 * - rest of the cleanup
 *
 * 2005/02/04 13:51:31  sezero
 * midi fixes for correctness' sake. it still fails with snd_sdl.
 *
 * 2005/02/04 11:28:59  sezero
 * make sdl_audio actually work (finally)
 *
 * 2004/12/18 13:20:37  sezero
 * make the music automatically restart when changed in the options
 * menu (stevena)
 *
 * 2002/01/04 13:50:06  phneutre
 * music looping fix
 *
 * 2002/01/02 14:25:59  phneutre
 * force -nomidi if -nosound is set
 *
 * 2002/01/01 00:55:21  phneutre
 * fixed compilation problem when -DUSEMIDI is not set
 *
 * 2001/12/15 14:35:27  phneutre
 * more midi stuff (loop and volume)
 *
 * 2001/12/13 17:28:38  phneutre
 * use ~/.aot/ instead of data1/ to create meshes or extract temp files
 *
 * 2001/12/13 16:48:55  phneutre
 * create glhexen/midi subdir to copy .mid files
 *
 * 2001/12/11 19:17:50  phneutre
 * initial support for MIDI music (requieres SDL_mixer, see Makefile
 *
 */
