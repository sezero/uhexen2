/*
 * $Header: /HexenWorld/Siege/Jctest.hc 3     5/25/98 1:39p Mgummelt $
 */

void() jctrig =
{
dprint ("here\n\n");
	lightstyle(0, "az");
};

/*QUAKED trigger_jctest (.5 .5 .5) ?
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void() trigger_jctest =
{
	setsize (self, self.mins, self.maxs);
	self.solid = SOLID_EDGE;
	self.touch = jctrig;
};

