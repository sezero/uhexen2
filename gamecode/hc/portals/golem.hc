
//**************************************************************************
//**
//** golem.hc
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/portals/golem.hc,v 1.1.1.1 2004-11-29 11:32:18 sezero Exp $
//**
//**************************************************************************

// FRAMES ------------------------------------------------------------------
// Common: Rest
$frame rest1 rest2 rest3 rest4 rest5 rest6 rest7 rest8 rest9 rest10
$frame rest11 rest12 rest13 rest14 rest15 rest16 rest17 rest18 rest19
$frame rest20 rest21 rest22

// Common: Transition from run to rest
$frame transa1 transa2 transa3 transa4 transa5 transa6 transa7 transa8
$frame transa9 transa10 transa11 transa12 transa13

// Common: Transition from rest to run
$frame transb1 transb2 transb3 transb4 transb5 transb6 transb7 transb8
$frame transb9 transb10 transb11 transb12 transb13

// Common: Transition from still to run
$frame wake1 wake2 wake3 wake4 wake5 wake6 wake7 wake8 wake9 wake10
$frame wake11 wake12 wake13 wake14 wake15 wake16

// Common: Walk
$frame walk1 walk2 walk3 walk4 walk5 walk6 walk7 walk8 walk9 walk10
$frame walk11 walk12 walk13 walk14 walk15 walk16 walk17 walk18 walk19
$frame walk20 walk21 walk22 walk23 walk24 walk25 walk26 walk27 walk28
$frame walk29 walk30 walk31 walk32 walk33 walk34

// Common: Run
$frame run1 run2 run3 run4 run5 run6 run7 run8 run9 run10 run11 run12
$frame run13 run14 run15 run16 run17 run18 run19 run20 run21 run22
$frame run23 run24

// Common: Right hand punch attack
$frame rpunch1 rpunch2 rpunch3 rpunch4 rpunch5 rpunch6 rpunch7 rpunch8
$frame rpunch9 rpunch10 rpunch11 rpunch12 rpunch13 rpunch14 rpunch15
$frame rpunch16 rpunch17 rpunch18 rpunch19 rpunch20 rpunch21
$frame rpunch22 rpunch23 rpunch24

// Common: Right hand pound attack
$frame rpound1 rpound2 rpound3 rpound4 rpound5 rpound6 rpound7 rpound8
$frame rpound9 rpound10 rpound11 rpound12 rpound13 rpound14 rpound15
$frame rpound16 rpound17 rpound18 rpound19 rpound20 rpound21
$frame rpound22 rpound23 rpound24

// Common: Death
$frame death1 death2 death3 death4 death5 death6 death7 death8 death9
$frame death10 death11 death12 death13 death14 death15 death16 death17
$frame death18 death19 death20 
$frame death21 death22

$framesave x

// Stone: Charge at the player
$frame rush1 rush2 rush3 rush4 rush5 rush6 rush7 rush8 rush9 rush10
$frame rush11 rush12 rush13 rush14 rush15 rush16 rush17 rush18 rush19
$frame rush20 rush21 rush22 rush23 rush24

$framerestore x

// Iron: Gem attack
$frame igem1 igem2 igem3 igem4 igem5 igem6 igem7 igem8 igem9 igem10
$frame igem11 igem12 igem13 igem14 igem15 igem16 igem17 igem18 igem19
$frame igem20 igem21 igem22 igem23 igem24

$framerestore x

// Bronze: Gem attack
$frame bgem1 bgem2 bgem3 bgem4 bgem5 bgem6 bgem7 bgem8 bgem9 bgem10
$frame bgem11 bgem12 bgem13 bgem14 bgem15 bgem16 bgem17 bgem18 bgem19
$frame bgem20 bgem21 bgem22 bgem23 bgem24

// Bronze: Stomp attack
$frame stomp1 stomp2 stomp3 stomp4 stomp5 stomp6 stomp7 stomp8 stomp9
$frame stomp10 stomp11 stomp12 stomp13 stomp14 stomp15 stomp16 stomp17
$frame stomp18 stomp19 stomp20 stomp21 stomp22 stomp23 stomp24

