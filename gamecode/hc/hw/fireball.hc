/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/fireball.hc,v 1.1.1.1 2004-11-29 11:26:21 sezero Exp $
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






















/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:03  theoddone33
 * Inital import
 *
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 20    9/19/97 2:06p Rlove
 * 
 * 19    9/04/97 3:19p Mgummelt
 * 
 * 18    9/04/97 3:00p Mgummelt
 * 
 * 17    9/02/97 2:55a Mgummelt
 * 
 * 16    8/26/97 8:11p Jweier
 * 
 * 15    8/15/97 2:12a Mgummelt
 * 
 * 14    8/13/97 1:28a Mgummelt
 * 
 * 13    8/09/97 1:49a Mgummelt
 * 
 * 12    7/18/97 11:06a Mgummelt
 * 
 * 11    7/15/97 4:47p Rjohnson
 * Updates
 * 
 * 10    6/19/97 3:08p Rjohnson
 * Code space optimizations
 * 
 * 9     6/14/97 2:21p Mgummelt
 * 
 * 8     6/03/97 10:48p Mgummelt
 * 
 * 7     5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 6     5/08/97 9:47p Mgummelt
 * 
 * 5     4/25/97 8:32p Mgummelt
 * 
 * 4     4/24/97 2:21p Mgummelt
 * 
 * 3     2/13/97 1:23p Rlove
 * Changes to Blast Radius so it will affect missiles.
 * 
 * 2     12/11/96 11:44a Rjohnson
 * Used monster modifiers for the fireball damage
 * 
 * 1     12/09/96 11:51a Rjohnson
 * Initial Revision
 * 
 */
