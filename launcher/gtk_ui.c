/*
 * gtk_ui.c
 * hexen2 launcher gtk+ interface
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

#include "launcher_defs.h"
#include "common.h"

#include "launcher_ui.h"
#include "gtk_ui.h"

#include <pthread.h>
#if !defined(DEMOBUILD)
#include "apply_patch.h"
#endif	/* !DEMOBUILD */

#include "config_file.h"
#include "games.h"

/*********************************************************************/


#if (GTK_MAJOR_VERSION <= 2)
static GtkTooltips	*tooltips;
#endif

static options_widget_t	Options;
static MainWindow_t	main_win;
static PatchWindow_t	patch_win;
static int		bmore = 0;
static GTK_SIGHANDLER_T	reslist_handler, conwlist_handler;
#ifndef DEMOBUILD
static GTK_SIGHANDLER_T	h2game_handler, hwgame_handler;
#endif

static const char *res_names[RES_MAX] =
{
	"320 x 240",
	"400 x 300",
	"512 x 384",
	"640 x 480",
	"800 x 600",
	"1024 x 768",
	"1280 x 1024",
	"1600 x 1200"
};

static const char *launch_status[] =
{
	"  Ready to run the game",
	"  Binary missing or not executable",
#if defined(DEMOBUILD)
	"  Installation problem"
#else	/* RETAIL */
	"  Installation problem",
	"  Pak files need patching"
#endif	/* DEMOBUILD */
};


/*********************************************************************
 Option menu crap'o'la....
 *********************************************************************/

#if (GTK_MAJOR_VERSION < 2)
#define	OPTMENU_CALLBACK_PARMS	GtkMenuShell *ms, GtkOptionMenu *optmenu
#define	GTKUI_OPTMENU_T		GtkOptionMenu
#define	GTKUI_OPTMENU_CAST(_m)	GTK_OPTION_MENU((_m))
#define	GTKUI_GET_OPTMENU_IDX	gtk_option_menu_get_history
#define	GTKUI_SET_OPTMENU_IDX	gtk_option_menu_set_history
#define	GTKUI_OPTMENU_VARS	GtkWidget *menu, *menu_item;
#define	GTKUI_OPTMENU_NEW()		gtk_option_menu_new()
#define	GTKUI_OPTMENUS_BEGIN(_m)	menu = gtk_menu_new()
#define	GTKUI_OPTMENUS_FRESHEN(_m)	gtk_option_menu_remove_menu(GTK_OPTION_MENU((_m)));	\
					GTKUI_OPTMENUS_BEGIN((_m))
#define	GTKUI_OPTMENUS_ADD(_m,_i)	menu_item = gtk_menu_item_new_with_label((_i));		\
					gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);	\
					gtk_widget_show(menu_item)
#define	GTKUI_OPTMENUS_FINISH(_m)	gtk_option_menu_set_menu(GTK_OPTION_MENU((_m)),menu)
#elif (GTK_MAJOR_VERSION < 3)
#define	OPTMENU_CALLBACK_PARMS	GtkOptionMenu *optmenu, gpointer user_data
#define	GTKUI_OPTMENU_T		GtkOptionMenu
#define	GTKUI_OPTMENU_CAST(_m)	GTK_OPTION_MENU((_m))
#define	GTKUI_GET_OPTMENU_IDX	gtk_option_menu_get_history
#define	GTKUI_SET_OPTMENU_IDX	gtk_option_menu_set_history
#define	GTKUI_OPTMENU_VARS	GtkWidget *menu, *menu_item;
#define	GTKUI_OPTMENU_NEW()		gtk_option_menu_new()
#define	GTKUI_OPTMENUS_BEGIN(_m)	menu = gtk_menu_new()
#define	GTKUI_OPTMENUS_FRESHEN(_m)	gtk_option_menu_remove_menu(GTK_OPTION_MENU((_m)));	\
					GTKUI_OPTMENUS_BEGIN((_m))
#define	GTKUI_OPTMENUS_ADD(_m,_i)	menu_item = gtk_menu_item_new_with_label((_i));		\
					gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);	\
					gtk_widget_show(menu_item)
#define	GTKUI_OPTMENUS_FINISH(_m)	gtk_option_menu_set_menu(GTK_OPTION_MENU((_m)),menu)
#else /* gtk3+ : */
#define	OPTMENU_CALLBACK_PARMS	GtkComboBox *optmenu, gpointer user_data
#define	GTKUI_OPTMENU_T		GtkComboBox
#define	GTKUI_OPTMENU_CAST(_m)	GTK_COMBO_BOX((_m))
#define	GTKUI_GET_OPTMENU_IDX	gtk_combo_box_get_active
#define	GTKUI_SET_OPTMENU_IDX	gtk_combo_box_set_active
#define	GTKUI_OPTMENU_VARS	/* none */
#define	GTKUI_OPTMENU_NEW()		gtk_combo_box_text_new()
#define	GTKUI_OPTMENUS_BEGIN(_m)	/* none */
#define	GTKUI_OPTMENUS_FRESHEN(_m)	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT((_m)))
#define	GTKUI_OPTMENUS_ADD(_m,_i)	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((_m)),(_i))
#define	GTKUI_OPTMENUS_FINISH(_m)	/* none */
#endif

static GTK_SIGHANDLER_T gtkui_set_optmenu_handler (GTKUI_OPTMENU_T *optmenu,
						   GTKUI_SIGNALFUNC_T handler)
{
#if (GTK_MAJOR_VERSION < 2)
/* GtkOptionMenu class doesn't have "changed"
 * signal in gtk1, so doing it indirectly: */
	return GTKUI_SIGNAL_CONNECT (GTK_MENU_SHELL(optmenu->menu),
					"selection-done", handler, optmenu);
#elif (GTK_MAJOR_VERSION < 3)
/* better way with gtk2: */
	return GTKUI_SIGNAL_CONNECT (optmenu, "changed", handler, NULL);
#else
/* gtk3 (or gtk2 >= 2.24) */
	return GTKUI_SIGNAL_CONNECT (optmenu, "changed", handler, NULL);
#endif
}

static void gtkui_kill_optmenu_handler (GTKUI_OPTMENU_T *optmenu,
					GTK_SIGHANDLER_T *handler)
{
#if (GTK_MAJOR_VERSION < 2)
	GTKUI_SIGNAL_DISCONNECT (GTK_MENU_SHELL(optmenu->menu), *handler);
#elif (GTK_MAJOR_VERSION < 3) /* gtk2: */
	GTKUI_SIGNAL_DISCONNECT (optmenu, *handler);
#else /* gtk3 (or gtk2 >= 2.24) */
	GTKUI_SIGNAL_DISCONNECT (optmenu, *handler);
#endif
	*handler = 0;
}

/*********************************************************************
 CALLBACK FUNCTIONS
 *********************************************************************/

static int CheckStats (void)
{
/* return broken installations */
#if !defined(DEMOBUILD)
	if (gameflags & (GAME_INSTBAD1|GAME_INSTBAD2|GAME_INSTBAD3))
		return 2;
	if (gameflags & GAME_INSTBAD0)
		return 3;
#else
	if (gameflags & GAME_INSTBAD)
		return 2;
#endif	/* DEMOBUILD */
/* return binary availability */
	if (opengl_support)
		return (0 == (gameflags & (HAVE_GLH2_BIN << destiny)));
	return (0 == (gameflags & (HAVE_H2_BIN << destiny)));
}

static void UpdateStats (void)
{
	int	status = CheckStats();
	gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), WGT_STATBAR_ID, launch_status[status]);
	gtk_widget_set_sensitive (WGT_LAUNCH, !status);
}

#if !defined(DEMOBUILD)
static const char *patch_status[] =
{
	"  Patch in progress.....",
	"  Patch process finished",
	"  Patch process failed"
};
#endif	/* !DEMOBUILD */

#if (GTK_MAJOR_VERSION < 2)
static GtkText *LogEntry = NULL;

static void ui_LogInit (GtkWidget *wgt)
{
	LogEntry = GTK_TEXT(wgt);
	/* clear the window */
	gtk_editable_delete_text (GTK_EDITABLE(wgt), 0, -1);
}

static void ui_LogPrint (const char *txt)
{
	if (! *txt)	/* nothing to do. */
		return;
	gtk_text_insert (LogEntry, NULL, NULL, NULL, txt, strlen(txt));
}

#else	/* here is the gtk2 version */
static GtkTextView *LogEntry = NULL;
static GtkTextMark *textmark = NULL;

static void ui_LogInit (GtkWidget *wgt)
{
	GtkTextBuffer	*buf;
	GtkTextIter	start, end;

	LogEntry = GTK_TEXT_VIEW(wgt);
	/* clear the window */
	buf = gtk_text_view_get_buffer (LogEntry);
	textmark = gtk_text_buffer_get_mark (buf, "ins_mark");
	gtk_text_buffer_get_start_iter (buf, &start);
	gtk_text_buffer_move_mark (buf, textmark, &start);
	gtk_text_view_scroll_to_mark (LogEntry, textmark, 0, 1, 1, 1);
	gtk_text_buffer_get_end_iter (buf, &end);
	gtk_text_buffer_delete (buf, &start, &end);
}

