#include <gdk/gdkkeysyms.h>
#include "launcher_defs.h"
#include "com_sys.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "support.h"
#include "config_file.h"

#ifndef DEMOBUILD
extern int mp_support;
extern int with_om;
#endif
unsigned short i;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int joystick;
extern int lan;
extern int destiny;
extern int hwgame;
extern unsigned missingexe;
extern const char *res_names[];
extern const char *hwgame_names[MAX_HWGAMES][2];
extern const char *snddrv_names[MAX_SOUND][2];

/*********************************************************************/

// Here we begin

GtkWidget* create_window1 (void)
{
  GtkWidget *window1;
  GtkWidget *fixed1;

  char *Title;
  GtkWidget *TxtTitle;	// Title Label
  GtkWidget *TxtGame0;	// Destiny label
  GtkWidget *TxtGame1;	// Hexen2 options
  GtkWidget *TxtGame2;	// Miss.Pack options
  GtkWidget *TxtVideo;	// Renderer, etc.
  GtkWidget *TxtSound;	// Sound options.
  GtkWidget *TxtResol;	// Resolution
  GtkWidget *hseparator1;
  GtkWidget *hseparator2;
  GtkWidget *hseparator3;
  GtkWidget *hseparator4;
  GtkWidget *vseparator1;

  static gamewidget_t Games;
  static sndwidget_t Sound;

  GSList *Destinies = NULL;
  GList *TmpList = NULL;
  GtkWidget *HWG_Entry;
  GtkWidget *SND_Entry;

  GtkWidget *bJOY;
  GtkWidget *bSAVE;
  GtkWidget *bQUIT;

#ifdef HAVE_GTK2
  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();
#endif

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  // here is a template for the final Title string
  // changes should be consistent for malloc().
  Title = (char *)malloc(sizeof("Linux Hexen2 demo Launcher, v9.99.9"));
#ifndef DEMOBUILD
  strcpy (Title, "Linux Hexen2 Launcher, v");
#else
  strcpy (Title, "Linux Hexen2 demo Launcher, v");
#endif
  strcat (Title, HOTL_VER);	// from our Makefile
  gtk_window_set_title (GTK_WINDOW (window1), Title);
  free (Title);
  gtk_window_set_policy (GTK_WINDOW (window1), TRUE, TRUE, FALSE);
  gtk_window_set_default_size (GTK_WINDOW (window1), -1, 384);

  fixed1 = gtk_fixed_new ();
  gtk_widget_ref (fixed1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "fixed1", fixed1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window1), fixed1);

/*********************************************************************/
  TxtTitle = gtk_label_new (_("Hammer of Thyrion 1.2.3"));
  gtk_widget_ref (TxtTitle);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtTitle", TxtTitle,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtTitle);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtTitle, 6, 8);
  gtk_widget_set_size_request (TxtTitle, 160, 24);
  gtk_label_set_justify (GTK_LABEL (TxtTitle), GTK_JUSTIFY_LEFT);

/*********************************************************************/
  WGT_LAUNCH = gtk_button_new_with_label (_("Launch Hexen2"));
  gtk_widget_ref (WGT_LAUNCH);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bLAUNCH", WGT_LAUNCH,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LAUNCH);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_LAUNCH, 16, 328);
  gtk_widget_set_size_request (WGT_LAUNCH, 112, 24);
//gtk_widget_set_sensitive (WGT_LAUNCH, !missingexe);

  WGT_STATUSBAR = gtk_statusbar_new ();
  gtk_widget_ref (WGT_STATUSBAR);
  gtk_object_set_data_full (GTK_OBJECT (window1), "LStat", WGT_STATUSBAR,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_STATUSBAR);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_STATUSBAR, 0, 358);
  gtk_widget_set_size_request (WGT_STATUSBAR, 340, 24);
  gtk_container_set_border_width (GTK_CONTAINER (WGT_STATUSBAR), 2);
  BIN_STATUS_ID = gtk_statusbar_get_context_id (GTK_STATUSBAR (WGT_STATUSBAR), "BinaryStatus");
  gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), BIN_STATUS_ID,_("   Waiting status..."));

