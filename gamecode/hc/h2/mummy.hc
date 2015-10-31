/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/mummy.hc,v 1.2 2007-02-07 16:57:08 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\mummy\mummy.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\mummy
$origin 0 0 0
$base BASE-F skin
$skin skin
$flags 0


// DEATH : FRAMES 1 - 13, no arms, one leg
$frame mdeath1      mdeath2      mdeath3      mdeath4      mdeath5      
$frame mdeath6      mdeath7      mdeath8      mdeath9      mdeath10     
$frame mdeath11     mdeath12     mdeath13     

// STAFF ATTACK : FRAMES 14 - 37,  both arms, both legs
$frame mstafA1      mstafA2      mstafA3      mstafA4      mstafA5      
$frame mstafA6      mstafA7      mstafA8      mstafA9      mstafA10     
$frame mstafA11     mstafA12     mstafA13     mstafA14     mstafA15     
$frame mstafA16     mstafA17     mstafA18     mstafA19     mstafA20     
$frame mstafA21     mstafA22     mstafA23     mstafA24     

// MELEE SWING : FRAMES 38 - 47, no arms, both legs, swings his stump at enemy
$frame mswingC1     mswingC2     mswingC3     mswingC4     mswingC5     
$frame mswingC6     mswingC7     mswingC8     mswingC9     mswingC10    

// MELEE SWING : FRAMES 48 - 65, both arms and legs, swings his staff at enemy
$frame mswngA1      mswngA2      mswngA3      mswngA4      mswngA5      
$frame mswngA6      mswngA7      mswngA8      mswngA9      mswngA10     
$frame mswngA11     mswngA12     mswngA13     mswngA14     mswngA15     
$frame mswngA16     mswngA17     mswngA18     

// MELEE SWING : FRAMES 66 - 83, one arm, both legs, swings his staff at enemy
$frame mswngB1      mswngB2      mswngB3      mswngB4      mswngB5      
$frame mswngB6      mswngB7      mswngB8      mswngB9      mswngB10     
$frame mswngB11     mswngB12     mswngB13     mswngB14     mswngB15     
$frame mswngB16     mswngB17     mswngB18     

// WALKING: frames 84 - 97, both arms and both legs
$frame mwalkA1      mwalkA2      mwalkA3      mwalkA4      mwalkA5      
$frame mwalkA6      mwalkA7      mwalkA8      mwalkA9      mwalkA10     
$frame mwalkA11     mwalkA12     mwalkA13     mwalkA14

// WALKING: frames 98 - 113, one arm and both legs
$frame mwalkB1      mwalkB2      mwalkB3      mwalkB4      mwalkB5      
$frame mwalkB6      mwalkB7      mwalkB8      mwalkB9      mwalkB10     
$frame mwalkB11     mwalkB12     mwalkB13     mwalkB14     mwalkB15
$frame mwalkB16

// WALKING: frames 114 - 131, no arms, both legs 
$frame mwalkC1      mwalkC2      mwalkC3      mwalkC4      mwalkC5      
$frame mwalkC6      mwalkC7      mwalkC8      mwalkC9      mwalkC10     
$frame mwalkC11     mwalkC12     mwalkC13     mwalkC14     mwalkC15     
$frame mwalkC16     mwalkC17     mwalkC18

// 132 - 147
$frame shoota1      shoota2      shoota3      shoota4      shoota5      
$frame shoota6      shoota7      shoota8      shoota9      shoota10     
$frame shoota11     shoota12     shoota13     shoota14     shoota15     
$frame shoota16     

//
$frame shootb1      shootb2      shootb3      shootb4      shootb5      
$frame shootb6      shootb7      shootb8      shootb9      shootb10     
$frame shootb11     shootb12     shootb13     shootb14     shootb15     
$frame shootb16     

//
$frame staffb1      staffb2      staffb3      staffb4      staffb5      
$frame staffb6      staffb7      staffb8      staffb9      staffb10     
$frame staffb11     staffb12     staffb13     staffb14     staffb15     
$frame staffb16     staffb17     staffb18     staffb19     staffb20     
$frame staffb21     staffb22     staffb23     staffb24     staffb25     
$frame staffb26     staffb27     