static void ui_LogPrint (const char *txt)
{
	GtkTextBuffer	*buf;
	GtkTextIter	end;

	if (! *txt)	/* nothing to do. */
		return;
	buf = gtk_text_view_get_buffer (LogEntry);
	gtk_text_buffer_get_end_iter (buf, &end);
	gtk_text_buffer_insert (buf, &end, txt, strlen(txt));
	gtk_text_buffer_get_end_iter (buf, &end);
	gtk_text_buffer_move_mark (buf, textmark, &end);
	gtk_text_view_scroll_to_mark (LogEntry, textmark, 0, 1, 1, 1);
}
#endif	/* _H2L_USE_GTK2 */

static void ui_LogEnd (void)
{
	LogEntry = NULL;
}

void ui_log (const char *fmt, ...)
{
	va_list         argptr;
	char            text[256];

	if (LogEntry == NULL)
		return;

	va_start (argptr, fmt);
	vsnprintf (text, sizeof (text), fmt, argptr);
	va_end (argptr);

	ui_LogPrint(text);
}

typedef struct LogQueue_s
{
	char		*data;
	struct LogQueue_s *next;
} LogQueue_t;

static LogQueue_t *log_queue;
#if !defined(DEMOBUILD)
static LogQueue_t *old_queue;
#endif
static pthread_mutex_t logmutex = PTHREAD_MUTEX_INITIALIZER;

void ui_log_queue (const char *fmt, ...)
{
	pthread_mutex_lock (&logmutex);

	if (log_queue->data)
	{	/* append to the queue: */
		va_list argptr;
		LogQueue_t *tmp, *newdata;
		newdata = (LogQueue_t *) g_malloc(sizeof(LogQueue_t));
		va_start (argptr, fmt);
		newdata->data = g_strdup_vprintf (fmt, argptr);
		va_end (argptr);
		newdata->next = NULL;
		tmp = log_queue;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = newdata;
	}
	else
	{	/* assign the initial data: */
		va_list argptr;
		va_start (argptr, fmt);
		log_queue->data = g_strdup_vprintf (fmt, argptr);
		va_end (argptr);
	}

	pthread_mutex_unlock (&logmutex);
}

static void report_status (gpointer *unused, PatchWindow_t *PatchWindow)
{
	int	end_log = 0;

	if (!LogEntry)
	{
		end_log = 1;
		ui_LogInit (PatchWindow->LOGVIEW);
	}

	ui_log ("Installation Summary:\n\n");
	ui_log ("Base directory: %s\n", basedir);
	ui_log ("Data directory: %s\n", (basedir_nonstd && game_basedir[0]) ? game_basedir : basedir);
	ui_log ("PAK file health: %s", (gameflags & GAME_INSTBAD) ? "BAD. Reason(s):\n" : "OK ");
	if (gameflags & GAME_INSTBAD)
	{
		if (gameflags & GAME_INSTBAD3)
			ui_log ("- Found an old, unsupported version of the demo.\n");
		if (gameflags & GAME_INSTBAD2)
			ui_log ("- Found mixed data from incompatible versions.\n");
		if (gameflags & GAME_INSTBAD1)
			ui_log ("- Neither of retail, demo or oem versions found.\n");
		if (gameflags & GAME_INSTBAD0)
			ui_log ("- Found pak files not patched to 1.11 version.\n");
		if (gameflags & GAME_CANPATCH)
			ui_log ("- Applying the 1.11 pak patch should solve this.\n");
	}
	else
	{
		ui_log ("(%s version.)\n", (gameflags & (GAME_DEMO|GAME_OLD_DEMO)) ? "demo" : ((gameflags & (GAME_OEM|GAME_OLD_OEM)) ? "oem" : "retail"));
		if (gameflags & (GAME_REGISTERED_OLD|GAME_OLD_DEMO|GAME_OLD_OEM))
			ui_log ("Using old/unsupported 1.03 version pak files.\n");
		if (gameflags & GAME_CANPATCH)
			ui_log ("Applying Raven's 1.11 pak patch is suggested.\n");
	}

	ui_log ("Mission Pack: %s", (gameflags & GAME_PORTALS) ? "present " : "not found");
	if (gameflags & GAME_PORTALS)
		ui_log ("%s", (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)) ? "\n" : "(ignored: no valid retail data)\n");
	else
		ui_log ("\n");

	ui_log ("HexenWorld: %s\n", (gameflags & GAME_HEXENWORLD) ? "present " : "not found");

#if defined(DEMOBUILD)
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_CANPATCH))
	{
		ui_log ("---------------------\n");
		ui_log ("This is a restricted build of Hexen II Launcher\n");
		ui_log ("for the demo version. Use the normal builds for\n");
		ui_log ("retail-only functionality and pakfile patching.\n");
	}
#endif	/* DEMOBUILD */

	if (end_log)
		ui_LogEnd ();
}

#if !defined(DEMOBUILD)
static unsigned long	last_written;
static gfloat	percentage;

static gboolean block_window_close (GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	return TRUE;
}

static void create_progressbar (PatchWindow_t *PatchWindow)
{
	gtk_widget_hide (PatchWindow->bAPPLY);
	gtk_widget_hide (PatchWindow->bCLOSE);
	gtk_widget_hide (PatchWindow->bREPORT);

	PatchWindow->palign = gtk_alignment_new (0, 0, 1, 1); /* (0.5, 0.5, 0, 0) */
	gtk_widget_show (PatchWindow->palign);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), PatchWindow->palign, 14, 218);
	gtk_widget_set_size_request (PatchWindow->palign, 324, 24);
	PatchWindow->progbar = gtk_progress_bar_new ();
	gtk_container_add (GTK_CONTAINER(PatchWindow->palign), PatchWindow->progbar);
	gtk_widget_show (PatchWindow->progbar);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PatchWindow->progbar), 0);
}

static void destroy_progressbar (PatchWindow_t *PatchWindow)
{
	gtk_widget_destroy (PatchWindow->progbar);
	gtk_widget_destroy (PatchWindow->palign);
	gtk_widget_show (PatchWindow->bCLOSE);
	gtk_widget_show (PatchWindow->bAPPLY);
	gtk_widget_show (PatchWindow->bREPORT);
}

static void flush_log_queue (void)
{
	LogQueue_t *next;

	if (! log_queue->data)
		return;

	pthread_mutex_lock (&logmutex);
	old_queue = log_queue;
	log_queue = (LogQueue_t *) g_malloc(sizeof(LogQueue_t));
	log_queue->data = NULL;
	log_queue->next = NULL;
	pthread_mutex_unlock (&logmutex);

	do
	{
		next = old_queue->next;
		ui_LogPrint(old_queue->data);
		g_free(old_queue->data);
		g_free(old_queue);
		old_queue = next;
	} while (next);
}

static void patch_gui_loop (PatchWindow_t *PatchWindow)
{
	while (thread_alive)
	{
		if (last_written != h2patch_progress.current_written)
		{
			/* update progress bar: */
			percentage = (gfloat)h2patch_progress.current_written / (gfloat)h2patch_progress.total_bytes;
			last_written = h2patch_progress.current_written;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(PatchWindow->progbar), percentage);
		}
		flush_log_queue ();
		ui_pump ();
		g_usleep (10000);	/* usleep (10000) */
	}

	/* finish leftovers */
	flush_log_queue ();
}

static void start_xpatch (gpointer *unused, PatchWindow_t *PatchWindow)
{
	pthread_t		thr;
	char		*wd;
	unsigned long	*ptr;
	guint	delete_handler;
	int		ret = 2;

	if (basedir_nonstd && game_basedir[0])
		wd = game_basedir;
	else
		wd = basedir;

	gtk_widget_set_sensitive (PatchWindow->bBASEDIR, FALSE);
	gtk_widget_set_sensitive (PatchWindow->dir_Entry, FALSE);
	create_progressbar (PatchWindow);
	delete_handler =
		GTKUI_SIGNAL_CONNECT(PatchWindow->mywindow, "delete-event", block_window_close, NULL);
	ui_LogInit (PatchWindow->LOGVIEW);

	last_written = 0;
	thread_alive = 1;
	if (pthread_create(&thr, NULL, apply_patches, wd) != 0)
	{
		ui_log ("pthread_create failed");
		goto finish;
	}

	gtk_statusbar_push (GTK_STATUSBAR(PatchWindow->StatusBar), PatchWindow->statbar_id, patch_status[0]);

	patch_gui_loop (PatchWindow);

	if (pthread_join(thr, (void **) (char *) &ptr) != 0)
	{
		ui_log ("pthread_join failed");
		goto finish;
	}

	ret = (*ptr & XPATCH_FAIL) ? 2 : 1;
	if (*ptr & XPATCH_APPLIED)
	{
		scan_game_installation();
		UpdateStats ();
		ui_log ("---------------------\n");
		report_status (NULL, PatchWindow);
		if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
		{	/* activate the extra game options, if necessary */
			gtk_widget_set_sensitive (WGT_H2GAME, TRUE);
			gtk_widget_set_sensitive (WGT_HWGAME, TRUE);
			if (gameflags & GAME_PORTALS && destiny == DEST_H2)
				gtk_widget_set_sensitive (WGT_PORTALS, TRUE);
		}
	}
finish:
	GTKUI_SIGNAL_DISCONNECT (PatchWindow->mywindow, delete_handler);
	gtk_widget_set_sensitive (PatchWindow->bBASEDIR, TRUE);
	gtk_widget_set_sensitive (PatchWindow->dir_Entry, TRUE);
	destroy_progressbar (PatchWindow);
	gtk_statusbar_push (GTK_STATUSBAR(PatchWindow->StatusBar), PatchWindow->statbar_id, patch_status[ret]);
	ui_LogEnd();
}
#endif	/* ! DEMOBUILD */

