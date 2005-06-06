/*
	snd_dma.c
	main control for any streaming sound output device

	$Id: snd_dma.c,v 1.19 2005-06-06 10:17:05 sezero Exp $
*/

#include "quakedef.h"

#ifndef PLATFORM_UNIX
#include "quakeinc.h"
#endif

void S_Play(void);
void S_PlayVol(void);
void S_SoundList(void);
void S_Update_();
void S_StopAllSounds(qboolean clear);
void S_StopAllSoundsC(void);

#ifdef H2W
// QuakeWorld hack...
#define	viewentity	playernum+1
#endif

// =======================================================================
// Internal sound data & structures
// =======================================================================

channel_t   channels[MAX_CHANNELS];
int		total_channels;

int		snd_blocked = 0;
static qboolean	snd_ambient = 1;
qboolean	snd_initialized = false;

// pointer should go away
volatile dma_t  *shm = 0;
volatile dma_t sn;

vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;
vec_t		sound_nominal_clip_dist=1000.0;

int		soundtime;	// sample PAIRS
int		paintedtime;	// sample PAIRS


#define	MAX_SFX		512
sfx_t		*known_sfx;	// hunk allocated [MAX_SFX]
int		num_sfx;

sfx_t		*ambient_sfx[NUM_AMBIENTS];

int 		desired_speed = 11025;
int 		desired_bits = 16;
int 		desired_channels = 2;

int sound_started=0;
int tryrates[MAX_TRYRATES] = { 11025, 22051, 44100, 8000 };

cvar_t bgmvolume = {"bgmvolume", "1", true};
cvar_t bgmtype = {"bgmtype", "cd", true};   // cd or midi
cvar_t sfxvolume = {"volume", "0.7", true};

cvar_t nosound = {"nosound", "0"};
cvar_t precache = {"precache", "1"};
cvar_t loadas8bit = {"loadas8bit", "0"};
cvar_t bgmbuffer = {"bgmbuffer", "4096"};
cvar_t ambient_level = {"ambient_level", "0.3"};
cvar_t ambient_fade = {"ambient_fade", "100"};
cvar_t snd_noextraupdate = {"snd_noextraupdate", "0"};
cvar_t snd_show = {"snd_show", "0"};
cvar_t _snd_mixahead = {"_snd_mixahead", "0.1", true};


void S_AmbientOff (void)
{
	snd_ambient = false;
}


void S_AmbientOn (void)
{
	snd_ambient = true;
}


