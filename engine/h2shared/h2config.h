/*
 * h2config.h -- Compile time options for Hexen II: Hammer of Thyrion.
 * $Id$
 *
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef	__HEXEN2_OPTIONS_H
#define	__HEXEN2_OPTIONS_H

/* ====================================================================
   ENDIAN_RUNTIME_DETECT
   Value  :	0 or 1
   Affects:	everywhere in all source using byte order functions.

   This option has to be edited in the file q_endian.h, NOT here !!!
   Default is off which is a little bit faster.
   ================================================================== */


/* =====================================================================
   PARANOID
   Value  :	not a value, but a define or undef
   Affects:	allows paranoid checks at many places: speed sapping
		error checking.  do NOT enable this unless you REALLY
		know what you are doing: many of those checks may not
		be actually necessary and the game would error out at
		any time!  besides, it really decreases the speed.
   =================================================================== */
#undef	PARANOID


/* =====================================================================
   DO_USERDIRS
   Value  :	0 or 1
   Affects:	file system, quakefs.c.

   Allows separating user directories on multi-user systems. We HIGHLY
   recommend keeping it as 1. Also see "sys.h" where DO_USERDIRS may be
   disabled on purpose for some platforms.
   =================================================================== */
#define	DO_USERDIRS			1


/* =====================================================================
   USE_PASSWORD_FILE
   Value  :	0 or 1
   Affects:	system initialization, sys_unix.c.

   Whether to use the password file to determine the path to the home
   directory. On any sane unix system we shall get the home directory
   based on the real uid. If this fails (not finding the user in the
   password file isn't normal) or if you disable this, we will get it
   by reading the HOME environment variable, only. Also see sys.h where
   USE_PASSWORD_FILE may be disabled on purpose for some platforms.
   =================================================================== */
#define	USE_PASSWORD_FILE		1


/* ====================================================================
   DISALLOW_DEMONESS_IN_OLD_GAME
   Value  :	0 or 1
   Affects:	menu.c (player class selections, hexen2 only).

   Change the define below to 0 if you want to allow the demoness
   class in the old mission through the menu system.
   ================================================================== */
#define	DISALLOW_DEMONESS_IN_OLD_GAME	1


/* ====================================================================
   ENABLE_OLD_RETAIL
   Value  :	0 or 1
   Affects:	filesystem init.

   Allow running with the old, pre-1.11 (such as 1.03 cdrom) versions
   of Hexen II.  The game actually seems to run fine with the original
   cdrom version, but Raven's later patches provided several fixes for
   map/scripting bugs.  Therefore, running with the old version may or
   may not result in unexpected gameplay behavior.  (Here are the maps
   changed between v1.03 and v1.11: demo2, village1, village2, village3,
   meso1, meso8, egypt6, rider2c, cath, tower, eidolon, ravdm1, ravdm3,
   ravdm5.  Models: assassin.mdl, ball.mdl, bonelump.mdl, scrbpwng.mdl.
   Sounds: spider/step1.wav, step2.wav, step3.wav,  weapons/ric2.wav.)
   Default: disabled (0).
   ================================================================== */
#define	ENABLE_OLD_RETAIL		0


/* ====================================================================
   ENABLE_OLD_DEMO
   Value  :	0 or 1
   Affects:	filesystem init. other places.

   Allow running with the old (original) version of the Hexen II Demo
   from 28.8.1997: It was class-restricted, paladin and assassin only,
   as a result it lacked certain models. It didn't include the demo3
   (The Mill) level which the later (Nov. 1997, v1.11) of the demo had,
   but it had the Mill area merged in the demo1 map instead.
   It lacks certain models that our current progs precache, therefore,
   it must be run using ITS OWN progs which requires uhexen2 v1.5.2 or
   newer.
   Default: disabled (0).
   ================================================================== */
#define	ENABLE_OLD_DEMO			0


/* ====================================================================
   H2MP
   Whether we want the mission pack support to be activated directly.
   Value  :	not a value, but a define or undef
   Affects:	quakefs.c (filesystem initialization)

   Default is no: player must use the -portals command line argument
   to activate it.  If you want direct activation, change the below
   undef to a define :  in that case, player must use the -noportals
   command line argument to disable mission pack support.
   ================================================================== */