//
$frame crawl1       crawl2       crawl3       crawl4       crawl5       
$frame crawl6       crawl7       crawl8       crawl9       crawl10      
$frame crawl11      crawl12      crawl13      crawl14      


// What parts are gone from the mummy
float MUMMY_NONE		= 0;	// None 
float MUMMY_LARM		= 1;	// Left arm is gone
float MUMMY_RARM		= 2;	// Left and Right arm are gone
float MUMMY_LEG			= 3;	// Left and Right arm are gone,Left leg is gone

float MUMMY_WAVER		= 1;
float MUMMY_DOWN 		= 2;

void mummyrun(void);
void mummywalk(void);
void mummymelee(void);

/*
============
pmissile_touch - missile1 hit something. Hurt it
============
*/
void mummissile_touch (void)
{
	float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		pmissile_gone();
		return;
	}

	damg = random(5,10);
	if (other.health)
		T_Damage (other, self, self.owner, damg );

	sound (self, CHAN_BODY, "weapons/expsmall.wav", 1, ATTN_NORM);
	self.origin = self.origin - 8*normalize(self.velocity);

	CreateFireCircle(self.origin - (v_forward * 8));

	self.effects = EF_NODRAW;
	self.solid = SOLID_NOT;
//	self.nextthink = time + .5;  // So explosion sound can finish out
	thinktime self : .5;

	self.think = pmissile_gone;
}


void mflame1_runup (void) [++ 0 .. 16 ]
{
	if (cycle_wrapped)
		if(self.cnt)
		{
			self.cnt-=1;
			particle2(self.origin+'0 0 17','0 0 25','0 0 25',168,7,5);
		}
		else
			remove(self);
}

void mflame2_runup (void) [++ 17 .. 33 ]
{
	if (cycle_wrapped)
		if(self.cnt)
		{
			self.cnt-=1;
			particle2(self.origin+'0 0 17','0 0 25','0 0 25',168,7,5);
		}
		else
			remove(self);
}

void mflame3_runup (void) [++ 34 .. 50 ]
{
	if (cycle_wrapped)
		if(self.cnt)
		{
			self.cnt-=1;
			particle2(self.origin+'0 0 17','0 0 25','0 0 25',168,7,5);
		}
		else
			remove(self);
}

void mflame_burn(void)
{
	float damg;

	if ((other.health) && (other != self.owner) && (self.pain_finished<time))
	{
		damg = self.dmg + random() * self.dmg;
		T_Damage (other, self, self.owner, damg);
		self.pain_finished = time + .05;
		if(self.t_width<time)
		{
			sound(self,CHAN_BODY,"crusader/sunhit.wav",1,ATTN_NORM);
			self.t_width=time+0.1;
		}
	}
}


void SpawnMummyFlame(void)
{
	entity new;
	float chance;

	traceline(self.origin,self.origin - '0 0 600',TRUE,self);

	if (trace_fraction==1)
		return;

	if(pointcontents(trace_endpos)==CONTENT_WATER)
	{
		remove(self);
		return;
	}

	new = spawn();

	CreateEntityNew(new,ENT_MUMMY_FIRE,"models/mumshot.mdl",SUB_Null);

	setorigin(new, trace_endpos);
	new.owner = self.owner;
	new.pain_finished = 0;
	new.drawflags=MLS_ABSLIGHT;
	new.abslight=0.5;
	new.angles = self.angles;
	new.dmg=3;//was 5, too much!

	chance = random();
	if (chance < .33)
		new.think = mflame1_runup;
	else if (chance < .66)
		new.think = mflame2_runup;
	else
		new.think = mflame3_runup;

//ew.nextthink = time + HX_FRAME_TIME;
	thinktime new : HX_FRAME_TIME;

	new.touch = mflame_burn;

	if(self.classname=="circfire")
	{
		new.angles = self.angles +'0 -90 0';
		new.scale=2.5;
		new.cnt=5;
		particle2(new.origin+'0 0 17','0 0 25','0 0 25',168,7,5);
	}
	else if (self.lifetime < time)
	{
		remove(self);
	}
	else
	{
		particle2(new.origin,'0 0 25','0 0 25',168,7,5);

	//	self.nextthink = time + .04;
		thinktime self : .04;

		self.think = SpawnMummyFlame;
	}
}

