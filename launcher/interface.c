/*
	interface.c
	hexen2 launcher gtk+ interface

	$Id: interface.c,v 1.60 2007-08-13 06:43:15 sezero Exp $

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

#include "launcher_defs.h"
#include "common.h"
#include <gtk/gtk.h>
#include "compat_gtk1.h"
#include "support.h"
#if !defined(DEMOBUILD)
#include <pthread.h>
#include "apply_patch.h"
#endif	/* !DEMOBUILD */
#include "widget_defs.h"
#include "interface.h"
#include "config_file.h"
#include "games.h"

/*********************************************************************/
// Macros:

#undef	GTK_DESTROYNOTIFY
#define	GTK_DESTROYNOTIFY(f)	(GtkDestroyNotify)f

#undef	GTK_INIT_FUNC
#define	GTK_INIT_FUNC(f)	(GtkFunction)f

/*********************************************************************/
// Extern data:

// from launch_bin.c
extern char	*snddrv_names[MAX_SOUND][2];
extern char	*snd_rates[MAX_RATES];

/*********************************************************************/
// Public data:

int			thread_alive;

/*********************************************************************/
// Private data:

static GtkTooltips	*tooltips;
static GtkWidget	*H2G_Entry;	// Hexen2 games listing
#ifndef DEMOBUILD
static GtkWidget	*HWG_Entry;	// Hexenworld games listing
#endif	/* DEMOBUILD */

static options_widget_t	Options;
static MainWindow_t	main_win;
static PatchWindow_t	patch_win;
static int	bmore = 0, lock = 0;

static char *res_names[RES_MAX] =
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

static char *launch_status[] =
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
 CALLBACK FUNCTIONS
 *********************************************************************/

static int CheckStats (void)
{
// return broken installations
#if !defined(DEMOBUILD)
	if (gameflags & (GAME_INSTBAD1|GAME_INSTBAD2|GAME_INSTBAD3))
		return 2;
	if (gameflags & GAME_INSTBAD0)
		return 3;
#else
	if (gameflags & GAME_INSTBAD)
		return 2;
#endif	/* DEMOBUILD */
// return binary availability
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

void Log_printf (const char *fmt, ...) __attribute__((format(printf,1,2)));

#if !defined(DEMOBUILD)
static char *patch_status[] =
{
	"  Patch in progress.....",
	"  Patch process finished",
	"  Patch process failed"
};
#endif	/* !DEMOBUILD */

#if defined(_H2L_USE_GTK1)
static GtkText *LogEntry = NULL;

static void ui_LogInit (GtkWidget *wgt)
{
	LogEntry = GTK_TEXT(wgt);
	// clear the window
	gtk_editable_delete_text (GTK_EDITABLE(wgt), 0, -1);
}

static void ui_LogPrint (const char *txt)
{
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
	// clear the window
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

void Log_printf (const char *fmt, ...)
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

static void report_status (GtkObject *Unused, PatchWindow_t *PatchWindow)
{
	int	end_log = 0;

	if (!LogEntry)
	{
		end_log = 1;
		ui_LogInit (PatchWindow->LOGVIEW);
	}

	Log_printf ("Installation Summary:\n\n");
	Log_printf ("Base directory: %s\n", basedir);
	Log_printf ("Data directory: %s\n", (basedir_nonstd && game_basedir[0]) ? game_basedir : basedir);
	Log_printf ("PAK file health: %s", (gameflags & GAME_INSTBAD) ? "BAD. Reason(s):\n" : "OK ");
	if (gameflags & GAME_INSTBAD)
	{
		if (gameflags & GAME_INSTBAD3)
			Log_printf ("- " "Found an old, unsupported version of the demo.\n");
		if (gameflags & GAME_INSTBAD2)
			Log_printf ("- " "Found mixed data from incompatible versions.\n");
		if (gameflags & GAME_INSTBAD1)
			Log_printf ("- " "Neither of retail, demo or oem versions found.\n");
		if (gameflags & GAME_INSTBAD0)
			Log_printf ("- " "Found pak files not patched to 1.11 version.\n");
		if (gameflags & GAME_CANPATCH)
			Log_printf ("- " "Applying the 1.11 pak patch should solve this.\n");
	}
	else
	{
		Log_printf ("(%s version.)\n", (gameflags & (GAME_DEMO|GAME_OLD_DEMO)) ? "demo" : ((gameflags & GAME_OEM) ? "oem" : "retail"));
		if (gameflags & (GAME_REGISTERED_OLD|GAME_OLD_DEMO))
			Log_printf ("Using old/unsupported 1.03 version pak files.\n");
		if (gameflags & GAME_CANPATCH)
			Log_printf ("Applying Raven's 1.11 pak patch is suggested.\n");
	}

	Log_printf ("Mission Pack: %s", (gameflags & GAME_PORTALS) ? "present " : "not found");
	if (gameflags & GAME_PORTALS)
		Log_printf ("%s", (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)) ? "\n" : "(ignored: no valid retail data)\n");
	else
		Log_printf ("\n");

	Log_printf ("HexenWorld: %s\n", (gameflags & GAME_HEXENWORLD) ? "present " : "not found");

#if defined(DEMOBUILD)
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_CANPATCH))
	{
		Log_printf ("---------------------\n");
		Log_printf ("This is a restricted build of Hexen II Launcher\n");
		Log_printf ("for the demo version. Use the normal builds for\n");
		Log_printf ("retail-only functionality and pakfile patching.\n");
	}
#endif	/* DEMOBUILD */

	if (end_log)
		ui_LogEnd ();
}

#if !defined(DEMOBUILD)
static gboolean block_window_close (GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	return TRUE;
}

static void start_xpatch (GtkObject *Unused, PatchWindow_t *PatchWindow)
{
	pthread_t		thr;
	char		*wd;
	long		*ptr;
	int		ret = 2;

	if (basedir_nonstd && game_basedir[0])
		wd = game_basedir;
	else
		wd = basedir;

	gtk_widget_set_sensitive (PatchWindow->bAPPLY, FALSE);
	gtk_widget_set_sensitive (PatchWindow->bBASEDIR, FALSE);
	gtk_widget_set_sensitive (PatchWindow->dir_Entry, FALSE);
	gtk_widget_set_sensitive (PatchWindow->bCLOSE, FALSE);
	gtk_widget_set_sensitive (PatchWindow->bREPORT, FALSE);
	PatchWindow->delete_handler =
		gtk_signal_connect(GTK_OBJECT(PatchWindow->mywindow), "delete-event", GTK_SIGNAL_FUNC(block_window_close), NULL);
	ui_LogInit (PatchWindow->LOGVIEW);

	thread_alive = 1;
	if (pthread_create(&thr, NULL, apply_patches, wd) != 0)
	{
		Log_printf ("pthread_create failed");
		goto finish;
	}

	gtk_statusbar_push (GTK_STATUSBAR(PatchWindow->StatusBar), PatchWindow->statbar_id, patch_status[0]);

	while (thread_alive)
	{
		ui_pump ();
		usleep (10000);
	}

	if (pthread_join(thr, (void **) (char *) &ptr) != 0)
	{
		Log_printf ("pthread_join failed");
		goto finish;
	}

	ret = (*ptr & XPATCH_FAIL) ? 2 : 1;
	if (*ptr & XPATCH_APPLIED)
	{
		scan_game_installation();
		UpdateStats ();
		Log_printf ("---------------------\n");
		report_status (NULL, PatchWindow);
		if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
		{	// activate the extra game options, if necessary
			gtk_widget_set_sensitive (WGT_H2GAME, TRUE);
			gtk_widget_set_sensitive (WGT_HWGAME, TRUE);
			if (gameflags & GAME_PORTALS && destiny == DEST_H2)
				gtk_widget_set_sensitive (WGT_PORTALS, TRUE);
		}
	}
finish:
	gtk_signal_disconnect (GTK_OBJECT(PatchWindow->mywindow), PatchWindow->delete_handler);
	gtk_widget_set_sensitive (PatchWindow->bCLOSE, TRUE);
	gtk_widget_set_sensitive (PatchWindow->bAPPLY, TRUE);
	gtk_widget_set_sensitive (PatchWindow->bBASEDIR, TRUE);
	gtk_widget_set_sensitive (PatchWindow->dir_Entry, TRUE);
	gtk_widget_set_sensitive (PatchWindow->bREPORT, TRUE);
	gtk_statusbar_push (GTK_STATUSBAR(PatchWindow->StatusBar), PatchWindow->statbar_id, patch_status[ret]);
	ui_LogEnd();
}
#endif	/* ! DEMOBUILD */