// CONSTANTS ---------------------------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void GolemInit(void);
void GolemCUse(void);
void GolemStand(void);
void GolemWalk(void);
void GolemRun(void);
void GolemSMeleeDecide(void);
void GolemIMeleeDecide(void);
void GolemBMeleeDecide(void);
void GolemPunchRight(void);
void GolemPoundRight(void);
void GolemSRushBegin(void);
void GolemSRushSlide(void);
void GolemSRushEnd(void);
float GolemFlinch(float firstFrame, float lastFrame);
void GolemSPain(void);
void GolemIPain(void);
void GolemBPain(void);
void GolemDie(void);
void GolemBBeamBegin(void);
void GolemBStomp(void);
void GolemIMissile(void);
float GolemBCheckBeamAttack();
float GolemICheckMissileAttack();

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// monster_golem_stone
//
//==========================================================================
/*QUAKED monster_golem_stone (1 0.3 0) (-32 -32 0) (32 32 88) AMBUSH STATUE JUMP PLAY_DEAD
Stone Golem.
------- key / value ----------------------------
health = 200
experience_value = 125
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_stone(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.th_init)
	{
		self.th_init=monster_golem_stone;
		self.init_org=self.origin;
	}
	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model4("models/golem_s.mdl");//converted for MP
		
		precache_model3("models/goarm.mdl");
		precache_model3("models/golegs.mdl");
		precache_model3("models/g-head.mdl");

		precache_sound3("golem/stnfall.wav");
		precache_sound3("golem/stnpain.wav");
		precache_sound3("golem/slide.wav");
		precache_sound3("imp/swoophit.wav");
		precache_sound3("golem/dthgroan.wav");
	}

	self.thingtype = THINGTYPE_GREYSTONE;
	setmodel(self, "models/golem_s.mdl");
	setsize(self, '-24 -24 0', '24 24 80');
	GolemInit();
	self.hull = HULL_PLAYER;
	if(!self.health)
		self.health = 200;
	if(!self.max_health)
		self.max_health=self.health;
	if(!self.experience_value)
		self.experience_value = 125;
	self.mintel = 4;
	self.th_melee = GolemSMeleeDecide;
	self.th_pain = GolemSPain;
	self.view_ofs = self.proj_ofs='0 0 64';
	self.init_exp_val = self.experience_value;
	walkmonster_start();
}

//==========================================================================
//
// monster_golem_iron
//
//==========================================================================
/*QUAKED monster_golem_iron (1 0.3 0) (-55 -55 0) (55 55 120) AMBUSH STATUE
Iron Golem.
------- key / value ----------------------------
health = 400
experience_value = 200
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_iron(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.th_init)
	{
		self.th_init=monster_golem_iron;
		self.init_org=self.origin;
	}

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model4("models/golem_i.mdl");//converted for MP

		precache_model2("models/goarm.mdl");
		precache_model2("models/golegs.mdl");
		precache_model2("models/g-head.mdl");

		precache_model2("models/golemmis.mdl");
		precache_sound2("golem/mtlfall.wav");
		precache_sound2("golem/mtlpain.wav");
		precache_sound2("golem/gbfire.wav");
		precache_sound2("golem/dthgroan.wav");
	}
	self.thingtype = THINGTYPE_METAL;
	setmodel(self, "models/golem_i.mdl");
	setsize(self, '-32 -32 0', '32 32 80');
	GolemInit();
	if(!self.health)
		self.health = 450;
	if(!self.max_health)
		self.max_health=self.health;
	self.mintel = 6;
	if(!self.experience_value)
		self.experience_value = 200;
	self.th_melee = GolemIMeleeDecide;
	self.th_pain = GolemIPain;
	self.view_ofs = self.proj_ofs='0 0 64';
	self.init_exp_val = self.experience_value;
	walkmonster_start();
}

//==========================================================================
//
// monster_golem_bronze
//
//==========================================================================
/*QUAKED monster_golem_bronze (1 0.3 0) (-64 -64 0) (64 64 194) AMBUSH STATUE
Bronze Golem.
------- key / value ----------------------------
health = 500
experience_value = 275
------- spawnflags -----------------------------
AMBUSH
*/
void monster_golem_bronze(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.th_init)
	{
		self.th_init=monster_golem_bronze;
		self.init_org=self.origin;
	}
	self.cnt = 0;
	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model4("models/golem_b.mdl");//converted for MP

		precache_model2("models/goarm.mdl");
		precache_model2("models/golegs.mdl");
		precache_model2("models/g-head.mdl");

		precache_sound2("golem/mtlfall.wav");
		precache_sound2("golem/mtlpain.wav");
		precache_sound2("golem/stomp.wav");
		precache_sound2("golem/gbcharge.wav");
		precache_sound2("golem/gbfire.wav");
		precache_sound2("golem/dthgroan.wav");
	}
	self.thingtype = THINGTYPE_METAL;
	setmodel(self, "models/golem_b.mdl");
	setsize(self, '-60 -60 0', '60 60 190');
	GolemInit();
	if(!self.health)
		self.health = 650;
	if(!self.max_health)
		self.max_health=self.health;
	self.mintel = 8;
	if(!self.experience_value)
		self.experience_value = 275;
	self.th_melee = GolemBMeleeDecide;
	self.th_pain = GolemBPain;
	self.view_ofs = self.proj_ofs='0 0 115';
	self.init_exp_val = self.experience_value;
	walkmonster_start();
}

