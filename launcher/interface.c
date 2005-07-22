#include <gdk/gdkkeysyms.h>
#include "launcher_defs.h"
#include "common.h"
#include "interface.h"
#include "widget_defs.h"
#include "callbacks.h"
#include "support.h"
#include "config_file.h"

unsigned short i;
// from config_file.c
extern int destiny;
extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int use_con;
extern int fxgamma;
extern int is8bit;
extern int mtex;
extern int vsync;
extern int use_fsaa;
extern int aasamples;
extern int gl_nonstd;
extern char gllibrary[256];
extern int midi;
extern int cdaudio;
extern int sound;
extern int sndrate;
extern int sndbits;
extern int lan;
extern int mouse;
extern int debug;
extern int use_heap;
extern int use_zone;
extern int heapsize;
extern int zonesize;
// from launch_bin.c
extern unsigned missingexe;
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
  static HoTWindow_t main_win;

  static gamewidget_t Games;
  static sndwidget_t Sound;

// Labels for basics
  GtkWidget *TxtTitle;	// Title Label
  GtkWidget *TxtGame0;	// Destiny label
  GtkWidget *TxtVideo;	// Renderer, etc.
  GtkWidget *TxtResol;	// Resolution
  GtkWidget *TxtSound;	// Sound driver combo
// Widgets for basics which needn't be in a relevant struct
  GtkWidget *SND_Entry; // Sound driver listing
  GtkWidget *bSAVE;	// Save options button
  GtkWidget *bQUIT;	// Quit button

// Labels for additionals
  GtkWidget *TxtMouse;	// Mouse options.
  GtkWidget *TxtNet;	// Networking options.
  GtkWidget *TxtAdv;	// Memory options.
  GtkWidget *TxtVidExt;	// Extra Video Options label
  GtkWidget *TxtSndExt;	// Extra Sound Options label
  GtkWidget *TxtSound2;	// Sound options extra
  GtkWidget *TxtSound3;	// Sound options extra
  GtkWidget *TxtGameT;	// GameType Label
  GtkWidget *TxtGameH2;	// Hexen2 GameType Label
  GtkWidget *TxtGameHW;	// Hexenworld GameType Label
// Widgets for additionals which needn't be in a relevant struct
  GtkWidget *SRATE_Entry; // Sampling rate listing
  GtkWidget *H2G_Entry;	// Hexen2 games listing
  GtkWidget *HWG_Entry;	// Hexenworld games listing

// Separators
  GtkWidget *hseparator0;
  GtkWidget *hseparator1;
  GtkWidget *hseparator2;

// Other stuff
  char *Title;
  GList *TmpList = NULL;
  GSList *Destinies = NULL;

  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();

/*********************************************************************/
// Create the main window

  MAIN_WINDOW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (MAIN_WINDOW), "mywindow", MAIN_WINDOW);
#ifndef DEMOBUILD
  gtk_window_set_title (GTK_WINDOW (MAIN_WINDOW), "Hexen2 Launcher " HOTL_VER);
#else
  gtk_window_set_title (GTK_WINDOW (MAIN_WINDOW), "Hexen2 demo Launcher " HOTL_VER);
#endif
  gtk_window_set_resizable (GTK_WINDOW (MAIN_WINDOW), FALSE);
  gtk_widget_set_size_request(MAIN_WINDOW, 230, 354);

/*********************************************************************/
// Create the option tabs on the main window

  HOLDER_BOX = gtk_hbox_new (TRUE, 2);
  gtk_widget_show (HOLDER_BOX);
  gtk_container_add (GTK_CONTAINER (MAIN_WINDOW), HOLDER_BOX);

  BOOK0 = gtk_notebook_new ();
  gtk_widget_ref (BOOK0);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "notebook0", BOOK0,
				(GtkDestroyNotify) gtk_widget_unref);
//gtk_container_add (GTK_CONTAINER (MAIN_WINDOW), BOOK0);
  gtk_widget_show (BOOK0);
  gtk_box_pack_start (GTK_BOX (HOLDER_BOX), BOOK0, TRUE, TRUE, 0);

  BASIC_TAB = gtk_fixed_new ();
  gtk_widget_ref (BASIC_TAB);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "fixed0", BASIC_TAB,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (BASIC_TAB);

  BOOK1 = gtk_notebook_new ();
  gtk_widget_ref (BOOK1);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "notebook1", BOOK1,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (BOOK1);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (BOOK1), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (BOOK1));
  gtk_box_pack_start (GTK_BOX (HOLDER_BOX), BOOK1, TRUE, TRUE, 0);

  ADDON_TAB1 = gtk_fixed_new ();
  gtk_widget_ref (ADDON_TAB1);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "fixed1", ADDON_TAB1,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (ADDON_TAB1);

  ADDON_TAB2 = gtk_fixed_new ();
  gtk_widget_ref (ADDON_TAB2);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "fixed2", ADDON_TAB2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (ADDON_TAB2);