static void on_SND (OPTMENU_CALLBACK_PARMS)
{
	int i = GTKUI_GET_OPTMENU_IDX(optmenu);

	sound = snd_drivers[i].id;
	gtk_widget_set_sensitive (WGT_MIDI, sound);
	gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
	gtk_widget_set_sensitive (WGT_SRATE, sound);
	gtk_widget_set_sensitive (WGT_SBITS, sound);
}

static void on_SRATE (OPTMENU_CALLBACK_PARMS)
{
	sndrate = GTKUI_GET_OPTMENU_IDX(optmenu);
}

static void on_SBITS (GtkButton *button, int *opt)
{
	*opt ^= 1;
	gtk_button_set_label(button, (*opt) ? "16 bit" : " 8 bit");
}

static void res_Change (OPTMENU_CALLBACK_PARMS);
static void Make_ResMenu (void)
{
	int	i, up;
	int	new_handler;
	GTKUI_OPTMENU_VARS

	new_handler = 0;
	up = (opengl_support) ? RES_MAX-1 : RES_640;
	i  = (opengl_support) ? RES_MINGL : 0;

	if (reslist_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_RESMENU), &reslist_handler);
		new_handler = 1;
	}

	GTKUI_OPTMENUS_FRESHEN(WGT_RESMENU);
	for ( ; i <= up; i++)
	{
		GTKUI_OPTMENUS_ADD(WGT_RESMENU,res_names[i]);
	}
	GTKUI_OPTMENUS_FINISH(WGT_RESMENU);

	i  = resolution;
	if (opengl_support)
		i -= RES_MINGL;
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_RESMENU), i);
	/* if there was a signal handler, install a new one: */
	if (new_handler)
		reslist_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_RESMENU), GTKUI_SIGNALFUNC(res_Change));
}

static void con_Change (OPTMENU_CALLBACK_PARMS);
static void Make_ConWidthMenu (void)
{
	int	i;
	int	new_handler;
	GTKUI_OPTMENU_VARS

	new_handler = 0;
	if (conwlist_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), &conwlist_handler);
		new_handler = 1;
	}

	GTKUI_OPTMENUS_FRESHEN(WGT_CONWMENU);
	for (i = 0; i <= resolution; i++)
	{
		GTKUI_OPTMENUS_ADD(WGT_CONWMENU, res_names[i]);
	}
	GTKUI_OPTMENUS_FINISH(WGT_CONWMENU);

	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_CONWMENU), conwidth);
	/* if there was a signal handler, install a new one: */
	if (new_handler)
		conwlist_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), GTKUI_SIGNALFUNC(con_Change));
}

static void on_OGL (GtkToggleButton *button, gpointer user_data)
{
	int	new_handler = 0;

	if (reslist_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_RESMENU), &reslist_handler);
		new_handler |= 1;
	}
	if (conwlist_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), &conwlist_handler);
		new_handler |= 2;
	}

	opengl_support ^= 1;
	if (opengl_support)
	{
		if (resolution < RES_640)
			resolution = RES_640;
		if (conwidth > resolution)
			conwidth = resolution;
	}
	else
	{
		if (resolution > RES_640)
			resolution = RES_640;
	}
	gtk_widget_set_sensitive (WGT_3DFX, opengl_support);
	gtk_widget_set_sensitive (WGT_GL8BIT, opengl_support);
	gtk_widget_set_sensitive (WGT_LM_BYTES, opengl_support);
	gtk_widget_set_sensitive (WGT_VSYNC, opengl_support);
	gtk_widget_set_sensitive (WGT_FSAA, opengl_support);
	gtk_widget_set_sensitive (WGT_LIBGL, opengl_support);
	gtk_widget_set_sensitive (WGT_CONWBUTTON, opengl_support);
	gtk_widget_set_sensitive (WGT_CONWMENU, opengl_support);
	Make_ResMenu ();
	if (opengl_support)
		Make_ConWidthMenu();
	UpdateStats ();

	if (new_handler & 1)
		reslist_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_RESMENU), GTKUI_SIGNALFUNC(res_Change));
	if (new_handler & 2)
		conwlist_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), GTKUI_SIGNALFUNC(con_Change));
}

static void res_Change (OPTMENU_CALLBACK_PARMS)
{
	int	new_handler = 0;

	if (conwlist_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), &conwlist_handler);
		new_handler = 1;
	}

	resolution = GTKUI_GET_OPTMENU_IDX(optmenu);
	if (opengl_support)
		resolution += RES_MINGL;
	if (opengl_support)
	{
		if (conwidth > resolution)
			conwidth = resolution;
		Make_ConWidthMenu ();
	}

	if (new_handler)
		conwlist_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_CONWMENU), GTKUI_SIGNALFUNC(con_Change));
}

static void con_Change (OPTMENU_CALLBACK_PARMS)
{
	conwidth = GTKUI_GET_OPTMENU_IDX(optmenu);
}

static void libgl_Change (GtkEntry *entry, gpointer user_data)
{
	const gchar *tmp = gtk_entry_get_text (entry);
	size_t len = strlen(tmp);
	if (len > sizeof(gllibrary)-1)
		len = sizeof(gllibrary)-1;
	if (len)
		memcpy (gllibrary, tmp, len);
	gllibrary[len] = 0;
}

static void extargs_Change (GtkEntry *entry, gpointer user_data)
{
	const gchar *tmp = gtk_entry_get_text (entry);
	size_t len = strlen(tmp);
	if (len > sizeof(ext_args)-1)
		len = sizeof(ext_args)-1;
	if (len)
		memcpy (ext_args, tmp, len);
	ext_args[len] = 0;
}

static void on_HEXEN2 (GtkButton *button, gpointer user_data)
{
	destiny = DEST_H2;
#ifndef DEMOBUILD
	if (gameflags & GAME_PORTALS && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
		gtk_widget_set_sensitive (WGT_PORTALS, TRUE);
	gtk_widget_set_sensitive (WGT_LANBUTTON, !h2game_names[h2game].is_botmatch);
	gtk_widget_hide (WGT_HWGAME);
	gtk_widget_show (WGT_H2GAME);
#else
	gtk_widget_set_sensitive (WGT_LANBUTTON, TRUE);
#endif
	UpdateStats ();
}

static void on_H2W (GtkButton *button, gpointer user_data)
{
	destiny = DEST_HW;
#ifndef DEMOBUILD
	if (gameflags & GAME_PORTALS && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
		gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
	gtk_widget_hide (WGT_H2GAME);
	gtk_widget_show (WGT_HWGAME);
#endif
	gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);
	UpdateStats ();
}

#if !defined(DEMOBUILD)
static void H2GameChange (OPTMENU_CALLBACK_PARMS)
{
	int i;
	int menu_index = GTKUI_GET_OPTMENU_IDX(optmenu);

	h2game = 0;
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (h2game_names[i].menu_index == menu_index)
		{
			h2game = i;
			break;
		}
	}
	gtk_widget_set_sensitive (WGT_LANBUTTON, !h2game_names[h2game].is_botmatch);
}

static void HWGameChange (OPTMENU_CALLBACK_PARMS)
{
	int i;
	int menu_index = GTKUI_GET_OPTMENU_IDX(optmenu);

	hwgame = 0;
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (hwgame_names[i].menu_index == menu_index)
		{
			hwgame = i;
			break;
		}
	}
}
#endif	/* ! DEMOBUILD */

static void BoolRevert (gpointer *unused, int *opt)
{
	*opt ^= 1;
}

static void adj_Change (GtkAdjustment *adj, int *opt)
{
	*opt = (int) gtk_adjustment_get_value(adj);
}

static void on_MORE (GtkButton *button, gpointer user_data)
{
	bmore ^= 1;
	gtk_button_set_label(button, bmore ? _("<<< Less") : _("More >>>"));
	gtk_widget_set_size_request(MAIN_WINDOW, bmore ? 460 : 230, 354);
	if (bmore)
		gtk_widget_show (BOOK1);
	else
		gtk_widget_hide (BOOK1);
}

static void basedir_Change (GtkButton *unused, gpointer user_data)
{
	static gboolean	in_progress = FALSE;	/* do I need this? */
#if !defined(DEMOBUILD)
	int		i, menu_index;
	int		new_handler;
	GTKUI_OPTMENU_VARS
#endif	/* ! DEMOBUILD */

	if (in_progress)
		return;
	in_progress = TRUE;

	basedir_nonstd ^= 1;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patch_win.bBASEDIR), basedir_nonstd);
	scan_game_installation();
	UpdateStats ();
	report_status (NULL, &patch_win);

/* activate the extra game options, if necessary */
	gtk_widget_set_sensitive (WGT_H2WORLD, (gameflags & GAME_HEXENWORLD) ? TRUE : FALSE);
