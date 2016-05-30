/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/pstboar.hc,v 1.2 2007-02-07 16:57:09 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\RdrPest\BOAR\final\Hpest.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RdrPest\BOAR\final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

// Boar frames
$frame Bcharg1      Bcharg2      Bcharg3      Bcharg4      Bcharg5      
$frame Bcharg6      Bcharg7      Bcharg8      

//
$frame BtranA1      BtranA2      BtranA3      BtranA4      BtranA5      
$frame BtranA6      BtranA7      BtranA8      BtranA9      BtranA10     
$frame BtranA11     BtranA12     BtranA13     

//
$frame BtranB1      BtranB2      BtranB3      BtranB4      BtranB5      
$frame BtranB6      BtranB7      BtranB8      BtranB9      BtranB10     
$frame BtranB11     BtranB12     

//
$frame BtranC1      BtranC2      BtranC3      BtranC4      BtranC5      
$frame BtranC6      BtranC7      BtranC8      

//
$frame BtranD1      BtranD2      BtranD3      BtranD4      BtranD5      
$frame BtranD6      BtranD7      BtranD8      

//
$frame Btrot1       Btrot2       Btrot3       Btrot4       Btrot5       
$frame Btrot6       Btrot7       Btrot8       Btrot9       Btrot10      
$frame Btrot11      Btrot12      

$framevalue 0

// Rider Frames
$frame Parrow1      Parrow2      Parrow3      Parrow4      Parrow5      
$frame Parrow6      

//
$frame Pcharg1      Pcharg2      Pcharg3      Pcharg4      Pcharg5      
$frame Pcharg6      Pcharg7      Pcharg8      

//
$frame Phive1       Phive2       Phive3       Phive4       Phive5       
$frame Phive6       Phive7       Phive8       Phive9       Phive10      
$frame Phive11      Phive12      Phive13      Phive14      Phive15      
$frame Phive16      Phive17      Phive18      Phive19      Phive20      
$frame Phive21      Phive22      Phive23      Phive24      Phive25      
$frame Phive26      Phive27      Phive28      Phive29      Phive30      
$frame Phive31      Phive32      Phive33      Phive34      Phive35      
$frame Phive36      Phive37      Phive38      

//
$frame PtranA1      PtranA2      PtranA3      PtranA4      PtranA5      
$frame PtranA6      PtranA7      PtranA8      PtranA9      PtranA10     
$frame PtranA11     PtranA12     PtranA13     

//
$frame PtranB1      PtranB2      PtranB3      PtranB4      PtranB5      
$frame PtranB6      PtranB7      PtranB8      PtranB9      PtranB10     
$frame PtranB11     PtranB12     

//
$frame PtranC1      PtranC2      PtranC3      PtranC4      PtranC5      
$frame PtranC6      PtranC7      PtranC8      

//
$frame PtranD1      PtranD2      PtranD3      PtranD4      PtranD5      
$frame PtranD6      PtranD7      PtranD8      

//
$frame Ptrot1       Ptrot2       Ptrot3       Ptrot4       Ptrot5       
$frame Ptrot6       Ptrot7       Ptrot8       Ptrot9       Ptrot10      
$frame Ptrot11      Ptrot12      

float pst_start[8] =
{
	$Btrot1,
	$Btrot1,
	$Btrot1,
	$Bcharg1,
	$BtranC1,
	$BtranD1,
	$BtranB1,
	$Btrot1
};

float pst_end[8] =
{
	$Btrot12,
	$Btrot12,
	$Btrot12,
	$Bcharg8,
	$BtranC8,
	$BtranD8,
	$BtranB1,
	$Btrot12
};

float pst_speed[8] =
{
	7,
	7,
	7,
	20,
	15,
	15,
	7,
	7
};

// Array to align frames
float PstRiderFrames[8] =
{
	$Ptrot1,			// Animation for trot
	$Parrow1,			// Animation for trot
	$Phive1,		// Animation for trot
	$Pcharg1,			// Animation for trot
	$PtranC1,			// Animation for trot
	$PtranD1,			// Animation for trot
	$PtranB1,		// Animation for trot
	$Ptrot1			// Animation for trot
};