//  TAB0_LABEL = gtk_label_new ("Basic Options");
//  gtk_widget_ref (TAB0_LABEL);
//  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "lbl_tab0", TAB0_LABEL, (GtkDestroyNotify) gtk_widget_unref);

  TAB1_LABEL = gtk_label_new (" Sound / Other ");
  gtk_widget_ref (TAB1_LABEL);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "lbl_tab1", TAB1_LABEL, (GtkDestroyNotify) gtk_widget_unref);

  TAB2_LABEL = gtk_label_new ("  Video / Gameplay ");
  gtk_widget_ref (TAB2_LABEL);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "lbl_tab2", TAB2_LABEL, (GtkDestroyNotify) gtk_widget_unref);

//  gtk_notebook_append_page (GTK_NOTEBOOK (BOOK0), BASIC_TAB, TAB0_LABEL);
  gtk_notebook_append_page (GTK_NOTEBOOK (BOOK0), BASIC_TAB, NULL);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (BOOK0), FALSE);
  gtk_notebook_append_page (GTK_NOTEBOOK (BOOK1), ADDON_TAB2, TAB2_LABEL);
  gtk_notebook_append_page (GTK_NOTEBOOK (BOOK1), ADDON_TAB1, TAB1_LABEL);

  gtk_widget_hide (BOOK1);

/*********************************************************************/
// Basic title representing the HoT-version the launcher is packed with

  TxtTitle = gtk_label_new ("Hammer of Thyrion " HOT_VER);
  gtk_widget_ref (TxtTitle);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtTitle", TxtTitle,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtTitle);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), TxtTitle, 14, 14);
  gtk_label_set_justify (GTK_LABEL (TxtTitle), GTK_JUSTIFY_LEFT);


/********************************************************************
 TAB - 1:		BASIC OPTIONS
 ********************************************************************/

// Quit button
  bQUIT = gtk_button_new_with_label (_("Quit"));
  gtk_widget_ref (bQUIT);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bQUIT", bQUIT,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bQUIT);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), bQUIT, 132, 300);
  gtk_widget_set_size_request (bQUIT, 80, 24);

// Save the options
  bSAVE = gtk_button_new_with_label (_("Save Options"));
  gtk_widget_ref (bSAVE);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bSAVE", bSAVE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bSAVE);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), bSAVE, 12, 272);
  gtk_widget_set_size_request (bSAVE, 112, 24);

/*********************************************************************/

// Launch button and Statusbar
  WGT_LAUNCH = gtk_button_new_with_label (_("Launch Hexen2"));
  gtk_widget_ref (WGT_LAUNCH);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bLAUNCH", WGT_LAUNCH,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LAUNCH);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_LAUNCH, 12, 300);
  gtk_widget_set_size_request (WGT_LAUNCH, 112, 24);

  WGT_STATUSBAR = gtk_statusbar_new ();
  gtk_widget_ref (WGT_STATUSBAR);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "LStat", WGT_STATUSBAR,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_STATUSBAR);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_STATUSBAR, 0, 328);
  gtk_widget_set_size_request (WGT_STATUSBAR, 230, 24);
#ifndef WITH_GTK1
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(WGT_STATUSBAR), FALSE);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (WGT_STATUSBAR), 2);
  BIN_STATUS_ID = gtk_statusbar_get_context_id (GTK_STATUSBAR (WGT_STATUSBAR), "BinaryStatus");
  gtk_statusbar_push (GTK_STATUSBAR(WGT_STATUSBAR), BIN_STATUS_ID,_("  Waiting status..."));

/*********************************************************************/

// DESTINY : Game/binary selection
  TxtGame0 = gtk_label_new (_("Choose your destiny:"));
  gtk_widget_ref (TxtGame0);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtGame0", TxtGame0,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGame0);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), TxtGame0, 14, 44);
  gtk_label_set_justify (GTK_LABEL (TxtGame0), GTK_JUSTIFY_LEFT);

// Destiny: Hexen2
#ifdef DEMOBUILD
  WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II demo");
#else
  WGT_HEXEN2 = gtk_radio_button_new_with_label (Destinies, "Hexen II");
