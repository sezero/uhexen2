/*
	games.h
	hexen2 launcher, game installation scanning

	$Id$

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
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef LAUNCHER_GAMES_H
#define LAUNCHER_GAMES_H

/* either keep binary names <= 16 chars including
   the gl prefix and the nul terminator, or change
   the definition below accordingly.		*/
#define BINNAME_CHARS	(16)
#define	BIN_OGL_PREFIX	"gl"
#define	H2_BINARY_NAME	"hexen2"
#define	HW_BINARY_NAME	"hwcl"

#define	GAME_DATADIR	"/usr/share/games/hexen2"	/* Gentoo guys install this way. */

#define	GAME_DEMO		(1 << 0)
#define	GAME_OEM		(1 << 1)
#define	GAME_OEM0		(1 << 2)
#define	GAME_OEM2		(1 << 3)
#define	GAME_REGISTERED		(1 << 4)
#define	GAME_REGISTERED0	(1 << 5)
#define	GAME_REGISTERED1	(1 << 6)
#define	GAME_PORTALS		(1 << 7)
#define	GAME_HEXENWORLD		(1 << 8)

/* FIXME: more detailed data needed for the oem
 * (Matrox m3D bundle) original 1.08 version.  */
#define	GAME_OLD_CDROM0		(1 << 9)
#define	GAME_OLD_CDROM1		(1 << 10)
#define	GAME_OLD_DEMO		(1 << 11)
#define	GAME_REGISTERED_OLD	(1 << 12)
#define	GAME_OLD_OEM		(1 << 13)
#define	GAME_OLD_OEM0		(1 << 14)
#define	GAME_OLD_OEM2		(1 << 15)

#define	GAME_MODIFIED		(1 << 16)

#define	GAME_CANPATCH0		(1 << 17)
#define	GAME_CANPATCH1		(1 << 18)
#define	GAME_CANPATCH		(1 << 19)
#define	GAME_INSTBAD		(1 << 20)	/* bad data. reason flags below. */
#define	GAME_INSTBAD0		(1 << 21)	/* un-patched data */
#define	GAME_INSTBAD1		(1 << 22)	/* no valid data. */
#define	GAME_INSTBAD2		(1 << 23)	/* mix'n'match data */
#define	GAME_INSTBAD3		(1 << 24)	/* other unsupported data */

/* binary availability flags: see scan_binaries()
   and CheckStats() before playing with these.	*/
#define	HAVE_H2_BIN		(1 << 25)
#define	HAVE_HW_BIN		(1 << 26)
#define	HAVE_GLH2_BIN		(1 << 27)
#define	HAVE_GLHW_BIN		(1 << 28)

extern	unsigned int		gameflags;

#if !defined(DEMOBUILD)
typedef struct
{
	const char	*dirname;
	const char	*name;		/* user-friendly name */
	const char	*checkfile;	/* filename for extra check */
	int	is_botmatch;
	int	available;
	int	menu_index;
} h2game_t;

extern	h2game_t	h2game_names[];
extern	h2game_t	hwgame_names[];

extern	const int	MAX_H2GAMES;
extern	const int	MAX_HWGAMES;
#endif	/* DEMOBUILD */

/* ====================================================================
   ENABLE_OLD_RETAIL:	0 or 1
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
   ENABLE_OLD_DEMO:	0 or 1
   Allow running with the old version (28.8.1997, v0.42? 1.07?) of the
   Hexen II Demo: It was class-restricted (paladin and assassin only),
   as a result it lacked certain models. It didn't include the demo3
   level which the later 1.11 version of the demo had. Grep the source
   for ENABLE_OLD_DEMO and GAME_OLD_DEMO for more info. Even with those
   runtime checks, it still lacks certain models that our current,
   1.11 and later version, progs require to be precached. Therefore, it
   it must be run using ITS OWN progs which is v1.03, requiring uhexen2
   version 1.5.2.
   Default: disabled (0).
   ================================================================== */
#define	ENABLE_OLD_DEMO			0

void scan_game_installation (void);


#endif	/* LAUNCHER_GAMES_H */