static void on_SND (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);

	for (i = 0; i < MAX_SOUND; i++)
	{
		if (strcmp(tmp, snddrv_names[i][1]) == 0)
		{
			g_free(tmp);
			sound = i;
			gtk_widget_set_sensitive (WGT_MIDI, sound);
			gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
			gtk_widget_set_sensitive (WGT_SRATE, sound);
			gtk_widget_set_sensitive (WGT_SBITS, sound);
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

static void on_SRATE (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);

	for (i = 0; i < MAX_RATES; i++)
	{
		if (strcmp(tmp, snd_rates[i]) == 0)
		{
			g_free(tmp);
			sndrate = i;
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

static void on_SBITS (GtkButton *button, int *opt)
{
	*opt ^= 1;
	gtk_button_set_label(button, (*opt) ? "16 bit" : " 8 bit");
}

static void Make_ResMenu (void)
{
	int	i, up;
	GList *ResList = NULL;

	up = (opengl_support) ? RES_MAX-1 : RES_640;
	for (i = 2*opengl_support; i <= up; i++)
		ResList = g_list_append (ResList, res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_RESCOMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY(WGT_RESLIST), res_names[resolution]);
}

static void Make_ConWidthMenu (void)
{
	int	i;
	GList *ResList = NULL;

	for (i = 0; i <= resolution; i++)
		ResList = g_list_append (ResList, res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_CONWCOMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY(WGT_CONWLIST), res_names[conwidth]);
}

static void on_OGL (GtkToggleButton *button, gpointer user_data)
{
//	Make_ResMenu() triggers "changed" signal
//	for RES_LIST, therefore prevent the fight
	lock = 1;

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
	gtk_widget_set_sensitive (WGT_CONWCOMBO, opengl_support);
	Make_ResMenu ();
	if (opengl_support)
		Make_ConWidthMenu();
	UpdateStats ();
	lock = 0;
}

static void res_Change (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar	*tmp;

	if (lock)
		return;

	lock = 1;
	tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < RES_MAX; i++)
	{
		if (strcmp(tmp, res_names[i]) == 0)
			resolution = i;
	}
	g_free(tmp);
	if (opengl_support)
	{
		if (conwidth > resolution)
			conwidth = resolution;
		Make_ConWidthMenu ();
	}
	lock = 0;
}

static void con_Change (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar	*tmp;

	if (lock)
		return;

	tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < RES_MAX; i++)
	{
		if (strcmp(tmp, res_names[i]) == 0)
		{
			g_free(tmp);
			conwidth = i;
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

static void libgl_Change (GtkEditable *editable, gpointer user_data)
{
	size_t len;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	len = strlen(tmp);
	if (len > sizeof(gllibrary)-1)
		len = sizeof(gllibrary)-1;
	if (len)
		memcpy (gllibrary, tmp, len);
	g_free (tmp);
	gllibrary[len] = 0;
}

static void extargs_Change (GtkEditable *editable, gpointer user_data)
{
	size_t len;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	len = strlen(tmp);
	if (len > sizeof(ext_args)-1)
		len = sizeof(ext_args)-1;
	if (len)
		memcpy (ext_args, tmp, len);
	g_free (tmp);
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
static void H2GameChange (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (strcmp(tmp, h2game_names[i].name) == 0)
		{
			g_free(tmp);
			h2game = i;
			gtk_widget_set_sensitive (WGT_LANBUTTON, !h2game_names[i].is_botmatch);
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

static void HWGameChange (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (strcmp(tmp, hwgame_names[i].name) == 0)
		{
			g_free(tmp);
			hwgame = i;
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}
#endif	/* ! DEMOBUILD */

static void BoolRevert (GtkObject *Unused, int *opt)
{
	*opt ^= 1;
}

static void adj_Change (GtkAdjustment *adj, int *opt)
{
	*opt = (int)(adj->value);
//	printf ("Value is: %d\n", *opt);
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
#if !defined(DEMOBUILD)
	int		i;
	GList *TmpList = NULL;
#endif	/* ! DEMOBUILD */

	if (lock)
		return;

	lock = 1;
	basedir_nonstd ^= 1;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patch_win.bBASEDIR), basedir_nonstd);
	scan_game_installation();
	UpdateStats ();
	report_status (NULL, &patch_win);

// activate the extra game options, if necessary
	gtk_widget_set_sensitive (WGT_H2WORLD, (gameflags & GAME_HEXENWORLD) ? TRUE : FALSE);
// rebuild the game mod lists
#if !defined(DEMOBUILD)
	if (mp_support)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_PORTALS), FALSE);
	h2game = hwgame = mp_support = 0;
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (h2game_names[i].available)
			TmpList = g_list_append (TmpList, h2game_names[i].name);
	}
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_H2GAME), TmpList);
	g_list_free (TmpList);
	TmpList = NULL;
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (hwgame_names[i].available)
			TmpList = g_list_append (TmpList, hwgame_names[i].name);
	}
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_HWGAME), TmpList);
	g_list_free (TmpList);
	gtk_entry_set_text (GTK_ENTRY(H2G_Entry), h2game_names[0].name);
	gtk_entry_set_text (GTK_ENTRY(HWG_Entry), hwgame_names[0].name);
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
#endif	/* ! DEMOBUILD */

	lock = 0;
}

static void basedir_ChangePath (GtkEditable *editable, gpointer user_data)
{
	size_t len;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	len = strlen(tmp);
	if (len > sizeof(game_basedir)-1)
		len = sizeof(game_basedir)-1;
	if (len)
		memcpy (game_basedir, tmp, len);
	game_basedir[len] = 0;
	g_free (tmp);

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
	GtkWidget *Txt1;	// Window label
	GtkWidget *TxtWindow;	// Holder Window for the textview
#if !defined(_H2L_USE_GTK1)
	GtkTextBuffer	*buf;
	GtkTextIter	start;
	GtkTextMark	*mark;
#endif	/* _H2L_USE_GTK1 */

	PATCH_WINDOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data (GTK_OBJECT(PATCH_WINDOW), "mywindow2", PATCH_WINDOW);
	gtk_window_set_title (GTK_WINDOW(PATCH_WINDOW), "Hexen II PAK patch");
	gtk_window_set_resizable (GTK_WINDOW(PATCH_WINDOW), FALSE);
	gtk_window_set_modal (GTK_WINDOW(PATCH_WINDOW), TRUE);
	gtk_widget_set_size_request(PATCH_WINDOW, 360, 272);

	PATCH_TAB = gtk_fixed_new ();
	gtk_widget_ref (PATCH_TAB);
	gtk_container_add (GTK_CONTAINER (PATCH_WINDOW), PATCH_TAB);
	gtk_widget_show (PATCH_TAB);

//	Txt1 = gtk_label_new ("Hexen II PAK files 1.11 patch");
	Txt1 = gtk_label_new ("Hexen II Installation status");
	gtk_widget_ref (Txt1);
	gtk_widget_show (Txt1);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), Txt1, 14, 12);
	gtk_label_set_justify (GTK_LABEL(Txt1), GTK_JUSTIFY_LEFT);

// custom basedir entry:
	patch_win.bBASEDIR = gtk_check_button_new_with_label (_("Data install path:"));
	gtk_widget_ref (patch_win.bBASEDIR);
	gtk_widget_show (patch_win.bBASEDIR);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bBASEDIR, 14, 186);
	gtk_widget_set_size_request (patch_win.bBASEDIR, 128, 24);
	gtk_widget_set_sensitive (patch_win.bBASEDIR, TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patch_win.bBASEDIR), basedir_nonstd);
	GTK_WIDGET_UNSET_FLAGS (patch_win.bBASEDIR, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, patch_win.bBASEDIR, _("Mark this in order to use a different game installation directory"), NULL);

	patch_win.dir_Entry = gtk_entry_new();
	gtk_widget_show (patch_win.dir_Entry);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.dir_Entry, 148, 186);
	gtk_widget_set_size_request (patch_win.dir_Entry, 190, 24);
	gtk_entry_set_max_length (GTK_ENTRY(patch_win.dir_Entry), sizeof(game_basedir)-1);
	gtk_entry_set_text (GTK_ENTRY(patch_win.dir_Entry), game_basedir);
	gtk_widget_ref (patch_win.dir_Entry);

