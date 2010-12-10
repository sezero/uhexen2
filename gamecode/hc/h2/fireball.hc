/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/fireball.hc,v 1.2 2007-02-07 16:57:04 sezero Exp $
 */
/*
==============================================================================

FIREBALL

==============================================================================
*/

// For building the model
$cd c:\model\fireball		// Directory to find model in
$origin 0 0 0		
// baseframe is in iceimp.3ds
$base fireball		
// skin is in iceimp.lbm
$skin fireball		

// Imp throwing 
$frame firbal1  firbal2  firbal3  firbal4  firbal5
$frame firbal6  firbal7  firbal8  firbal9  firbal10


//============================================================================
void FireFizzle (void)
{
	sound (self, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
	DeathBubbles(1);
	remove(self);
}

void() fireballTouch =
{
	local float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if (self.dmg == -1)
		damg = random(5,10);
	else if (self.dmg)
		damg = self.dmg;
	else
		damg = random(12,22);
	
	if (other.health)
	{
		if (self.owner.classname == "cube_of_force")
			T_Damage (other, self.owner, self.owner.owner, damg );
		else
			T_Damage (other, self, self.owner, damg );
	}

	// don't do radius damage to the other, because all the damage
	// was done in the impact
	T_RadiusDamage (self, self.owner, damg, other);

//	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
	self.origin = self.origin - 8*normalize(self.velocity);

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_EXPLOSION);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);

	remove(self);
//	BecomeExplosion ();
};





//============================================================================


void fireball_1(void)
{
	float retval;

	self.nextthink = time + HX_FRAME_TIME;

	retval = AdvanceFrame($firbal1,$firbal10);

	if (retval == AF_BEGINNING)
	{
		if (pointcontents(self.origin) != CONTENT_EMPTY) 
			FireFizzle();
	}
}

//============================================================================


void(vector offset) do_fireball =
{
entity missile;
vector vec;

	missile = spawn ();
	missile.owner = self;
	missile.speed=500;
	if(self.classname=="monster_imp_lord")
	{
		missile.dmg=random(80,120);
		missile.speed+=500;
		missile.scale=2;
	}
	else
		missile.dmg = self.dmg;

	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.health = 10;

	setmodel (missile, "models/fireball.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

// set missile speed	

	makevectors (self.angles);
	vec = self.origin + self.view_ofs + v_factor(offset);
	setorigin (missile, vec);

	vec = self.enemy.origin - missile.origin + self.enemy.view_ofs;
	vec = normalize(vec);

	missile.velocity = (vec+aim_adjust(self.enemy))*missile.speed;
	missile.angles = vectoangles('0 0 0'-missile.velocity);
	
	missile.touch = fireballTouch;

	missile.think = fireball_1;
	missile.nextthink = time + HX_FRAME_TIME;
};