//==========================================================================
//
// monster_golem_crystal
//
//==========================================================================
/*QUAKED monster_golem_crystal (1 0.3 0) (-32 -32 -24) (32 32 64) AMBUSH
Crystal Golem.
------- key / value ----------------------------
health = 400
experience_value = 650
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_crystal(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	precache_model3("models/golem_s.mdl");
	precache_sound3("golem/stnpain.wav");
	precache_sound3("golem/slide.wav");
	precache_sound3("golem/dthgroan.wav");
	self.thingtype = THINGTYPE_ICE;
	setmodel(self, "models/golem_s.mdl");
	setsize(self, '-24 -24 0', '24 24 80');
	GolemInit();
	self.hull = HULL_PLAYER;
	self.drawflags = DRF_TRANSLUCENT|MLS_ABSLIGHT;
	self.abslight = 1.4;
	self.skin = GLOBAL_SKIN_ICE;
	self.health = 400;
	if(!self.max_health)
		self.max_health=self.health;
	self.experience_value = 650;
	self.th_melee = GolemSMeleeDecide;
	self.th_pain = GolemSPain;
	self.use = GolemCUse;
	
	self.view_ofs = self.proj_ofs='0 0 64';
	
	self.init_exp_val = self.experience_value;
	walkmonster_start();
	
	self.takedamage = DAMAGE_NO;
}

//==========================================================================
//
// GolemInit
//
//==========================================================================

void GolemInit(void)
{
	self.netname="golem";
	self.flags (+) FL_MONSTER;
	self.flags2 (+) FL_ALIVE;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.yaw_speed = 14;
	self.mass = 75;
	self.mintel = 2;
	self.hull = HULL_GOLEM;
	self.th_stand = GolemStand;
	self.th_walk = GolemWalk;
	self.th_run = GolemRun;
	self.th_die = GolemDie;
	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_sound3("golem/awaken.wav");
		precache_sound3("golem/step.wav");
		precache_sound3("golem/swing.wav");
	}
}

//==========================================================================
//
// GolemCUse
//
//==========================================================================

void GolemCUse(void)
{
	self.takedamage = DAMAGE_YES;
	self.drawflags = DRF_TRANSLUCENT|MLS_CRYSTALGOLEM;
}

//==========================================================================
//
// GolemStand
//
//==========================================================================

void golem_awaken ()
{
	sound(self,CHAN_AUTO,"golem/awaken.wav",1,ATTN_NORM);
	self.takedamage=DAMAGE_YES;
	if(activator.classname=="player")
	{
		self.enemy = activator;
		thinktime self : 0;
		self.think = FoundTarget;
	}
	else
		self.think=self.th_stand;
}

void GolemStand(void) [++ $rest1..$rest22]
{
	if(self.spawnflags&2)
	{
		self.takedamage=DAMAGE_NO;
		self.use=self.think=golem_awaken;
		self.nextthink=-1;
	}
	else
	{
		ai_stand();
		thinktime self : 0.2;
	}
}

//==========================================================================
//
// GolemWalk
//
//==========================================================================

void GolemWalk(void) [++ $walk1..$walk34]
{
	if(self.frame == $walk16 || self.frame == $walk33)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	ai_walk(2);
}

//==========================================================================
//
// GolemRun
//
//==========================================================================

void GolemRun(void) [++ $run1..$run24]
{
	float len;
	float hdiff;
	float dist, r;

	check_pos_enemy();
	checkenemy();

	if (coop && !visible(self.enemy))
		LocateTarget();

	if (self.classname == "monster_golem_stone" || self.classname == "monster_golem_crystal") 
	{	
		len = vlen(self.origin - self.enemy.origin);
		hdiff = fabs(self.origin_z - self.enemy.origin_z);
		if(len > 50 && len < 300 && hdiff < 80)
		{
			if(random() < 0.05)
			{
				GolemSRushBegin();
					return;
			}
		}
		ai_run(8);
	}
	
	if (self.classname == "monster_golem_bronze")
	{
		dist = vlen(self.enemy.origin - self.origin);
		r = random(0, 10);

		if (dist < 100)
			GolemBMeleeDecide();
		else if (dist > 256 && visible(self.enemy) && r < 0.6)
		{
			if (GolemBCheckBeamAttack() == 1)
				GolemBBeamBegin();
		}
		else if (dist > 100 && dist < 256 && r < 0.3)
			GolemBStomp();

		ai_run(8);
	}

	if (self.classname == "monster_golem_iron")
	{
		dist = vlen(self.enemy.origin - self.origin);
		r = random(0, 10);

		if (dist < 100)
			GolemBMeleeDecide();
		else if (dist > 100 && visible(self.enemy) && r < 0.4)
			if (GolemICheckMissileAttack())
				GolemIMissile();

		ai_run(5);
	}


	if(self.frame == $run12 || self.frame == $run24)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
}

void GolemDoMelee(float critical)
{
vector	delta;
float 	ldmg;

	if (!self.enemy)
		return;		// removed before stroke
		
	delta = self.enemy.origin - self.origin;

	if (self.classname == "monster_golem_bronze")
	{
		if (vlen(delta) > 128)
			return;
		ldmg = random(15);
	}
	else if (self.classname == "monster_golem_iron")
	{
		if (vlen(delta) > 128)
			return;
		ldmg = random(12);
	}
	else 
	{
		ldmg = random(8);
		if (vlen(delta) > 128) return;
	}

	if (critical) ldmg = ldmg * 1.5;

	if (self.attack_finished < time) 
	{
		sound(self, CHAN_BODY, "imp/swoophit.wav", 1, ATTN_NORM);
		if(skill>=4)
			self.attack_finished=0;
		else
			self.attack_finished = time + 1;
	}
	
	if (self.enemy.health - ldmg <= 0 && critical&&self.enemy.flags2&FL_ALIVE)
		self.enemy.decap = 2;

	T_Damage (self.enemy, self, self, ldmg);
}

//==========================================================================
//
// GolemSMeleeDecide
//
//==========================================================================

void GolemSMeleeDecide(void)
{
	if(random() < 0.5)
	{
		GolemPunchRight();
	}
	else
	{
		GolemPoundRight();
	}
}

void GolemIMissileTouch(void)
{
	if (other.health > 0 && other.flags & FL_ALIVE)
		T_Damage(other, self, self.owner, random(13,17));

	remove(self);
}

void GolemIMissileThink(void)
{
	if (self.count > time)
		HomeThink();	

	particle4(self.origin,20,random(128,143),PARTICLETYPE_GRAV,4);
	
	self.angles = vectoangles(self.velocity);

	self.think = GolemIMissileThink;
	thinktime self : 0.1;
}

void GolemISpawnMissile(vector vect, vector offset, float vel)
{
	local entity missile;
	local vector vec;

	self.last_attack=time;
	missile = spawn ();
	missile.classname = "golem_iron_proj";
	missile.owner = self;
	missile.enemy = missile.goalentity = self.enemy;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.yaw_speed = 3;
	missile.drawflags (+) MLS_POWERMODE | SCALE_TYPE_UNIFORM;
	missile.scale = 2.5;
	

	setmodel (missile, "models/golemmis.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

	// set missile speed	

	makevectors (self.angles);
	setorigin (missile, self.origin + offset);

	vec = self.enemy.origin - missile.origin + self.enemy.view_ofs;
	
	vec = normalize(vec);

	vec += vect;

	missile.speed = 300;
	missile.velocity = vec * (300 - random(vel));
	missile.angles = vectoangles(missile.velocity);
	
	missile.touch = GolemIMissileTouch;

	missile.think = GolemIMissileThink;

	missile.veer=0;				//slight veering, random course modifications
	missile.turn_time = 0.5;
	missile.hoverz=TRUE;			//slow down on turns
	missile.ideal_yaw=TRUE;

	missile.count = time + 2;

	thinktime missile : 0.2;
}

void GolemIMissile(void) [++ $igem1..$igem24]
{
	vector vect;

	ai_face();

	makevectors(self.angles);

	if (self.frame == $igem1)
		self.colormap = 128 + 16;

	if (self.frame == $igem1)
	{
		vect = self.origin + (v_forward * 16);
		particle4(vect + '0 0 90',15,256+random(128,143),PARTICLETYPE_GRAV,10);
	}

	self.colormap -= 0.5;

	if (self.frame == $igem17 && FacingIdeal())
	{
		sound(self, CHAN_WEAPON, "golem/gbfire.wav", 1, ATTN_NORM);
		//GolemISpawnMissile(v_right * -1, '0 0 75', 20);
		
		GolemISpawnMissile('0 0 0', '0 0 75', 100);
		/*GolemISpawnMissile(v_right, '0 0 75', 20);
		GolemISpawnMissile(v_up * -1, '0 0 75', 20);
		GolemISpawnMissile(v_up, '0 0 75', 20);*/
	
		self.think = self.th_run;
		self.colormap = 0;
		thinktime self : 0.1;
	}

}
	
