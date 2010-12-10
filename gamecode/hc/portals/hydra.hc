/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/hydra.hc,v 1.2 2007-02-07 16:59:33 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\hydra\final\hydra.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\hydra\final
$origin 0 0 0
$base base 450 200
$skin skin1
$flags 0

//
$frame hadie1    hadie2    hadie3    hadie4    hadie5    
$frame hadie6    hadie7    hadie8    hadie9    hadie10   
$frame hadie11   hadie12   hadie13   hadie14   hadie15   
$frame hadie16   hadie17   hadie18   hadie19   hadie20   
$frame hadie21   hadie22   hadie23   hadie24   hadie25   
$frame hadie26   hadie27   hadie28   hadie29   hadie30   
$frame hadie31   hadie32   hadie33   hadie34   hadie35   
$frame hadie36   

//
$frame hapan1    hapan2    hapan3    hapan4    hapan5    
$frame hapan6    hapan7    hapan8    hapan9    hapan10   

//
$frame hlft1     hlft2     hlft3     hlft4     hlft5     
$frame hlft6     hlft7     hlft8     hlft9     hlft10    
$frame hlft11    hlft12    hlft13    hlft14    hlft15    
$frame hlft16    hlft17    hlft18    hlft19    hlft20    
$frame hlft21    hlft22    hlft23    hlft24    hlft25    
$frame hlft26    hlft27    hlft28    hlft29    hlft30    

//
$frame hopen1    hopen2    hopen3    hopen4    hopen5    
$frame hopen6    hopen7    hopen8    

//
$frame hrit1     hrit2     hrit3     hrit4     hrit5     
$frame hrit6     hrit7     hrit8     hrit9     hrit10    
$frame hrit11    hrit12    hrit13    hrit14    hrit15    
$frame hrit16    hrit17    hrit18    hrit19    hrit20    
$frame hrit21    hrit22    hrit23    hrit24    hrit25    
$frame hrit26    hrit27    hrit28    hrit29    hrit30    

//
$frame hsdie1    hsdie2    hsdie3    hsdie4    hsdie5    
$frame hsdie6    hsdie7    hsdie8    hsdie9    hsdie10   
$frame hsdie11   hsdie12   hsdie13   hsdie14   hsdie15   
$frame hsdie16   hsdie17   hsdie18   hsdie19   hsdie20   
$frame hsdie21   hsdie22   hsdie23   hsdie24   hsdie25   
$frame hsdie26   hsdie27   hsdie28   hsdie29   hsdie30   
$frame hsdie31   hsdie32   hsdie33   hsdie34   hsdie35   
$frame hsdie36   

//
$frame hspan1    hspan2    hspan3    hspan4    hspan5    
$frame hspan6    hspan7    hspan8    hspan9    hspan10   

//
$frame hspit1    hspit2    hspit3    hspit4    hspit5    
$frame hspit6    hspit7    hspit8    hspit9    hspit10   
$frame hspit11   hspit12   

//
$frame hswim1    hswim2    hswim3    hswim4    hswim5    
$frame hswim6    hswim7    hswim8    hswim9    hswim10   
$frame hswim11   hswim12   hswim13   hswim14   hswim15   
$frame hswim16   hswim17   hswim18   hswim19   hswim20   

//
$frame htent1    htent2    htent3    htent4    htent5    
$frame htent6    htent7    htent8    htent9    htent10   
$frame htent11   htent12   htent13   htent14   htent15   
$frame htent16   htent17   htent18   htent19   htent20   
$frame htent21   htent22   htent23   htent24   



// Monster Stages
float HYDRA_STAGE_WAIT     = 0;
float HYDRA_STAGE_SWIM     = 1;
float HYDRA_STAGE_FLOAT    = 2;
float HYDRA_STAGE_STRAIGHT = 3;
float HYDRA_STAGE_REVERSE  = 4;
float HYDRA_STAGE_CHARGE   = 5;
float HYDRA_STAGE_ATTACK   = 10;

void hydra_attack(void);
void hydra_CloseFrames(void);
void hydra_move(float thrust);
void hydra_bob(void);

void hydra_init(void)
{  // Set the hydra ready for swim
	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	self.think = self.th_run;
	thinktime self : random(.1,.6);
	self.count = 0;
	self.monster_stage = HYDRA_STAGE_SWIM;
	self.hydra_FloatTo = 0;
	self.goalentity = self.enemy;

	self.monster_awake = TRUE;
}

void hydra_wait(void)
{
	if (LocateTarget())
	{  // We found a target
		hydra_init();
	}		
}

// Starting frames for making a large turn
//void hydra_BigLeft(void);
//void hydra_BigRight(void);

// Starting frames for making a short turn
//void hydra_ShortLeft(void);
//void hydra_ShortRight(void); 

void hydra_OpenFrames(void);
void hydra_TentFrames(void);
void hydra_SpitFrames(void);

float hydra_check_blind_melee();


void hydra_charge_finish()
{
	self.yaw_speed = 5;
	self.cnt = time + random(5,10);

	if (CheckMonsterAttack(MA_MELEE, 3.0)) 
	{
		self.monster_stage += HYDRA_STAGE_ATTACK;
		self.monster_check = 0;
		self.think = hydra_attack;
		thinktime self : 0.05;
		return;
	}

	self.monster_stage = HYDRA_STAGE_SWIM;
	self.think = self.th_run;
	thinktime self : 0.1;
}

void hydra_charge()
{
	float dist;
	
	self.yaw_speed = 8;

	check_pos_enemy();

	ai_face();

	dist = vlen(self.enemy.origin - self.origin);

	movetogoal(dist / 8);

	if (range(self.enemy) == RANGE_MELEE) 
	{
		self.monster_check = 0;
		hydra_charge_finish();
		return;
	}

	if (self.hydra_chargeTime < time)
	{
		self.monster_check = 0;
		hydra_charge_finish();
		return;
	}

	self.think = hydra_charge;
	thinktime self : 0.05;
}

float hydra_check_blind_melee(void)
{
	float dist, c1;//, c2;

	if (self.enemy.watertype != CONTENT_WATER) return 0;
	if (self.cnt > time) return 0;

	dist = vhlen(self.enemy.origin - self.origin);

	traceline(self.origin, self.enemy.origin, FALSE, self);

	c1 = fov(self, self.enemy, 80);

	if ((dist < 256) && c1 && trace_ent == self.enemy)
	{
		if (random() < 0.2)
			return 1;
	}

	return 0;
}

void hydra_blind(void)
{
	stuffcmd (self.enemy, "df\n");

	self.hydra_chargeTime = time + 1;
	self.think = hydra_charge;
	thinktime self : 0.05;
}

void hydra_checkForBlind(void)
{
	float r;

	r = pointcontents(self.enemy.origin);

	if (r != CONTENT_WATER) 
	{
		self.think = self.th_run;
		thinktime self : 0.1;
		return;
	}

	r = vhlen(self.enemy.origin - self.origin);

	if ((r < 256) && (fov(self, self.enemy, 80)) && (fov(self.enemy, self, 80)))
	{
		thinktime self : 0.1;
		self.think = hydra_blind;
	}
	else 
	{
		self.think = self.th_run;
		thinktime self : 0.1;
	}
}

void hydra_swim(float thrust) 
{
	float dist;
	float temp;

	if (self.velocity) self.velocity = self.velocity * 0.7;
	
	if (!self.enemy.flags2 & FL_ALIVE)
	{
		self.goalentity = self.enemy = world;
		self.monster_stage = HYDRA_STAGE_WAIT;
		return;
	}
	
	dist = (4.0 + thrust*6);  // Movement distance this turn
		
	movetogoal(dist);
	
	if (self.hydra_FloatTo == 0)
	{
		dist = self.enemy.origin_z - self.origin_z;
		if (dist < -50) self.hydra_FloatTo = dist - random(60);
		else if (dist > 50) self.hydra_FloatTo = dist + random(60);
	}

	if (self.hydra_FloatTo < -10)
	{
		temp = random(-3.5,-2.5);
//		movestep(0,0,temp, FALSE);
		self.hydra_FloatTo -= temp;
	}
	else if (self.hydra_FloatTo > 10)
	{
		temp = random(2.5,3.5);
//		movestep(0,0,temp, FALSE);
		self.hydra_FloatTo -= temp;
	}
	else self.hydra_FloatTo = 0;

	self.th_save = self.think;

	//checkenemy();

	enemy_range = range (self.enemy);
	if (hydra_check_blind_melee())
	{
		self.monster_check = 2;
		hydra_attack();
	}

	dist = vlen(self.enemy.origin - self.origin);
	if (dist > 350)
	{
		if (CheckMonsterAttack(MA_MISSILE,8.0))
			return;
	}	
	else
		CheckMonsterAttack(MA_MELEE,3.0);
}

void hydra_float(void)
{
	float Length;

	if (self.velocity) self.velocity = self.velocity * 0.7;

	Length = vhlen(self.origin - self.enemy.origin);

	if (Length < 300.0)
	{
		self.monster_stage = HYDRA_STAGE_SWIM;
		self.hydra_FloatTo = 0;
		return;
	}

	/*self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	ChangeYaw();*/
	ai_face();
	
	self.th_save = self.think;
	enemy_range = range (self.enemy);
	if (hydra_check_blind_melee())
	{
		self.monster_check = 2;
		hydra_attack();
	}
	CheckMonsterAttack(MA_MISSILE,8.0);
}

/*
void hydra_reverse(void)
{
	float retval;
	float dist;

	self.monster_duration -= 1;
	dist = 4.0;  // Movement distance this turn

	retval = walkmove(self.angles_y + 180, dist, FALSE);
	
	//self.monster_stage = HYDRA_STAGE_FLOAT;
	
}
*/
void hydra_move(float thrust) 
{
	check_pos_enemy();

	if (self.monster_stage == HYDRA_STAGE_SWIM)
	{
		hydra_swim(thrust);
		return;
	}
	else if (self.monster_stage == HYDRA_STAGE_FLOAT)
	{
		hydra_float();	
		return;
	}
	else if (self.monster_stage == HYDRA_STAGE_WAIT)
	{
		hydra_wait();
		return;
	}
	else if (self.monster_stage == HYDRA_STAGE_REVERSE)
	{
//		hydra_reverse();
		return;
	}
}

void hydra_attack(void)
{
	if (self.monster_check > 0)
	{
		hydra_SpitFrames();
	}
	else
	{
		hydra_TentFrames();
	}
}

void hydra_fire(void)
{
	sound (self, CHAN_WEAPON, "hydra/spit.wav", 1, ATTN_NORM);
	do_spit('0 0 0');
	do_spit('0 0 0');
	do_spit('0 0 0');
}

void hydra_tent(float TryHit) 
{
	ai_face();

	check_pos_enemy();

	if (TryHit)
	{
		makevectors(self.angles);
		traceline(self.origin,self.origin+v_forward*128,FALSE,self);

		if (trace_ent.takedamage)
		{
			sound (self, CHAN_WEAPON, "hydra/tent.wav", 1, ATTN_NORM);
			T_Damage (trace_ent, self, self, random(1,2));
		}
		else
			movetogoal(15);
	}
}

void hydra_open(void)
{
	ai_face();
}

void hydra_bob(void)
{
	local float	 rnd1, rnd2, rnd3;
//	local vector vtmp1, modi;

	rnd1 = self.velocity_x + random(-10,10);
	rnd2 = self.velocity_y + random(-10,10);
	rnd3 = self.velocity_z + random(10);

	if (rnd1 > 10)
		rnd1 = 10;
	if (rnd1 < -10)
		rnd1 = -10;
		
	if (rnd2 > 10)
		rnd2 = 10;
	if (rnd2 < -10)
		rnd2 = -10;
		
/*	if (rnd3 < 10)
		rnd3 = 15;*/
	if (rnd3 > 55)
		rnd3 = 50;
	
	self.velocity_x = rnd1;
	self.velocity_y = rnd2;
	self.velocity_z = rnd3;
}

// Swimming
float hydra_Swim[20] =
{
	-0.2,
	-0.1,
	0.0,
	0.1,  // Top going down
	0.2,
	0.3,
	0.4,
	0.5,  // middle
	0.4,
	0.3,
	0.2,
	0.1,  // bottom
	0.0,
	-0.1,
	-0.2,
	-0.3,
	-0.4,
	-0.5,  // middle
	-0.4,
	-0.3
};

// Top = 1, tr = 2, br = 3, tl = 4, bl = 5
float hydra_TentAttacks[24] =
{
	0,
	0,
	0,
	0,
	1,
	0,
	3,
	0,
	4,
	0,
	0,
	5,
	0,
	0,
	0,
	2,
	0,
	0,
	0,
	4,
	3,
	1,
	0,
	0
};



















// Attacking others
void hydra_AttackDieFrames(void)
{
	self.think = hydra_AttackDieFrames;
	thinktime self : HX_FRAME_TIME;

	if (self.frame != $hadie36)
		AdvanceFrame($hadie1,$hadie36);

	if (self.frame == $hadie35)
	{
		self.solid = SOLID_NOT;
		MakeSolidCorpse();
	}

	if (self.frame >= $hadie13)
	    hydra_bob();

	if(self.health<-30)
		chunk_death();
}

void hydra_AttackPainFrames(void) 
{
	self.think = hydra_AttackPainFrames;
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($hapan1,$hapan10);
}

// Swimming others
void hydra_SwimDieFrames(void)
{
	self.think = hydra_SwimDieFrames;
	thinktime self : HX_FRAME_TIME;

	if (self.frame != $hsdie36)
		AdvanceFrame($hsdie1,$hsdie36);

	if (self.frame == $hsdie35)
	{
		self.solid = SOLID_NOT;
		MakeSolidCorpse();
	}

	if (self.frame >= $hsdie8)
	    hydra_bob();

	if(self.health<-30)
		chunk_death();
}

void hydra_SwimPainFrames(void) 
{
	self.think = hydra_SwimPainFrames;
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($hspan1,$hspan10);
}

// Attacking
void hydra_OpenFrames(void) 
{
	self.think = hydra_OpenFrames;
	thinktime self : HX_FRAME_TIME;

	if (self.enemy.watertype != CONTENT_WATER) 
	{			
		self.monster_stage = HYDRA_STAGE_FLOAT;
		self.think = self.th_run;
		thinktime self : 0.1;

		return;
	}

	hydra_open();
	if (AdvanceFrame($hopen1,$hopen8) == AF_END)
	{
		hydra_attack();
	}
}

void hydra_CloseFrames(void) 
{
	self.think = hydra_CloseFrames;
	thinktime self : HX_FRAME_TIME;

	if (RewindFrame($hopen8,$hopen1) == AF_END)
	{
		self.monster_stage -= HYDRA_STAGE_ATTACK;
		self.think = self.th_save; 
	}
}

void hydra_SpitFrames(void) 
{
	vector vecA,vecB;

	self.think = hydra_SpitFrames;
	thinktime self : HX_FRAME_TIME;

	self.angles_y = vectoyaw(self.enemy.origin - self.origin);

	if (AdvanceFrame($hspit1,$hspit12) == AF_END)
	{
		self.think = hydra_CloseFrames;
		self.monster_check = -1;
	}
	else if (self.frame == $hspit7 && self.monster_check == 2)
	{
		sound (self, CHAN_WEAPON, "hydra/spit.wav", 1, ATTN_NORM);	
		vecA = self.enemy.origin - self.origin + self.enemy.proj_ofs;
		vecA = vectoangles(vecA);
		makevectors(vecA);
		v_forward_z = 0 - v_forward_z;

		vecA = v_factor('-40 400 -40');
		vecB = v_factor('40 500 40');

		particle2(self.origin+v_forward*40,vecA,vecB,256,12,400);
		self.think = hydra_checkForBlind;
		thinktime self : 0.1;
	}
	else if (self.frame == $hspit8 && self.monster_check == 1)
	{
		hydra_fire();
	}
}

void hydra_TentFrames(void) 
{
	float r;

	if (!self.enemy.flags2 & FL_ALIVE)
	{
		self.goalentity = self.enemy = world;
		self.monster_stage = HYDRA_STAGE_WAIT;
		self.think = self.th_stand;
		thinktime self : 0.1;
		return;
	}

	self.think = hydra_TentFrames;
	thinktime self : 0.025;

	if (AdvanceFrame($htent1,$htent24) == AF_END)
	{
		r = range(self.enemy);
		if ((r == RANGE_MELEE) && (random(0, 1) < 0.3))
		{
			self.frame = $htent1;
			hydra_tent(hydra_TentAttacks[self.frame - $htent1]);
		}
		else		
			self.think = hydra_CloseFrames;
	}
	else
	{
		hydra_tent(hydra_TentAttacks[self.frame - $htent1]);
	}
}