// Apply Patch button
	patch_win.bAPPLY = gtk_button_new_with_label (_("Apply Pak Patch"));
	gtk_widget_ref (patch_win.bAPPLY);
	gtk_widget_show (patch_win.bAPPLY);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bAPPLY, 14, 218);
	gtk_widget_set_size_request (patch_win.bAPPLY, 112, 24);
#if !defined(DEMOBUILD)
	gtk_tooltips_set_tip (tooltips, patch_win.bAPPLY, _("Apply the v1.11 pakfiles patch by Raven Software."), NULL);
#endif	/* ! DEMOBUILD */

// Installation status button
	patch_win.bREPORT = gtk_button_new_with_label (_("Make Report"));
	gtk_widget_ref (patch_win.bREPORT);
	gtk_widget_show (patch_win.bREPORT);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bREPORT, 132, 218);
	gtk_widget_set_size_request (patch_win.bREPORT, 112, 24);

// Holder window for the textview
	TxtWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (TxtWindow);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), TxtWindow, 14, 32);
	gtk_widget_set_size_request (TxtWindow, 324, 146);
	gtk_widget_show (TxtWindow);
//	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(TxtWindow), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(TxtWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(TxtWindow), GTK_SHADOW_ETCHED_IN);

// The textview:
#if defined(_H2L_USE_GTK1)
	patch_win.LOGVIEW = gtk_text_new (NULL, NULL);
#else	/* GTK2 */
	patch_win.LOGVIEW = gtk_text_view_new ();
#endif	/* _H2L_USE_GTK2 */
	gtk_widget_ref (patch_win.LOGVIEW);
	gtk_widget_set_size_request (patch_win.LOGVIEW, 324, 146);
	gtk_container_add (GTK_CONTAINER (TxtWindow), patch_win.LOGVIEW);
	gtk_widget_show (patch_win.LOGVIEW);
	GTK_WIDGET_UNSET_FLAGS (patch_win.LOGVIEW, GTK_CAN_FOCUS);
//	gtk_widget_set_sensitive (patch_win.LOGVIEW, FALSE);
#if defined(_H2L_USE_GTK1)
//	gtk_text_set_line_wrap(GTK_TEXT(patch_win.LOGVIEW), FALSE);
	gtk_text_set_editable (GTK_TEXT(patch_win.LOGVIEW), FALSE);
#else	/* GTK2 */
	gtk_text_view_set_editable (GTK_TEXT_VIEW(patch_win.LOGVIEW), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(patch_win.LOGVIEW), FALSE);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW(patch_win.LOGVIEW), 2);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW(patch_win.LOGVIEW), 2);
	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(patch_win.LOGVIEW));
	gtk_text_buffer_get_start_iter (buf, &start);
	mark = gtk_text_buffer_create_mark (buf, "ins_mark", &start, 0);
#endif	/* _H2L_USE_GTK2 */

// Close button
	patch_win.bCLOSE = gtk_button_new_with_label (_("Close"));
	gtk_widget_ref (patch_win.bCLOSE);
	gtk_widget_show (patch_win.bCLOSE);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), patch_win.bCLOSE, 250, 218);
	gtk_widget_set_size_request (patch_win.bCLOSE, 88, 24);

// Statusbar
	PATCH_STATBAR = gtk_statusbar_new ();
	gtk_widget_ref (PATCH_STATBAR);
	gtk_widget_show (PATCH_STATBAR);
	gtk_fixed_put (GTK_FIXED(PATCH_TAB), PATCH_STATBAR, 0, 246);
	gtk_widget_set_size_request (PATCH_STATBAR, 354, 24);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(PATCH_STATBAR), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER(PATCH_STATBAR), 2);
	patch_win.statbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(PATCH_STATBAR), "PatchStatus");
	gtk_statusbar_push (GTK_STATUSBAR(PATCH_STATBAR), patch_win.statbar_id, _("  Ready..."));

	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "fixed1", PATCH_TAB, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "Txt1", Txt1, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "bAPPLY", patch_win.bAPPLY, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "bREPORT", patch_win.bREPORT, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "bBASEDIR", patch_win.bBASEDIR, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "dir_Entry", patch_win.dir_Entry, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "TxtWindow", TxtWindow, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "LOGVIEW", patch_win.LOGVIEW, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "bCLOSE", patch_win.bCLOSE, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(PATCH_WINDOW), "PatchStat", PATCH_STATBAR, GTK_DESTROYNOTIFY(gtk_widget_unref));

	gtk_signal_connect (GTK_OBJECT(PATCH_WINDOW), "destroy", GTK_SIGNAL_FUNC(destroy_window2), NULL);
	gtk_signal_connect (GTK_OBJECT(patch_win.bCLOSE), "clicked", GTK_SIGNAL_FUNC(destroy_window2), NULL);
	gtk_signal_connect (GTK_OBJECT(patch_win.bBASEDIR), "toggled", GTK_SIGNAL_FUNC(basedir_Change), NULL);
	gtk_signal_connect (GTK_OBJECT(patch_win.dir_Entry), "changed", GTK_SIGNAL_FUNC(basedir_ChangePath), NULL);
#if !defined(DEMOBUILD)
	gtk_signal_connect (GTK_OBJECT(patch_win.bAPPLY), "clicked", GTK_SIGNAL_FUNC(start_xpatch), &patch_win);
	gtk_signal_connect (GTK_OBJECT(patch_win.bREPORT), "clicked", GTK_SIGNAL_FUNC(report_status), &patch_win);
#endif	/* !DEMOBUILD */

#if defined(DEMOBUILD)
	gtk_widget_set_sensitive (patch_win.bAPPLY, FALSE);
	gtk_widget_set_sensitive (patch_win.bREPORT, FALSE);
#endif	/* DEMOBUILD */

	gtk_window_set_transient_for (GTK_WINDOW(PATCH_WINDOW), GTK_WINDOW(MAIN_WINDOW));
//	gtk_window_set_position (GTK_WINDOW(PATCH_WINDOW), GTK_WIN_POS_NONE);
	gtk_window_set_position (GTK_WINDOW(PATCH_WINDOW), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_show (PATCH_WINDOW);

// give a short summary about installation
	report_status (NULL, &patch_win);
}

static void create_window1 (void)
{
	int			i;

// Labels for basics
	GtkWidget *TxtTitle;	// Title Label
	GtkWidget *TxtGame0;	// Destiny label
	GtkWidget *TxtVideo;	// Renderer, etc.
	GtkWidget *TxtResol;	// Resolution
	GtkWidget *TxtSound;	// Sound driver combo
// Widgets for basics which needn't be in a relevant struct
	GtkWidget *SND_Entry;	// Sound driver listing
	GtkWidget *bQUIT;	// Quit button

// Labels for additionals
	GtkWidget *TxtMouse;	// Mouse options.
	GtkWidget *TxtNet;	// Networking options.
	GtkWidget *TxtAdv;	// Memory options.
	GtkWidget *TxtVidExt;	// Extra Video Options label
	GtkWidget *TxtSndExt;	// Extra Sound Options label
	GtkWidget *TxtSound2;	// Sound options extra
	GtkWidget *TxtSound3;	// Sound options extra
	GtkWidget *TxtGameT;	// GameType Label
// Widgets for additionals which needn't be in a relevant struct
	GtkWidget *bPATCH;	// PATCH button
	GtkWidget *SRATE_Entry;	// Sampling rate listing

// Separators
	GtkWidget *hseparator0;
	GtkWidget *hseparator1;

// Other stuff
	GList *TmpList = NULL;
	GSList *Destinies = NULL;

	tooltips = gtk_tooltips_new ();

// Create the main window

	MAIN_WINDOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data (GTK_OBJECT(MAIN_WINDOW), "mywindow", MAIN_WINDOW);
#ifndef DEMOBUILD
	gtk_window_set_title (GTK_WINDOW(MAIN_WINDOW), "Hexen II Launcher " LAUNCHER_VERSION_STR);
#else
	gtk_window_set_title (GTK_WINDOW(MAIN_WINDOW), "Hexen2 demo Launcher " LAUNCHER_VERSION_STR);
#endif
	gtk_window_set_resizable (GTK_WINDOW(MAIN_WINDOW), FALSE);
	gtk_widget_set_size_request(MAIN_WINDOW, 230, 354);

// Create the option tabs on the main window

	HOLDER_BOX = gtk_hbox_new (TRUE, 2);
	gtk_widget_show (HOLDER_BOX);
	gtk_container_add (GTK_CONTAINER(MAIN_WINDOW), HOLDER_BOX);

	BOOK0 = gtk_notebook_new ();
	gtk_widget_ref (BOOK0);
//	gtk_container_add (GTK_CONTAINER (MAIN_WINDOW), BOOK0);
	gtk_widget_show (BOOK0);
	gtk_box_pack_start (GTK_BOX(HOLDER_BOX), BOOK0, TRUE, TRUE, 0);

	BASIC_TAB = gtk_fixed_new ();
	gtk_widget_ref (BASIC_TAB);
	gtk_widget_show (BASIC_TAB);

	BOOK1 = gtk_notebook_new ();
	gtk_widget_ref (BOOK1);
	gtk_widget_show (BOOK1);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK(BOOK1), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK(BOOK1));
	gtk_box_pack_start (GTK_BOX(HOLDER_BOX), BOOK1, TRUE, TRUE, 0);

	ADDON_TAB1 = gtk_fixed_new ();
	gtk_widget_ref (ADDON_TAB1);
	gtk_widget_show (ADDON_TAB1);

	ADDON_TAB2 = gtk_fixed_new ();
	gtk_widget_ref (ADDON_TAB2);
	gtk_widget_show (ADDON_TAB2);

