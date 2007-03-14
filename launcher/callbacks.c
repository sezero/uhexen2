/*
	callbacks.c
	hexen2 launcher callbacks

	$Id: callbacks.c,v 1.40 2007-03-14 21:04:23 sezero Exp $
*/

#include "common.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "launcher_defs.h"
#include "config_file.h"
#include "support.h"
#include "compat_gtk1.h"
#ifndef DEMOBUILD
#include <pthread.h>
#include "apply_patch.h"
#endif	/* DEMOBUILD */

// from launch_bin.c
extern int missingexe;
extern int is_botmatch;
#ifndef DEMOBUILD
static size_t	string_size = 0;
extern char *h2game_names[MAX_H2GAMES][3];
extern char *hwgame_names[MAX_HWGAMES][3];
#endif
extern char *snddrv_names[MAX_SOUND][2];
extern char *snd_rates[MAX_RATES];

static int	bmore = 0, lock = 0;

static char *res_names[RES_MAX] = {

	"320 x 240",
	"400 x 300",
	"512 x 384",
	"640 x 480",
	"800 x 600",
	"1024 x 768",
	"1280 x 1024",
	"1600 x 1200"
};

static char *stats[] = {

	"  Ready to run the game",
	"  Binary missing or not executable"
};

#ifndef DEMOBUILD
void Log_printf (const char *fmt, ...) __attribute__((format(printf,1,2)));

static char *pstats[] = {

	"  Patch in progress.....",
	"  Patch process finished"
};
int			thread_alive;

static void ui_pump (void)
{
	while (gtk_events_pending ())
		gtk_main_iteration ();
}

#ifdef WITH_GTK1
static GtkText *LogEntry = NULL;

static void ui_LogInit (GtkWidget *wgt)
{
	LogEntry = GTK_TEXT (wgt);
	// clear the window
	gtk_editable_delete_text (GTK_EDITABLE(wgt), 0, -1);
}

static void ui_LogPrint (const char *txt)
{
	gtk_text_insert (LogEntry, NULL, NULL, NULL, txt, strlen(txt));
}

#else	/* here is the gtk2 version */
static GtkTextView *LogEntry = NULL;

static void ui_LogInit (GtkWidget *wgt)
{
	GtkTextBuffer	*buf;
	GtkTextIter	start, end;

	LogEntry = GTK_TEXT_VIEW (wgt);
	// clear the window
	buf = gtk_text_view_get_buffer (LogEntry);
	gtk_text_buffer_get_start_iter (buf, &start);
	gtk_text_buffer_get_end_iter (buf, &end);
	gtk_text_buffer_delete (buf, &start, &end);
}

static void ui_LogPrint (const char *txt)
{
	GtkTextBuffer	*buf;
	GtkTextIter	start, end;

	buf = gtk_text_view_get_buffer (LogEntry);
	gtk_text_buffer_get_start_iter(buf, &start);
	gtk_text_buffer_get_end_iter(buf, &end);
	gtk_text_buffer_insert_at_cursor (buf, txt, strlen(txt));
}
#endif

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

void start_xpatch (GtkObject *Unused, PatchWindow_t *PatchWindow)
{
	pthread_t		thr;
	void		*ptr = NULL;

	gtk_widget_set_sensitive (PatchWindow->bAPPLY, FALSE);
	gtk_widget_set_sensitive (PatchWindow->bCLOSE, FALSE);
	ui_LogInit (PatchWindow->LOGVIEW);

	thread_alive = 1;
	if (pthread_create(&thr, NULL, apply_patches, NULL) != 0)
	{
		Log_printf ("pthread_create failed");
		goto finish;
	}

	gtk_statusbar_push (GTK_STATUSBAR (PatchWindow->PStat), PatchWindow->BinStat, pstats[0]);

	while (thread_alive)
	{
		ui_pump ();
		usleep (10000);
	}

	pthread_join(thr, (void **) &ptr);

finish:
	ui_LogEnd();
	gtk_widget_set_sensitive (PatchWindow->bCLOSE, TRUE);
	gtk_widget_set_sensitive (PatchWindow->bAPPLY, TRUE);
	gtk_statusbar_push (GTK_STATUSBAR (PatchWindow->PStat), PatchWindow->BinStat, pstats[1]);
}
#endif	/* ! DEMOBUILD */

