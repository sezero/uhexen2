void res_Change (GtkAdjustment *adj, struct Video_s *wgt);
void UpdateRScale (GtkAdjustment *adj, GtkWidget *label, int ch_scale);
void UpdateStats (struct Launch_s *wgt);
void on_HEXEN2 (GtkButton *button, gamewidget_t *wgt);
void on_H2W (GtkButton *button, gamewidget_t *wgt);
#ifndef DEMOBUILD
void on_H2MP (GtkToggleButton *togglebutton, gamewidget_t *wgt);
void on_OLDM (GtkToggleButton *togglebutton, gpointer user_data);
#endif
void on_SND (GtkToggleButton *togglebutton, sndwidget_t *wgt);
void on_MIDI (GtkToggleButton *togglebutton, gpointer user_data);
void on_CDA (GtkToggleButton *togglebutton, gpointer user_data);
void on_LAN (GtkToggleButton *togglebutton, gpointer user_data);
void on_JOY (GtkToggleButton *togglebutton, gpointer user_data);
void on_OGL (GtkToggleButton *button, gamewidget_t *wgt);
void on_FULS (GtkToggleButton *togglebutton, gpointer user_data);
void on_SAVE (GtkButton *button, gpointer user_data);
