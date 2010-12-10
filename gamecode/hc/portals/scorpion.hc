
//**************************************************************************
//**
//** scorpion.hc
//** bgokey
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/portals/scorpion.hc,v 1.1.1.1 2004-11-29 11:35:22 sezero Exp $
//**
//**************************************************************************

// FRAMES ------------------------------------------------------------------

// Attack 1 (claw and tail)
$frame ScAttA1 ScAttA2 ScAttA3 ScAttA4 ScAttA5
$frame ScAttA6 ScAttA7 ScAttA8 ScAttA9 ScAttA10
$frame ScAttA11 ScAttA12 ScAttA13 ScAttA14 ScAttA15
$frame ScAttA16 ScAttA17 ScAttA18 ScAttA19 ScAttA20
$frame ScAttA21 ScAttA22 ScAttA23 ScAttA24 ScAttA25

// Attack 2 (claw only)
$frame ScAttB1 ScAttB2 ScAttB3 ScAttB4 ScAttB5
$frame ScAttB6 ScAttB7 ScAttB8 ScAttB9 ScAttB10
$frame ScAttB11 ScAttB12 ScAttB13 ScAttB14 ScAttB15
$frame ScAttB16 ScAttB17 ScAttB18 ScAttB19 ScAttB20
$frame ScAttB21 ScAttB22 ScAttB23 ScAttB24 ScAttB25
$frame ScAttB26 ScAttB27

// Attack 3 (tail only)
$frame ScAttC1 ScAttC2 ScAttC3 ScAttC4 ScAttC5
$frame ScAttC6 ScAttC7 ScAttC8 ScAttC9 ScAttC10
$frame ScAttC11 ScAttC12 ScAttC13 ScAttC14 ScAttC15
$frame ScAttC16 ScAttC17 ScAttC18 ScAttC19 ScAttC20
$frame ScAttC21 ScAttC22

// Death
$frame SCDead1 SCDead2 SCDead3 SCDead4 SCDead5
$frame SCDead6 SCDead7 SCDead8 SCDead9 SCDead10
$frame SCDead11 SCDead12 SCDead13 SCDead14 SCDead15
$frame SCDead16 SCDead17 SCDead18 SCDead19 SCDead20
$frame SCDead21

// Pain
$frame ScPain1 ScPain2 ScPain3 ScPain4 ScPain5
$frame ScPain6 ScPain7 ScPain8 ScPain9 ScPain10

// Transition from standing to attack pose
$frame ScRedy1 ScRedy2 ScRedy3 ScRedy4 ScRedy5
$frame ScRedy6 ScRedy7 ScRedy8 ScRedy9 ScRedy10
$frame ScRedy11

// Transition from standing to walk
$frame ScStep1 ScStep2 ScStep3 ScStep4

// Transition from walk to standing
$frame ScStop1 ScStop2 ScStop3 ScStop4 ScStop5
$frame ScStop6 ScStop7 ScStop8 ScStop9

// Wake from resting (ScWake1 = resting)
$frame ScWake1 ScWake2 ScWake3 ScWake4 ScWake5
$frame ScWake6 ScWake7 ScWake8 ScWake9 ScWake10
$frame ScWake11 ScWake12 ScWake13 ScWake14 ScWake15
$frame ScWake16 ScWake17 ScWake18 ScWake19 ScWake20
$frame ScWake21 ScWake22 ScWake23 ScWake24 ScWake25
$frame ScWake26 ScWake27 ScWake28 ScWake29 ScWake30

// Walk
$frame ScWalk1 ScWalk2 ScWalk3 ScWalk4 ScWalk5
$frame ScWalk6 ScWalk7 ScWalk8 ScWalk9 ScWalk10
$frame ScWalk11 ScWalk12 ScWalk13 ScWalk14 ScWalk15
$frame ScWalk16

// CONSTANTS ---------------------------------------------------------------

