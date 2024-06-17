/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* These headers are from sdl12-compat, and are intended to give just enough
functionality to let you build an SDL-1.2-based project without having the
real SDL-1.2 available to you. */

#ifndef _SDL_H
#define _SDL_H

#include  <SDL/SDL_main.h>
#include  <SDL/SDL_stdinc.h>
#include  <SDL/SDL_audio.h>
#include  <SDL/SDL_cdrom.h>
#include  <SDL/SDL_cpuinfo.h>
#include  <SDL/SDL_endian.h>
#include  <SDL/SDL_error.h>
#include  <SDL/SDL_events.h>
#include  <SDL/SDL_loadso.h>
#include  <SDL/SDL_mutex.h>
#include  <SDL/SDL_rwops.h>
#include  <SDL/SDL_thread.h>
#include  <SDL/SDL_timer.h>
#include  <SDL/SDL_video.h>
#include  <SDL/SDL_version.h>

#include  <SDL/begin_code.h>

#define SDL_INIT_TIMER 0x00000001
#define SDL_INIT_AUDIO 0x00000010
#define SDL_INIT_VIDEO 0x00000020
#define SDL_INIT_CDROM 0x00000100
#define SDL_INIT_JOYSTICK 0x00000200
#define SDL_INIT_NOPARACHUTE 0x00100000
#define SDL_INIT_EVENTTHREAD 0x01000000
#define SDL_INIT_EVERYTHING 0x0000FFFF

extern DECLSPEC int SDLCALL SDL_Init(Uint32 flags);
extern DECLSPEC int SDLCALL SDL_InitSubSystem(Uint32 flags);
extern DECLSPEC void SDLCALL SDL_QuitSubSystem(Uint32 flags);
extern DECLSPEC Uint32 SDLCALL SDL_WasInit(Uint32 flags);
extern DECLSPEC void SDLCALL SDL_Quit(void);

#include  <SDL/close_code.h>

#endif

