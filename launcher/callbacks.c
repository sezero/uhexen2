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
extern int sound;
extern int destiny;

extern GtkWidget *MP_button;
extern GtkWidget *_800_button;
extern GtkWidget *_1024_button;
extern GtkWidget *_640_button;

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
    sound=0;
  }

  else {
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
  gtk_widget_set_sensitive (_640_button, FALSE);
  gtk_widget_set_sensitive (_800_button, FALSE);
  gtk_widget_set_sensitive (_1024_button, FALSE);
}


void
on_GL_button_released                  (GtkButton       *button,
                                        gpointer         user_data)
{
  opengl_support=1;
  gtk_widget_set_sensitive (_640_button, TRUE);
  gtk_widget_set_sensitive (_800_button, TRUE);
  gtk_widget_set_sensitive (_1024_button, TRUE);
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
  gtk_widget_set_sensitive (MP_button, FALSE);
}