/*********************************************************************/
  TxtVideo = gtk_label_new (_("Graphics :"));
  gtk_widget_ref (TxtVideo);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtVideo", TxtVideo,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtVideo);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtVideo, 4, 186);
  gtk_widget_set_size_request (TxtVideo, 80, 24);
  gtk_label_set_justify (GTK_LABEL (TxtVideo), GTK_JUSTIFY_LEFT);

  WGT_OPENGL = gtk_check_button_new_with_label (_("OpenGL support"));
  gtk_widget_ref (WGT_OPENGL);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bOGL", WGT_OPENGL,
                            (GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_OPENGL, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_OPENGL);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_OPENGL, 16, 212);
  gtk_widget_set_size_request (WGT_OPENGL, 136, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_OPENGL), opengl_support);

  WGT_FULLSCR = gtk_check_button_new_with_label (_("Fullscreen mode"));
  gtk_widget_ref (WGT_FULLSCR);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bFULS", WGT_FULLSCR,
                            (GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_FULLSCR, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_FULLSCR);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_FULLSCR, 16, 236);
  gtk_widget_set_size_request (WGT_FULLSCR, 136, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_FULLSCR), fullscreen);

  TxtResol = gtk_label_new (_("Resolution:"));
  gtk_widget_ref (TxtResol);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtResol", TxtResol,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtResol);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtResol, 12, 262);
  gtk_widget_set_size_request (TxtResol, 72, 24);

  WGT_RESTEXT = gtk_label_new (_(res_names[resolution]));
  gtk_widget_ref (WGT_RESTEXT);
  gtk_object_set_data_full (GTK_OBJECT (window1), "rText", WGT_RESTEXT,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_RESTEXT);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_RESTEXT, 78, 264);
  gtk_widget_set_size_request (WGT_RESTEXT, 80, 20);

  WGT_RES_ADJUST = gtk_adjustment_new (resolution, 2*opengl_support, 3+3*opengl_support, 1, 1, 0);
  WGT_RES_SCALE = gtk_hscale_new (WGT_RES_ADJUST);
  gtk_object_set_data_full (GTK_OBJECT (window1), "rScale", WGT_RES_SCALE,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_RES_SCALE);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_RES_SCALE, 16, 286);
  gtk_widget_set_size_request (WGT_RES_SCALE, 140, 24);
  gtk_scale_set_draw_value (GTK_SCALE (WGT_RES_SCALE), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (WGT_RES_SCALE), GTK_POS_RIGHT);
  GTK_WIDGET_UNSET_FLAGS (WGT_RES_SCALE, GTK_CAN_FOCUS);
  gtk_range_set_update_policy (GTK_RANGE (WGT_RES_SCALE), GTK_UPDATE_DELAYED);

/*********************************************************************/
  bSAVE = gtk_button_new_with_label (_("Save Options"));
  gtk_widget_ref (bSAVE);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bSAVE", bSAVE,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bSAVE);
  gtk_fixed_put (GTK_FIXED (fixed1), bSAVE, 148, 328);
  gtk_widget_set_size_request (bSAVE, 96, 24);

  bQUIT = gtk_button_new_with_label (_("Quit"));
  gtk_widget_ref (bQUIT);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bQUIT", bQUIT,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bQUIT);
  gtk_fixed_put (GTK_FIXED (fixed1), bQUIT, 264, 328);
  gtk_widget_set_size_request (bQUIT, 64, 24);

