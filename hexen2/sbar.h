/*
	sbar.h

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sbar.h,v 1.2 2004-12-12 14:14:42 sezero Exp $
*/

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SB_Init(void);
void SB_Changed(void);
void SB_Draw(void);
void SB_IntermissionOverlay(void);
void SB_FinaleOverlay(void);
void SB_InvChanged(void);
void SB_InvReset(void);
void SB_ViewSizeChanged(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int sb_lines; // scan lines to draw
