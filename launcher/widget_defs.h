/* Widgets to be played with remotely are put in to
   these structs properly for easier handling.
   We mostly send a pointer to the _whole_ Games struct,
   but the binary output is smaller now.
*/

struct Launch_s {
	GtkWidget *LStat;	// Status bar, (launch status)
	gint	BinStat;	// statbar context id
	GtkWidget *bLAUNCH;	// Launch button
};

struct Video_s {
	GtkWidget *bOGL;	// OpenGL / Software
	GtkAdjustment *rAdj;	// Resolution slider's Adj. object
	GtkWidget *rScale;	// Resolution slider
	GtkWidget *rText;	// Resolution display
	GtkWidget *bFULS;	// Fullscreen / windowed
};

struct Misc_s {
	GtkWidget *bLAN;	// LAN button
};

typedef struct {
	GtkWidget *bSND;	// Global sound option
	GtkWidget *bMIDI;	// Midi music option
	GtkWidget *bCDA;	// CD Audio option
} sndwidget_t;

typedef struct {
	GtkWidget *bHEXEN2;	// Hexen II
	GtkWidget *bH2W;	// HexenWorld Client
	GtkWidget *bH2MP;	// Expansion Pack
	GtkWidget *bOLDM;	// Old Mission support for H2MP
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
#define WGT_RES_ADJUST	Games.Video.rAdj
#define WGT_RES_SCALE	Games.Video.rScale
#define WGT_RESTEXT	Games.Video.rText
#define WGT_FULLSCR	Games.Video.bFULS
#define WGT_SOUND	Sound.bSND
#define WGT_MIDI	Sound.bMIDI
#define WGT_CDAUDIO	Sound.bCDA
#define WGT_HEXEN2	Games.bHEXEN2
#define WGT_H2WORLD	Games.bH2W
#define WGT_PORTALS	Games.bH2MP
#define WGT_OLDMISS	Games.bOLDM
#define VID_STRUCT	(Games.Video)
#define LAUNCH_STRUCT	(Games.Launch)
#define WGT_LANBUTTON	Games.Others.bLAN

// Friendlier definitions for use in callbacks.c
#define RESOL_TEXT1	Video.rText
#define RESOL_ADJUST	Video.rAdj

#define MIDI_BUTTON	bMIDI
#define CDAUDIO_BUTTON	bCDA
#define RESOL_TEXT0	rText
#define STATUSBAR	LStat
#define PORTALS_BUTTON	bH2MP
#define OLD_MISSION	bOLDM
#define LAN_BUTTON	Others.bLAN
#define LAUNCH_BUTTON	bLAUNCH

/* These widgets aren't played remotely by other
   functions, so ne need to place them in here:
   
   bJOY		Jostick disabling button
   bQUIT	Quit button
   bSAVE	Save Options button

   And the other separators, static labels, etc, of course.
*/
