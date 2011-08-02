/*
	h2config.h
	Compile time options for Hexen II: Hammer of Thyrion

	$Id$

	Read the explanations for each option and edit the
	relevant option accordingly.  Most probably, you will
	need to do a clean rebuild after editing.
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
   recommend keeping it as 1. Also see quakefs.h where DO_USERDIRS may
   be disabled on purpose for some platforms.
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
   map/scripting bugs. Therefore, running with the old version may or
   may not result in unexpected gameplay behavior. Remember that you
   must still use 1.11 or later (preferably Hammer of Thyrion provided)
   progs.dat files: this only enables the use of un-patched pak files.
   FYI, here are the maps that changed between 1.03 and 1.11 versions:
   demo2, village1, village2, village3, meso1, meso8, egypt6, rider2c,
   cath, tower, eidolon, ravdm1, ravdm3, ravdm5.  Here are the models
   that changed:  assassin.mdl, ball.mdl, bonelump.mdl, scrbpwng.mdl.
   Four sound files (spider: step1.wav, step2.wav and step3.wav,  and
   weapons: ric2.wav) changed, too.
   Default: disabled (0).
   ================================================================== */
#define	ENABLE_OLD_RETAIL		0


/* ====================================================================
   ENABLE_OLD_DEMO
   Value  :	0 or 1
   Affects:	filesystem init. other places.

   Allow running with the old version (28.8.1997, v0.42? 1.07?) of the
   Hexen II Demo: It was class-restricted (paladin and assassin only),
   as a result it lacked certain models. It didn't include the demo3
   level which the later 1.11 version of the demo had. Grep the source
   for ENABLE_OLD_DEMO and GAME_OLD_DEMO for more info. Even with those
   runtime checks, it still lacks certain models that our current,
   1.11 and later version, progs require to be precached. Therefore
   the old demo version doesn't run as it is.  Besides, it was given
   out even before the retail version was released, so it may contain
   map and scripting bugs.
   Default: disabled (0). Enabling NOT recommended.
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
/* When compiling HexenWorld, H2MP must not be defined !  =========== */
#ifdef	H2W
#undef	H2MP
#endif


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
   ========================  CODECS SETUP:  ===========================
   ================================================================== */

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

