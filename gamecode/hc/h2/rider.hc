/*QUAKED rider_path (0.5 0.3 0) (-8 -8 -8) (8 8 8)
Specifies a path point for a rider.  You can specify
up to 4 other places that the rider can go after this
point.
-------------------------FIELDS-------------------------
path_id: number (id) of this path point
next_path_1: number (id) of a possible next point
next_path_2: number (id) of a possible next point
next_path_3: number (id) of a possible next point
next_path_4: number (id) of a possible next point
--------------------------------------------------------

*/
void rider_path(void)
{
}

void riderpath_init(void)
{
entity search,found;

	found = world;
	search = find(world, classname, "rider_path");
	while(search != world && found == world)
	{
		if (search.path_id == 1) 
			found = search;
		else
			search = find(search, classname, "rider_path");
	}

	if (found==world)
	{
		dprint("No starting point for rider\n");
		remove(self);
		return;
	}
	self.path_current=self.path_last = found;
}

void riderpath_findnext(void)
{
	entity search,found;
	float next,num_points,position;

	num_points = 0;
	if (self.path_current.next_path_1) 
		num_points += 1;
	if (self.path_current.next_path_2) 
		num_points += 1;
	if (self.path_current.next_path_3) 
		num_points += 1;
	if (self.path_current.next_path_4) 
		num_points += 1;

	if (!num_points)
	{
		dprintf("rider path %s has no next points\n",self.path_id);
		remove(self);
		return;
	}

	position = random(num_points);
//	dprintf("Num_points is %s   ",num_points);
//	dprintf("position is %s\n",position);

	num_points = next = 0;
	if (self.path_current.next_path_1) 
	{
		num_points += 1;
		if (position <= num_points && !next) 
			next = self.path_current.next_path_1;
	}
	if (self.path_current.next_path_2) 
	{
		num_points += 1;
		if (position <= num_points && !next) 
			next = self.path_current.next_path_2;
	}

	if (self.path_current.next_path_3) 
	{
		num_points += 1;
		if (position <= num_points && !next) 
			next = self.path_current.next_path_3;
	}

	if (self.path_current.next_path_4) 
	{
		num_points += 1;
		if (position <= num_points && !next) 
			next = self.path_current.next_path_4;
	}

	if (!next)
	{
		dprint("Internal error for rider path\n");
		dprintf("   Next is %s\n",next);
		dprintf("   Num_points is %s\n",num_points);
		dprintf("   position is %s\n",position);
		return;
	}

	found = world;
	search = find(world, classname, "rider_path");
	while(search != world && found == world)
	{
		if (search.path_id == next) 
			found = search;
		else
			search = find(search, classname, "rider_path");
	}

	if (!found)
	{
		dprintf("Could not find rider path %s\n",next);
		remove(self);
		return;
	}
	else
		self.path_current = found;

}

void rider_die(void);

void riderpath_move(float move_speed)
{
float distance, altitude, temp;
vector displace;
entity save_ent;

	if(self.velocity!='0 0 0')
		self.velocity='0 0 0';

	self.ideal_yaw = vectoyaw(self.path_current.origin - self.origin);
	self.rider_last_y_change = self.rider_y_change;
	self.rider_y_change = ChangeYaw();

//rj rider_die();

	if (self.movetype == MOVETYPE_FLY)
	{
		distance = vhlen(self.origin - self.path_current.origin);
		altitude = self.path_current.origin_z - self.origin_z;

		if (distance < 400)
		{
//			altitude *= 0.06;
			temp = (distance - self.rider_path_distance) / (400-self.rider_path_distance);
			temp = 1-temp;
			temp = temp / 6;

			if (altitude > 30)
			{
				self.angles_x = temp * 200;
				self.rider_move_adjustment -= 0.1;
			}
			else if (altitude < -30)
			{
				self.angles_x = 0 - (temp * 200);
				self.rider_move_adjustment += 0.15;
			}

			if (altitude > 60)
			{
				self.rider_move_adjustment -= 0.1;
			}
			else if (altitude < -60)
			{
				self.rider_move_adjustment += 0.15;
			}

			altitude *= temp;
		}
		else 
		{
			altitude = 0;
			self.angles_x -= self.angles_x / 10;
			self.rider_move_adjustment -= self.rider_move_adjustment / 15;
		}
//		dprintf("Move adjustment %s\n",self.rider_move_adjustment);

		self.origin_z += altitude;

//		altitude = self.path_current.origin_z - self.origin_z;
//		dprintf("Flying: distance %s",distance);
//		dprintf("  Alt is %s\n",altitude);
	}

	move_speed += self.rider_move_adjustment;

	if(!walkmove(self.angles_y, move_speed, TRUE))
	{
		save_ent=self.goalentity;
		self.goalentity=self.path_current;
		movetogoal(move_speed);
		self.goalentity=save_ent;
	}
	
	if (self.classname != "rider_famine")  // Famine doesn't do radius pain
	{
		if (trace_ent)//!=world&&trace_ent.classname!="ghost")
		{
			displace = normalize(trace_ent.origin - self.origin);
			if (infront(trace_ent))
			{
				trace_ent.velocity += displace*random(1000,1600);
				T_Damage (trace_ent, self, self, random(25,35));
				if(trace_ent.flags&FL_CLIENT)
				{
					trace_ent.punchangle_x = random(-9,-1);
					trace_ent.punchangle_y = random(-10,10);
					trace_ent.punchangle_z = random(-10,10);
				}
			}
			else
			{
				trace_ent.velocity += displace*random(700,900);
				T_Damage (trace_ent, self, self, random(15,20));
				if(trace_ent.flags&FL_CLIENT)
				{
					trace_ent.punchangle_x = random(-3,2);
					trace_ent.punchangle_y = random(-5,5);
					trace_ent.punchangle_z = random(-5,5);
				}
			}
		}
	}

	distance = vhlen(self.origin - self.path_current.origin);
	if (distance < self.rider_path_distance)
		riderpath_findnext();
}