/* rebuild the game mod lists */
#if !defined(DEMOBUILD)
	new_handler = 0;
	if (h2game_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_H2GAME), &h2game_handler);
		new_handler |= 1;
	}
	if (hwgame_handler)
	{
		gtkui_kill_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_HWGAME), &hwgame_handler);
		new_handler |= 2;
	}

	if (mp_support)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_PORTALS), FALSE);
	h2game = hwgame = mp_support = 0;
	menu_index = 0;
	GTKUI_OPTMENUS_FRESHEN(WGT_H2GAME);
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (h2game_names[i].available)
		{
			GTKUI_OPTMENUS_ADD(WGT_H2GAME, h2game_names[i].name);
			h2game_names[i].menu_index = menu_index;
			menu_index++;
		}
		else	h2game_names[i].menu_index = -1;
	}
	GTKUI_OPTMENUS_FINISH(WGT_H2GAME);

	menu_index = 0;
	GTKUI_OPTMENUS_FRESHEN(WGT_HWGAME);
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (hwgame_names[i].available)
		{
			GTKUI_OPTMENUS_ADD(WGT_HWGAME, hwgame_names[i].name);
			hwgame_names[i].menu_index = menu_index;
			menu_index++;
		}
		else	hwgame_names[i].menu_index = -1;
	}
	GTKUI_OPTMENUS_FINISH(WGT_HWGAME);

	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_H2GAME), 0);
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_HWGAME), 0);
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		gtk_widget_set_sensitive (WGT_H2GAME, TRUE);
		gtk_widget_set_sensitive (WGT_HWGAME, TRUE);
		if (gameflags & GAME_PORTALS && destiny == DEST_H2)
			gtk_widget_set_sensitive (WGT_PORTALS, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (WGT_H2GAME, FALSE);
		gtk_widget_set_sensitive (WGT_HWGAME, FALSE);
		gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
	}

	/* if there was a signal handler, install a new one: */
	if (new_handler & 1)
		h2game_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_H2GAME), GTKUI_SIGNALFUNC(H2GameChange));
	if (new_handler & 2)
		hwgame_handler = gtkui_set_optmenu_handler(GTKUI_OPTMENU_CAST(WGT_HWGAME), GTKUI_SIGNALFUNC(HWGameChange));
#endif	/* ! DEMOBUILD */

	in_progress = FALSE;
}

static void basedir_ChangePath (GtkEntry *entry, gpointer user_data)
{
	const gchar *tmp = gtk_entry_get_text (entry);
	size_t len = strlen(tmp);
	if (len > sizeof(game_basedir)-1)
		len = sizeof(game_basedir)-1;
	if (len)
		memcpy (game_basedir, tmp, len);
	game_basedir[len] = 0;

	if (basedir_nonstd)	/* FIXME: any better way? */
		basedir_Change (NULL, NULL);
}

/*********************************************************************
 WINDOW CREATING
 *********************************************************************/

static void destroy_window2(GtkWidget *unused1, gpointer user_data)
{
	gtk_widget_destroy (PATCH_WINDOW);
}

static void create_window2 (GtkWidget *unused1, gpointer user_data)
{
	GtkWidget *Txt1;	/* Window label */
	GtkWidget *TxtWindow;	/* Holder Window for the textview */
#if (GTK_MAJOR_VERSION > 1)
	GtkTextBuffer	*buf;
	GtkTextIter	start;
#endif	/* GTK2+ */

	PATCH_WINDOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(PATCH_WINDOW), "Hexen II PAK patch");
	gtk_window_set_resizable (GTK_WINDOW(PATCH_WINDOW), FALSE);
	gtk_window_set_modal (GTK_WINDOW(PATCH_WINDOW), TRUE);
	gtk_widget_set_size_request(PATCH_WINDOW, 360, 272);

	PATCH_TAB = gtk_fixed_new ();
	gtk_container_add (GTK_CONTAINER (PATCH_WINDOW), PATCH_TAB);
	gtk_widget_show (PATCH_TAB);

	Txt1 = gtk_label_new ("Hexen II Installation status");
	gtk_widget_show (Txt1);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), Txt1, 14, 12);
	gtk_label_set_justify (GTK_LABEL(Txt1), GTK_JUSTIFY_LEFT);

/* custom basedir entry: */
	patch_win.bBASEDIR = gtk_check_button_new_with_label (_("Data install path:"));
	gtk_widget_show (patch_win.bBASEDIR);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bBASEDIR, 14, 186);
	gtk_widget_set_size_request (patch_win.bBASEDIR, 128, 24);
	gtk_widget_set_sensitive (patch_win.bBASEDIR, TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patch_win.bBASEDIR), basedir_nonstd);
	GTKUI_DISABLE_FOCUS (patch_win.bBASEDIR);
	gtk_tooltips_set_tip (tooltips, patch_win.bBASEDIR, _("Mark this in order to use a different game installation directory"), NULL);

	patch_win.dir_Entry = gtk_entry_new();
	gtk_widget_show (patch_win.dir_Entry);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.dir_Entry, 148, 186);
	gtk_widget_set_size_request (patch_win.dir_Entry, 190, 24);
	gtk_entry_set_max_length (GTK_ENTRY(patch_win.dir_Entry), sizeof(game_basedir)-1);
	gtk_entry_set_text (GTK_ENTRY(patch_win.dir_Entry), game_basedir);

/* Apply Patch button */
	patch_win.bAPPLY = gtk_button_new_with_label (_("Apply Pak Patch"));
	gtk_widget_show (patch_win.bAPPLY);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bAPPLY, 14, 218);
	gtk_widget_set_size_request (patch_win.bAPPLY, 112, 24);
#if !defined(DEMOBUILD)
	gtk_tooltips_set_tip (tooltips, patch_win.bAPPLY, _("Apply the v1.11 pakfiles patch by Raven Software."), NULL);
#endif	/* ! DEMOBUILD */

/* Installation status button */
	patch_win.bREPORT = gtk_button_new_with_label (_("Make Report"));
	gtk_widget_show (patch_win.bREPORT);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bREPORT, 132, 218);
	gtk_widget_set_size_request (patch_win.bREPORT, 112, 24);

/* Holder window for the textview */
	TxtWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), TxtWindow, 14, 32);
	gtk_widget_set_size_request (TxtWindow, 324, 146);
	gtk_widget_show (TxtWindow);
/*	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(TxtWindow), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);*/
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(TxtWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(TxtWindow), GTK_SHADOW_ETCHED_IN);

/* The textview: */
#if (GTK_MAJOR_VERSION < 2)
	patch_win.LOGVIEW = gtk_text_new (NULL, NULL);
#else	/* GTK2 */
	patch_win.LOGVIEW = gtk_text_view_new ();
#endif	/* GTK_MAJOR_VERSION */
	gtk_widget_set_size_request (patch_win.LOGVIEW, 324, 146);
	gtk_container_add (GTK_CONTAINER (TxtWindow), patch_win.LOGVIEW);
	gtk_widget_show (patch_win.LOGVIEW);
	GTKUI_DISABLE_FOCUS (patch_win.LOGVIEW);
/*	gtk_widget_set_sensitive (patch_win.LOGVIEW, FALSE);*/
#if (GTK_MAJOR_VERSION < 2)
/*	gtk_text_set_line_wrap(GTK_TEXT(patch_win.LOGVIEW), FALSE);*/
	gtk_text_set_editable (GTK_TEXT(patch_win.LOGVIEW), FALSE);
#else	/* GTK2 */
	gtk_text_view_set_editable (GTK_TEXT_VIEW(patch_win.LOGVIEW), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(patch_win.LOGVIEW), FALSE);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW(patch_win.LOGVIEW), 2);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW(patch_win.LOGVIEW), 2);
	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(patch_win.LOGVIEW));
	gtk_text_buffer_get_start_iter (buf, &start);
	gtk_text_buffer_create_mark (buf, "ins_mark", &start, 0);
#endif	/* GTK_MAJOR_VERSION */

/* Close button */
	patch_win.bCLOSE = gtk_button_new_with_label (_("Close"));
	gtk_widget_show (patch_win.bCLOSE);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bCLOSE, 250, 218);
	gtk_widget_set_size_request (patch_win.bCLOSE, 88, 24);

/* Statusbar */
	PATCH_STATBAR = gtk_statusbar_new ();
	gtk_widget_show (PATCH_STATBAR);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), PATCH_STATBAR, 0, 246);
	gtk_widget_set_size_request (PATCH_STATBAR, 354, 24);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(PATCH_STATBAR), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER(PATCH_STATBAR), 2);
	patch_win.statbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(PATCH_STATBAR), "PatchStatus");
	gtk_statusbar_push (GTK_STATUSBAR(PATCH_STATBAR), patch_win.statbar_id, _("  Ready..."));

	GTKUI_SIGNAL_CONNECT (PATCH_WINDOW, "destroy", destroy_window2, NULL);
	GTKUI_SIGNAL_CONNECT (patch_win.bCLOSE, "clicked", destroy_window2, NULL);
	GTKUI_SIGNAL_CONNECT (patch_win.bBASEDIR, "toggled", basedir_Change, NULL);
	GTKUI_SIGNAL_CONNECT (patch_win.dir_Entry, "changed", basedir_ChangePath, NULL);
#if !defined(DEMOBUILD)
	GTKUI_SIGNAL_CONNECT (patch_win.bAPPLY, "clicked", start_xpatch, &patch_win);
	GTKUI_SIGNAL_CONNECT (patch_win.bREPORT, "clicked", report_status, &patch_win);
#endif	/* !DEMOBUILD */

#if defined(DEMOBUILD)
	gtk_widget_set_sensitive (patch_win.bAPPLY, FALSE);
	gtk_widget_set_sensitive (patch_win.bREPORT, FALSE);
