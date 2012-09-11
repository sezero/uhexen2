/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/archer.hc,v 1.2 2007-02-07 16:56:55 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\archerK\final\archer.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\archerK\final
$origin 0 0 0
$base base skin
$skin skin
$flags 0


//
$frame backup1      backup2      backup3      backup4      backup5      
$frame backup6      backup7      backup8      

//
$frame deathA1      deathA2      deathA3      deathA4      deathA5      
$frame deathA6      deathA7      deathA8      deathA9      deathA10     
$frame deathA11     deathA12     deathA13     deathA14     deathA15     
$frame deathA16     deathA17     deathA18     deathA19     deathA20     
$frame deathA21     deathA22     

//
$frame draw1        draw2        draw3        draw4        draw5        
$frame draw6        draw7        draw8        draw9        draw10       
$frame draw11       draw12       draw13       

//
$frame duck1        duck2        duck3        duck4        duck5        
$frame duck6        duck7        duck8        duck9        duck10       
$frame duck11       duck12       duck13       duck14       

//
$frame fire1        fire2        fire3        fire4        

//
$frame pain1        pain2        pain3        pain4        pain5        
$frame pain6        pain7        pain8        pain9        pain10       
$frame pain11       pain12       pain13       pain14       pain15       
$frame pain16       

//
$frame patrol1      patrol2      patrol3      patrol4      patrol5      
$frame patrol6      patrol7      patrol8      patrol9      patrol10     
$frame patrol11     patrol12     patrol13     patrol14     patrol15     
$frame patrol16     patrol17     patrol18     patrol19     patrol20     
$frame patrol21     patrol22     

//
$frame redraw1      redraw2      redraw3      redraw4      redraw5      
$frame redraw6      redraw7      redraw8      redraw9      redraw10     
$frame redraw11     redraw12     

//
$frame tranA1       tranA2       tranA3       tranA4       tranA5       
$frame tranA6       tranA7       tranA8       tranA9       tranA10      
$frame tranA11      tranA12      tranA13      

//
$frame tranB1       tranB2       tranB3       tranB4       tranB5       
$frame tranB6       tranB7       tranB8       tranB9       tranB10      
$frame tranB11      tranB12      tranB13      tranB14      tranB15      
$frame tranB16      

//
$frame tranC1       tranC2       tranC3       tranC4       tranC5       
$frame tranC6       tranC7       tranC8       tranC9       tranC10      

//
$frame tranD1       tranD2       tranD3       tranD4       tranD5       
$frame tranD6       tranD7       tranD8       tranD9       

//
$frame waitA1       waitA2       waitA3       waitA4       waitA5       
$frame waitA6       waitA7       waitA8       waitA9       waitA10      
$frame waitA11      waitA12      waitA13      waitA14      waitA15      
$frame waitA16      waitA17      waitA18      

//
$frame waitB1       waitB2       waitB3       waitB4       waitB5       
$frame waitB6       waitB7       waitB8       waitB9       waitB10      
$frame waitB11      waitB12      

//
$frame walk1        walk2        walk3        walk4        walk5        
$frame walk6        walk7        walk8        walk9        walk10       
$frame walk11       walk12       walk13       walk14       walk15       
$frame walk16       


float ARCHER_STUCK = 2;	// Archer can't run

float GREEN_ARROW = 0;
float RED_ARROW = 1;
float GOLD_ARROW = 2;



void archer_run(void);
void archer_stand(void);
void archerredraw(void);
void archerdraw(void);
void()archerdrawhold;
void archermissile(void);

void()archer_check_defense;

float archer_check_shot(void)
{
	vector spot1,spot2;

	makevectors(self.angles);	
	// see if any entities are in the way of the shot
	spot1 = self.origin + v_right * 10 + v_up * 36;
	spot2 = self.enemy.origin + self.enemy.view_ofs;

	traceline (spot1, spot2, FALSE, self);

	if (trace_ent.thingtype >= THINGTYPE_WEBS)
		traceline(trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != self.enemy)
	{
		if ((trace_ent.thingtype!=THINGTYPE_GLASS) || !trace_ent.takedamage || 
			(trace_ent.flags & FL_MONSTER && trace_ent.classname!="player_sheep"))
		{
			return FALSE;			
		}
	}
	return TRUE;
}

void archer_duck () [++ $duck1..$duck14]
{
	ai_face();
	if(cycle_wrapped)
	{
		setsize(self,'-16 -16 0','16 16 56');
		if(infront(self.enemy))
			self.think=archermissile;
		else
			self.think=archerdrawhold;
		thinktime self : 0;
		return;
	}
	else if (self.frame==$duck1)
		if (self.classname == "monster_archer")
			sound (self, CHAN_VOICE, "archer/growl.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "archer/pain2.wav", 1, ATTN_NORM);
	else if (self.frame == $duck5)	
			setsize(self,'-16 -16 0','16 16 28');
	else if (self.frame==$duck7)
	{
		archer_check_defense();
		thinktime self :0.2;
	}
}

void archer_check_defense()
{
	entity enemy_proj;

	if(random(2)>skill/10+self.skin/2)
		return;

	if (self.enemy.last_attack+0.5<time&&self.oldenemy.last_attack+0.5<time)
		return;

	enemy_proj=look_projectiles();

	if(!enemy_proj)
		if(lineofsight(self,self.enemy))
		{
			enemy_proj=self.enemy;
			self.level=vlen(self.enemy.origin-self.origin)/1000;
		}
		else
			return;

	if(mezzo_check_duck(enemy_proj))
	{
		if(self.think==archer_duck)
			self.frame=$duck6;
		else
		{
			self.think=archer_duck;
			thinktime self : 0;
		}
		return;
	}
}

void archer_arrow_touch(void)
{
	float damg;
	vector delta;

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if (self.owner.classname == "monster_archer")
		damg = random(5,7);
	else
	{
		if (self.classname == "red_arrow")
			damg = random(8,12);
		else	// the gold arrow
			damg = random(13,17);
	}

	if ((other.classname == "player") && (self.classname == "gold_arrow"))
	{
			delta = other.origin - self.origin;
			other.velocity = delta * 10;
			if (other.flags & FL_ONGROUND)
				other.flags (-) FL_ONGROUND;
			other.velocity_z = 150;
	}

	T_Damage (other, self, self.owner, damg );

	self.origin = self.origin - 8 * normalize(self.velocity) - '0 0 40';
	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	if (self.classname == "gold_arrow")
		CreateSpark (self.origin); 
	else if (self.classname == "red_arrow")
		CreateRedSpark (self.origin); 
	else
		CreateGreenSpark (self.origin); 

	remove(self);

}


void archer_dying (void) [++ $deathA1..$deathA22]
{
	sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);
	if (self.health < -80)
	{
		chunk_death();
		remove(self);
	}

	if (cycle_wrapped)
	{
		self.frame = $deathA22;
		MakeSolidCorpse();
	}
}