/*********************************************************************/
  TxtGame0 = gtk_label_new (_("Choose your destiny:"));
  gtk_widget_ref (TxtGame0);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGame0", TxtGame0,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGame0);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGame0, 0, 44);
  gtk_widget_set_size_request (TxtGame0, 152, 16);

  WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, _("Hexen II"));
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_HEXEN2));
  gtk_widget_ref (WGT_HEXEN2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bHEXEN2", WGT_HEXEN2,
                            (GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_HEXEN2, GTK_CAN_FOCUS);
#ifdef HAVE_GTK2
#ifdef DEMOBUILD
  gtk_tooltips_set_tip (tooltips, WGT_HEXEN2, _("Hexen II demo"), NULL);
#else
  gtk_tooltips_set_tip (tooltips, WGT_HEXEN2, _("Hexen II game"), NULL);
#endif
#endif
  gtk_widget_show (WGT_HEXEN2);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_HEXEN2, 24, 64);
  gtk_widget_set_size_request (WGT_HEXEN2, 132, 24);
  if (destiny == DEST_H2)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_HEXEN2), TRUE);

  WGT_H2WORLD = gtk_radio_button_new_with_label (Destinies, _("HexenWorld Client"));
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_H2WORLD));
  gtk_widget_ref (WGT_H2WORLD);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bH2W", WGT_H2WORLD,
                            (GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_H2WORLD, GTK_CAN_FOCUS);
#ifdef HAVE_GTK2
  gtk_tooltips_set_tip (tooltips, WGT_H2WORLD, _("Multiplayer-only\nClient"), NULL);
#endif
  gtk_widget_show (WGT_H2WORLD);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_H2WORLD, 24, 90);
  gtk_widget_set_size_request (WGT_H2WORLD, 132, 24);
  if (destiny == DEST_HW)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_H2WORLD), TRUE);

  WGT_HWGAME = gtk_combo_new ();
  gtk_widget_ref (WGT_HWGAME);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SelHW", WGT_HWGAME,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_HWGAME), FALSE);
  gtk_widget_set_size_request (WGT_HWGAME, 140, 24);
  TmpList = g_list_append (TmpList, (gpointer) "DeathMatch");
  Title = (char *)malloc(32);
#ifndef DEMOBUILD
  for (i=1; i<MAX_HWGAMES; i++) {
	strcpy (Title, hwgame_names[i][0]);
	strcat (Title, "/hwprogs.dat");
	printf("Looking for %s (%s) ... ", Title, (char *)hwgame_names[i][1]);
	if (access(Title, R_OK) == 0) {
		TmpList = g_list_append (TmpList, (char *)hwgame_names[i][1]);
		printf("Found OK.\n");
	} else
		printf("NOT found.\n");
  }
#endif
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_HWGAME), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_HWGAME, 170, 90);
  gtk_widget_show (WGT_HWGAME);
  HWG_Entry = GTK_COMBO (WGT_HWGAME)->entry;
  gtk_widget_ref (HWG_Entry);
  gtk_object_set_data_full (GTK_OBJECT (window1), "HWG_Entry", HWG_Entry,
			    (GtkDestroyNotify) gtk_widget_unref);
#ifndef DEMOBUILD
  if (hwgame > 0) {
	strcpy (Title, hwgame_names[hwgame][0]);
	strcat (Title, "/hwprogs.dat");
	if (access(Title, R_OK) == 0)
	    gtk_entry_set_text (GTK_ENTRY (HWG_Entry), (char *)hwgame_names[hwgame][1]);
	else {
	    gtk_entry_set_text (GTK_ENTRY (HWG_Entry), "DeathMatch");
	    hwgame = 0;
	}
  } else
#endif
	gtk_entry_set_text (GTK_ENTRY (HWG_Entry), "DeathMatch");
  free (Title);
  gtk_entry_set_editable (GTK_ENTRY (HWG_Entry), FALSE);
  gtk_widget_show (HWG_Entry);
  if (destiny != DEST_HW)
      gtk_widget_set_sensitive (WGT_HWGAME, FALSE);

/*********************************************************************/
  TxtGame1 = gtk_label_new (_("Hexen II options :"));
  gtk_widget_ref (TxtGame1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGame1", TxtGame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGame1);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGame1, 16, 128);
  gtk_widget_set_size_request (TxtGame1, 100, 24);

  WGT_PORTALS = gtk_check_button_new_with_label (_("with Mission Pack"));
  gtk_widget_ref (WGT_PORTALS);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bH2MP", WGT_PORTALS,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_PORTALS, 24, 150);
  gtk_widget_show (WGT_PORTALS);
#ifndef DEMOBUILD
#ifdef HAVE_GTK2
  gtk_tooltips_set_tip (tooltips, WGT_PORTALS, _("play Hexen II with\nPortal of Praevus support"), NULL);
#endif
  GTK_WIDGET_UNSET_FLAGS (WGT_PORTALS, GTK_CAN_FOCUS);
  gtk_widget_set_size_request (WGT_PORTALS, 128, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_PORTALS), mp_support);
  if (destiny==DEST_HW)
      gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
