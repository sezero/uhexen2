
//**************************************************************************
//**
//** spider.hc
//** bgokey
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/hw/spider.hc,v 1.1.1.1 2004-11-29 11:29:41 sezero Exp $
//**
//**************************************************************************

// FRAMES ------------------------------------------------------------------

// Scuttle
$frame swalk1 swalk2 swalk3 swalk4 swalk5
$frame swalk6 swalk7 swalk8 swalk9 swalk10
$frame swalk11 swalk12 swalk13 swalk14 swalk15
$frame swalk16

// Resting
$frame swait1 swait2 swait3 swait4 swait5
$frame swait6 swait7 swait8 swait9 swait10
$frame swait11 swait12 swait13 swait14 swait15
$frame swait16 swait17 swait18 swait19 swait20
$frame swait21 swait22 swait23 swait24 swait25
$frame swait26

// Transition from scuttle/resting to attack pose
$frame sattak1 sattak2 sattak3 sattak4 sattak5
$frame sattak6 sattak7 sattak8 sattak9 sattak10
$frame sattak11 sattak12

// Bite attack (from attack pose)
$frame sbite1 sbite2 sbite3 sbite4 sbite5
$frame sbite6 sbite7 sbite8 sbite9 sbite10

// Jump (from scuttle/resting)
$frame sjump1 sjump2 sjump3 sjump4 sjump5
$frame sjump6 sjump7 sjump8 sjump9 sjump10
$frame sjump11 sjump12 sjump13 sjump14 sjump15
$frame sjump16

// Pain (from scuttle/resting)
$frame sdpain1 sdpain2 sdpain3 sdpain4 sdpain5
$frame sdpain6 sdpain7 sdpain8 sdpain9 sdpain10

// Pain (from attack pose)
$frame supain1 supain2 supain3 supain4 supain5
$frame supain6 supain7 supain8

// Death
$frame sdeath1 sdeath2 sdeath3 sdeath4 sdeath5
$frame sdeath6 sdeath7 sdeath8 sdeath9 sdeath10
$frame sdeath11 sdeath12 sdeath13 sdeath14 sdeath15
$frame sdeath16 sdeath17 sdeath18 sdeath19 sdeath20

// CONSTANTS ---------------------------------------------------------------

float SPIDER_RED_LARGE = 0;
float SPIDER_RED_SMALL = 1;
float SPIDER_YELLOW_LARGE = 2;
float SPIDER_YELLOW_SMALL = 3;
float ONWALL = 32;
float SPIDER_FRAME_TIME = 0.025;

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void SpiderInit(float type);
void SpiderStand(void);
void SpiderWalk(void);
void SpiderRun(void);
void SpiderMeleeBegin(void);
void SpiderJumpBegin(void);
void SpiderMelee(void);
void SpiderMeleeEnd(void);
void SpiderPause(void);
void SpiderPain(entity attacker, float total_damage);
void SpiderPainLow(void);
void SpiderPainHigh(void);
void SpiderDie(void);
void SpiderGone(void);
void SpiderJumpOffWall();
// PRIVATE DATA DEFINITIONS ------------------------------------------------

float SpiderHealth[4] =
{
	175,		// SPIDER_RED_LARGE
	50,			// SPIDER_RED_SMALL
	75,			// SPIDER_YELLOW_LARGE
	30			// SPIDER_YELLOW_SMALL
};

float SpiderExp[4] =
{
	225,	// SPIDER_RED_LARGE
	50,		// SPIDER_RED_SMALL
	100,	// SPIDER_YELLOW_LARGE
	25		// SPIDER_YELLOW_SMALL
};

// CODE --------------------------------------------------------------------

void monster_spider(void) {}

//==========================================================================
//
// monster_spider_red_large
//
//==========================================================================
/*QUAKED monster_spider_red_large (1 0.3 0) (-16 -16 0) (16 16 26) AMBUSH STUCK JUMP PLAY_DEAD DORMANT ONWALL
Large red spider.

------- key / value ----------------------------------
health = 100
experience_value = 800
------- spawnflags -----------------------------------
AMBUSH
ONWALL = Spider can wait on a wall or ceiling and jump down then it finds an enemy or is triggered.  Put the spider next to a wall or cieling and face him away from the surface you want it to hang from.
IMPORTANT!  Put ONWALL spiders 8 pixels away from the wall you want them to be on, or they'll be all screwed up!
*/

void monster_spider_red_large(void)
{
	SpiderInit(SPIDER_RED_LARGE);
}