//==========================================================================
//
// GolemIMeleeDecide
//
//==========================================================================

void GolemIMeleeDecide(void)
{
	if(random() < 0.5)
	{
		GolemPunchRight();
	}
	else
	{
		GolemPoundRight();
	}
}

//==========================================================================
//
// GolemBMeleeDecide
//
//==========================================================================

void GolemBMeleeDecide(void)
{
	if(random() < 0.5)
	{
		GolemPunchRight();
	}
	else
	{
		GolemPoundRight();
	}
}

//==========================================================================
//
// GolemPunchRight
//
//==========================================================================

void GolemPunchRight(void) [++ $rpunch1..$rpunch24]
{
	vector checkPos;

	if(cycle_wrapped)
	{
		GolemRun();
		return;
	}
	if(self.frame == $rpunch10)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
		sound(self, CHAN_WEAPON, "golem/swing.wav", 1, ATTN_NORM);
	}
	else if(self.frame == $rpunch24)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	//ai_charge(8);
	ai_face();

	if (!walkmove(self.angles_y, 8, TRUE))
	{
		if (trace_ent.health <= 0) return;
	}

	if(self.frame > $rpunch10 && self.frame < $rpunch16)
	{
		makevectors(self.enemy.angles);
		
		checkPos = self.enemy.origin + (v_forward * -24);
		checkPos_z = self.enemy.origin_z + self.enemy.view_ofs_z;

		traceline(self.enemy.origin, checkPos, FALSE, self.enemy);

		
		if (trace_fraction < 1&&!trace_ent.flags2&FL_ALIVE)
			GolemDoMelee(1);
		else
			GolemDoMelee(0);
	}
}