void() archer_die =
{
	// check for gib
	if (self.health < -30)
	{
		chunk_death();
		return;
	}
	else
	{
		if (self.classname == "monster_archer")
			sound (self, CHAN_VOICE, "archer/death.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "archer/death2.wav", 1, ATTN_NORM);
	}

	archer_dying();
};

/*-----------------------------------------
	archer_pain - flinch in pain
  -----------------------------------------*/
void archer_pain_anim () [++ $pain1 .. $pain16]
{
	if (self.frame == $pain2)
	{
		if (self.classname == "monster_archer")
			sound (self, CHAN_VOICE, "archer/pain.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "archer/pain2.wav", 1, ATTN_NORM);
	}

	if (self.frame==$pain16)
	{
		self.pain_finished = time + random(4,6);
		archerredraw();
	}
}

void archer_pain (entity attacker, float damg)
{
	if ((self.frame >= $pain1) && (self.frame <= $pain16)) // Still going through pain anims
		return;

	if (self.attack_state == AS_MISSILE) 
		return;

	// Hurt him, will he flinch or fight back?
	setsize(self,'-16 -16 0','16 16 56');
	if ((self.pain_finished > time) ||  (random() < .5))
	{
		archerredraw();
	}
	else
		archer_pain_anim();
}

void archer_launcharrow (float arrowtype,vector spot1,vector spot2)
{
	self.last_attack=time;

	makevectors(self.angles);	
	if (arrowtype==GREEN_ARROW)
	{
		sound (self, CHAN_WEAPON, "archer/arrowg.wav", 1, ATTN_NORM);
		Create_Missile(self,spot1,spot2, "models/akarrow.mdl","green_arrow",0,1000,archer_arrow_touch);
	}
	else if (arrowtype==RED_ARROW)
	{
		sound (self, CHAN_WEAPON, "archer/arrowr.wav", 1, ATTN_NORM);
		CreateRedFlash(spot1);
		Create_Missile(self,spot1,spot2,"models/akarrow.mdl","red_arrow",1,1000,archer_arrow_touch);
	}
	else
	{
		sound (self, CHAN_WEAPON, "archer/arrowg.wav", 1, ATTN_NORM);
		CreateRedFlash(spot1);
		Create_Missile(self,spot1,spot2,"models/akarrow.mdl","gold_arrow",2,1000,archer_arrow_touch);
	}
	newmis.drawflags(+)MLS_ABSLIGHT;
	newmis.abslight=0.5;
	thinktime newmis : 2.5;
}
/*-----------------------------------------
	archerdrawdone - drawing his bow to fire
  -----------------------------------------*/
void archerdrawdone () [-- $tranA7..$tranA1]
{
	archer_check_defense();

	walkmove(self.angles_y,0.5,FALSE);

	if(visible(self.enemy))
		ai_face();

	if(cycle_wrapped)
	{
		self.pausetime = time + random(.5,2);
		self.attack_state = AS_WAIT;

		archer_run();
	}
}

/*-----------------------------------------
	archermissile - fire his arrow
  -----------------------------------------*/
void archermissile () [++ $fire1..$fire4]
{
float enemy_range,chance,ok,tspeed;
vector spot1, spot2;

	self.attack_state = AS_MISSILE;

	if(visible(self.enemy))
		ai_face();
	else
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}

	if (self.frame == $fire2)	// FIRE!!!!
	{
		makevectors(self.angles);	
		spot1 = self.origin + v_forward*4 + v_right * 10 + v_up * 36;
		if(self.classname=="monster_archer_lord")
		{
			tspeed=vlen(self.enemy.velocity);
			if(tspeed>100)
				spot2=extrapolate_pos_for_speed(spot1,1000,self.enemy,0.3);
		}

		if(spot2=='0 0 0')
		{
			spot2 = self.enemy.origin + self.enemy.view_ofs;
			traceline (spot1, spot2, FALSE, self);

			if(trace_ent.thingtype>=THINGTYPE_WEBS)
				traceline (trace_endpos, spot2, FALSE, trace_ent);

			if (trace_ent == self.enemy) 
				ok=TRUE;
			else if((trace_ent.health<=25||trace_ent.thingtype>=THINGTYPE_WEBS)&&trace_ent.takedamage&&(!trace_ent.flags&FL_MONSTER||trace_ent.classname=="player_sheep"))
				ok=TRUE;
		}
		else
			ok=TRUE;

		if(ok)
		{	
			enemy_range = range(self.enemy);
			if (enemy_range < RANGE_MELEE)   // Which arrow to use?
				chance = 0.80;
			else if (enemy_range < RANGE_NEAR)
				chance = 0.50;
			else if (enemy_range < RANGE_MID)
				chance = 0.30;
			else if (enemy_range < RANGE_FAR)
				chance = 0.10;

			if (self.classname=="monster_archer")
			{
				if (random(1) < chance)			
					archer_launcharrow(RED_ARROW,spot1,spot2);
				else
					archer_launcharrow(GREEN_ARROW,spot1,spot2);
			}
			else // Archer Lord
			{
				if (random(1) < chance)			
					archer_launcharrow(GOLD_ARROW,spot1,spot2);
				else
					archer_launcharrow(RED_ARROW,spot1,spot2);
			}
			self.attack_finished = time + random(.5,1);
			if(!self.skin)
				chance-=0.3;
			chance+=skill/10;

			// Reattack?
			if (random () > chance)	// Is he done?
				archerdrawdone();
		}
		else
		{
			self.attack_finished = time + random();
			sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);
			archer_run();
			return;
		}
	}
	if(cycle_wrapped)
	{
		self.frame = $draw10 ;
		archerredraw();
	}
	else if(visible(self.enemy))
		ai_face();
}



