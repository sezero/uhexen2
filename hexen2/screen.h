// screen.h

void SCR_Init (void);

void SCR_UpdateScreen (void);


void SCR_SizeUp (void);
void SCR_SizeDown (void);
void SCR_BringDownConsole (void);
void SCR_CenterPrint (char *str);

void SCR_BeginLoadingPlaque (void);
void SCR_EndLoadingPlaque (void);

int SCR_ModalMessage (char *text);

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	int			scr_fullupdate;	// set to 0 to force full redraw
extern	int			scr_topupdate;	// set to 0 to force top redraw
extern	int			sb_lines;

extern	int			clearnotify;	// set to 0 whenever notify text is drawn
extern	qboolean	scr_disabled_for_loading;
extern	qboolean	scr_skipupdate;

extern	cvar_t		scr_viewsize;

extern cvar_t scr_viewsize;

// only the refresh window will be updated unless these variables are flagged 
extern	int			scr_copytop;
extern	int			scr_copyeverything;

extern int			total_loading_size, current_loading_size, entity_file_size, loading_stage;

void SCR_UpdateWholeScreen (void);