void S_SoundInfo_f(void)
{
	char *s_sys = NULL;
	if (!sound_started || !shm)
	{
		Con_Printf ("sound system not started\n");
		return;
	}
	
	switch (snd_system) {
	case S_SYS_OSS:
		s_sys = "OSS";
		break;
	case S_SYS_SDL:
		s_sys = "SDL";
		break;
#ifndef NO_ALSA
	case S_SYS_ALSA:
		s_sys = "ALSA";
		break;
#endif
	}
	Con_Printf("Driver: %s\n", s_sys);
	Con_Printf("%5d stereo\n", shm->channels - 1);
	Con_Printf("%5d samples\n", shm->samples);
	Con_Printf("%5d samplepos\n", shm->samplepos);
	Con_Printf("%5d samplebits\n", shm->samplebits);
	Con_Printf("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf("%5d speed\n", shm->speed);
	Con_Printf("0x%x dma buffer\n", shm->buffer);
	Con_Printf("%5d total_channels\n", total_channels);
}


#ifdef PLATFORM_UNIX
void S_GetSubsys (void)
{
	switch (snd_system) {
		case S_SYS_SDL:
			SNDDMA_Init	 = S_SDL_Init;
			SNDDMA_GetDMAPos = S_SDL_GetDMAPos;
			SNDDMA_Shutdown	 = S_SDL_Shutdown;
			SNDDMA_Submit	 = S_SDL_Submit;
			break;
#ifndef NO_ALSA
		case S_SYS_ALSA:
			SNDDMA_Init	 = S_ALSA_Init;
			SNDDMA_GetDMAPos = S_ALSA_GetDMAPos;
			SNDDMA_Shutdown	 = S_ALSA_Shutdown;
			SNDDMA_Submit	 = S_ALSA_Submit;
			break;
#endif
		case S_SYS_OSS:
			SNDDMA_Init	 = S_OSS_Init;
			SNDDMA_GetDMAPos = S_OSS_GetDMAPos;
			SNDDMA_Shutdown	 = S_OSS_Shutdown;
			SNDDMA_Submit	 = S_OSS_Submit;
			break;
		case S_SYS_NULL:
		default:
		// Paranoia: We should never have come this far!..
		// No function to point at, set snd_initialized to false.
			snd_initialized = false;
			break;
	}
}
#endif	// PLATFORM_UNIX


/*
================
S_Startup
================
*/
void S_Startup (void)
{
	int		rc, tmp;

#ifdef PLATFORM_UNIX
	S_GetSubsys();
#endif

	if (!snd_initialized)
		return;

	if ((tmp = COM_CheckParm("-sndspeed")) != 0)
	{
		tmp = atoi(com_argv[tmp+1]);
		/* I won't rely on users' precision in typing or their needs
		   here. If you know what you're doing, then change this. */
		switch (tmp) {
			case  8000:
			case 11025:
			case 22050:
			case 22051:
			case 44100:
			case 48000:
				desired_speed = tmp;
				break;
			default:
				break;
		}
	}

	if ((tmp = COM_CheckParm("-sndbits")) != 0)
	{
		tmp = atoi(com_argv[tmp+1]);
		if ((tmp == 16) || (tmp == 8))
			desired_bits = tmp;
	}

	if ((tmp = COM_CheckParm("-sndmono")) != 0)
		desired_channels = 1;

	rc = SNDDMA_Init();

	if (!rc)
	{
#ifndef	_WIN32
		Con_Printf("S_Startup: SNDDMA_Init failed.\n");
#endif
		sound_started = 0;
		return;
	}
	sound_started = 1;
}


/*
================
S_Init
================
*/
void S_Init (void)
{

	// We need this thing even if the snd won't be initalized
	// at all. Fixes the demos without models with -nosound
	Cvar_RegisterVariable(&precache);

	if (COM_CheckParm("-nosound") || COM_CheckParm("--nosound")
			|| COM_CheckParm("-s"))
		return;

	Con_Printf("\nSound Initialization\n");

	Cmd_AddCommand("play", S_Play);
	Cmd_AddCommand("playvol", S_PlayVol);
	Cmd_AddCommand("stopsound", S_StopAllSoundsC);
	Cmd_AddCommand("soundlist", S_SoundList);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);

	Cvar_RegisterVariable(&nosound);
	Cvar_RegisterVariable(&sfxvolume);
	Cvar_RegisterVariable(&loadas8bit);
	Cvar_RegisterVariable(&bgmvolume);
	Cvar_RegisterVariable(&bgmtype);
	Cvar_RegisterVariable(&bgmbuffer);
	Cvar_RegisterVariable(&ambient_level);
	Cvar_RegisterVariable(&ambient_fade);
	Cvar_RegisterVariable(&snd_noextraupdate);
	Cvar_RegisterVariable(&snd_show);
	Cvar_RegisterVariable(&_snd_mixahead);

	if (host_parms.memsize < 0x800000)
	{
		Cvar_Set ("loadas8bit", "1");
		Con_Printf ("loading all sounds as 8bit\n");
	}


	snd_initialized = true;

	S_Startup ();

	if (sound_started == 0)
		return;

	SND_InitScaletable ();

	known_sfx = Hunk_AllocName (MAX_SFX*sizeof(sfx_t), "sfx_t");
	num_sfx = 0;

	if ( shm ) {
		Con_Printf ("Sound sampling rate: %i\n", shm->speed);
	}

	// provides a tick sound until washed clean

//	if (shm->buffer)
//		shm->buffer[4] = shm->buffer[5] = 0x7f;	// force a pop for debugging

	ambient_sfx[AMBIENT_WATER] = S_PrecacheSound ("ambience/water1.wav");
	ambient_sfx[AMBIENT_SKY] = S_PrecacheSound ("ambience/wind2.wav");

	S_StopAllSounds (true);
}