float PB_STAGE_NORMAL		= 0;
float PB_STAGE_ATTACK		= 1;
float PB_STAGE_ATTACK2		= 2;
float PB_STAGE_CHARGE		= 3;
float PB_STAGE_CHARGE_ST	= 4;
float PB_STAGE_CHARGE_END	= 5;
float PB_STAGE_STAND		= 6;
float PB_STAGE_BACKUP		= 7;

void rider_death();


void hive_trail ()
{
	if(self.lifetime<time)
	{
	    self.think=self.th_die;
		thinktime self : 0;
	}
	else
	{
//		particle4(self.origin,0.5,284,PARTICLETYPE_SLOWGRAV,3);
		thinktime self : 0.05;
	}
}
void()hive_touch;
void throw_hive (void)
{
	entity newmis;
	vector diff;

    makevectors(self.movechain.angles);
	sound(self,CHAN_AUTO,"misc/whoosh.wav",1,ATTN_NORM);
    newmis=spawn();
    newmis.owner=self;
    newmis.classname="hive";
    newmis.movetype=MOVETYPE_BOUNCE;
    newmis.solid=SOLID_BBOX;
	newmis.takedamage=DAMAGE_YES;
	newmis.touch=newmis.th_die=hive_touch;
	newmis.lifetime=time + 2.5;

	newmis.speed=500;
	
	setorigin (newmis, self.origin + '0 0 110' + v_forward * 25);
	diff = (self.enemy.origin + self.enemy.view_ofs) - newmis.origin;
	newmis.velocity = normalize(diff)+v_up*0.5;
	newmis.velocity = newmis.velocity * newmis.speed;

	newmis.avelocity=randomv('-300 -300 -300','300 300 300');
    setmodel(newmis,"models/boss/hive.mdl");
	newmis.scale=2;
	newmis.drawflags=MLS_POWERMODE;
    setsize(newmis,'0 0 0','0 0 0');

	newmis.think=hive_trail;
	thinktime newmis : 0;
}

void poison_think ()
{
	self.enemy.deathtype="poison";
	T_Damage (self.enemy, self, self.owner, 1 );
	if(self.enemy.flags&FL_CLIENT)
		stuffcmd(self.enemy,"bf\n");
	if(self.lifetime<time||self.enemy.health<=0)
		self.think=SUB_Remove;
	thinktime self : 1;
}

void spawn_poison ()
{
	newmis=spawn();
	newmis.think=poison_think;
	newmis.enemy=self.enemy;
	newmis.owner=self.owner;

	thinktime newmis : 0.05;
	newmis.lifetime=time+random(5,10);
}

void pestilence_missile_touch(void)
{
	float damg;

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(10,20);

	self.origin = self.origin - 8 * normalize(self.velocity) - '0 0 40';
	if(other.takedamage)
	{
		self.enemy=other;
		if(other.flags&FL_CLIENT)
			stuffcmd(other,"bf\n");
		if(other.classname!="rider_pestilence")
			spawn_poison();
		other.deathtype="poison";
		T_Damage (other, self, self.owner, damg );
		sound (self, CHAN_WEAPON, "pest/xbowhit.wav", 1, ATTN_NORM);
	}
	else
		sound (self, CHAN_WEAPON, "weapons/expsmall.wav", 1, ATTN_NORM);

	CreateGreenSpark (self.origin); 

	remove(self);
}


void pestilence_missile_think (void)
{
	if (self.lifetime < time)
	{
		sound (self, CHAN_WEAPON, "pest/xbowhit.wav", 1, ATTN_NORM);
		CreateGreenSpark (self.origin); 
		remove(self);
	}
	else
	{
		HomeThink();
		
		self.think=pestilence_missile_think;
		thinktime self : HX_FRAME_TIME;
	}
}

