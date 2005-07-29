/*
 * $Header: /home/ozzie/Download/0000/uhexen2/00_unused/gamecode/siege/jctest.hc,v 1.1.1.1 2005-07-29 20:43:13 sezero Exp $
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
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/01/26 17:26:11  sezero
 * Raven's original Siege hcode.
 *
 * 
 * 3     5/25/98 1:39p Mgummelt
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 2     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */
