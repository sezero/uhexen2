void res_Change (GtkAdjustment *adj, struct Video_s *wgt);
void UpdateRScale (GtkAdjustment *adj, GtkWidget *label, int ch_scale);
void UpdateStats (struct Launch_s *wgt);
void on_HEXEN2 (GtkButton *button, gamewidget_t *wgt);
void on_H2W (GtkButton *button, gamewidget_t *wgt);
#ifndef DEMOBUILD
void on_H2MP (GtkToggleButton *togglebutton, gamewidget_t *wgt);
void HWGameChange (GtkEditable *editable, gpointer user_data);
#endif
void on_SND (GtkEditable *editable, sndwidget_t *wgt);
void on_OGL (GtkToggleButton *button, gamewidget_t *wgt);

// having to add a not-to-be-used arg to a function, so nice..
void ReverseOpt (GtkObject *Unused, int *opt);
