#include <gdk/gdkkeysyms.h>
#include "launcher_defs.h"
#include "com_sys.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "support.h"
#include "config_file.h"

unsigned short i;
// from config_file.c
extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int sndrate;
extern int sndbits;
extern int stereo;
extern int lan;
extern int destiny;
// from launch_bin.c
extern unsigned missingexe;
extern const char *res_names[];
extern const char *snddrv_names[MAX_SOUND][2];
extern const char *snd_rates[MAX_RATES];
#ifndef DEMOBUILD
extern int hwgame;
extern const char *h2game_names[MAX_H2GAMES][2];
extern const char *hwgame_names[MAX_HWGAMES][2];
#endif

/*********************************************************************/

GtkWidget* create_window1 (void)
{
  GtkWidget *window1;
  GtkWidget *fixed1;

  char *Title;
  GList *TmpList = NULL;
  GSList *Destinies = NULL;

// for basics
  GtkWidget *TxtTitle;	// Title Label
  GtkWidget *TxtGame0;	// Destiny label
  GtkWidget *TxtVideo;	// Renderer, etc.
  GtkWidget *TxtResol;	// Resolution
  GtkWidget *TxtSound;	// Sound driver combo
  GtkWidget *SND_Entry; // Sound driver listing
  GtkWidget *bSAVE;	// Save options button
  GtkWidget *bQUIT;	// Quit button

// for additionals
  GtkWidget *bMORE;	// More/Less button
  GtkWidget *TxtNet;	// Networking options.
  GtkWidget *TxtSound2;	// Sound options extra
  GtkWidget *TxtSound3;	// Sound options extra
  GtkWidget *SRATE_Entry; // Sampling rate listing
  GtkWidget *TxtGameT;	// GameType Label
  GtkWidget *TxtGameH2;	// Hexen2 GameType Label
  GtkWidget *H2G_Entry;	// Hexen2 games listing
  GtkWidget *TxtGameHW;	// Hexenworld GameType Label
  GtkWidget *HWG_Entry;	// Hexenworld games listing

  static gamewidget_t Games;
  static sndwidget_t Sound;

  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();

/*********************************************************************/
// Here we begin

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  // here is a template for the final Title string
  // changes should be consistent for malloc().
  Title = (char *)malloc(sizeof("Hexen2 demo Launcher 9.99.9"));
#ifndef DEMOBUILD
  strcpy (Title, "Hexen2 Launcher ");
#else
  strcpy (Title, "Hexen2 demo Launcher ");
#endif
  strcat (Title, HOTL_VER);	// from our Makefile
  gtk_window_set_title (GTK_WINDOW (window1), Title);
  free (Title);
  gtk_window_set_resizable (GTK_WINDOW (window1), FALSE);
  gtk_widget_set_size_request(window1, 230, 352);

  fixed1 = gtk_fixed_new ();
  gtk_widget_ref (fixed1);
  gtk_object_set_data_full (GTK_OBJECT (window1), "fixed1", fixed1,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (window1), fixed1);

/*********************************************************************/

  TxtTitle = gtk_label_new ("Hammer of Thyrion 1.2.4");
  gtk_widget_ref (TxtTitle);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtTitle", TxtTitle,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtTitle);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtTitle, 14, 14);
  gtk_label_set_justify (GTK_LABEL (TxtTitle), GTK_JUSTIFY_LEFT);


/********************************************************************
		BASIC OPTIONS
 ********************************************************************/

// Quit button
  bQUIT = gtk_button_new_with_label (_("Quit"));
  gtk_widget_ref (bQUIT);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bQUIT", bQUIT,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bQUIT);
  gtk_fixed_put (GTK_FIXED (fixed1), bQUIT, 132, 300);
  gtk_widget_set_size_request (bQUIT, 80, 24);

// Save the options
  bSAVE = gtk_button_new_with_label (_("Save Options"));
  gtk_widget_ref (bSAVE);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bSAVE", bSAVE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bSAVE);
  gtk_fixed_put (GTK_FIXED (fixed1), bSAVE, 12, 300);
  gtk_widget_set_size_request (bSAVE, 112, 24);

/*********************************************************************/

// Launch button and Statusbar
  WGT_LAUNCH = gtk_button_new_with_label (_("Launch Hexen2"));
  gtk_widget_ref (WGT_LAUNCH);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bLAUNCH", WGT_LAUNCH,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LAUNCH);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_LAUNCH, 12, 272);
  gtk_widget_set_size_request (WGT_LAUNCH, 112, 24);

  WGT_STATUSBAR = gtk_statusbar_new ();
  gtk_widget_ref (WGT_STATUSBAR);
  gtk_object_set_data_full (GTK_OBJECT (window1), "LStat", WGT_STATUSBAR,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_STATUSBAR);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_STATUSBAR, 0, 328);
  gtk_widget_set_size_request (WGT_STATUSBAR, 230, 24);
  gtk_container_set_border_width (GTK_CONTAINER (WGT_STATUSBAR), 2);
  BIN_STATUS_ID = gtk_statusbar_get_context_id (GTK_STATUSBAR (WGT_STATUSBAR), "BinaryStatus");
  gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), BIN_STATUS_ID,_("  Waiting status..."));

