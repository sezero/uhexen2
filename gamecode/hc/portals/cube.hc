/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/cube.hc,v 1.1.1.1 2004-11-29 11:31:29 sezero Exp $
 */


float cube_distance = 500;

void CubeDie(void)
{
	stopSound(self,0);
	self.owner.artifact_flags(-)self.artifact_flags;
	remove(self);
}

float cube_find_target(void)
{
	entity item;
	float pass;

	while(pass<2)
	{//on pass 2, accept corpses
		item = findradius(self.origin, cube_distance);
		while (item)
		{
			if (((item.flags & FL_MONSTER) || (item.classname == "player" && deathmatch == 1&&item!=self.controller)||(pass==1&&(item.classname=="player_sheep"||item.netname=="corpse"||item.netname=="head"))) &&	item.health > 0)
			{
				if(item.controller!=self.controller)
				{
					traceline (self.origin,(item.absmin+item.absmax)*0.5,TRUE,self);
					if (trace_fraction==1.0)
					{
						if ((!item.effects & EF_NODRAW)||item.classname=="monster_pentacles")
						{
							sound(self, CHAN_ITEM, "misc/Beep1.wav", 1, ATTN_NORM);
							self.attack_finished=time+random(0.5);
							self.drawflags(+)MLS_POWERMODE;
							self.last_attack=0;
							self.enemy = item;
							return TRUE;
						}
					}
				}
			}
			item = item.chain;
		}
		pass+=1;
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
	vector targ_org;
//	vector targ_size_min,targ_size_max;
	float Distance,beam_color;
	entity temp;

	if (time > self.monster_duration || self.owner.health <= 0)
	{
		CubeDie();
		return;
	}

	if (self.enemy)
	{
		if (self.enemy.health <= 0)
		{
			self.enemy = world;
			self.drawflags(-)MLS_POWERMODE;
		}
	}

	if (!self.enemy)
		cube_find_target();

	if (self.enemy)
	{
		Distance = vlen(self.origin - self.enemy.origin);
		if (Distance > cube_distance*2)
		{
			self.enemy = world;
			self.drawflags(-)MLS_POWERMODE;
		}
		else if (Distance < cube_distance)
		{
			// Got to do this otherwise tracearea sees right through you
			temp = self.owner;
			self.owner = self;

			/*
			targ_size_min = self.enemy.maxs - self.enemy.mins;
			targ_size_max =targ_size_min;
			targ_size_min *=-0.5;
			targ_org = (self.enemy.absmin+self.enemy.absmax)*0.5 + randomv(targ_size_min,targ_size_max);
			*/
			if(self.enemy.proj_ofs!='0 0 0')
				targ_org=self.enemy.origin+self.enemy.proj_ofs;
			else
				targ_org=(self.enemy.absmin+self.enemy.absmax)*0.5;
			traceline (self.origin,targ_org,FALSE,self);
			if(trace_ent!=self.enemy)
			{//First try missed
				targ_org=(self.enemy.absmin+self.enemy.absmax)*0.5;
				traceline (self.origin,targ_org,FALSE,self);
			}
			if (trace_ent == self.enemy)
			{
				self.shot_cnt+=1;
				self.adjust_velocity = CubeDirection[random(0,5)];
				self.effects(+)EF_MUZZLEFLASH;
				if(self.last_attack+1.5<time)
					sound(self, CHAN_WEAPON, "golem/gbfire.wav", 1, ATTN_NORM);
				else
					sound(self, CHAN_BODY, "crusader/sunhum.wav", 1, ATTN_NORM);
				updateSoundPos(self,CHAN_BODY);
				updateSoundPos(self,CHAN_WEAPON);
				beam_color=rint(random(0,4));
				self.last_attack=time;
				WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
				WriteByte (MSG_BROADCAST, TE_STREAM_COLORBEAM);	//beam type
				WriteEntity (MSG_BROADCAST, self);				//owner
				WriteByte (MSG_BROADCAST, 0);					//tag + flags
				WriteByte (MSG_BROADCAST, 1);					//time
				WriteByte (MSG_BROADCAST, beam_color);			//color

				WriteCoord (MSG_BROADCAST, self.origin_x);
				WriteCoord (MSG_BROADCAST, self.origin_y);
				WriteCoord (MSG_BROADCAST, self.origin_z);

				WriteCoord (MSG_BROADCAST, targ_org_x);
				WriteCoord (MSG_BROADCAST, targ_org_y);
				WriteCoord (MSG_BROADCAST, targ_org_z);

				LightningDamage (self.origin, targ_org, self, beam_color+1,"sunbeam");
			}
			else
			{
				traceline (self.origin,(self.enemy.absmin+self.enemy.absmax)*0.5,TRUE,self);
				if(trace_fraction!=1.0)
				{
					self.cnt+=1;
					if(self.cnt>=5)
					{//can't see enemy for last 10 tries, find someone else
						self.enemy=world;
						self.cnt=0;
						self.drawflags(-)MLS_POWERMODE;
					}
				}
			}

			self.owner = temp;
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

	if(random()<0.1)
		sound(self, CHAN_VOICE, "misc/cubehum.wav", 1, ATTN_NORM);
	updateSoundPos(self,CHAN_VOICE);
	if (!self.owner.flags2 & FL_ALIVE) 
	{
		CubeDie();
		return;
	}

	if(self.attack_finished<time)
	{
		if(random()<0.5)
			cube_fire();
		if(self.shot_cnt>17)
		{
			self.shot_cnt=0;
			self.attack_finished=time+random(0.5,2);
		}
	}

	if (self.adjust_velocity == '0 0 0')
	{
		if (self.adjust_velocity == '0 0 0')
		{
			if (random() < 0.02)
			{
				self.adjust_velocity = CubeDirection[random(0,5)];
			}
		}
	}
	cube_rotate();

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
		self.drawflags(-)MLS_POWERMODE;
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

void UseCubeOfForce(float on_spawn)
{
	entity cube;

	if ((self.artifact_flags & AFL_CUBE_LEFT) &&
		(self.artifact_flags & AFL_CUBE_RIGHT))
	{  // Already got two running
		return;
	}

	cube = spawn();

	cube.owner = self;
	cube.controller = self;
	cube.solid = SOLID_SLIDEBOX;
	cube.movetype = MOVETYPE_NOCLIP;//MOVETYPE_FLY;
	cube.flags (+) FL_FLY | FL_NOTARGET;
	setorigin (cube, cube.owner.origin);
	setmodel (cube, "models/cube.mdl");
	setsize (cube, '-5 -5 -5', '5 5 5');		

	sound(self, CHAN_VOICE, "misc/cubehum.wav", 1, ATTN_LOOP);
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
	if(on_spawn)
		cube.monster_duration = time + 10;
	else
		cube.monster_duration = time + 45;

	cube.movedir = '100 100 0';
	cube.count = random(360);
	self.movedir_z = random(360);

	self.cnt_cubeofforce -= 1;
}


/*
 * $Log: not supported by cvs2svn $
 * 
 * 7     3/19/98 12:17a Mgummelt
 * last bug fixes
 * 
 * 6     3/17/98 4:06p Mgummelt
 * 
 * 5     3/16/98 6:38a Mgummelt
 * 
 * 4     3/16/98 2:19a Mgummelt
 * 
 * 3     3/14/98 11:09p Mgummelt
 * 
 * 2     3/14/98 9:24p Mgummelt
 * 
 * 23    10/28/97 1:00p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
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