void mumshot_gone(void)
{
	float damg;

	if (other.health)
	{
		damg = 15 + random() * 10;
		T_Damage (other, self, self.owner, damg);
	}

	remove(self);
}

void launch_mumshot ()
{
	local vector diff;

	self.last_attack=time;

	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;

	setmodel (newmis,"models/mumshot.mdl");
	setsize (newmis, '0 0 0', '0 0 0');
	makevectors (self.angles);
	setorigin (newmis, self.origin + v_forward*14 + v_right * 15 + v_up * 50);
	diff = (self.enemy.origin + self.enemy.view_ofs) - newmis.origin ;
	newmis.velocity = normalize(diff);
	newmis.velocity = newmis.velocity * 400;
	newmis.classname = "mumshot";
	newmis.angles = vectoangles(newmis.velocity);

	newmis.touch = mumshot_gone;

	CreateRedFlash(self.origin + v_forward*-14 + v_right * 15 + v_up * 50);

	newmis.effects = EF_NODRAW;
	sound (self, CHAN_WEAPON, "mummy/mislfire.wav", 1, ATTN_NORM);

	newmis.lifetime = time + 2.5;

// set missile duration
//	newmis.nextthink = time + .04;
	thinktime newmis : .04;

	newmis.think = SpawnMummyFlame;

}

void launch_mumshot2 (void)
{
	local vector diff;

	self.last_attack=time;
	newmis = spawn ();

	CreateEntityNew(newmis,ENT_MUMMY_MISSILE,"models/mumshot.mdl",SUB_Null);  // This is invisible

	newmis.owner = self;
	setorigin (newmis, self.origin + v_forward*-14 + v_right * -15 + v_up * 50);

	sound (self, CHAN_WEAPON, "mummy/mislfire.wav", 1, ATTN_NORM);

	// set missile speed
	makevectors (self.angles);
	diff = (self.enemy.origin + '0 0 20') - newmis.origin ;
	newmis.velocity = normalize(diff);
	newmis.velocity = newmis.velocity * 600;
	newmis.classname = "mummymissile";
	newmis.avelocity = '0 0 1000';

	newmis.angles = vectoangles(newmis.velocity);
	newmis.touch = T_MissileTouch;

// set missile duration
//	newmis.nextthink = time + 2.5;
	thinktime newmis : 2.5;

	newmis.think = SUB_Remove;
}

void mummy_die()
{
	if (self.classname == "monster_mummy_lord")
		sound (self, CHAN_VOICE, "mummy/die2.wav", 1, ATTN_NORM);
	else
		sound (self, CHAN_VOICE, "mummy/die.wav", 1, ATTN_NORM);

	chunk_death ();

//	self.nextthink = time + 0.01;
	thinktime self : .01;

	self.think=SUB_Remove;
}


void mummy_throw_rightleg()
{
	entity new;
	new = spawn();

	CreateEntityNew(new,ENT_MUMMY_LEG,"models/leg.mdl",SUB_Null);

	makevectors(self.angles);
	new.origin = self.origin + v_right * 10 + v_up * 60;
	new.velocity = VelocityForDamage (40);
	new.avelocity_x = random()*600;
	new.avelocity_y = random()*600;
	new.avelocity_z = random()*600;

	self.parts_gone = MUMMY_LEG;   // So he's missing a leg
	self.mummy_state = MUMMY_WAVER;

	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_PHASE;
	new.flags(-)FL_ONGROUND;
	new.think=MakeSolidCorpse;
//	new.nextthink = time + HX_FRAME_TIME * 10;
	thinktime new : HX_FRAME_TIME * 10;

//	bloodspew_create();
}


void mummy_throw_rightarm()
{
	entity new;
	new = spawn();

	CreateEntityNew(new,ENT_MUMMY_ARM,"models/larm.mdl",SUB_Null);
	makevectors(self.angles);
	new.origin = self.origin + v_right * 10 + v_up * 60;
	new.velocity = VelocityForDamage (40);
	new.avelocity_x = random()*600;
	new.avelocity_y = random()*600;
	new.avelocity_z = random()*600;

	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_PHASE;
	new.flags(-)FL_ONGROUND;

	new.think=MakeSolidCorpse;
	thinktime new : HX_FRAME_TIME * 10;

	self.parts_gone = MUMMY_RARM;   // So he's missing his right arm
//	bloodspew_create('0 10 40');
}


