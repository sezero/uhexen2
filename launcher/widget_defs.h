/* Widgets to be played with remotely are put into
   these structs properly for easier handling.
*/

struct Launch_s {
	GtkWidget *LStat;	// Status bar, (launch status)
	gint	BinStat;	// statbar context id
	GtkWidget *bLAUNCH;	// Launch button
};

struct Video_s {
	GtkWidget *bOGL;	// OpenGL / Software
	GtkWidget *cRES;	// Resolution combo
	GtkWidget *eRES;	// Resolution combo listing
	GtkWidget *bFULS;	// Fullscreen / windowed
};

struct Misc_s {
	GtkWidget *bLAN;	// LAN button
};

typedef struct {
	GtkWidget *cSND;	// Global sound option
	GtkWidget *cSRATE;	// Sampling rate
	GtkWidget *bSBITS;	// Sample format
	GtkWidget *bSTEREO;	// Stereo
	GtkWidget *bMIDI;	// Midi music option
	GtkWidget *bCDA;	// CD Audio option
} sndwidget_t;

typedef struct {
	GtkWidget *bHEXEN2;	// Hexen II
	GtkWidget *bH2W;	// HexenWorld Client
	GtkWidget *SelH2;	// Hexen2 Game type selection
	GtkWidget *SelHW;	// HexenWorld Game type selection
	GtkWidget *bH2MP;	// Expansion Pack
	struct Launch_s Launch;
	struct Video_s Video;
	struct Misc_s Others;
} gamewidget_t;


// Friendlier definitions for struct members
// WGT for "widget"
#define WGT_STATUSBAR	Games.Launch.LStat
#define BIN_STATUS_ID	Games.Launch.BinStat
#define WGT_LAUNCH	Games.Launch.bLAUNCH
#define WGT_OPENGL	Games.Video.bOGL
#define WGT_RESCOMBO	Games.Video.cRES
#define WGT_RESLIST	Games.Video.eRES
#define WGT_FULLSCR	Games.Video.bFULS
#define WGT_SOUND	Sound.cSND
#define WGT_SRATE	Sound.cSRATE
#define WGT_SBITS	Sound.bSBITS
#define WGT_STEREO	Sound.bSTEREO
#define WGT_MIDI	Sound.bMIDI
#define WGT_CDAUDIO	Sound.bCDA
#define WGT_HEXEN2	Games.bHEXEN2
#define WGT_H2WORLD	Games.bH2W
#define WGT_H2GAME	Games.SelH2
#define WGT_HWGAME	Games.SelHW
#define WGT_PORTALS	Games.bH2MP
#define VID_STRUCT	(Games.Video)
#define LAUNCH_STRUCT	(Games.Launch)
#define WGT_LANBUTTON	Games.Others.bLAN

// Friendlier definitions for use in callbacks.c
#define SND_RATE	cSRATE
#define SND_BITS	bSBITS
#define SND_STEREO	bSTEREO
#define MUSIC_MIDI	bMIDI
#define MUSIC_CD	bCDA
#define RES_COMBO	cRES
#define RES_LIST	eRES
#define STATUSBAR	LStat
#define PORTALS_BUTTON	bH2MP
#define H2GAME		SelH2
#define HWGAME		SelHW
#define LAN_BUTTON	Others.bLAN
#define LAUNCH_BUTTON	bLAUNCH

