/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/crossbow.hc,v 1.4 2007-02-07 16:57:00 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\crossbow\final\crossbow.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\crossbow\final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     select13     select14     select15     

//
$frame shoot1       shoot2       shoot3       shoot4       shoot5       
$frame shoot6       shoot7       shoot8       shoot9       shoot10      
$frame shoot11      shoot12      shoot13      shoot14      shoot15      
$frame shoot16      shoot17      shoot18      shoot19      


void flashspin ()
{
	if(self.lifetime<time)
		remove(self);
	else
	{
		self.scale+=0.05;
		thinktime self : 0.075;
	}
}

void MakeFlash(vector org)
{
	newmis=spawn_temp();
	newmis.movetype=MOVETYPE_NOCLIP;
	newmis.angles=vectoangles(v_forward);
	newmis.avelocity_z=random(200,700);
	newmis.scale=0.1;
	newmis.drawflags(+)MLS_ABSLIGHT|SCALE_ORIGIN_CENTER;
	newmis.abslight=0.5;
	newmis.lifetime=time+0.3;
	newmis.think=flashspin;
	thinktime newmis : 0;
	setmodel(newmis,"models/arrowhit.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,org);
}

void() FallAndRemove =
{
//self.enemy.movetype?
	traceline(self.origin,self.origin-'0 0 24',TRUE,self);
	if(pointcontents(self.origin)==CONTENT_SOLID||trace_fraction<1)
		remove(self);
	self.movetype=MOVETYPE_BOUNCE;
	self.velocity_z=random(-60,-150);
	self.flags(-)FL_ONGROUND;
	self.avelocity=RandomVector('50 50 50');
	self.think=SUB_Remove;
	thinktime self : self.wait;
};

void() CB_BoltStick=
{
	if(self.wait<=time)
	{
		if(self.classname=="bolt")
		{
			self.wait=2;
			self.think=FallAndRemove;
		}
		else if(self.classname=="stickmine")
			self.think=MultiExplode;
		else
			self.think=DarkExplosion;
	}
	else if(self.enemy.health<=0&&self.health)
	{
		self.health=0;
		if(self.classname=="bolt")
		{
			self.wait=random(1,3);
			self.think=FallAndRemove;
		}
		else
		{
			self.movetype=MOVETYPE_BOUNCE;
			self.velocity_z=random(-100,100);
			self.avelocity=RandomVector('50 50 50');
		}
	}
	else if(self.movetype!=MOVETYPE_BOUNCE)
	{
		setorigin(self,self.enemy.origin+self.view_ofs);
		self.angles=self.o_angle + self.enemy.angles;
		self.think=CB_BoltStick;
	}
	thinktime self : 0;
};

void() CB_BoltHit=
{
	if(other==self.owner||(other.owner==self.owner&&other.classname==self.classname))
	    return;

vector stickdir, stickspot,center;
float rad,stick;

	v_forward=normalize(self.velocity);
	sound(self,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
	setsize(self,'0 0 0','0 0 0');
	self.takedamage=DAMAGE_NO;

	self.velocity='0 0 0';
	self.movetype=MOVETYPE_NOCLIP;
	self.solid=SOLID_NOT;
	self.touch=SUB_Null;
	self.health=other.health;

	if(other.thingtype==THINGTYPE_FLESH)
		sound(self, CHAN_WEAPON, "assassin/arr2flsh.wav", 1, ATTN_NORM);
	else if(other.thingtype==THINGTYPE_WOOD)
		sound(self, CHAN_WEAPON, "assassin/arr2wood.wav", 1, ATTN_NORM);
	else
		sound(self, CHAN_WEAPON, "weapons/met2stn.wav", 1, ATTN_NORM);

	MakeFlash(self.origin-v_forward*8);

	if(other.takedamage)
	{
		if(self.classname=="bolt")
			T_Damage(other,self,self.owner,10);
		else
			T_Damage(other,self,self.owner,3);
		SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,other);
		if(other.solid!=SOLID_BSP)
		{
		//Put it right below view of player
			if(other.classname=="player")
			{
				stickdir_z=other.origin_z+other.proj_ofs_z+ 1;
				stickdir=other.origin+normalize(self.origin-other.origin)*12;
				stick=TRUE;
				setorigin(self,stickdir);
			}
			else
			{
				rad=(other.maxs_x+other.maxs_z)*0.5;
				center=(other.absmax+other.absmin)*0.5;
				stickspot=self.origin+v_forward*other.maxs_x*2;
				if(vlen(center-stickspot)<rad*0.5)
				{
					stick=TRUE;
					setorigin(self,stickspot);
				}
				else
					stick=FALSE;
			}
			self.wait=time + random(0.1,2);
		}
	}
	else
	{
		CreateWhiteSmoke(self.origin-v_forward*8,'0 0 8',HX_FRAME_TIME);
		SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,world);
		if(self.classname=="bolt")
		{
			if(random()<0.7)
				chunk_death();
			else if(random()<0.5)
			{
				self.movetype=MOVETYPE_BOUNCE;
				self.velocity_z=-20;
				self.flags(-)FL_ONGROUND;
				self.avelocity_x=random(-360,360);
				self.avelocity_y=random(-360,360);
				self.avelocity_z=random(-360,360);
				self.touch=SUB_Null;
				self.think=SUB_Remove;
				thinktime self : random(0.5,1.5);
				return;
			}
		}
		else
			stick=TRUE;
		self.wait=time + random(1,3);
	}

//FIXME: only stick in if thingtype is wood or flesh,
//otherwise, no damage and bounce off!
	if(other.movetype||other.takedamage||stick||self.health)
	{
		if(stick)
		{
			self.enemy=other;
			self.view_ofs=(self.origin-other.origin);
			self.o_angle=(self.angles-self.enemy.angles);
			if(other.health)
				self.health=other.health;
			else
				self.health=FALSE;
			self.think=CB_BoltStick;
			thinktime self : 0;
		}
		else if(self.classname=="bolt")
			remove(self);
		else
			DarkExplosion();
	}
	else
	{
		self.movetype=MOVETYPE_NONE;
		if(self.classname=="bolt")
			self.think=SUB_Remove;
		else
			self.think=DarkExplosion;
		thinktime self : 2;
	}
};