/*********************************************************************/

// DESTINY : Game/binary selection
  TxtGame0 = gtk_label_new (_("Choose your destiny:"));
  gtk_widget_ref (TxtGame0);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGame0", TxtGame0,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGame0);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGame0, 14, 44);
  gtk_label_set_justify (GTK_LABEL (TxtGame0), GTK_JUSTIFY_LEFT);

// Destiny: Hexen2
#ifdef DEMOBUILD
  WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II demo");
#else
  WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II");
#endif
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_HEXEN2));
  gtk_widget_ref (WGT_HEXEN2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bHEXEN2", WGT_HEXEN2,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_HEXEN2, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_HEXEN2);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_HEXEN2, 24, 64);
#ifndef DEMOBUILD
  gtk_widget_set_size_request (WGT_HEXEN2, 80, 24);
#else
  gtk_widget_set_size_request (WGT_HEXEN2, 180, 24);
#endif
  if ((destiny == DEST_H2) && !mp_support)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_HEXEN2), TRUE);

// Destiny: H2MP
#ifndef DEMOBUILD
  WGT_PORTALS = gtk_radio_button_new_with_label (Destinies, "Praevus");
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_PORTALS));
  gtk_widget_ref (WGT_PORTALS);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bH2MP", WGT_PORTALS,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_PORTALS, 112, 64);
  gtk_widget_show (WGT_PORTALS);
  gtk_tooltips_set_tip (tooltips, WGT_PORTALS, _("play Hexen II with Mission Pack"), NULL);
  GTK_WIDGET_UNSET_FLAGS (WGT_PORTALS, GTK_CAN_FOCUS);
  gtk_widget_set_size_request (WGT_PORTALS, 80, 24);
  if ((destiny == DEST_H2) && mp_support)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_PORTALS), TRUE);
#endif

// Destiny: HexenWorld
  WGT_H2WORLD = gtk_radio_button_new_with_label (Destinies, "HexenWorld Multiplayer");
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_H2WORLD));
  gtk_widget_ref (WGT_H2WORLD);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bH2W", WGT_H2WORLD,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_H2WORLD, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_H2WORLD);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_H2WORLD, 24, 90);
  gtk_widget_set_size_request (WGT_H2WORLD, 180, 24);
  if (destiny == DEST_HW)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_H2WORLD), TRUE);

/*********************************************************************/

// Video Options
  TxtVideo = gtk_label_new (_("Graphics  :"));
  gtk_widget_ref (TxtVideo);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtVideo", TxtVideo,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtVideo);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtVideo, 14, 166);
  gtk_label_set_justify (GTK_LABEL (TxtVideo), GTK_JUSTIFY_LEFT);