#endif	/* DEMOBUILD */

	gtk_window_set_transient_for (GTK_WINDOW(PATCH_WINDOW), GTK_WINDOW(MAIN_WINDOW));
/*	gtk_window_set_position (GTK_WINDOW(PATCH_WINDOW), GTK_WIN_POS_NONE);*/
	gtk_window_set_position (GTK_WINDOW(PATCH_WINDOW), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show (PATCH_WINDOW);

/* give a short summary about installation */
	report_status (NULL, &patch_win);
}

static void create_window1 (void)
{
	int			i;

/* Labels for basics */
	GtkWidget *TxtTitle;	/* Title Label */
	GtkWidget *TxtGame0;	/* Destiny label */
	GtkWidget *TxtVideo;	/* Renderer, etc. */
	GtkWidget *TxtResol;	/* Resolution */
	GtkWidget *TxtSound;	/* Sound driver menu */
/* Widgets for basics which needn't be in a relevant struct */
	GtkWidget *bQUIT;	/* Quit button */

/* Labels for additionals */
	GtkWidget *TxtMouse;	/* Mouse options. */
	GtkWidget *TxtNet;	/* Networking options. */
	GtkWidget *TxtAdv;	/* Memory options. */
	GtkWidget *TxtVidExt;	/* Extra Video Options label */
	GtkWidget *TxtSndExt;	/* Extra Sound Options label */
	GtkWidget *TxtSound2;	/* Sound options extra */
	GtkWidget *TxtSound3;	/* Sound options extra */
	GtkWidget *TxtGameT;	/* GameType Label */
/* Widgets for additionals which needn't be in a relevant struct */
	GtkWidget *bPATCH;	/* PATCH button */

/* Separators */
	GtkWidget *hseparator0;
	GtkWidget *hseparator1;

/* Other stuff */
	GTKUI_OPTMENU_VARS
	GSList *Destinies;
#if !defined(DEMOBUILD)
	int	game_index, menu_index;
#endif
#if (GTK_MAJOR_VERSION <= 2)
	tooltips = gtk_tooltips_new ();
#endif

/* Create the main window */
	MAIN_WINDOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(MAIN_WINDOW), "Hexen II Launcher " LAUNCHER_VERSION_STR);
	gtk_window_set_resizable (GTK_WINDOW(MAIN_WINDOW), FALSE);
	gtk_widget_set_size_request(MAIN_WINDOW, 230, 354);

/* Create the option tabs on the main window */
	HOLDER_BOX = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (HOLDER_BOX);
	gtk_container_add (GTK_CONTAINER(MAIN_WINDOW), HOLDER_BOX);

	BOOK0 = gtk_notebook_new ();
/*	gtk_container_add (GTK_CONTAINER (MAIN_WINDOW), BOOK0);*/
	gtk_widget_show (BOOK0);
	gtk_box_pack_start (GTK_BOX(HOLDER_BOX), BOOK0, TRUE, TRUE, 0);

	BASIC_TAB = gtk_fixed_new ();
	gtk_widget_show (BASIC_TAB);

	BOOK1 = gtk_notebook_new ();
	gtk_widget_show (BOOK1);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK(BOOK1), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK(BOOK1));
	gtk_box_pack_start (GTK_BOX(HOLDER_BOX), BOOK1, TRUE, TRUE, 0);

	ADDON_TAB1 = gtk_fixed_new ();
	gtk_widget_show (ADDON_TAB1);

	ADDON_TAB2 = gtk_fixed_new ();
	gtk_widget_show (ADDON_TAB2);

/*	TAB0_LABEL = gtk_label_new ("Basic Options");*/
	TAB1_LABEL = gtk_label_new (" Sound / Other ");
	TAB2_LABEL = gtk_label_new ("  Video / Gameplay ");

/*	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK0), BASIC_TAB, TAB0_LABEL);*/
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK0), BASIC_TAB, NULL);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(BOOK0), FALSE);
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK1), ADDON_TAB2, TAB2_LABEL);
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK1), ADDON_TAB1, TAB1_LABEL);

	gtk_widget_hide (BOOK1);

/* Basic title representing the HoT-version the launcher is packed with */
	TxtTitle = gtk_label_new ("Hammer of Thyrion " HOT_VERSION_STR);
	gtk_widget_show (TxtTitle);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtTitle, 14, 12);
	gtk_label_set_justify (GTK_LABEL(TxtTitle), GTK_JUSTIFY_LEFT);

/********************************************************************
 TAB - 1:		BASIC OPTIONS
 ********************************************************************/

/* pakfiles 1.11 patch option: */
/* PATCH button */
	bPATCH = gtk_button_new_with_label (_("Info & 1.11 patch"));
	gtk_widget_show (bPATCH);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), bPATCH, 12, 272);
	gtk_widget_set_size_request (bPATCH, 112, 24);
#ifdef DEMOBUILD
	gtk_tooltips_set_tip (tooltips, bPATCH, _("See a short summary of your installation."), NULL);
#else
	gtk_tooltips_set_tip (tooltips, bPATCH, _("See a short summary of your installation. Apply the v1.11 pakfiles patch by Raven Software, if you haven't done already."), NULL);
#endif

/* Launch button and Statusbar */
	WGT_LAUNCH = gtk_button_new_with_label (_("Play Hexen II"));
	gtk_widget_show (WGT_LAUNCH);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_LAUNCH, 12, 300);
	gtk_widget_set_size_request (WGT_LAUNCH, 112, 24);

	WGT_STATUSBAR = gtk_statusbar_new ();
	gtk_widget_show (WGT_STATUSBAR);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_STATUSBAR, 0, 328);
	gtk_widget_set_size_request (WGT_STATUSBAR, 230, 24);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(WGT_STATUSBAR), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (WGT_STATUSBAR), 2);
	WGT_STATBAR_ID = gtk_statusbar_get_context_id (GTK_STATUSBAR(WGT_STATUSBAR), "BinaryStatus");
	gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), WGT_STATBAR_ID,_("  Waiting status..."));

/* Quit button */
	bQUIT = gtk_button_new_with_label (_("Quit"));
	gtk_widget_show (bQUIT);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), bQUIT, 132, 300);
	gtk_widget_set_size_request (bQUIT, 80, 24);

/*********************************************************************/

/* DESTINY : Game/binary selection */
	TxtGame0 = gtk_label_new (_("Choose your destiny:"));
	gtk_widget_show (TxtGame0);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtGame0, 14, 40);
	gtk_label_set_justify (GTK_LABEL(TxtGame0), GTK_JUSTIFY_LEFT);

/* Destiny: Hexen2 */
#ifdef DEMOBUILD
	WGT_HEXEN2 = gtk_radio_button_new_with_label (NULL, "Hexen II demo");
#else
	WGT_HEXEN2 = gtk_radio_button_new_with_label (NULL, "Hexen II");
#endif
	Destinies = gtk_radio_button_get_group (GTK_RADIO_BUTTON(WGT_HEXEN2));
	GTKUI_DISABLE_FOCUS (WGT_HEXEN2);
	gtk_widget_show (WGT_HEXEN2);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_HEXEN2, 24, 60);
#ifndef DEMOBUILD
	gtk_widget_set_size_request (WGT_HEXEN2, 80, 24);
#else
	gtk_widget_set_size_request (WGT_HEXEN2, 180, 24);
#endif

/* Destiny: HexenWorld */
	WGT_H2WORLD = gtk_radio_button_new_with_label (Destinies, "HexenWorld Multiplayer");
	GTKUI_DISABLE_FOCUS (WGT_H2WORLD);
	gtk_widget_show (WGT_H2WORLD);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_H2WORLD, 24, 86);
	gtk_widget_set_size_request (WGT_H2WORLD, 180, 24);
	if (!(gameflags & GAME_HEXENWORLD))
		gtk_widget_set_sensitive (WGT_H2WORLD, FALSE);

	if (destiny == DEST_HW)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_H2WORLD), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_HEXEN2), TRUE);

/* Mission Pack */
#ifndef DEMOBUILD
	WGT_PORTALS = gtk_check_button_new_with_label ("Praevus");
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_PORTALS, 112, 60);
	gtk_widget_show (WGT_PORTALS);
	gtk_tooltips_set_tip (tooltips, WGT_PORTALS, _("play Hexen II with Mission Pack"), NULL);
	GTKUI_DISABLE_FOCUS (WGT_PORTALS);
	gtk_widget_set_size_request (WGT_PORTALS, 80, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_PORTALS), mp_support);
	if (destiny != DEST_H2 || !(gameflags & GAME_PORTALS && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
#endif	/* DEMOBUILD */

/*********************************************************************/

/* Video Options */
	TxtVideo = gtk_label_new (_("Graphics    :"));
	gtk_widget_show (TxtVideo);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtVideo, 14, 128);
	gtk_label_set_justify (GTK_LABEL(TxtVideo), GTK_JUSTIFY_LEFT);

/* opengl/software toggle */
	WGT_OPENGL = gtk_check_button_new_with_label ("OpenGL");
	GTKUI_DISABLE_FOCUS (WGT_OPENGL);
	gtk_widget_show (WGT_OPENGL);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_OPENGL, 102, 124);
	gtk_widget_set_size_request (WGT_OPENGL, 110, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_OPENGL), opengl_support);

