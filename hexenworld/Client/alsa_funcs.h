/*
	alsa_funcs.h
	$Id: alsa_funcs.h,v 1.9 2007-06-30 11:19:47 sezero Exp $

	ALSA function list
	make sure NOT to protect this file against multiple inclusions!

	Copyright (C) 2001 Bill Currie <bill@taniwha.org>

	Author: Bill Currie
	Date: 2002/4/19

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA
*/

// We require alsa-lib 0.9.8 or newer to function. Refuse otherwise.
#if SND_LIB_MAJOR < 1 && (SND_LIB_MINOR < 9 || (SND_LIB_MINOR == 9 && SND_LIB_SUBMINOR < 8))
#error Alsa libraries v0.9.8 or newer are required
#endif

#ifndef HX2_ALSA
#define HX2_ALSA(ret, func, params)
#define UNDEF_HX2_ALSA
#endif

HX2_ALSA (int, snd_pcm_close, (snd_pcm_t *pcmdev))
HX2_ALSA (int, snd_pcm_hw_params, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params))
HX2_ALSA (int, snd_pcm_hw_params_any, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params))
HX2_ALSA (int, snd_pcm_hw_params_get_buffer_size, (const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val))
HX2_ALSA (int, snd_pcm_hw_params_get_period_size, (const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *frames, int *dir))
HX2_ALSA (int, snd_pcm_hw_params_set_access, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params, snd_pcm_access_t acc))
HX2_ALSA (int, snd_pcm_hw_params_set_period_size_near, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir))
HX2_ALSA (int, snd_pcm_hw_params_set_rate_near, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params, unsigned int *val, int *dir))
HX2_ALSA (int, snd_pcm_hw_params_set_channels, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params, unsigned int val))
HX2_ALSA (int, snd_pcm_hw_params_set_format, (snd_pcm_t *pcmdev, snd_pcm_hw_params_t *params, snd_pcm_format_t val))
HX2_ALSA (int, snd_pcm_mmap_begin, (snd_pcm_t *pcmdev, const snd_pcm_channel_area_t **areas, snd_pcm_uframes_t *offset, snd_pcm_uframes_t *frames))
HX2_ALSA (int, snd_pcm_avail_update, (snd_pcm_t *pcmdev))
HX2_ALSA (snd_pcm_sframes_t, snd_pcm_mmap_commit, (snd_pcm_t *pcmdev, snd_pcm_uframes_t offset, snd_pcm_uframes_t frames))
HX2_ALSA (int, snd_pcm_open, (snd_pcm_t **pcmdev, const char *name, snd_pcm_stream_t stream, int mode))
HX2_ALSA (int, snd_pcm_start, (snd_pcm_t *pcmdev))
HX2_ALSA (snd_pcm_state_t, snd_pcm_state, (snd_pcm_t *pcmdev))
HX2_ALSA (int, snd_pcm_sw_params, (snd_pcm_t *pcmdev, snd_pcm_sw_params_t *params))
HX2_ALSA (int, snd_pcm_sw_params_current, (snd_pcm_t *pcmdev, snd_pcm_sw_params_t *params))
HX2_ALSA (int, snd_pcm_sw_params_set_start_threshold, (snd_pcm_t *pcmdev, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val))
HX2_ALSA (int, snd_pcm_sw_params_set_stop_threshold, (snd_pcm_t *pcmdev, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val))
HX2_ALSA (const char *, snd_strerror, (int errnum))
HX2_ALSA (int, snd_pcm_drop, (snd_pcm_t *pcmdev))
HX2_ALSA (int, snd_pcm_hw_params_malloc, (snd_pcm_hw_params_t **ptr))
HX2_ALSA (int, snd_pcm_sw_params_malloc, (snd_pcm_sw_params_t **ptr))
HX2_ALSA (void, snd_pcm_hw_params_free, (snd_pcm_hw_params_t *obj))
HX2_ALSA (void, snd_pcm_sw_params_free, (snd_pcm_sw_params_t *obj))
/*
HX2_ALSA (int, snd_pcm_pause, (snd_pcm_t *pcmdev, int enable))
*/

#ifdef UNDEF_HX2_ALSA
#undef HX2_ALSA
#undef UNDEF_HX2_ALSA
#endif

