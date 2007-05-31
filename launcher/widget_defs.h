/*
	widget_defs.h
	hexen2 launcher: gtk+ widget defs

	$Id: widget_defs.h,v 1.18 2007-05-31 21:27:26 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef	LAUNCHER_WIDGETDEFS_H
#define	LAUNCHER_WIDGETDEFS_H

/* Widgets to be played with remotely are put into
   these structs properly for easier handling.
*/

typedef struct
{
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
} MainWindow_t;

typedef struct
{
	GtkWidget *mywindow;	// Main window
	GtkWidget *fixed1;	// Widgets container
	GtkWidget *bBASEDIR;	// Use a different game basedir
	GtkWidget *bCLOSE;	// Close button
	GtkWidget *bAPPLY;	// Apply patch button
	GtkWidget *bREPORT;	// Report installation status
	GtkWidget *LOGVIEW;	// LogEntry line for patch process
	GtkWidget *dir_Entry;	// path for game basedir
	GtkWidget *StatusBar;	// Status bar, (patch status)
	gint	statbar_id;	// statbar context id
	guint	delete_handler;
} PatchWindow_t;

struct Launch_s
{
	GtkWidget *StatusBar;	// Status bar, (launch status)
	gint	statbar_id;	// statbar context id
	GtkWidget *bLAUNCH;	// Launch button
};

struct Video_s
{
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

struct Sound_s
{
	GtkWidget *cSND;	// Global sound option
	GtkWidget *cSRATE;	// Sampling rate
	GtkWidget *bSBITS;	// Sample format
	GtkWidget *bMIDI;	// Midi music option
	GtkWidget *bCDA;	// CD Audio option
};

struct Misc_s
{
	GtkWidget *bLAN;	// LAN button
	GtkWidget *bMOUSE;	// Mouse button
	GtkWidget *bDBG;	// Log Debuginfo button
	GtkWidget *bDBG2;	// Full Log Debuginfo
	GtkWidget *bMEMHEAP;	// Heapsize check button
	GtkObject *adjHEAP;	// Heapsize adjustment
	GtkWidget *spnHEAP;	// Heapsize entry
	GtkWidget *bMEMZONE;	// Zonesize check button
	GtkObject *adjZONE;	// Zonesize adjustment
	GtkWidget *spnZONE;	// Zonesize entry
};

typedef struct
{
	GtkWidget *bHEXEN2;	// Hexen II
	GtkWidget *bH2W;	// HexenWorld Client
	GtkWidget *SelH2;	// Hexen2 Game type selection
	GtkWidget *SelHW;	// HexenWorld Game type selection
	GtkWidget *bH2MP;	// Expansion Pack
	struct Launch_s Launch;
	struct Video_s Video;
	struct Sound_s Sound;
	struct Misc_s Others;
} options_widget_t;


// These crap are some supposed-to-be-friendlier macros for
// the main window layout members..

// Friendlier definitions for main window layout members
#define	MAIN_WINDOW	main_win.mywindow
#define	PATCH_WINDOW	patch_win.mywindow
#define	HOLDER_BOX	main_win.main_box
#define	BOOK0		main_win.notebook0
#define	BOOK1		main_win.notebook1
#define	BASIC_TAB	main_win.fixed0
#define	PATCH_TAB	patch_win.fixed1
#define	ADDON_TAB1	main_win.fixed1
#define	ADDON_TAB2	main_win.fixed2
#define	TAB0_LABEL	main_win.lbl_tab0
#define	TAB1_LABEL	main_win.lbl_tab1
#define	TAB2_LABEL	main_win.lbl_tab2
#define	MORE_LESS	main_win.bMORE

// Friendlier definitions for struct members
// WGT for "widget"
#define	_OPT		Options
#define	_VIDEO		_OPT.Video
#define	_SOUND		_OPT.Sound
#define	_LAUNCH		_OPT.Launch
#define	_OTHERS		_OPT.Others

#define	WGT_HEXEN2	_OPT.bHEXEN2
#define	WGT_H2WORLD	_OPT.bH2W
#define	WGT_H2GAME	_OPT.SelH2
#define	WGT_HWGAME	_OPT.SelHW
#define	WGT_PORTALS	_OPT.bH2MP

#define	WGT_OPENGL	_VIDEO.bOGL
#define	WGT_RESCOMBO	_VIDEO.cRES
#define	WGT_RESLIST	_VIDEO.eRES
#define	WGT_CONWBUTTON	_VIDEO.bCONW
#define	WGT_CONWCOMBO	_VIDEO.cCONW
#define	WGT_CONWLIST	_VIDEO.eCONW
#define	WGT_FULLSCR	_VIDEO.bFULS
#define	WGT_3DFX	_VIDEO.b3DFX
#define	WGT_GL8BIT	_VIDEO.b8BIT
#define	WGT_LM_BYTES	_VIDEO.bLM_BYTES
#define	WGT_VSYNC	_VIDEO.bVSYNC
#define	WGT_FSAA	_VIDEO.bFSAA
#define	WGT_ADJFSAA	_VIDEO.adjFSAA
#define	WGT_AASAMPLES	_VIDEO.spnFSAA
#define	WGT_GLPATH	_VIDEO.GL_Entry
#define	WGT_LIBGL	_VIDEO.bLIBGL

#define	WGT_SOUND	_SOUND.cSND
#define	WGT_SRATE	_SOUND.cSRATE
#define	WGT_SBITS	_SOUND.bSBITS
#define	WGT_MIDI	_SOUND.bMIDI
#define	WGT_CDAUDIO	_SOUND.bCDA

#define	WGT_LANBUTTON	_OTHERS.bLAN
#define	WGT_MOUSEBUTTON	_OTHERS.bMOUSE
#define	WGT_DBGLOG	_OTHERS.bDBG
#define	WGT_DBGLOG2	_OTHERS.bDBG2
#define	WGT_MEMHEAP	_OTHERS.bMEMHEAP
#define	WGT_HEAPADJ	_OTHERS.adjHEAP
#define	WGT_HEAPSIZE	_OTHERS.spnHEAP
#define	WGT_MEMZONE	_OTHERS.bMEMZONE
#define	WGT_ZONEADJ	_OTHERS.adjZONE
#define	WGT_ZONESIZE	_OTHERS.spnZONE

#define	WGT_LAUNCH	_LAUNCH.bLAUNCH
#define	WGT_STATBAR_ID	_LAUNCH.statbar_id
#define	WGT_STATUSBAR	_LAUNCH.StatusBar

#define	PATCH_STATBAR	patch_win.StatusBar

#endif	/* LAUNCHER_WIDGETDEFS_H */

