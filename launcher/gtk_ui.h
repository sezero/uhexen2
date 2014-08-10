/*
 * gtk_ui.h
 * gtk ui, main header
 *
 * $Id$
 *
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
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

#ifndef	LAUNCHER_GTK_H
#define	LAUNCHER_GTK_H

#include <gtk/gtk.h>


/*
 =============================
	VERSION DEFS
 =============================
*/

#if (GTK_MAJOR_VERSION < 2)
#if !GTK_CHECK_VERSION(1,2,0)
#error You need at least 1.2.0 version of GTK+
#endif
#endif

#if (GTK_MAJOR_VERSION < 2)		/* "gtk-1.2 doesnt have it" wrappers */

#define	GTK_WIN_POS_CENTER_ON_PARENT	GTK_WIN_POS_NONE
#define	gtk_statusbar_set_has_resize_grip(x, y)
#define	gtk_scrolled_window_set_shadow_type(x, y)
#define	gtk_widget_set_size_request	gtk_widget_set_usize
#define	gtk_window_set_resizable(x, y)	gtk_window_set_policy((x), (y), (y), (y))
#define	gtk_button_set_label(x, y)	gtk_label_set_text (GTK_LABEL(GTK_BIN((x))->child), (y))
#define	gtk_progress_bar_set_fraction	gtk_progress_bar_update
#define	gtk_adjustment_get_value(a)	(a)->value
#define	gtk_radio_button_get_group	gtk_radio_button_group
#define	gtk_option_menu_get_history(m)	g_list_index(GTK_MENU_SHELL(GTK_MENU((m)->menu))->children,	\
								gtk_menu_get_active(GTK_MENU((m)->menu)))
#endif	/* GTK1 */

#if (GLIB_MAJOR_VERSION < 2)
#define g_usleep			usleep
#endif	/* glib-1.2.x */

/* further compatibility macros */
#define	GTKUI_DISABLE_FOCUS(widget)	GTK_WIDGET_UNSET_FLAGS((widget),GTK_CAN_FOCUS)

#if (GTK_MAJOR_VERSION < 2)	/* gtk-1.2 */
#define	GTKUI_SIGNALFUNC_T	GtkSignalFunc
#define	GTKUI_SIGNALFUNC(f)	GTK_SIGNAL_FUNC(f)
#define	GTK_SIGHANDLER_T	guint
#define	GTKUI_SIGNAL_CONNECT(object, name, func, func_data)		\
	gtk_signal_connect(GTK_OBJECT((object)),(name),GTK_SIGNAL_FUNC((func)),(func_data))
#define	GTKUI_SIGNAL_CONNECT_OBJECT(object, name, func, slot_object)	\
	gtk_signal_connect_object(GTK_OBJECT((object)),(name),GTK_SIGNAL_FUNC((func)),GTK_OBJECT((slot_object)))
#define	GTKUI_SIGNAL_DISCONNECT(object, handler)			\
	gtk_signal_disconnect(GTK_OBJECT((object)), (handler))
#define	GTKUI_SIGNAL_HANDLER_BLOCK(object, handler)			\
	gtk_signal_handler_block (GTK_OBJECT((object)), (handler))
#define	GTKUI_SIGNAL_HANDLER_UNBLOCK(object, handler)			\
	gtk_signal_handler_unblock (GTK_OBJECT((object)), (handler))
#else	/* gtk-2 / gtk-3 */
#define	GTKUI_SIGNALFUNC_T	GCallback
#define	GTKUI_SIGNALFUNC(f)	G_CALLBACK(f)
#define	GTK_SIGHANDLER_T	gulong
#define	GTKUI_SIGNAL_CONNECT(object, name, func, func_data)		\
	g_signal_connect ((gpointer)(object),(name),G_CALLBACK((func)),(gpointer)(func_data))
#define	GTKUI_SIGNAL_CONNECT_OBJECT(object, name, func, slot_object)	\
	g_signal_connect_object((gpointer)(object)),(name),G_CALLBACK((func)),(gpointer)(slot_object),(GConnectFlags)0)
#define	GTKUI_SIGNAL_DISCONNECT(object, handler)			\
	g_signal_handler_disconnect((gpointer)(object), (handler))
#define	GTKUI_SIGNAL_HANDLER_BLOCK(object, handler)			\
	g_signal_handler_block ((gpointer)(object), (handler))
#define	GTKUI_SIGNAL_HANDLER_UNBLOCK(object, handler)			\
	g_signal_handler_unblock ((gpointer)(object), (handler))
#endif	/* GTK_MAJOR_VERSION */

#if (GTK_MAJOR_VERSION > 2)
#undef	gtk_tooltips_set_tip
#define	gtk_tooltips_set_tip(t, widget, tiptext, private)		\
	gtk_widget_set_tooltip_text ((widget), (tiptext))