void rider_multi_wait(void)
{
	if (self.max_health)
	{
		self.health = self.max_health;
		self.takedamage = DAMAGE_NO_GRENADE;
		self.solid = SOLID_BBOX;
	}
}

void rider_multi_trigger(void)
{
	if (self.nextthink > time)
	{
		return;		// already been triggered
	}
	if (self.enemy.classname != "rider_war")
	{
		return;
	}

	if (random() <= self.rt_chance)
	{
		if (self.noise) sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);

	// don't trigger again until reset
		self.takedamage = DAMAGE_NO;

		activator = self.enemy;

		SUB_UseTargets();
	}

	if (self.wait > 0)	
	{
		self.think = rider_multi_wait;
		thinktime self : self.wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		self.touch = SUB_Null;
		thinktime self : 0.1;
		self.think = SUB_Remove;
	}
}


void rider_multi_use(void)
{
	if (time < self.attack_finished)
		return;

	self.enemy = activator;
	rider_multi_trigger();
}

void rider_multi_touch(void)
{
	if (time < self.attack_finished)
		return;

// if the trigger has an angles field, check player's facing direction

	if (self.movedir != '0 0 0')
	{
		makevectors (other.angles);
		if (v_forward * self.movedir < 0)
			return;		// not facing the right way
	}

	self.enemy = other;

	rider_multi_trigger ();
}

/*QUAKED rider_trigger_multiple (0.5 0.15 0) ?
-------------------------FIELDS-------------------------
rt_chance: chance (0-1) that the trigger will be run
--------------------------------------------------------

*/
void rider_trigger_multiple(void)
{
	if (!self.wait)
		self.wait = 0.2;
	self.use = rider_multi_use;

	InitTrigger ();

	if (self.health)
	{
		self.max_health = self.health;
		self.th_die = multi_killed;
		self.takedamage = DAMAGE_NO_GRENADE;
		self.solid = SOLID_BBOX;
		setorigin (self, self.origin);	// make sure it links into the world
	}
	else
	{
		self.touch = rider_multi_touch;
	}
}

/*QUAKED rider_trigger_once (0.5 0.15 0) ?
-------------------------FIELDS-------------------------
rt_chance: chance (0-1) that the trigger will be run
--------------------------------------------------------

*/
void rider_trigger_once(void)
{
	self.wait = -1;
	rider_trigger_multiple();
}


void beam_move(void)
{
	thinktime self : HX_FRAME_TIME;

	if (self.scale < self.beam_max_scale)
		self.scale += 0.6;
	else
		self.scale = self.beam_max_scale;

	if (self.beam_direction)
	{
		self.beam_angle_a += self.beam_speed;
		if (self.beam_angle_a >= 360) self.beam_angle_a -= 360;
	}
	else
	{
		self.beam_angle_a -= self.beam_speed;
		if (self.beam_angle_a < 0) self.beam_angle_a += 360;
	}

	self.angles_x = cos(self.beam_angle_a) * self.beam_angle_b;
	self.angles_z = sin(self.beam_angle_a) * self.beam_angle_b;
}