//==========================================================================
//
// GolemPoundRight
//
//==========================================================================

void GolemPoundRight(void) [++ $rpound1..$rpound24]
{
	vector checkPos;

	if(cycle_wrapped)
	{
		GolemRun();
		return;
	}
	if(self.frame == $rpound10)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
		sound(self, CHAN_WEAPON, "golem/swing.wav", 1, ATTN_NORM);
	}
	else if(self.frame == $rpound24)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	
	ai_face();

	if (!walkmove(self.angles_y, 10, TRUE))
	{
		if (trace_ent.health <= 0) return;
	}
	
	if(self.frame > $rpound10 && self.frame < $rpound16)
	{
		makevectors(self.enemy.angles);
		
		checkPos = self.enemy.origin + (v_forward * -24);
		checkPos_z = self.enemy.origin_z + self.enemy.view_ofs_z;

		traceline(self.enemy.origin, checkPos, FALSE, self.enemy);

		
		if (trace_fraction < 1)
			GolemDoMelee(1);
		else
			GolemDoMelee(0);
	}
}

//==========================================================================
//
// GolemSRushBegin
//
//==========================================================================

void GolemSRushBegin(void) [++ $rush1..$rush12]
{
	if(self.frame == $rush12)
	{
		self.golemSlideCounter = 8;
		self.think = GolemSRushSlide;
		sound(self, CHAN_WEAPON, "golem/slide.wav", 1, ATTN_NORM);
	}
	else if(self.frame == $rush10)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	//ai_charge(5);
	ai_face();

	if (!walkmove(self.angles_y, 5, TRUE))
	{
		if (trace_ent.health <= 0) return;
	}
	
}

//==========================================================================
//
// GolemSRushSlide
//
//==========================================================================

void GolemSRushSlide(void) [$rush12 GolemSRushSlide]
{
	if(walkmove(self.angles_y, 20, FALSE))
	{
		if(random() < 0.2)
		{
			CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
		}
	}
	else
	{
		self.think = GolemSRushEnd;
		return;
	}
	self.golemSlideCounter -= 1;
	if(self.golemSlideCounter < 0)
	{
		self.think = GolemSRushEnd;
	}
}

float GolemBCheckBeamAttack(void)
{
	vector p1, p2, off;
	float dist;

	makevectors(self.angles);

	dist = vlen(self.enemy.origin - self.origin);
	
	off = v_forward * 15 + v_right * 1;
	p1 = self.origin + '0 0 92' + off;

	p2 = p1 + (v_forward * dist);
	p2_z = self.enemy.origin_z + self.enemy.proj_ofs_z;
		
	traceline(p1, p2, FALSE, self);

	if (trace_ent == self.enemy) 
		return 1;
	
	return 0;
}

float GolemICheckMissileAttack(void)
{
	vector p1, p2, off;
	float dist;

	makevectors(self.angles);

	dist = vlen(self.enemy.origin - self.origin);
	
	off = v_forward * 15 + v_right * 1;
	p1 = self.origin + '0 0 92' + off;

	p2 = p1 + (v_forward * dist);
	p2_z = self.enemy.origin_z + self.enemy.proj_ofs_z;
		
	traceline(p1, p2, FALSE, self);

	if (trace_ent == self.enemy) 
		return 1;
	
	return 0;
}

void GolemBBeamFinish(void) [++ $bgem18..$bgem24]
{
	if (self.frame == $bgem24)
	{
		self.colormap = 0;
		self.think = self.th_run;
		thinktime self : 0.1;
		return;
	}
}