/* fullscreen/windowed toggle */
	WGT_FULLSCR = gtk_check_button_new_with_label (_("Fullscreen"));
	GTKUI_DISABLE_FOCUS (WGT_FULLSCR);
	gtk_widget_show (WGT_FULLSCR);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_FULLSCR, 102, 148);
	gtk_widget_set_size_request (WGT_FULLSCR, 110, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_FULLSCR), fullscreen);

/* resolution */
	TxtResol = gtk_label_new (_("Resolution  :"));
	gtk_widget_show (TxtResol);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtResol, 14, 180);
	gtk_label_set_justify (GTK_LABEL(TxtResol), GTK_JUSTIFY_LEFT);

/* resolution menu */
	WGT_RESMENU = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_RESMENU, 110, 24);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_RESMENU, 102, 176);
	/* menu listing for resolution come from a callback */
	Make_ResMenu ();
	gtk_widget_show (WGT_RESMENU);

/*********************************************************************/

/* Sound options (basic: driver selection) */
	TxtSound = gtk_label_new (_("Sound        :"));
	gtk_widget_show (TxtSound);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtSound, 14, 212);
	gtk_label_set_justify (GTK_LABEL(TxtSound), GTK_JUSTIFY_LEFT);

	WGT_SOUND = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_SOUND, 110, 24);
	GTKUI_OPTMENUS_BEGIN(WGT_SOUND);
	for (i = 0; snd_drivers[i].id != INT_MIN; i++)
	{
		GTKUI_OPTMENUS_ADD(WGT_SOUND, snd_drivers[i].name);
	}
	GTKUI_OPTMENUS_FINISH(WGT_SOUND);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_SOUND, 102, 208);
	gtk_widget_show (WGT_SOUND);
	for (i = 0; snd_drivers[i].id != INT_MIN; i++)
	{
		if (sound == snd_drivers[i].id)
		{
			GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_SOUND), i);
			break;
		}
	}

/* Sampling rate selection */
	TxtSound2 = gtk_label_new (_("Sample Rate:"));
	gtk_widget_show (TxtSound2);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtSound2, 14, 242);
	gtk_label_set_justify (GTK_LABEL(TxtSound2), GTK_JUSTIFY_LEFT);

	WGT_SRATE = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_SRATE, 110, 24);
	GTKUI_OPTMENUS_BEGIN(WGT_SRATE);
	for (i = 0; i < MAX_RATES; i++)
	{
		GTKUI_OPTMENUS_ADD(WGT_SRATE, snd_rates[i]);
	}
	GTKUI_OPTMENUS_FINISH(WGT_SRATE);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_SRATE, 102, 238);
	gtk_widget_show (WGT_SRATE);
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_SRATE), sndrate);

/********************************************************************
 TAB - 2:		ADDITIONAL OPTIONS
 ********************************************************************/

	MORE_LESS = gtk_button_new_with_label (_("More >>>"));
	gtk_widget_show (MORE_LESS);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), MORE_LESS, 132, 272);
	gtk_widget_set_size_request (MORE_LESS, 80, 24);

/*********************************************************************/

/* Additional game-types */
	TxtGameT = gtk_label_new (_("Extra Game Types:"));
	gtk_widget_show (TxtGameT);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), TxtGameT, 14, 16);
	gtk_label_set_justify (GTK_LABEL(TxtGameT), GTK_JUSTIFY_LEFT);

/* game types menu for hexen2 */
	WGT_H2GAME = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_H2GAME, 172, 32);
#ifndef DEMOBUILD
	game_index = menu_index = 0;
	GTKUI_OPTMENUS_BEGIN(WGT_H2GAME);
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (h2game_names[i].available)
		{
			GTKUI_OPTMENUS_ADD(WGT_H2GAME, h2game_names[i].name);
			h2game_names[i].menu_index = menu_index;
			if (i == h2game)
				game_index = menu_index;
			menu_index++;
		}
		else	h2game_names[i].menu_index = -1;
	}
	GTKUI_OPTMENUS_FINISH(WGT_H2GAME);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_H2GAME, 36, 36);
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_H2GAME), game_index);
	if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_H2GAME, FALSE);
	if (destiny == DEST_H2)
		gtk_widget_show (WGT_H2GAME);
#else
	GTKUI_OPTMENUS_BEGIN(WGT_H2GAME);
	GTKUI_OPTMENUS_ADD(WGT_H2GAME, "(  None  )");
	GTKUI_OPTMENUS_FINISH(WGT_H2GAME);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_H2GAME, 36, 36);
	gtk_widget_show (WGT_H2GAME);
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_H2GAME), 0);
	gtk_widget_set_sensitive (WGT_H2GAME, FALSE);
#endif

#ifndef DEMOBUILD
/* game types menu for hexenworld */
	WGT_HWGAME = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_HWGAME, 172, 32);
	game_index = menu_index = 0;
	GTKUI_OPTMENUS_BEGIN(WGT_HWGAME);
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (hwgame_names[i].available)
		{
			GTKUI_OPTMENUS_ADD(WGT_HWGAME,hwgame_names[i].name);
			hwgame_names[i].menu_index = menu_index;
			if (i == hwgame)
				game_index = menu_index;
			menu_index++;
		}
		else	hwgame_names[i].menu_index = -1;
	}
	GTKUI_OPTMENUS_FINISH(WGT_HWGAME);
/*	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_HWGAME, 68, 66);*/
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_HWGAME, 36, 36);
	GTKUI_SET_OPTMENU_IDX (GTKUI_OPTMENU_CAST(WGT_HWGAME), game_index);
	if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_HWGAME, FALSE);
	if (destiny == DEST_HW)
		gtk_widget_show (WGT_HWGAME);
	/*
	if (!(gameflags & GAME_HEXENWORLD))
		gtk_widget_set_sensitive (WGT_HWGAME, FALSE);
	*/
#endif	/* DEMOBUILD */

/*********************************************************************/

	hseparator0 = gtk_hseparator_new ();
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), hseparator0, 14, 72);
	gtk_widget_set_size_request (hseparator0, 200, 8);
	gtk_widget_show (hseparator0);

/*********************************************************************/

/* Additional OpenGL options */
	TxtVidExt = gtk_label_new (_("Additional OpenGL Options:"));
	gtk_widget_show (TxtVidExt);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), TxtVidExt, 14, 90);
	gtk_label_set_justify (GTK_LABEL(TxtVidExt), GTK_JUSTIFY_LEFT);

/* 3dfx Voodoo1/2/Rush support */
	WGT_3DFX = gtk_check_button_new_with_label (_("3dfx gamma support"));
	gtk_widget_show (WGT_3DFX);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_3DFX, 14, 112);
	gtk_widget_set_size_request (WGT_3DFX, 160, 24);
	gtk_widget_set_sensitive (WGT_3DFX, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_3DFX), fxgamma);
	GTKUI_DISABLE_FOCUS (WGT_3DFX);
	gtk_tooltips_set_tip (tooltips, WGT_3DFX, _("Special brightness support for old Voodoo1/2 boards"), NULL);

/* Whether to use GL_LUMINANCE lightmap format (the -lm_1 and -lm_4 switches) */
	WGT_LM_BYTES = gtk_check_button_new_with_label (_("Use old lightmap format"));
	gtk_widget_show (WGT_LM_BYTES);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_LM_BYTES, 14, 134);
	gtk_widget_set_size_request (WGT_LM_BYTES, 180, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LM_BYTES), use_lm1);
	GTKUI_DISABLE_FOCUS (WGT_LM_BYTES);
	gtk_tooltips_set_tip (tooltips, WGT_LM_BYTES, _("GL_RGBA is default. Mark to use the old GL_LUMINANCE format"), NULL);
	gtk_widget_set_sensitive (WGT_LM_BYTES, opengl_support);

/* 8-bit GL Extensions (Paletted Textures) */
	WGT_GL8BIT = gtk_check_button_new_with_label (_("Enable 8-bit textures"));
	gtk_widget_show (WGT_GL8BIT);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_GL8BIT, 14, 158);
	gtk_widget_set_size_request (WGT_GL8BIT, 160, 24);
	gtk_widget_set_sensitive (WGT_GL8BIT, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_GL8BIT), is8bit);
	GTKUI_DISABLE_FOCUS (WGT_GL8BIT);
	gtk_tooltips_set_tip (tooltips, WGT_GL8BIT, _("Enable 8-bit OpenGL texture extensions"), NULL);

/* conwidth toggle button */
	WGT_CONWBUTTON = gtk_check_button_new_with_label (_("Conwidth :"));
	gtk_widget_show (WGT_CONWBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_CONWBUTTON, 14, 182);
	gtk_widget_set_size_request (WGT_CONWBUTTON, 84, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_CONWBUTTON), use_con);
	GTKUI_DISABLE_FOCUS (WGT_CONWBUTTON);
	gtk_tooltips_set_tip (tooltips, WGT_CONWBUTTON, _("Allow bigger/readable text and HUD in high resolutions. Smaller the number, bigger the text. 640 is recommended"), NULL);
	gtk_widget_set_sensitive (WGT_CONWBUTTON, opengl_support);

/* conwidth menu */
	WGT_CONWMENU = GTKUI_OPTMENU_NEW();
	gtk_widget_set_size_request (WGT_CONWMENU, 108, 24);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_CONWMENU, 100, 182);
	gtk_widget_set_sensitive (WGT_CONWMENU, opengl_support);
	/* menu listing for conwidth come from a callback */
	Make_ConWidthMenu();
	gtk_widget_show (WGT_CONWMENU);

