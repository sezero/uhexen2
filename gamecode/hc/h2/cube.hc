/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/h2/cube.hc,v 1.1.1.1 2004-11-29 11:37:48 sezero Exp $
 */


float cube_distance = 500;

void CubeDie(void)
{
	self.owner.artifact_flags(-)self.artifact_flags;
	remove(self);
}

float cube_find_target(void)
{
	entity item;

	item = findradius(self.origin, cube_distance);

	while (item)
	{
		if ((item.flags & FL_MONSTER) || ((item.classname == "player") && deathmatch == 1) && item.health > 0)
		{
			tracearea (self.origin,item.origin,self.mins,self.maxs,FALSE,self);
			if (trace_ent == item)
			{
				if (!item.effects & EF_NODRAW)
				{
					self.enemy = item;
					return TRUE;
				}
			}
		}

		item = item.chain;
	}

	return FALSE;
}


void do_fireball(vector offset);

vector CubeDirection[6] =
{
	'90 0 0',
	'-90 0 0',
	'0 90 0',
	'0 -90 0',
	'0 0 90',
	'0 0 -90'
};

void cube_fire(void)
{
//	float RanVal;
	float Distance;
	entity temp;

	if (time > self.monster_duration || self.owner.health <= 0 || self.shot_cnt >= 10)
	{
		CubeDie();
		return;
	}

	if (!self.enemy)
	{
		self.cnt += 1;
		if (self.cnt > 5)
		{
			cube_find_target();
			self.cnt = 0;
		}
	}

	if (self.enemy)
	{
		if (self.enemy.health <= 0)
		{
			self.enemy = world;
			//self.drawflags (+) DRF_TRANSLUCENT;
		}
	}

	if (self.enemy)
	{
		if (random() < .7)
		{
			Distance = vlen(self.origin - self.enemy.origin);
			if (Distance > cube_distance*2)
			{
				self.enemy = world;
				//self.drawflags (+) DRF_TRANSLUCENT;
			}
			else
			{
				// Got to do this otherwise tracearea sees right through you
				temp = self.owner;
				self.owner = self;

				tracearea (self.origin,self.enemy.origin,self.mins,self.maxs,FALSE,self);
				if (trace_ent == self.enemy)
				{
					self.adjust_velocity = CubeDirection[random(0,5)];
					self.abslight = 1;

					self.shot_cnt += 1;

					do_fireball('0 0 0');
				}
				else 
				{
					self.enemy = world;
					//self.drawflags (+) DRF_TRANSLUCENT;
				}

				self.owner = temp;
			}
		}
	}
}

void cube_rotate(void)
{
	vector NewOffset;
	
	NewOffset = concatv(self.adjust_velocity,'5 5 5');

	self.adjust_velocity -= NewOffset;
	self.v_angle += NewOffset;
}

vector CubeFollowRate = '14 14 14';
vector CubeAttackRate = '3 3 3';

void CubeThinkerB(void)
{
	vector NewSpot;
	float Distance;
	thinktime self : 0.05;

	if (!self.owner.flags2 & FL_ALIVE) 
	{
		CubeDie();
		return;
	}

	if (self.adjust_velocity == '0 0 0')
	{
		cube_fire();
		if (self.adjust_velocity == '0 0 0')
		{
			if (random() < 0.02)
			{
				self.adjust_velocity = CubeDirection[random(0,5)];
			}
		}
	}
	cube_rotate();

	if (self.abslight > .1) 
		self.abslight -= 0.1;

	self.angles = self.owner.angles + self.v_angle;
	
	self.count += random(4,6);
	if (self.count > 360) 
	{
		self.count -= 360;
	}

	Distance = vlen(self.origin - self.owner.origin);
	if (Distance > cube_distance)
	{
		self.enemy = world;
		//self.drawflags (+) DRF_TRANSLUCENT;
	}

	if (self.enemy != world)
	{
		NewSpot = self.enemy.origin + self.enemy.view_ofs;

		if (self.artifact_flags & AFL_CUBE_LEFT)
		{
			NewSpot += (cos(self.count) * 40 * '1 0 0') + (sin(self.count) * 40 * '0 1 0');
		}
		else
		{
			NewSpot += (sin(self.count) * 40 * '1 0 0') + (cos(self.count) * 40 * '0 1 0');
		}

		self.movedir_z += random(10,12);
		if (self.movedir_z > 360) 
		{
			self.movedir_z -= 360;
		}

		NewSpot_z += sin(self.movedir_z) * 10;

		NewSpot = self.origin + concatv(NewSpot - self.origin, CubeAttackRate);
	}
	else
	{
		makevectors(self.owner.v_angle);

		if (self.artifact_flags & AFL_CUBE_LEFT)
		{
   			NewSpot = self.owner.origin + self.owner.view_ofs + '0 0 10' + v_factor('40 60 0');
		}
		else
		{
   			NewSpot = self.owner.origin + self.owner.view_ofs + '0 0 10' + v_factor('-40 60 0');
		}

		self.movedir_z += random(10,12);
		if (self.movedir_z > 360) 
		{
			self.movedir_z -= 360;
		}

		NewSpot += (v_right * cos(self.count) * 15) + (v_up * sin(self.count) * 15) +
				   (v_forward * sin(self.movedir_z) * 15);
	
		NewSpot = self.origin + concatv(NewSpot - self.origin, CubeFollowRate);
	}

	setorigin(self,NewSpot);
}

