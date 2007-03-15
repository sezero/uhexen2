/*
	compat_gtk1.h
	"gtk-1.2 doesnt have it" wrappers...

	$Id: compat_gtk1.h,v 1.5 2007-03-15 18:18:15 sezero Exp $

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

#ifndef	LAUNCHER_GTK1COMPAT_H
#define	LAUNCHER_GTK1COMPAT_H

#ifdef WITH_GTK1

#define GTK_WIN_POS_CENTER_ON_PARENT	GTK_WIN_POS_MOUSE
#define gtk_widget_set_size_request gtk_widget_set_usize
#define gtk_window_set_resizable(x, y) gtk_window_set_policy((x), (y), (y), (y))
#define gtk_button_set_label(x, y) gtk_label_set_text (GTK_LABEL(GTK_BIN((x))->child), (y))

#endif	// WITH_GTK1

#endif	// LAUNCHER_GTK1COMPAT_H