#else
  gtk_widget_set_sensitive (WGT_PORTALS, FALSE);
#endif

/*********************************************************************/
  TxtGame2 = gtk_label_new (_("Mission Pack options :"));
  gtk_widget_ref (TxtGame2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGame2", TxtGame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGame2);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGame2, 170, 128);
  gtk_widget_set_size_request (TxtGame2, 128, 24);
  gtk_label_set_justify (GTK_LABEL (TxtGame2), GTK_JUSTIFY_LEFT);

  WGT_OLDMISS = gtk_check_button_new_with_label (_("Enable the Old Mission"));
  gtk_widget_ref (WGT_OLDMISS);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bOLDM", WGT_OLDMISS,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_OLDMISS, 172, 150);
#ifndef DEMOBUILD
#ifdef HAVE_GTK2
  gtk_tooltips_set_tip (tooltips, WGT_OLDMISS, _("Support the Old Mission\nin Portal of Praevus"), NULL);
#endif
  GTK_WIDGET_UNSET_FLAGS (WGT_OLDMISS, GTK_CAN_FOCUS);
  gtk_widget_set_size_request (WGT_OLDMISS, 160, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_OLDMISS), with_om);
  if ((destiny==DEST_HW) || (!mp_support))
      gtk_widget_set_sensitive (WGT_OLDMISS, FALSE);
#else
  gtk_widget_set_sensitive (WGT_OLDMISS, FALSE);
#endif
  gtk_widget_show (WGT_OLDMISS);


/*********************************************************************/
// Sound options

  TxtSound = gtk_label_new (_("Sound:"));
  gtk_widget_ref (TxtSound);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtSound", TxtSound,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtSound, 175, 188);
  gtk_widget_set_size_request (TxtSound, 50, 24);
  gtk_label_set_justify (GTK_LABEL (TxtSound), GTK_JUSTIFY_LEFT);

  WGT_SOUND = gtk_combo_new ();
  gtk_widget_ref (WGT_SOUND);
  gtk_object_set_data_full (GTK_OBJECT (window1), "cSND", WGT_SOUND,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_SOUND), FALSE);
  gtk_widget_set_size_request (WGT_SOUND, 100, 24);
  TmpList = NULL;
  for (i=0; i<MAX_SOUND; i++)
		TmpList = g_list_append (TmpList, (char *)snddrv_names[i][1]);
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_SOUND), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_SOUND, 225, 188);
  gtk_widget_show (WGT_SOUND);
  SND_Entry = GTK_COMBO (WGT_SOUND)->entry;
  gtk_widget_ref (SND_Entry);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SND_Entry", SND_Entry,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (SND_Entry), (char *)snddrv_names[sound][1]);
  gtk_entry_set_editable (GTK_ENTRY (SND_Entry), FALSE);
  gtk_widget_show (SND_Entry);

  WGT_MIDI = gtk_check_button_new_with_label (_("Disable MIDI music"));
  gtk_widget_ref (WGT_MIDI);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bMIDI", WGT_MIDI,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MIDI);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_MIDI, 196, 236);
  gtk_widget_set_size_request (WGT_MIDI, 140, 24);
  gtk_widget_set_sensitive (WGT_MIDI, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MIDI), !midi);

  WGT_CDAUDIO = gtk_check_button_new_with_label (_("Disable CD music"));
  gtk_widget_ref (WGT_CDAUDIO);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bCDA", WGT_CDAUDIO,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_CDAUDIO);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_CDAUDIO, 196, 212);
  gtk_widget_set_size_request (WGT_CDAUDIO, 140, 24);
  gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_CDAUDIO), !cdaudio);