void star_think(void)
{
	thinktime self : HX_FRAME_TIME;

	self.velocity = self.velocity * 1.05;

	if (self.scale < 2)
		self.scale *= 1.08;
}

void rider_star(void)
{
	entity star;
	float angle;

	if (random() < 0.5) 
		return;

	star = spawn();

	setmodel(star,"models/boss/star.mdl");
	setorigin(star,self.origin);

	setsize (star, '0 0 0', '0 0 0');		
	star.owner = self.owner;
	star.movetype = MOVETYPE_FLYMISSILE;
	star.solid = SOLID_BBOX;
	star.avelocity = randomv('40 40 40', '100 100 100');
	star.scale = 0.1;

	angle = random(360);
	star.velocity_x = cos(angle)*300;
	star.velocity_y = sin(angle)*300;

	star.classname = "rider_temp";
	star.think = star_think;
	star.touch = SUB_Remove;

	thinktime star : HX_FRAME_TIME;
}

void circle_think(void)
{
	thinktime self : HX_FRAME_TIME;

	rider_star();

	if (self.monster_stage == 0)
	{
		self.scale *= 1.0275;
		if (self.scale >= 2.5) 
		{
			self.monster_stage = 1;
			self.scale = 2.5;
		}
	}
	else if (self.monster_stage == 1)
	{
		self.scale -= random(.1);
		if (self.scale < 1.5 || random() < 0.1) 
			self.monster_stage = 2;
	}
	else if (self.monster_stage == 2)
	{
		self.scale += random(.1);
		if (self.scale >= 2.5)
		{
			self.monster_stage = 1;
			self.scale = 2.5;
		}
		else if (random() < 0.1) 
			self.monster_stage = 1;
	}
}

void rider_eol(void)
{
	entity search;

	if (self.count == 0)
	{
		self.count = 1;
		self.effects (-) EF_BRIGHTLIGHT;
		self.effects (+) EF_NODRAW;
		if(self.movechain!=world)
			self.movechain.effects (+) EF_NODRAW;
		
		thinktime self : 5;

		search = find(world, classname, "rider_temp");
		while (search != world)
		{
			remove(search);
			search = find(search, classname, "rider_temp");
		}

		return;
	}

//	return;

	nextmap = self.map;
	nextstartspot = self.target;
//rj nextmap = "rick1";

	intermission_running = 1;

	if (deathmatch)
		intermission_exittime = time + 5;
	else if (self.classname == "monster_eidolon")
		intermission_exittime = time + 99999;
	else
		intermission_exittime = time + 2;


	//Remove cross-level trigger server flags for next hub
	serverflags(-)(SFL_CROSS_TRIGGER_1|
				SFL_CROSS_TRIGGER_2|
				SFL_CROSS_TRIGGER_3|
				SFL_CROSS_TRIGGER_4|
				SFL_CROSS_TRIGGER_5|
				SFL_CROSS_TRIGGER_6|
				SFL_CROSS_TRIGGER_7|
				SFL_CROSS_TRIGGER_8);

	search=find(world,classname,"player");
	while(search)
	{//Take away all their goodies
		search.puzzle_inv1 = string_null;
		search.puzzle_inv2 = string_null;
		search.puzzle_inv3 = string_null;
		search.puzzle_inv4 = string_null;
		search.puzzle_inv5 = string_null;
		search.puzzle_inv6 = string_null;
		search.puzzle_inv7 = string_null;
		search.puzzle_inv8 = string_null;
		search=find(search,classname,"player");
	}

	WriteByte (MSG_ALL, SVC_INTERMISSION);
	if (!cvar("registered") && cvar("oem"))
	{
		WriteByte (MSG_ALL, 9);
		intermission_exittime = time + 99999;
	}
	else if (self.classname == "rider_famine")
		WriteByte (MSG_ALL, 1);
	else if (self.classname == "rider_death")
		WriteByte (MSG_ALL, 2);
	else if (self.classname == "rider_pestilence")
		WriteByte (MSG_ALL, 3);
	else if (self.classname == "rider_war")
		WriteByte (MSG_ALL, 4);
	else if (self.classname == "monster_eidolon")
		WriteByte (MSG_ALL, 6);
	else
		dprint("Invalid boss creature\n");

	FreezeAllEntities();
}