//==========================================================================
//
// monster_spider_red_small
//
//==========================================================================
/*QUAKED monster_spider_red_small (1 0.3 0) (-12 -12 0) (12 12 16) AMBUSH STUCK JUMP PLAY_DEAD DORMANT ONWALL
Small red spider.

------- key / value ----------------------------------
health = 100
experience_value = 800
------- spawnflags -----------------------------------
AMBUSH
ONWALL = Spider can wait on a wall or ceiling and jump down then it finds an enemy or is triggered.  Put the spider next to a wall or cieling and face him away from the surface you want it to hang from.
IMPORTANT!  Put ONWALL spiders 8 pixels away from the wall you want them to be on, or they'll be all screwed up!
*/

void monster_spider_red_small(void)
{
	SpiderInit(SPIDER_RED_SMALL);
}

//==========================================================================
//
// monster_spider_yellow_large
//
//==========================================================================
/*QUAKED monster_spider_yellow_large (1 0.3 0) (-16 -16 0) (16 16 26) AMBUSH STUCK JUMP PLAY_DEAD DORMANT ONWALL
Large yellow spider.

------- key / value ----------------------------------
health = 100
experience_value = 800
------- spawnflags -----------------------------------
AMBUSH
ONWALL = Spider can wait on a wall or ceiling and jump down then it finds an enemy or is triggered.  Put the spider next to a wall or cieling and face him away from the surface you want it to hang from.
IMPORTANT!  Put ONWALL spiders 8 pixels away from the wall you want them to be on, or they'll be all screwed up!
*/

void monster_spider_yellow_large(void)
{
	SpiderInit(SPIDER_YELLOW_LARGE);
}

//==========================================================================
//
// monster_spider_yellow_small
//
//==========================================================================
/*QUAKED monster_spider_yellow_small (1 0.3 0) (-12 -12 0) (12 12 16) AMBUSH STUCK JUMP PLAY_DEAD DORMANT ONWALL
Small yellow spider.
------- key / value ----------------------------------
health = 100
experience_value = 800
------- spawnflags -----------------------------------
AMBUSH
ONWALL = Spider can wait on a wall or ceiling and jump down then it finds an enemy or is triggered.  Put the spider next to a wall or cieling and face him away from the surface you want it to hang from.
IMPORTANT!  Put ONWALL spiders 8 pixels away from the wall you want them to be on, or they'll be all screwed up!
*/

void monster_spider_yellow_small(void)
{
	SpiderInit(SPIDER_YELLOW_SMALL);
}

void spider_possum_up (void) [-- $sdeath20..$sdeath1]
{
	if (cycle_wrapped)
		self.think=self.th_run;
}

void spider_playdead (void)
{
	self.frame=$sdeath20;
	self.think=spider_playdead;
	self.nextthink=time+0.1;
	ai_stand();
}

//==========================================================================
//
// SpiderInit
//
//==========================================================================

void spider_noise ()
{
float r;
	r=random();
	if(r<0.33)
		self.noise="spider/step1.wav";
	else if(r<0.66)
		self.noise="spider/step2.wav";
	else
		self.noise="spider/step3.wav";
	sound(self, CHAN_BODY, self.noise, random(0.7, 1), ATTN_NORM);
}

void spider_onwall_awakened ()
{
	self.use=monster_use;
	sound(self, CHAN_VOICE, "spider/pain.wav", 1, ATTN_NORM);
	self.goalentity=self.enemy;
	setorigin(self,self.origin+self.movedir*22);
	makevectors(self.enemy.angles);
	self.view_ofs='0 0 22';
	self.velocity=normalize(self.enemy.origin+v_forward*24-self.origin)*200;
	self.flags(-)FL_FLY;
	self.movetype=MOVETYPE_TOSS;
	self.last_attack=time;
	self.think=SpiderJumpOffWall;
	thinktime self : random(0.5);
}

void spider_onwall_use ()
{
	self.enemy = activator;
	if (self.enemy.classname == "player")
	{	// let other monsters see this monster for a while
		sight_entity = self;
		sight_entity_time = time;
	}
	self.show_hostile = time + 1;		// wake up other monsters
	spider_onwall_awakened();
}

void spider_onwall_wait () [++ $swait1..$swait25]
{
	thinktime self : 0.1;
	if(random()<0.5)
		if(LocateTarget())
			spider_onwall_awakened();
}
	
void spider_onwall_wait_init ()
{
	total_monsters += 1;
	if(self.angles_y==-2)
		self.angles='90 0 0';	//face down
	self.flags(+)FL_FLY;		//No gravity
	walkmonster_start();
	makevectors(self.angles);
	setorigin(self,self.origin-v_forward*22);
	self.view_ofs=v_forward*36;
	self.movedir=v_forward;
	traceline(self.origin,self.origin-v_forward*100,TRUE,self);
	pitch_roll_for_slope(trace_plane_normal);
	self.use=spider_onwall_use;
	if(self.targetname=="")
	{
		self.think=spider_onwall_wait;
		thinktime self : 0;
	}
	else
		self.nextthink=-1;
}