void mummy_throw_leftarm()
{
	entity new;
	new = spawn();

	CreateEntityNew(new,ENT_MUMMY_ARM,"models/larm.mdl",SUB_Null);

	makevectors(self.angles);
	new.origin = self.origin - v_right * 10 + v_up * 60;
	new.velocity = VelocityForDamage (40);
	new.avelocity_x = random()*600;
	new.avelocity_y = random()*600;
	new.avelocity_z = random()*600;

	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_PHASE;
	new.flags(-)FL_ONGROUND;

	self.parts_gone = MUMMY_LARM;   // So he's missing an arm
	new.think=MakeSolidCorpse;
	thinktime new : HX_FRAME_TIME * 10;

//	bloodspew_create('0 -10 40');
}

void mummy_pain(void)
{
	float hold_parts;

	if (self.pain_finished > time)
		return;

	if (self.classname == "monster_mummy_lord")
		sound (self, CHAN_VOICE, "mummy/pain2.wav", 1, ATTN_NORM);
	else
		sound (self, CHAN_VOICE, "mummy/pain.wav", 1, ATTN_NORM);

	hold_parts = self.parts_gone;

	if (self.health < 30)
	{
		if (self.parts_gone <= MUMMY_NONE)
			mummy_throw_leftarm();

		if (self.parts_gone <= MUMMY_LARM)
			mummy_throw_rightarm();

		if (self.parts_gone <= MUMMY_RARM)
			mummy_throw_rightleg();
	}
	else if (self.health < 60)
	{
		if (self.parts_gone <= MUMMY_NONE)
			mummy_throw_leftarm();

		if (self.parts_gone <= MUMMY_LARM)
			mummy_throw_rightarm();
	}
	else if (self.health < 100)
	{
		if (self.parts_gone == MUMMY_NONE)
			mummy_throw_leftarm();
	}

	if (hold_parts != self.parts_gone)
		sound (self, CHAN_BODY, "mummy/limbloss.wav", 1, ATTN_NORM);
}


void lordmummymissile (void)
{
	float result;
	vector delta;

	thinktime self : HX_FRAME_TIME;
	self.think = lordmummymissile;

	delta = self.enemy.origin - self.origin;
	if (vlen(delta) < 70)  // Too close to shoot with a missile
		mummymelee();

	if (self.parts_gone == MUMMY_NONE)
	{
		result = AdvanceFrame($mstafA1,$mstafA24);

		if (self.frame == $mstafA10)
			sound (self, CHAN_WEAPON, "mummy/tap.wav", 1, ATTN_NORM);

		if (self.frame == $mstafA17)
			launch_mumshot();

		if (result == AF_END)
			mummyrun();
		else
			ai_face();
	}
	else if (self.parts_gone == MUMMY_LARM)
	{
		result=AdvanceFrame($mwalkB1,$mwalkB16);  // Because there is no one 
		if (self.frame == $mwalkB16)
			launch_mumshot();

		if (result == AF_END)
			mummyrun();
		else
			ai_face();
	}
	else
		mummyrun();

}

