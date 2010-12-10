/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/spit.hc,v 1.2 2007-02-07 16:57:10 sezero Exp $
 */
/*
==============================================================================

spit

==============================================================================
*/

// For building the model
$cd c:\model\spit		// Directory to find model in
$origin 0 0 0		
// baseframe is in iceimp.3ds
$base spit		
// skin is in iceimp.lbm
$skin spit		

// Imp throwing 
$frame hyspit1


//============================================================================

void() SpitTouch =
{//FIXME: Add quick palette shift to black, fade back down
	local float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(5,8);
	
	if (other.health)
	{
		T_Damage (other, self, self.owner, damg );
	}

	// don't do radius damage to the other, because all the damage
	// was done in the impact
	T_RadiusDamage (self, self.owner, damg, other);

//	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
	self.origin = self.origin - 8*normalize(self.velocity);

/*	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_EXPLOSION);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);*/

	remove(self);
//	BecomeExplosion ();
};





//============================================================================


void spit_1 (void)
{
	self.veer += 10;
	Veer(self.veer);

	thinktime self : 0.1;
}


//============================================================================


void(vector offset) do_spit =
{
	local	entity missile;
	local vector vec;

	self.last_attack=time;
	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;

	setmodel (missile, "models/spit.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

	// set missile speed	

	makevectors (self.angles);
	setorigin (missile, self.origin + v_factor(offset));

    //If enemy is sinking, lead him a little
	if (self.enemy.velocity_z < 0) 
		vec = self.enemy.origin - missile.origin;
	else 
		vec = self.enemy.origin - missile.origin + self.enemy.view_ofs;
	
	vec = normalize(vec);

	missile.velocity = vec * (650 - random(30));
	missile.angles = vectoangles('0 0 0'-missile.velocity);
	
	missile.touch = SpitTouch;

	missile.veer = 5;
	missile.think = spit_1;
	thinktime missile : 0.1;
};

