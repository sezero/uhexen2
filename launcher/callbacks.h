/*
	callbacks.h
	hexen2 launcher callbacks

	$Id: callbacks.h,v 1.22 2007-03-15 18:18:15 sezero Exp $

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

#ifndef	LAUNCHER_CALLBACKS_H
#define	LAUNCHER_CALLBACKS_H

void Make_ResMenu (struct Video_s *wgt);
void Make_ConwMenu (struct Video_s *wgt);
void res_Change (GtkEditable *editable, struct Video_s *wgt);
void con_Change (GtkEditable *editable, gpointer user_data);
void libgl_Change (GtkEditable *editable, gpointer user_data);
void adj_Change (GtkAdjustment *adj, int *opt);
void UpdateStats (struct Launch_s *wgt);
void on_HEXEN2 (GtkButton *button, gamewidget_t *wgt);
void on_H2W (GtkButton *button, gamewidget_t *wgt);
#ifndef DEMOBUILD
void start_xpatch (GtkObject *Unused, PatchWindow_t *PatchWindow);
void H2GameScan (GList *GameList);
void HWGameScan (GList *GameList);
void H2GameChange (GtkEditable *editable, gamewidget_t *wgt);
void HWGameChange (GtkEditable *editable, gpointer user_data);
#endif	/* DEMOBUILD */
void on_SND (GtkEditable *editable, sndwidget_t *wgt);
void on_SRATE (GtkEditable *editable, gpointer user_data);
void on_SBITS (GtkButton *button, int *opt);
void on_OGL (GtkToggleButton *button, gamewidget_t *wgt);
void ReverseOpt (GtkObject *Unused, int *opt);
void on_MORE (GtkButton *button, HoTWindow_t *window);

#endif	// LAUNCHER_CALLBACKS_H