void mummymissile (void)
{
	float result,chance;
	vector delta;

	thinktime self : HX_FRAME_TIME;
	self.think = mummymissile;

	delta = self.enemy.origin - self.origin;
	if (vlen(delta) < 70)  // Too close to shoot with a missile
		mummymelee();

	if (self.parts_gone == MUMMY_NONE)
	{
		result = AdvanceFrame($shoota1,$shoota16);

		if (self.frame == $shoota6)
		{
			makevectors(self.angles);
			Create_Missile(self,self.origin + v_forward*14 - v_right * 9 + v_up * 25, 
				self.enemy.origin+self.enemy.view_ofs,"models/akarrow.mdl","green_arrow",0,1000,mummissile_touch);
		}

		if (self.frame == $shoota12)
		{
			if (enemy_range < RANGE_NEAR)
				chance = 0.80;
			else if (enemy_range < RANGE_MID)
				chance = 0.70;
			else if (enemy_range < RANGE_FAR)
				chance = 0.40;

			if (random() < chance)  // Repeat as necessary
				self.frame = $shoota5;
		}

		if (result == AF_END)
		{
			mummyrun();
		}
		else
			ai_face();
	}
	else if (self.parts_gone == MUMMY_LARM)
	{
		result=AdvanceFrame($shootb1,$shootb16);  // Because there is no one 

		if (self.frame == $shootb6)
		{
			makevectors(self.angles);
			Create_Missile(self,self.origin + v_forward*14 + v_right * 11 + v_up * 40, 
				self.enemy.origin+self.enemy.view_ofs,"models/akarrow.mdl","green_arrow",0,1000,mummissile_touch);
		}

		if (self.frame == $shootb12)
		{
			if (enemy_range < RANGE_NEAR)
				chance = 0.80;
			else if (enemy_range < RANGE_MID)
				chance = 0.70;
			else if (enemy_range < RANGE_FAR)
				chance = 0.40;

			if (random() < chance)  // Repeat as necessary
				self.frame = $shootb5;
		}

		if (result == AF_END)
			mummyrun();
		else
			ai_face();
	}
	else
		mummyrun();
}

void mummylordchoice (void)
{
	float chance;

	// He's more likely to use his flame attack when enemy is
	// farther away

	if (enemy_range < RANGE_NEAR)
		chance = 0.60;
	else if (enemy_range < RANGE_MID)
		chance = 0.80;
	else if (enemy_range < RANGE_FAR)
		chance = 0.90;

	if (random() < chance) 
		lordmummymissile();
	else
		mummymissile();
}

void mummypunch ()
{
	local vector	delta;
	local float	ldmg;

	delta = self.enemy.origin - self.origin;
	if (vlen(delta) > 50)
		return;

	self.last_attack=time;

	if (self.classname == "monster_mummy")
		ldmg = DMG_MUMMY_PUNCH;
	else
		ldmg = DMG_MUMMY_PUNCH * 2;

	T_Damage (self.enemy, self, self, ldmg);

	sound (self, CHAN_WEAPON, "weapons/gauntht1.wav", 1, ATTN_NORM);
}

void mummybite(void)
{
	local vector	delta;
	local float	ldmg;

	delta = self.enemy.origin - self.origin;
	if (vlen(delta) > 50)
		return;

	self.last_attack=time;
	ldmg = random() * DMG_MUMMY_PUNCH;

	T_Damage (self.enemy, self, self, ldmg);

	sound (self, CHAN_WEAPON, "mummy/bite.wav", 1, ATTN_NORM);
}

void mummymelee(void)
{
	float result;
	vector delta;

	self.nextthink = time + HX_FRAME_TIME;
	self.think = mummymelee;

	if (self.parts_gone == MUMMY_NONE)
	{
		result=AdvanceFrame($mswngA1,$mswngA18);

		if (self.frame == $mswngA8)
			sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);

		if (self.frame == $mswngA11)
			mummypunch();
	}
	else if (self.parts_gone == MUMMY_LARM)
	{
		result=AdvanceFrame($mswngB1,$mswngB18);

		if (self.frame == $mswngB8)
			sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);

		if (self.frame == $mswngB11)
			mummypunch();
	}
	else if (self.parts_gone == MUMMY_RARM)
	{
		result=AdvanceFrame($mswingC1,$mswingC10);

		if (self.frame == $mswingC5)
			sound (self, CHAN_WEAPON, "weapons/gaunt1.wav", 1, ATTN_NORM);

		if (self.frame == $mswingC8)
			mummypunch();
	}
	else	// He's on the floor so he'll bite your legs off
	{
	//	self.nextthink = time + HX_FRAME_TIME * 2;
		thinktime self : HX_FRAME_TIME *2;

		self.mummy_state=MUMMY_DOWN;
		result=AdvanceFrame($crawl1,$crawl14);
		if (self.frame == $crawl14)
			sound (self, CHAN_BODY, "mummy/crawl.wav", 1, ATTN_NORM);

		if (self.frame == $crawl7)
			mummybite();
	}

	if (result == AF_END)
	{
		delta = self.enemy.origin - self.origin;
		if (vlen(delta) > 80)
			mummyrun();
	}
	else
		ai_charge(1);
}

