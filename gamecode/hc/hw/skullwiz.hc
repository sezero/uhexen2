/*
 * hw/skullwiz.hc
 */

//
$frame skdeth1      skdeth2      skdeth3      skdeth4      skdeth5      
$frame skdeth6      skdeth7      skdeth8      skdeth9      skdeth10     
$frame skdeth11     skdeth12     skdeth13     skdeth14     skdeth15     

//
$frame skgate1      skgate2      skgate3      skgate4      skgate5      
$frame skgate6      skgate7      skgate8      skgate9      skgate10     
$frame skgate11     skgate12     skgate13     skgate14     skgate15     
$frame skgate16     skgate17     skgate18     skgate19     skgate20     
$frame skgate21     skgate22     skgate23     skgate24     skgate25     
$frame skgate26     skgate27     skgate28     skgate29     skgate30     

// Frame 46 - 57
$frame skpain1      skpain2      skpain3      skpain4      skpain5      
$frame skpain6      skpain7      skpain8      skpain9      skpain10     
$frame skpain11     skpain12     

// Frame 58 - 69
$frame skredi1      skredi2      skredi3      skredi4      skredi5      
$frame skredi6      skredi7      skredi8      skredi9      skredi10     
$frame skredi11     skredi12     

//
//$frame skspel1      skspel2      skspel3      skspel4      skspel5      
//$frame skspel6      skspel7      skspel8      skspel9      skspel10     
//$frame skspel11     skspel12     skspel13     skspel14     skspel15     
//$frame skspel16     skspel17     skspel18     skspel19     skspel20     
//$frame skspel21     skspel22     skspel23     skspel24     skspel25     
//$frame skspel26     skspel27     skspel28     skspel29     skspel30     
//$frame skspel31     

// Frame 70 - 84
$frame skspel2      skspel4            
$frame skspel6      skspel8      skspel10     
$frame skspel12     skspel14     
$frame skspel16     skspel18     skspel20     
$frame skspel22     skspel24          
$frame skspel26     skspel28     skspel30     


//
//$frame sktele1      sktele2      sktele3      sktele4      sktele5      
//$frame sktele6      sktele7      sktele8      sktele9      sktele10     
//$frame sktele11     sktele12     sktele13     sktele14     sktele15     
//$frame sktele16     sktele17     sktele18     sktele19     sktele20     
//$frame sktele21     sktele22     sktele23     sktele24     sktele25     
//$frame sktele26     sktele27     sktele28     sktele29     sktele30     
//$frame sktele31     

$frame sktele2      sktele4            
$frame sktele6      sktele8      sktele10     
$frame sktele12     sktele14          
$frame sktele16     sktele18     sktele20     
$frame sktele22     sktele24          
$frame sktele26     sktele28     sktele30     
//
$frame sktran1      sktran2      sktran3      sktran4      sktran5      
$frame sktran6      sktran7      

//
$frame skwait1      skwait2      skwait10     skwait11     skwait12     
$frame skwait13     skwait14     skwait15     skwait16     skwait17     
$frame skwait18     skwait19     skwait20     skwait21     skwait22     
$frame skwait23     skwait24     skwait25     skwait26     

//
$frame skwalk1      skwalk2      skwalk3      skwalk4      skwalk5      
$frame skwalk6      skwalk7      skwalk8      skwalk9      skwalk10     
$frame skwalk11     skwalk12     skwalk13     skwalk14     skwalk15     
$frame skwalk16     skwalk17     skwalk18     skwalk19     skwalk20     
$frame skwalk21     skwalk22     skwalk23     skwalk24     


void skullwiz_walk(void);
void skullwiz_run(void);
void skullwiz_melee(void);
void skullwiz_blink(void);
void skullwiz_push (void);
void skullwiz_missile_init (void);

float SKULLBOOK  =0;
float SKULLHEAD  =1;

