/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/crossbow.hc,v 1.2 2007-02-07 17:00:37 sezero Exp $
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


/*
void burner_think ()
{
vector org,vel;
	if(self.lifetime<time||self.enemy.health<0)
	{
		sound(self.enemy,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
		remove(self);
		return;
	}
	else if(pointcontents(self.enemy.origin)==CONTENT_WATER)
	{
		sound (self.enemy, CHAN_BODY, "misc/fout.wav", 1, ATTN_NORM);
		smolder((self.enemy.absmin+self.enemy.absmax)*0.5);
		remove(self);
		return;
	}
	else
	{
		org=(self.enemy.absmin+self.enemy.absmax)*0.5+randomv(self.enemy.size*-0.25,self.enemy.size*0.25);
		vel=randomv('-3 -3 0','3 3 7'); 
		if(random()<=0.33)
			starteffect(CE_FLAMESTREAM, org,vel, 0);
		else if(random()<=0.5)
			starteffect(CE_GREEN_SMOKE, org,vel, 0);
		else
			starteffect(CE_RED_SMOKE, org,vel, 0);
		thinktime self : random(0.5);
		T_Damage(self.enemy,self,self.owner,2);
		sound(self.enemy,CHAN_BODY,"raven/fire1.wav",1,ATTN_NORM);
	}
}

void spawn_burner (entity loser)
{
entity burner;
	burner=spawn();
	burner.owner=self.owner;
	burner.enemy=loser;
	burner.lifetime=time+random(5)+15;
	burner.think=burner_think;
	thinktime burner : 0;
	sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NORM);
	starteffect(CE_LG_EXPLOSION , self.origin);
}
*/