void mummyrun(void)
{
	float distance;

//	self.nextthink = time + HX_FRAME_TIME;
	thinktime self : HX_FRAME_TIME;
	self.think = mummyrun;

	if (self.parts_gone==MUMMY_NONE)
	{
		if ((random() < .10) && (self.frame == $mwalkA1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkA1,$mwalkA14);

		if (self.frame==$mwalkA6)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkA7)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);

		if ((self.frame >= $mwalkA1) && (self.frame <= $mwalkA4))
			distance = 3.25;
		else 
			distance = 2.25;
	}
	else if (self.parts_gone==MUMMY_LARM)
	{
		if ((random() < .10) && (self.frame == $mwalkB1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkB1,$mwalkB16);
		if (self.frame==$mwalkB6)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkB8)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);
		distance = 3;
	}
	else if (self.parts_gone==MUMMY_RARM)
	{
		if ((random() < .10) && (self.frame == $mwalkC1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkC1,$mwalkC18);
		if (self.frame==$mwalkC8)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkC10)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);
		distance = 3;
	}
	else if (self.parts_gone <= MUMMY_LEG)
	{
		if (self.mummy_state==MUMMY_WAVER)
		{
			AdvanceFrame($mdeath1,$mdeath13);
			if (self.frame==$mdeath13)
				self.mummy_state=MUMMY_DOWN;
			distance = 0;
		}
		else
		{
		//	self.nextthink = time + HX_FRAME_TIME * 2;
			thinktime self : HX_FRAME_TIME *2;

			AdvanceFrame($crawl1,$crawl14);

			if (self.frame == $crawl2)
				sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);

			if ((self.frame >= $crawl1) && (self.frame <= $crawl5))
				distance = 3;
			else
				distance = 0;
		}
	}

	ai_run(distance);
}


void mummywalk(void)
{
	float distance;

//	self.nextthink = time + HX_FRAME_TIME + .02;
	thinktime self : HX_FRAME_TIME + .02;
	self.think = mummywalk;

	if (self.parts_gone==MUMMY_NONE)
	{
		if ((random() < .10) && (self.frame == $mwalkA1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkA1,$mwalkA14);

		if (self.frame==$mwalkA6)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkA7)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);

		if ((self.frame >= $mwalkA1) && (self.frame <= $mwalkA4))
			distance = 3.25;
		else
			distance = 2.25;
	}
	else if (self.parts_gone==MUMMY_LARM)
	{
		if ((random() < .10) && (self.frame == $mwalkB1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkB1,$mwalkB16);
		distance = 2.25;

		if (self.frame==$mwalkB6)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkB8)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);
	}
	else if (self.parts_gone==MUMMY_RARM)
	{
		if ((random() < .10) && (self.frame == $mwalkC1))
		{
			if (self.classname == "monster_mummy_lord")
				sound (self, CHAN_VOICE, "mummy/moan2.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "mummy/moan.wav", 1, ATTN_NORM);
		}

		AdvanceFrame($mwalkC1,$mwalkC18);
		distance = 2.25;

		if (self.frame==$mwalkC8)
			sound (self, CHAN_BODY, "mummy/step.wav", 1, ATTN_NORM);
		else if (self.frame==$mwalkC10)
			sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);
	}
	else if (self.parts_gone <= MUMMY_LEG)
	{
		if (self.mummy_state==MUMMY_WAVER)
		{
			AdvanceFrame($mdeath1,$mdeath13);
			if (self.frame==$mdeath13)
				self.mummy_state=MUMMY_DOWN;
			distance = 0;
		}
		else
		{
		//	self.nextthink = time + HX_FRAME_TIME * 2;
			thinktime self : HX_FRAME_TIME *2;

			if (self.frame == $crawl2)
				sound (self, CHAN_BODY, "mummy/slide.wav", 1, ATTN_NORM);

			AdvanceFrame($crawl1,$crawl14);
			if (self.frame == $crawl14)
				sound (self, CHAN_BODY, "mummy/crawl.wav", 1, ATTN_NORM);
			distance = 2;
		}
	}

	ai_walk(distance);
}