// =======================================================================
// Shutdown sound engine
// =======================================================================
#ifndef PLATFORM_UNIX
extern HINSTANCE hInstDS;
#endif

void S_Shutdown(void)
{
	if (!sound_started)
		return;

/* This doesnt serve anything. Why do we still keep it in dma_t struct?..
	if (shm)
		shm->gamealive = 0;
*/
	sound_started = 0;

	SNDDMA_Shutdown();
	shm = NULL;

#ifndef PLATFORM_UNIX
	if (hInstDS)
	{
		FreeLibrary(hInstDS);
		hInstDS=NULL;
	}
#endif
}


// =======================================================================
// Load a sound
// =======================================================================

/*
==================
S_FindName

==================
*/
sfx_t *S_FindName (char *name)
{
	int		i;
	sfx_t	*sfx;

	if (!name)
		Sys_Error ("S_FindName: NULL\n");

	if (strlen(name) >= MAX_QPATH)
		Sys_Error ("Sound name too long: %s", name);

// see if already loaded
	for (i=0 ; i < num_sfx ; i++)
		if (!strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}

	if (num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");
	
	sfx = &known_sfx[i];
	strcpy (sfx->name, name);

	num_sfx++;
	
	return sfx;
}


/*
==================
S_TouchSound

==================
*/
void S_TouchSound (char *name)
{
	sfx_t	*sfx;
	
	if (!sound_started)
		return;

	sfx = S_FindName (name);
	Cache_Check (&sfx->cache);
}

/*
==================
S_PrecacheSound

==================
*/
sfx_t *S_PrecacheSound (char *name)
{
	sfx_t	*sfx;

	if (!sound_started || nosound.value)
		return NULL;

	sfx = S_FindName (name);
	
// cache it in
	if (precache.value)
		S_LoadSound (sfx);
	
	return sfx;
}


//=============================================================================

/*
=================
SND_PickChannel
=================
*/
channel_t *SND_PickChannel(int entnum, int entchannel)
{
    int ch_idx;
    int first_to_die;
    int life_left;

// Check for replacement sound, or find the best one to replace
    first_to_die = -1;
    life_left = 0x7fffffff;
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++)
    {
		if (entchannel != 0		// channel 0 never overrides
		&& channels[ch_idx].entnum == entnum
		&& (channels[ch_idx].entchannel == entchannel || entchannel == -1) )
		{	// allways override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && channels[ch_idx].sfx)
			continue;

		if (channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
   }

	if (first_to_die == -1)
		return NULL;

	if (channels[first_to_die].sfx)
		channels[first_to_die].sfx = NULL;

    return &channels[first_to_die];    
}       

/*
=================
SND_Spatialize
=================
*/
void SND_Spatialize(channel_t *ch)
{
    vec_t dot;
    vec_t dist;
    vec_t lscale, rscale, scale;
    vec3_t source_vec;
    sfx_t *snd;

// anything coming from the view entity will allways be full volume
	if (ch->entnum == cl.viewentity)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

// calculate stereo seperation and distance attenuation

	snd = ch->sfx;
	VectorSubtract(ch->origin, listener_origin, source_vec);
	
	dist = VectorNormalize(source_vec) * ch->dist_mult;
	
	dot = DotProduct(listener_right, source_vec);

	if (shm->channels == 1)
	{
		rscale = 1.0;
		lscale = 1.0;
	}
	else
	{
		rscale = 1.0 + dot;
		lscale = 1.0 - dot;
	}

// add in distance effect
	scale = (1.0 - dist) * rscale;
	ch->rightvol = (int) (ch->master_vol * scale);
	if (ch->rightvol < 0)
		ch->rightvol = 0;

	scale = (1.0 - dist) * lscale;
	ch->leftvol = (int) (ch->master_vol * scale);
	if (ch->leftvol < 0)
		ch->leftvol = 0;
}           


// =======================================================================
// Start a sound effect
// =======================================================================

void S_StartSound(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
	channel_t *target_chan, *check;
	sfxcache_t	*sc;
	int		vol;
	int		ch_idx;
	int		skip;
	qboolean skip_dist_check = false;

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound.value)
		return;

	vol = fvol*255;

// pick a channel to play on
	target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;
		
	if(attenuation==4)//Looping sound- always play
	{
		skip_dist_check=true;
		attenuation=1;//was 3 - static
	}

// spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);

	if (!skip_dist_check)
		if (!target_chan->leftvol && !target_chan->rightvol)
			return;		// not audible at all

// new channel
	sc = S_LoadSound (sfx);
	if (!sc)
	{
		target_chan->sfx = NULL;
		return;		// couldn't load the sound's data
	}

	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
    target_chan->end = paintedtime + sc->length;	

// if an identical sound has also been started this frame, offset the pos
// a bit to keep it from just making the first one louder
	check = &channels[NUM_AMBIENTS];
    for (ch_idx=NUM_AMBIENTS ; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; ch_idx++, check++)
    {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
			skip = 0.1 * shm->speed;
			if (skip > sc->length)
				skip = sc->length;
			if (skip > 0)
				skip = rand() % skip;
			target_chan->pos += skip;
			target_chan->end -= skip;
			break;
		}
		
	}
}

void S_StopSound(int entnum, int entchannel)
{
	int i;

	for (i=NUM_AMBIENTS  ; i<NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; i++)
	{
		if (channels[i].entnum == entnum
			&& ((!entchannel)||channels[i].entchannel == entchannel))	// 0 matches any
		{
			channels[i].end = 0;
			channels[i].sfx = NULL;
			if (entchannel)
				return;	//got a match, not looking for more.
		}
	}
}

void S_UpdateSoundPos (int entnum, int entchannel, vec3_t origin)
{
	int i;

	for (i=NUM_AMBIENTS  ; i<NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS ; i++)
	{
		if (channels[i].entnum == entnum
			&& channels[i].entchannel == entchannel)
		{
			VectorCopy(origin, channels[i].origin);
			return;
		}
	}
}

void S_StopAllSounds(qboolean clear)
{
	int		i;

	if (!sound_started)
		return;

	total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

	for (i=0 ; i<MAX_CHANNELS ; i++)
		if (channels[i].sfx)
			channels[i].sfx = NULL;

	memset(channels, 0, MAX_CHANNELS * sizeof(channel_t));

	if (clear)
		S_ClearBuffer ();
}

void S_StopAllSoundsC (void)
{
	S_StopAllSounds (true);
}

void S_ClearBuffer (void)
{
	int		clear;
		
#ifdef _WIN32
	if (!sound_started || !shm || (!shm->buffer && !pDSBuf))
#else
	if (!sound_started || !shm || !shm->buffer)
#endif
		return;

	if (shm->samplebits == 8)
		clear = 0x80;
	else
		clear = 0;

#ifdef _WIN32
	if (pDSBuf)
	{
		DWORD	dwSize;
		DWORD	*pData;
		int		reps;
		HRESULT	hresult;

		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pData, &dwSize, NULL, NULL, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_ClearBuffer: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_ClearBuffer: DS: couldn't restore buffer\n");
				S_Shutdown ();
				return;
			}
		}

		memset(pData, clear, shm->samples * shm->samplebits/8);

		pDSBuf->lpVtbl->Unlock(pDSBuf, pData, dwSize, NULL, 0);
	
	}
	else
#endif
	{
		memset(shm->buffer, clear, shm->samples * shm->samplebits/8);
	}
}


/*
=================
S_StaticSound
=================
*/
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
	channel_t	*ss;
	sfxcache_t		*sc;

	if (!sfx)
		return;

	if (total_channels == MAX_CHANNELS)
	{
		Con_Printf ("StaticSound: MAX_CHANNELS reached\n");
	//	Con_Printf (" failed at (%.2f, %.2f, %.2f)\n",origin[0],origin[1],origin[2]);
		return;
	}

	ss = &channels[total_channels];
	total_channels++;

	sc = S_LoadSound (sfx);
	if (!sc)
		return;

	if (sc->loopstart == -1)
	{
		Con_Printf ("Sound %s not looped\n", sfx->name);
		return;
	}
	
	ss->sfx = sfx;
	VectorCopy (origin, ss->origin);
	ss->master_vol = vol;
	ss->dist_mult = (attenuation/64) / sound_nominal_clip_dist;
    ss->end = paintedtime + sc->length;	
	
	SND_Spatialize (ss);
}


//=============================================================================

/*
===================
S_UpdateAmbientSounds
===================
*/
void S_UpdateAmbientSounds (void)
{
	mleaf_t		*l;
	float		vol;
	int			ambient_channel;
	channel_t	*chan;

	if (!snd_ambient)
		return;

// calc ambient sound levels
	if (!cl.worldmodel)
		return;

	l = Mod_PointInLeaf (listener_origin, cl.worldmodel);
	if (!l || !ambient_level.value)
	{
		for (ambient_channel = 0 ; ambient_channel< NUM_AMBIENTS ; ambient_channel++)
			channels[ambient_channel].sfx = NULL;
		return;
	}

	for (ambient_channel = 0 ; ambient_channel< NUM_AMBIENTS ; ambient_channel++)
	{
		chan = &channels[ambient_channel];	
		chan->sfx = ambient_sfx[ambient_channel];
	
		vol = ambient_level.value * l->ambient_sound_level[ambient_channel];
		if (vol < 8)
			vol = 0;

	// don't adjust volume too fast
		if (chan->master_vol < vol)
		{
			chan->master_vol += host_frametime * ambient_fade.value;
			if (chan->master_vol > vol)
				chan->master_vol = vol;
		}
		else if (chan->master_vol > vol)
		{
			chan->master_vol -= host_frametime * ambient_fade.value;
			if (chan->master_vol < vol)
				chan->master_vol = vol;
		}
		
		chan->leftvol = chan->rightvol = chan->master_vol;
	}
}


/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	int			i, j;
	int			total;
	channel_t	*ch;
	channel_t	*combine;

	if (!sound_started || (snd_blocked > 0))
		return;

	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);
	
// update general area ambient sound sources
	S_UpdateAmbientSounds ();

	combine = NULL;

// update spatialization for static and dynamic sounds	
	ch = channels+NUM_AMBIENTS;
	for (i=NUM_AMBIENTS ; i<total_channels; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		SND_Spatialize(ch);         // respatialize channel
		if (!ch->leftvol && !ch->rightvol)
			continue;

	// try to combine static sounds with a previous channel of the same
	// sound effect so we don't mix five torches every frame
	
		if (i >= MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS)
		{
		// see if it can just use the last one
			if (combine && combine->sfx == ch->sfx)
			{
				combine->leftvol += ch->leftvol;
				combine->rightvol += ch->rightvol;
				ch->leftvol = ch->rightvol = 0;
				continue;
			}
		// search for one
			combine = channels+MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
			for (j=MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS ; j<i; j++, combine++)
				if (combine->sfx == ch->sfx)
					break;
					
			if (j == total_channels)
			{
				combine = NULL;
			}
			else
			{
				if (combine != ch)
				{
					combine->leftvol += ch->leftvol;
					combine->rightvol += ch->rightvol;
					ch->leftvol = ch->rightvol = 0;
				}
				continue;
			}
		}
		
		
	}

