/*
 * NE PAS ÉDITER CE FICHIER - il est généré par Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "launch_bin.h"

int mp_support;
int opengl_support;
int fullscreen;
int resolution;
int sound;
int destiny;

GtkWidget *MP_button;
GtkWidget *_800_button;
GtkWidget *_1024_button;
GtkWidget *_640_button;

GtkWidget*
create_window1 (void)
{
  GtkWidget *window1;
  GtkWidget *fixed1;
  GtkWidget *LAUNCH_button;
  GtkWidget *label1;
  GtkWidget *QUIT_button;
  GtkWidget *SAVE_button;
  GtkWidget *label7;
  GtkWidget *FS_button;
  GtkWidget *label6;
  GtkWidget *hseparator1;
  GtkWidget *hseparator2;
  GtkWidget *hseparator3;
  GtkWidget *label3;
  GtkWidget *hseparator4;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label2;
  GSList *renderer_group = NULL;
  GtkWidget *GL_button;
  GSList *resolution_group = NULL;
  //GtkWidget *_640_button;
  GtkWidget *SOFT_button;
  GSList *destiny_group = NULL;
  GtkWidget *HEXEN2_button;
  GtkWidget *HW_button;
  //GtkWidget *MP_button;
  //GtkWidget *_800_button;
  //GtkWidget *_1024_button;
  GtkWidget *SOUND_button;

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  gtk_window_set_title (GTK_WINDOW (window1), _("Hexen II for Linux Launcher"));
  gtk_window_set_policy (GTK_WINDOW (window1), TRUE, TRUE, FALSE);

  fixed1 = gtk_fixed_new ();
  gtk_widget_ref (fixed1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window1), fixed1);

  LAUNCH_button = gtk_button_new_with_label (_("Launch Hexen2"));
  gtk_widget_ref (LAUNCH_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "LAUNCH_button", LAUNCH_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (LAUNCH_button);
  gtk_fixed_put (GTK_FIXED (fixed1), LAUNCH_button, 224, 400);
  gtk_widget_set_uposition (LAUNCH_button, 224, 400);
  gtk_widget_set_usize (LAUNCH_button, 112, 24);

  label1 = gtk_label_new (_("Hammer of Thyrion  1.2.2"));
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_fixed_put (GTK_FIXED (fixed1), label1, 0, 0);
  gtk_widget_set_uposition (label1, 0, 0);
  gtk_widget_set_usize (label1, 160, 24);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  QUIT_button = gtk_button_new_with_label (_("Quit"));
  gtk_widget_ref (QUIT_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "QUIT_button", QUIT_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (QUIT_button);
  gtk_fixed_put (GTK_FIXED (fixed1), QUIT_button, 24, 400);
  gtk_widget_set_uposition (QUIT_button, 24, 400);
  gtk_widget_set_usize (QUIT_button, 54, 24);

  SAVE_button = gtk_button_new_with_label (_("Save Options"));
  gtk_widget_ref (SAVE_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SAVE_button", SAVE_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (SAVE_button);
  gtk_fixed_put (GTK_FIXED (fixed1), SAVE_button, 104, 400);
  gtk_widget_set_uposition (SAVE_button, 104, 400);
  gtk_widget_set_usize (SAVE_button, 96, 24);

  label7 = gtk_label_new (_("Choose your destiny:"));
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_fixed_put (GTK_FIXED (fixed1), label7, 0, 32);
  gtk_widget_set_uposition (label7, 0, 32);
  gtk_widget_set_usize (label7, 152, 16);

  FS_button = gtk_check_button_new_with_label (_("Fullscreen mode"));
  gtk_widget_ref (FS_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "FS_button", FS_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (FS_button);
  gtk_fixed_put (GTK_FIXED (fixed1), FS_button, 168, 264);
  gtk_widget_set_uposition (FS_button, 168, 264);
  gtk_widget_set_usize (FS_button, 136, 24);
  if (fullscreen)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (FS_button), TRUE);

  label6 = gtk_label_new (_("Screen resolution"));
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_fixed_put (GTK_FIXED (fixed1), label6, 32, 328);
  gtk_widget_set_uposition (label6, 32, 328);
  gtk_widget_set_usize (label6, 112, 24);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator1", hseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator1);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator1, 0, 16);
  gtk_widget_set_uposition (hseparator1, 0, 16);
  gtk_widget_set_usize (hseparator1, 352, 16);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator2", hseparator2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator2);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator2, 0, 288);
  gtk_widget_set_uposition (hseparator2, 0, 288);
  gtk_widget_set_usize (hseparator2, 352, 16);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator3", hseparator3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator3);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator3, 0, 80);
  gtk_widget_set_uposition (hseparator3, 0, 80);
  gtk_widget_set_usize (hseparator3, 352, 16);

  label3 = gtk_label_new (_("Hexen II Options"));
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_fixed_put (GTK_FIXED (fixed1), label3, 8, 88);
  gtk_widget_set_uposition (label3, 8, 88);
  gtk_widget_set_usize (label3, 112, 24);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  hseparator4 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator4", hseparator4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator4);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator4, 0, 136);
  gtk_widget_set_uposition (hseparator4, 0, 136);
  gtk_widget_set_usize (hseparator4, 352, 16);

  label4 = gtk_label_new (_("General options"));
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_fixed_put (GTK_FIXED (fixed1), label4, 8, 152);
  gtk_widget_set_uposition (label4, 8, 152);
  gtk_widget_set_usize (label4, 104, 16);

  label5 = gtk_label_new (_("OpenGL options"));
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_fixed_put (GTK_FIXED (fixed1), label5, 16, 304);
  gtk_widget_set_uposition (label5, 16, 304);
  gtk_widget_set_usize (label5, 104, 24);

  label2 = gtk_label_new (_("Renderer"));
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_fixed_put (GTK_FIXED (fixed1), label2, 16, 176);
  gtk_widget_set_uposition (label2, 16, 176);
  gtk_widget_set_usize (label2, 64, 24);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  GL_button = gtk_radio_button_new_with_label (renderer_group, _("OpenGL"));
  renderer_group = gtk_radio_button_group (GTK_RADIO_BUTTON (GL_button));
  gtk_widget_ref (GL_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "GL_button", GL_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (GL_button);
  gtk_fixed_put (GTK_FIXED (fixed1), GL_button, 40, 232);
  gtk_widget_set_uposition (GL_button, 40, 232);
  gtk_widget_set_usize (GL_button, 110, 26);
  if (opengl_support)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (GL_button), TRUE);

  _640_button = gtk_radio_button_new_with_label (resolution_group, _("640x480"));
  resolution_group = gtk_radio_button_group (GTK_RADIO_BUTTON (_640_button));
  gtk_widget_ref (_640_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "_640_button", _640_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (_640_button);
  gtk_fixed_put (GTK_FIXED (fixed1), _640_button, 40, 360);
  gtk_widget_set_uposition (_640_button, 40, 360);
  gtk_widget_set_usize (_640_button, 88, 24);

  SOFT_button = gtk_radio_button_new_with_label (renderer_group, _("Software"));
  renderer_group = gtk_radio_button_group (GTK_RADIO_BUTTON (SOFT_button));
  gtk_widget_ref (SOFT_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SOFT_button", SOFT_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (SOFT_button);
  gtk_fixed_put (GTK_FIXED (fixed1), SOFT_button, 40, 200);
  gtk_widget_set_uposition (SOFT_button, 40, 200);
  gtk_widget_set_usize (SOFT_button, 110, 26);
  if (opengl_support==0)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SOFT_button), TRUE);

  HEXEN2_button = gtk_radio_button_new_with_label (destiny_group, _("Hexen II"));
  destiny_group = gtk_radio_button_group (GTK_RADIO_BUTTON (HEXEN2_button));
  gtk_widget_ref (HEXEN2_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "HEXEN2_button", HEXEN2_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (HEXEN2_button);
  gtk_fixed_put (GTK_FIXED (fixed1), HEXEN2_button, 24, 56);
  gtk_widget_set_uposition (HEXEN2_button, 24, 56);
  gtk_widget_set_usize (HEXEN2_button, 88, 24);
  if (destiny == DEST_H2) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (HEXEN2_button), TRUE);
  }

  HW_button = gtk_radio_button_new_with_label (destiny_group, _("HexenWorld Client"));
  destiny_group = gtk_radio_button_group (GTK_RADIO_BUTTON (HW_button));
  gtk_widget_ref (HW_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "HW_button", HW_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (HW_button);
  gtk_fixed_put (GTK_FIXED (fixed1), HW_button, 128, 56);
  gtk_widget_set_uposition (HW_button, 128, 56);
  gtk_widget_set_usize (HW_button, 152, 24);
  if (destiny == DEST_HW) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (HW_button), TRUE);
  }

  MP_button = gtk_check_button_new_with_label (_("Portal of Praevus Mission Pack support"));
  gtk_widget_ref (MP_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "MP_button", MP_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (MP_button);
  gtk_fixed_put (GTK_FIXED (fixed1), MP_button, 24, 112);
  gtk_widget_set_uposition (MP_button, 24, 112);
  gtk_widget_set_usize (MP_button, 288, 24);
  if (mp_support)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MP_button), TRUE);

  if (destiny==DEST_HW)
    gtk_widget_set_sensitive (MP_button, FALSE);

  else gtk_widget_set_sensitive (MP_button, TRUE);

  _800_button = gtk_radio_button_new_with_label (resolution_group, _("800x600"));
  resolution_group = gtk_radio_button_group (GTK_RADIO_BUTTON (_800_button));
  gtk_widget_ref (_800_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "_800_button", _800_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (_800_button);
  gtk_fixed_put (GTK_FIXED (fixed1), _800_button, 144, 360);
  gtk_widget_set_uposition (_800_button, 144, 360);
  gtk_widget_set_usize (_800_button, 88, 24);
  if (resolution == RES_800)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_800_button), TRUE);

  _1024_button = gtk_radio_button_new_with_label (resolution_group, _("1024x768"));
  resolution_group = gtk_radio_button_group (GTK_RADIO_BUTTON (_1024_button));
  gtk_widget_ref (_1024_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "_1024_button", _1024_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (_1024_button);
  gtk_fixed_put (GTK_FIXED (fixed1), _1024_button, 240, 360);
  gtk_widget_set_uposition (_1024_button, 240, 360);
  gtk_widget_set_usize (_1024_button, 96, 24);
  if (resolution == RES_1024)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (_1024_button), TRUE);

  SOUND_button = gtk_check_button_new_with_label (_("Enable sound"));
  gtk_widget_ref (SOUND_button);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SOUND_button", SOUND_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (SOUND_button);
  gtk_fixed_put (GTK_FIXED (fixed1), SOUND_button, 32, 264);
  gtk_widget_set_uposition (SOUND_button, 32, 264);
  gtk_widget_set_usize (SOUND_button, 117, 26);
  if (sound)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SOUND_button), TRUE);


  if (opengl_support==0) {
    gtk_widget_set_sensitive (_640_button, FALSE);
    gtk_widget_set_sensitive (_800_button, FALSE);
    gtk_widget_set_sensitive (_1024_button, FALSE);
  }

  gtk_signal_connect (GTK_OBJECT (window1), "destroy",
                      GTK_SIGNAL_FUNC (on_window1_destroy),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (LAUNCH_button), "clicked",
                      GTK_SIGNAL_FUNC (on_Launch_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (QUIT_button), "clicked",
                      GTK_SIGNAL_FUNC (on_QUIT_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (SAVE_button), "clicked",
                      GTK_SIGNAL_FUNC (on_SAVE_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (FS_button), "toggled",
                      GTK_SIGNAL_FUNC (on_FS_button_toggled),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (GL_button), "released",
                      GTK_SIGNAL_FUNC (on_GL_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (_640_button), "released",
                      GTK_SIGNAL_FUNC (on_640_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (SOFT_button), "released",
                      GTK_SIGNAL_FUNC (on_SOFT_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (HEXEN2_button), "released",
                      GTK_SIGNAL_FUNC (on_HEXEN2_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (HW_button), "released",
                      GTK_SIGNAL_FUNC (on_HW_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (MP_button), "toggled",
                      GTK_SIGNAL_FUNC (on_MP_button_toggled),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (_800_button), "released",
                      GTK_SIGNAL_FUNC (on_800_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (_1024_button), "released",
                      GTK_SIGNAL_FUNC (on_1024_button_released),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (SOUND_button), "toggled",
                      GTK_SIGNAL_FUNC (on_SOUND_button_toggled),
                      NULL);

  return window1;
}