#endif
  Destinies = gtk_radio_button_group (GTK_RADIO_BUTTON (WGT_HEXEN2));
  gtk_widget_ref (WGT_HEXEN2);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bHEXEN2", WGT_HEXEN2,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_HEXEN2, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_HEXEN2);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_HEXEN2, 24, 64);
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
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bH2MP", WGT_PORTALS,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_PORTALS, 112, 64);
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
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bH2W", WGT_H2WORLD,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_H2WORLD, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_H2WORLD);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_H2WORLD, 24, 90);
  gtk_widget_set_size_request (WGT_H2WORLD, 180, 24);
  if (destiny == DEST_HW)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_H2WORLD), TRUE);

/*********************************************************************/

// Video Options
  TxtVideo = gtk_label_new (_("Graphics    :"));
  gtk_widget_ref (TxtVideo);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtVideo", TxtVideo,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtVideo);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), TxtVideo, 14, 166);
  gtk_label_set_justify (GTK_LABEL (TxtVideo), GTK_JUSTIFY_LEFT);

// opengl/software toggle
  WGT_OPENGL = gtk_check_button_new_with_label ("OpenGL");
  gtk_widget_ref (WGT_OPENGL);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bOGL", WGT_OPENGL,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_OPENGL, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_OPENGL);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_OPENGL, 100, 162);
  gtk_widget_set_size_request (WGT_OPENGL, 108, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_OPENGL), opengl_support);

// fullscreen/windowed toggle
  WGT_FULLSCR = gtk_check_button_new_with_label (_("Fullscreen"));
  gtk_widget_ref (WGT_FULLSCR);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bFULS", WGT_FULLSCR,
				(GtkDestroyNotify) gtk_widget_unref);
  GTK_WIDGET_UNSET_FLAGS (WGT_FULLSCR, GTK_CAN_FOCUS);
  gtk_widget_show (WGT_FULLSCR);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_FULLSCR, 100, 186);
  gtk_widget_set_size_request (WGT_FULLSCR, 108, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_FULLSCR), fullscreen);

// resolution
  TxtResol = gtk_label_new (_("Resolution  :"));
  gtk_widget_ref (TxtResol);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtResol", TxtResol,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtResol);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), TxtResol, 14, 218);
  gtk_label_set_justify (GTK_LABEL (TxtResol), GTK_JUSTIFY_LEFT);

//resolution combo
  WGT_RESCOMBO = gtk_combo_new ();
  gtk_widget_ref (WGT_RESCOMBO);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "cRES", WGT_RESCOMBO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_RESCOMBO), FALSE);
  gtk_widget_set_size_request (WGT_RESCOMBO, 108, 24);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_RESCOMBO, 100, 214);
//resolution display
  WGT_RESLIST = GTK_COMBO (WGT_RESCOMBO)->entry;
  gtk_widget_ref (WGT_RESLIST);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "eRES", WGT_RESLIST,
				(GtkDestroyNotify) gtk_widget_unref);
  //gtk_entry_set_alignment (GTK_ENTRY (WGT_RESLIST), 1);
  gtk_entry_set_editable (GTK_ENTRY (WGT_RESLIST), FALSE);

//conwidth toggle button
  WGT_CONWBUTTON = gtk_check_button_new_with_label (_("Conwidth :"));
  gtk_widget_ref (WGT_CONWBUTTON);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bCONW", WGT_CONWBUTTON,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_CONWBUTTON);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_CONWBUTTON, 14, 244);
  gtk_widget_set_size_request (WGT_CONWBUTTON, 84, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_CONWBUTTON), use_con);
  GTK_WIDGET_UNSET_FLAGS (WGT_CONWBUTTON, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_CONWBUTTON, _("Allow bigger/readable text and HUD in high resolutions. 640 is recommended"), NULL);
  gtk_widget_set_sensitive (WGT_CONWBUTTON, opengl_support);

//conwidth combo
  WGT_CONWCOMBO = gtk_combo_new ();
  gtk_widget_ref (WGT_CONWCOMBO);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "cCONW", WGT_CONWCOMBO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_CONWCOMBO), FALSE);
  gtk_widget_set_size_request (WGT_CONWCOMBO, 108, 24);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_CONWCOMBO, 100, 244);
  gtk_widget_set_sensitive (WGT_CONWCOMBO, opengl_support);
//conwidth display
  WGT_CONWLIST = GTK_COMBO (WGT_CONWCOMBO)->entry;
  gtk_widget_ref (WGT_CONWLIST);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "eCONW", WGT_CONWLIST,
				(GtkDestroyNotify) gtk_widget_unref);
  //gtk_entry_set_alignment (GTK_ENTRY (WGT_CONWLIST), 1);
  gtk_entry_set_editable (GTK_ENTRY (WGT_CONWLIST), FALSE);