/* Enable VSync */
	WGT_VSYNC = gtk_check_button_new_with_label (_("Enable VSync"));
	gtk_widget_show (WGT_VSYNC);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_VSYNC, 14, 206);
	gtk_widget_set_size_request (WGT_VSYNC, 160, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_VSYNC), vsync);
	GTKUI_DISABLE_FOCUS (WGT_VSYNC);
	gtk_tooltips_set_tip (tooltips, WGT_VSYNC, _("Synchronize with Monitor Refresh"), NULL);
	gtk_widget_set_sensitive (WGT_VSYNC, opengl_support);

/* Multisampling (artialiasing) */
	WGT_FSAA = gtk_check_button_new_with_label (_("FSAA (Antialiasing) :"));
	gtk_widget_show (WGT_FSAA);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_FSAA, 14, 230);
	gtk_widget_set_size_request (WGT_FSAA, 148, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_FSAA), use_fsaa);
	GTKUI_DISABLE_FOCUS (WGT_FSAA);
	gtk_tooltips_set_tip (tooltips, WGT_FSAA, _("Enable Antialiasing"), NULL);
	gtk_widget_set_sensitive (WGT_FSAA, opengl_support);

	WGT_ADJFSAA = (GtkAdjustment *) gtk_adjustment_new (aasamples, 0, 8, 2, 10, 0);
	WGT_AASAMPLES = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ADJFSAA), 2, 0);
	gtk_editable_set_editable (GTK_EDITABLE(WGT_AASAMPLES), FALSE);
	gtk_widget_show (WGT_AASAMPLES);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_AASAMPLES, 166, 230);
	gtk_widget_set_size_request (WGT_AASAMPLES, 40, 24);

/* Custom OpenGL Library */
	WGT_LIBGL = gtk_check_button_new_with_label (_("Use a different GL library:"));
	gtk_widget_show (WGT_LIBGL);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_LIBGL, 14, 256);
	gtk_widget_set_size_request (WGT_LIBGL, 180, 24);
	gtk_widget_set_sensitive (WGT_LIBGL, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LIBGL), gl_nonstd);
	GTKUI_DISABLE_FOCUS (WGT_LIBGL);
	gtk_tooltips_set_tip (tooltips, WGT_LIBGL, _("Type the full path of the OpenGL library"), NULL);

	WGT_GLPATH = gtk_entry_new ();
	gtk_widget_show (WGT_GLPATH);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_GLPATH, 36, 280);
	gtk_widget_set_size_request (WGT_GLPATH, 172, 24);
	gtk_entry_set_max_length (GTK_ENTRY(WGT_GLPATH), sizeof(gllibrary)-1);
	gtk_entry_set_text (GTK_ENTRY(WGT_GLPATH), gllibrary);
	/*
	if (!opengl_support || !gl_nonstd)
		gtk_widget_set_sensitive (WGT_GLPATH, FALSE);
	*/

/********************************************************************
 TAB - 3:		ADDITIONAL OPTIONS
 ********************************************************************/

/* Additional Sound options */
	TxtSndExt = gtk_label_new (_("Additional Options:"));
	gtk_widget_show (TxtSndExt);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtSndExt, 14, 16);
	gtk_label_set_justify (GTK_LABEL(TxtSndExt), GTK_JUSTIFY_LEFT);

/* 8/16-bit format toggle */
	TxtSound3 = gtk_label_new (_("Sound Format:"));
	gtk_widget_show (TxtSound3);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtSound3, 14, 44);
	gtk_label_set_justify (GTK_LABEL(TxtSound3), GTK_JUSTIFY_LEFT);

	WGT_SBITS = gtk_check_button_new_with_label (sndbits ? "16 bit" : " 8 bit");
	gtk_widget_show (WGT_SBITS);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_SBITS, 112, 40);
	gtk_widget_set_size_request (WGT_SBITS, 76, 24);
	gtk_widget_set_sensitive (WGT_SBITS, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_SBITS), sndbits);
	GTKUI_DISABLE_FOCUS (WGT_SBITS);

/*********************************************************************/

/* MUSIC (Additional Sound options-2) */

/* disabling cdaudio */
	WGT_CDAUDIO = gtk_check_button_new_with_label (_("No CDaudio"));
	gtk_widget_show (WGT_CDAUDIO);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_CDAUDIO, 112, 70);
	gtk_widget_set_size_request (WGT_CDAUDIO, 96, 24);
	gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_CDAUDIO), !cdaudio);
	GTKUI_DISABLE_FOCUS (WGT_CDAUDIO);
	gtk_tooltips_set_tip (tooltips, WGT_CDAUDIO, _("Mark to Disable CDAudio"), NULL);

/* disabling midi */
	WGT_MIDI = gtk_check_button_new_with_label (_("No MIDI"));
	gtk_widget_show (WGT_MIDI);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MIDI, 14, 70);
	gtk_widget_set_size_request (WGT_MIDI, 76, 24);
	gtk_widget_set_sensitive (WGT_MIDI, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MIDI), !midi);
	GTKUI_DISABLE_FOCUS (WGT_MIDI);
	gtk_tooltips_set_tip (tooltips, WGT_MIDI, _("Mark to Disable Midi"), NULL);

/*********************************************************************/

/* Network: disabling LAN */
	TxtNet = gtk_label_new (_("Networking :"));
	gtk_widget_show (TxtNet);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtNet, 14, 102);
	gtk_label_set_justify (GTK_LABEL(TxtNet), GTK_JUSTIFY_LEFT);

	WGT_LANBUTTON = gtk_check_button_new_with_label (_("Disable LAN"));
	gtk_widget_show (WGT_LANBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_LANBUTTON, 112, 98);
	gtk_widget_set_size_request (WGT_LANBUTTON, 108, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LANBUTTON), !lan);
	GTKUI_DISABLE_FOCUS (WGT_LANBUTTON);
	gtk_tooltips_set_tip (tooltips, WGT_LANBUTTON, _("Mark to Disable Net"), NULL);
	if (destiny == DEST_HW
#if !defined(DEMOBUILD)
		|| h2game_names[h2game].is_botmatch
#endif	/* DEMOBUILD */
	   )
		gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);

/*********************************************************************/

/* Mouse: disabling Mouse */
	TxtMouse = gtk_label_new (_("Mouse  :"));
	gtk_widget_show (TxtMouse);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtMouse, 14, 122);
	gtk_label_set_justify (GTK_LABEL(TxtMouse), GTK_JUSTIFY_LEFT);

	WGT_MOUSEBUTTON = gtk_check_button_new_with_label (_("Disable Mouse"));
	gtk_widget_show (WGT_MOUSEBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MOUSEBUTTON, 112, 118);
	gtk_widget_set_size_request (WGT_MOUSEBUTTON, 112, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MOUSEBUTTON), !mouse);
	GTKUI_DISABLE_FOCUS (WGT_MOUSEBUTTON);
	gtk_tooltips_set_tip (tooltips, WGT_MOUSEBUTTON, _("Mark to Disable Mouse"), NULL);

/*********************************************************************/

	hseparator1 = gtk_hseparator_new ();
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), hseparator1, 14, 144);
	gtk_widget_set_size_request (hseparator1, 200, 8);
	gtk_widget_show (hseparator1);

/*********************************************************************/

/* Advanced options */
	TxtAdv = gtk_label_new (_("Advanced :"));
	gtk_widget_show (TxtAdv);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtAdv, 14, 162);
	gtk_label_set_justify (GTK_LABEL(TxtAdv), GTK_JUSTIFY_LEFT);

/* Memory options (heapsize) */
	WGT_MEMHEAP = gtk_check_button_new_with_label (_("Heapsize (KB):"));
	gtk_widget_show (WGT_MEMHEAP);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MEMHEAP, 14, 182);
	gtk_widget_set_size_request (WGT_MEMHEAP, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MEMHEAP), use_heap);
	GTKUI_DISABLE_FOCUS (WGT_MEMHEAP);
	gtk_tooltips_set_tip (tooltips, WGT_MEMHEAP, _("The main memory to allocate in KB"), NULL);

	WGT_HEAPADJ = (GtkAdjustment *) gtk_adjustment_new (heapsize, HEAP_MINSIZE, HEAP_MAXSIZE, 1024, 10, 0);
	WGT_HEAPSIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_HEAPADJ), 1024, 0);
	gtk_editable_set_editable (GTK_EDITABLE(WGT_HEAPSIZE), FALSE);
	gtk_widget_show (WGT_HEAPSIZE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_HEAPSIZE, 134, 182);
	gtk_widget_set_size_request (WGT_HEAPSIZE, 64, 24);

/* Memory options (zonesize) */
	WGT_MEMZONE = gtk_check_button_new_with_label (_("Zonesize (KB):"));
	gtk_widget_show (WGT_MEMZONE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MEMZONE, 14, 208);
	gtk_widget_set_size_request (WGT_MEMZONE, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MEMZONE), use_zone);
	GTKUI_DISABLE_FOCUS (WGT_MEMZONE);
	gtk_tooltips_set_tip (tooltips, WGT_MEMZONE, _("Dynamic zone memory to allocate in KB"), NULL);

	WGT_ZONEADJ = (GtkAdjustment *) gtk_adjustment_new (zonesize, ZONE_MINSIZE, ZONE_MAXSIZE, 1, 10, 0);
	WGT_ZONESIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ZONEADJ), 1, 0);
	gtk_editable_set_editable (GTK_EDITABLE(WGT_ZONESIZE), FALSE);
	gtk_widget_show (WGT_ZONESIZE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_ZONESIZE, 134, 208);
	gtk_widget_set_size_request (WGT_ZONESIZE, 64, 24);