void GolemDoBeam(float offset, float damage)
{
	vector p1, p2, off, dir;
	float dist;

	makevectors(self.angles);

	dist = vlen(self.enemy.origin - self.origin);
	
	off = v_forward * 15 + v_right * 1;
	p1 = self.origin + '0 0 92' + off;

	dist = vlen(p1 - self.enemy.origin + self.enemy.proj_ofs - '0 0 6');

	/*p2 = p1 + (v_forward * dist + v_right * offset);
	p2_z = self.enemy.origin_z + self.enemy.proj_ofs_z;*/

	dir = normalize(v_forward*100 + v_right * offset);
	p2 = p1 + dir*dist;
	p2_z = self.enemy.origin_z + self.enemy.proj_ofs_z - 6;

	dir = normalize(p2-p1);
	traceline(p1, p2+dir*(dist/2), FALSE, self);
			
	//traceline(p1, p2, FALSE, self);

	if (trace_ent != world && trace_ent.health > 0) 
	{
		sound(trace_ent,CHAN_AUTO,"crusader/sunhit.wav",1,ATTN_NORM);

		if (trace_ent.health - damage <= 0) 
			trace_ent.decap = TRUE;

		if (trace_ent.flags & FL_MONSTER)
			T_Damage(trace_ent,self,self,damage/2);
		else 
			T_Damage(trace_ent,self,self,damage);
	}

	SpawnPuff(trace_endpos, '0 0 10', 1, trace_ent);
	
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_STREAM_COLORBEAM);	//beam type
	WriteEntity (MSG_BROADCAST, self);				//owner
	WriteByte (MSG_BROADCAST, 0);					//tag + flags
	WriteByte (MSG_BROADCAST, 1);					//time
	WriteByte (MSG_BROADCAST, 2);			//color

	WriteCoord (MSG_BROADCAST, p1_x);
	WriteCoord (MSG_BROADCAST, p1_y);
	WriteCoord (MSG_BROADCAST, p1_z);

	WriteCoord (MSG_BROADCAST, trace_endpos_x);
	WriteCoord (MSG_BROADCAST, trace_endpos_y);
	WriteCoord (MSG_BROADCAST, trace_endpos_z);		
}

void GolemBBeamFireArch1(void) [$bgem18 GolemBBeamFireArch1]
{
	if (self.golemBeamDelay < time)
	{
		self.think = GolemBBeamFinish;
		thinktime self : 0.1;
		return;
	}
	
	self.golemBeamOff1 += 5;
	
	GolemDoBeam(self.golemBeamOff1, random(7, 12));
}

void GolemBBeamFireArch2(void) [$bgem18 GolemBBeamFireArch2]
{
	if (self.golemBeamDelay < time)
	{
		self.think = GolemBBeamFinish;
		thinktime self : 0.1;
		return;
	}
	
	if (!self.golemBeamOff2) 
		self.golemBeamOff1 += 10;
	else 
		self.golemBeamOff1 -= 10;

	
	if (self.golemBeamOff1 == 100)
		self.golemBeamOff2 = 1;

	GolemDoBeam(self.golemBeamOff1, random(7, 12));
}

void GolemBBeamFire(void) [++ $bgem11..$bgem18]
{
	float r;
	
	ai_face();

	if (self.frame == $bgem18)
	{	
		r = random();

		if (visible(self.enemy) && FacingIdeal())
		{
			sound(self, CHAN_WEAPON, "golem/gbfire.wav", 1, ATTN_NORM);
			
			if (r < 0.5)
			{
				self.golemBeamDelay = time + 2;
				self.golemBeamOff1 = -100;
				self.think = GolemBBeamFireArch1;
				thinktime self : 0.1;
			}
			else 
			{
				self.golemBeamDelay = time + 2;
				self.golemBeamOff1 = -100;
				self.golemBeamOff2 = 0;
				self.think = GolemBBeamFireArch2;
				thinktime self : 0.1;
			}
		}
		else
		{
			self.think = self.th_run;
			thinktime self : 0.1;
		}
	}
}

void GolemBBeamPause(void) [$bgem11 GolemBBeamPause]
{
	if (self.golemBeamDelay < time && !self.cnt)
	{
		self.golemBeamDelay = time + 0.5;
		self.cnt = 1;
		self.colormap = 176 + 16;
	}

	self.colormap -= 1;

	if (self.golemBeamDelay > time) 
	{
		particle4(self.origin + self.view_ofs,5,185 + random(6),PARTICLETYPE_SLOWGRAV,1);
		ai_face();
		return;
	}
		
	self.cnt = 0;
	GolemBBeamFire();
}

//==========================================================================
//
// GolemBBeam
//
//==========================================================================

void GolemBBeamBegin(void) [++ $bgem1..$bgem11]
{
	sound(self, CHAN_WEAPON, "golem/gbcharge.wav", 1, ATTN_NORM);

	if (self.frame == $bgem11) 
	{
		thinktime self : 0.1;
		self.think = GolemBBeamPause;
	}
}

void GolemBStompEffect(void)
{
	float dist;

	dist = vlen(self.enemy.origin - self.origin);

	MonsterQuake(350);	

	if (dist < 350)
		T_Damage(self.enemy, self, self, random(50/dist));
}
	
//==========================================================================
//
// GolemBStomp
//
//==========================================================================

void GolemBStomp(void) [++ $stomp1..$stomp24]
{
	float numPuffs;
	vector vect;

	if (self.frame == $stomp13)
	{
		sound(self, CHAN_BODY, "golem/stomp.wav", 1, ATTN_NORM);
		numPuffs = random(4,10);

		makevectors(self.angles);

		while(numPuffs > 0)
		{
			vect_x = self.origin_x;
			vect_y = self.origin_y;
			vect_z = self.absmin_z;

			vect_z += v_up_z * 3;
			vect_x += v_forward_x * random(0, self.size_x);
			vect_y += v_forward_y * random(0, self.size_y);

			particle4(vect,20+random(1,10),256+random(90, 95),PARTICLETYPE_GRAV,30);
			numPuffs -= 1;
		}
			GolemBStompEffect();
	}

	if (self.frame == $stomp24)
	{
		self.think = self.th_run;
		thinktime self : 0.1;
	}
}