void on_SND (GtkEditable *editable, sndwidget_t *wgt)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);

	for (i = 0; i < MAX_SOUND; i++)
	{
		if (strcmp(tmp, snddrv_names[i][1]) == 0)
		{
			g_free(tmp);
			sound = i;
			gtk_widget_set_sensitive (wgt->MUSIC_MIDI, sound);
			gtk_widget_set_sensitive (wgt->MUSIC_CD, sound);
			gtk_widget_set_sensitive (wgt->SND_RATE, sound);
			gtk_widget_set_sensitive (wgt->SND_BITS, sound);
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

void on_SRATE (GtkEditable *editable, gpointer user_data)
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

void on_SBITS (GtkButton *button, int *opt)
{
	*opt = !(*opt);
	gtk_button_set_label(button, (*opt) ? "16 bit" : " 8 bit");
}

void Make_ResMenu (struct Video_s *wgt)
{
	int	i, up;
	GList *ResList = NULL;

	up = (opengl_support) ? RES_MAX-1 : RES_640;
	for (i = 2*opengl_support; i <= up; i++)
		ResList = g_list_append (ResList, res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO (wgt->RES_COMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY (wgt->RES_LIST), res_names[resolution]);
}

void Make_ConwMenu (struct Video_s *wgt)
{
	int	i;
	GList *ResList = NULL;

	for (i = 0; i <= resolution; i++)
		ResList = g_list_append (ResList, res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO (wgt->CONW_COMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY (wgt->CONW_LIST), res_names[conwidth]);
}

void on_OGL (GtkToggleButton *button, gamewidget_t *wgt)
{
//	Make_ResMenu() triggers "changed" signal
//	for RES_LIST, therefore prevent the fight
	lock = 1;

	opengl_support = !opengl_support;
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
	gtk_widget_set_sensitive (wgt->TDFX_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->GL8BIT_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->LM_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->VSYNC_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->FSAA_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->LIBGL_BUTTON, opengl_support);
	gtk_widget_set_sensitive (wgt->CONW_BUTTON_S, opengl_support);
	gtk_widget_set_sensitive (wgt->CONW_COMBO_S, opengl_support);
	Make_ResMenu(&(wgt->Video));
	if (opengl_support)
		Make_ConwMenu(&(wgt->Video));
	UpdateStats(&(wgt->Launch));
	lock = 0;
}

void res_Change (GtkEditable *editable, struct Video_s *wgt)
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
		Make_ConwMenu(wgt);
	}
	lock = 0;
}

void con_Change (GtkEditable *editable, gpointer user_data)
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

void libgl_Change (GtkEditable *editable, gpointer user_data)
{
	size_t len;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	len = strlen(tmp);
	if (len > sizeof(gllibrary)-1)
		len = sizeof(gllibrary)-1;
	memset (gllibrary, 0, sizeof(gllibrary));
	memcpy (gllibrary, tmp, len);
	g_free (tmp);
}

void UpdateStats (struct Launch_s *wgt)
{
	CheckExe();
	gtk_statusbar_push (GTK_STATUSBAR (wgt->STATUSBAR), wgt->BinStat, stats[missingexe]);
	gtk_widget_set_sensitive (wgt->LAUNCH_BUTTON, !missingexe);
}

void on_HEXEN2 (GtkButton *button, gamewidget_t *wgt)
{
	destiny = DEST_H2;
#ifndef DEMOBUILD
	gtk_widget_set_sensitive (wgt->PORTALS, TRUE);
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, !is_botmatch);
	gtk_widget_hide (wgt->HWGAME);
	gtk_widget_show (wgt->H2GAME);
#else
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, TRUE);
#endif
	UpdateStats(&(wgt->Launch));
}