/*-----------------------------------------
	archerredraw - redrawing his bow to fire
  -----------------------------------------*/
void archerredraw () [++ $redraw1..$redraw12]
{
	self.attack_state = AS_MISSILE;

	archer_check_defense();

	if (self.frame == $redraw8)
		sound (self, CHAN_WEAPON, "archer/draw.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		archermissile();
	else if(visible(self.enemy))
		ai_face();
	else
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}
}

/*-----------------------------------------
	archerdrawhold - waiting for right chance to attack
  -----------------------------------------*/
void archerdrawhold ()
{
float chance,startframe,endframe;

	if(visible(self.enemy))
		ai_face();
	else
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}
	
	archer_check_defense();

	startframe=$waitB1;
	endframe=$waitB12;

	if (!self.spawnflags & ARCHER_STUCK)
	{
		if (vlen(self.enemy.origin - self.origin)<=200)
		{
			if(infront(self.enemy))
				if(walkmove(self.angles_y+180,3,FALSE))
				{
					startframe=$backup1;
					endframe=$backup8;
				}
		}
	}

	AdvanceFrame(startframe,endframe);

	self.think=archerdrawhold;
	thinktime self : 0.05;

	enemy_range = range(self.enemy);
	if (enemy_range < RANGE_NEAR && random() < 0.4)
		archermissile();
	else if (cycle_wrapped||random()<skill/20+self.skin/10)
	{
		if (!archer_check_shot())
		{
			sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);
			self.attack_state = AS_STRAIGHT;					
			self.think = archer_run;
			thinktime self : HX_FRAME_TIME;
			return;
		}

		if (self.classname == "monster_archer")   // Monster archer's not so smart
		{
			if (enemy_range <= RANGE_MELEE)
			chance = 1;
			else if (enemy_range <= RANGE_NEAR)
				chance = 0.70;
			else if (enemy_range <= RANGE_MID)
				chance = 0.60;
			else if (enemy_range <= RANGE_FAR)
				chance = 0.50;
		}
		else
			chance = 1;

		if (random() < chance)
			archermissile();
	}
}

/*-----------------------------------------
	archerdraw - drawing his bow to fire
  -----------------------------------------*/
void archerdraw () [++ $tranA1..$tranA13]
{

	archer_check_defense();

	if (self.frame == $tranA1)	// See if he should even try to shoot
	{
		if (!archer_check_shot())
		{
			self.attack_state = AS_STRAIGHT;					
			self.think = archer_run;
			thinktime self : HX_FRAME_TIME;
			return;
		}
	}

	if (self.frame == $tranA6)
		sound (self, CHAN_WEAPON, "archer/draw.wav", 1, ATTN_NORM);

	if ((self.frame >= $tranA1) && (self.frame <= $tranA13))
		walkmove(self.angles_y+180,.28,FALSE);

	if (cycle_wrapped)  // Archer has drawn the arrow
		archerdrawhold();
	else if (visible(self.enemy))
		ai_face();
	else
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}

	if ((random(1)<.10) && (self.frame == $walk1))
	{
		if (self.classname == "monster_archer")
			sound (self, CHAN_BODY, "archer/growl.wav", 1, ATTN_NORM);
		else
		{
			if (random() < .70)	
				sound (self, CHAN_BODY, "archer/growl2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_BODY, "archer/growl3.wav", 1, ATTN_NORM);
		}
	}
}


