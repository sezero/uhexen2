/*
	sdl_inc.h
	common SDL header

	$Id: sdl_inc.h,v 1.7 2010-01-27 16:33:59 sezero Exp $
*/

#ifndef __HX2_SDL_INC
#define __HX2_SDL_INC

#if !defined(SDLQUAKE)
#error "SDLQUAKE must be defined in order to use sdl_inc.h"
#endif	/* SDLQUAKE */

#include "SDL.h"

/* =================================================================
Minimum required SDL versions:
Although the 1.1.x series might work fine, we require at least 1.2.0
If we want midi support through SDL_mixer, we want SDL_mixer>= 1.2.4
which, in turn, requires SDL >= 1.2.4. We could have required v1.2.1
of SDL_Mixer, but 1.2.4 have several fixes in it.
=================================================================== */

#define SDL_MIXER_MIN_X	1
#define SDL_MIXER_MIN_Y	2
#define SDL_MIXER_MIN_Z	4
#define MIX_REQUIREDVERSION	(SDL_VERSIONNUM(SDL_MIXER_MIN_X,SDL_MIXER_MIN_Y,SDL_MIXER_MIN_Z))

#define SDL_MIN_X	1
/* =================================================================
if we are compiling against SDL-1.3.x, then require version 1.3.0 or
newer at runtime, too.

2010-01-27:
SDL-1.3 is still in active development and its API hasn't solidified
yet.  As of this writing, SDL-1.2 and 1.3 has some api differences:
see, for example, SDL_GetMouseState() which is not binary compatible
between SDL-1.2 / 1.3: http://forums.libsdl.org/viewtopic.php?t=5458
For now, I am limiting our own requirement to the compiled version,
hence the SDL_NEW_VERSION_REJECT macro below.
=================================================================== */
#if SDL_VERSION_ATLEAST(1,3,0)
#define SDL_MIN_Y	3
#else
#define SDL_MIN_Y	2
#endif

#if SDL_MIN_Y >= 3
#   define SDL_MIN_Z	0
#else	/* SDL-1.2.x: */
# define SDL_NEW_VERSION_REJECT SDL_VERSION_ATLEAST(1,3,0) /* reject 1.3.0 and newer at runtime. */
# if defined(__MACOSX__) || defined(__APPLE__)
#   define SDL_MIN_Z	8
# elif defined(_MIDI_SDLMIXER)
#   if MIX_REQUIREDVERSION > (SDL_VERSIONNUM(1,2,6))
#     define SDL_MIN_Z	10
#   elif MIX_REQUIREDVERSION > (SDL_VERSIONNUM(1,2,1))
#     define SDL_MIN_Z	4
#   else
#     define SDL_MIN_Z	0
#   endif
# else
#   define SDL_MIN_Z	0
# endif
#endif

#define SDL_REQUIREDVERSION	(SDL_VERSIONNUM(SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z))

#if !(SDL_VERSION_ATLEAST(SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z))
#error SDL version found is too old
#endif

#if defined(_MIDI_SDLMIXER)
#include "SDL_mixer.h"
/* starting with 1.2.1, SDL_mixer provides MIX_xxx version
   macros. the new SDL_MIXER_xxx version macros start with
   1.2.6 but they keep backwards compatibility. 1.2.0 does
   not even have any version macros, so let's reject it */
#if !defined(MIX_MAJOR_VERSION) || !defined(MIX_MINOR_VERSION) || !defined(MIX_PATCHLEVEL)
#error SDL_mixer version found is too old
#endif
#ifndef MIX_COMPILEDVERSION
#define MIX_COMPILEDVERSION	(SDL_VERSIONNUM(MIX_MAJOR_VERSION,MIX_MINOR_VERSION,MIX_PATCHLEVEL))
#endif
#if MIX_COMPILEDVERSION < MIX_REQUIREDVERSION
#error SDL_mixer version found is too old
#endif	/* end of bad version error */
#endif	/* end of SDL_mixer checks */


/* the defines below are actually part of SDL_GLattr enums in SDL
   versions supporting that relevant feature. */

/* gl stereo attribute was added to SDL beginning from v1.2.5 */
#define SDL_VER_WITH_STEREO		(SDL_VERSIONNUM(1,2,5))
#if SDL_COMPILEDVERSION < SDL_VER_WITH_STEREO
#define SDL_GL_STEREO			(SDL_GL_ACCUM_ALPHA_SIZE+1)
#endif	/* SDL_VER_WITH_STEREO */

/* multisampling was added to SDL beginning from v1.2.6 */
#define SDL_VER_WITH_MULTISAMPLING	(SDL_VERSIONNUM(1,2,6))
#if SDL_COMPILEDVERSION < SDL_VER_WITH_MULTISAMPLING
#define SDL_GL_MULTISAMPLEBUFFERS	(SDL_GL_ACCUM_ALPHA_SIZE+2)
#define SDL_GL_MULTISAMPLESAMPLES	(SDL_GL_ACCUM_ALPHA_SIZE+3)
#endif	/* SDL_VER_WITH_MULTISAMPLING */

/* swapinterval was added to SDL beginning from v1.2.10 */
#define SDL_VER_WITH_SWAPINTERVAL	(SDL_VERSIONNUM(1,2,10))
#if SDL_COMPILEDVERSION < SDL_VER_WITH_SWAPINTERVAL
#define SDL_GL_ACCELERATED_VISUAL	(SDL_GL_MULTISAMPLESAMPLES+1)
#define SDL_GL_SWAP_CONTROL		(SDL_GL_MULTISAMPLESAMPLES+2)
#endif	/* SDL_VER_WITH_SWAPINTERVAL */

#endif	/* __HX2_SDL_INC */

