/*
	h2option.h
	Compile time options for Hexen II: Hammer of Thyrion

	$Id: h2option.h,v 1.1 2007-03-14 17:45:29 sezero Exp $
*/


#ifndef	__HEXEN2_OPTIONS_H
#define	__HEXEN2_OPTIONS_H

/* ====================================================================
   Read the explanations for each option and edit the relevant option
   accordingly. Most probably, you will need to perform a clean build
   after editing.
   ================================================================== */


/* =====================================================================
   PARANOID
   Value  :	not a value, but a define or undef
   Affects:	allows paranoid checks at many places (speed sapping
		error checking).
   =================================================================== */
#undef	PARANOID


/* ====================================================================
   DISALLOW_DEMONESS_IN_OLD_GAME
   Value  :	0 or 1
   Affects:	menu.c (player class selections, hexen2 only).

   Change the define below to 0 if you want to allow the demoness
   class in the old mission through the menu system.
   ================================================================== */
#define	DISALLOW_DEMONESS_IN_OLD_GAME	1


/* ====================================================================
   NET_USE_VCR
   Value  :	0 or 1
   Affects:	host.c, net_main.c, net_vcr.c (hexen2 only)

   VCR facility as a lan driver:
   The "-record" commandline argument creates a file named quake.vcr
   and records the game. This is NOT an ordinary demo recording, but
   a server only feature for debugging server bugs.
   If you want to use the vcr feature regardless of building a debug
   version, change the below definition to 1.  For debug builds, it
   will be set to 1 down below.
   ================================================================== */
#define	NET_USE_VCR			0
/* Allow net_vcr in the debug builds.	============================= */
#ifdef	DEBUG_BUILD
#undef	NET_USE_VCR
#define	NET_USE_VCR			1
#endif


/* ====================================================================
   H2MP
   Whether we want the mission pack support to be activated directly.
   Value  :	not a value, but a define or undef
   Affects:	common.c (filesystem initialization)

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
   FULLSCREEN_INTERMISSIONS
   Value  :	0 or 1
   Affects:	screen.c, gl_screen.c, menu.c (hexen2 and hexenworld)

   If you want the intermissions and help screens to be drawn full-
   screen keep the define below as 1. Otherwise, if you want them to
   be drawn unscaled with regard to the resolution, change the define
   below to 0
   ================================================================== */
#define	FULLSCREEN_INTERMISSIONS	1


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

   OpenGL OPTIONS:

   ================================================================== */


/* ====================================================================
   DO_MESH_CACHE
   Value  :	0 or 1
   Affects:	gl_mesh.c

   If you want to cache the alias model meshes to disk, the defini-
   tion below must be 1, otherwise 0. Caching gives bad results upon
   the following conditions:
   - you change the mdl file with an override file, but you have a
     cached ms2 file in the glhexen directory from the old mdl,
   - you run a mod without the mission pack first and with mission
     pack later, or vice versa, and the ms2 cache from the previous
     instance would mismatch the mdl from the latter.
   ================================================================== */
#define	DO_MESH_CACHE			0


/* ====================================================================
   USE_HEXEN2_PALTEX_CODE
   Value  :	0 or 1
   Affects:	gl_draw.c, gl_vidsdl.c, gl_vidnt.c, win32res.rc

   To use hexenworld (quake)'s palettized texture code instead of the
   original hexen2 code, change the definition below to 0.  That will
   result in lower quality. see gl_draw.c (and gl_vidXXX.c) for more
   details.
   NOTE: If you set this to 0 and you are compiling for windows with
   the splash screens enabled, make sure to enable the comctl32 link
   flag in the Makefile.
   ================================================================== */
#define	USE_HEXEN2_PALTEX_CODE		1


/* ====================================================================
   USE_HEXEN2_RESAMPLER_CODE
   Value  :	0 or 1
   Affects:	gl_draw.c

   To use hexenworld (quake)'s texture resampler code instead of the
   original hexen2 code, change the definition below to 0. That will
   result in slightly sharper but "jaggier" textures here and there.
   See in gl_draw.c. bad results especially detectable on help screens
   when using old cards with small (256k) texture size support (read:
   old voodoo boards.)
   ================================================================== */
#define	USE_HEXEN2_RESAMPLER_CODE	1


/* ====================================================================
   GL_DLSYM
   Value  :	not a value, but a define or undef
   Affects:	all gl sources and linkage.

   This option has to be edited in the Makefile (see the LINK_GL_LIBS
   option in there), not here.  This affects the final linkage of the
   binary.
   ================================================================== */


/* ====================================================================
   GLTEST
   Value  :	not a value, but a define or undef
   Affects:	gl_test.c, gl_rmain.c, gl_rmisc.c, cl_tent.c

   The macro GLTEST enables some experimentation stuff for developers.
   If you want to enable it, change the below undef to a define. This
   option isn't for users.
   ================================================================== */
#undef	GLTEST


#endif	/* __HEXEN2_OPTIONS_H		*/