void GolemICheckRushDamage( void )
{
	float r;
	float damage;
	
	if(!self.enemy)
		return;

	r = vlen(self.enemy.origin - self.origin);

	damage = random(20,30);

	
	if (infront(self.enemy) && r < 60)
	{
		sound(self, CHAN_BODY, "imp/swoophit.wav", 1, ATTN_NORM);

		makevectors(self.angles);

		self.enemy.flags (-) FL_ONGROUND;
		self.enemy.velocity = (v_forward * (damage * 20));
		self.enemy.velocity_z = random(300, 350);

		T_Damage(self.enemy, self, self, damage);
	}
}

//==========================================================================
//
// GolemSRushEnd
//
//==========================================================================

void GolemSRushEnd(void) [++ $rush13..$rush24]
{
	if(cycle_wrapped)
	{
		GolemRun();
		return;
	}
	
	if (self.frame == $rush15)
	{
		GolemICheckRushDamage();
	}

	if(self.frame == $rush24)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	//ai_charge(4);
	ai_face();

	if (!walkmove(self.angles_y, 4, TRUE))
	{
		if (trace_ent.health <= 0) return;
	}
}

//==========================================================================
//
// GolemFlinch
//
//==========================================================================

float GolemFlinch(float firstFrame, float lastFrame)
{
	if(self.frame < firstFrame || self.frame > lastFrame)
	{
		return 0;
	}
	self.nextthink += 0.1+random()*0.2;
	self.frame = self.frame - 8 - rint(random() * 12);
	self.pain_finished = time + 1;
	if(self.frame < firstFrame)
	{ // Wrap
		self.frame = lastFrame + 1 - (firstFrame - self.frame);
	}
	return 1;
}

//==========================================================================
//
// GolemSPain
//
//==========================================================================

void GolemSPain(void)
{
	if(self.pain_finished > time)
	{
		return;
	}
	//if(GolemFlinch($gwalk1, $gwalk60)) return;
	//if(GolemFlinch($gLpnch1, $gLpnch22)) return;
	//if(GolemFlinch($gRpnch1, $gRpnch22)) return;

	//GolemFlinch($run1, $run24);
}

//==========================================================================
//
// GolemIPain
//
//==========================================================================

void GolemIPain(void)
{
	if(self.pain_finished > time)
	{
		return;
	}
	//if(GolemFlinch($gwalk1, $gwalk60)) return;
	//if(GolemFlinch($ggem1, $ggem25)) return;
	//if(GolemFlinch($gRpnch1, $gRpnch22)) return;

	//GolemFlinch($run1, $run24);
}

//==========================================================================
//
// GolemBPain
//
//==========================================================================

void GolemBPain(void)
{
	if(self.pain_finished > time)
	{
		return;
	}
	//if(GolemFlinch($gwalk1, $gwalk60)) return;
	//if(GolemFlinch($ggem1, $ggem25)) return;
	//if(GolemFlinch($gLpnch1, $gLpnch22)) return;
	//if(GolemFlinch($gRpnd1, $gRpnd20)) return;

	//GolemFlinch($run1, $run24);
}

float GolemCheckSolidGround( void )
{
	vector p1, p2, p3, p4, destin;
	float numSolid = 4;

	makevectors(self.angles);

	p1 = self.origin + (v_forward * (self.size_y * 0.8));
	p2 = self.origin - (v_forward * (self.size_y * 0.8));
	p3 = self.origin + (v_right * (self.size_x * 0.5));
	p4 = self.origin - (v_right * (self.size_x * 0.5));
		
	destin = p1 - (v_up * self.size_y);
	traceline(p1, destin, FALSE, self);
	if (trace_fraction == 1) numSolid -= 1;

	destin = p2 - (v_up * self.size_y);
	traceline(p2, destin, FALSE, self);
	if (trace_fraction == 1) numSolid -= 1;

	destin = p3 - (v_up * self.size_y);
	traceline(p3, destin, FALSE, self);
	if (trace_fraction == 1) numSolid -= 1;

	destin = p4 - (v_up * 2);
	traceline(p4, destin, FALSE, self);
	if (trace_fraction == 1) numSolid -= 1;

	if (numSolid < 3)
		return 0;
	
	return 1;
}