float() SkullFacingIdeal =
{
	local	float	delta;
	
	delta = anglemod(self.angles_y - self.ideal_yaw);
	if (delta > 25 && delta < 335)
		return FALSE;
	return TRUE;
};

void phase_init (void)
{
	vector spot1,newangle;
	float loop_cnt,forward;

	trace_fraction =0;
	loop_cnt = 0;
	do
	{
		newangle = self.angles;
		newangle_y = random(0,360);
   		makevectors (newangle);
		forward = random(40,100);
		spot1 = self.origin + v_forward * forward;
		tracearea (spot1,spot1 + v_up * 80,'-32 -32 -10','32 32 46',FALSE,self);
		if ((trace_fraction == 1.0) && (!trace_allsolid)) // Check there is a floor at the new spot
		{
			traceline (spot1, spot1 + (v_up * -4) , FALSE, self);

			if (trace_fraction ==1)   // Didn't hit anything?  There was no floor
				trace_fraction = 0;   // So it will loop
			else 
				trace_fraction = 1;   // So it will end loop					
		}
		else
			trace_fraction = 0;

		loop_cnt += 1;

		if (loop_cnt > 500)   // No endless loops
		{
			self.nextthink = time + 2;
			return;
		}

	} while (trace_fraction != 1.0);

	self.origin = spot1;	
}

float check_defense_blink ()
{
vector spot1,spot2,dangerous_dir;
float dot;
	if(!self.enemy)
		return FALSE;

	if(!visible(self.enemy))
		return FALSE;

	if(self.enemy.last_attack<time - 0.5)
		return FALSE;

	spot1=self.enemy.origin+self.enemy.proj_ofs;
	spot2=(self.absmin+self.absmax)*0.5;
	dangerous_dir=normalize(spot2-spot1);

	if(self.enemy.classname=="player")
		makevectors(self.enemy.v_angle);
	else
		makevectors(self.enemy.angles);

	dot=dangerous_dir*v_forward;
	if(dot>0.8)
		return TRUE;
	else
		return FALSE;
}

void skullwiz_throw(float part)
{
	entity new;
	new = spawn();

	if (part==SKULLBOOK)
		setmodel (new, "models/skulbook.mdl");
	else
		setmodel (new, "models/skulhead.mdl");

	new.origin_x = random(10,10);
	new.origin_y = random(10,10);
	new.origin_z = 40;

	setorigin(new,self.origin + new.origin);
	setsize (new, '0 0 0', '0 0 0');
	new.velocity_z = random(100,150);
	new.velocity_x = random(100,150);
	new.velocity_y = random(100,150);

	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_NOT;
	new.avelocity_x = random(400,600);
	new.avelocity_y = random(400,600);
	new.avelocity_z = random(400,600);
	new.flags(-)FL_ONGROUND;

	new.think=MakeSolidCorpse;
	new.nextthink = time + HX_FRAME_TIME * 15;
}


void()skullwiz_summon;
void spider_spawn (float spawn_side)
{
	newmis = spawn ();
	newmis.cnt=spawn_side;			// Shows which side to appear on
	newmis.flags2=FL_SUMMONED;
	newmis.nextthink = time + .01;
	newmis.think = skullwiz_summon;
	newmis.origin = self.origin;
	newmis.controller = self;

	newmis.angles = self.angles;
}