void pestilence_missile(void)
{
	entity newmis;

	sound(self,CHAN_AUTO,"pest/xbowfire.wav",1,ATTN_NORM);	
	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
	newmis.drawflags=MLS_POWERMODE;
		
    setmodel (newmis, "models/pestshot.mdl");
	setsize (newmis, '0 0 0', '0 0 0');		

	newmis.angles = self.movechain.angles;

	makevectors(self.movechain.angles);	
	newmis.velocity = normalize(v_forward);
	newmis.angles = vectoangles(newmis.velocity);
	newmis.avelocity_z=700;

	setorigin (newmis, self.origin + '0 0 90' + v_forward * 20);
	
	newmis.speed=700;	//Speed
	newmis.classname = "faminemissile";

	newmis.veer=FALSE;	//No random wandering
	newmis.turn_time=3;	//Lower the number, tighter the turn
	newmis.ideal_yaw=FALSE;//Only track things in front
	newmis.lifetime = time + 2;
	newmis.think=pestilence_missile_think;
	thinktime newmis : 0;

	newmis.touch = pestilence_missile_touch;
}

void pest_hurt (entity attacker,float total_dam)
{
	self.pain_finished=time+0.5;
	if(attacker!=world&&attacker!=self)
		if(self.enemy==world||(random()<0.2&&attacker!=self.enemy))
		{
			if(random()<0.2)
				sound(self,CHAN_VOICE,"pest/sight.wav",1,ATTN_NONE);	
			self.enemy=self.goalentity=attacker;
			self.lifespan=time+4;
		}
}

void pest_use ()
{
	self.monster_awake=TRUE;
	self.lifespan=time+10;
}