//	TAB0_LABEL = gtk_label_new ("Basic Options");
//	gtk_widget_ref (TAB0_LABEL);

	TAB1_LABEL = gtk_label_new (" Sound / Other ");
	gtk_widget_ref (TAB1_LABEL);

	TAB2_LABEL = gtk_label_new ("  Video / Gameplay ");
	gtk_widget_ref (TAB2_LABEL);

//	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK0), BASIC_TAB, TAB0_LABEL);
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK0), BASIC_TAB, NULL);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(BOOK0), FALSE);
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK1), ADDON_TAB2, TAB2_LABEL);
	gtk_notebook_append_page (GTK_NOTEBOOK(BOOK1), ADDON_TAB1, TAB1_LABEL);

	gtk_widget_hide (BOOK1);

// Basic title representing the HoT-version the launcher is packed with

	TxtTitle = gtk_label_new ("Hammer of Thyrion " HOT_VERSION_STR);
	gtk_widget_ref (TxtTitle);
	gtk_widget_show (TxtTitle);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtTitle, 14, 12);
	gtk_label_set_justify (GTK_LABEL(TxtTitle), GTK_JUSTIFY_LEFT);

/********************************************************************
 TAB - 1:		BASIC OPTIONS
 ********************************************************************/

// pakfiles 1.11 patch option:
// PATCH button
	bPATCH = gtk_button_new_with_label (_("Info & 1.11 patch"));
	gtk_widget_ref (bPATCH);
	gtk_widget_show (bPATCH);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), bPATCH, 12, 272);
	gtk_widget_set_size_request (bPATCH, 112, 24);
#ifdef DEMOBUILD
	gtk_tooltips_set_tip (tooltips, bPATCH, _("See a short summary of your installation."), NULL);
#else
	gtk_tooltips_set_tip (tooltips, bPATCH, _("See a short summary of your installation. Apply the v1.11 pakfiles patch by Raven Software, if you haven't done already."), NULL);
#endif

// Launch button and Statusbar
	WGT_LAUNCH = gtk_button_new_with_label (_("Play Hexen II"));
	gtk_widget_ref (WGT_LAUNCH);
	gtk_widget_show (WGT_LAUNCH);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_LAUNCH, 12, 300);
	gtk_widget_set_size_request (WGT_LAUNCH, 112, 24);

	WGT_STATUSBAR = gtk_statusbar_new ();
	gtk_widget_ref (WGT_STATUSBAR);
	gtk_widget_show (WGT_STATUSBAR);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_STATUSBAR, 0, 328);
	gtk_widget_set_size_request (WGT_STATUSBAR, 230, 24);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(WGT_STATUSBAR), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (WGT_STATUSBAR), 2);
	WGT_STATBAR_ID = gtk_statusbar_get_context_id (GTK_STATUSBAR(WGT_STATUSBAR), "BinaryStatus");
	gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), WGT_STATBAR_ID,_("  Waiting status..."));

// Quit button
	bQUIT = gtk_button_new_with_label (_("Quit"));
	gtk_widget_ref (bQUIT);
	gtk_widget_show (bQUIT);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), bQUIT, 132, 300);
	gtk_widget_set_size_request (bQUIT, 80, 24);

/*********************************************************************/

// DESTINY : Game/binary selection
	TxtGame0 = gtk_label_new (_("Choose your destiny:"));
	gtk_widget_ref (TxtGame0);
	gtk_widget_show (TxtGame0);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtGame0, 14, 40);
	gtk_label_set_justify (GTK_LABEL(TxtGame0), GTK_JUSTIFY_LEFT);

// Destiny: Hexen2
#ifdef DEMOBUILD
	WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II demo");
#else
	WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II");
#endif
	Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON(WGT_HEXEN2));
	gtk_widget_ref (WGT_HEXEN2);
	GTK_WIDGET_UNSET_FLAGS (WGT_HEXEN2, GTK_CAN_FOCUS);
	gtk_widget_show (WGT_HEXEN2);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_HEXEN2, 24, 60);
#ifndef DEMOBUILD
	gtk_widget_set_size_request (WGT_HEXEN2, 80, 24);
#else
	gtk_widget_set_size_request (WGT_HEXEN2, 180, 24);
#endif

// Destiny: HexenWorld
	WGT_H2WORLD = gtk_radio_button_new_with_label (Destinies, "HexenWorld Multiplayer");
	Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON(WGT_H2WORLD));
	gtk_widget_ref (WGT_H2WORLD);
	GTK_WIDGET_UNSET_FLAGS (WGT_H2WORLD, GTK_CAN_FOCUS);
	gtk_widget_show (WGT_H2WORLD);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_H2WORLD, 24, 86);
	gtk_widget_set_size_request (WGT_H2WORLD, 180, 24);
	if (!(gameflags & GAME_HEXENWORLD))
		gtk_widget_set_sensitive (WGT_H2WORLD, FALSE);

	if (destiny == DEST_HW)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_H2WORLD), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_HEXEN2), TRUE);

// Mission Pack
#ifndef DEMOBUILD
	WGT_PORTALS = gtk_check_button_new_with_label ("Praevus");
	gtk_widget_ref (WGT_PORTALS);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_PORTALS, 112, 60);
	gtk_widget_show (WGT_PORTALS);
	gtk_tooltips_set_tip (tooltips, WGT_PORTALS, _("play Hexen II with Mission Pack"), NULL);
	GTK_WIDGET_UNSET_FLAGS (WGT_PORTALS, GTK_CAN_FOCUS);
	gtk_widget_set_size_request (WGT_PORTALS, 80, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_PORTALS), mp_support);
	if (destiny != DEST_H2 || !(gameflags & GAME_PORTALS && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
#endif	/* DEMOBUILD */

/*********************************************************************/

// Video Options
	TxtVideo = gtk_label_new (_("Graphics    :"));
	gtk_widget_ref (TxtVideo);
	gtk_widget_show (TxtVideo);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtVideo, 14, 128);
	gtk_label_set_justify (GTK_LABEL(TxtVideo), GTK_JUSTIFY_LEFT);

// opengl/software toggle
	WGT_OPENGL = gtk_check_button_new_with_label ("OpenGL");
	gtk_widget_ref (WGT_OPENGL);
	GTK_WIDGET_UNSET_FLAGS (WGT_OPENGL, GTK_CAN_FOCUS);
	gtk_widget_show (WGT_OPENGL);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_OPENGL, 102, 124);
	gtk_widget_set_size_request (WGT_OPENGL, 110, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_OPENGL), opengl_support);

