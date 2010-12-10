/*
 * $Header: /cvsroot/uhexen2/reference/unused/gamecode/siege/jctest.hc,v 1.2 2007-02-07 16:56:46 sezero Exp $
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