void psthorse_move(void)
{
float chance;
vector displace;
entity oself;

	self.think = psthorse_move;
	thinktime self : HX_FRAME_TIME;

	if(self.monster_stage!=PB_STAGE_STAND)
		AdvanceFrame(pst_start[self.monster_stage],pst_end[self.monster_stage]);

	if(self.frame!=$BtranB1)
	{
		if(!self.goalentity)
			self.goalentity=self.enemy;

		if (random(100)==1)
			if(self.monster_stage>=PB_STAGE_CHARGE&&self.monster_stage<PB_STAGE_STAND)
				sound(self,CHAN_BODY,"pest/snort2.wav",1,ATTN_NORM);	
			else
				sound(self,CHAN_BODY,"pest/snort.wav",1,ATTN_NORM);	

		if(self.t_width<time)
		{
			if(self.monster_stage>=PB_STAGE_CHARGE&&self.monster_stage<PB_STAGE_STAND)
				sound(self,CHAN_ITEM,"pest/gallop.wav",1,ATTN_NORM);	
			else
			{
				chance = random();
				if (chance < .33)
					sound (self, CHAN_ITEM, "pest/clop1.wav", 1, ATTN_NORM);
				else if (chance < .66)
					sound (self, CHAN_ITEM, "pest/clop2.wav", 1, ATTN_NORM);
				else
					sound (self, CHAN_ITEM, "pest/clop3.wav", 1, ATTN_NORM);
			}
			self.t_width=time+0.5;
		}

		self.wallspot=self.origin;
		if(self.enemy!=world&&visible(self.enemy)||self.search_time>time)
		{
			if(!walkmove(self.angles_y,self.speed,TRUE))
			{
				if (trace_ent!=world&&trace_ent.takedamage&&self.torchtime<time)
				{//hit once every 1/2 sec
					self.torchtime=time+0.5;
					sound(trace_ent,CHAN_AUTO,"imp/swoophit.wav",1,ATTN_NORM);
					displace = normalize(trace_ent.origin - self.origin);
					if (infront(trace_ent))
					{
						trace_ent.velocity += displace*random(1000,1600);
						trace_ent.punchangle= randomv('-9 -10 -10','-1 10 10');
						T_Damage (trace_ent, self, self, random(25,35));
					}
					else
					{
						trace_ent.velocity += displace*random(700,900);
						trace_ent.punchangle = randomv('-3 -5 -5','2 5 5');
						T_Damage (trace_ent, self, self, random(15,20));
					}
				}
				movetogoal(self.speed);
				if(self.monster_stage==PB_STAGE_BACKUP)
					self.monster_stage=self.rider_gallop_mode=PB_STAGE_NORMAL;
			}
		}
		else if(self.monster_awake)
		{
			if (!self.path_current)
				riderpath_init();
			riderpath_move(self.speed);
		}


		if(vlen(self.origin-self.wallspot)<self.speed*0.75&&self.turn_time<time&&!trace_ent)
			self.aflag+=1;
		else
			self.aflag=0;
		self.ideal_yaw=vectoyaw(self.goalentity.origin-self.origin);
		if(self.aflag>30)
		{
			self.monster_stage=PB_STAGE_BACKUP;
			self.aflag=0;
			self.turn_time=time+3;//Back up for 3 secs
		}
		if(self.aflag)
		{
			makevectors(self.angles);
			tracearea(self.origin,self.origin+v_forward*200,'-48 -48 0','48 48 100',TRUE,self);
			if(trace_plane_normal!='0 0 0')
				self.ideal_yaw=vectoyaw((trace_plane_normal+v_forward)*0.5);
			navigate(fabs(self.speed));
		}
		ChangeYaw();
	}

	if ((!self.enemy) || (self.enemy.health <= 0))
	{
		if(self.enemy)
			sound(self,CHAN_VOICE,"pest/laugh.wav",1,ATTN_NONE);	
		
		if (!FindTarget(TRUE))
		{
			self.enemy = world;
			if(!self.monster_awake)
				self.monster_stage = self.rider_gallop_mode = PB_STAGE_STAND;
		}
		else
			sound(self,CHAN_VOICE,"pest/sight.wav",1,ATTN_NONE);	
		self.goalentity = self.enemy;
	}
	else
		self.monster_awake=TRUE;

	enemy_vis=visible(self.enemy);
	if(enemy_vis)
		self.search_time=time+5-coop*2;

	if(self.rider_gallop_mode<PB_STAGE_CHARGE_ST&&enemy_vis)
	{
		self.movechain.ideal_yaw=vectoyaw(self.enemy.origin-self.movechain.origin);
		if(self.movechain.ideal_yaw>self.angles_y+89&&self.movechain.ideal_yaw<self.angles_y+180)	//Flexible guy!
			self.movechain.ideal_yaw=self.angles_y+89;
		else if(self.movechain.ideal_yaw<self.angles_y - 89&&self.movechain.ideal_yaw>self.angles_y - 180)
			self.movechain.ideal_yaw=self.angles_y - 89;
	}
	else
		self.movechain.ideal_yaw=self.angles_y;
	oself=self;
	self=self.movechain;
	ChangeYaw();
	self=oself;
	
	if(self.rider_gallop_mode==PB_STAGE_ATTACK||self.rider_gallop_mode==PB_STAGE_ATTACK2)
		self.movechain.frame+=1;
	else if(self.rider_gallop_mode<PB_STAGE_STAND)
		self.movechain.frame = PstRiderFrames[self.rider_gallop_mode] + 
			(self.frame - pst_start[self.rider_gallop_mode]);

	// Is rider attacking player
	if (self.rider_gallop_mode == PB_STAGE_ATTACK)
	{
		if (self.movechain.frame == $Parrow4)
			pestilence_missile();

		if (self.movechain.frame >= $Parrow6)
			if(self.monster_stage==PB_STAGE_BACKUP)
				self.rider_gallop_mode=PB_STAGE_NORMAL;
			else
				self.rider_gallop_mode=self.monster_stage;
	}
	else if(self.rider_gallop_mode == PB_STAGE_ATTACK2)
	{
		if (self.movechain.frame == $Phive33)
			throw_hive();

		if (self.movechain.frame >= $Phive38)
			if(self.monster_stage==PB_STAGE_BACKUP)
				self.rider_gallop_mode=PB_STAGE_NORMAL;
			else
				self.rider_gallop_mode=self.monster_stage;
	}

	if(self.monster_stage >= PB_STAGE_CHARGE&&self.monster_stage<PB_STAGE_STAND)
	{
		if(random()<0.3)
		{
			makevectors(self.angles);
			CreateWhiteSmoke(self.origin-v_forward*random(50,160)+v_right*random(-30,30),v_up*random(10,30)+v_forward*random(-10,-30),HX_FRAME_TIME*2);
		}
		if(self.frame==$BtranC8)
		{
			self.movechain.frame=$Pcharg1;
			self.monster_stage=self.rider_gallop_mode=PB_STAGE_CHARGE;
		}
		if (self.frame == $Bcharg8 &&self.monster_stage == PB_STAGE_CHARGE)
		{
			self.level+=1;
			if(self.level>=random(20,40))
			{
				self.movechain.frame=$PtranD1;
				self.monster_stage=self.rider_gallop_mode=PB_STAGE_CHARGE_END;
			}
		}
		if(self.frame==$BtranD8)
		{
			self.lifespan=time + 5;
			self.monster_stage = self.rider_gallop_mode=PB_STAGE_NORMAL;
			self.yaw_speed=4;
		}
		if(self.rider_gallop_mode==PB_STAGE_CHARGE)
		{
			if(random(1000)<=5)
			{
				enemy_infront=infront_of_ent(self.enemy,self.movechain);
				if(enemy_vis&&(enemy_infront||random()<0.2))
					if (random()<0.1&&self.cnt<=0) // Shoot 
					{
						self.rider_gallop_mode = PB_STAGE_ATTACK2;
						self.movechain.frame = $Phive1;
					}
					else
					{
						self.rider_gallop_mode = PB_STAGE_ATTACK;
						self.movechain.frame = $Parrow1;
					}
			}
		}
	}

	if (fabs(pst_speed[self.monster_stage] - fabs(self.speed)) < 0.2)
		self.speed = pst_speed[self.monster_stage];
	else if (pst_speed[self.monster_stage] > fabs(self.speed)) 
		self.speed = fabs(self.speed)+0.2;
	else
		self.speed=fabs(self.speed) - 0.2;

	if(self.monster_stage==PB_STAGE_BACKUP)
		if(self.turn_time<time)
			self.monster_stage=self.rider_gallop_mode=PB_STAGE_NORMAL;
		else
			self.speed*=-1;

	if(self.enemy!=world||self.monster_awake)
	{
		enemy_vis=visible(self.enemy);
		enemy_infront=infront_of_ent(self.enemy,self.movechain);
		if (self.rider_gallop_mode == PB_STAGE_NORMAL&&self.enemy!=world&&enemy_vis&&enemy_infront) 
		{
			// Force a new gallop frame in

			chance = random(100);

			if(self.pain_finished>time)
				chance/=3;

			if(random(100)<3)
				sound(self,CHAN_VOICE,"pest/laugh.wav",1,ATTN_NONE);	

			if (chance < 3&&self.cnt<=0) // Shoot 
			{
				self.rider_gallop_mode = PB_STAGE_ATTACK2;
				self.movechain.frame = $Phive1;
			}
			else if(chance < 5) // Shoot 
			{
				self.rider_gallop_mode = PB_STAGE_ATTACK;
				self.movechain.frame = $Parrow1;
			}
			else if(chance < 8&&self.monster_stage<PB_STAGE_CHARGE&&self.lifespan<time) // Charge
			{
				sound(self,CHAN_VOICE,"pest/charge.wav",1,ATTN_NORM);	
				self.rider_gallop_mode = 
				self.monster_stage = PB_STAGE_CHARGE_ST;
				self.movechain.frame = $PtranC1;
				self.yaw_speed=8;
			}
		}
		else if (self.monster_stage == PB_STAGE_STAND) 
			if(self.frame==$BtranB12)
			{
				self.frame=$Btrot1;
				self.movechain.frame=$Ptrot1;
				self.monster_stage=self.rider_gallop_mode=PB_STAGE_NORMAL;
			}
			else
			{
				AdvanceFrame($BtranB1,$BtranB12);
				self.movechain.frame+=1;
			}
	}
	else
		self.rider_gallop_mode=self.monster_stage=PB_STAGE_STAND;
}


// Frame Code
void create_pstrider(entity horse)
{
entity rider;

	rider = spawn();

	rider.solid = SOLID_NOT;
	rider.movetype = MOVETYPE_NOCLIP;
	rider.origin = horse.origin;
	rider.angles = self.angles;

	setmodel (rider, "models/boss/pstrider.mdl");
	setsize(rider,'0 0 0','0 0 0');
	rider.skin = 0;

	horse.movechain = rider;
	rider.flags (+) FL_MOVECHAIN_ANGLE;
}

void swarm_touch(void)
{
	if(other!=world&&other.classname!="swarm"&&other.monsterclass<CLASS_BOSS)
	{
		if(random()<0.1)
			if (other.takedamage&&other.thingtype==THINGTYPE_FLESH)
			{
			float r;
				if(self.pain_finished<time)
				{
					r=random();
					if(r<.34)
						self.noise="pest/sting1.wav";
					else if(r<.67)
						self.noise="pest/sting2.wav";
					else
						self.noise="pest/sting3.wav";
					sound(self,CHAN_AUTO,self.noise,1,ATTN_NORM);
					self.pain_finished=time+1;
				}
				T_Damage (other,self,self,random(3,10));
			}
	}
}

void hive_die(void)
{
	if (AdvanceFrame(40 + self.beginframe,54 + self.beginframe) == AF_END)
	{
		self.owner.cnt-=1;
		remove(self);
	}
	else
	{
		if(self.t_width<time)
		{
			sound(self,CHAN_BODY,"pest/buzz.wav",0.5,ATTN_NORM);	
			self.t_width=time+0.5;
		}
		self.think = hive_die;
		thinktime self : HX_FRAME_TIME;
	}
}

void hive_live(void)
{
	movetogoal(10);
	self.velocity = normalize(self.enemy.origin - self.origin)* 100;
	AdvanceFrame(10 + self.beginframe,39 + self.beginframe);

	if (self.lifetime < time)
		hive_die();
	else
	{
		if(self.t_width<time)
		{
			sound(self,CHAN_BODY,"pest/buzz.wav",1,ATTN_NORM);	
			self.t_width=time+0.5;
		}
		self.think = hive_live;
		thinktime self : HX_FRAME_TIME;
	}
}

void hive_grow(void)
{
	if (AdvanceFrame(0 + self.beginframe,9 + self.beginframe) == AF_END)
	{
		self.lifetime = time + 20;	
		hive_live();
	}
	else
	{
		if(self.t_width<time)
		{
			sound(self,CHAN_BODY,"pest/buzz.wav",0.5,ATTN_NORM);	
			self.t_width=time+0.5;
		}
		self.think = hive_grow;
		thinktime self : HX_FRAME_TIME;
	}
}


void bugcloud_spawn (float cloudtype)
{
entity swarm;

	CreateWhiteSmoke(self.origin,'0 0 20',HX_FRAME_TIME * 4);
	CreateWhiteSmoke(self.origin,'0 0 21',HX_FRAME_TIME * 4);

	swarm = spawn();
	swarm.owner=self.owner;
	swarm.classname="swarm";
	swarm.solid = SOLID_TRIGGER;
	swarm.movetype = MOVETYPE_FLYMISSILE;
	swarm.flags(+)FL_FLY;
	
	swarm.touch = swarm_touch;
	swarm.scale = 2.5;		
	swarm.enemy = swarm.goalentity = self.owner.enemy;

	if (cloudtype == 1)
	{
		swarm.beginframe = 0;
		swarm.avelocity_y = random(-450,-600);
	}
	else if (cloudtype == 2)
	{
		swarm.beginframe = 54;
		swarm.avelocity_y = random(450,600);
	}
	else if (cloudtype == 3)
	{
		swarm.beginframe = 109;
		swarm.avelocity_y = random(450,600);
	}

	swarm.frame=swarm.beginframe;
	setmodel (swarm,"models/boss/swarm.mdl");
	setsize(swarm,'-50 -50 -50','50 50 50');
	swarm.hull=HULL_CROUCH;
	setorigin (swarm,self.origin + '0 0 5');

	swarm.think = hive_grow;
	thinktime swarm : HX_FRAME_TIME;
}

