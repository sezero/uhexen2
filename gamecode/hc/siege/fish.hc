/*
 * siege/fish.hc
 */

/*
==============================================================================

Q:\art\models\monsters\FISH\FISH1.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\FISH
$origin 0 0 0
$base BASE SKIN
$skin SKIN
$skin SKIN2
$flags 0

//
$frame SWIM01       SWIM02       SWIM03       SWIM04       SWIM05       
$frame SWIM06       SWIM07       SWIM08       SWIM09       SWIM10       
$frame SWIM11       SWIM12       SWIM13       SWIM14       SWIM15       
$frame SWIM16       SWIM17       SWIM18       SWIM19       SWIM20       
$frame SWIM21       SWIM22       SWIM23       SWIM24       SWIM25       
$frame SWIM26       SWIM27       SWIM28       SWIM29       SWIM30       
$frame SWIM31       SWIM32       SWIM33       SWIM34       SWIM35       
$frame SWIM36       SWIM37       SWIM38       SWIM39       SWIM40       




// Frame Code
float FISH_STAGE_MOVE = 1;
float FISH_STAGE_FOLLOW = 2;
float FISH_STAGE_BORED = 3;

void fish_hover(void);
void fish_move(void);


float fish_friends(void)
{
	entity item,test;
	float bad;

	item = findradius(self.origin, 100);
	while (item)
	{
		if (item.classname == "monster_fish" && item != self)
		{
			test = item.goalentity;
			bad = FALSE;
			while(test != world && bad != TRUE)
			{
				if (test == self) bad = TRUE;

				test = test.goalentity;
			}
			if (!bad)
			{
				self.goalentity = item;
				self.goalentity.fish_leader_count += 1;
				return TRUE;
			}
		}
		item = item.chain;
	}

	return FALSE;
}

void fish_follow(void)
{
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	if (self.goalentity == world)	/* experienced by Rugxulo */
	{
		self.think = fish_hover;
		self.monster_stage = FISH_STAGE_MOVE;
		return;
	}

	if (random() > 0.1)
		return;

	if (random() < .05)
	{
		self.monster_duration = random(250,450);
		self.monster_stage = FISH_STAGE_BORED;
		self.think = fish_hover;
		self.goalentity.fish_leader_count -= 1;
		self.goalentity = world;
		//dprint("Fish got bored\n");
		//self.drawflags (-) MLS_ABSLIGHT;
		return;	/* a return was missing here -- O.S. */
	}

	self.movedir = self.monster_last_seen - self.origin + randomv('-20 -20 -25', '20 20 25');
	if (self.goalentity.goalentity)
	{
		self.goalentity.fish_leader_count -= 1;
		self.goalentity = self.goalentity.goalentity;
		self.goalentity.fish_leader_count += 1;
	}

	self.monster_last_seen = self.goalentity.origin;

	self.count = 80 + random(20);
	self.movedir_x /= self.count;
	self.movedir_y /= self.count;
	self.movedir_z /= self.count;
	self.fish_speed = vhlen(self.movedir);

	self.think = fish_move;
}

void fish_move(void)
{
	float retval;

	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	self.ideal_yaw = vectoyaw(self.movedir);
	ChangeYaw();
	retval = walkmove(self.angles_y, self.fish_speed, FALSE);
	retval = movestep(0, 0, self.movedir_z, FALSE);
	/*
	if (retval != 2)
	{
		self.goalentity = world;
		self.monster_stage = FISH_STAGE_MOVE;
	}
	*/
	if (self.count >= 170)
		self.fish_speed *= 1.05;
	else if (self.count < 30)
		self.fish_speed *= .9;

	self.count -= 1;
	if (self.count < 1)
	{
		self.count = 0;
		if (self.monster_stage == FISH_STAGE_MOVE)
		{
			if (fish_friends())
			{
				self.monster_stage = FISH_STAGE_FOLLOW;
				self.think = fish_follow;
				self.monster_last_seen = self.goalentity.origin;
				//self.drawflags (+) MLS_ABSLIGHT;
				//self.abslight = 0;
				//self.goalentity.drawflags (+) MLS_ABSLIGHT;
				//self.goalentity.abslight = 2.5;
				//dprint("Following!\n");
			}
			else
				self.think = fish_hover;
		}
		else if (self.monster_stage == FISH_STAGE_FOLLOW)
			self.think = fish_follow;
		else if (self.monster_stage == FISH_STAGE_BORED)
			self.think = fish_hover;
	}
}

