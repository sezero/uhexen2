#include "common.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "launcher_defs.h"
#include "support.h"
#include "compat_gtk1.h"

extern int missingexe;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int conwidth;
extern char gllibrary[256];
extern int sound;
extern int sndrate;
extern int destiny;
#ifndef DEMOBUILD
extern int mp_support;
extern int h2game;
extern int hwgame;
extern const char *h2game_names[MAX_H2GAMES][2];
extern const char *hwgame_names[MAX_HWGAMES][2];
#endif
extern const char *snddrv_names[MAX_SOUND][2];
extern const char *snd_rates[MAX_RATES];
int	bmore = 0, lock = 0;

const char *res_names[] = {

	"320 x 240",
	"400 x 300",
	"512 x 384",
	"640 x 480",
	"800 x 600",
	"1024 x 768",
	"1280 x 1024",
	"1600 x 1200"
};

const char *stats[] = {

	"  Ready to run the game",
	"  Binary missing or not executable"
};

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
	(*(opt)) = !(*(opt));
	gtk_button_set_label(button, (*(opt)) ? "16 bit" : " 8 bit");
}

void Make_ResMenu (struct Video_s *wgt)
{
	int	i, up;
	GList *ResList = NULL;

	up = (opengl_support) ? RES_MAX : RES_640;
	for (i = 2*opengl_support; i <= up; i++)
		ResList = g_list_append (ResList, (char *)res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO (wgt->RES_COMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY (wgt->RES_LIST), (char *)res_names[resolution]);
}

void Make_ConwMenu (struct Video_s *wgt)
{
	int	i;
	GList *ResList = NULL;

	for (i = 0; i <= resolution; i++)
		ResList = g_list_append (ResList, (char *)res_names[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO (wgt->CONW_COMBO), ResList);
	g_list_free (ResList);
	gtk_entry_set_text (GTK_ENTRY (wgt->CONW_LIST), (char *)res_names[conwidth]);
}

void on_OGL (GtkToggleButton *button, gamewidget_t *wgt)
{
/*	Make_ResMenu() triggers "changed" signal for RES_LIST
	Prevent the fight: res_Change() wont do a thing if(lock)
*/	lock = 1;
	opengl_support = !opengl_support;
	switch (opengl_support)
	{
	case 0:
		if (resolution > RES_640)
			resolution = RES_640;
		break;
	case 1:
		if (resolution < RES_640)
			resolution = RES_640;
		if (conwidth > resolution)
			conwidth = resolution;
		break;
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

	if (!lock)
	{
		lock = 1;
		tmp = gtk_editable_get_chars (editable, 0, -1);
		for (i = 0; i <= RES_MAX; i++)
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
}

void con_Change (GtkEditable *editable, gpointer user_data)
{
	int	i;
	gchar	*tmp;

	if (!lock)
	{
		tmp = gtk_editable_get_chars (editable, 0, -1);
		for (i = 0; i <= RES_MAX; i++)
		{
			if (strcmp(tmp, res_names[i]) == 0)
			{
				g_free(tmp);
				conwidth = i;
				return;
			}
		// Normally, we should be all set within this loop
		}
	}
}

void libgl_Change (GtkEditable *editable, gpointer user_data)
{
	gchar *tmp = gtk_editable_get_chars (editable, 0, -1);
	memset (gllibrary, 0, sizeof(gllibrary));
	memcpy (gllibrary, tmp, strlen(tmp));
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
	mp_support = 0;
	gtk_widget_set_sensitive (wgt->H2GAME, TRUE);
	gtk_widget_set_sensitive (wgt->HWGAME, FALSE);
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, !h2game);
#else
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, TRUE);
#endif
	UpdateStats(&(wgt->Launch));
}

#ifndef DEMOBUILD
void on_H2MP (GtkButton *button, gamewidget_t *wgt)
{
	destiny = DEST_H2;
	mp_support = 1;
	gtk_widget_set_sensitive (wgt->H2GAME, FALSE);
	gtk_widget_set_sensitive (wgt->HWGAME, FALSE);
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, TRUE);
	UpdateStats(&(wgt->Launch));
}
#endif

void on_H2W (GtkButton *button, gamewidget_t *wgt)
{
	destiny = DEST_HW;
#ifndef DEMOBUILD
	gtk_widget_set_sensitive (wgt->H2GAME, FALSE);
	gtk_widget_set_sensitive (wgt->HWGAME, TRUE);
#endif
	gtk_widget_set_sensitive (wgt->LAN_BUTTON, FALSE);
	UpdateStats(&(wgt->Launch));
}

#ifndef DEMOBUILD
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
			gtk_widget_set_sensitive (wgt->LAN_BUTTON, !h2game);
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
#endif

void ReverseOpt (GtkObject *Unused, int *opt)
{
	(*(opt)) = !(*(opt));
}

void adj_Change (GtkAdjustment *adj, int *opt)
{
	(*(opt)) = (int)(adj->value);
//	printf ("Value is: %d\n", *(opt));
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