void hive_touch(void)
{
	sound(self,CHAN_AUTO,"pest/hivehit.wav",1,ATTN_NORM);

	bugcloud_spawn(1);
	bugcloud_spawn(2);
	bugcloud_spawn(3);

	self.owner.cnt+=3;
	self.level=-666;//Just a flag to turn off chunk sounds
	chunk_death();
}

/*
void create_swarm (void)
{
entity newmis;
vector diff;
	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_TOSS;
	newmis.solid = SOLID_BBOX;
	newmis.thingtype = THINGTYPE_FLESH;
	newmis.classname="hive";
		
	setmodel (newmis,"models/boss/hive.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	makevectors (self.v_angle);

	setorigin (newmis,self.origin + (v_forward * 14) + (v_up * 40));

	diff = ((self.origin  + (100 * v_forward)) - self.origin);
	newmis.velocity = normalize(diff);
	newmis.velocity = newmis.velocity * 1000;
	newmis.angles = vectoangles(newmis.velocity);

	newmis.enemy = self;
	newmis.touch = hive_touch;
}
*/
/*QUAKED rider_pestilence (1 0 0)  (-55 -55 -24) (55 55 100) TRIGGER_WAIT
Pestilence rider monster.  You must place rider_path entites
on the map.  The rider will first proceed to the 
rider_path point with a path_id of 1.
-------------------------FIELDS-------------------------
map: next map to go to when you kill the rider
target: start spot on the next map
--------------------------------------------------------

*/
void rider_pestilence(void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2 ("models/boss/boar.mdl");
	precache_model2 ("models/boss/pstrider.mdl");
	precache_model2 ("models/boss/hive.mdl");
	precache_model2 ("models/boss/swarm.mdl");
    precache_model2 ("models/boss/shaft.mdl");
	precache_model2 ("models/pestshot.mdl");
	precache_sound2 ("pest/snort.wav");
	precache_sound2 ("pest/clop1.wav");
	precache_sound2 ("pest/clop2.wav");
	precache_sound2 ("pest/clop3.wav");
	precache_sound2 ("pest/gallop.wav");
	precache_sound2 ("pest/sight.wav");
	precache_sound2 ("pest/sting1.wav");
	precache_sound2 ("pest/sting2.wav");
	precache_sound2 ("pest/sting3.wav");
	precache_sound2 ("pest/buzz.wav");
	precache_sound2 ("pest/hivehit.wav");
	precache_sound2 ("pest/xbowfire.wav");
	precache_sound2 ("pest/xbowhit.wav");
	precache_sound2 ("pest/die.wav");
	precache_sound2 ("pest/charge.wav");
	precache_sound2 ("pest/laugh.wav");
	precache_sound2 ("pest/snort2.wav");
	rider_init();

	self.noise="pest/die.wav";
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
    self.flags (+) FL_MONSTER;
	self.flags2(+) FL_ALIVE;
	self.yaw_speed = 4;
	self.speed=7;

	setmodel (self, "models/boss/boar.mdl");
	self.skin = 0;

	setsize (self, '-84 -84 0', '84 84 100');
	self.health = self.max_health = 4400;
	self.experience_value = 1500;
	self.th_pain = pest_hurt;
	self.rider_gallop_mode = self.monster_stage = PB_STAGE_NORMAL;
	self.speed = pst_speed[self.rider_gallop_mode];
	self.rider_path_distance = 200;
	self.mass = 30000;

	create_pstrider(self);

	self.use=pest_use;
	self.hull=HULL_GOLEM;

	self.frame=$BtranB1;
	self.movechain.frame=$PtranB1;
	self.movechain.yaw_speed=10;
	self.monster_stage=self.rider_gallop_mode=PB_STAGE_STAND;
	self.monster_awake=FALSE;
	self.th_save=psthorse_move;

	self.think = multiplayer_health;
	thinktime self : 1;
}