// fullscreen/windowed toggle
	WGT_FULLSCR = gtk_check_button_new_with_label (_("Fullscreen"));
	gtk_widget_ref (WGT_FULLSCR);
	GTK_WIDGET_UNSET_FLAGS (WGT_FULLSCR, GTK_CAN_FOCUS);
	gtk_widget_show (WGT_FULLSCR);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_FULLSCR, 102, 148);
	gtk_widget_set_size_request (WGT_FULLSCR, 110, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_FULLSCR), fullscreen);

// resolution
	TxtResol = gtk_label_new (_("Resolution  :"));
	gtk_widget_ref (TxtResol);
	gtk_widget_show (TxtResol);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtResol, 14, 180);
	gtk_label_set_justify (GTK_LABEL(TxtResol), GTK_JUSTIFY_LEFT);

// resolution combo
	WGT_RESCOMBO = gtk_combo_new ();
	gtk_widget_ref (WGT_RESCOMBO);
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_RESCOMBO), FALSE);
	gtk_widget_set_size_request (WGT_RESCOMBO, 110, 24);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_RESCOMBO, 102, 176);
// resolution display
	WGT_RESLIST = GTK_COMBO(WGT_RESCOMBO)->entry;
	gtk_widget_ref (WGT_RESLIST);
//	gtk_entry_set_alignment (GTK_ENTRY(WGT_RESLIST), 1);
	gtk_entry_set_editable (GTK_ENTRY(WGT_RESLIST), FALSE);
	// menu listing for resolution come from a callback
	Make_ResMenu ();
	gtk_widget_show (WGT_RESCOMBO);
	gtk_widget_show (WGT_RESLIST);

/*********************************************************************/

// Sound options (basic: driver selection)
	TxtSound = gtk_label_new (_("Sound        :"));
	gtk_widget_ref (TxtSound);
	gtk_widget_show (TxtSound);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtSound, 14, 212);
	gtk_label_set_justify (GTK_LABEL(TxtSound), GTK_JUSTIFY_LEFT);

	WGT_SOUND = gtk_combo_new ();
	gtk_widget_ref (WGT_SOUND);
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_SOUND), FALSE);
	gtk_widget_set_size_request (WGT_SOUND, 110, 24);
	TmpList = NULL;
	for (i = 0; i < MAX_SOUND; i++)
		TmpList = g_list_append (TmpList, snddrv_names[i][1]);
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_SOUND), TmpList);
	g_list_free (TmpList);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_SOUND, 102, 208);
	gtk_widget_show (WGT_SOUND);
	SND_Entry = GTK_COMBO(WGT_SOUND)->entry;
	gtk_widget_ref (SND_Entry);
	gtk_entry_set_text (GTK_ENTRY(SND_Entry), snddrv_names[sound][1]);
	gtk_entry_set_editable (GTK_ENTRY(SND_Entry), FALSE);
//	gtk_entry_set_alignment (GTK_ENTRY(SND_Entry), 1);
	gtk_widget_show (SND_Entry);

// Sampling rate selection
	TxtSound2 = gtk_label_new (_("Sample Rate:"));
	gtk_widget_ref (TxtSound2);
	gtk_widget_show (TxtSound2);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), TxtSound2, 14, 242);
	gtk_label_set_justify (GTK_LABEL(TxtSound2), GTK_JUSTIFY_LEFT);

	WGT_SRATE = gtk_combo_new ();
	gtk_widget_ref (WGT_SRATE);
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_SRATE), FALSE);
	gtk_widget_set_size_request (WGT_SRATE, 110, 24);
	TmpList = NULL;
	for (i = 0; i < MAX_RATES; i++)
		TmpList = g_list_append (TmpList, snd_rates[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_SRATE), TmpList);
	g_list_free (TmpList);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), WGT_SRATE, 102, 238);
	gtk_widget_show (WGT_SRATE);
	SRATE_Entry = GTK_COMBO(WGT_SRATE)->entry;
	gtk_widget_ref (SRATE_Entry);
	gtk_entry_set_text (GTK_ENTRY(SRATE_Entry), snd_rates[sndrate]);
	gtk_entry_set_editable (GTK_ENTRY(SRATE_Entry), FALSE);
	gtk_widget_show (SRATE_Entry);

/********************************************************************
 TAB - 2:		ADDITIONAL OPTIONS
 ********************************************************************/

	MORE_LESS = gtk_button_new_with_label (_("More >>>"));
	gtk_widget_ref (MORE_LESS);
	gtk_widget_show (MORE_LESS);
	gtk_fixed_put (GTK_FIXED(BASIC_TAB), MORE_LESS, 132, 272);
	gtk_widget_set_size_request (MORE_LESS, 80, 24);

/*********************************************************************/

// Additional game-types
	TxtGameT = gtk_label_new (_("Extra Game Types:"));
	gtk_widget_ref (TxtGameT);
	gtk_widget_show (TxtGameT);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), TxtGameT, 14, 16);
	gtk_label_set_justify (GTK_LABEL(TxtGameT), GTK_JUSTIFY_LEFT);

// game types menu for hexen2
	WGT_H2GAME = gtk_combo_new ();
	gtk_widget_ref (WGT_H2GAME);
	gtk_widget_set_size_request (WGT_H2GAME, 172, 32);
#ifndef DEMOBUILD
	TmpList = NULL;
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_H2GAME), FALSE);
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (h2game_names[i].available)
			TmpList = g_list_append (TmpList, h2game_names[i].name);
	}
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_H2GAME), TmpList);
	g_list_free (TmpList);
#endif	/* DEMOBUILD */
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_H2GAME, 36, 36);
	H2G_Entry = GTK_COMBO(WGT_H2GAME)->entry;
	gtk_widget_ref (H2G_Entry);
#ifndef DEMOBUILD
	gtk_entry_set_editable (GTK_ENTRY(H2G_Entry), FALSE);
	gtk_entry_set_text (GTK_ENTRY(H2G_Entry), h2game_names[h2game].name);
	if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_H2GAME, FALSE);
	if (destiny == DEST_H2)
	{
		gtk_widget_show (WGT_H2GAME);
		gtk_widget_show (H2G_Entry);
	}
#else
	gtk_widget_show (WGT_H2GAME);
	gtk_widget_show (H2G_Entry);
	gtk_entry_set_text (GTK_ENTRY(H2G_Entry), "(  None  )");
	gtk_widget_set_sensitive (WGT_H2GAME, FALSE);
#endif

#ifndef DEMOBUILD
// game types menu for hexenworld
	WGT_HWGAME = gtk_combo_new ();
	gtk_widget_ref (WGT_HWGAME);
	gtk_widget_set_size_request (WGT_HWGAME, 172, 32);
	TmpList = NULL;
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_HWGAME), FALSE);
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (hwgame_names[i].available)
			TmpList = g_list_append (TmpList, hwgame_names[i].name);
	}
	gtk_combo_set_popdown_strings (GTK_COMBO(WGT_HWGAME), TmpList);
	g_list_free (TmpList);
//	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_HWGAME, 68, 66);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_HWGAME, 36, 36);
	HWG_Entry = GTK_COMBO(WGT_HWGAME)->entry;
	gtk_widget_ref (HWG_Entry);
	gtk_entry_set_editable (GTK_ENTRY(HWG_Entry), FALSE);
	gtk_entry_set_text (GTK_ENTRY(HWG_Entry), hwgame_names[hwgame].name);
	if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		gtk_widget_set_sensitive (WGT_HWGAME, FALSE);
	if (destiny == DEST_HW)
	{
		gtk_widget_show (WGT_HWGAME);
		gtk_widget_show (HWG_Entry);
	}
//	if (!(gameflags & GAME_HEXENWORLD))
//		gtk_widget_set_sensitive (WGT_HWGAME, FALSE);
#endif	/* DEMOBUILD */

/*********************************************************************/

	hseparator0 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator0);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), hseparator0, 14, 72);
	gtk_widget_set_size_request (hseparator0, 200, 8);
	gtk_widget_show (hseparator0);

/*********************************************************************/

// Additional OpenGL options

	TxtVidExt = gtk_label_new (_("Additional OpenGL Options:"));
	gtk_widget_ref (TxtVidExt);
	gtk_widget_show (TxtVidExt);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), TxtVidExt, 14, 90);
	gtk_label_set_justify (GTK_LABEL(TxtVidExt), GTK_JUSTIFY_LEFT);

