/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/ravenai.hc,v 1.2 2007-02-07 17:01:22 sezero Exp $
 */

float() LocateTarget =
{
	return FindTarget(TRUE);
};

float MA_SUCCESSFUL	= 0;
float MA_BLOCKED	= -1;
float MA_CROSSED	= -2;
float MA_NOWEAPON	= -3;
float MA_TOOSOON	= -4;
float MA_TOOFAR		= -5;
float MA_NOATTACK	= -6;

float MA_MELEE		= 1;
float MA_MISSILE	= 2;
float MA_BOTH		= 3;
float MA_FAR_MELEE	= 4;
float MA_SHORT_MISSILE	= 8;

// You must perform the following call sometime before calling this function:
//    enemy_range = range (self.enemy);
float(float AttackType, float ChanceModifier) CheckMonsterAttack =
{
	local vector	spot1, spot2;
	local entity	targ;
	local float	chance;

	targ = self.enemy;

	if (self.classname == "monster_hydra")
		if (self.enemy.watertype != CONTENT_WATER)
		{
			if (self.search_time < time)
			{
				self.monster_stage = 0;
				self.enemy = world;
				return 0;
			}
			return 0;
		}
		else self.search_time = time + 5;

// see if any entities are in the way of the shot
	spot1 = self.origin + self.view_ofs;
	spot2 = targ.origin + targ.view_ofs;

	traceline (spot1, spot2, FALSE, self);

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != targ)
		if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
			return MA_BLOCKED;		// don't have a clear shot

	if (trace_inopen && trace_inwater)
		return MA_CROSSED;			// sight line crossed contents

	if (enemy_range == RANGE_MELEE)
	{	// melee attack
		if (AttackType & MA_SHORT_MISSILE)
		{
			if (random() < 0.5)
			{
			   self.th_missile ();
			   return MA_SUCCESSFUL;
			}
		}
		if (self.th_melee)
		{
			if (AttackType & MA_MELEE)
			{
				self.th_melee ();
				return MA_SUCCESSFUL;
			}
			else
				return MA_NOWEAPON;
		}
	}

// missile attack
	if (!self.th_missile || !(AttackType & (MA_MISSILE | MA_FAR_MELEE)))
	{
		return MA_NOWEAPON;
	}

	if (time < self.attack_finished)
		return MA_TOOSOON;

	if (enemy_range == RANGE_FAR)
		return MA_TOOFAR;

	if (enemy_range == RANGE_MELEE)
	{
		chance = 0.9;
		self.attack_finished = 0;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		if (self.th_melee)
			chance = 0.2;
		else
			chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		if (self.th_melee)
			chance = 0.05;
		else
			chance = 0.1;
	}
	else
		chance = 0;

	chance = chance * ChanceModifier;
	if (chance > 0.95) chance = 0.95;

	if (random () < chance)
	{
		if (self.th_melee)
		{  // quake c wouldn't allow me to put this in on if!!!
			if	(AttackType & MA_FAR_MELEE)
			{
				self.th_melee ();
			}
			else
			{
				self.th_missile ();
			}
		}
		else
		{
			self.th_missile ();
		}
//		SUB_AttackFinished (random(2,0));
		SUB_AttackFinished (random(8,0));
		return MA_SUCCESSFUL;
	}

	return MA_NOATTACK;
};

/*
float (vector offset, vector to_where)monster_checkpos =
{
	// This function will trace 2 lines - the first line will go from the origin to the offset from the origin.
	// If this could be done atleast half way, then a 2nd trace is done from the end point of the first trace
	// to the final destination.  If this was mostly successful, then the function will return true, otherwise 
	// false.  You would use this function in a situation where you want to see if the enemy is visibile from
	// your right side, so you first see if you can go do the right side, then go forward from there to the enemy.
	local vector start;
	local float length;

	start = self.origin + offset;
	traceline (self.origin, start, FALSE, self);
	if (trace_fraction < 0.5)
	{  // Couldn't get to the offset
		return FALSE;
	}
	length = vlen(self.origin-start) * trace_fraction;
	start = trace_endpos;
	traceline (start,to_where, FALSE, self);
	if (trace_ent == self.enemy || trace_fraction > 0.98) 
	{  // We found the enemy!
		length = length + vlen(start-self.enemy.origin) * trace_fraction;
		return length;
	}

	return FALSE;
};

void (float l, float r, float u, float d, float last_move, vector where) find_path =
{
	// This function will check to see if an enemy can be located from the top, bottom, left, and right sides.
	// The l, r, u, d parameters specify the order for which the search should be done.  If it couldn't find
	// the enemy, then it will try the last seen position of the enemy.  last_move indicates that the previous
	// move was successful (i.e. the monster could move forward).  where is the position to check for.
	local float length;
	local float newyaw;
	local float newz;
	local float c;
	local float retval;
//	local vector a, b;

	makevectors (self.angles);
	length = 99999;
	newyaw = self.ideal_yaw;
	newz = self.velocity_z;

	c = 0;
	while(c<=4)
	{  // We have 5 checks to do
		if (c == 0 && last_move)
		{  // Try checking forward
			retval = monster_checkpos(v_forward*300,where);
			if (retval && retval < length)
			{
				//dprint("found you to the forward\n");
				self.monster_duration = 18 + 5;
				length = retval;
			}
		}
		if (c == l)
		{  // Try checking to the left
			retval = monster_checkpos(v_right*-200,where);
			if (retval && retval < length)
			{
				//dprint("found you to the left\n");
				newyaw = self.angles_y + 90;
				self.monster_duration = 18 + 5;
				length = retval;
			}
		}
		else if (c == r)
		{  // Try checking to the right
			retval = monster_checkpos(v_right*200,where);
			if (retval && retval < length)
			{
				//dprint("found you to the right\n");
				newyaw = self.angles_y - 90;
				self.monster_duration = 18 + 5;
				length = retval;
			}
		}
		else if (c == u)
		{  // Try checking to the top
			retval = monster_checkpos(v_up*200,where);
			if (retval && retval < length)
			{
				//dprint("found you to the up\n");
				newz = 30;
				self.monster_duration = 18 + 5;
				length = retval;
			}
		}
		else if (c == d)
		{  // Try checking to the bottom
			retval = monster_checkpos(v_up*-200,where);
			if (retval && retval < length)
			{
				//dprint("found you to the down\n");
				newz = -30;
				self.monster_duration = 18 + 5;
				length = retval;
			}
		}
		c = c + 1;
	}

	if (length == 99999 && self.monster_last_seen != where)
	{  // If we didn't find a direction, and we haven't done this, try looking where the enemy
		// was last seen
		find_path(l,r,u,d,0,self.monster_last_seen);
		//dprint("Using last seen\n");
	}
	else
	{
		self.ideal_yaw = newyaw;
		self.velocity_z = newz;
	}
};

float () FindDir =
{ // Monster couldn't go in the direction it is pointed to, so find one it can go to
	local vector a,b,c;
	local float inc,step;

	if (random() < 0.5) inc = 45;
	else inc = -45;

	c = '0 0 0';
	c_y = c_y + inc;

	step = 0;
	while(step < 6)
	{  // 7 directions to check (45 degrees each)
		makevectors (self.angles + c);
		a = self.origin + self.view_ofs;
		b = a + v_forward*100;

		traceline (a, b, FALSE, self);

		if (trace_fraction > 0.9)
		{  // We can mostly go this direction
			return self.angles_y + c_y;
			//dprint("   found\n");
		}
		c_y = c_y + inc;
		step = step + 1;
	}

	return self.angles_y;
};
*/