void UseCubeOfForce(void)
{
	entity cube;

	if ((self.artifact_flags & AFL_CUBE_LEFT) &&
		(self.artifact_flags & AFL_CUBE_RIGHT))
	{  // Already got two running
		return;
	}

	cube = spawn();

	cube.owner = self;
	cube.solid = SOLID_SLIDEBOX;
	cube.movetype = MOVETYPE_NOCLIP;//MOVETYPE_FLY;
	cube.flags (+) FL_FLY | FL_NOTARGET;
	setorigin (cube, cube.owner.origin);
	setmodel (cube, "models/cube.mdl");
	setsize (cube, '-5 -5 -5', '5 5 5');		

	cube.classname = "cube_of_force";
	cube.health = 10;
	cube.dmg = -1;

	if (self.artifact_flags & AFL_CUBE_LEFT)
	{
		self.artifact_flags (+) AFL_CUBE_RIGHT;
		cube.artifact_flags (+) AFL_CUBE_RIGHT;
	}
	else
	{
		self.artifact_flags (+) AFL_CUBE_LEFT;
		cube.artifact_flags (+) AFL_CUBE_LEFT;
	}
	cube.think = CubeThinkerB;
	cube.th_die = CubeDie;

	thinktime cube : 0.01;
	cube.monster_duration = time + 45;
	cube.shot_cnt = 0;

	cube.movedir = '100 100 0';
	cube.count = random(360);
	self.movedir_z = random(360);

//	cube.drawflags (+) DRF_TRANSLUCENT;
	cube.drawflags (+) MLS_ABSLIGHT;

	cube.abslight = .1;

	self.cnt_cubeofforce -= 1;
}


/*
 * $Log: not supported by cvs2svn $
 * 
 * 21    9/11/97 12:02p Mgummelt
 * 
 * 20    9/02/97 10:11p Rlove
 * 
 * 19    8/26/97 8:30p Jweier
 * 
 * 18    8/26/97 8:11p Jweier
 * 
 * 17    8/20/97 7:08p Jweier
 * 
 * 16    8/14/97 11:22p Bgokey
 * 
 * 15    7/21/97 4:03p Mgummelt
 * 
 * 14    7/21/97 4:02p Mgummelt
 * 
 * 13    7/15/97 4:49p Rjohnson
 * Removed a debug statement
 * 
 * 12    7/15/97 4:47p Rjohnson
 * Updates
 * 
 * 11    6/26/97 9:08p Rjohnson
 * Update
 * 
 * 10    6/26/97 4:45p Rjohnson
 * Update
 * 
 * 9     6/18/97 6:21p Mgummelt
 * 
 * 8     6/18/97 4:00p Mgummelt
 * 
 * 7     5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 6     5/07/97 11:12a Rjohnson
 * Added a new field to walkmove and movestep to allow for setting the
 * traceline info
 * 
 * 5     2/12/97 4:38p Rjohnson
 * Looks good at this point
 * 
 * 4     2/10/97 4:28p Rjohnson
 * More movement updates
 * 
 * 3     2/04/97 3:26p Rjohnson
 * Will spawn it on the left or right, 2 at most
 * 
 * 2     2/04/97 10:46a Rjohnson
 * Added different type of movement
 */