/* Debug output option */
	WGT_DBGLOG = gtk_check_button_new_with_label (_("Generate log"));
	gtk_widget_show (WGT_DBGLOG);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_DBGLOG, 14, 232);
	gtk_widget_set_size_request (WGT_DBGLOG, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_DBGLOG), debug);
	GTKUI_DISABLE_FOCUS (WGT_DBGLOG);
	gtk_tooltips_set_tip (tooltips, WGT_DBGLOG, _("Write console output to a log file"), NULL);

	WGT_DBGLOG2 = gtk_check_button_new_with_label (_("Full log"));
	gtk_widget_show (WGT_DBGLOG2);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_DBGLOG2, 134, 232);
	gtk_widget_set_size_request (WGT_DBGLOG2, 72, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_DBGLOG2), debug2);
	GTKUI_DISABLE_FOCUS (WGT_DBGLOG2);
	gtk_tooltips_set_tip (tooltips, WGT_DBGLOG2, _("Full developer mode logging"), NULL);

/* Extra User Arguments: */
	WGT_EXTBTN = gtk_check_button_new_with_label (_("Use extra arguments:"));
	gtk_widget_show (WGT_EXTBTN);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_EXTBTN, 14, 256);
	gtk_widget_set_size_request (WGT_EXTBTN, 180, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_EXTBTN), use_extra);
	GTKUI_DISABLE_FOCUS (WGT_EXTBTN);
	gtk_tooltips_set_tip (tooltips, WGT_EXTBTN, _("Type the extra arguments you want to pass to the command line"), NULL);

	WGT_EXTARGS = gtk_entry_new ();
	gtk_widget_show (WGT_EXTARGS);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_EXTARGS, 36, 280);
	gtk_widget_set_size_request (WGT_EXTARGS, 172, 24);
	gtk_entry_set_max_length (GTK_ENTRY(WGT_EXTARGS), sizeof(ext_args)-1);
	gtk_entry_set_text (GTK_ENTRY(WGT_EXTARGS), ext_args);

/***	END OF OPTION WIDGETS	***/

/* callback functions setup */
	GTKUI_SIGNAL_CONNECT (MAIN_WINDOW, "destroy", ui_quit, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_LAUNCH, "clicked", launch_hexen2_bin, NULL);
	GTKUI_SIGNAL_CONNECT (bQUIT, "clicked", ui_quit, NULL);
#ifndef DEMOBUILD
	GTKUI_SIGNAL_CONNECT (WGT_PORTALS, "toggled", BoolRevert, &mp_support);
#endif
	GTKUI_SIGNAL_CONNECT (bPATCH, "clicked", create_window2, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_HEXEN2, "clicked", on_HEXEN2, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_H2WORLD, "clicked", on_H2W, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_OPENGL, "toggled", on_OGL, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_SBITS, "toggled", on_SBITS, &sndbits);
	GTKUI_SIGNAL_CONNECT (WGT_MIDI, "toggled", BoolRevert, &midi);
	GTKUI_SIGNAL_CONNECT (WGT_CDAUDIO, "toggled", BoolRevert, &cdaudio);
	GTKUI_SIGNAL_CONNECT (WGT_LANBUTTON, "toggled", BoolRevert, &lan);
	GTKUI_SIGNAL_CONNECT (WGT_FULLSCR, "toggled", BoolRevert, &fullscreen);
	GTKUI_SIGNAL_CONNECT (WGT_CONWBUTTON, "toggled", BoolRevert, &use_con);
	GTKUI_SIGNAL_CONNECT (WGT_3DFX, "toggled", BoolRevert, &fxgamma);
	GTKUI_SIGNAL_CONNECT (WGT_GL8BIT, "toggled", BoolRevert, &is8bit);
	GTKUI_SIGNAL_CONNECT (WGT_LM_BYTES, "toggled", BoolRevert, &use_lm1);
	GTKUI_SIGNAL_CONNECT (WGT_VSYNC, "toggled", BoolRevert, &vsync);
	GTKUI_SIGNAL_CONNECT (WGT_FSAA, "toggled", BoolRevert, &use_fsaa);
	GTKUI_SIGNAL_CONNECT (WGT_ADJFSAA, "value_changed", adj_Change, &aasamples);
	GTKUI_SIGNAL_CONNECT (WGT_LIBGL, "toggled", BoolRevert, &gl_nonstd);
	GTKUI_SIGNAL_CONNECT (WGT_DBGLOG, "toggled", BoolRevert, &debug);
	GTKUI_SIGNAL_CONNECT (WGT_DBGLOG2, "toggled", BoolRevert, &debug2);
	GTKUI_SIGNAL_CONNECT (WGT_MEMHEAP, "toggled", BoolRevert, &use_heap);
	GTKUI_SIGNAL_CONNECT (WGT_MEMZONE, "toggled", BoolRevert, &use_zone);
	GTKUI_SIGNAL_CONNECT (WGT_EXTBTN, "toggled", BoolRevert, &use_extra);
	GTKUI_SIGNAL_CONNECT (WGT_GLPATH, "changed", libgl_Change, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_EXTARGS, "changed", extargs_Change, NULL);
	GTKUI_SIGNAL_CONNECT (WGT_HEAPADJ, "value_changed", adj_Change, &heapsize);
	GTKUI_SIGNAL_CONNECT (WGT_ZONEADJ, "value_changed", adj_Change, &zonesize);
	GTKUI_SIGNAL_CONNECT (MORE_LESS, "clicked", on_MORE, NULL);
	gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_SOUND), GTKUI_SIGNALFUNC(on_SND));
	gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_SRATE), GTKUI_SIGNALFUNC(on_SRATE));
#	ifndef DEMOBUILD
	h2game_handler = gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_H2GAME), GTKUI_SIGNALFUNC(H2GameChange));
	hwgame_handler = gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_HWGAME), GTKUI_SIGNALFUNC(HWGameChange));
#	endif
	reslist_handler = gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_RESMENU), GTKUI_SIGNALFUNC(res_Change));
	conwlist_handler = gtkui_set_optmenu_handler (GTKUI_OPTMENU_CAST(WGT_CONWMENU), GTKUI_SIGNALFUNC(con_Change));

/* show the window */
	gtk_widget_show (MAIN_WINDOW);
}


/*********************************************************************/
/* EXPOSED UI FUNCTIONS: */

static gint ui_start (gpointer user_data)
{
/* create the main window */
	create_window1();

/* update the launch button status */
	UpdateStats();

/* if we have a bad installation (such as un-patched versions
 * of gamedata), pop up the patch window and print a report: */
	if (gameflags & GAME_INSTBAD)
	{
		/*
		on_MORE (GTK_BUTTON(MORE_LESS), NULL);
		ui_pump ();
		*/
		create_window2 (NULL, NULL);
	}

	return 0;
}

int ui_init (int *argc, char ***argv)
{
#ifdef ENABLE_NLS
	/*
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
	*/
#endif
/*	gtk_set_locale ();	*/

	gtk_init (argc, argv);

	log_queue = (LogQueue_t *) g_malloc(sizeof(LogQueue_t));
	log_queue->data = NULL;
	log_queue->next = NULL;

	return 0;
}

int ui_main (void)
{
	ui_start (NULL);
	gtk_main ();

	return 0;
}

void ui_quit (void)
{
	if (gtk_main_level ())
		gtk_main_quit ();
}

void ui_pump (void)
{
	while (gtk_events_pending ())
		gtk_main_iteration ();
}

#if (GTK_MAJOR_VERSION > 1)
/* gtk-2.x version: */
void ui_error (const char *msg)
{
	GtkWidget	*dialog;

	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
					 "%s", msg);
	gtk_window_set_title (GTK_WINDOW(dialog), "Hexen II Launcher: Error");
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}
#else
/* gtk-1.2.x version, taken from loki_setup tools: */
static gboolean	user_responded;
static void prompt_button_slot (GtkWidget* widget, gpointer func_data)
{
	user_responded = TRUE;
}

void ui_error (const char *msg)
{
	GtkWidget	*dialog;
	GtkWidget	*label, *ok_button;

	dialog = gtk_dialog_new();
	label = gtk_label_new (NULL);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	gtk_label_set_text (GTK_LABEL(label), msg);
	ok_button = gtk_button_new_with_label("Close");

	/* Ensure that the dialog box is destroyed when the user clicks ok. */
	GTKUI_SIGNAL_CONNECT_OBJECT (ok_button, "clicked", prompt_button_slot, dialog);
	GTKUI_SIGNAL_CONNECT_OBJECT (dialog, "delete-event", prompt_button_slot, dialog);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), ok_button);

	/* Add the label, and show everything we've added to the dialog. */
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_window_set_title (GTK_WINDOW(dialog), "Hexen II Launcher: Error");
	gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_widget_show_all (dialog);

	user_responded = FALSE;

	while (user_responded != TRUE)
	{
		usleep (10000);
		gtk_main_iteration ();
	}

	gtk_widget_destroy(dialog);
}
#endif	/* GTK_MAJOR_VERSION */

