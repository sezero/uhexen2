/*
	screen.h

	$Id: screen.h,v 1.8 2007-08-20 08:16:06 sezero Exp $
*/

#ifndef __HX2_SCREEN_H
#define __HX2_SCREEN_H

// width for the info plaques
#define PLAQUE_WIDTH	26

void SCR_Init (void);
void SCR_UpdateScreen (void);
void SCR_UpdateWholeScreen (void);

void SCR_CenterPrint (const char *str);
//void SCR_BringDownConsole (void);

void SCR_BeginLoadingPlaque (void);
void SCR_EndLoadingPlaque (void);

int SCR_ModalMessage (char *text);

void SCR_DrawLoading (void);	// for the Loading plaque

extern	int			total_loading_size;	// global vars for
extern	int			current_loading_size;	// the Loading screen
extern	int			entity_file_size, loading_stage;

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	int			scr_fullupdate;	// set to 0 to force full redraw
extern	int			scr_topupdate;	// set to 0 to force top redraw
extern	int			sb_lines;

extern	int			clearnotify;	// set to 0 whenever notify text is drawn
extern	qboolean	scr_disabled_for_loading;
extern	qboolean	scr_skipupdate;
extern	qboolean	block_drawing;

extern	cvar_t		scr_viewsize;

// only the refresh window will be updated unless these variables are flagged 
extern	int			scr_copytop;
extern	int			scr_copyeverything;

#endif	/* __HX2_SCREEN_H */