void skullwiz_die (void) [++ $skdeth1.. $skdeth15]
{
	entity newent,holdent;

	thinktime self : HX_FRAME_TIME * 1.5;
	self.scale = 1;

	if (self.frame == $skdeth1)
	{
		self.solid = SOLID_NOT;
		CreateWhiteSmoke(self.origin + '0 0 20', '0  0 12', HX_FRAME_TIME *10);
		CreateWhiteSmoke(self.origin + '0 0 20', '0  8  8', HX_FRAME_TIME *10);
		CreateWhiteSmoke(self.origin + '0 0 20', '0 -8  8', HX_FRAME_TIME *10);

		if (self.classname == "monster_skull_wizard")  
			sound (self, CHAN_VOICE, "skullwiz/death.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "skullwiz/death2.wav", 1, ATTN_NORM);

		spider_spawn(0);
		if (random() < .5)
			spider_spawn(1);

		if (self.classname == "monster_skull_wizard_lord")  // Another two for the wizard lord
		{
			spider_spawn(0);
//			if (random() < .5)
//				spider_spawn(1);
		}
	}

	if (self.frame == $skdeth6)
		skullwiz_throw(SKULLBOOK);

	if (self.frame == $skdeth7)
	{
		setorigin(self,self.origin + '0 0 10');   // Throw robe
		self.flags(-)FL_ONGROUND;
		self.velocity_z = random(100,150);
		self.velocity_x = random(100,150);
		self.velocity_y = random(100,150);

		self.avelocity_x = random(400,600);
		self.avelocity_y = random(400,600);
		self.avelocity_z = random(400,600);
		self.mass = 99999;
	}

	if (self.frame == $skdeth8)
	{
		skullwiz_throw(SKULLHEAD);
	}

	if (self.frame == $skdeth15)
	{
		if (self.classname == "monster_skull_wizard_lord")  
		{
			newent = spawn();
			setorigin(newent,self.origin + '0 0 16');
			newent.lifespan = random(10,15);
			newent.lifetime = time + newent.lifespan;
			newent.thingtype = GREY_PUFF;
			newent.wait = 1.5;

			holdent = self;
			self = newent;
			fx_smoke_generator();
			self = holdent;
		}

		MakeSolidCorpse();
	}
}

void spider_grow(void)
{
	thinktime self : HX_FRAME_TIME;
	self.think = spider_grow;

	self.scale += 0.03;

	if (self.scale>= 0.50)
		walkmonster_start();
}


void skullwiz_summon(void)
{
	vector newangle,spot1,spot2,spot3;
	float loop_cnt;

	setmodel(self, "models/spider.mdl");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.drawflags=SCALE_ORIGIN_BOTTOM;

	self.flags2=FL_SUMMONED;
	self.yaw_speed = 10;
	self.mass = 1;
	self.speed=5;

	// Small spiders
	setsize(self, '-7 -7 0', '7 7 10');

	self.lifetime = time + 30;

	self.skin = 1;
	self.health = 5;
	self.experience_value = SpiderExp[1];

	self.drawflags = SCALE_ORIGIN_BOTTOM;
	self.spawnflags (+) JUMP;

	self.spiderType = 1;
	self.spiderGoPause = 35;
	self.mintel = 10;
	self.netname = "spider";

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
	self.classname = "monster_spider_yellow_small";

	self.flags (+) FL_MONSTER;

	if (self.controller.flags2&FL_ALIVE)
	{
		trace_fraction = 0;
		loop_cnt =0;
		while (trace_fraction < 1)
		{
			newangle = self.angles;
	
			if (self.cnt)			// On one side
				newangle_y -=random(45);
			else						// On the other side
				newangle_y +=random(45);
	
			makevectors (newangle);
	
			spot1 = self.origin;
			spot2 = spot1 + (v_forward * random(60,160));
			traceline (spot1, spot2 , FALSE, self);
			
			if (trace_fraction == 1.0 && trace_ent == world)		// Check no one is standing where monster wants to be
			{
				tracearea (spot1,spot2,'-30 -30 0','30 30 64',FALSE,self);
	
				if (trace_fraction == 1)		// Check there is a floor at the new spot
				{
					spot3 = spot2 + (v_up * -4);
					traceline (spot2, spot3 , FALSE, self);
	
					if (trace_fraction ==1)   // Didn't hit anything?  There was no floor
						trace_fraction = 0;   // So it will loop
					else 
						trace_fraction = 1;   // So it will end loop
						
				}
			}
			loop_cnt +=1;
			if (loop_cnt > 500)   // No endless loops
			{
				self.nextthink = time + 2;
				return;
			}
		}
		spot2 = spot2 + (v_forward * -16);  // Move back 16 from point to allow for side of his bounding box
 	}
	else
	{
		phase_init();
		spot2 = self.origin;
	}
	self.scale = 0.1;

	setorigin(self,spot2);	
	//reateWhiteSmoke (self.origin+'0 0 3','0 0 8');

	sound (self, CHAN_VOICE, "skullwiz/gate.wav", 1, ATTN_NORM);

	spider_grow();
}

void skullwiz_summoninit (void) [++ $skgate1..$skgate30]
{

	if (self.frame == $skgate2)   // Gate in the creatures
		sound (self, CHAN_VOICE, "skullwiz/gatespk.wav", 1, ATTN_NORM);

	if (self.frame == $skgate21)   // Gate in the creatures
	{
		spider_spawn(0);

		if (random() < 0.15)   // 15% chance he'll do another
		{
			spider_spawn(1);
		}
	}

	if (cycle_wrapped)
		skullwiz_run();
}

/*-----------------------------------------
	skullwiz_transition - transition from 
  -----------------------------------------*/
void skullwiz_transition (void) [++ $sktran1.. $sktran7]
{
	if (self.frame == $sktran1)
		self.attack_finished = time + random(0.5,3.5);

	if (cycle_wrapped)
		skullwiz_run();
}

/*-----------------------------------------
	skullwiz_pain - flinch in pain
  -----------------------------------------*/
void skullwiz_pain_anim () [++ $skpain1 .. $skpain12]
{
	if (self.frame == $skpain2)
	{
		if (self.classname == "monster_skull_wizard")
			sound (self, CHAN_BODY, "skullwiz/pain.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_BODY, "skullwiz/pain2.wav", 1, ATTN_NORM);
	}

	if (self.frame < $skpain11)
		self.frame += 1;

	if (self.frame>=$skpain12)
	{
		self.pain_finished = time + 3;

		if (random() < .20)
			skullwiz_blink();
		else
			skullwiz_run();
	}
}

void skullwiz_pain (entity attacker, float damg)
{
	if (self.pain_finished > time||random(self.health)>damg)
		return;

	skullwiz_pain_anim();
}


void SkullMissileTouch (void)
{
	float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if (self.owner.classname == "monster_skull_wizard")
		damg = random(5,13);
	else
		damg = random(10,18);

	if (other.health)
		T_Damage (other, self, self.owner, damg );

	self.origin = self.origin - 8*normalize(self.velocity);
	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

    BecomeExplosion (FALSE);
}

void SkullMissile_Twist2(void)
{
	vector holdangle;

	self.think = SkullMissile_Twist2;
	thinktime self : .2;

	holdangle = self.angles;
	if (!self.cnt)
	{
		holdangle_x = 0 - holdangle_x + 10;
		self.cnt = 1;
	}
	else
	{
		holdangle_x = 0 - holdangle_x - 10;
		self.cnt = 0;
	}

		
	makevectors (holdangle);
	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * 600;

	if (self.lifetime < time )
		remove(self);

	if (self.scream_time < time)
	{
		sound (self, CHAN_BODY, "skullwiz/scream2.wav", 1, ATTN_NORM);
		self.scream_time = time + random(.50,1);
	}	
}

void SkullMissile_Twist(void)
{
	self.think = SkullMissile_Twist;
	thinktime self : .2;

	if (self.lifetime < time )
		remove(self);

	if (self.scream_time < time)
	{
		sound (self, CHAN_BODY, "skullwiz/scream.wav", 1, ATTN_NORM);
		self.scream_time = time + random(.50,1);
	}	
}

/*-----------------------------------------
	launch_skullshot - launch the missile
  -----------------------------------------*/
void launch_skullshot ()
{
	local vector diff;

	self.last_attack=time;
	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
//	newmis.drawflags = MLS_FIREFLICKER;

	setmodel (newmis, "models/skulshot.mdl");
	newmis.hull=HULL_POINT;
	newmis.takedamage=DAMAGE_YES;
	newmis.health=1;
	newmis.dmg=10;
	newmis.th_die=MultiExplode;
	setsize (newmis, '-7 -7 -10', '7 7 10');		
	makevectors (self.angles);
	setorigin (newmis, self.origin + v_forward*20 - v_right * 16 + v_up * 45);

// set missile speed	
	diff = (self.enemy.origin + self.enemy.view_ofs) - newmis.origin ;
	newmis.velocity = normalize(diff+aim_adjust(self.enemy));
	newmis.velocity = newmis.velocity * 600;
	newmis.classname = "skullmissile";
	newmis.angles = vectoangles(newmis.velocity);
	

	newmis.scale=1.7;
	newmis.touch = SkullMissileTouch;

// set missile duration
	thinktime newmis : .10;
	newmis.lifetime = time + 2.5;
	newmis.scream_time = time + random(.5,1);

	self.cnt = 0;
	if (self.classname == "monster_skull_wizard_lord")
	{
		sound (newmis, CHAN_BODY, "skullwiz/scream2.wav", 1, ATTN_NORM);
		newmis.skin = 1;
		newmis.think = SkullMissile_Twist2;
	}
	else
	{
		sound (newmis, CHAN_BODY, "skullwiz/scream.wav", 1, ATTN_NORM);
		newmis.think = SkullMissile_Twist;
		newmis.scale = .90;
	}
	
}

/*-----------------------------------------
	skullwiz_missile - throw missile
  -----------------------------------------*/
void skullwiz_missile (void) [++ $skspel2..$skspel30]
{
	vector delta,spot1,spot2;

	delta = self.enemy.origin - self.origin;
//f (vlen(delta) < 50)  // Too close to shoot with a missile
//skullwiz_melee();

	if (self.frame == $skspel20)
	{
		ai_face();

		makevectors(self.angles);	
		// see if any entities are in the way of the shot
		spot1 = self.origin + v_right * 10 + v_up * 36;
		spot2 = self.enemy.origin + self.enemy.view_ofs;

		traceline (spot1, spot2, FALSE, self);
		if (trace_ent == self.enemy)
		{
			if (SkullFacingIdeal())
			{
				sound (self, CHAN_WEAPON, "skullwiz/firemisl.wav", 1, ATTN_NORM);
				launch_skullshot();
			}
			else
				self.frame-=1;
		}
	}

	if (cycle_wrapped)
	{
		// Attack again or walk a little
		if (random() < .5)		// Shoot again
			skullwiz_missile();			
		else if(self.skin&&random()<skill/10+0.2)
			skullwiz_blink();
		else
			skullwiz_transition();
	}
//	else
//		ai_face();

}

/*-----------------------------------------
	skullwiz_missile_init - ready to throw missile
  -----------------------------------------*/
void skullwiz_missile_init (void) [++ $skredi1..$skredi12]
{

	self.frame += 2;

	if (cycle_wrapped)
	   skullwiz_missile();
}

void skullwiz_blinkin(void) 
{
	float max_scale;

	thinktime self : HX_FRAME_TIME;
	self.think = skullwiz_blinkin;

	self.scale += 0.10;
	ai_face();

	if (self.classname == "monster_skull_wizard")
		max_scale = 1;
	else
		max_scale = 1.20;

	if (self.scale >= max_scale)
	{
		self.scale=max_scale;
		self.th_pain=skullwiz_pain;
		self.takedamage = DAMAGE_YES;
		skullwiz_run();
	}

}

void skullwiz_blinkin1 (void) 
{
	thinktime self : HX_FRAME_TIME;
	self.think = skullwiz_blinkin;

	setmodel(self, "models/skullwiz.mdl");
	self.frame = $skwalk1;
}

void skullwiz_ininit (void)
{
vector spot1,spot2,spot3,newangle,enemy_dir;
float loop_cnt,forward,dot;

	if (self.enemy != world)
	{
		if (!self.enemy.flags2 & FL_ALIVE)
		{
			self.enemy = world;
			self.goalentity = world;
		}
	}

	trace_fraction =0;
	loop_cnt = 0;
	do
	{
		if(self.enemy)
		{
			makevectors(self.enemy.angles);
			enemy_dir=self.enemy.velocity;
			enemy_dir_z=0;
			enemy_dir=normalize(enemy_dir);
			dot=enemy_dir*v_forward;
			enemy_dir_y=self.enemy.angles_y+360;
			if(dot>0.5)
				newangle_y=enemy_dir_y+random(-45,45);
			else
				newangle_y=enemy_dir_y+random(45,315);
		}
		else
		{
			newangle = self.angles;
			newangle_y = random(360);
		}

   		makevectors (newangle);
		if (self.enemy)
			spot1 = self.enemy.origin;
		else
			spot1 = self.origin;

		forward = random(120,200);
		spot2 = spot1 + (v_forward * forward);
		traceline (spot1, spot2 + (v_forward * 30) , FALSE, self.enemy);
		if (trace_fraction == 1.0) //  Check no one is standing where monster wants to be
		{
   			makevectors (newangle);
			tracearea (spot2,spot2 + v_up * 80,'-32 -32 -10','32 32 46',FALSE,self);
			if ((trace_fraction == 1.0) && (!trace_allsolid)) // Check there is a floor at the new spot
			{
				spot3 = spot2 + (v_up * -4);
				traceline (spot2, spot3 , FALSE, self);

				if (trace_fraction ==1)   // Didn't hit anything?  There was no floor
				{
					trace_fraction = 0;   // So it will loop
				}
				else 
				{
   					makevectors (newangle);
					traceline (spot1, spot2, FALSE, self.enemy);

					if (trace_fraction == 1.0)
					{
						setsize(self, '-24 -24 0', '24 24 64');
						self.hull = 2;
						self.solid = SOLID_SLIDEBOX;
						setorigin(self,spot2);

						if (walkmove(self.angles_y, .05, TRUE))		// You have to move it a little bit to make it solid
							trace_fraction = 1;   // So it will end loop					
						else
							trace_fraction = 0;   // So it will loop					
					}
					else
					{
						trace_fraction = 0;   // So it will loop					
					}
				}
			}
			else
			{
				trace_fraction = 0;
			}
		}
		else
		{
			trace_fraction = 0;
		}

		loop_cnt += 1;

		if (loop_cnt > 500)   // No endless loops
		{
			self.nextthink = time + 2;
			return;
		}

	} while (trace_fraction != 1.0);

	self.think=skullwiz_blinkin1;
	self.nextthink = time;	
	sound (self, CHAN_VOICE, "skullwiz/blinkin.wav", 1, ATTN_NORM);
	CreateRedFlash(self.origin + '0 0 40');
	
}

/*-----------------------------------------
	skullwiz_blinkout - blink out
  -----------------------------------------*/
void skullwiz_blinkout(void) 
{
	thinktime self : HX_FRAME_TIME;
	self.think = skullwiz_blinkout;

	self.scale -= 0.10;

	if ((self.scale > 0.19) && (self.scale < 0.29))
	{
		sound (self, CHAN_BODY, "skullwiz/blinkout.wav", 1, ATTN_NORM);
		CreateRedFlash(self.origin + '0 0 40');
	}

	if (self.scale < 0.10)
	{
		setmodel(self,string_null);
		thinktime self : random(0.5,3);		// Reappear when
		self.think = skullwiz_ininit;
	}
}

/*-----------------------------------------
	skullwiz_blink - assume stance to blink out
  -----------------------------------------*/
void skullwiz_blink(void) [++ $sktele2..$sktele30]
{

	if (self.frame == $sktele2)
	{
		if (self.classname == "monster_skull_wizard")
			sound (self, CHAN_VOICE, "skullwiz/blinkspk.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "skullwiz/blnkspk2.wav", 1, ATTN_NORM);
	}

	if(self.aflag)
	{
		if(self.frame+2<=$sktele30)
			self.frame+=2;
		thinktime self : 0.005;
	}

	if (self.frame == $sktele30)
	{
		self.aflag=FALSE;
		self.takedamage = DAMAGE_NO;  // So t_damage won't force him into another state 
		self.scale = 1;
		self.drawflags = (self.drawflags & SCALE_TYPE_MASKOUT) | SCALE_TYPE_XYONLY;
		self.solid = SOLID_NOT;
		self.th_pain=SUB_Null;
		skullwiz_blinkout();	
	}
}

/*-----------------------------------------
	skullwiz_push - push the enemy away
  -----------------------------------------*/
void skullwiz_push ()
{
	local vector	delta;
	local float 	ldmg;

	if (self.enemy.classname != "player")
		return;

	delta = self.enemy.origin - self.origin;
//if (vlen(delta) > 80)
//		return;

	self.last_attack=time;
	ldmg = random(10);

	T_Damage (self.enemy, self, self, ldmg);
	sound (self, CHAN_VOICE, "skullwiz/push.wav", 1, ATTN_NORM);

	if (self.enemy.flags & FL_ONGROUND)
		self.enemy.flags(-)FL_ONGROUND;

	if (self.classname == "monster_skull_wizard")
	{
		self.enemy.velocity = delta * 10;
		self.enemy.velocity_z = 100;
	}
	else
	{
		self.enemy.velocity = delta * 10;
		self.enemy.velocity_z = 200;
	}
}

/*-----------------------------------------
	skullwiz_melee - push enemy away so you can throw a missile
  -----------------------------------------*/
void skullwiz_melee (void) [++ $skspel2..$skspel30]
{
	vector	delta;
	
	if (self.frame == $skspel20)   // Push enemy away
	{
		if (self.classname == "monster_skull_wizard")
		{
			skullwiz_push();
			if (random() < 0.5)
				skullwiz_missile_init();
		}
		else  // Only the skull wizard lord can summon
		{
			if (random()< 0.15)
				skullwiz_summoninit();
			else
			{
				skullwiz_push();
				if (random() < 0.5)
					skullwiz_missile_init();
			}
		}
	}

	if (cycle_wrapped)
	{
		delta = self.enemy.origin - self.origin;
		if (vlen(delta) > 80)
			skullwiz_run();
	}
	else
		ai_charge(1.3);

}
/*-----------------------------------------
	skullwiz_run - run towards the enemy
  -----------------------------------------*/
void skullwiz_run (void) [++ $skwalk1..$skwalk24]
{
	vector delta;

	if(check_defense_blink()&&(self.health<50||self.classname=="monster_skull_wizard_lord"))
	{
		self.solid = SOLID_NOT;
		self.aflag=TRUE;
		self.think=skullwiz_blink;
		thinktime self : 0;
		return;
	}

	if ((random(1)<.10) && (self.frame == $skwalk1))
	{
		if (self.classname == "monster_skull_wizard")
			sound (self, CHAN_VOICE, "skullwiz/growl.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "skullwiz/growl2.wav", 1, ATTN_NORM);
	}

	delta = self.enemy.origin - self.origin;
	if (vlen(delta) < 80)		// Too close so don't ignore enemy
		self.attack_finished = time - 1; 

	if (self.frame == $skwalk1)  // Decide if he should BLINK away
	{

		if (self.classname == "monster_skull_wizard")
		{
			if (random() < .20)
				skullwiz_blink();
		}
		else	// Skull Wizard BLINKS more often
		{
			if (random() < .30||self.search_time<time + 1)
				skullwiz_blink();
		}
	}
	else
	{
		if (self.attack_finished > time)
			movetogoal(1.3);
		else 	
			ai_run(1.5);
	}
}

/*-----------------------------------------
	skullwiz_walk - walking his beat
  -----------------------------------------*/
void skullwiz_walk (void) [++ $skwalk1..$skwalk24]
{
	if ((random(1)<.05) && (self.frame == $skwalk1))
	{
		if (self.classname == "monster_skull_wizard")
			sound (self, CHAN_VOICE, "skullwiz/growl.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "skullwiz/growl2.wav", 1, ATTN_NORM);
	}

	ai_walk(1.3);
}

/*-----------------------------------------
	skullwiz_stand - standing and waiting
  -----------------------------------------*/
void skullwiz_stand (void) [++ $skwait1..$skwait26]
{
	if (random() < .5)
		ai_stand();
}


void skullwizard_init(void)
{
	if(!self.flags2&FL_SUMMONED)
	{
		precache_model("models/skullwiz.mdl");
		precache_model("models/skulbook.mdl");
		precache_model("models/skulhead.mdl");
		precache_model("models/skulshot.mdl");
		precache_model("models/spider.mdl");

		if (self.classname == "monster_skull_wizard")
		{
			precache_sound("skullwiz/death.wav");
			precache_sound("skullwiz/blinkspk.wav");
			precache_sound("skullwiz/growl.wav");
			precache_sound("skullwiz/scream.wav");
			precache_sound("skullwiz/pain.wav");
//	precache_sound("spider/death.wav");
		}	
		else
		{
			precache_sound("skullwiz/death2.wav");
			precache_sound("skullwiz/blnkspk2.wav");
			precache_sound("skullwiz/growl2.wav");
			precache_sound("skullwiz/scream2.wav");
			precache_sound("skullwiz/gatespk.wav");
			precache_sound("skullwiz/pain2.wav");
		}	

		precache_sound("skullwiz/gate.wav");
		precache_sound("skullwiz/blinkin.wav");
		precache_sound("skullwiz/blinkout.wav");
		precache_sound("skullwiz/push.wav");
		precache_sound("skullwiz/firemisl.wav");

//precache_sound("spider/scuttle.wav");
//precache_sound("spider/bite.wav");
//precache_sound("spider/pain.wav");
//precache_sound("spider/death.wav");

		precache_spider ();

	}

	setmodel(self, "models/skullwiz.mdl");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.mass = 4;
	self.mintel = 5;
	self.thingtype=THINGTYPE_FLESH;

	self.th_stand = skullwiz_stand;
	self.th_walk = skullwiz_walk;
	self.th_run = skullwiz_run;
	self.th_melee = skullwiz_melee;
	self.th_missile = skullwiz_missile_init;
	self.th_pain = skullwiz_pain;
	self.th_die = skullwiz_die;

	setsize(self, '-24 -24 0', '24 24 64');
	self.hull = 2;

	self.flags(+)FL_MONSTER;
	self.yaw_speed = 10;

}


/*QUAKED monster_skull_wizard (1 0.3 0) (-24 -24 0) (24 24 64) AMBUSH
A skull wizard
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void monster_skull_wizard (void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	skullwizard_init();

	self.health = 150;
	self.experience_value = 90;
	self.monsterclass = CLASS_GRUNT;

	walkmonster_start();
}

/*QUAKED monster_skull_wizard_lord (1 0.3 0) (-24 -24 0) (24 24 64) AMBUSH
A skull wizard lord
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void monster_skull_wizard_lord (void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	skullwizard_init();

	self.health = 650;
	self.experience_value = 325;
	self.monsterclass = CLASS_LEADER;
	self.skin = 1;
	self.scale = 1.20;
	walkmonster_start();


}

