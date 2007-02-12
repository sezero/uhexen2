#ifndef	LAUNCHER_WIDGETDEFS_H
#define	LAUNCHER_WIDGETDEFS_H

/* Widgets to be played with remotely are put into
   these structs properly for easier handling.
*/

typedef struct {
	GtkWidget *mywindow;	// Main window
	GtkWidget *main_box;	// Two pane horizontal box
	GtkWidget *notebook0;	// Basic options pane
	GtkWidget *fixed0;	// Basic options container
//	GtkWidget *lbl_tab0;	// Basic options label
	GtkWidget *notebook1;	// Additional options pane
	GtkWidget *fixed1;	// Additional options Tab-1
	GtkWidget *lbl_tab1;	// Additional options Tab-1 label
	GtkWidget *fixed2;	// Additional options Tab-2
	GtkWidget *lbl_tab2;	// Additional options Tab-2 label
	GtkWidget *bMORE;	// More/Less button
} HoTWindow_t;

typedef struct {
	GtkWidget *mywindow;	// Main window
	GtkWidget *fixed1;	// Widgets container
	GtkWidget *bCLOSE;	// Close button
	GtkWidget *bAPPLY;	// Apply patch button
	GtkWidget *LOGVIEW;	// LogEntry line for patch process
	GtkWidget *PStat;	// Status bar, (patch status)
	gint	BinStat;	// statbar context id
} PatchWindow_t;

struct Launch_s {
	GtkWidget *LStat;	// Status bar, (launch status)
	gint	BinStat;	// statbar context id
	GtkWidget *bLAUNCH;	// Launch button
};

struct Video_s {
	GtkWidget *bOGL;	// OpenGL / Software
	GtkWidget *cRES;	// Resolution combo
	GtkWidget *eRES;	// Resolution combo listing
	GtkWidget *bCONW;	// Allow conwidth toggle
	GtkWidget *cCONW;	// Conwidth combo
	GtkWidget *eCONW;	// Conwidth combo listing
	GtkWidget *bFULS;	// Fullscreen / windowed
	GtkWidget *b3DFX;	// 3dfx specific Gamma
	GtkWidget *b8BIT;	// 8-bit texture extensions
	GtkWidget *bFSAA;	// Multisampling check button
	GtkObject *adjFSAA;	// Multisampling adjustment
	GtkWidget *spnFSAA;	// Multisampling entry
	GtkWidget *bVSYNC;	// Enable vertical sync
	GtkWidget *bLM_BYTES;	// Whether to use GL_LUMINANCE lightmaps
	GtkWidget *bLIBGL;	// Custom GL library toggle
	GtkWidget *GL_Entry;	// Custom GL library path
};

struct Misc_s {
	GtkWidget *bLAN;	// LAN button
	GtkWidget *bMOUSE;	// Mouse button
	GtkWidget *bDBG;	// Log Debuginfo button
	GtkWidget *bMEMHEAP;	// Heapsize check button
	GtkObject *adjHEAP;	// Heapsize adjustment
	GtkWidget *spnHEAP;	// Heapsize entry
	GtkWidget *bMEMZONE;	// Zonesize check button
	GtkObject *adjZONE;	// Zonesize adjustment
	GtkWidget *spnZONE;	// Zonesize entry
};

typedef struct {
	GtkWidget *cSND;	// Global sound option
	GtkWidget *cSRATE;	// Sampling rate
	GtkWidget *bSBITS;	// Sample format
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


// These crap are some supposed-to-be-friendlier macros for
// the main window layout members..

// Friendlier definitions for main window layout members
#define MAIN_WINDOW	main_win.mywindow
#define PATCH_WINDOW	patch_win.mywindow
#define HOLDER_BOX	main_win.main_box
#define BOOK0		main_win.notebook0
#define BOOK1		main_win.notebook1
#define BASIC_TAB	main_win.fixed0
#define PATCH_TAB	patch_win.fixed1
#define ADDON_TAB1	main_win.fixed1
#define ADDON_TAB2	main_win.fixed2
#define TAB0_LABEL	main_win.lbl_tab0
#define TAB1_LABEL	main_win.lbl_tab1
#define TAB2_LABEL	main_win.lbl_tab2
#define MORE_LESS	main_win.bMORE

// Friendlier definitions for struct members
// WGT for "widget"
#define WGT_STATUSBAR	Games.Launch.LStat
#define PATCH_STATBAR	patch_win.PStat
#define BIN_STATUS_ID	Games.Launch.BinStat
#define WGT_LAUNCH	Games.Launch.bLAUNCH
#define WGT_OPENGL	Games.Video.bOGL
#define WGT_RESCOMBO	Games.Video.cRES
#define WGT_RESLIST	Games.Video.eRES
#define WGT_CONWBUTTON	Games.Video.bCONW
#define WGT_CONWCOMBO	Games.Video.cCONW
#define WGT_CONWLIST	Games.Video.eCONW
#define WGT_FULLSCR	Games.Video.bFULS
#define WGT_3DFX	Games.Video.b3DFX
#define WGT_GL8BIT	Games.Video.b8BIT
#define WGT_LM_BYTES	Games.Video.bLM_BYTES
#define WGT_VSYNC	Games.Video.bVSYNC
#define WGT_FSAA	Games.Video.bFSAA
#define WGT_ADJFSAA	Games.Video.adjFSAA
#define WGT_AASAMPLES	Games.Video.spnFSAA
#define WGT_GLPATH	Games.Video.GL_Entry
#define WGT_LIBGL	Games.Video.bLIBGL
#define WGT_SOUND	Sound.cSND
#define WGT_SRATE	Sound.cSRATE
#define WGT_SBITS	Sound.bSBITS
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
#define WGT_MOUSEBUTTON	Games.Others.bMOUSE
#define WGT_DBGLOG	Games.Others.bDBG
#define WGT_MEMHEAP	Games.Others.bMEMHEAP
#define WGT_HEAPADJ	Games.Others.adjHEAP
#define WGT_HEAPSIZE	Games.Others.spnHEAP
#define WGT_MEMZONE	Games.Others.bMEMZONE
#define WGT_ZONEADJ	Games.Others.adjZONE
#define WGT_ZONESIZE	Games.Others.spnZONE

// Friendlier definitions for use in callbacks.c
#define SND_RATE	cSRATE
#define SND_BITS	bSBITS
#define MUSIC_MIDI	bMIDI
#define MUSIC_CD	bCDA
#define RES_COMBO	cRES
#define RES_LIST	eRES
#define CONW_COMBO	cCONW
#define CONW_LIST	eCONW
#define CONW_BUTTON	bCONW
#define STATUSBAR	LStat
#define PORTALS		bH2MP
#define H2GAME		SelH2
#define HWGAME		SelHW
#define CONW_COMBO_S	Video.cCONW
#define CONW_BUTTON_S	Video.bCONW
#define TDFX_BUTTON	Video.b3DFX
#define GL8BIT_BUTTON	Video.b8BIT
#define LM_BUTTON	Video.bLM_BYTES
#define VSYNC_BUTTON	Video.bVSYNC
#define FSAA_BUTTON	Video.bFSAA
#define LIBGL_BUTTON	Video.bLIBGL
#define LAN_BUTTON	Others.bLAN
#define LAUNCH_BUTTON	bLAUNCH

#endif	// LAUNCHER_WIDGETDEFS_H

