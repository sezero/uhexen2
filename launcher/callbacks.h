#include <gtk/gtk.h>


void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

void
on_Launch_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_SOUND_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_MP_button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_SOFT_button_released                (GtkButton       *button,
                                        gpointer         user_data);

void
on_GL_button_released                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_640_button_released                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_800_button_released                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_1024_button_released                (GtkButton       *button,
                                        gpointer         user_data);

void
on_FS_button_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_QUIT_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_SAVE_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_HEXEN2_button_released              (GtkButton       *button,
                                        gpointer         user_data);

void
on_HW_button_released                  (GtkButton       *button,
                                        gpointer         user_data);