/*********************************************************************/
  WGT_LANBUTTON = gtk_check_button_new_with_label (_("Disable Networking"));
  gtk_widget_ref (WGT_LANBUTTON);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bLAN", WGT_LANBUTTON,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LANBUTTON);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_LANBUTTON, 180, 284);
  gtk_widget_set_size_request (WGT_LANBUTTON, 136, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_LANBUTTON), !lan);
  if (destiny==DEST_HW)
      gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);

/*********************************************************************/
  bJOY = gtk_check_button_new_with_label (_("Disable Joystick"));
  gtk_widget_ref (bJOY);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bJOY", bJOY,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bJOY);
  gtk_fixed_put (GTK_FIXED (fixed1), bJOY, 180, 260);
  gtk_widget_set_size_request (bJOY, 136, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bJOY), !joystick);

/*********************************************************************/
// Separators

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator1", hseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator1);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator1, 0, 28);
  gtk_widget_set_size_request (hseparator1, 340, 16);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator2", hseparator2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator2);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator2, 0, 116);
  gtk_widget_set_size_request (hseparator2, 340, 16);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator3", hseparator3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator3);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator3, 0, 172);
  gtk_widget_set_size_request (hseparator3, 340, 16);

  hseparator4 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator4);
  gtk_object_set_data_full (GTK_OBJECT (window1), "hseparator4", hseparator4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator4);
  gtk_fixed_put (GTK_FIXED (fixed1), hseparator4, 0, 308);
  gtk_widget_set_size_request (hseparator4, 340, 16);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_ref (vseparator1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "vseparator1", vseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vseparator1);
  gtk_fixed_put (GTK_FIXED (fixed1), vseparator1, 160, 180);
  gtk_widget_set_size_request (vseparator1, 16, 136);


/*********************************************************************/
  gtk_signal_connect (GTK_OBJECT (window1), "destroy",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_LAUNCH), "clicked",
			GTK_SIGNAL_FUNC (launch_hexen2_bin), NULL);
  gtk_signal_connect (GTK_OBJECT (bQUIT), "clicked",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (bSAVE), "clicked",
			GTK_SIGNAL_FUNC (write_config_file), NULL);
#ifndef DEMOBUILD
  gtk_signal_connect (GTK_OBJECT (HWG_Entry), "changed",
			GTK_SIGNAL_FUNC (HWGameChange), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_PORTALS), "toggled",
			GTK_SIGNAL_FUNC (on_H2MP), &Games);
  gtk_signal_connect (GTK_OBJECT (WGT_OLDMISS), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &with_om);
#endif
  gtk_signal_connect (GTK_OBJECT (WGT_HEXEN2), "released",
			GTK_SIGNAL_FUNC (on_HEXEN2), &Games);
  gtk_signal_connect (GTK_OBJECT (WGT_H2WORLD), "released",
			GTK_SIGNAL_FUNC (on_H2W), &Games);
  gtk_signal_connect (GTK_OBJECT (WGT_OPENGL), "released",
			GTK_SIGNAL_FUNC (on_OGL), &Games);
  gtk_signal_connect (GTK_OBJECT (SND_Entry), "changed",
			GTK_SIGNAL_FUNC (on_SND), &Sound);
  gtk_signal_connect (GTK_OBJECT (WGT_MIDI), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &midi);
  gtk_signal_connect (GTK_OBJECT (WGT_CDAUDIO), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &cdaudio);
  gtk_signal_connect (GTK_OBJECT (WGT_LANBUTTON), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &lan);
  gtk_signal_connect (GTK_OBJECT (bJOY), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &joystick);
  gtk_signal_connect (GTK_OBJECT (WGT_FULLSCR), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &fullscreen);
  gtk_signal_connect (GTK_OBJECT (WGT_RES_ADJUST), "value_changed",
			GTK_SIGNAL_FUNC (res_Change), &VID_STRUCT);
#ifdef HAVE_GTK2
  gtk_object_set_data (GTK_OBJECT (window1), "tooltips", tooltips);
#endif

  UpdateStats(&LAUNCH_STRUCT);

  return window1;
}