//menu listing for resolution and conwidth come from a callback
  Make_ResMenu(&VID_STRUCT);
  Make_ConwMenu(&VID_STRUCT);
  gtk_widget_show (WGT_RESCOMBO);
  gtk_widget_show (WGT_RESLIST);
  gtk_widget_show (WGT_CONWCOMBO);
  gtk_widget_show (WGT_CONWLIST);

/*********************************************************************/

// Sound options (basic: driver selection)
  TxtSound = gtk_label_new (_("Sound        :"));
  gtk_widget_ref (TxtSound);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtSound", TxtSound,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), TxtSound, 14, 136);
  gtk_label_set_justify (GTK_LABEL (TxtSound), GTK_JUSTIFY_LEFT);

  WGT_SOUND = gtk_combo_new ();
  gtk_widget_ref (WGT_SOUND);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "cSND", WGT_SOUND,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_SOUND), FALSE);
  gtk_widget_set_size_request (WGT_SOUND, 108, 24);
  TmpList = NULL;
  for (i=0; i<MAX_SOUND; i++)
	TmpList = g_list_append (TmpList, (char *)snddrv_names[i][1]);
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_SOUND), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), WGT_SOUND, 100, 132);
  gtk_widget_show (WGT_SOUND);
  SND_Entry = GTK_COMBO (WGT_SOUND)->entry;
  gtk_widget_ref (SND_Entry);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "SND_Entry", SND_Entry,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (SND_Entry), (char *)snddrv_names[sound][1]);
  gtk_entry_set_editable (GTK_ENTRY (SND_Entry), FALSE);
//gtk_entry_set_alignment (GTK_ENTRY (SND_Entry), 1);
  gtk_widget_show (SND_Entry);


/********************************************************************
 TAB - 2:		ADDITIONAL OPTIONS
 ********************************************************************/

/*********************************************************************/


  MORE_LESS = gtk_button_new_with_label (_("More >>>"));
  gtk_widget_ref (MORE_LESS);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMORE", MORE_LESS,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (MORE_LESS);
  gtk_fixed_put (GTK_FIXED (BASIC_TAB), MORE_LESS, 132, 272);
  gtk_widget_set_size_request (MORE_LESS, 80, 24);


// Additional Sound options

  TxtSndExt = gtk_label_new (_("Additional Sound Options:"));
  gtk_widget_ref (TxtSndExt);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtSndExt", TxtSndExt,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSndExt);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtSndExt, 14, 20);
  gtk_label_set_justify (GTK_LABEL (TxtSndExt), GTK_JUSTIFY_LEFT);

// Sampling rate selection
  TxtSound2 = gtk_label_new (_("Sampling Rate:"));
  gtk_widget_ref (TxtSound2);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtSound2", TxtSound2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound2);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtSound2, 14, 44);
  gtk_label_set_justify (GTK_LABEL (TxtSound2), GTK_JUSTIFY_LEFT);

  WGT_SRATE = gtk_combo_new ();
  gtk_widget_ref (WGT_SRATE);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "cSRATE", WGT_SRATE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_combo_set_use_arrows (GTK_COMBO (WGT_SRATE), FALSE);
  gtk_widget_set_size_request (WGT_SRATE, 86, 24);
  TmpList = NULL;
  for (i=0; i<MAX_RATES; i++)
	TmpList = g_list_append (TmpList, (char *)snd_rates[i]);
  gtk_combo_set_popdown_strings (GTK_COMBO (WGT_SRATE), TmpList);
  g_list_free (TmpList);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_SRATE, 112, 40);
  gtk_widget_show (WGT_SRATE);
  SRATE_Entry = GTK_COMBO (WGT_SRATE)->entry;
  gtk_widget_ref (SRATE_Entry);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "SRATE_Entry", SRATE_Entry,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (SRATE_Entry), (char *)snd_rates[sndrate]);
  gtk_entry_set_editable (GTK_ENTRY (SRATE_Entry), FALSE);
  gtk_widget_show (SRATE_Entry);

// 8/16-bit format toggle
  TxtSound3 = gtk_label_new (_("Sound Format:"));
  gtk_widget_ref (TxtSound3);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtSound3", TxtSound3,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtSound3);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtSound3, 14, 70);
  gtk_label_set_justify (GTK_LABEL (TxtSound3), GTK_JUSTIFY_LEFT);

  WGT_SBITS = gtk_check_button_new_with_label (sndbits ? "16 bit" : " 8 bit");
  gtk_widget_ref (WGT_SBITS);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bSBITS", WGT_SBITS,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_SBITS);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_SBITS, 112, 66);
  gtk_widget_set_size_request (WGT_SBITS, 76, 24);
  gtk_widget_set_sensitive (WGT_SBITS, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_SBITS), sndbits);
  GTK_WIDGET_UNSET_FLAGS (WGT_SBITS, GTK_CAN_FOCUS);