//
// debugging output
//
	if (snd_show.value)
	{
		total = 0;
		ch = channels;
		for (i=0 ; i<total_channels; i++, ch++)
			if (ch->sfx && (ch->leftvol || ch->rightvol) )
			{
				//Con_Printf ("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total++;
			}
		
		Con_Printf ("----(%i)----\n", total);
	}

// mix some sound
	S_Update_();
}

void GetSoundtime(void)
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;
	int		fullsamples;
	
	fullsamples = shm->samples / shm->channels;

// it is possible to miscount buffers if it has wrapped twice between
// calls to S_Update.  Oh well. --- S.A well indeeed
	samplepos = SNDDMA_GetDMAPos();

	if (samplepos < oldsamplepos)
	{
		buffers++;					// buffer wrapped
		
		if (paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			paintedtime = fullsamples;
			S_StopAllSounds (true);
		}
	}
	oldsamplepos = samplepos;

	soundtime = buffers*fullsamples + samplepos/shm->channels;
}

void S_ExtraUpdate (void)
{

#ifdef _WIN32
	IN_Accumulate ();
#endif

	if (snd_noextraupdate.value)
		return;		// don't pollute timings
	S_Update_();
}



void S_Update_(void)
{
 if (snd_system != S_SYS_SDL) {
	unsigned        endtime;
	int				samps;
	
	if (!sound_started || (snd_blocked > 0))
		return;

// Updates DMA time
	GetSoundtime();

// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
	{
		//Con_Printf ("S_Update_ : overflow\n");
		paintedtime = soundtime;
	}

// mix ahead of current position
	endtime = soundtime + _snd_mixahead.value * shm->speed;
	samps = shm->samples >> (shm->channels-1);
	if (endtime - soundtime > samps)
		endtime = soundtime + samps;

#ifdef _WIN32
// if the buffer was lost or stopped, restore it and/or restart it
	{
		DWORD	dwStatus;

		if (pDSBuf)
		{
			if (pDSBuf->lpVtbl->GetStatus (pDSBuf, &dwStatus) != DD_OK)
				Con_Printf ("Couldn't get sound buffer status\n");
			
			if (dwStatus & DSBSTATUS_BUFFERLOST)
				pDSBuf->lpVtbl->Restore (pDSBuf);
			
			if (!(dwStatus & DSBSTATUS_PLAYING))
				pDSBuf->lpVtbl->Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);
		}
	}
#endif

	S_PaintChannels (endtime);

	SNDDMA_Submit ();
 }
}

/*
===============================================================================

console functions

===============================================================================
*/

void S_Play(void)
{
	static int hash=345;
	int 	i;
	char name[256];
	sfx_t	*sfx;
	
	i = 1;
	while (i<Cmd_Argc())
	{
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			strcpy(name, Cmd_Argv(i));
			strcat(name, ".wav");
		}
		else
			strcpy(name, Cmd_Argv(i));
		sfx = S_PrecacheSound(name);
		S_StartSound(hash++, 0, sfx, listener_origin, 1.0, 1.0);
		i++;
	}
}

void S_PlayVol(void)
{
	static int hash=543;
	int i;
	float vol;
	char name[256];
	sfx_t	*sfx;
	
	i = 1;
	while (i<Cmd_Argc())
	{
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			strcpy(name, Cmd_Argv(i));
			strcat(name, ".wav");
		}
		else
			strcpy(name, Cmd_Argv(i));
		sfx = S_PrecacheSound(name);
		vol = atof(Cmd_Argv(i+1));
		S_StartSound(hash++, 0, sfx, listener_origin, vol, 1.0);
		i+=2;
	}
}