void rider_die(void)
{
entity beam;
entity save;
entity found;
vector new_origin;

	if (self.think != rider_die)
	{
		SUB_UseTargets();

		self.think = rider_die;
		self.count = 0;
		thinktime self : HX_FRAME_TIME;
		self.rider_death_speed = 0.2;
//		self.effects = EF_NODRAW;

		return;
	}

	if (self.count == 0)
	{
		sound (self, CHAN_AUTO, "famine/flashdie.wav", 1, ATTN_NONE);  // Start of the death flash
		found=find(world,classname,"player");
		while(found)
		{//Give them all the exp
			AwardExperience(found,self,self.experience_value);
			found=find(found,classname,"player");
		}
		self.experience_value=FALSE;
		self.drawflags (+) MLS_ABSLIGHT;
		self.abslight = 3;
		if(self.noise)
			sound(self,CHAN_VOICE,self.noise,1,ATTN_NONE);
		self.movetype=MOVETYPE_NONE;
		self.velocity='0 0 0';
	}

	thinktime self : self.rider_death_speed;
	self.rider_death_speed += 0.1;

	if (self.count >= 10) 
	{
		if (self.count == 3)
		{
			beam = spawn();

			new_origin = self.origin + '0 0 50';

			setmodel(beam,"models/boss/circle.mdl");
			setorigin(beam,new_origin);

			setsize (beam, '0 0 0', '0 0 0');		
			beam.owner = self;
			beam.movetype = MOVETYPE_FLYMISSILE;
			beam.solid = SOLID_NOT;
			beam.drawflags = SCALE_TYPE_UNIFORM;
			beam.scale = .1;
			beam.skin = 0;
			beam.avelocity = '0 0 300';
			beam.think = circle_think;
			thinktime beam : HX_FRAME_TIME;
			self.count = 13;
		}

		self.count = 0;
		self.think = rider_eol;
		thinktime self : .5;

		return;
	}
	else
	{
		self.effects = EF_BRIGHTLIGHT;
//		self.effects = EF_NODRAW;

		if (self.count == 3)
		{
			beam = spawn();

			new_origin = self.origin + '0 0 50';

			setmodel(beam,"models/boss/circle.mdl");
			setorigin(beam,new_origin);

			setsize (beam, '0 0 0', '0 0 0');		
			beam.owner = self;
			beam.movetype = MOVETYPE_FLYMISSILE;
			beam.solid = SOLID_NOT;
			beam.drawflags = SCALE_TYPE_UNIFORM;
			beam.scale = .1;
			beam.skin = 0;
			beam.avelocity = '0 0 300';
			beam.classname = "rider_temp";
			beam.think = circle_think;
			thinktime beam : HX_FRAME_TIME;
		}
		else if (self.count == 0)
		{
			starteffect(18, self.origin + '0 0 40');
		}

	}

	self.count += 1;

	beam = spawn();

	makevectors(self.angles);
//	new_origin = v_factorrange('-3 -25 45', '3 25 50') + self.origin;
	new_origin = self.origin + '0 0 50';

	setmodel(beam,"models/boss/shaft.mdl");
	setorigin(beam,new_origin);

	setsize (beam, '0 0 0', '0 0 0');		
	beam.owner = self;
	beam.drawflags = SCALE_ORIGIN_BOTTOM | SCALE_TYPE_XYONLY;
	beam.movetype = MOVETYPE_FLYMISSILE;
	beam.solid = SOLID_NOT;
	beam.think = beam_move;
	beam.angles = '0 0 0';
	beam.angles_x = random(-50,50);
	beam.angles_z = random(-50,50);
	beam.beam_angle_a = random(360);
	beam.beam_angle_b = random(20,130);
	beam.scale = .1;
	beam.beam_max_scale = random(.5,1.5);
	beam.classname = "rider_temp";
	if (random() > 0.5) 
		beam.beam_direction = 1;

	beam.beam_speed = random(2,4.5);

	save = self;
	self = beam;
	beam_move();
	self = save;
}

void rider_use(void)
{
	thinktime self : 0.2; // wait for path points to spawn
}

void rider_init(void)
{
    precache_model3 ("models/boss/shaft.mdl");
    precache_model3 ("models/boss/circle.mdl");
    precache_model3 ("models/boss/star.mdl");
	precache_sound3 ("famine/flashdie.wav");

	total_monsters += 1;

	self.takedamage = DAMAGE_YES;
	self.flags(+)FL_MONSTER;
	self.flags2(+)FL_ALIVE;
	self.thingtype=THINGTYPE_FLESH;

	if(self.classname!="monster_eidolon")
		self.monsterclass=CLASS_BOSS;

	self.th_die = rider_die;
	self.use = rider_use;

	if (!(self.spawnflags & 1))
		thinktime self : 0.2; // wait for path points to spawn
}