// opengl/software toggle
  WGT_OPENGL = gtk_check_button_new_with_label ("OpenGL");
  gtk_widget_ref (WGT_OPENGL);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bOGL", WGT_OPENGL,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_OPENGL, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_OPENGL);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_OPENGL, 84, 162);
  gtk_widget_set_size_request (WGT_OPENGL, 108, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_OPENGL), opengl_support);

// fullscreen/windowed toggle
  WGT_FULLSCR = gtk_check_button_new_with_label (_("Fullscreen"));
  gtk_widget_ref (WGT_FULLSCR);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bFULS", WGT_FULLSCR,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_FULLSCR, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_FULLSCR);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_FULLSCR, 84, 186);
  gtk_widget_set_size_request (WGT_FULLSCR, 108, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_FULLSCR), fullscreen);

// resolution
  TxtResol = gtk_label_new (_("Resolution:"));
  gtk_widget_ref (TxtResol);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtResol", TxtResol,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtResol);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtResol, 14, 218);
  gtk_label_set_justify (GTK_LABEL (TxtResol), GTK_JUSTIFY_LEFT);

//resolution combo
  WGT_RESCOMBO = gtk_combo_new ();
  gtk_widget_ref (WGT_RESCOMBO);
  gtk_object_set_data_full (GTK_OBJECT (window1), "cRES", WGT_RESCOMBO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_RESCOMBO), FALSE);
  gtk_widget_set_size_request (WGT_RESCOMBO, 108, 24);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_RESCOMBO, 84, 214);
//resolution display
  WGT_RESLIST = GTK_COMBO (WGT_RESCOMBO)->entry;
  gtk_widget_ref (WGT_RESLIST);
  gtk_object_set_data_full (GTK_OBJECT (window1), "eRES", WGT_RESLIST,
				(GtkDestroyNotify) gtk_widget_unref);
  //gtk_entry_set_alignment (GTK_ENTRY (WGT_RESLIST), 1);
  gtk_entry_set_editable (GTK_ENTRY (WGT_RESLIST), FALSE);
//menu listing from a callback
  Make_ResMenu(&VID_STRUCT);
  gtk_widget_show (WGT_RESCOMBO);
  gtk_widget_show (WGT_RESLIST);

/*********************************************************************/

// Sound options (basic: driver selection)
  TxtSound = gtk_label_new (_("Sound     :"));
  gtk_widget_ref (TxtSound);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtSound", TxtSound,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtSound, 14, 136);
  gtk_label_set_justify (GTK_LABEL (TxtSound), GTK_JUSTIFY_LEFT);

  WGT_SOUND = gtk_combo_new ();
  gtk_widget_ref (WGT_SOUND);
  gtk_object_set_data_full (GTK_OBJECT (window1), "cSND", WGT_SOUND,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_SOUND), FALSE);
  gtk_widget_set_size_request (WGT_SOUND, 108, 24);
  TmpList = NULL;
  for (i=0; i<MAX_SOUND; i++)
	TmpList = g_list_append (TmpList, (char *)snddrv_names[i][1]);
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_SOUND), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_SOUND, 84, 132);
  gtk_widget_show (WGT_SOUND);
  SND_Entry = GTK_COMBO (WGT_SOUND)->entry;
  gtk_widget_ref (SND_Entry);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SND_Entry", SND_Entry,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (SND_Entry), (char *)snddrv_names[sound][1]);
  gtk_entry_set_editable (GTK_ENTRY (SND_Entry), FALSE);
//gtk_entry_set_alignment (GTK_ENTRY (SND_Entry), 1);
  gtk_widget_show (SND_Entry);