void S_SoundList(void)
{
	int		i;
	sfx_t	*sfx;
	sfxcache_t	*sc;
	int		size, total;

	total = 0;
	for (sfx=known_sfx, i=0 ; i<num_sfx ; i++, sfx++)
	{
		sc = Cache_Check (&sfx->cache);
		if (!sc)
			continue;
		size = sc->length*sc->width*(sc->stereo+1);
		total += size;
		if (sc->loopstart >= 0)
			Con_Printf ("L");
		else
			Con_Printf (" ");
		Con_Printf("(%2db) %6i : %s\n",sc->width*8,  size, sfx->name);
	}
	Con_Printf ("Total resident: %i\n", total);
}


void S_LocalSound (char *sound)
{
	sfx_t	*sfx;

	if (nosound.value)
		return;
	if (!sound_started)
		return;
		
	sfx = S_PrecacheSound (sound);
	if (!sfx)
	{
		Con_Printf ("S_LocalSound: can't cache %s\n", sound);
		return;
	}
	S_StartSound (cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}


void S_ClearPrecache (void)
{
}


void S_BeginPrecaching (void)
{
}


void S_EndPrecaching (void)
{
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.18  2005/05/20 15:26:34  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 * Revision 1.17  2005/05/19 10:58:02  sezero
 * synced h2 and hw versions of snd_dma.c (use the new H2W define)
 *
 * Revision 1.16  2005/04/30 08:39:09  sezero
 * silenced shadowed decleration warnings about volume (now sfxvolume)
 *
 * Revision 1.15  2005/03/03 17:02:37  sezero
 * - fix and activate munmap of shm->buffer for snd_oss.c
 * - comment-out shm->gamealive thing which serves nothing
 *
 * Revision 1.14  2005/02/25 14:56:33  sezero
 * exact syncing between hexen2 and hw versions of snd_dma.c
 *
 * Revision 1.13  2005/02/20 12:46:43  sezero
 * - Process all command line options in snd_dma.c, S_Startup() only.
 *   Targets will do to its bidding first. And don't die immediately,
 *   try setting alternative hw parameters. (FWIW, snd_oss.c now applies
 *   all hardware settings before mmaping the buffer)
 * - Check for requested and set rate mismatches and fail (Found in alsa
 *   examples, is it necessary at all? Commented out for now.)
 *
 * Revision 1.12  2005/02/14 15:12:52  sezero
 * added ability to disable ALSA support at compile time
 *
 * Revision 1.11  2005/02/11 23:47:15  sezero
 * paranoid sound driver checks
 *
 * Revision 1.10  2005/02/09 14:33:37  sezero
 * make compiler happy (uninitialized warnings)
 *
 * Revision 1.9  2005/02/04 13:40:52  sezero
 * build all all the sound drivers in and choose from command line
 *
 * Revision 1.8  2005/02/04 11:42:10  sezero
 * kill fakedma (simsound). one less thing to worry about.
 *
 * Revision 1.7  2005/02/04 11:33:52  sezero
 * some snd_dma.c fixes from the tenebrae project
 *
 * Revision 1.6  2005/02/04 11:29:38  sezero
 * make sdl_audio actually work (finally)
 *
 * Revision 1.5  2005/02/01 13:38:22  sezero
 * fix the demos without models with -nosound
 *
 * Revision 1.4  2005/01/29 03:31:28  sezero
 * - initial style-only sound code syncing between h2 and hw
 * - add #error to midi.c in case somone defines USE_MIDI
 *
 * Revision 1.3  2005/01/01 21:50:49  sezero
 * warnings cleanup: unused stuff
 *
 * Revision 1.2  2004/12/05 12:25:58  sezero
 * Sync with Steven's changes to hexen2, 2004-12-04
 *
 * Revision 1.1.1.1  2004/11/28 08:56:46  sezero
 * Initial import of AoT 1.2.0 code
 */
