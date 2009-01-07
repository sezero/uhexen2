/*
	console.h
	the game console

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/console.h,v 1.11 2009-01-07 09:36:33 sezero Exp $
*/

#ifndef __CONSOLE_H
#define __CONSOLE_H

#define		CON_TEXTSIZE	16384
extern	int con_totallines;
extern	int con_backscroll;
extern	qboolean con_forcedup;	// because no entities to refresh
extern	qboolean con_initialized;
extern	byte *con_chars;
extern	int con_notifylines;	// scan lines to clear for notify lines

void Con_DrawCharacter (int cx, int line, int num);

void Con_CheckResize (void);
void Con_Init (void);
void Con_DrawConsole (int lines);

void Con_ShowList (int , const char **);
//void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);

void Con_NotifyBox (const char *text);	// during startup for sound / cd warnings

#endif	/* __CONSOLE_H */