void SpiderInit(float type)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.flags2&FL_SUMMONED)
		precache_spider();

	setmodel(self, "models/spider.mdl");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;

	self.yaw_speed = 10;

	if(type > SPIDER_RED_SMALL)
	{ // Yellow spiders use skin 1
		self.skin = 1;
	}

	if(type&1)
	{ // Small spiders
		self.scale = random(0.7,0.9);
		self.mass = 1;
		self.speed=5;
		setsize(self, '-12 -12 0', '12 12 16');//was 16 - 28 to stop them from stepping over each other?
	}
	else
	{ // Large spiders
		self.scale = random(1.1,1.4);
		self.mass = 3;
		self.speed=10;
		setsize(self, '-16 -16 0', '16 16 26');//was 26 - 28 to stop them from stepping over each other?
	}
	self.hull=HULL_CROUCH;

	self.drawflags = SCALE_ORIGIN_BOTTOM;
	self.spawnflags (+) JUMP;

	self.spiderType = type;
	self.spiderGoPause = 35;
	self.mintel = 10;
	self.netname = "spider";

	self.health = SpiderHealth[type];
	self.experience_value = SpiderExp[type];

	self.attack_state = AS_STRAIGHT;
	self.thingtype = THINGTYPE_FLESH;
	self.th_stand = SpiderStand;
	self.th_walk = SpiderWalk;
	self.th_run = SpiderRun;
	self.th_die = SpiderDie;
	self.th_melee = SpiderMeleeBegin;
	self.th_missile = SpiderJumpBegin;
	self.th_pain = SpiderPain;
	self.th_possum = spider_playdead;
	self.th_possum_up = spider_possum_up;

	self.flags (+) FL_MONSTER;

	if(self.spawnflags&ONWALL)
		spider_onwall_wait_init();
	else
		walkmonster_start();
}


void SpiderDie(void) [++ $sdeath1..$sdeath20]
{
	if(self.frame == $sdeath1)
	{
		sound(self, CHAN_VOICE, "spider/death.wav", 1, ATTN_NORM);
		self.flags(-)FL_FLY;
		if(!self.flags&FL_ONGROUND)
			self.movetype=MOVETYPE_BOUNCE;
	}

	if(self.frame == $sdeath20)
	{
		self.think=SpiderGone;
	}
	if(self.health<-20)
		chunk_death();
}

void SpiderGone(void)
{
	MakeSolidCorpse();
}

void SpiderPain(entity attacker, float total_damage)
{
	if(random(self.health)>total_damage)
		return;

	sound(self, CHAN_VOICE, "spider/pain.wav", 1, ATTN_NORM);
	if(self.flags&FL_FLY)		//on wall
	{
		spider_onwall_awakened();
	}
	else if(self.frame < $sattak8 || self.frame > $sbite10)
	{
		SpiderPainLow();
	}
	else
	{
		SpiderPainHigh();
	}
}

