/*
	sbar.h

	$Id: sbar.h,v 1.4 2005-08-23 12:31:43 sezero Exp $
*/

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SB_Init(void);
void SB_Changed(void);
void SB_Draw(void);
#if 0
void SB_IntermissionOverlay(void);
void SB_FinaleOverlay(void);
#endif
void SB_InvChanged(void);
void SB_InvReset(void);
void SB_ViewSizeChanged(void);
void Sbar_DeathmatchOverlay(void);

void Sbar_Init(void);
void Sbar_Changed(void);
void Sbar_Draw(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int sb_lines; // scan lines to draw