// 3dfx Voodoo1/2/Rush support
	WGT_3DFX = gtk_check_button_new_with_label (_("3dfx gamma support"));
	gtk_widget_ref (WGT_3DFX);
	gtk_widget_show (WGT_3DFX);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_3DFX, 14, 112);
	gtk_widget_set_size_request (WGT_3DFX, 160, 24);
	gtk_widget_set_sensitive (WGT_3DFX, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_3DFX), fxgamma);
	GTK_WIDGET_UNSET_FLAGS (WGT_3DFX, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_3DFX, _("Special brightness support for old Voodoo1/2 boards"), NULL);

// Whether to use GL_LUMINANCE lightmap format (the -lm_1 and -lm_4 switches)
	WGT_LM_BYTES = gtk_check_button_new_with_label (_("Use old lightmap format"));
	gtk_widget_ref (WGT_LM_BYTES);
	gtk_widget_show (WGT_LM_BYTES);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_LM_BYTES, 14, 134);
	gtk_widget_set_size_request (WGT_LM_BYTES, 180, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LM_BYTES), use_lm1);
	GTK_WIDGET_UNSET_FLAGS (WGT_LM_BYTES, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_LM_BYTES, _("GL_RGBA is default. Mark to use the old GL_LUMINANCE format"), NULL);
	gtk_widget_set_sensitive (WGT_LM_BYTES, opengl_support);

// 8-bit GL Extensions (Paletted Textures)
	WGT_GL8BIT = gtk_check_button_new_with_label (_("Enable 8-bit textures"));
	gtk_widget_ref (WGT_GL8BIT);
	gtk_widget_show (WGT_GL8BIT);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_GL8BIT, 14, 158);
	gtk_widget_set_size_request (WGT_GL8BIT, 160, 24);
	gtk_widget_set_sensitive (WGT_GL8BIT, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_GL8BIT), is8bit);
	GTK_WIDGET_UNSET_FLAGS (WGT_GL8BIT, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_GL8BIT, _("Enable 8-bit OpenGL texture extensions"), NULL);

// conwidth toggle button
	WGT_CONWBUTTON = gtk_check_button_new_with_label (_("Conwidth :"));
	gtk_widget_ref (WGT_CONWBUTTON);
	gtk_widget_show (WGT_CONWBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_CONWBUTTON, 14, 182);
	gtk_widget_set_size_request (WGT_CONWBUTTON, 84, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_CONWBUTTON), use_con);
	GTK_WIDGET_UNSET_FLAGS (WGT_CONWBUTTON, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_CONWBUTTON, _("Allow bigger/readable text and HUD in high resolutions. Smaller the number, bigger the text. 640 is recommended"), NULL);
	gtk_widget_set_sensitive (WGT_CONWBUTTON, opengl_support);

// conwidth combo
	WGT_CONWCOMBO = gtk_combo_new ();
	gtk_widget_ref (WGT_CONWCOMBO);
	gtk_combo_set_use_arrows (GTK_COMBO(WGT_CONWCOMBO), FALSE);
	gtk_widget_set_size_request (WGT_CONWCOMBO, 108, 24);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_CONWCOMBO, 100, 182);
	gtk_widget_set_sensitive (WGT_CONWCOMBO, opengl_support);
// conwidth display
	WGT_CONWLIST = GTK_COMBO(WGT_CONWCOMBO)->entry;
	gtk_widget_ref (WGT_CONWLIST);
//	gtk_entry_set_alignment (GTK_ENTRY(WGT_CONWLIST), 1);
	gtk_entry_set_editable (GTK_ENTRY(WGT_CONWLIST), FALSE);
	// menu listing for conwidth come from a callback
	Make_ConWidthMenu();
	gtk_widget_show (WGT_CONWCOMBO);
	gtk_widget_show (WGT_CONWLIST);

// Enable VSync
	WGT_VSYNC = gtk_check_button_new_with_label (_("Enable VSync"));
	gtk_widget_ref (WGT_VSYNC);
	gtk_widget_show (WGT_VSYNC);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_VSYNC, 14, 206);
	gtk_widget_set_size_request (WGT_VSYNC, 160, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_VSYNC), vsync);
	GTK_WIDGET_UNSET_FLAGS (WGT_VSYNC, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_VSYNC, _("Synchronize with Monitor Refresh"), NULL);
	gtk_widget_set_sensitive (WGT_VSYNC, opengl_support);

// Multisampling (artialiasing)
	WGT_FSAA = gtk_check_button_new_with_label (_("FSAA (Antialiasing) :"));
	gtk_widget_ref (WGT_FSAA);
	gtk_widget_show (WGT_FSAA);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_FSAA, 14, 230);
	gtk_widget_set_size_request (WGT_FSAA, 148, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_FSAA), use_fsaa);
	GTK_WIDGET_UNSET_FLAGS (WGT_FSAA, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_FSAA, _("Enable Antialiasing"), NULL);
	gtk_widget_set_sensitive (WGT_FSAA, opengl_support);

	WGT_ADJFSAA =  gtk_adjustment_new (aasamples, 0, 4, 2, 10, 10);
	WGT_AASAMPLES = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ADJFSAA), 2, 0);
	gtk_entry_set_editable (GTK_ENTRY(WGT_AASAMPLES), FALSE);
	gtk_widget_show (WGT_AASAMPLES);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_AASAMPLES, 166, 230);
	gtk_widget_set_size_request (WGT_AASAMPLES, 40, 24);
	gtk_widget_ref (WGT_AASAMPLES);

// Custom OpenGL Library
	WGT_LIBGL = gtk_check_button_new_with_label (_("Use a different GL library:"));
	gtk_widget_ref (WGT_LIBGL);
	gtk_widget_show (WGT_LIBGL);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_LIBGL, 14, 256);
	gtk_widget_set_size_request (WGT_LIBGL, 180, 24);
	gtk_widget_set_sensitive (WGT_LIBGL, opengl_support);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LIBGL), gl_nonstd);
	GTK_WIDGET_UNSET_FLAGS (WGT_LIBGL, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_LIBGL, _("Type the full path of the OpenGL library"), NULL);

	WGT_GLPATH = gtk_entry_new ();
	gtk_widget_show (WGT_GLPATH);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB2), WGT_GLPATH, 36, 280);
	gtk_widget_set_size_request (WGT_GLPATH, 172, 24);
	gtk_entry_set_max_length (GTK_ENTRY(WGT_GLPATH), sizeof(gllibrary)-1);
	gtk_entry_set_text (GTK_ENTRY(WGT_GLPATH), gllibrary);
//	if (!opengl_support || !gl_nonstd)
//		gtk_widget_set_sensitive (WGT_GLPATH, FALSE);
	gtk_widget_ref (WGT_GLPATH);

/********************************************************************
 TAB - 3:		ADDITIONAL OPTIONS
 ********************************************************************/

// Additional Sound options

	TxtSndExt = gtk_label_new (_("Additional Options:"));
	gtk_widget_ref (TxtSndExt);
	gtk_widget_show (TxtSndExt);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtSndExt, 14, 16);
	gtk_label_set_justify (GTK_LABEL(TxtSndExt), GTK_JUSTIFY_LEFT);

// 8/16-bit format toggle
	TxtSound3 = gtk_label_new (_("Sound Format:"));
	gtk_widget_ref (TxtSound3);
	gtk_widget_show (TxtSound3);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtSound3, 14, 44);
	gtk_label_set_justify (GTK_LABEL(TxtSound3), GTK_JUSTIFY_LEFT);

	WGT_SBITS = gtk_check_button_new_with_label (sndbits ? "16 bit" : " 8 bit");
	gtk_widget_ref (WGT_SBITS);
	gtk_widget_show (WGT_SBITS);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_SBITS, 112, 40);
	gtk_widget_set_size_request (WGT_SBITS, 76, 24);
	gtk_widget_set_sensitive (WGT_SBITS, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_SBITS), sndbits);
	GTK_WIDGET_UNSET_FLAGS (WGT_SBITS, GTK_CAN_FOCUS);

/*********************************************************************/

// MUSIC (Additional Sound options-2)

// disabling cdaudio
	WGT_CDAUDIO = gtk_check_button_new_with_label (_("No CDaudio"));
	gtk_widget_ref (WGT_CDAUDIO);
	gtk_widget_show (WGT_CDAUDIO);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_CDAUDIO, 112, 70);
	gtk_widget_set_size_request (WGT_CDAUDIO, 96, 24);
	gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_CDAUDIO), !cdaudio);
	GTK_WIDGET_UNSET_FLAGS (WGT_CDAUDIO, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_CDAUDIO, _("Mark to Disable CDAudio"), NULL);

