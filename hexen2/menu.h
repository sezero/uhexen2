/*
 * $Header: /home/ozzie/Download/0000/uhexen2/hexen2/menu.h,v 1.1.1.1 2004-11-28 00:05:05 sezero Exp $
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
void M_Draw (void);
void M_ToggleMenu_f (void);

void M_DrawTextBox (int x, int y, int width, int lines);

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:04:01  theoddone33
 * Inital import
 *
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