/********************************************************************
		ADDITIONAL OPTIONS
 ********************************************************************/

  bMORE = gtk_button_new_with_label (_("More >>>"));
  gtk_widget_ref (bQUIT);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bMORE", bMORE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bMORE);
  gtk_fixed_put (GTK_FIXED (fixed1), bMORE, 132, 272);
  gtk_widget_set_size_request (bMORE, 80, 24);

/*********************************************************************/

// Additional game-types
  TxtGameT = gtk_label_new (_("Extra Game Types:"));
  gtk_widget_ref (TxtGameT);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGameT", TxtGameT,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameT);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGameT, 232, 44);
  gtk_label_set_justify (GTK_LABEL (TxtGameT), GTK_JUSTIFY_LEFT);

// game types menu for hexen2
  TxtGameH2 = gtk_label_new ("Hexen2:");
  gtk_widget_ref (TxtGameH2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGameH2", TxtGameH2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameH2);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGameH2, 232, 68);
  gtk_label_set_justify (GTK_LABEL (TxtGameH2), GTK_JUSTIFY_LEFT);

// game types menu for hexen2
// there are two botmatch mods only...
  WGT_H2GAME = gtk_combo_new ();
  gtk_widget_ref (WGT_H2GAME);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SelH2", WGT_H2GAME,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_size_request (WGT_H2GAME, 132, 24);
  TmpList = NULL;
  TmpList = g_list_append (TmpList, (gpointer) "(  None  )");
#ifndef DEMOBUILD
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_H2GAME), FALSE);
  Title = (char *)malloc(32);
  for (i=1; i<MAX_H2GAMES; i++) {
	strcpy (Title, h2game_names[i][0]);
	strcat (Title, "/progs.dat");
	printf("Looking for %s (%s) ... ", Title, (char *)h2game_names[i][1]);
	if (access(Title, R_OK) == 0) {
		TmpList = g_list_append (TmpList, (char *)h2game_names[i][1]);
		printf("Found OK.\n");
	} else
		printf("NOT found.\n");
  }
  free (Title);
#endif
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_H2GAME), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_H2GAME, 280, 64);
  gtk_widget_show (WGT_H2GAME);
  H2G_Entry = GTK_COMBO (WGT_H2GAME)->entry;
  gtk_widget_ref (H2G_Entry);
  gtk_object_set_data_full (GTK_OBJECT (window1), "H2G_Entry", H2G_Entry,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (H2G_Entry), "(  None  )");
  gtk_entry_set_editable (GTK_ENTRY (H2G_Entry), FALSE);
  gtk_widget_show (H2G_Entry);
#ifndef DEMOBUILD
  if ((destiny != DEST_H2) || mp_support)
#endif
	gtk_widget_set_sensitive (WGT_H2GAME, FALSE);

// game types menu for hexenworld
  TxtGameHW = gtk_label_new ("HWorld:");
  gtk_widget_ref (TxtGameHW);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtGameHW", TxtGameHW,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameHW);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtGameHW, 232, 94);
  gtk_label_set_justify (GTK_LABEL (TxtGameHW), GTK_JUSTIFY_LEFT);

  WGT_HWGAME = gtk_combo_new ();
  gtk_widget_ref (WGT_HWGAME);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SelHW", WGT_HWGAME,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_size_request (WGT_HWGAME, 132, 24);
  TmpList = NULL;
  TmpList = g_list_append (TmpList, (gpointer) "DeathMatch");
  Title = (char *)malloc(32);
#ifndef DEMOBUILD
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_HWGAME), FALSE);
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
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_HWGAME, 280, 90);
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
#ifndef DEMOBUILD
  if (destiny != DEST_HW)
#endif
	gtk_widget_set_sensitive (WGT_HWGAME, FALSE);

/*********************************************************************/

// Additional Sound options