// Regular swimming / movement
void hydra_SwimFrames(void) 
{
	self.think = hydra_SwimFrames;
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($hswim1,$hswim20);
	if (self.frame == $hswim1)
		sound (self, CHAN_WEAPON, "hydra/swim.wav", 1, ATTN_NORM);

	hydra_move((hydra_Swim[self.frame - $hswim1]) + 0.3);
}

/*QUAKED misc_fountain (0 1 0.8) (0 0 0) (32 32 32) 
New item for QuakeEd

-------------------------FIELDS-------------------------
angles    0 0 0  the direction it should move the particles
movedir   1 1 1  the force it should move them
color     256    the color
cnt       2      the number of particles each time
--------------------------------------------------------

*/
void misc_fountain(void)
{
	starteffect(CE_FOUNTAIN, self.origin, self.angles,self.movedir,self.color,self.cnt);
}

void do_hydra_spit(void)
{
	self.monster_check = 1;
	self.monster_stage += HYDRA_STAGE_ATTACK;

	sound (self, CHAN_WEAPON, "hydra/open.wav", 1, ATTN_NORM);
	hydra_OpenFrames();
}

void do_hydra_tent(void)
{
	self.monster_check = 0;
	self.monster_stage += HYDRA_STAGE_ATTACK;

	sound (self, CHAN_WEAPON, "hydra/open.wav", 1, ATTN_NORM);
	hydra_OpenFrames();
}

void do_hydra_die(void)
{
	self.flags (+) FL_SWIM;

	sound (self, CHAN_WEAPON, "hydra/die.wav", 1, ATTN_NORM);

	if (self.monster_stage >= HYDRA_STAGE_ATTACK)
		hydra_AttackDieFrames();
	else
		hydra_SwimDieFrames();
}

/*
void hydra_retreat()
{
	self.monster_stage = HYDRA_STAGE_REVERSE;
	self.think = self.th_run;
	thinktime self : 0.1;
}
*/
	
void hydra_pain(entity attacker, float damage) 
{
	sound (self, CHAN_WEAPON, "hydra/pain.wav", 1, ATTN_NORM);
}

void init_hydra(void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	self.monster_stage = HYDRA_STAGE_WAIT;

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model ("models/hydra.mdl");
		precache_model ("models/spit.mdl");
	}

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_SWIM;
	self.thingtype=THINGTYPE_FLESH;
	self.classname="monster_hydra";
	self.mass = 7;

	setmodel (self, "models/hydra.mdl");
	self.skin = 0;

	setsize (self, '-30 -30 -24', '30 30 24');
	self.hull = HULL_SCORPION;
//self.hull = HULL_HYDRA;
	if(!self.health)
		self.health = 125;
	if(!self.max_health)
		self.max_health=self.health;
	self.experience_value = 50;
	self.mintel = 4;

	self.th_stand = hydra_SwimFrames;
	self.th_walk = hydra_SwimFrames;
	self.th_run = hydra_SwimFrames;
	self.th_pain = hydra_pain;
	self.th_die = do_hydra_die;
	self.th_missile = do_hydra_spit;
	self.th_melee = do_hydra_tent;

	self.takedamage = DAMAGE_YES;
	self.flags2 (+) FL_ALIVE;

	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 5;
	self.view_ofs = '0 0 0';
	self.use = monster_use;

	self.flags (+) FL_SWIM | FL_MONSTER;

	self.pausetime = 99999999;

	total_monsters += 1;

	self.init_exp_val = self.experience_value;

	thinktime self : random(0.5);
	self.think = self.th_stand;
}


/*QUAKED monster_hydra (1 0.3 0) (-40 -40 -42) (40 40 42) STAND HOVER JUMP x DORMANT
New item for QuakeEd

-------------------------FIELDS-------------------------
NOTE:  Normal QuakEd monster spawnflags don't apply here (no_jump, x, no_drop)
--------------------------------------------------------

*/
void monster_hydra(void)
{
	if(!self.th_init)
	{
		self.th_init=monster_hydra;
		self.init_org=self.origin;
	}
	init_hydra();

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_sound("hydra/pain.wav");
		precache_sound("hydra/die.wav");
		precache_sound("hydra/open.wav");
		precache_sound("hydra/turn-s.wav");
		precache_sound("hydra/turn-b.wav");
		precache_sound("hydra/swim.wav");
		precache_sound("hydra/tent.wav");
		precache_sound("hydra/spit.wav");
	}
}