float SCORPION_YELLOW = 0;
float SCORPION_BLACK = 1;

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void ScorpionInit(float type);
void ScorpionStand(void);
void ScorpionWake(void);
void ScorpionWalk(void);
void ScorpionRun(void);
void ScorpionRunBlack(void);
void ScorpionPainDecide(void);
void ScorpionPain(void);
void ScorpionMeleeDecide(void);
void ScorpionMelee1(void);
void ScorpionMelee2(void);
void ScorpionMelee3(void);
void ScorpionMelee4(void);
//void ScorpionDie(void);
void ScorpionDieInit(void);
entity ScorpionLookProjectiles(void);
float ScorpionCheckDefense(void);
void ScorpionStrafeDefense(void);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

float ScorpionStandFrames[6] =
{
	$scwake1, $scwake2, $scwake3, $scwake4, $scwake3, $scwake2
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// monster_scorpion_yellow
//
//==========================================================================
/*QUAKED monster_scorpion_yellow (1 0.3 0) (-10 -10 0) (10 10 64) AMBUSH
Yellow scorpion.
------- key / value ----------------------------
health = 100
experience_value = 60
------- spawnflags -----------------------------
AMBUSH
*/

void monster_scorpion_yellow(void)
{
	if(!self.th_init)
	{
		self.th_init=monster_scorpion_yellow;
		self.init_org=self.origin;
	}
	ScorpionInit(SCORPION_YELLOW);
}

//==========================================================================
//
// monster_scorpion_black
//
//==========================================================================
/*QUAKED monster_scorpion_black (1 0.3 0) (-10 -10 0) (10 10 64) AMBUSH
Black scorpion.
------- key / value ----------------------------
health = 200
experience_value = 150
------- spawnflags -----------------------------
AMBUSH
*/

void monster_scorpion_black(void)
{
	if(!self.th_init)
	{
		self.th_init=monster_scorpion_black;
		self.init_org=self.origin;
	}
	ScorpionInit(SCORPION_BLACK);
}

//==========================================================================
//
// ScorpionInit
//
//==========================================================================

void ScorpionInit(float type)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model2("models/scorpion.mdl");

		precache_sound2("scorpion/awaken.wav");
		precache_sound2("scorpion/walk.wav");
		precache_sound2("scorpion/clawsnap.wav");
		precache_sound2("scorpion/tailwhip.wav");
		precache_sound2("scorpion/pain.wav");
		precache_sound2("scorpion/death.wav");
	}
	setmodel(self, "models/scorpion.mdl");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.thingtype = THINGTYPE_FLESH;

	if (type == SCORPION_YELLOW)
		self.yaw_speed = 8;
	else 
		self.yaw_speed = 10;

	self.mass = 9;
	self.mintel = 4;

	//self.touch = SUB_Null;
	//self.use = SUB_Null;
	setsize(self, '-16 -16 0', '16 16 64');
	self.hull=HULL_PLAYER;

	if(type == SCORPION_YELLOW)
	{
		if(!self.health)
			self.health = 100;
		self.experience_value = 60;
	}
	else
	{
		if(!self.health)
			self.health = 200;
		self.experience_value = 150;
	}
	if(!self.max_health)
		self.max_health=self.health;

	self.takedamage = DAMAGE_YES;

	self.th_stand = ScorpionStand;
	self.th_walk = ScorpionWalk;

	if(type == SCORPION_BLACK)
		self.th_run = ScorpionRunBlack;
	else 
		self.th_run = ScorpionRun;

	self.th_melee = ScorpionMeleeDecide;
	self.th_pain = ScorpionPainDecide;
	self.th_die = ScorpionDieInit;

	self.view_ofs = '0 0 12';

	self.scorpionType = type;
	if(type == SCORPION_BLACK)
	{
		self.skin = 1;
	}
	self.init_exp_val = self.experience_value;
	walkmonster_start();
}

//==========================================================================
//
// ScorpionStand
//
//==========================================================================