// Sampling rate selection
  TxtSound2 = gtk_label_new (_("Sampling Rate:"));
  gtk_widget_ref (TxtSound2);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtSound2", TxtSound2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound2);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtSound2, 232, 136);
  gtk_label_set_justify (GTK_LABEL (TxtSound2), GTK_JUSTIFY_LEFT);

  WGT_SRATE = gtk_combo_new ();
  gtk_widget_ref (WGT_SRATE);
  gtk_object_set_data_full (GTK_OBJECT (window1), "cSRATE", WGT_SRATE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_SRATE), FALSE);
  gtk_widget_set_size_request (WGT_SRATE, 86, 24);
  TmpList = NULL;
  for (i=0; i<MAX_RATES; i++)
	TmpList = g_list_append (TmpList, (char *)snd_rates[i]);
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_SRATE), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_SRATE, 324, 132);
  gtk_widget_show (WGT_SRATE);
  SRATE_Entry = GTK_COMBO (WGT_SRATE)->entry;
  gtk_widget_ref (SRATE_Entry);
  gtk_object_set_data_full (GTK_OBJECT (window1), "SRATE_Entry", SRATE_Entry,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (SRATE_Entry), (char *)snd_rates[sndrate]);
  gtk_entry_set_editable (GTK_ENTRY (SRATE_Entry), FALSE);
  gtk_widget_show (SRATE_Entry);

// 8/16-bit format toggle
  TxtSound3 = gtk_label_new (_("Sound Format:"));
  gtk_widget_ref (TxtSound3);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtSound3", TxtSound3,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound3);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtSound3, 232, 168);
  gtk_label_set_justify (GTK_LABEL (TxtSound3), GTK_JUSTIFY_LEFT);

  WGT_SBITS = gtk_check_button_new_with_label (sndbits ? "16 bit" : " 8 bit");
  gtk_widget_ref (WGT_SBITS);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bSBITS", WGT_SBITS,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_SBITS);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_SBITS, 324, 164);
  gtk_widget_set_size_request (WGT_SBITS, 86, 24);
  gtk_widget_set_sensitive (WGT_SBITS, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_SBITS), sndbits);
  GTK_WIDGET_UNSET_FLAGS (WGT_SBITS, GTK_CAN_FOCUS);

// stereo/mono toggle
  WGT_STEREO = gtk_check_button_new_with_label ("Stereo");
  gtk_widget_ref (WGT_STEREO);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bSTEREO", WGT_STEREO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_STEREO);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_STEREO, 324, 186);
  gtk_widget_set_size_request (WGT_STEREO, 86, 24);
  gtk_widget_set_sensitive (WGT_STEREO, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_STEREO), stereo);
  GTK_WIDGET_UNSET_FLAGS (WGT_STEREO, GTK_CAN_FOCUS);

/*********************************************************************/

// MUSIC (Additional Sound options-2)
// disabling cdaudio
  WGT_CDAUDIO = gtk_check_button_new_with_label (_("No CDaudio"));
  gtk_widget_ref (WGT_CDAUDIO);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bCDA", WGT_CDAUDIO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_CDAUDIO);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_CDAUDIO, 230, 214);
  gtk_widget_set_size_request (WGT_CDAUDIO, 92, 24);
  gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_CDAUDIO), !cdaudio);
  GTK_WIDGET_UNSET_FLAGS (WGT_CDAUDIO, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_CDAUDIO, _("Mark to Disable CDAudio"), NULL);

// disabling midi
  WGT_MIDI = gtk_check_button_new_with_label (_("No MIDI"));
  gtk_widget_ref (WGT_MIDI);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bMIDI", WGT_MIDI,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MIDI);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_MIDI, 324, 214);
  gtk_widget_set_size_request (WGT_MIDI, 86, 24);
  gtk_widget_set_sensitive (WGT_MIDI, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MIDI), !midi);
  GTK_WIDGET_UNSET_FLAGS (WGT_MIDI, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MIDI, _("Mark to Disable Midi"), NULL);

/*********************************************************************/