/*********************************************************************/

// MUSIC (Additional Sound options-2)

// disabling cdaudio
  WGT_CDAUDIO = gtk_check_button_new_with_label (_("No CDaudio"));
  gtk_widget_ref (WGT_CDAUDIO);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bCDA", WGT_CDAUDIO,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_CDAUDIO);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_CDAUDIO, 112, 96);
  gtk_widget_set_size_request (WGT_CDAUDIO, 96, 24);
  gtk_widget_set_sensitive (WGT_CDAUDIO, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_CDAUDIO), !cdaudio);
  GTK_WIDGET_UNSET_FLAGS (WGT_CDAUDIO, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_CDAUDIO, _("Mark to Disable CDAudio"), NULL);

// disabling midi
  WGT_MIDI = gtk_check_button_new_with_label (_("No MIDI"));
  gtk_widget_ref (WGT_MIDI);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMIDI", WGT_MIDI,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MIDI);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_MIDI, 14, 96);
  gtk_widget_set_size_request (WGT_MIDI, 76, 24);
  gtk_widget_set_sensitive (WGT_MIDI, sound);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MIDI), !midi);
  GTK_WIDGET_UNSET_FLAGS (WGT_MIDI, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MIDI, _("Mark to Disable Midi"), NULL);

/*********************************************************************/

  hseparator0 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator0);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "hseparator0", hseparator0,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), hseparator0, 14, 102);
  gtk_widget_set_size_request (hseparator0, 200, 8);
  gtk_widget_show (hseparator0);

/*********************************************************************/

// Additional OpenGL options

  TxtVidExt = gtk_label_new (_("OpenGL Options:"));
  gtk_widget_ref (TxtVidExt);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtVidExt", TxtVidExt,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtVidExt);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), TxtVidExt, 14, 116);
  gtk_label_set_justify (GTK_LABEL (TxtVidExt), GTK_JUSTIFY_LEFT);

// 3dfx Voodoo1/2/Rush support
  WGT_3DFX = gtk_check_button_new_with_label (_("3dfx gamma support"));
  gtk_widget_ref (WGT_3DFX);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "b3DFX", WGT_3DFX,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_3DFX);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_3DFX, 14, 134);
  gtk_widget_set_size_request (WGT_3DFX, 160, 24);
  gtk_widget_set_sensitive (WGT_3DFX, opengl_support);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_3DFX), fxgamma);
  GTK_WIDGET_UNSET_FLAGS (WGT_3DFX, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_3DFX, _("Special brightness support for old Voodoo1/2 boards"), NULL);

// 8-bit GL Extensions (Paletted Textures)
  WGT_GL8BIT = gtk_check_button_new_with_label (_("Enable 8-bit textures"));
  gtk_widget_ref (WGT_GL8BIT);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "b8BIT", WGT_GL8BIT,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_GL8BIT);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_GL8BIT, 14, 158);
  gtk_widget_set_size_request (WGT_GL8BIT, 160, 24);
  gtk_widget_set_sensitive (WGT_GL8BIT, opengl_support);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_GL8BIT), is8bit);
  GTK_WIDGET_UNSET_FLAGS (WGT_GL8BIT, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_GL8BIT, _("Enable 8-bit OpenGL texture extensions"), NULL);

// Disable Multitexture detection
  WGT_MTEX = gtk_check_button_new_with_label (_("Disable Multitexturing"));
  gtk_widget_ref (WGT_MTEX);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMTEX", WGT_MTEX,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MTEX);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_MTEX, 14, 182);
  gtk_widget_set_size_request (WGT_MTEX, 180, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MTEX), !mtex);
  GTK_WIDGET_UNSET_FLAGS (WGT_MTEX, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MTEX, _("Disable multitexture detection"), NULL);
  gtk_widget_set_sensitive (WGT_MTEX, opengl_support);

// Enable VSync
  WGT_VSYNC = gtk_check_button_new_with_label (_("Enable VSync"));
  gtk_widget_ref (WGT_VSYNC);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bVSYNC", WGT_VSYNC,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_VSYNC);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_VSYNC, 14, 206);
  gtk_widget_set_size_request (WGT_VSYNC, 160, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_VSYNC), vsync);
  GTK_WIDGET_UNSET_FLAGS (WGT_VSYNC, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_VSYNC, _("Synchronize with Monitor Refresh"), NULL);
  gtk_widget_set_sensitive (WGT_VSYNC, opengl_support);