void ScorpionStand(void)
{
	self.think = ScorpionStand;
	self.nextthink = time + 0.2;
	self.scorpionRest += 2;
	if(self.scorpionRest < 0 || self.scorpionRest > 5)
	{
		self.scorpionRest = 0;
	}
	self.frame = ScorpionStandFrames[self.scorpionRest];
	ai_stand();
	if(self.think != ScorpionStand)
	{ // Wake up
		self.th_save = self.think;
		self.think = ScorpionWake;
		sound(self, CHAN_VOICE, "scorpion/awaken.wav", 1, ATTN_NORM);
	}
}

//==========================================================================
//
// ScorpionWake
//
//==========================================================================

void ScorpionWake(void) [++ $scwake1..$scwake30]
{
	if(cycle_wrapped)
	{
		self.cnt = time;
		self.think = self.th_save;
		self.think();
	}
}

//==========================================================================
//
// ScorpionWalk
//
//==========================================================================

void ScorpionWalk(void) [++ $scwalk1..$scwalk16]
{
	if(((self.scorpionWalkCount += 1)&3) == 0)
	{
		sound(self, CHAN_BODY, "scorpion/walk.wav", random(0.9, 1), ATTN_NORM);
	}
	ai_walk(2);
	if(random()<0.1)
		pitch_roll_for_slope('0 0 0',self);
}

//==========================================================================
//
// ScorpionRunBlack
//
//==========================================================================

void ScorpionRunBlack(void) [++ $scwalk1..$scwalk16]
{
	float enemy_dist;

	if(((self.scorpionWalkCount += 1)&3) == 0)
	{
		sound(self, CHAN_BODY, "scorpion/walk.wav", random(0.9, 1), ATTN_NORM);
	}
	
	if ((self.enemy.last_attack > time - 1) && (fov(self, self.enemy, 45)))
	{
		if (ScorpionCheckDefense()) 
		{
			ScorpionStrafeDefense();
			return;
		}
	}

	if (self.attack_state == AS_SLIDING)
		if ((random() < 0.8) && (self.cnt <= time))
		{
			self.cnt = time + random(0.5, 1.0);
			self.attack_state = AS_STRAIGHT;
			ScorpionMelee4();
			return;
		}

	enemy_dist = vlen(self.enemy.origin - self.origin);
	
	if (enemy_dist < 120)
	{
		if ((random() < 0.33) && (infront(self.enemy)) && (self.cnt <= time))
		{
			self.attack_state = AS_SLIDING;
			
			self.cnt = time + random(0.5, 1.0);
			self.lefty = random(0,1);
			if (self.lefty < 0.5) self.lefty = 0;
			else self.lefty = 1;

			ai_run(10);
			return;
		}
		else if (!infront(self.enemy))
			self.attack_state = AS_STRAIGHT;
	}
	else self.attack_state = AS_STRAIGHT;

	ai_run(8);
	if(random()<0.1)
		pitch_roll_for_slope('0 0 0',self);
}

//==========================================================================
//
// ScorpionRun
//
//==========================================================================

void ScorpionRun(void) [++ $scwalk1..$scwalk16]
{
	float enemy_dist;

	if(((self.scorpionWalkCount += 1)&3) == 0)
	{
		sound(self, CHAN_BODY, "scorpion/walk.wav", random(0.9, 1), ATTN_NORM);
	}
	
	
	if ((self.enemy.last_attack > time - 1) && (fov(self, self.enemy, 45)))
	{
		if (ScorpionCheckDefense()) 
		{
			ScorpionStrafeDefense();
			return;
		}
	}

	if (self.attack_state == AS_SLIDING)
		if ((random() < 0.5) && (self.cnt <= time))
		{
			self.cnt = time + random(0.5, 1.0);
			self.attack_state = AS_STRAIGHT;
			ScorpionMelee4();
			return;
		}

	enemy_dist = vlen(self.enemy.origin - self.origin);
	
	if (enemy_dist < 120)
	{
		if ((random() < 0.33) && (infront(self.enemy)) && (self.cnt <= time))
		{
			self.attack_state = AS_SLIDING;
			
			self.cnt = time + random(0.5, 1.0);
			self.lefty = random(0,1);
			if (self.lefty < 0.5) self.lefty = 0;
			else self.lefty = 1;

			ai_run(10);
			return;
		}
		else if (!infront(self.enemy))
			self.attack_state = AS_STRAIGHT;
	}
	else self.attack_state = AS_STRAIGHT;

	ai_run(6);
	if(random()<0.1)
		pitch_roll_for_slope('0 0 0',self);
}
//==========================================================================
//
// ScorpionPainDecide
//
//==========================================================================

