
//**************************************************************************
//**
//** sbar.h
//**
//** $Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/sbar.h,v 1.2 2005-01-01 21:55:47 sezero Exp $
//**
//**************************************************************************

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

void Sbar_Init(void);
void Sbar_Changed(void);
void Sbar_DeathmatchOverlay(void);
void Sbar_Draw(void);
void Sbar_IntermissionOverlay(void);
void Sbar_FinaleOverlay(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int sb_lines; // scan lines to draw