/*-----------------------------------------
	archer_run - run towards the enemy
  -----------------------------------------*/
void archer_run(void) 
{
	self.think = archer_run;
	thinktime self : HX_FRAME_TIME;

	archer_check_defense();

	if ((random(1)<.10) && (self.frame == $walk1))
	{
		if (self.classname == "monster_archer")
			sound (self, CHAN_VOICE, "archer/growl.wav", 1, ATTN_NORM);
		else
		{
			if (random() < .70)	
				sound (self, CHAN_VOICE, "archer/growl2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "archer/growl3.wav", 1, ATTN_NORM);
		}
	}

	if  ((self.spawnflags & ARCHER_STUCK) || (self.attack_state == AS_WAIT))
	{
		AdvanceFrame($waitA1,$waitA18);
		ai_run(0);
	}
	else
	{
		AdvanceFrame($walk1,$walk16);
		ai_run(4);
	}
}


/*-----------------------------------------
	archer_walk - walking his beat
  -----------------------------------------*/
void archer_walk(void) [++ $patrol1..$patrol22]
{
	thinktime self : HX_FRAME_TIME + .01;	// Make him move a little slower so his run will look faster

	archer_check_defense();

	if ((random()<.05) && (self.frame == $patrol1))
	{
		if (self.classname == "monster_archer")
			sound (self, CHAN_VOICE, "archer/growl.wav", 1, ATTN_NORM);
		else
		{
			if (random() < .70)	
				sound (self, CHAN_VOICE, "archer/growl2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "archer/growl3.wav", 1, ATTN_NORM);
		}
	}	

	if  (self.spawnflags & ARCHER_STUCK)	// No moving
	{
		self.frame = $patrol1;				// FIXME: this should be the wait animations
		ai_walk(0);
	}
	else
		ai_walk(2.3);

}

/*-----------------------------------------
	archer_stand - standing and waiting
  -----------------------------------------*/
void archer_stand(void) [++ $waitA1..$waitA18]
{
	if (random()<0.5)
	{
		
		archer_check_defense();
		ai_stand();
	}
}


/*QUAKED monster_archer (1 0.3 0) (-16 -16 0) (16 16 50) AMBUSH STUCK JUMP PLAY_DEAD DORMANT
The Archer Knight monster
-------------------------FIELDS-------------------------
Health : 80
Experience Pts: 25
Favorite TV shows: Friends & Baywatch
Favorite Color: Blue
Likes: Shooting arrows into people and long walks along the beach
Dislikes: Anything having to do with Pauly Shore
--------------------------------------------------------
*/
void monster_archer ()
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	if (!self.flags2 & FL_SUMMONED)
	{
		precache_archer();
	}

	CreateEntityNew(self,ENT_ARCHER,"models/archer.mdl",archer_die);

	self.th_stand = archer_stand;
	self.th_walk = archer_walk;
	self.th_run = archer_run;
	self.th_melee = archerdraw;
	self.th_missile = archerdraw;
	self.th_pain = archer_pain;
	self.decap = 0;
	self.headmodel = "models/archerhd.mdl";
	self.mintel = 7;
	self.monsterclass = CLASS_GRUNT;
	self.experience_value = 25;
	self.health = 80;

	self.flags (+) FL_MONSTER;
	self.yaw_speed = 10;
	self.view_ofs = '0 0 40';

	walkmonster_start();
}

/*QUAKED monster_archer_lord (1 0.3 0) (-16 -16 0) (16 16 50) AMBUSH STUCK JUMP PLAY_DEAD DORMANT
The Archer Lord monster
-------------------------FIELDS-------------------------
Health : 325
Experience Pts: 200
Favorite Cities: Madrid & Las Vegas
Favorite Flower: Orchid
What people don't know about me: I cry at sad movies
What people say when they see me: Don't shoot!! Don't shoot!!
--------------------------------------------------------
*/
void monster_archer_lord ()
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	if (!self.flags2 & FL_SUMMONED)
	{
		precache_model("models/archer.mdl");
		precache_model("models/archerhd.mdl");

		precache_model("models/gspark.spr");

		precache_sound ("archer/arrowg.wav");
		precache_sound ("archer/arrowr.wav");

		precache_sound ("archer/growl2.wav");
		precache_sound ("archer/growl3.wav");
		precache_sound ("archer/pain2.wav");
		precache_sound ("archer/sight2.wav");
		precache_sound ("archer/death2.wav");
		precache_sound ("archer/draw.wav");

	}

	CreateEntityNew(self,ENT_ARCHER,"models/archer.mdl",archer_die);

	self.th_stand = archer_stand;
	self.th_walk = archer_walk;
	self.th_run = archer_run;
	self.th_melee = archerdraw;
	self.th_missile = archerdraw;
	self.th_pain = archer_pain;
	self.decap = 0;
	self.headmodel = "models/archerhd.mdl";
	self.mintel = 7;
	self.monsterclass = CLASS_HENCHMAN;
	self.experience_value = 200;
	self.health = 325;
	self.skin = 1;

	self.flags (+) FL_MONSTER;
	self.yaw_speed = 10;
	self.view_ofs = '0 0 40';

	walkmonster_start();
}