void mummystand(void)
{
//	self.nextthink = time + HX_FRAME_TIME;
	thinktime self : HX_FRAME_TIME;
	self.think = mummystand;

	self.frame = $mwalkA1;

	if (random() < .5)
		ai_stand();
}

/*QUAKED monster_mummy (1 0.3 0) (-16 -16 0) (16 16 50) AMBUSH 
No, it's not Keith Richards or Bob Dylan.
It's the mummy.
-------------------------FIELDS-------------------------
health : 150
experience : 500
--------------------------------------------------------
*/
void monster_mummy (void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.flags2&FL_SUMMONED)
	{
		precache_model2("models/mummy.mdl");
		precache_model2 ("models/larm.mdl");
		precache_model2 ("models/leg.mdl");
		precache_model2 ("models/mumshot.mdl");
		precache_model2 ("models/muhead.mdl");

		precache_sound2 ("mummy/sight.wav");
		precache_sound2 ("mummy/die.wav");
		precache_sound2 ("mummy/mislfire.wav");
		precache_sound2 ("mummy/limbloss.wav");
		precache_sound2 ("mummy/moan.wav");
		precache_sound2 ("mummy/pain.wav");
		precache_sound2 ("mummy/crawl.wav");
		precache_sound2 ("mummy/slide.wav");
		precache_sound2 ("mummy/step.wav");
		precache_sound2 ("mummy/tap.wav");
		precache_sound2 ("mummy/bite.wav");
	}

	CreateEntityNew(self,ENT_MUMMY,"models/mummy.mdl",mummy_die);

	self.mintel = 3;
	self.monsterclass = CLASS_GRUNT;
	self.th_stand = mummystand;
	self.th_walk = mummywalk;
	self.th_run = mummyrun;
	self.th_melee = mummymelee;
	self.th_missile = mummymissile;
	self.th_pain = mummy_pain;
	self.parts_gone = MUMMY_NONE;
	self.skin = 0;

	self.flags (+) FL_MONSTER;
	self.yaw_speed = 10;
	self.health = 200;
	self.experience_value = 200;
	walkmonster_start();
}

/*QUAKED monster_mummy_lord (1 0.3 0) (-16 -16 0) (16 16 50) AMBUSH STUCK JUMP PLAY_DEAD DORMANT
He's big, he's bad, he's wrapped in moldy bandages - he's the mummy.
-------------------------FIELDS-------------------------
health : 500
experience : 300
--------------------------------------------------------
*/
void monster_mummy_lord (void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.flags2&FL_SUMMONED)
	{
		precache_model2("models/mummy.mdl");
		precache_model2("models/larm.mdl");
		precache_model2("models/leg.mdl");
		precache_model2("models/mumshot.mdl");
		precache_model2("models/muhead.mdl");

		precache_sound2 ("mummy/sight2.wav");
		precache_sound2 ("mummy/die2.wav");
		precache_sound2 ("mummy/mislfire.wav");
		precache_sound2 ("mummy/limbloss.wav");
		precache_sound2 ("mummy/moan2.wav");
		precache_sound2 ("mummy/pain2.wav");
		precache_sound2 ("mummy/crawl.wav");
		precache_sound2 ("mummy/slide.wav");
		precache_sound2 ("mummy/step.wav");
		precache_sound2 ("mummy/tap.wav");
		precache_sound2 ("mummy/bite.wav");
	}

	CreateEntityNew(self,ENT_MUMMY,"models/mummy.mdl",mummy_die);

	self.mintel = 3;
	self.monsterclass = CLASS_GRUNT;
	self.th_stand = mummystand;
	self.th_walk = mummywalk;
	self.th_run = mummyrun;
	self.th_melee = mummymelee;
	self.th_missile = mummylordchoice;
	self.th_pain = mummy_pain;
	self.parts_gone = MUMMY_NONE;
	self.skin = 1;
	self.headmodel="models/muhead.mdl";

	self.flags (+) FL_MONSTER;
	self.yaw_speed = 10;
	self.health = 400;
	self.experience_value = 300;
	walkmonster_start();
}