void fish_hover(void)
{
	float try;

	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	if (self.monster_stage == FISH_STAGE_BORED)
	{
		self.monster_duration -= 1;
		if (self.monster_duration <= 0)
			self.monster_stage = FISH_STAGE_MOVE;
	}

	if (random() < 0.02)
	{
		try = 0;
		while (try < 10)
		{
			self.movedir = randomv('-100 -100 -30', '100 100 30');
			tracearea(self.origin, self.origin + self.movedir, self.mins, self.maxs, FALSE, self);
			if (trace_fraction == 1)
			{
				self.think = fish_move;

				self.count = 170 + random(30);
				self.movedir_x /= 400;
				self.movedir_y /= 400;
				self.movedir_z /= 400;
				self.fish_speed = vhlen(self.movedir);
				try = 999;
			}
			try += 1;
		}
	}
}

void fish_die(void)
{
	remove(self);
}

/*QUAKED monster_fish (1 0 0) (-16 -16 -8) (16 16 8) 
Ambient Fish

-------------------------FIELDS-------------------------
skin:  0 = bright colored, 1 = darker colored
--------------------------------------------------------

*/
void monster_fish(void)
{
	precache_model2 ("models/fish.mdl");

	self.takedamage = DAMAGE_YES;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_FLY;
	self.flags (+) FL_SWIM | FL_MONSTER;
	self.yaw_speed = 2;
	self.hull = HULL_PLAYER;
	self.monster_stage = FISH_STAGE_MOVE;
	self.mass = 99999; // Big fishies!

//	self.drawflags (+) MLS_POWERMODE;

	setmodel (self, "models/fish.mdl");
	self.skin = 0;

	setsize (self, '-10 -10 -8', '10 10 8');
	self.health = 1;

	self.th_die = fish_die;
	self.think = fish_hover;
	thinktime self : HX_FRAME_TIME;
}

//SIEGE PIRHANA
void() swimmonster_start_go =
{
//	if(!self.touch)
//		self.touch=obj_push;

	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 10;

	if(self.view_ofs=='0 0 0');
		self.view_ofs = '0 0 2';
	if(self.proj_ofs=='0 0 0');
		self.proj_ofs = '0 0 2';

	self.use = monster_use;

	self.flags(+)FL_MONSTER;

	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
		if (!self.pathentity)
		{
			dprint ("Monster can't find target at ");
			dprint (vtos(self.origin));
			dprint ("\n");
		}
	// this used to be an objerror
		self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
		self.th_walk ();
	}
	else
	{
		self.pausetime = 99999999;
		self.th_stand ();
	}

// spread think times so they don't all happen at same time
	self.nextthink+=random(0.5);
};

void() swimmonster_start =
{
// spread think times so they don't all happen at same time
	self.takedamage=DAMAGE_YES;
	self.nextthink+=random(0.5);
	self.think = swimmonster_start_go;
	total_monsters = total_monsters + 1;
	self.monster_awake=TRUE;
};

void()fish_melee;
void pirhana_flop_back ()
{
	if(pointcontents(self.origin)==CONTENT_WATER)
	{
		self.avelocity='0 0 0';
		self.movetype=MOVETYPE_SWIM;
		self.angles_x=self.angles_z=0;
		self.think = self.th_stand;
		thinktime self : 0;
		return;
	}
	if(self.cnt>3)
	{
		self.cnt=0;
		setorigin(self,self.init_org);
		self.think=pirhana_flop_back;
		thinktime self: 0.5;
	}
	else
	{
		self.cnt+=1;
		self.velocity=normalize(self.wallspot - self.origin);
		self.velocity=self.velocity*400;
		self.velocity_z=270;
		self.avelocity=self.velocity*random(-3,3);
		self.flags(-)FL_ONGROUND;
		self.think=pirhana_flop_back;
		thinktime self: 2;
	}
}

void pirhana_throw ()
{
	self.wallspot=self.origin;
	self.velocity=normalize(self.enemy.origin+(random(self.enemy.maxs_z)*'0 0 1')-self.origin);
	self.angles=vectoangles(self.velocity);
	self.velocity=self.velocity*400+' 0 0 200';
	self.movetype=MOVETYPE_BOUNCE;
	self.flags(-)FL_ONGROUND;
	self.touch=fish_melee;
	self.think=pirhana_flop_back;
	thinktime self: 2;
}

void pirhana_check_enemy ()
{
	if(pointcontents(self.enemy.origin)==CONTENT_WATER)
		self.search_time=time+5;

	if(self.enemy.health<=0||self.enemy.rings & RING_WATER||self.search_time<time)
		self.enemy=world;
	else
	{
		if(self.search_time<=time+4.5)
		{
			if(self.last_time<time)
			{
				if(random()<0.5)
				{
					if(vlen(self.enemy.origin-self.origin)<200)
					{
						if(lineofsight(self,self.enemy)||random()<0.3)
						{
							self.last_time=time+10;
							self.think=pirhana_throw;
							thinktime self : 0;
							pirhana_throw();
						}
					}
				}
			}
		}
	}
}

void pirhana_hunt ()
{
//Ideas...
//When find one tell all other pirhana so they don't have to do
//this search?  Better yet, have only one thinker that does all the
//monster finding/checking for all the fish?
//Also allow some fish to be interrupted or ignore someone so they
//can go after multiple targets?  Choose target by range?
entity found;
	if(self.enemy.health>0)
		return;

	found=nextent(world);
	while(found!=world)
	{
		if(pointcontents(found.origin)==CONTENT_WATER&&
			found.takedamage&&
			found.classname!=self.classname&&
			found.thingtype==THINGTYPE_FLESH&&
			(!found.rings & RING_WATER))
		{
			self.monster_awake=TRUE;
			self.enemy=self.goalentity=found;
			found=world;
			self.think=self.th_run;
			thinktime self : 0;
		}
		else
			found=nextent(found);
	}
}

void fish_run () //[++0 .. 39 ]
{
	self.think=fish_run;
	thinktime self: 0.05;
	if(self.velocity_x||self.velocity_y||self.velocity_z)
		self.velocity=self.velocity*0.75;

	pirhana_check_enemy();
	if(!self.enemy)
	{
		if(self.pathentity)
			self.think=self.th_walk;
		else
			self.think=self.th_stand;
		thinktime self : 0;
		return;
	}
	ai_run(self.speed*0.75);
	if(random()<0.05)
		pirhana_hunt();
}

void fish_walk () //[++0 .. 39 ]
{
	self.think=fish_walk;
	thinktime self: 0.05;
	if(self.velocity_x||self.velocity_y||self.velocity_z)
		self.velocity=self.velocity*0.75;

	pirhana_check_enemy();
	ai_walk(self.speed/3);
	if(random()<0.05)
		pirhana_hunt();
}

void fish_stand () //[++0 .. 39 ]
{
	self.think=fish_stand;
	thinktime self: 0.05;
	if(self.velocity_x||self.velocity_y||self.velocity_z)
		self.velocity=self.velocity*0.75;

	pirhana_check_enemy();

	ai_stand();
	if(random()<0.05)
		pirhana_hunt();
}

void fish_melee ()
{
vector	delta,hitspot;
float	ldmg;

	if(self.attack_finished>time)
		return;

	if (!self.enemy)
		return;		// removed before bite
		
	hitspot = self.enemy.origin+(random(self.enemy.maxs_z))*'0 0 1';
	delta = hitspot- self.origin;

	if (vlen(delta) > 60)
		return;
		
	if(random()<0.3)
		sound (self.enemy, CHAN_ITEM, "spider/bite.wav", 1, ATTN_NORM);
	ldmg = (random() + random()) * 3;
	if(self.enemy.health<=ldmg)
		ldmg=100;
	T_Damage (self.enemy, self, self, ldmg);
	MeatChunks (hitspot,randomv('-200 -200 0','200 200 400'), 3,self.enemy);
	SpawnPuff (hitspot, '0 0 0', 3,self.enemy);

	if(self.movetype==MOVETYPE_BOUNCE)
		self.attack_finished=time + 1;
}

void fish_attack()// [++0 .. 39 ]
{
	self.think=fish_attack;
	thinktime self: 0.05;
	if(self.velocity_x||self.velocity_y||self.velocity_z)
		self.velocity=self.velocity*0.75;

	pirhana_check_enemy();
	/*
	if(pointcontents(self.enemy.origin)!=CONTENT_WATER)
	{
		if(vlen(self.enemy.origin-self.origin)<128)
			if(random()<0.05)
			{
				self.flags(+)FL_FLY;
				self.flags(-)FL_SWIM;
			}
		if(self.flags&FL_SWIM)
			self.enemy=world;
	}
	*/
	if(!self.enemy)
	{
		if(self.pathentity)
			self.think=self.th_walk;
		else
			self.think=self.th_stand;
		thinktime self : 0;
		return;
	}
	ai_charge(self.speed);
	if(random()<0.3)
		fish_melee();
}

/*QUAKED monster_pirhana (1 0 0) (-16 -16 0) (16 16 28) 
*/
void() monster_pirhana =
{
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;

	precache_model ("models/fish.mdl");
	precache_sound ("spider/bite.wav");

	setmodel (self,"models/fish.mdl");

	setsize (self, '0 0 0', '0 0 0');
//	setsize (self, '-4 -4 -4', '4 4 4');
	self.hull=HULL_POINT;//use pentacles
	self.health=10000000;
	self.init_org=self.origin;
	/*
	if(skill==3)
		self.health = 800;
	else
		self.health = 400;
	*/
	self.mass = 0.1;
	self.speed=14+random(6);

	self.thingtype=THINGTYPE_FLESH;
	self.th_stand = fish_stand;
	self.th_walk = fish_walk;
	self.th_run = fish_run;
	self.th_die = chunk_death;
	self.th_melee = fish_attack;
//	self.flags2(+)FL_ALIVE;
	self.flags(+)FL_SWIM;

	swimmonster_start ();
};