#undef	H2MP
/* When building HexenWorld or demo-specific, H2MP mustn't be defined */
#if defined(H2W) || defined(DEMOBUILD)
#undef	H2MP
#endif		/* H2W || DEMOBUILD */


/* ====================================================================
   USE_AOT_FRICTION
   Value  :	0 or 1
   Affects:	sv_user.c (SV_UserFriction(), hexen2 only)

   Hexen II v1.11 progs does not initialize the friction properly:
   always 0.  In fact, applying a patch to the progs may solve this
   issue, but that would leave several unpatched mods as broken.
   Note: Compared to the AoT solution, the USE_AOT_FRICTION 0  option
   makes pure hexen2 to feel slightly more slippery.
   If you want to use the Anvil of Thyrion solution for the original
   hexen2 friction emulation, then define USE_AOT_FRICTION as 1,
   otherwise define it as 0.
   ================================================================== */
#define	USE_AOT_FRICTION		0


/* ====================================================================
   MGNET
   Value  :	not a value, but a define or undef
   Affects:	HexenWorld, server/sv_ents.c :: SV_WritePlayersToClient

   This doesn't ~seem~ to be in the latest binary release of Raven, but
   it is in the source release and the macro is defined in qwsv.dsp ie.
   the MSVC project file. It uses cardioid_rating() and might send an
   additional server message, svc_playerskipped.  The client calls
   CL_SavePlayer() upon receiving this message (see in cl_parse.c) but
   that code seems incomplete (see in cl_ents.c).  Enabling this option
   should require bumping the protocol version.  Disabled by default.
   ================================================================== */
#undef	MGNET


/* ====================================================================
   ========================  MEMORY SETUP:  ===========================
   ================================================================== */

/* ====================================================================
   The amount of memory needed by Watt-32 (WatTCP) for DOS:
   - Hexen II server opens a socket for each client, plus it needs its
     accept socket and broadcast socket, so (MAX_PLAYERS + 3) * 11K is
     normally the maximum needed memory.
   - HexenWorld opens one socket, but it sends two large data, i.e. the
     soundlist and the modellist, in a single chunk, therefore WatTCP
     needs to allocate some additional memory (~ 2*64K?) to reassemble
     the fragmented data.
   Affects:	sys_dos.c.
   ================================================================== */

#define	WATT32_NEEDMEM	0x30000	/* 192 K */

/* ====================================================================
   If CODECS_USE_ZONE is defined (see the Makefile), then mp3 (libmad)
   and ogg/vorbis codecs will allocate on the zone instead of system
   memory.  Remember that this requires recompiling the decoder library
   with proper memory allocator changes to it.  DOS builds are example
   for this case.  The memory requirements for individual decoders are
   defined below.
   Affects:	zone.c.
   ================================================================== */

#define	LIBMAD_NEEDMEM	0x10000	/* 64K is fairly enough for libmad */
#define	VORBIS_NEEDMEM	0x60000	/* 400K enough for vorbis most times */


/* ====================================================================
   WAL_TEXTURES
   Value  :	not a value, but a define or undef
   Affects:	model.c, gl_model.c (opengl & software renderers both.)

   Support for loading external wal mip textures instead of the ones
   embedded in the mdl. (cvar: r_texture_external.)
   ================================================================== */
#undef	WAL_TEXTURES


/* ====================================================================
   FULLSCREEN_INTERMISSIONS
   Value  :	0 or 1
   Affects:	screen.c, gl_screen.c, menu.c (hexen2 and hexenworld)

   If you want the intermissions and help screens to be drawn full-
   screen keep the define below as 1.  Otherwise, if you want them to
   be drawn unscaled with regard to the resolution, change the define
   below to 0
   ================================================================== */
#define	FULLSCREEN_INTERMISSIONS	1


/* ====================================================================
   =======================  OpenGL OPTIONS:  ==========================
   ================================================================== */

/* ====================================================================
   GL_DLSYM
   Value  :	not a value, but a define or undef
   Affects:	all gl sources and linkage.

   This option has to be edited in the Makefile (see the LINK_GL_LIBS
   option in there), not here.  This affects the final linkage of the
   binary.
   ================================================================== */


#endif	/* __HEXEN2_OPTIONS_H		*/