// Network: disabling LAN
  TxtNet = gtk_label_new (_("Networking :"));
  gtk_widget_ref (TxtNet);
  gtk_object_set_data_full (GTK_OBJECT (window1), "TxtNet", TxtNet,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtNet);
  gtk_fixed_put (GTK_FIXED (fixed1), TxtNet, 232, 272);
  gtk_label_set_justify (GTK_LABEL (TxtNet), GTK_JUSTIFY_LEFT);

  WGT_LANBUTTON = gtk_check_button_new_with_label (_("Disable LAN"));
  gtk_widget_ref (WGT_LANBUTTON);
  gtk_object_set_data_full (GTK_OBJECT (window1), "bLAN", WGT_LANBUTTON,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LANBUTTON);
  gtk_fixed_put (GTK_FIXED (fixed1), WGT_LANBUTTON, 314, 268);
  gtk_widget_set_size_request (WGT_LANBUTTON, 100, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_LANBUTTON), !lan);
  GTK_WIDGET_UNSET_FLAGS (WGT_LANBUTTON, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_LANBUTTON, _("Mark to Disable Net"), NULL);
  if (destiny==DEST_HW)
	gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);

/********************************************************************
		END OF OPTION WIDGETS
 ********************************************************************/


/********************************************************************/
// callback functions setup
  gtk_signal_connect (GTK_OBJECT (window1), "destroy",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_LAUNCH), "clicked",
			GTK_SIGNAL_FUNC (launch_hexen2_bin), NULL);
  gtk_signal_connect (GTK_OBJECT (bQUIT), "clicked",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (bSAVE), "clicked",
			GTK_SIGNAL_FUNC (write_config_file), NULL);
  gtk_signal_connect (GTK_OBJECT (bMORE), "clicked",
			GTK_SIGNAL_FUNC (on_MORE), window1);
#ifndef DEMOBUILD
  gtk_signal_connect (GTK_OBJECT (H2G_Entry), "changed",
			GTK_SIGNAL_FUNC (H2GameChange), &Games);
  gtk_signal_connect (GTK_OBJECT (HWG_Entry), "changed",
			GTK_SIGNAL_FUNC (HWGameChange), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_PORTALS), "released",
			GTK_SIGNAL_FUNC (on_H2MP), &Games);
#endif
  gtk_signal_connect (GTK_OBJECT (WGT_HEXEN2), "released",
			GTK_SIGNAL_FUNC (on_HEXEN2), &Games);
  gtk_signal_connect (GTK_OBJECT (WGT_H2WORLD), "released",
			GTK_SIGNAL_FUNC (on_H2W), &Games);
  gtk_signal_connect (GTK_OBJECT (WGT_OPENGL), "released",
			GTK_SIGNAL_FUNC (on_OGL), &Games);
  gtk_signal_connect (GTK_OBJECT (SND_Entry), "changed",
			GTK_SIGNAL_FUNC (on_SND), &Sound);
  gtk_signal_connect (GTK_OBJECT (SRATE_Entry), "changed",
			GTK_SIGNAL_FUNC (on_SRATE), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_SBITS), "toggled",
			GTK_SIGNAL_FUNC (on_SBITS), &sndbits);
  gtk_signal_connect (GTK_OBJECT (WGT_STEREO), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &stereo);
  gtk_signal_connect (GTK_OBJECT (WGT_MIDI), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &midi);
  gtk_signal_connect (GTK_OBJECT (WGT_CDAUDIO), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &cdaudio);
  gtk_signal_connect (GTK_OBJECT (WGT_LANBUTTON), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &lan);
  gtk_signal_connect (GTK_OBJECT (WGT_FULLSCR), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &fullscreen);
  gtk_signal_connect (GTK_OBJECT (WGT_RESLIST), "changed",
			GTK_SIGNAL_FUNC (res_Change), NULL);
  gtk_object_set_data (GTK_OBJECT (window1), "tooltips", tooltips);
// End of callbacks setup
/********************************************************************/

// update the launch button status
  UpdateStats(&LAUNCH_STRUCT);

  return window1;
}

