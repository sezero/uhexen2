/*
	snd_dma.c
	main control for any streaming sound output device

	$Id: snd_dma.c,v 1.62 2007-09-14 14:16:23 sezero Exp $
*/

#include "quakedef.h"
#include "snd_sys.h"

static void S_Play (void);
static void S_PlayVol (void);
static void S_ToggleMute(void);
static void S_VolumeDown(void);
static void S_VolumeUp(void);
static void S_SoundList (void);
static void S_Update_ (void);
void S_StopAllSounds (qboolean clear);
static void S_StopAllSoundsC (void);

#if defined(H2W)
// HexenWorld hack...
#define	viewentity	playernum+1
#endif

// =======================================================================
// Internal sound data & structures
// =======================================================================

channel_t   snd_channels[MAX_CHANNELS];
int		total_channels;

int		snd_blocked = 0;
static qboolean	snd_initialized = false;

static const struct
{
	char	*name;
	qboolean	available;
} snd_drivers[S_SYS_MAX] =
{
	{  "NULL" ,	1		},
	{  "OSS"  ,	HAVE_OSS_SOUND	},
	{  "SDL"  ,	HAVE_SDL_SOUND	},
	{  "ALSA" ,	HAVE_ALSA_SOUND	},
	{  "SUN"  ,	HAVE_SUN_SOUND	},
	{  "WIN"  ,	HAVE_WIN_SOUND	}
};

volatile dma_t	sn;
volatile dma_t	*shm = NULL;

vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;

#define	sound_nominal_clip_dist	1000.0

int		soundtime;	// sample PAIRS
int		paintedtime;	// sample PAIRS


#define	MAX_SFX		512
static sfx_t	*known_sfx = NULL;	// hunk allocated [MAX_SFX]
static int	num_sfx;

static sfx_t	*ambient_sfx[NUM_AMBIENTS];

static qboolean	sound_started = false;

int		desired_speed = 11025;
int		desired_bits = 16;
int		desired_channels = 2;
const int	tryrates[] = { 44100, 48000, 22050, 22051, 24000, 11025, 16000, 8000 };
const int	MAX_TRYRATES = sizeof(tryrates)/sizeof(tryrates[0]);

cvar_t		bgmvolume = {"bgmvolume", "1", CVAR_ARCHIVE};
cvar_t		bgmtype = {"bgmtype", "cd", CVAR_ARCHIVE};	// cd or midi
cvar_t		sfxvolume = {"volume", "0.7", CVAR_ARCHIVE};

cvar_t		precache = {"precache", "1", CVAR_NONE};
cvar_t		loadas8bit = {"loadas8bit", "0", CVAR_NONE};

static	cvar_t	sfx_mutedvol = {"sfx_mutedvol", "0", CVAR_ARCHIVE};
static	cvar_t	bgm_mutedvol = {"bgm_mutedvol", "0", CVAR_ARCHIVE};

static	cvar_t	nosound = {"nosound", "0", CVAR_NONE};
static	cvar_t	ambient_level = {"ambient_level", "0.3", CVAR_NONE};
static	cvar_t	ambient_fade = {"ambient_fade", "100", CVAR_NONE};
static	cvar_t	snd_noextraupdate = {"snd_noextraupdate", "0", CVAR_NONE};
static	cvar_t	snd_show = {"snd_show", "0", CVAR_NONE};
static	cvar_t	_snd_mixahead = {"_snd_mixahead", "0.1", CVAR_ARCHIVE};