// Multisampling (artialiasing)
  WGT_FSAA = gtk_check_button_new_with_label (_("FSAA (Antialiasing):"));
  gtk_widget_ref (WGT_FSAA);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bFSAA", WGT_FSAA,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_FSAA);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_FSAA, 14, 230);
  gtk_widget_set_size_request (WGT_FSAA, 144, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_FSAA), use_fsaa);
  GTK_WIDGET_UNSET_FLAGS (WGT_FSAA, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_FSAA, _("Enable Antialiasing"), NULL);
  gtk_widget_set_sensitive (WGT_FSAA, opengl_support);

  WGT_ADJFSAA =  gtk_adjustment_new (aasamples, 0, 4, 2, 10, 10);
  WGT_AASAMPLES = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ADJFSAA), 2, 0);
  gtk_entry_set_editable (GTK_ENTRY (WGT_AASAMPLES), FALSE);
  gtk_widget_show (WGT_AASAMPLES);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_AASAMPLES, 162, 230);
  gtk_widget_set_size_request (WGT_AASAMPLES, 38, 24);
  gtk_widget_ref (WGT_AASAMPLES);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "spnFSAA", WGT_AASAMPLES,
				(GtkDestroyNotify) gtk_widget_unref);

// Custom OpenGL Library
  WGT_LIBGL = gtk_check_button_new_with_label (_("Use a different GL library:"));
  gtk_widget_ref (WGT_LIBGL);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bLIBGL", WGT_LIBGL,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LIBGL);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_LIBGL, 14, 256);
  gtk_widget_set_size_request (WGT_LIBGL, 180, 24);
  gtk_widget_set_sensitive (WGT_LIBGL, opengl_support);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_LIBGL), gl_nonstd);
  GTK_WIDGET_UNSET_FLAGS (WGT_LIBGL, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_LIBGL, _("Type the full path of the OpenGL library"), NULL);

  WGT_GLPATH = gtk_entry_new ();
  gtk_widget_show (WGT_GLPATH);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_GLPATH, 36, 280);
  gtk_widget_set_size_request (WGT_GLPATH, 164, 24);
  gtk_entry_set_text (GTK_ENTRY (WGT_GLPATH), gllibrary);
//if (!opengl_support || !gl_nonstd)
//	gtk_widget_set_sensitive (WGT_GLPATH, FALSE);
  gtk_widget_ref (WGT_GLPATH);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "GL_Entry", WGT_GLPATH,
				(GtkDestroyNotify) gtk_widget_unref);


/********************************************************************
 TAB - 3:		ADDITIONAL OPTIONS
 ********************************************************************/

/*********************************************************************/

// Additional game-types
  TxtGameT = gtk_label_new (_("Extra Game Types:"));
  gtk_widget_ref (TxtGameT);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtGameT", TxtGameT,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameT);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), TxtGameT, 14, 20);
  gtk_label_set_justify (GTK_LABEL (TxtGameT), GTK_JUSTIFY_LEFT);

// game types menu for hexen2
  TxtGameH2 = gtk_label_new ("Hexen2:");
  gtk_widget_ref (TxtGameH2);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtGameH2", TxtGameH2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameH2);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), TxtGameH2, 16, 44);
  gtk_label_set_justify (GTK_LABEL (TxtGameH2), GTK_JUSTIFY_LEFT);

// game types menu for hexen2
// there are two botmatch mods only...
  WGT_H2GAME = gtk_combo_new ();
  gtk_widget_ref (WGT_H2GAME);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "SelH2", WGT_H2GAME,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_size_request (WGT_H2GAME, 132, 32);
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
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_H2GAME, 68, 36);
  gtk_widget_show (WGT_H2GAME);
  H2G_Entry = GTK_COMBO (WGT_H2GAME)->entry;
  gtk_widget_ref (H2G_Entry);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "H2G_Entry", H2G_Entry,
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
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtGameHW", TxtGameHW,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtGameHW);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), TxtGameHW, 16, 70);
  gtk_label_set_justify (GTK_LABEL (TxtGameHW), GTK_JUSTIFY_LEFT);

  WGT_HWGAME = gtk_combo_new ();
  gtk_widget_ref (WGT_HWGAME);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "SelHW", WGT_HWGAME,
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
  gtk_fixed_put (GTK_FIXED (ADDON_TAB2), WGT_HWGAME, 68, 66);
  gtk_widget_show (WGT_HWGAME);
  HWG_Entry = GTK_COMBO (WGT_HWGAME)->entry;
  gtk_widget_ref (HWG_Entry);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "HWG_Entry", HWG_Entry,
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

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "hseparator1", hseparator1,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), hseparator1, 14, 128);
  gtk_widget_set_size_request (hseparator1, 200, 8);
  gtk_widget_show (hseparator1);

/*********************************************************************/

