#include "com_sys.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "config_file.h"
#include "launcher_defs.h"
//#include "support.h"

#ifndef DEMOBUILD
extern int mp_support;
extern int with_om;
#endif
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int joystick;
extern int lan;
extern int destiny;
extern unsigned missingexe;

const char *res_names[]={
  " 320 x 240 ",
  " 400 x 300 ",
  " 512 x 384 ",
  " 640 x 480 ",
  " 800 x 600 ",
  "1024 x 768 ",
  "1280 x 1024"
};

const char *stats[]={

  "   Ready to run the game",
  "   Game binary missing or not executable"
};

void on_SND (GtkToggleButton *togglebutton, sndwidget_t *wgt) {
    sound=!sound;
    gtk_widget_set_sensitive (wgt->MIDI_BUTTON, sound);
    gtk_widget_set_sensitive (wgt->CDAUDIO_BUTTON, sound);
}

#ifndef DEMOBUILD
void on_H2MP (GtkToggleButton *togglebutton, gamewidget_t *wgt) {
  mp_support = !mp_support;
  gtk_widget_set_sensitive (wgt->OLD_MISSION, mp_support);
  UpdateStats(&(wgt->Launch));
}

void on_OLDM (GtkToggleButton *togglebutton, gpointer user_data) {
    with_om=!with_om;
}
#endif

void on_OGL (GtkToggleButton *button, gamewidget_t *wgt) {
  opengl_support=!opengl_support;
  switch (opengl_support) {
  case 0:
	if (resolution > 3 )
		resolution = 3;
	break;
  case 1:
	if (resolution < 3 )
		resolution = 3;
	break;
  }
  UpdateRScale (wgt->RESOL_ADJUST, wgt->RESOL_TEXT1, 1);
  UpdateStats(&(wgt->Launch));
}

void res_Change (GtkAdjustment *adj, struct Video_s *wgt) {
   resolution = (int)(adj->value);
   if ( (adj->value)-resolution > .5 )
   	resolution++;
   UpdateRScale (adj, wgt->RESOL_TEXT0, 0);
}

void UpdateRScale (GtkAdjustment *adj, GtkWidget *label, int ch_scale) {
   gtk_adjustment_set_value (adj, resolution);
   if (ch_scale) {
	adj->lower=2*opengl_support;
	adj->upper=3+3*opengl_support;
	gtk_signal_emit_by_name (GTK_OBJECT (adj), "changed");
   }
   gtk_label_set_text (GTK_LABEL (label), res_names[resolution]);
}

void UpdateStats (struct Launch_s *wgt) {
   CheckExe();
   gtk_statusbar_push (GTK_STATUSBAR (wgt->STATUSBAR), wgt->BinStat, stats[missingexe]);
   gtk_widget_set_sensitive (wgt->LAUNCH_BUTTON, !missingexe);
}

void on_FULS (GtkToggleButton *togglebutton, gpointer user_data) {
  fullscreen=!fullscreen;
}

void on_SAVE (GtkButton *button, gpointer user_data) {
  if (write_config_file() == 0)
    printf("Options saved successfully\n");
}

void on_HEXEN2 (GtkButton *button, gamewidget_t *wgt) {
  destiny=DEST_H2;
#ifndef DEMOBUILD
  gtk_widget_set_sensitive (wgt->PORTALS_BUTTON, TRUE);
  if(mp_support)
    gtk_widget_set_sensitive (wgt->OLD_MISSION, TRUE);
#endif
  gtk_widget_set_sensitive (wgt->LAN_BUTTON, TRUE);
  UpdateStats(&(wgt->Launch));
}

void on_H2W (GtkButton *button, gamewidget_t *wgt) {
  destiny=DEST_HW;
#ifndef DEMOBUILD
  if(mp_support)
    gtk_widget_set_sensitive (wgt->OLD_MISSION, FALSE);
  gtk_widget_set_sensitive (wgt->PORTALS_BUTTON, FALSE);
#endif
  gtk_widget_set_sensitive (wgt->LAN_BUTTON, FALSE);
  UpdateStats(&(wgt->Launch));
}

void on_MIDI (GtkToggleButton *togglebutton, gpointer user_data) {
  midi=!midi;
}

void on_CDA (GtkToggleButton *togglebutton, gpointer user_data) {
  cdaudio=!cdaudio;
}

void on_LAN (GtkToggleButton *togglebutton, gpointer user_data) {
  lan=!lan;
}

void on_JOY (GtkToggleButton *togglebutton, gpointer user_data) {
  joystick=!joystick;
}
