#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "launch_bin.h"
#include "config_file.h"

extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int joystick;
extern int mouse;
extern int destiny;

extern GtkWidget *fixed1;
extern GtkWidget *MIDI_button;
extern GtkWidget *CDAUDIO_button;
extern GtkWidget *MP_button;
extern GtkWidget *_320_button;
extern GtkWidget *_400_button;
extern GtkWidget *_512_button;
extern GtkWidget *_640_button;
extern GtkWidget *_800_button;
extern GtkWidget *_1024_button;
extern GtkWidget *_1280_button;

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{
  gtk_main_quit();
}


void
on_Launch_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  launch_hexen2_bin();
}


void
on_SOUND_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (sound) {
    gtk_widget_set_sensitive (MIDI_button, FALSE);
    gtk_widget_set_sensitive (CDAUDIO_button, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MIDI_button), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (CDAUDIO_button), FALSE);
    midi=0;
    cdaudio=0;
    sound=0;
  }

  else {
    gtk_widget_set_sensitive (MIDI_button, TRUE);
    gtk_widget_set_sensitive (CDAUDIO_button, TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MIDI_button), TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (CDAUDIO_button), TRUE);
    midi=1;
    cdaudio=1;
    sound=1;
  }
}


void
on_MP_button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (mp_support) {
    mp_support=0;
  }

  else {
    mp_support=1;
  }
}


void
on_SOFT_button_released                (GtkButton       *button,
                                        gpointer         user_data)
{
  opengl_support=0;
  gtk_widget_set_sensitive (_800_button, FALSE);
  gtk_widget_set_sensitive (_1024_button, FALSE);
  gtk_widget_set_sensitive (_1280_button, FALSE);
  gtk_widget_hide (_800_button);
  gtk_widget_hide (_1024_button);
  gtk_widget_hide (_1280_button);
  gtk_widget_set_uposition (_512_button, 144, 328);
  gtk_fixed_move (GTK_FIXED (fixed1), _512_button, 144, 328);
  gtk_widget_set_uposition (_640_button, 240, 328);
  gtk_fixed_move (GTK_FIXED (fixed1), _640_button, 240, 328);
  gtk_widget_show (_320_button);
  gtk_widget_show (_400_button);
  if (resolution >= 2 ) {
      resolution = 1;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_400_button), TRUE);
  }
}


void
on_GL_button_released                  (GtkButton       *button,
                                        gpointer         user_data)
{
  opengl_support=1;
  gtk_widget_hide (_320_button);
  gtk_widget_hide (_400_button);
  gtk_widget_set_uposition (_512_button, 144, 304);
  gtk_fixed_move (GTK_FIXED (fixed1), _512_button, 144, 304);
  gtk_widget_set_uposition (_640_button, 240, 304);
  gtk_fixed_move (GTK_FIXED (fixed1), _640_button, 240, 304);
  gtk_widget_show (_800_button);
  gtk_widget_show (_1024_button);
  gtk_widget_show (_1280_button);
  gtk_widget_set_sensitive (_800_button, TRUE);
  gtk_widget_set_sensitive (_1024_button, TRUE);
  gtk_widget_set_sensitive (_1280_button, TRUE);
  if (resolution < 3 ) {
      resolution = 3;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_640_button), TRUE);
  }
}


void
on_320_button_released                 (GtkButton       *button,
                                        gpointer         user_data)
{
   resolution=RES_320;
}


void
on_400_button_released                 (GtkButton       *button,
                                        gpointer         user_data)
{
   resolution=RES_400;
}


void
on_512_button_released                 (GtkButton       *button,
                                        gpointer         user_data)
{
   resolution=RES_512;
}


void
on_640_button_released                 (GtkButton       *button,
                                        gpointer         user_data)
{
  resolution=RES_640;
}


void
on_800_button_released                 (GtkButton       *button,
                                        gpointer         user_data)
{
  resolution=RES_800;
}


void
on_1024_button_released                (GtkButton       *button,
                                        gpointer         user_data)
{
  resolution=RES_1024;
}


void
on_1280_button_released                (GtkButton       *button,
                                        gpointer         user_data)
{
   resolution=RES_1280;
}


void
on_FS_button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
 if (fullscreen) {
   fullscreen=0;
  }

  else {
    fullscreen=1;
  } 
}


void
on_QUIT_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_main_quit();
}


void
on_SAVE_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if (write_config_file() == 0)
    printf("Options saved successfully\n");
}


void
on_HEXEN2_button_released              (GtkButton       *button,
                                        gpointer         user_data)
{
  destiny=DEST_H2;
  gtk_widget_set_sensitive (MP_button, TRUE);
}


void
on_HW_button_released                  (GtkButton       *button,
                                        gpointer         user_data)
{
  destiny=DEST_HW;
  mp_support=0;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MP_button), FALSE);
  gtk_widget_set_sensitive (MP_button, FALSE);
}


void
on_MIDI_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (midi) {
    midi=0;
  }

  else {
    midi=1;
  }

}


void
on_CDAUDIO_button_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (cdaudio) {
    cdaudio=0;
  }

  else {
    cdaudio=1;
  }

}


void
on_MOUS_button_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (mouse) {
    mouse=0;
  }
  else {
    mouse=1;
  }
}


void
on_JOY_button_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (joystick) {
    joystick=0;
  }
  else {
    joystick=1;
  }
}