void GolemChunkPlace(string gibname, vector pos, vector vel)
{
	local	entity new;

	makevectors(self.angles);

	new = spawn();
	new.origin = pos;
	setmodel (new, gibname);
	setsize (new, '0 0 0', '0 0 0');
	
	new.velocity = vel;
	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_NOT;
	new.angles = self.angles;

	if (gibname != "models/golegs.mdl")
	{
		new.avelocity_y = random(100,500);
		
		new.velocity_x += random(v_forward_x * 100,v_forward_x * 300);
		new.velocity_y += random(v_forward_y * 100,v_forward_y * 300);
		new.velocity_z += random(v_forward_z * 350,v_forward_z * 600);

		new.flags (-) FL_ONGROUND;
	}	
	
	if (self.classname == "monster_golem_crystal")
	{
		new.drawflags = DRF_TRANSLUCENT|MLS_ABSLIGHT;
		new.abslight = 1.4;
		new.skin = GLOBAL_SKIN_ICE;
	}
	if (self.classname == "monster_golem_stone")
		new.skin = 0;	
	if (self.classname == "monster_golem_iron")
	{
		new.scale = 1.5;
		new.skin = 1;
	}
	if (self.classname == "monster_golem_bronze")
	{
		new.scale = 2.0;
		new.skin = 2;
	}

	new.think = SUB_Remove;
	new.ltime = time;
	new.nextthink = time + 10 + random()*10;
	new.frame = 0;
	new.flags = 0;
}

void GolemChunkDeath(void)
{
	float numPuffs;
	vector vect, dir;

	makevectors(self.angles);
	dir = v_right*random(150, 200);
	dir_z = random(100, 300);
	GolemChunkPlace("models/goarm.mdl", self.origin + (v_right * 16), dir);

	dir = v_right*random(150, 200)*-1;
	dir_z = random(100, 300);
	GolemChunkPlace("models/goarm.mdl", self.origin + (v_right * -16), dir);

	GolemChunkPlace("models/golegs.mdl", self.origin + (v_forward * -12), '0 0 0');
	GolemChunkPlace("models/g-head.mdl", self.origin + (v_forward * random(10, 20)), '0 0 250');
	numPuffs = random(4,10);

	while(numPuffs > 0)
	{
		vect = self.origin;

		vect_z += v_up_z * 4;
		vect_x += v_forward_x * random(0, self.size_x);
		vect_y += v_forward_y * random(0, self.size_y);

		particle4(vect,20+random(1,10),256+random(90, 95),PARTICLETYPE_GRAV,10);
		numPuffs -= 1;
	}

	if (self.classname == "monster_golem_crystal")
		self.thingtype = THINGTYPE_ICE;

	self.think = chunk_death;
	thinktime self: 0.1;
}

void GolemDeathFinish(void) [++ $death12..$death22]
{	if(self.frame == $death22)
	{
		self.nextthink = -1;
		if(self.classname == "monster_golem_stone" || self.classname == "monster_golem_crystal")
		{
			sound(self, CHAN_BODY, "golem/stnfall.wav", 1, ATTN_NORM);
		}
		else if(self.classname == "monster_golem_iron")
		{
			sound(self, CHAN_BODY, "golem/mtlfall.wav", 1, ATTN_NORM);
		}
		else
		{ // Assumed bronze
			sound(self, CHAN_BODY, "golem/mtlfall.wav", 1, ATTN_NORM);
		}


		if (GolemCheckSolidGround())
		{
			GolemChunkDeath();
			//MakeSolidCorpse();
		}
		else
		{
			self.think = chunk_death;
			thinktime self : 0.1;
		}

		return;
	}
	
	if(self.frame == $death16)
	{
		self.solid = SOLID_NOT;
	}
	
	if (self.frame > $death16)
	{
		makevectors(self.angles);
		self.origin += v_forward * 4;
	}

	if(self.health < -50)
	{
		self.think = chunk_death;
	}

	thinktime self : 0.07;
}

void GolemDeathPause(void) [$death11 GolemDeathPause]
{
	vector vect;

	if ((self.cnt - time) == 1)
	{
		sound(self, CHAN_BODY, "golem/dthgroan.wav", 1, ATTN_NORM);
	
		if (self.classname == "monster_golem_bronze")
		{
			makevectors(self.angles);
			vect = self.origin + (v_forward * (self.size_x / 2));
			particle4(vect + '0 0 80',15,256+random(184, 191),PARTICLETYPE_GRAV,10);
		}
		if (self.classname == "monster_golem_iron")
		{
			makevectors(self.angles);
			vect = self.origin + (v_forward * (self.size_x / 2));
			particle4(vect + '0 0 70',15,256+random(128,143),PARTICLETYPE_GRAV,10);
		}
	}

	if (self.cnt < time)
		GolemDeathFinish();
}

//==========================================================================
//
// GolemDie
//
//==========================================================================

void GolemDie(void) [++ $death1..$death11]
{
	self.colormap = 0;

	if (self.frame == $death11)
	{
		if (self.classname == "monster_golem_bronze" || self.classname == "monster_golem_iron")
			sound(self, CHAN_BODY, "golem/mtlpain.wav", 1, ATTN_NORM);
		else
			sound(self, CHAN_BODY, "golem/stnpain.wav", 1, ATTN_NORM);

		self.cnt = time + 1;
		GolemDeathPause();
	}
}