void() CB_BoltHit=
{//FIXME: Flaming arrows catch wood & flesh on fire
	if(other==self.owner||(other.owner==self.owner&&other.classname==self.classname))
	    return;

vector stickdir, stickspot,center,hitspot;
float rad,stick,hitdmg,skiprest,headdist;

	v_forward=normalize(self.velocity);
	hitdmg=(vlen(self.velocity) - 600)/10;
	if(hitdmg<5)
		hitdmg=5;
	if(other.health>200&&other.thingtype!=THINGTYPE_FLESH)//So it can't break catapults and ballistas and ramparts
		hitdmg=1;

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
		if(other.solid!=SOLID_BSP)
			if(self.frags)
				if(flammable(other))
					spawn_burner(other,FALSE);

		if(other.flags2&FL_ALIVE&&other.classname!="player_sheep")
		{
			self.owner.experience+=(time - self.last_time)*30;//0.5 sec dist = 15 point bonus
			headdist=vlen(other.origin+other.view_ofs - self.origin);
			hitspot=self.origin+v_forward*headdist;
			if(vlen(hitspot-self.origin)<10)//head shot, instant kill
			{
				T_Damage(other,self,self.owner,other.health+30);
				if(!other.flags2&FL_ALIVE)
				{
					self.experience+=50;
					if(self.owner.classname=="player"&&self.owner.flags&FL_CLIENT)
						centerprint(self.owner,"Headshot!\n");
				}
				else
					self.experience+=10;
				skiprest=TRUE;
			}
		}

		if(!skiprest)
		{
			if(self.classname=="bolt")
			{
				if(other.thingtype==THINGTYPE_FLESH||other.classname=="barrel")
						T_Damage(other,self,self.owner,hitdmg);
					else
						T_Damage(other,self,self.owner,random(2,5));
			}
			else
				T_Damage(other,self,self.owner,3);
		}
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
	if(self.lifetime<time&&!self.frags)
	{
		self.velocity_z-=30;
		self.angles=vectoangles(self.velocity);
	}
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

void(float powered_up) FireCB_Bolt =
{
entity missile;
float offset,speed_mod;
vector v_offset;
	self.cnt_arrows-=1;
	makevectors(self.v_angle);
	missile=spawn();
	missile.owner=self;
	missile.solid=SOLID_BBOX;
	missile.hull=HULL_POINT;
	missile.health=3;
	missile.last_time = time;
//	if(deathmatch)
//		offset*=.333;
	if(powered_up)
	{
	    if(self.torchtime>time)
	    {
		missile.frags=TRUE;
		missile.thingtype=THINGTYPE_METAL;
		setmodel(missile,"models/flaming.mdl");
		missile.speed=random(1100,1400);
		missile.classname="flarrow";
		missile.drawflags(+)MLS_ABSLIGHT|MLS_FIREFLICKER;
		missile.abslight = .75;
	    }
	    else
	    {
		missile.classname="bolt";
		missile.thingtype=THINGTYPE_WOOD_METAL;
		setmodel(missile,"models/arrow.mdl");
		speed_mod=melee_dmg_mod_for_strength (self.strength);
		missile.speed=random(1100,1400);
	    }
	}
	else
	{
		missile.classname="bolt";
		missile.thingtype=THINGTYPE_WOOD;
		setmodel(missile,"models/arrow.mdl");
		speed_mod=melee_dmg_mod_for_strength (self.strength);
		missile.speed=800+100*(random(speed_mod));
	}
	missile.movetype=MOVETYPE_FLYMISSILE;
	missile.th_die=chunk_death;
	missile.touch=CB_BoltHit;

float num_weap;
	num_weap=num_for_weap (self.weapon);
	offset = rate_and_acc_for_weap [(self.playerclass - 1)*6+(num_weap - 1)*2+1]/2;

	v_offset = v_right*(random(offset)*2 - offset) + v_up*(random(offset)*2 - offset) + v_forward;
	v_offset = normalize(v_offset);
	missile.o_angle=missile.velocity=normalize(v_offset)*missile.speed;
	missile.angles=vectoangles(missile.velocity);

	if(!powered_up)
	{
		missile.think = Missile_Arc;
		thinktime missile : 0.2;
	}
//	missile.lifetime=time+0.2;

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
float num_weap;
	self.wfs = advanceweaponframe($shoot1,$shoot18);
	self.th_weapon=crossbow_fire;
	if(self.button0&&self.weaponframe==$shoot2 &&self.weapon==IT_WEAPON3)
		self.weaponframe=$shoot1;
	else if (self.weaponframe == $shoot2)
	{
		num_weap = num_for_weap (self.weapon);
		if(self.playerclass==CLASS_ASSASSIN&&self.weapon==IT_WEAPON3)
		{
			sound(self,CHAN_WEAPON,"assassin/firefblt.wav",1,ATTN_NORM);
			FireCB_Bolt(TRUE);
			self.attack_finished=time+2;//rate_and_acc_for_weap[(self.playerclass - 1)*6+(num_weap - 1)*2];
		}
		else
		{
			sound(self,CHAN_WEAPON,"assassin/firebolt.wav",1,ATTN_NORM);
			FireCB_Bolt(FALSE);
			self.attack_finished=time+rate_and_acc_for_weap[(self.playerclass - 1)*6+(num_weap - 1)*2];
		}
	}
	else if (self.wfs==WF_CYCLE_WRAPPED)
		crossbow_idle();
}

void crossbow_select (void)
{
//selection sound?
	self.wfs = advanceweaponframe($select15,$select1);
//	self.weaponmodel = "models/xbow2.mdl";
	self.weaponmodel = "models/crossbow.mdl";
	self.th_weapon=crossbow_select;
	if (self.weaponframe==$select1)
	{
		self.attack_finished = time - 1;
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

//Dark Adder
void()longbow_fire;
void longbow_idle(void)
{
	self.th_weapon=longbow_idle;
	self.weaponframe=19;
}

void longbow_fire (void)
{
float num_weap;
	self.wfs = advanceweaponframe(20,39);
	self.th_weapon=longbow_fire;
	if (self.weaponframe == 21)
	{
		num_weap = num_for_weap (self.weapon);
		if(self.playerclass==CLASS_ASSASSIN&&self.weapon==IT_WEAPON3)
		{
			sound(self,CHAN_WEAPON,"assassin/firefblt.wav",1,ATTN_NORM);
			FireCB_Bolt(TRUE);
			self.attack_finished=time+2;//rate_and_acc_for_weap[(self.playerclass - 1)*6+(num_weap - 1)*2];
		}
		else
		{
			sound(self,CHAN_WEAPON,"assassin/firebolt.wav",1,ATTN_NORM);
			FireCB_Bolt(FALSE);
			self.attack_finished=time+rate_and_acc_for_weap[(self.playerclass - 1)*6+(num_weap - 1)*2];
		}
	}
	else if (self.wfs==WF_CYCLE_WRAPPED)
		longbow_idle();
}

void longbow_select (void)
{
//selection sound?
	self.wfs = advanceweaponframe(0,19);
	self.weaponmodel = "models/lbow.mdl";
	self.th_weapon=longbow_select;
	if (self.weaponframe==19)
	{
		self.attack_finished = time - 1;
		longbow_idle();
	}
}

void longbow_deselect (void)
{
	self.wfs = advanceweaponframe(40,54);
	self.th_weapon=longbow_deselect;
	if (self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