static void S_SoundInfo_f (void)
{
	if (!sound_started || !shm)
	{
		Con_Printf ("sound system not started\n");
		return;
	}

	Con_Printf("Driver: %s\n", snd_drivers[snd_system].name);
	Con_Printf("%d bit, %s, %d Hz\n", shm->samplebits,
			(shm->channels == 2) ? "stereo" : "mono", shm->speed);
	Con_Printf("%5d samples\n", shm->samples);
	Con_Printf("%5d samplepos\n", shm->samplepos);
	Con_Printf("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf("%5d total_channels\n", total_channels);
	Con_Printf("0x%p dma buffer\n", shm->buffer);
}


/*
================
S_Startup
================
*/
void S_Startup (void)
{
	int		i, tmp;

	if (!snd_initialized)
		return;

	tmp = COM_CheckParm("-sndspeed");
	if (tmp != 0 && tmp < com_argc-1)
	{
		tmp = atoi(com_argv[tmp+1]);
		/* I won't rely on users' precision in typing or their needs
		   here. If you know what you're doing, then change this. */
		for (i = 0; i < MAX_TRYRATES; i++)
		{
			if (tmp == tryrates[i])
			{
				desired_speed = tmp;
				break;
			}
		}
	}

	tmp = COM_CheckParm("-sndbits");
	if (tmp != 0 && tmp < com_argc-1)
	{
		tmp = atoi(com_argv[tmp+1]);
		if ((tmp == 16) || (tmp == 8))
			desired_bits = tmp;
	}

	tmp = COM_CheckParm("-sndmono");
	if (tmp != 0)
		desired_channels = 1;

	sound_started = SNDDMA_Init();

	if (!sound_started)
	{
		if (snd_system != S_SYS_NULL)
			Con_Printf("Failed initializing sound\n");
	}
	else
	{
		Con_Printf("%s Audio initialized (%d bit, %s, %d Hz)\n",
				snd_drivers[snd_system].name,
				shm->samplebits,
				(shm->channels == 2) ? "stereo" : "mono",
				shm->speed);
	}
}


/*
================
S_Init
================
*/
void S_Init (void)
{
	if (snd_initialized)
	{
		Con_Printf("Sound is already initialized\n");
		return;
	}

// point to correct platform versions of driver functions
	S_InitDrivers();

	Cvar_RegisterVariable(&precache);
	Cvar_RegisterVariable(&bgmtype);

	if (safemode || COM_CheckParm("-nosound") || COM_CheckParm("-s"))
		return;

	Con_Printf("\nSound Initialization\n");

	Cmd_AddCommand("play", S_Play);
	Cmd_AddCommand("playvol", S_PlayVol);
	Cmd_AddCommand("stopsound", S_StopAllSoundsC);
	Cmd_AddCommand("soundlist", S_SoundList);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);
	Cmd_AddCommand("mute", S_ToggleMute);
	Cmd_AddCommand("volumeup", S_VolumeUp);
	Cmd_AddCommand("volumedown", S_VolumeDown);

	Cvar_RegisterVariable(&nosound);
	Cvar_RegisterVariable(&sfxvolume);
	Cvar_RegisterVariable(&sfx_mutedvol);
	Cvar_RegisterVariable(&loadas8bit);
	Cvar_RegisterVariable(&bgmvolume);
	Cvar_RegisterVariable(&bgm_mutedvol);
	Cvar_RegisterVariable(&ambient_level);
	Cvar_RegisterVariable(&ambient_fade);
	Cvar_RegisterVariable(&snd_noextraupdate);
	Cvar_RegisterVariable(&snd_show);
	Cvar_RegisterVariable(&_snd_mixahead);

	if (host_parms->memsize < 0x800000)
	{
		Cvar_SetValue ("loadas8bit", 1);
		Con_Printf ("loading all sounds as 8bit\n");
	}

	SND_InitScaletable ();

	known_sfx = (sfx_t *) Hunk_AllocName (MAX_SFX*sizeof(sfx_t), "sfx_t");
	num_sfx = 0;

	snd_initialized = true;

	S_Startup ();
	if (sound_started == 0)
		return;

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
void S_Shutdown (void)
{
	if (!sound_started)
		return;

	sound_started = 0;

	SNDDMA_Shutdown();
	shm = NULL;
}


// =======================================================================
// Load a sound
// =======================================================================

/*
==================
S_FindName

==================
*/
static sfx_t *S_FindName (const char *name)
{
	int		i;
	sfx_t	*sfx;

	if (!name)
		Sys_Error ("%s: NULL", __thisfunc__);

	if (strlen(name) >= MAX_QPATH)
		Sys_Error ("Sound name too long: %s", name);

// see if already loaded
	for (i = 0; i < num_sfx; i++)
	{
		if (!strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}
	}

	if (num_sfx == MAX_SFX)
		Sys_Error ("%s: out of sfx_t", __thisfunc__);

	sfx = &known_sfx[i];
	Q_strlcpy (sfx->name, name, MAX_QPATH);

	num_sfx++;

	return sfx;
}


/*
==================
S_TouchSound

==================
*/
void S_TouchSound (const char *name)
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
sfx_t *S_PrecacheSound (const char *name)
{
	sfx_t	*sfx;

	if (!sound_started || nosound.integer)
		return NULL;

	sfx = S_FindName (name);

// cache it in
	if (precache.integer)
		S_LoadSound (sfx);

	return sfx;
}


//=============================================================================

/*
=================
SND_PickChannel

picks a channel based on priorities, empty slots, number of channels
=================
*/
channel_t *SND_PickChannel (int entnum, int entchannel)
{
	int	ch_idx;
	int	first_to_die;
	int	life_left;

// Check for replacement sound, or find the best one to replace
	first_to_die = -1;
	life_left = 0x7fffffff;
	for (ch_idx = NUM_AMBIENTS; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ch_idx++)
	{
		if (entchannel != 0		// channel 0 never overrides
			&& snd_channels[ch_idx].entnum == entnum
			&& (snd_channels[ch_idx].entchannel == entchannel || entchannel == -1) )
		{	// always override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (snd_channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && snd_channels[ch_idx].sfx)
			continue;

		if (snd_channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = snd_channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
	}

	if (first_to_die == -1)
		return NULL;

	if (snd_channels[first_to_die].sfx)
		snd_channels[first_to_die].sfx = NULL;

	return &snd_channels[first_to_die];
}

/*
=================
SND_Spatialize

spatializes a channel
=================
*/
void SND_Spatialize (channel_t *ch)
{
	vec_t	dot;
	vec_t	dist;
	vec_t	lscale, rscale, scale;
	vec3_t	source_vec;

// anything coming from the view entity will always be full volume
	if (ch->entnum == cl.viewentity)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

// calculate stereo seperation and distance attenuation
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

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
	channel_t	*target_chan, *check;
	sfxcache_t	*sc;
	int		ch_idx;
	int		skip;
	qboolean	skip_dist_check = false;

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound.integer)
		return;

// pick a channel to play on
	target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;

	if (attenuation == 4)	// Looping sound- always play
	{
		skip_dist_check = true;
		attenuation = 1;	// was 3 - static
	}

// spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = (int) (fvol * 255);
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);

	if (!skip_dist_check)
	{
		if (!target_chan->leftvol && !target_chan->rightvol)
			return;		// not audible at all
	}

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
	check = &snd_channels[NUM_AMBIENTS];
	for (ch_idx = NUM_AMBIENTS; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ch_idx++, check++)
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

void S_StopSound (int entnum, int entchannel)
{
	int	i;

	for (i = NUM_AMBIENTS; i < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; i++)
	{
		if (snd_channels[i].entnum == entnum
			&& ((!entchannel) || snd_channels[i].entchannel == entchannel))	// 0 matches any
		{
			snd_channels[i].end = 0;
			snd_channels[i].sfx = NULL;
			if (entchannel)
				return;	//got a match, not looking for more.
		}
	}
}

void S_UpdateSoundPos (int entnum, int entchannel, vec3_t origin)
{
	int	i;

	for (i = NUM_AMBIENTS; i < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; i++)
	{
		if (snd_channels[i].entnum == entnum
			&& snd_channels[i].entchannel == entchannel)
		{
			VectorCopy(origin, snd_channels[i].origin);
			return;
		}
	}
}

void S_StopAllSounds (qboolean clear)
{
	int		i;

	if (!sound_started)
		return;

	total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (snd_channels[i].sfx)
			snd_channels[i].sfx = NULL;
	}

	memset(snd_channels, 0, MAX_CHANNELS * sizeof(channel_t));

	if (clear)
		S_ClearBuffer ();
}

static void S_StopAllSoundsC (void)
{
	S_StopAllSounds (true);
}

void S_ClearBuffer (void)
{
	int		clear;

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (!sound_started || !shm || (!shm->buffer && !pDSBuf))
#else
	if (!sound_started || !shm || !shm->buffer)
#endif
		return;

	if (shm->samplebits == 8)
		clear = 0x80;
	else
		clear = 0;

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (pDSBuf)
	{
		DWORD	dwSize;
		LPVOID	pData;
		int		reps;
		HRESULT	hresult;

		reps = 0;

		while ((hresult = IDirectSoundBuffer_Lock(pDSBuf, 0, gSndBufSize, (LPVOID *) &pData, &dwSize, NULL, NULL, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("%s: DS::Lock Sound Buffer Failed\n", __thisfunc__);
				S_Shutdown ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("%s: DS: couldn't restore buffer\n", __thisfunc__);
				S_Shutdown ();
				return;
			}
		}

		memset(pData, clear, shm->samples * shm->samplebits/8);

		IDirectSoundBuffer_Unlock(pDSBuf, pData, dwSize, NULL, 0);
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
		Con_Printf ("%s: MAX_CHANNELS reached\n", __thisfunc__);
	//	Con_Printf (" failed at (%.2f, %.2f, %.2f)\n",origin[0],origin[1],origin[2]);
		return;
	}

	ss = &snd_channels[total_channels];
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
	ss->master_vol = (int)vol;
	ss->dist_mult = (attenuation / 64) / sound_nominal_clip_dist;
	ss->end = paintedtime + sc->length;

	SND_Spatialize (ss);
}


//=============================================================================

/*
===================
S_UpdateAmbientSounds
===================
*/
static void S_UpdateAmbientSounds (void)
{
	mleaf_t		*l;
	int		vol, ambient_channel;
	channel_t	*chan;

// calc ambient sound levels
	if (!cl.worldmodel)
		return;

	l = Mod_PointInLeaf (listener_origin, cl.worldmodel);
	if (!l || !ambient_level.value)
	{
		for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
			snd_channels[ambient_channel].sfx = NULL;
		return;
	}

	for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
	{
		chan = &snd_channels[ambient_channel];
		chan->sfx = ambient_sfx[ambient_channel];

		vol = (int) (ambient_level.value * l->ambient_sound_level[ambient_channel]);
		if (vol < 8)
			vol = 0;

	// don't adjust volume too fast
		if (chan->master_vol < vol)
		{
			chan->master_vol += (int) (host_frametime * ambient_fade.value);
			if (chan->master_vol > vol)
				chan->master_vol = vol;
		}
		else if (chan->master_vol > vol)
		{
			chan->master_vol -= (int) (host_frametime * ambient_fade.value);
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
void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
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
	ch = snd_channels + NUM_AMBIENTS;
	for (i = NUM_AMBIENTS; i < total_channels; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		SND_Spatialize(ch);	// respatialize channel
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
			combine = snd_channels + MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
			for (j = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS; j < i; j++, combine++)
			{
				if (combine->sfx == ch->sfx)
					break;
			}

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
	if (snd_show.integer)
	{
		total = 0;
		ch = snd_channels;
		for (i = 0; i < total_channels; i++, ch++)
		{
			if (ch->sfx && (ch->leftvol || ch->rightvol) )
			{
			//	Con_Printf ("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total++;
			}
		}

		Con_Printf ("----(%i)----\n", total);
	}

// mix some sound
	S_Update_();
}

static void GetSoundtime (void)
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;
	int		fullsamples;

	fullsamples = shm->samples / shm->channels;

// it is possible to miscount buffers if it has wrapped twice between
// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();

	if (samplepos < oldsamplepos)
	{
		buffers++;	// buffer wrapped

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
	IN_Accumulate ();

	if (snd_noextraupdate.integer)
		return;		// don't pollute timings
	S_Update_();
}

static void S_Update_ (void)
{
	unsigned int	endtime;
	int		samps;

#if SDLSOUND_PAINTS_CHANNELS
	if (snd_system == S_SYS_SDL)
		return;
#endif

	if (!sound_started || (snd_blocked > 0))
		return;

// Updates DMA time
	GetSoundtime();

// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
	{
	//	Con_Printf ("%s : overflow\n", __thisfunc__);
		paintedtime = soundtime;
	}

// mix ahead of current position
	endtime = soundtime + (unsigned int)(_snd_mixahead.value * shm->speed);
	samps = shm->samples >> (shm->channels - 1);
	endtime = min(endtime, (unsigned int)(soundtime + samps));

#ifdef PLATFORM_WINDOWS
// if the buffer was lost or stopped, restore it and/or restart it
// FIXME: move this to its platform driver!
	if (pDSBuf)
	{
		DWORD	dwStatus;

		if (IDirectSoundBuffer_GetStatus(pDSBuf, &dwStatus) != DD_OK)
			Con_Printf ("Couldn't get sound buffer status\n");

		if (dwStatus & DSBSTATUS_BUFFERLOST)
			IDirectSoundBuffer_Restore(pDSBuf);

		if (!(dwStatus & DSBSTATUS_PLAYING))
			IDirectSoundBuffer_Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);
	}
#endif

	S_PaintChannels (endtime);

	SNDDMA_Submit ();
}

/*
===============================================================================

console functions

===============================================================================
*/

// S.A. volume procs
static void S_ToggleMute (void)
{
	if (sfx_mutedvol.value || bgm_mutedvol.value)
	{
		Cvar_SetValue("volume", sfx_mutedvol.value);
		Cvar_SetValue("bgmvolume", bgm_mutedvol.value);
		Cvar_SetValue("sfx_mutedvol", 0.0f);
		Cvar_SetValue("bgm_mutedvol", 0.0f);
		if (sfxvolume.value || bgmvolume.value)
			Con_Printf ("Unmuted\n");
	}
	else
	{
		Cvar_SetValue("sfx_mutedvol", sfxvolume.value);
		Cvar_SetValue("bgm_mutedvol", bgmvolume.value);
		Cvar_SetValue("volume", 0);
		Cvar_SetValue("bgmvolume", 0);
		if (sfx_mutedvol.value || bgm_mutedvol.value)
			Con_Printf ("Muted\n");
	}
}

static void S_VolumeDown (void)
{
	if (sfxvolume.value >= 0.1)
		Cvar_SetValue("volume",sfxvolume.value - 0.1);
	if (bgmvolume.value >= 0.1)
		Cvar_SetValue("bgmvolume",bgmvolume.value - 0.1);
	Con_Printf ("Volume is %3.1f\n", sfxvolume.value);
}

static void S_VolumeUp (void)
{
	if (sfxvolume.value <= 0.9)
		Cvar_SetValue("volume",sfxvolume.value + 0.1);
	if (bgmvolume.value <= 0.9)
		Cvar_SetValue("bgmvolume",bgmvolume.value + 0.1);
	Con_Printf ("Volume is %3.1f\n", sfxvolume.value);
}

static void S_Play (void)
{
	static int hash = 345;
	int		i;
	char	name[256];
	sfx_t	*sfx;

	i = 1;
	while (i < Cmd_Argc())
	{
		Q_strlcpy(name, Cmd_Argv(i), sizeof(name));
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			Q_strlcat(name, ".wav", sizeof(name));
		}

		sfx = S_PrecacheSound(name);
		S_StartSound(hash++, 0, sfx, listener_origin, 1.0, 1.0);
		i++;
	}
}

static void S_PlayVol (void)
{
	static int hash = 543;
	int		i;
	float	vol;
	char	name[256];
	sfx_t	*sfx;

	i = 1;
	while (i < Cmd_Argc())
	{
		Q_strlcpy(name, Cmd_Argv(i), sizeof(name));
		if (!strrchr(Cmd_Argv(i), '.'))
		{
			Q_strlcat(name, ".wav", sizeof(name));
		}

		sfx = S_PrecacheSound(name);
		vol = atof(Cmd_Argv(i + 1));
		S_StartSound(hash++, 0, sfx, listener_origin, vol, 1.0);
		i += 2;
	}
}

static void S_SoundList (void)
{
	int		i;
	sfx_t	*sfx;
	sfxcache_t	*sc;
	int		size, total;

	total = 0;
	for (sfx = known_sfx, i = 0; i < num_sfx; i++, sfx++)
	{
		sc = (sfxcache_t *) Cache_Check (&sfx->cache);
		if (!sc)
			continue;
		size = sc->length*sc->width*(sc->stereo + 1);
		total += size;
		if (sc->loopstart >= 0)
			Con_Printf ("L");
		else
			Con_Printf (" ");
		Con_Printf("(%2db) %6i : %s\n", sc->width*8, size, sfx->name);
	}
	Con_Printf ("Total resident: %i\n", total);
}


void S_LocalSound (const char *sound)
{
	sfx_t	*sfx;

	if (nosound.integer)
		return;
	if (!sound_started)
		return;

	sfx = S_PrecacheSound (sound);
	if (!sfx)
	{
		Con_Printf ("%s: can't cache %s\n", __thisfunc__, sound);
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

