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