// disabling midi
	WGT_MIDI = gtk_check_button_new_with_label (_("No MIDI"));
	gtk_widget_ref (WGT_MIDI);
	gtk_widget_show (WGT_MIDI);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MIDI, 14, 70);
	gtk_widget_set_size_request (WGT_MIDI, 76, 24);
	gtk_widget_set_sensitive (WGT_MIDI, sound);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MIDI), !midi);
	GTK_WIDGET_UNSET_FLAGS (WGT_MIDI, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_MIDI, _("Mark to Disable Midi"), NULL);

/*********************************************************************/

// Network: disabling LAN
	TxtNet = gtk_label_new (_("Networking :"));
	gtk_widget_ref (TxtNet);
	gtk_widget_show (TxtNet);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtNet, 14, 102);
	gtk_label_set_justify (GTK_LABEL(TxtNet), GTK_JUSTIFY_LEFT);

	WGT_LANBUTTON = gtk_check_button_new_with_label (_("Disable LAN"));
	gtk_widget_ref (WGT_LANBUTTON);
	gtk_widget_show (WGT_LANBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_LANBUTTON, 112, 98);
	gtk_widget_set_size_request (WGT_LANBUTTON, 108, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_LANBUTTON), !lan);
	GTK_WIDGET_UNSET_FLAGS (WGT_LANBUTTON, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_LANBUTTON, _("Mark to Disable Net"), NULL);
	if (destiny == DEST_HW
#if !defined(DEMOBUILD)
		|| h2game_names[h2game].is_botmatch
#endif	/* DEMOBUILD */
	   )
		gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);

/*********************************************************************/

// Mouse: disabling Mouse
	TxtMouse = gtk_label_new (_("Mouse  :"));
	gtk_widget_ref (TxtMouse);
	gtk_widget_show (TxtMouse);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtMouse, 14, 122);
	gtk_label_set_justify (GTK_LABEL(TxtMouse), GTK_JUSTIFY_LEFT);

	WGT_MOUSEBUTTON = gtk_check_button_new_with_label (_("Disable Mouse"));
	gtk_widget_ref (WGT_MOUSEBUTTON);
	gtk_widget_show (WGT_MOUSEBUTTON);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MOUSEBUTTON, 112, 118);
	gtk_widget_set_size_request (WGT_MOUSEBUTTON, 112, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MOUSEBUTTON), !mouse);
	GTK_WIDGET_UNSET_FLAGS (WGT_MOUSEBUTTON, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_MOUSEBUTTON, _("Mark to Disable Mouse"), NULL);

/*********************************************************************/

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator1);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), hseparator1, 14, 144);
	gtk_widget_set_size_request (hseparator1, 200, 8);
	gtk_widget_show (hseparator1);

/*********************************************************************/

// Advanced options
	TxtAdv = gtk_label_new (_("Advanced :"));
	gtk_widget_ref (TxtAdv);
	gtk_widget_show (TxtAdv);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), TxtAdv, 14, 162);
	gtk_label_set_justify (GTK_LABEL(TxtAdv), GTK_JUSTIFY_LEFT);

// Memory options (heapsize)
	WGT_MEMHEAP = gtk_check_button_new_with_label (_("Heapsize (KB):"));
	gtk_widget_ref (WGT_MEMHEAP);
	gtk_widget_show (WGT_MEMHEAP);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MEMHEAP, 14, 182);
	gtk_widget_set_size_request (WGT_MEMHEAP, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MEMHEAP), use_heap);
	GTK_WIDGET_UNSET_FLAGS (WGT_MEMHEAP, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_MEMHEAP, _("The main memory to allocate in KB"), NULL);

	WGT_HEAPADJ =  gtk_adjustment_new (heapsize, HEAP_MINSIZE, HEAP_MAXSIZE, 1024, 10, 10);
	WGT_HEAPSIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_HEAPADJ), 1024, 0);
	gtk_entry_set_editable (GTK_ENTRY(WGT_HEAPSIZE), FALSE);
	gtk_widget_show (WGT_HEAPSIZE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_HEAPSIZE, 134, 182);
	gtk_widget_set_size_request (WGT_HEAPSIZE, 64, 24);
	gtk_widget_ref (WGT_HEAPSIZE);

// Memory options (zonesize)
	WGT_MEMZONE = gtk_check_button_new_with_label (_("Zonesize (KB):"));
	gtk_widget_ref (WGT_MEMZONE);
	gtk_widget_show (WGT_MEMZONE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_MEMZONE, 14, 208);
	gtk_widget_set_size_request (WGT_MEMZONE, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_MEMZONE), use_zone);
	GTK_WIDGET_UNSET_FLAGS (WGT_MEMZONE, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_MEMZONE, _("Dynamic zone memory to allocate in KB"), NULL);

	WGT_ZONEADJ =  gtk_adjustment_new (zonesize, ZONE_MINSIZE, ZONE_MAXSIZE, 1, 10, 10);
	WGT_ZONESIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ZONEADJ), 1, 0);
	gtk_entry_set_editable (GTK_ENTRY(WGT_ZONESIZE), FALSE);
	gtk_widget_show (WGT_ZONESIZE);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_ZONESIZE, 134, 208);
	gtk_widget_set_size_request (WGT_ZONESIZE, 64, 24);
	gtk_widget_ref (WGT_ZONESIZE);

// Debug output option
	WGT_DBGLOG = gtk_check_button_new_with_label (_("Generate log"));
	gtk_widget_ref (WGT_DBGLOG);
	gtk_widget_show (WGT_DBGLOG);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_DBGLOG, 14, 232);
	gtk_widget_set_size_request (WGT_DBGLOG, 116, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_DBGLOG), debug);
	GTK_WIDGET_UNSET_FLAGS (WGT_DBGLOG, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_DBGLOG, _("Write console output to a log file"), NULL);

	WGT_DBGLOG2 = gtk_check_button_new_with_label (_("Full log"));
	gtk_widget_ref (WGT_DBGLOG2);
	gtk_widget_show (WGT_DBGLOG2);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_DBGLOG2, 134, 232);
	gtk_widget_set_size_request (WGT_DBGLOG2, 72, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_DBGLOG2), debug2);
	GTK_WIDGET_UNSET_FLAGS (WGT_DBGLOG2, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_DBGLOG2, _("Full developer mode logging"), NULL);

// Extra User Arguments:
	WGT_EXTBTN = gtk_check_button_new_with_label (_("Use extra arguments:"));
	gtk_widget_ref (WGT_EXTBTN);
	gtk_widget_show (WGT_EXTBTN);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_EXTBTN, 14, 256);
	gtk_widget_set_size_request (WGT_EXTBTN, 180, 24);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(WGT_EXTBTN), use_extra);
	GTK_WIDGET_UNSET_FLAGS (WGT_EXTBTN, GTK_CAN_FOCUS);
	gtk_tooltips_set_tip (tooltips, WGT_EXTBTN, _("Type the extra arguments you want to pass to the command line"), NULL);

	WGT_EXTARGS = gtk_entry_new ();
	gtk_widget_show (WGT_EXTARGS);
	gtk_fixed_put (GTK_FIXED(ADDON_TAB1), WGT_EXTARGS, 36, 280);
	gtk_widget_set_size_request (WGT_EXTARGS, 172, 24);
	gtk_entry_set_max_length (GTK_ENTRY(WGT_EXTARGS), sizeof(ext_args)-1);
	gtk_entry_set_text (GTK_ENTRY(WGT_EXTARGS), ext_args);
	gtk_widget_ref (WGT_EXTARGS);

/***	END OF OPTION WIDGETS	***/

// setup the tooltips
	gtk_object_set_data (GTK_OBJECT(MAIN_WINDOW), "tooltips", tooltips);

