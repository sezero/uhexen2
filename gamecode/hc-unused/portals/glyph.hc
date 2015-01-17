/*
 * $Header: /H3MP/hcode/glyph.hc 13    10/28/97 1:01p Mgummelt $
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

