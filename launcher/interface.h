/*
	interface.h
	hexen2 launcher gtk+ interface

	$Id: interface.h,v 1.12 2007-08-09 06:08:23 sezero Exp $

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

#ifndef	LAUNCHER_INTERFACE_H
#define	LAUNCHER_INTERFACE_H

int  ui_main (int *argc, char ***argv);
void ui_pump (void);
void ui_quit (void);

#endif	/* LAUNCHER_INTERFACE_H */