// destroy notifications setup
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "notebook0", BOOK0, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "fixed0", BASIC_TAB, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "notebook1", BOOK1, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "fixed1", ADDON_TAB1, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "fixed2", ADDON_TAB2, GTK_DESTROYNOTIFY(gtk_widget_unref));
//	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "lbl_tab0", TAB0_LABEL, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "lbl_tab1", TAB1_LABEL, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "lbl_tab2", TAB2_LABEL, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtTitle", TxtTitle, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bQUIT", bQUIT, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bLAUNCH", WGT_LAUNCH, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "LaunchStatbar", WGT_STATUSBAR, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtGame0", TxtGame0, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bHEXEN2", WGT_HEXEN2, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bH2W", WGT_H2WORLD, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtVideo", TxtVideo, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bOGL", WGT_OPENGL, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bFULS", WGT_FULLSCR, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtResol", TxtResol, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "cRES", WGT_RESCOMBO, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "eRES", WGT_RESLIST, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bCONW", WGT_CONWBUTTON, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "cCONW", WGT_CONWCOMBO, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "eCONW", WGT_CONWLIST, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtSound", TxtSound, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "cSND", WGT_SOUND, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "SND_Entry", SND_Entry, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bMORE", MORE_LESS, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtSndExt", TxtSndExt, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtSound2", TxtSound2, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "cSRATE", WGT_SRATE, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "SRATE_Entry", SRATE_Entry, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtSound3", TxtSound3, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bSBITS", WGT_SBITS, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bCDA", WGT_CDAUDIO, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bMIDI", WGT_MIDI, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "hseparator0", hseparator0, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtVidExt", TxtVidExt, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "b3DFX", WGT_3DFX, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "b8BIT", WGT_GL8BIT, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bLM_BYTES", WGT_LM_BYTES, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bVSYNC", WGT_VSYNC, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bFSAA", WGT_FSAA, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "spnFSAA", WGT_AASAMPLES, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bLIBGL", WGT_LIBGL, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "GL_Entry", WGT_GLPATH, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bPATCH", bPATCH, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtGameT", TxtGameT, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "SelH2", WGT_H2GAME, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "H2G_Entry", H2G_Entry, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "hseparator1", hseparator1, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtNet", TxtNet, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bLAN", WGT_LANBUTTON, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtMouse", TxtMouse, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bMOUSE", WGT_MOUSEBUTTON, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "TxtAdv", TxtAdv, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bMEMHEAP", WGT_MEMHEAP, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "spnHEAP", WGT_HEAPSIZE, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bMEMZONE", WGT_MEMZONE, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "spnZONE", WGT_ZONESIZE, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bDBG", WGT_DBGLOG, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bDBG2", WGT_DBGLOG2, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bEXTBTN", WGT_EXTBTN, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "EXT_Entry", WGT_EXTARGS, GTK_DESTROYNOTIFY(gtk_widget_unref));
#ifndef DEMOBUILD
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "bH2MP", WGT_PORTALS, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "SelHW", WGT_HWGAME, GTK_DESTROYNOTIFY(gtk_widget_unref));
	gtk_object_set_data_full (GTK_OBJECT(MAIN_WINDOW), "HWG_Entry", HWG_Entry, GTK_DESTROYNOTIFY(gtk_widget_unref));
#endif	/* DEMOBUILD */

// callback functions setup
	gtk_signal_connect (GTK_OBJECT(MAIN_WINDOW), "destroy", GTK_SIGNAL_FUNC(ui_quit), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_LAUNCH), "clicked", GTK_SIGNAL_FUNC(launch_hexen2_bin), NULL);
	gtk_signal_connect (GTK_OBJECT(bQUIT), "clicked", GTK_SIGNAL_FUNC(ui_quit), NULL);
#ifndef DEMOBUILD
	gtk_signal_connect (GTK_OBJECT(H2G_Entry), "changed", GTK_SIGNAL_FUNC(H2GameChange), NULL);
	gtk_signal_connect (GTK_OBJECT(HWG_Entry), "changed", GTK_SIGNAL_FUNC(HWGameChange), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_PORTALS), "clicked", GTK_SIGNAL_FUNC(BoolRevert), &mp_support);
#endif	/* DEMOBUILD */
	gtk_signal_connect (GTK_OBJECT(bPATCH), "clicked", GTK_SIGNAL_FUNC(create_window2), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_HEXEN2), "clicked", GTK_SIGNAL_FUNC(on_HEXEN2), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_H2WORLD), "clicked", GTK_SIGNAL_FUNC(on_H2W), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_OPENGL), "clicked", GTK_SIGNAL_FUNC(on_OGL), NULL);
	gtk_signal_connect (GTK_OBJECT(SND_Entry), "changed", GTK_SIGNAL_FUNC(on_SND), NULL);
	gtk_signal_connect (GTK_OBJECT(SRATE_Entry), "changed", GTK_SIGNAL_FUNC(on_SRATE), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_SBITS), "toggled", GTK_SIGNAL_FUNC(on_SBITS), &sndbits);
	gtk_signal_connect (GTK_OBJECT(WGT_MIDI), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &midi);
	gtk_signal_connect (GTK_OBJECT(WGT_CDAUDIO), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &cdaudio);
	gtk_signal_connect (GTK_OBJECT(WGT_LANBUTTON), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &lan);
	gtk_signal_connect (GTK_OBJECT(WGT_FULLSCR), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &fullscreen);
	gtk_signal_connect (GTK_OBJECT(WGT_CONWBUTTON), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_con);
	gtk_signal_connect (GTK_OBJECT(WGT_3DFX), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &fxgamma);
	gtk_signal_connect (GTK_OBJECT(WGT_GL8BIT), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &is8bit);
	gtk_signal_connect (GTK_OBJECT(WGT_LM_BYTES), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_lm1);
	gtk_signal_connect (GTK_OBJECT(WGT_VSYNC), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &vsync);
	gtk_signal_connect (GTK_OBJECT(WGT_FSAA), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_fsaa);
	gtk_signal_connect (GTK_OBJECT(WGT_ADJFSAA), "value_changed", GTK_SIGNAL_FUNC(adj_Change), &aasamples);
	gtk_signal_connect (GTK_OBJECT(WGT_LIBGL), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &gl_nonstd);
	gtk_signal_connect (GTK_OBJECT(WGT_DBGLOG), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &debug);
	gtk_signal_connect (GTK_OBJECT(WGT_DBGLOG2), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &debug2);
	gtk_signal_connect (GTK_OBJECT(WGT_MEMHEAP), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_heap);
	gtk_signal_connect (GTK_OBJECT(WGT_MEMZONE), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_zone);
	gtk_signal_connect (GTK_OBJECT(WGT_EXTBTN), "toggled", GTK_SIGNAL_FUNC(BoolRevert), &use_extra);
	gtk_signal_connect (GTK_OBJECT(WGT_RESLIST), "changed", GTK_SIGNAL_FUNC(res_Change), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_CONWLIST), "changed", GTK_SIGNAL_FUNC(con_Change), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_GLPATH), "changed", GTK_SIGNAL_FUNC(libgl_Change), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_EXTARGS), "changed", GTK_SIGNAL_FUNC(extargs_Change), NULL);
	gtk_signal_connect (GTK_OBJECT(WGT_HEAPADJ), "value_changed", GTK_SIGNAL_FUNC(adj_Change), &heapsize);
	gtk_signal_connect (GTK_OBJECT(WGT_ZONEADJ), "value_changed", GTK_SIGNAL_FUNC(adj_Change), &zonesize);
	gtk_signal_connect (GTK_OBJECT(MORE_LESS), "clicked", GTK_SIGNAL_FUNC(on_MORE), NULL);

// show the window
	gtk_widget_show (MAIN_WINDOW);
}


/*********************************************************************/
// EXPOSED UI FUNCTIONS:

static gint ui_init (gpointer user_data)
{
// create the main window
	create_window1();

// update the launch button status
	UpdateStats();

// if we have a bad installation (such as un-patched versions
// of gamedata), pop up the patch window and print a report:
	if (gameflags & GAME_INSTBAD)
	{
	//	on_MORE (GTK_BUTTON(MORE_LESS), NULL);
	//	ui_pump ();
		create_window2 (NULL, NULL);
	}

	return 0;
}

int ui_main (int *argc, char ***argv)
{
/*
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif
	gtk_set_locale ();
*/

	gtk_init (argc, argv);

/*	add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
*/

	gtk_init_add (GTK_INIT_FUNC(ui_init), NULL);
	gtk_main ();

	return 0;
}

void ui_quit (void)
{
	write_config_file ();
	gtk_main_quit ();
}

void ui_pump (void)
{
	while (gtk_events_pending ())
		gtk_main_iteration ();
}