void on_H2W (GtkButton *button, gamewidget_t *wgt)
{
	destiny = DEST_HW;
#ifndef DEMOBUILD
	gtk_widget_set_sensitive (wgt->PORTALS, FALSE);
	gtk_widget_hide (wgt->H2GAME);
	gtk_widget_show (wgt->HWGAME);
#endif
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, FALSE);
	UpdateStats(&(wgt->Launch));
}

#ifndef DEMOBUILD
static void FindMaxStringSize (void)
{
	size_t	i, len;

	for (i = 1; i < MAX_H2GAMES; i++)
	{
		len = strlen(h2game_names[i][0]) + 9;	// strlen("progs.dat") == 9
		if (string_size < len)
			string_size = len;
	}

	for (i = 1; i < MAX_HWGAMES; i++)
	{
		len = strlen(hwgame_names[i][0]) + 11;	// strlen("hwprogs.dat") == 11
		if (string_size < len)
			string_size = len;
		len = len + strlen(hwgame_names[i][2]) - 11;
		if (string_size < len)
			string_size = len;
	}

	string_size += 2;			// 1 for "/" + 1 for null termination
}

void H2GameScan (GList *GameList)
{
	int	i;
	char	*Title;

	if (!string_size)
		FindMaxStringSize ();

	Title = (char *)malloc(string_size);
	for (i = 1; i < MAX_H2GAMES; i++)
	{
		printf("Looking for %s ... ", h2game_names[i][1]);
		strcpy (Title, h2game_names[i][0]);
		strcat (Title, "/progs.dat");
		if (access(Title, R_OK) == 0)
		{
			GameList = g_list_append (GameList, h2game_names[i][1]);
			printf("Found OK.\n");
		}
		else
		{
			printf("NOT found.\n");
			if (h2game == i)
				h2game = 0;
		}
	}
	free (Title);
	is_botmatch = atoi (h2game_names[h2game][2]);
}

void HWGameScan (GList *GameList)
{
	int	i, j;
	char	*Title;

	if (!string_size)
		FindMaxStringSize ();

	Title = (char *)malloc(string_size);
	for (i = 1; i < MAX_HWGAMES; i++)
	{
		printf("Looking for %s ... ", hwgame_names[i][1]);
		strcpy (Title, hwgame_names[i][0]);
		strcat (Title, "/hwprogs.dat");
		j = access(Title, R_OK);
		if (j == 0)
		{
			strcpy (Title, hwgame_names[i][0]);
			strcat (Title, "/");
			strcat (Title, hwgame_names[i][2]);
			j = access(Title, R_OK);
		}
		if (j == 0)
		{
			GameList = g_list_append (GameList, hwgame_names[i][1]);
			printf("Found OK.\n");
		}
		else
		{
			printf("NOT found.\n");
			if (hwgame == i)
				hwgame = 0;
		}
	}
	free (Title);
}

void H2GameChange (GtkEditable *editable, gamewidget_t *wgt)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < MAX_H2GAMES; i++)
	{
		if (strcmp(tmp, h2game_names[i][1]) == 0)
		{
			g_free(tmp);
			h2game = i;
			is_botmatch = atoi (h2game_names[i][2]);
			gtk_widget_set_sensitive (wgt->LAN_BUTTON, !is_botmatch);
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

void HWGameChange (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	for (i = 0; i < MAX_HWGAMES; i++)
	{
		if (strcmp(tmp, hwgame_names[i][1]) == 0)
		{
			g_free(tmp);
			hwgame = i;
			return;
		}
// Normally, we should be all set within this loop, thus no "else"
	}
}

#endif	/* ! DEMOBUILD */

void ReverseOpt (GtkObject *Unused, int *opt)
{
	*opt = !(*opt);
}

void adj_Change (GtkAdjustment *adj, int *opt)
{
	*opt = (int)(adj->value);
//	printf ("Value is: %d\n", *opt);
}

void on_MORE (GtkButton *button, HoTWindow_t *window)
{ 
	bmore = !bmore;
	gtk_button_set_label(button, bmore ? _("<<< Less") : _("More >>>"));
	gtk_widget_set_size_request(window->mywindow, bmore ? 460 : 230, 354);
	if (bmore)
		gtk_widget_show (window->notebook1);
	else
		gtk_widget_hide (window->notebook1);
}

