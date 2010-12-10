/*
 * $Header: /cvsroot/uhexen2/reference/unused/gamecode/siege/flag.hc,v 1.2 2007-02-07 16:56:44 sezero Exp $
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

