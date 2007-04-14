/*
	games.h
	hexen2 launcher, game installation scanning

	$Id: games.h,v 1.1 2007-04-14 21:30:15 sezero Exp $

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

#define	H2_BINARY_NAME	"hexen2"
#define	HW_BINARY_NAME	"hwcl"

#define	GAME_DEMO		(1 << 0)
#define	GAME_OEM		(1 << 1)
#define	GAME_MODIFIED		(1 << 2)
#define	GAME_REGISTERED		(1 << 3)
#define	GAME_REGISTERED0	(1 << 4)
#define	GAME_REGISTERED1	(1 << 5)
#define	GAME_PORTALS		(1 << 6)
#define	GAME_HEXENWORLD		(1 << 7)

#define	GAME_OLD_CDROM0		(1 << 8)
#define	GAME_OLD_CDROM1		(1 << 9)
#define	GAME_OLD_DEMO		(1 << 10)

#define	GAME_INSTBAD		(1 << 13)	/* bad data. reason flags below. */
#define	GAME_INSTBAD0		(1 << 14)	/* un-patched data */
#define	GAME_INSTBAD1		(1 << 15)	/* no valid data. */
#define	GAME_INSTBAD2		(1 << 16)	/* mix'n'match data */

// see scan_binaries() before playing with these
#define	HAVE_H2_BIN		(1 << 17)
#define	HAVE_HW_BIN		(1 << 18)
#define	HAVE_GLH2_BIN		(1 << 19)
#define	HAVE_GLHW_BIN		(1 << 20)

extern	unsigned int		gameflags;

#if !defined(DEMOBUILD)
typedef struct
{
	char	*dirname;
	char	*name;
	int	is_botmatch;
	int	available;
} h2game_t;

typedef struct
{
	char	*dirname;
	char	*name;		// user-friendly name
	char	*checkfile;	// filename for extra check
	int	available;
} hwgame_t;

extern	h2game_t	h2game_names[];
extern	hwgame_t	hwgame_names[];

extern	const int	MAX_H2GAMES;
extern	const int	MAX_HWGAMES;
#endif	/* DEMOBUILD */

void scan_game_installation (void);


#endif	/* LAUNCHER_GAMES_H */

