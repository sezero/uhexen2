/*
 * $Header: /HexenWorld/Siege/Flag.hc 3     5/25/98 1:38p Mgummelt $
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