void ScorpionPainDecide(void)
{
	if(random() < 0.3)
	{
		ScorpionPain();
		sound(self, CHAN_VOICE, "scorpion/pain.wav", 1, ATTN_NORM);
	}
}

//==========================================================================
//
// ScorpionPain
//
//==========================================================================

void ScorpionPain(void) [++ $scpain1..$scpain10]
{
	if(cycle_wrapped)
	{
		self.th_run();
	}
}

//==========================================================================
//
// ScorpionMelee
//
//==========================================================================
void ScorpionMelee(float damage)
{
	vector source;

	makevectors (self.angles);
	source = self.origin;
	traceline (source, source + v_forward*60, FALSE, self);

	if (trace_ent != self.enemy) return;

	damage += random(0.1, 1);
	T_Damage (self.enemy, self, self, damage);
}

//==========================================================================
//
// ScorpionMeleeDecide
//
//==========================================================================

void ScorpionMeleeDecide(void)
{
	float r;

	r = random();
	self.last_attack=time;
	if (self.classname == "monster_scorpion_black")
	{
		if(r < 0.2)
		{
			ScorpionMelee1();
		}
		else if(r < 0.4)
		{
			ScorpionMelee2();
		}
		else if (r < 0.6)
		{
			ScorpionMelee3();
		}
		else 
			ScorpionMelee4();
	}
	else
	{
		if(r < 0.3)
		{
			ScorpionMelee1();
		}
		else if(r < 0.6)
		{
			ScorpionMelee2();
		}
		else if (r < 0.9)
		{
			ScorpionMelee3();
		}
		else 
			ScorpionMelee4();
	}		
}

//==========================================================================
//
// ScorpionMelee1
//
//==========================================================================

void ScorpionMelee1(void) [++ $scatta1..$scatta25]
{
	if(self.frame == $scatta4 || self.frame == $scatta9)
	{
		sound(self, CHAN_VOICE, "scorpion/clawsnap.wav", 1, ATTN_NORM);
		ScorpionMelee(1);
	}
	else if(self.frame == $scatta14)
	{
		sound(self, CHAN_BODY, "scorpion/tailwhip.wav", 1, ATTN_NORM);
		ScorpionMelee(1);
	}

	if(self.frame > $scatta16 && self.frame < $scatta20)
	{
		ai_charge(4);
		if (self.classname == "monster_scorpion_yellow")
			ScorpionMelee(2);
		else
			ScorpionMelee(3);
	}
	else
	{
		ai_charge(2);
	}
	
	if(cycle_wrapped)
		self.think = self.th_run;
}

//==========================================================================
//
// ScorpionMelee2
//
//==========================================================================

void ScorpionMelee2(void) [++ $scattb1..$scattb27]
{
	if(self.frame == $scattb4 || self.frame == $scattb8
		|| self.frame == $scattb13)
	{
		sound(self, CHAN_VOICE, "scorpion/clawsnap.wav", 1, ATTN_NORM);
		ScorpionMelee(1);
	}

	if(self.frame > $scattb16 && self.frame < $scattb20)
	{
		ai_charge(4);
		if (self.classname == "monster_scorpion_yellow")
			ScorpionMelee(2);
		else
			ScorpionMelee(3);
	}
	else
	{
		ai_charge(2);
	}
	if(cycle_wrapped)
		self.think = self.th_run;
}

//==========================================================================
//
// ScorpionMelee3
//
//==========================================================================

