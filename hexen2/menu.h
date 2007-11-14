/*
	menu.h

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/menu.h,v 1.11 2007-11-14 07:32:20 sezero Exp $
*/

#ifndef __HX2_MENU_H
#define __HX2_MENU_H

//
// the net drivers should just set the apropriate bits in m_activenet,
// instead of having the menu code look through their internal tables
//
#define	MNET_IPX		1
#define	MNET_TCP		2

extern	int	m_activenet;

//
// menus
//
void M_Init (void);
void M_Keydown (int key);
void M_ToggleMenu_f (void);

void M_Menu_Options_f (void);
void M_Menu_Quit_f (void);

void M_Print (int x, int y, const char *str);
void M_Print2 (int x, int y, const char *str);
void M_PrintWhite (int x, int y, const char *str);

void M_Draw (void);
void M_DrawCharacter (int cx, int line, int num);

void M_DrawPic (int x, int y, qpic_t *pic);
void M_DrawTransPic (int x, int y, qpic_t *pic);
void M_DrawTextBox (int x, int y, int width, int lines);
void M_DrawTextBox2 (int x, int y, int w, int l, qboolean b);

void M_DrawCheckbox (int x, int y, int on);

void ScrollTitle (const char *name);

#endif	/* __HX2_MENU_H */