// Network: disabling LAN
  TxtNet = gtk_label_new (_("Networking :"));
  gtk_widget_ref (TxtNet);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtNet", TxtNet,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtNet);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtNet, 14, 144);
  gtk_label_set_justify (GTK_LABEL (TxtNet), GTK_JUSTIFY_LEFT);

  WGT_LANBUTTON = gtk_check_button_new_with_label (_("Disable LAN"));
  gtk_widget_ref (WGT_LANBUTTON);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bLAN", WGT_LANBUTTON,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_LANBUTTON);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_LANBUTTON, 112, 140);
  gtk_widget_set_size_request (WGT_LANBUTTON, 108, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_LANBUTTON), !lan);
  GTK_WIDGET_UNSET_FLAGS (WGT_LANBUTTON, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_LANBUTTON, _("Mark to Disable Net"), NULL);
  if (destiny==DEST_HW)
	gtk_widget_set_sensitive (WGT_LANBUTTON, FALSE);

/*********************************************************************/

// Mouse: disabling Mouse
  TxtMouse = gtk_label_new (_("Mouse  :"));
  gtk_widget_ref (TxtMouse);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtMouse", TxtMouse,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtMouse);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtMouse, 14, 166);
  gtk_label_set_justify (GTK_LABEL (TxtMouse), GTK_JUSTIFY_LEFT);

  WGT_MOUSEBUTTON = gtk_check_button_new_with_label (_("Disable Mouse"));
  gtk_widget_ref (WGT_MOUSEBUTTON);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMOUSE", WGT_MOUSEBUTTON,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MOUSEBUTTON);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_MOUSEBUTTON, 112, 162);
  gtk_widget_set_size_request (WGT_MOUSEBUTTON, 112, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MOUSEBUTTON), !mouse);
  GTK_WIDGET_UNSET_FLAGS (WGT_MOUSEBUTTON, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MOUSEBUTTON, _("Mark to Disable Mouse"), NULL);

/*********************************************************************/

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator2);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "hseparator2", hseparator2,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), hseparator2, 14, 192);
  gtk_widget_set_size_request (hseparator2, 200, 8);
  gtk_widget_show (hseparator2);

/*********************************************************************/

// Advanced options
  TxtAdv = gtk_label_new (_("Advanced :"));
  gtk_widget_ref (TxtAdv);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "TxtAdv", TxtAdv,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (TxtAdv);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), TxtAdv, 14, 210);
  gtk_label_set_justify (GTK_LABEL (TxtAdv), GTK_JUSTIFY_LEFT);


// Memory options (heapsize)
  WGT_MEMHEAP = gtk_check_button_new_with_label (_("Heapsize (KB):"));
  gtk_widget_ref (WGT_MEMHEAP);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMEMHEAP", WGT_MEMHEAP,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MEMHEAP);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_MEMHEAP, 14, 230);
  gtk_widget_set_size_request (WGT_MEMHEAP, 116, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MEMHEAP), use_heap);
  GTK_WIDGET_UNSET_FLAGS (WGT_MEMHEAP, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MEMHEAP, _("The main memory to allocate in KB"), NULL);

  WGT_HEAPADJ =  gtk_adjustment_new (heapsize, HEAP_MINSIZE, HEAP_MAXSIZE, 1024, 10, 10);
  WGT_HEAPSIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_HEAPADJ), 1024, 0);
  gtk_entry_set_editable (GTK_ENTRY (WGT_HEAPSIZE), FALSE);
  gtk_widget_show (WGT_HEAPSIZE);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_HEAPSIZE, 134, 230);
  gtk_widget_set_size_request (WGT_HEAPSIZE, 64, 24);
  gtk_widget_ref (WGT_HEAPSIZE);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "spnHEAP", WGT_HEAPSIZE,
				(GtkDestroyNotify) gtk_widget_unref);

// Memory options (zonesize)
  WGT_MEMZONE = gtk_check_button_new_with_label (_("Zonesize (KB):"));
  gtk_widget_ref (WGT_MEMZONE);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bMEMZONE", WGT_MEMZONE,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_MEMZONE);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_MEMZONE, 14, 256);
  gtk_widget_set_size_request (WGT_MEMZONE, 116, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_MEMZONE), use_zone);
  GTK_WIDGET_UNSET_FLAGS (WGT_MEMZONE, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_MEMZONE, _("Dynamic zone memory to allocate in KB"), NULL);

  WGT_ZONEADJ =  gtk_adjustment_new (zonesize, ZONE_MINSIZE, ZONE_MAXSIZE, 1, 10, 10);
  WGT_ZONESIZE = gtk_spin_button_new (GTK_ADJUSTMENT (WGT_ZONEADJ), 1, 0);
  gtk_entry_set_editable (GTK_ENTRY (WGT_ZONESIZE), FALSE);
  gtk_widget_show (WGT_ZONESIZE);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_ZONESIZE, 134, 256);
  gtk_widget_set_size_request (WGT_ZONESIZE, 64, 24);
  gtk_widget_ref (WGT_ZONESIZE);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "spnZONE", WGT_ZONESIZE,
				(GtkDestroyNotify) gtk_widget_unref);

// Debug output option
  WGT_DBGLOG = gtk_check_button_new_with_label (_("Log debug output to a file"));
  gtk_widget_ref (WGT_DBGLOG);
  gtk_object_set_data_full (GTK_OBJECT (MAIN_WINDOW), "bDBG", WGT_DBGLOG,
				(GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (WGT_DBGLOG);
  gtk_fixed_put (GTK_FIXED (ADDON_TAB1), WGT_DBGLOG, 14, 280);
  gtk_widget_set_size_request (WGT_DBGLOG, 176, 24);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WGT_DBGLOG), debug);
  GTK_WIDGET_UNSET_FLAGS (WGT_DBGLOG, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip (tooltips, WGT_DBGLOG, _("Write console debug output to qconsole.log"), NULL);


/********************************************************************
		END OF OPTION WIDGETS
 ********************************************************************/


/********************************************************************/
// callback functions setup
  gtk_signal_connect (GTK_OBJECT (MAIN_WINDOW), "destroy",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_LAUNCH), "clicked",
			GTK_SIGNAL_FUNC (launch_hexen2_bin), NULL);
  gtk_signal_connect (GTK_OBJECT (bQUIT), "clicked",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (bSAVE), "clicked",
			GTK_SIGNAL_FUNC (write_config_file), NULL);
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
  gtk_signal_connect (GTK_OBJECT (WGT_MIDI), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &midi);
  gtk_signal_connect (GTK_OBJECT (WGT_CDAUDIO), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &cdaudio);
  gtk_signal_connect (GTK_OBJECT (WGT_LANBUTTON), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &lan);
  gtk_signal_connect (GTK_OBJECT (WGT_FULLSCR), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &fullscreen);
  gtk_signal_connect (GTK_OBJECT (WGT_CONWBUTTON), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &use_con);
  gtk_signal_connect (GTK_OBJECT (WGT_3DFX), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &fxgamma);
  gtk_signal_connect (GTK_OBJECT (WGT_GL8BIT), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &is8bit);
  gtk_signal_connect (GTK_OBJECT (WGT_MTEX), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &mtex);
  gtk_signal_connect (GTK_OBJECT (WGT_VSYNC), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &vsync);
  gtk_signal_connect (GTK_OBJECT (WGT_FSAA), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &use_fsaa);
  gtk_signal_connect (GTK_OBJECT (WGT_ADJFSAA), "value_changed",
			GTK_SIGNAL_FUNC (adj_Change), &aasamples);
  gtk_signal_connect (GTK_OBJECT (WGT_LIBGL), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &gl_nonstd);
  gtk_signal_connect (GTK_OBJECT (WGT_DBGLOG), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &debug);
  gtk_signal_connect (GTK_OBJECT (WGT_MEMHEAP), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &use_heap);
  gtk_signal_connect (GTK_OBJECT (WGT_MEMZONE), "toggled",
			GTK_SIGNAL_FUNC (ReverseOpt), &use_zone);
  gtk_signal_connect (GTK_OBJECT (WGT_RESLIST), "changed",
			GTK_SIGNAL_FUNC (res_Change), &VID_STRUCT);
  gtk_signal_connect (GTK_OBJECT (WGT_CONWLIST), "changed",
			GTK_SIGNAL_FUNC (con_Change), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_GLPATH), "changed",
			GTK_SIGNAL_FUNC (libgl_Change), NULL);
  gtk_signal_connect (GTK_OBJECT (WGT_HEAPADJ), "value_changed",
			GTK_SIGNAL_FUNC (adj_Change), &heapsize);
  gtk_signal_connect (GTK_OBJECT (WGT_ZONEADJ), "value_changed",
			GTK_SIGNAL_FUNC (adj_Change), &zonesize);
  gtk_signal_connect (GTK_OBJECT (MORE_LESS), "clicked",
			GTK_SIGNAL_FUNC (on_MORE), &main_win);
  gtk_object_set_data (GTK_OBJECT (MAIN_WINDOW), "tooltips", tooltips);

// End of callbacks setup
/********************************************************************/

// update the launch button status
  UpdateStats(&LAUNCH_STRUCT);

  return MAIN_WINDOW;
}