void ScorpionMelee3(void) [++ $scattc1..$scattc22]
{
	if(self.frame == $scattc9)
	{
		sound(self, CHAN_BODY, "scorpion/tailwhip.wav", 1, ATTN_NORM);
		ScorpionMelee(1);
	}

	if(self.frame > $scattc16 && self.frame < $scattc20)
	{
		ai_charge(4);
		if (self.classname == "monster_scorpion_yellow")
			ScorpionMelee(2);
		else
			ScorpionMelee(3);
	}
	else
	{
		ai_charge(2);
	}
	if(cycle_wrapped)
		self.think = self.th_run;
}

//==========================================================================
//
// ScorpionMelee4
//
//==========================================================================

void ScorpionMelee4(void) [++ $scatta1..$scatta25]
{
	if (self.frame == $scatta4 || self.frame == $scatta9)
	{
		sound(self, CHAN_VOICE, "scorpion/clawsnap.wav", 1, ATTN_NORM);
		ScorpionMelee(1);
	}
	else if(self.frame == $scatta14)
	{
		sound(self, CHAN_BODY, "scorpion/tailwhip.wav", 1, ATTN_NORM);
		if (self.classname == "monster_scorpion_yellow")
			ScorpionMelee(2);
		else
			ScorpionMelee(3);
	}

	if(self.frame > $scatta16 && self.frame < $scatta20)
	{
		ai_charge(16);
		if (self.classname == "monster_scorpion_yellow")
			ScorpionMelee(2);
		else
			ScorpionMelee(4);
	}
	else
	{
		ai_charge(12);
	}
	if(cycle_wrapped)
		self.think = self.th_run;
}

//==========================================================================
//
// ScorpionStrafeDefense
//
//==========================================================================

void ScorpionStrafeDefense(void) [++ $scwalk1..$scwalk8]
{
	float ofs;

	if (cycle_wrapped)
	{
		thinktime self : 0.1;
		self.think =  self.th_run;
		return;
	}
	
	makevectors(self.v_angle);
	
	if (self.lefty == -1)
		ofs = 90;
	else 
		ofs = -90;

	if (walkmove (self.ideal_yaw + ofs, 10, FALSE))
		return;
	
	walkmove (self.ideal_yaw - ofs, 10, FALSE);
	if (self.lefty == -1)
		self.lefty = 1;
	else self.lefty = -1;
}
	

//==========================================================================
//
// ScorpionCheckDefense
//
//==========================================================================

float ScorpionCheckDefense()
{
	entity enemy_proj;
	float r;
	
	enemy_proj=ScorpionLookProjectiles();

	if (!enemy_proj) return 0;

	r=range(enemy_proj);
	
	self.lefty=check_heading_left_or_right(enemy_proj);

	if(r==RANGE_NEAR)
	{
		if (random() < 0.2) 
			return 1;
	}
	return 0;
}

//==========================================================================
//
// ScorpionLookProjectiles
//
//==========================================================================
entity ScorpionLookProjectiles ()
{
	entity found, enemy_proj;

	found=findradius(self.origin,1000);
	while(found)
	{
		if(found.movetype==MOVETYPE_FLYMISSILE||found.movetype==MOVETYPE_BOUNCE||found.movetype==MOVETYPE_BOUNCEMISSILE)
		if(visible(found))
		{	
			if(heading(self,found,0.9))
				enemy_proj=found;
		}
		found=found.chain;
	}
	if(enemy_proj)
		return enemy_proj;
	else
		return world;
}

//==========================================================================
//
// ScorpionDie
//
//==========================================================================


void ScorpionDie(void)
{
	if(self.frame == $scdead21)
	{
		MakeSolidCorpse();
		return;
	}


	if(self.health < -30)
	{
		chunk_death();
	}

	self.frame += 1;
	thinktime self : HX_FRAME_TIME;
}

void ScorpionDieInit(void) [$scdead1 ScorpionDie]
{
	sound(self, CHAN_VOICE, "scorpion/death.wav", 1, ATTN_NORM);
}
