/*
	menu.h

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/menu.h,v 1.5 2006-01-07 09:36:09 sezero Exp $
*/

//
// the net drivers should just set the apropriate bits in m_activenet,
// instead of having the menu code look through their internal tables
//
#define	MNET_IPX		1
#define	MNET_TCP		2

extern	int	m_activenet;

extern char	*plaquemessage;     // Pointer to current plaque
extern char	*errormessage;     // Pointer to current plaque

extern char BigCharWidth[27][27];

//
// menus
//
void M_Init (void);
void M_Keydown (int key);
void M_ToggleMenu_f (void);

void M_Menu_Options_f (void);
void M_Menu_Quit_f (void);

void M_Print (int x, int y, char *str);
void M_Print2 (int x, int y, char *str);
void M_PrintWhite (int x, int y, char *str);

void M_Draw (void);
void M_DrawCharacter (int cx, int line, int num);

void M_DrawPic (int x, int y, qpic_t *pic);
void M_DrawTransPic (int x, int y, qpic_t *pic);
void M_DrawTextBox (int x, int y, int width, int lines);
void M_DrawTextBox2 (int x, int y, int w, int l, qboolean b);

void M_DrawCheckbox (int x, int y, int on);

void ScrollTitle (char *name);


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/01/01 21:43:47  sezero
 * prototypes clean-up
 *
 * Revision 1.3  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:05:05  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 5     9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 4     4/17/97 3:42p Rjohnson
 * Modifications for the gl version for menus
 * 
 * 3     2/19/97 11:29a Rjohnson
 * Id Updates
 */