void ArrowFlyThink (void)
{
	if(self.lifetime<time&&self.mins=='0 0 0')
	{
		self.takedamage=DAMAGE_YES;
//		setsize(self,'-3 -3 -2','3 3 2');
	}
	if(self.model=="models/flaming.mdl")
	{
		self.frame+=1;
		if(self.frame>9)
			self.frame=0;
	}
	self.angles=vectoangles(self.velocity);
	self.think=ArrowFlyThink;
	thinktime self : 0.05;
}

void ArrowSound (void)
{
	//attn_static instead?
//	sound(self,CHAN_BODY,"assassin/arrowfly.wav",1,ATTN_NORM);
	self.think=ArrowFlyThink;
	thinktime self : 0;
}

void FlamingArrowThink (void)
{
float waterornot;
	waterornot=pointcontents(self.origin);
	if(waterornot==CONTENT_WATER||waterornot==CONTENT_SLIME)
	{
		sound (self, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
		DeathBubbles(1);
		setmodel(self,"models/NFarrow.mdl");
	}
	ArrowSound();
}

void ArrowThink (void)
{
vector dir;
	dir=normalize(self.velocity);
	traceline(self.origin,self.origin+dir*1000,FALSE,self);
	if(!trace_ent.takedamage)
		HomeThink();
	self.angles=vectoangles(self.velocity);
	if(self.classname=="bolt")
		self.think=ArrowSound;
	else
		self.think=FlamingArrowThink;
	thinktime self : 0;
}

void(float offset, float powered_up) FireCB_Bolt =
{
local entity missile;
	makevectors(self.v_angle);
	missile=spawn();
	missile.owner=self;
	missile.solid=SOLID_BBOX;

	missile.hull=HULL_POINT;
	missile.health=3;
	if(deathmatch)
		offset*=.333;
	if(powered_up)
	{
		missile.frags=TRUE;
		missile.thingtype=THINGTYPE_METAL;
		missile.movetype=MOVETYPE_FLYMISSILE;
		missile.classname="flaming arrow";
		setmodel(missile,"models/flaming.mdl");
		missile.dmg=40;
		missile.drawflags(+)MLS_FIREFLICKER;
		missile.th_die=MultiExplode;
	}
	else
	{
		missile.thingtype=THINGTYPE_WOOD;
		missile.movetype=MOVETYPE_FLYMISSILE;
		missile.classname="bolt";
		setmodel(missile,"models/arrow.mdl");
		missile.th_die=chunk_death;
	}
	missile.touch=CB_BoltHit;
	missile.speed=random(700,1200);
	missile.o_angle=missile.velocity=normalize(v_forward)*missile.speed+v_right*offset;
	missile.angles=vectoangles(missile.velocity);

	missile.ideal_yaw=TRUE;
	missile.turn_time = 0;
	missile.veer=0;

	missile.think= ArrowThink;
	thinktime missile : 0;
	missile.lifetime=time+0.2;

	setsize(missile,'0 0 0','0 0 0');
	setorigin(missile,self.origin+self.proj_ofs+v_forward*8);
};


void()crossbow_fire;
void crossbow_idle(void)
{
	self.th_weapon=crossbow_idle;
	self.weaponframe=$shoot19;
}

void crossbow_fire (void)
{
	// Pa3PyX: rewrote the code for framerate independence
	local float advance_frames;
	local float cnt_frame;
	local float attackframe1_passed;
	local float attackframe2_passed;
	local float attackframe3_passed;
	local float arate_factor;

	// Did the delay from previous attack expire yet?
	if ((time >= self.attack_finished) || (self.ltime > 0)) {
		if (self.ltime <= 0)
			self.ltime = time;
		// Tomed xbow: 3 shots per second; untomed: 2 shots per second
		// (xbow has 20 frames, so unscaled animation is 1 second long)
		if (self.artifact_active & ART_TOMEOFPOWER)
			arate_factor = 3.0;
		else
			arate_factor = 2.0;
		// Animation loop factor
		advance_frames = rint(arate_factor * (time - self.ltime) / HX_FRAME_TIME);
		if (advance_frames >= 1) {
			cnt_frame = 0;
			attackframe1_passed = attackframe2_passed = attackframe3_passed = FALSE;
			// Advance <advance_frames> frames
			while ((cnt_frame < advance_frames) && (self.wfs != WF_LAST_FRAME)) {
				self.wfs = advanceweaponframe($shoot1,$shoot18);
				self.weaponframe_cnt += 1;
				// Did we go over any attack frames?
				if (self.weaponframe_cnt == 2)
					attackframe1_passed = TRUE;
				if (self.weaponframe_cnt == 3)
					attackframe2_passed = TRUE;
				if (self.weaponframe_cnt == 4)
					attackframe3_passed = TRUE;
				cnt_frame += 1;
			}
			if (self.wfs == WF_LAST_FRAME) {
				// End of animation, clean up and exit
				self.wfs = WF_NORMAL_ADVANCE;
				self.weaponframe_cnt = 0;
				self.ltime = -1;
				self.attack_finished = time;
				self.th_weapon = crossbow_idle;
			}
			else {
				self.ltime = time;
				self.th_weapon=crossbow_fire;
			}
			// Attack frames were encountered in frame advance --
			// perform attack actions
			if (self.artifact_active & ART_TOMEOFPOWER) {
				if (attackframe1_passed) {
					sound(self,CHAN_WEAPON,"assassin/firefblt.wav",1,ATTN_NORM);
					self.bluemana-=10;
					FireCB_Bolt(0,TRUE);
				}
				if (attackframe2_passed) {
					FireCB_Bolt(-100,TRUE);
					FireCB_Bolt(100,TRUE);
				}
				if (attackframe3_passed) {
					FireCB_Bolt(-200,TRUE);
					FireCB_Bolt(200,TRUE);
				}
			}
			else {
				if (attackframe1_passed)
				{
					sound(self,CHAN_WEAPON,"assassin/firebolt.wav",1,ATTN_NORM);
					self.bluemana-=3;
					FireCB_Bolt(0,FALSE);
				}
				if (attackframe2_passed)
					FireCB_Bolt(-100,FALSE);
				if (attackframe3_passed)
					FireCB_Bolt(100,FALSE);
			}
		}
		else
			self.th_weapon = crossbow_fire;
	}
	else
		self.th_weapon = crossbow_fire;

	thinktime self: 0;
}

void crossbow_select (void)
{
//selection sound?
	self.wfs = advanceweaponframe($select15,$select1);
	self.weaponmodel = "models/crossbow.mdl";
	self.th_weapon=crossbow_select;
	if (self.weaponframe==$select1)
	{
		self.attack_finished = time - 1;

		// Pa3PyX
		self.ltime = -1;
		self.weaponframe_cnt = 0;
		self.wfs = WF_NORMAL_ADVANCE;

		crossbow_idle();
	}
}

void crossbow_deselect (void)
{
	self.wfs = advanceweaponframe($select1,$select15);
	self.th_weapon=crossbow_deselect;
	if (self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

