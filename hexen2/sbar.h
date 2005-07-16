/*
	sbar.h

	$Id: sbar.h,v 1.4 2005-07-16 23:35:19 sezero Exp $
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
void Sbar_DeathmatchOverlay(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int sb_lines; // scan lines to draw
