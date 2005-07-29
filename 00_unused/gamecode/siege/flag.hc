/*
 * $Header: /home/ozzie/Download/0000/uhexen2/00_unused/gamecode/siege/flag.hc,v 1.1.1.1 2005-07-29 20:42:49 sezero Exp $
 */
/*QUAKED item_deathball (.3 .3 1) (0 0 0) (32 32 32)
-----------------------FIELDS-------------------------

--------------------------------------------------------
*/
void() deathball_touch;

void() item_deathball =
{	
	self.touch = deathball_touch;
};
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/01/26 17:26:10  sezero
 * Raven's original Siege hcode.
 *
 * 
 * 3     5/25/98 1:38p Mgummelt
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 2     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */
