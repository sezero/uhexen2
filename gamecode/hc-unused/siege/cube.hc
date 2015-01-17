/*
 * $Header: /HexenWorld/Siege/cube.hc 4     5/25/98 1:38p Mgummelt $
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