void SpiderPainLow(void) [++ $sdpain1..$sdpain10]
{
	if(cycle_wrapped)
	{
		self.enemy = world;
		SpiderRun();
	}
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderPainHigh(void) [++ $supain1..$supain8]
{
	if(cycle_wrapped)
	{
		self.enemy = world;
		SpiderRun();
	}
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderMeleeBegin(void) [++ $sattak1..$sattak12]
{
	ai_charge(self.speed/2);
	if(self.frame == $sattak1)
		self.last_attack=time;
	else if(self.frame == $sattak12)
	{
		self.think = SpiderMelee;
	}
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderMeleeSettle (void) [++ $sjump12..$sjump16]
{
	ai_charge(self.speed/2);
	if(self.frame==$sjump16)
		self.think=self.th_run;
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderJumpTouch ()
{
	if(other.takedamage&&other.model!="models/spider.mdl")
	{
		T_Damage(other,self,self,10);
		self.touch=obj_push;
	}
}

void SpiderJumpBegin(void) [++ $sjump1..$sjump11]
{
	if(self.frame==$sjump1)
	{
		if(!CheckJump()||self.level<20)
		{
			self.think=SpiderRun;
			self.nextthink=time;
			return;
		}
		else
			self.last_attack=time;
	}
	if(self.frame<$sjump5)
	{
		self.angles_y=self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
		ai_charge(self.speed/2);
	}
	else if(self.frame==$sjump5)
	{
//FIXME: If land on someone, jump off!
		self.last_attack=time;
		self.touch=SpiderJumpTouch;
		self.last_attack=time;
		makevectors(self.angles);
	    self.jump_flag=time + 7;        //Only try to jump once every 7 seconds
	    self.velocity=v_forward*self.level*12*self.scale;
	    self.velocity_z = self.level*10*self.scale;
		self.flags(-)FL_ONGROUND;
	}
	else if(self.frame == $sjump11)
	{
		self.touch=obj_push;
		self.frame=$sattak7;
		self.think = SpiderMeleeBegin;
	}
}

void SpiderJumpOffWall(void) [++ $sjump1..$sjump11]
{
	if(self.angles_x!=0)
	{
		if(self.angles_x<=10&&self.angles_x>=-10)
			self.angles_x=0;
		else
			self.angles_x/=1.5;
	}
	if(self.angles_z!=0)
	{
		if(self.angles_z<=10&&self.angles_z>=-10)
			self.angles_z=0;
		else
			self.angles_z/=1.5;
	}
	ai_face();

	if(self.flags&FL_ONGROUND)
		if(self.movetype!=MOVETYPE_STEP)
		{
			self.movetype=MOVETYPE_STEP;
			sound(self,CHAN_BODY,"player/land.wav",1,ATTN_NORM);
		}

	if(self.frame==$sjump11)
		if(self.flags&FL_ONGROUND)
		{
			self.movetype=MOVETYPE_STEP;
			self.angles_x=0;
			self.angles_z=0;
			self.frame=$sattak7;
			self.think = SpiderMeleeBegin;
		}
		else
			self.frame-=1;
}

void SpiderMelee(void) [++ $sbite1..$sbite10]
{
	self.last_attack=time;
	if(self.frame == $sbite7)
	{//FIXME: adjust dmg for scale
		ai_melee();
	}
	if(self.frame == $sbite6)
	{
		sound(self, CHAN_VOICE, "spider/bite.wav", 1, ATTN_NORM);
	}
	if(self.frame > $sbite5 && self.frame < $sbite9)
	{
		ai_charge(self.speed/1.5);
	}
	else
	{
		ai_charge(self.speed/2.5);
	}
	if(self.frame == $sbite10)
	{
		if(range(self.enemy)==RANGE_MELEE)
		{
			if(random()<0.7-skill/10)	//Based on skill level, try attacking again
				self.think = SpiderMeleeEnd;
		}
		else
			self.think = SpiderMeleeEnd;
	}
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderMeleeEnd(void) [-- $sattak12..$sattak1]
{
	if(self.frame == $sattak1)
	{
		//self.think = SpiderPause;
		self.think = SpiderRun;
	}
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderPauseInit(void)
{
	self.th_save = self.think;

	// Frames to remain paused
	self.spiderPauseLength = 7+random()*8;

	// Generate a new active/pause threshold
	self.spiderGoPause = 16+random()*16;

	self.spiderPauseCount = self.spiderActiveCount = 0;
	SpiderPause();
}

void SpiderPause(void) [++ $swait1..$swait26]
{
	if((self.spiderPauseCount += 1) > self.spiderPauseLength)
	{
		self.think = self.th_save;
		if not(self.think)
		{
			self.think = SpiderStand;
		}
	}
}

void SpiderRun(void) [++ $swalk1..$swalk16]
{
	if((self.spiderActiveCount += 1) > self.spiderGoPause && random()>skill/5)
	{ // Pause for a bit
		SpiderPauseInit();
		return;
	}

	if(self.attack_state==AS_STRAIGHT)
	{
		if(random()<0.1)
			self.attack_state=AS_SLIDING;
	}
	else if(self.attack_state==AS_SLIDING)
		if(random()<0.3)
			self.attack_state=AS_STRAIGHT;

	if(self.spiderType&1)
	{ // Small spiders
		ai_run(self.speed/self.attack_state);//slower strafing
	}
	else
	{ // Large spiders
		ai_run(self.speed/self.attack_state);
	}
	if(random()<0.2)
	{
		spider_noise();
		pitch_roll_for_slope('0 0 0');
	}

	if ((self.flags2 & FL_SUMMONED)  && (self.lifetime < time))  // Summoned spiders only live a little while
		SpiderDie();
	thinktime self : SPIDER_FRAME_TIME;
}

void SpiderStand(void) [++ $swait1..$swait25]
{
	if(random()<0.5)
		ai_stand();
}

void SpiderWalk(void) [++ $swalk1..$swalk16]
{
	ai_walk(self.speed/2);
	if(random()<0.1)
	{
		spider_noise();
		pitch_roll_for_slope('0 0 0');
	}
}
