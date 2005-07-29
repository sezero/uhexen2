/*
 * $Header: /home/ozzie/Download/0000/uhexen2/00_unused/gamecode/hw/glyph.hc,v 1.1.1.1 2005-07-29 20:44:25 sezero Exp $
 */

void (float explodetype) BecomeExplosion ;

void () TouchGlyph =
{
	local float damg;

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_EXPLOSION);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);

	BecomeExplosion (FALSE);

   damg = GLYPH_BASE_DAMAGE + random(GLYPH_ADD_DAMAGE);

	self.owner = self;

	if (other.health)
		T_Damage (other, self, self.owner, damg );

	// don't do radius damage to the other, because all the damage
	// was done in the impact
	T_RadiusDamage (self, self.owner, damg, other);
	T_RadiusManaDamage (self, self.owner, damg, other);

	remove(self);
};

void () DecrementGlyph =
{

	if (self.lifetime > time)
	{
		self.nextthink = time + 1;
		return;
	}
	else
		TouchGlyph ();

};

void () Use_Glyph =
{
	local entity glyph;

	glyph = spawn();

	glyph.owner = self;
	
//	precache_model("models/glyph.mdl");
	CreateEntityNew(self,ENT_GLYPH,"models/glyph.mdl",SUB_Null);

	glyph.touch = TouchGlyph;
	glyph.classname = "running_glyph";

	setorigin (glyph, self.origin);

	glyph.think = DecrementGlyph;
	glyph.nextthink = time + 1;
	glyph.lifetime = time + 10;

};

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/29 11:26:31  sezero
 * Initial import
 *
 * Revision 1.1.1.1  2001/11/09 17:05:04  theoddone33
 * Inital import
 *
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 11    6/21/97 10:09a Rlove
 * 
 * 10    6/18/97 6:36p Mgummelt
 * 
 * 9     6/04/97 8:16p Mgummelt
 * 
 * 8     5/28/97 8:13p Mgummelt
 * 
 * 7     5/28/97 10:45a Rlove
 * Moved sprite effects to client side - smoke, explosions, and flashes.
 * 
 * 6     4/30/97 5:03p Mgummelt
 * 
 * 5     3/27/97 11:26a Rlove
 * More precache model clean up
 * 
 * 4     3/19/97 5:56p Jweier
 * added CreateEntity (was backed up!)
 * 
 * 3     1/06/97 3:28p Rlove
 * Now blows up when someone comes in contact with it.
 * 
 * 2     12/31/96 8:39a Rlove
 * Glyph of the Ancients is working
 * 
 * 1     12/30/96 10:27a Rlove
 */