static inline GtkWidget* MY_gtk_hbox_new (gboolean homogeneous, gint spacing) {
	GtkWidget* hbx = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);
	if (homogeneous) gtk_box_set_homogeneous (GTK_BOX(hbx), TRUE);
	return hbx;
}
#undef	gtk_hbox_new
#define	gtk_hbox_new MY_gtk_hbox_new

static inline GtkWidget* MY_gtk_vbox_new (gboolean homogeneous, gint spacing) {
	GtkWidget* vbx = gtk_box_new (GTK_ORIENTATION_VERTICAL, spacing);
	if (homogeneous) gtk_box_set_homogeneous (GTK_BOX(vbx), TRUE);
	return vbx;
}
#undef	gtk_vbox_new
#define	gtk_vbox_new MY_gtk_vbox_new

#undef	gtk_hseparator_new
#define	gtk_hseparator_new() gtk_separator_new(GTK_ORIENTATION_HORIZONTAL)

#undef	gtk_vseparator_new
#define	gtk_vseparator_new() gtk_separator_new(GTK_ORIENTATION_VERTICAL)

#undef	GTKUI_DISABLE_FOCUS
#define	GTKUI_DISABLE_FOCUS(widget)	gtk_widget_set_can_focus((widget),FALSE)

#define	gtk_statusbar_set_has_resize_grip(x, y)
#endif	/*  */

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String)				dgettext(PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String)				gettext_noop(String)
#  else
#    define N_(String)				(String)
#  endif
#else
#  define textdomain(String)			(String)
#  define gettext(String)			(String)
#  define dgettext(Domain,Message)		(Message)
#  define dcgettext(Domain,Message,Type)	(Message)
#  define bindtextdomain(Domain,Directory)	(Domain)
#  define _(String)				(String)
#  define N_(String)				(String)
#endif

/*
 =============================
	WIDGET DEFS
 =============================
*/

typedef struct
{
	GtkWidget *mywindow;	/* Main window */
	GtkWidget *main_box;	/* Two pane horizontal box */
	GtkWidget *notebook0;	/* Basic options pane */
	GtkWidget *fixed0;	/* Basic options container */
/*	GtkWidget *lbl_tab0;*/	/* Basic options label */
	GtkWidget *notebook1;	/* Additional options pane */
	GtkWidget *fixed1;	/* Additional options Tab-1 */
	GtkWidget *lbl_tab1;	/* Additional options Tab-1 label */
	GtkWidget *fixed2;	/* Additional options Tab-2 */
	GtkWidget *lbl_tab2;	/* Additional options Tab-2 label */
	GtkWidget *bMORE;	/* More/Less button */
} MainWindow_t;

typedef struct
{
	GtkWidget *mywindow;	/* Main window */
	GtkWidget *fixed1;	/* Widgets container */
	GtkWidget *bBASEDIR;	/* Use a different game basedir */
	GtkWidget *bCLOSE;	/* Close button */
	GtkWidget *bAPPLY;	/* Apply patch button */
	GtkWidget *bREPORT;	/* Report installation status */
	GtkWidget *LOGVIEW;	/* LogEntry line for patch process */
	GtkWidget *dir_Entry;	/* path for game basedir */
	GtkWidget *progbar;	/* patch progress bar */
	GtkWidget *palign;	/* holder for progbar, GtkAlignment */
	GtkWidget *StatusBar;	/* Status bar, (patch status) */
	gint	statbar_id;	/* statbar context id */
} PatchWindow_t;

struct Launch_s
{
	GtkWidget *StatusBar;	/* Status bar, (launch status) */
	gint	statbar_id;	/* statbar context id */
	GtkWidget *bLAUNCH;	/* Launch button */
};

struct Video_s
{
	GtkWidget *bOGL;	/* OpenGL / Software */
	GtkWidget *mRES;	/* Resolution menu */
	GtkWidget *bCONW;	/* Allow conwidth toggle */
	GtkWidget *mCONW;	/* Conwidth menu */
	GtkWidget *bFULS;	/* Fullscreen / windowed */
	GtkWidget *b3DFX;	/* 3dfx specific Gamma */
	GtkWidget *b8BIT;	/* 8-bit texture extensions */
	GtkWidget *bFSAA;	/* Multisampling check button */
	GtkAdjustment *adjFSAA;	/* Multisampling adjustment */
	GtkWidget *spnFSAA;	/* Multisampling entry */
	GtkWidget *bVSYNC;	/* Enable vertical sync */
	GtkWidget *bLM_BYTES;	/* Whether to use GL_LUMINANCE lightmaps */
	GtkWidget *bLIBGL;	/* Custom GL library toggle */
	GtkWidget *GL_Entry;	/* Custom GL library path */
};

struct Sound_s
{
	GtkWidget *cSND;	/* Global sound option */
	GtkWidget *cSRATE;	/* Sampling rate */
	GtkWidget *bSBITS;	/* Sample format */
	GtkWidget *bMIDI;	/* Midi music option */
	GtkWidget *bCDA;	/* CD Audio option */
};

struct Misc_s
{
	GtkWidget *bLAN;	/* LAN button */
	GtkWidget *bMOUSE;	/* Mouse button */
	GtkWidget *bDBG;	/* Log Debuginfo button */
	GtkWidget *bDBG2;	/* Full Log Debuginfo */
	GtkWidget *bMEMHEAP;	/* Heapsize check button */
	GtkAdjustment *adjHEAP;	/* Heapsize adjustment */
	GtkWidget *spnHEAP;	/* Heapsize entry */
	GtkWidget *bMEMZONE;	/* Zonesize check button */
	GtkAdjustment *adjZONE;	/* Zonesize adjustment */
	GtkWidget *spnZONE;	/* Zonesize entry */
	GtkWidget *bEXTBTN;	/* Custom arguments toggle */
	GtkWidget *EXT_Entry;	/* Custom arguments string */
};

typedef struct
{
	GtkWidget *bHEXEN2;	/* Hexen II */
	GtkWidget *bH2W;	/* HexenWorld Client */
	GtkWidget *SelH2;	/* Hexen2 Game type selection */
	GtkWidget *SelHW;	/* HexenWorld Game type selection */
	GtkWidget *bH2MP;	/* Expansion Pack */
	struct Launch_s Launch;
	struct Video_s Video;
	struct Sound_s Sound;
	struct Misc_s Others;
} options_widget_t;


/* These crap are some supposed-to-be-friendlier
   macros for the main window layout members.. */

/* Main window layout members: */
#define	MAIN_WINDOW	main_win.mywindow
#define	PATCH_WINDOW	patch_win.mywindow
#define	HOLDER_BOX	main_win.main_box
#define	BOOK0		main_win.notebook0
#define	BOOK1		main_win.notebook1
#define	BASIC_TAB	main_win.fixed0
#define	PATCH_TAB	patch_win.fixed1
#define	ADDON_TAB1	main_win.fixed1
#define	ADDON_TAB2	main_win.fixed2
#define	TAB0_LABEL	main_win.lbl_tab0
#define	TAB1_LABEL	main_win.lbl_tab1
#define	TAB2_LABEL	main_win.lbl_tab2
#define	MORE_LESS	main_win.bMORE

/* Options struct members. WGT for "widget" */
#define	WGT_HEXEN2	Options.bHEXEN2
#define	WGT_H2WORLD	Options.bH2W
#define	WGT_H2GAME	Options.SelH2
#define	WGT_HWGAME	Options.SelHW
#define	WGT_PORTALS	Options.bH2MP

#define	WGT_OPENGL	Options.Video.bOGL
#define	WGT_RESMENU	Options.Video.mRES
#define	WGT_CONWBUTTON	Options.Video.bCONW
#define	WGT_CONWMENU	Options.Video.mCONW
#define	WGT_FULLSCR	Options.Video.bFULS
#define	WGT_3DFX	Options.Video.b3DFX
#define	WGT_GL8BIT	Options.Video.b8BIT
#define	WGT_LM_BYTES	Options.Video.bLM_BYTES
#define	WGT_VSYNC	Options.Video.bVSYNC
#define	WGT_FSAA	Options.Video.bFSAA
#define	WGT_ADJFSAA	Options.Video.adjFSAA
#define	WGT_AASAMPLES	Options.Video.spnFSAA
#define	WGT_GLPATH	Options.Video.GL_Entry
#define	WGT_LIBGL	Options.Video.bLIBGL

#define	WGT_SOUND	Options.Sound.cSND
#define	WGT_SRATE	Options.Sound.cSRATE
#define	WGT_SBITS	Options.Sound.bSBITS
#define	WGT_MIDI	Options.Sound.bMIDI
#define	WGT_CDAUDIO	Options.Sound.bCDA

#define	WGT_LANBUTTON	Options.Others.bLAN
#define	WGT_MOUSEBUTTON	Options.Others.bMOUSE
#define	WGT_DBGLOG	Options.Others.bDBG
#define	WGT_DBGLOG2	Options.Others.bDBG2
#define	WGT_MEMHEAP	Options.Others.bMEMHEAP
#define	WGT_HEAPADJ	Options.Others.adjHEAP
#define	WGT_HEAPSIZE	Options.Others.spnHEAP
#define	WGT_MEMZONE	Options.Others.bMEMZONE
#define	WGT_ZONEADJ	Options.Others.adjZONE
#define	WGT_ZONESIZE	Options.Others.spnZONE
#define	WGT_EXTARGS	Options.Others.EXT_Entry
#define	WGT_EXTBTN	Options.Others.bEXTBTN

#define	WGT_LAUNCH	Options.Launch.bLAUNCH
#define	WGT_STATBAR_ID	Options.Launch.statbar_id
#define	WGT_STATUSBAR	Options.Launch.StatusBar

#define	PATCH_STATBAR	patch_win.StatusBar

#endif	/* LAUNCHER_GTK_H */