/*
=================
ArcherCheckAttack
=================
*/
float ArcherCheckAttack (void)
{
	local vector	spot1, spot2;	
	local entity	targ;
	local float		chance;

//	if (enemy_range <= RANGE_MELEE)		// Enemy is too close...attack
//	{
//		self.attack_state = AS_MISSILE;
		self.pain_finished = 0;
//		self.attack_finished = 0;
//	}

	if (self.attack_finished > time)
		return FALSE;

	if (!enemy_vis)
		return FALSE;

	if (enemy_range == RANGE_FAR)
	{
		if (self.attack_state != AS_STRAIGHT)
		{
			self.attack_state = AS_STRAIGHT;
			archer_run ();
		}
		return FALSE;
	}
		
	targ = self.enemy;

	makevectors(self.angles);	
// see if any entities are in the way of the shot
	spot1 = self.origin + v_right * 10 + v_up * 36;
	spot2 = targ.origin + targ.view_ofs;

	traceline (spot1, spot2, FALSE, self);

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != targ)
		if((trace_ent.health>25&&trace_ent.thingtype!=THINGTYPE_GLASS)||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
		{	
			self.attack_state = AS_STRAIGHT;					
			return FALSE;
		}
	
	// Chances of attack
	//     50% at MID range
	//     65% at NEAR range
	//     80% at MELEE range		
	enemy_range = range(self.enemy);
	if (enemy_range == RANGE_MELEE)
		chance = 0.40;
	else if (enemy_range == RANGE_NEAR)
		chance = 0.3;
	else if (enemy_range == RANGE_MID)
		chance = 0.2;
	else
		chance = 0;

	if ((random () < chance) && (self.attack_state != AS_MISSILE))	// Will he attack?
	{
		self.attack_state = AS_MISSILE;
		return TRUE;
	}

	if (enemy_range == RANGE_MID)
	{
		if (random (1) < .5)	// Will he side step?
			self.attack_state = AS_SLIDING;
		else
			self.attack_state = AS_STRAIGHT;			
	}
	else
	{
		if (self.attack_state != AS_SLIDING)
			self.attack_state = AS_SLIDING;
	}
	
	return FALSE;
}

