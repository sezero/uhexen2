/*
	launcher_ui.h
	hexen2 launcher, global ui functions

	$Id: launcher_ui.h,v 1.6 2008-12-19 17:55:05 sezero Exp $

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

#ifndef	LAUNCHER_UI_H
#define	LAUNCHER_UI_H

int  ui_init (int *argc, char ***argv);
	/* initializes the gui loop.
	   call this from your main() */

int  ui_main (void);
	/* runs the gui loop.
	   call this from your main() */

void ui_error (const char *msg);
	/* displays an error message.
	   for errors when gui is inactive. */

void ui_quit (void);
	/* aborts the gui loop. */

void ui_pump (void);
	/* updates the gui. use it when doing
	   time consuming stuff (eg. patch) */

void ui_log (const char *fmt, ...) __attribute__((format(printf,1,2)));
	/* prints the given log content to a
	   text box on the user interface.  */

void ui_log_queue (const char *fmt, ...) __attribute__((format(printf,1,2)));
	/* queues the given log content for printing to a
	   text box on the user interface.  */

#endif	/* LAUNCHER_UI_H */

