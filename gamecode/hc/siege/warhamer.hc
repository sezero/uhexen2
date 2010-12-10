/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/warhamer.hc,v 1.2 2007-02-07 16:25:01 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\warhammer\final\warham.hc
MG
==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\warhammer\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame Chop1        Chop2        Chop3        Chop4        Chop5        
$frame Chop6        Chop7        Chop8        Chop9        Chop10       
$frame Chop11       Chop12       

//
$frame Hack1        Hack2        Hack3        Hack4        Hack5        
$frame Hack6        Hack7        Hack8        Hack9        Hack10       

//
$frame LtoR1        LtoR2        LtoR3        LtoR4        LtoR5        
$frame LtoR6        LtoR7        LtoR8        LtoR9        LtoR10       
$frame LtoR11

//
$frame Return1      Return2      Return3	  Return4    

//
$frame RtoL1        RtoL2        RtoL3        RtoL4        RtoL5        
$frame RtoL6        RtoL7        RtoL8        RtoL9        RtoL10       
$frame RtoL11       RtoL12       RtoL13       

//
$frame Select1      Select2      Select3      Select4      Select5      
$frame Select6      Select7      Select8      Select9      

//
$frame Throw1       Throw2       Throw3       Throw4       Throw5       
$frame Throw6       Throw7       Throw8       Throw9       Throw10

//
$frame idle


void(vector org) spawn_tfog;
void(float max_strikes, float damg)CastLightning;
void()warhammer_idle;
void()Cru_Wham_Fire;
void()warhammer_select;

void ThrowHammerReturn (void)
{
	// This sound is kinda irritating, I think
//	sound(self.controller, CHAN_ITEM, "weapons/weappkup.wav",1, ATTN_NORM);
//	if(self.controller.weapon==IT_WEAPON1)
// Only play selection if they're still on weapon1
//Or, if using another weapon, play the select frames,
//then go back to what you were doing with the current weapon
	self.controller.weapon=IT_WEAPON1;
	self.controller.th_weapon=warhammer_select;
   	stopSound(self,CHAN_VOICE);
	//sound(self, CHAN_VOICE, "misc/null.wav", 0.3, ATTN_NORM);
	remove(self);
}

void hammer_bounce ()
{
	if(other.thingtype!=THINGTYPE_FLESH)
	{
		if(self.t_width<time)
		{
			self.t_width=time+0.3;
			if(random()<0.3)
			{
				sound(self,CHAN_BODY,"weapons/met2stn.wav",0.5,ATTN_NORM);
			}
			else if(random()<0.5)
			{
				sound(self,CHAN_BODY,"weapons/vorpht2.wav",0.5,ATTN_NORM);
			}
			else
			{
				sound(self,CHAN_BODY,"paladin/axric1.wav",0.75,ATTN_NORM);
			}
		}
	}
}

void() ThrowHammerThink =
{
	vector destiny;
	float distance;

	makevectors(self.controller.v_angle);
	destiny=self.controller.origin+self.controller.proj_ofs+v_right*7+'0 0 -7';
	distance=vlen(self.origin-destiny);	

	if(self.lifetime<time||(distance<28&&(self.bloodloss<time||self.aflag==-1)))
	{
		ThrowHammerReturn();
	}

	if(self.bloodloss<=time)
	{
		self.owner=self;
	}
	if(distance>377)
	{
		self.aflag= -1;
	}
	
	if (self.aflag == -1)
	{
		local vector dir;

		dir = normalize(destiny - self.origin);
		if (self.watertype < -2)
		{
			self.velocity = dir * self.speed*0.5;
		}
	    else self.velocity = dir * self.speed;
		{
			self.angles = vectoangles(self.velocity);
		}
		if (self.flags & FL_ONGROUND)
	    {
	        self.flags(-)FL_ONGROUND;
		}
    }
	
	if(self.controller.health<=0||!self.controller.flags2&FL_ALIVE||self.controller.model=="models/sheep.mdl")
    {
       	stopSound(self,CHAN_VOICE);
		//sound(self, CHAN_VOICE, "misc/null.wav", 0.3, ATTN_NORM);
		remove(self);
		return;
    }
//	HammerZap();
    thinktime self : 0.1;
	self.think = ThrowHammerThink;
};


void HammerTouch ()
{
	// aflag is if it's returning or not --> set means it's returning
	float inertia;

	if (other == self.controller)
	{
		if (self.aflag||self.bloodloss<time)
		{
			ThrowHammerReturn();
		}
        else 
		{
			return;
		}
	}
	else if (other.takedamage)
	{
		if(self.velocity != VEC_ORIGIN && other != self.controller)
		{
			if (self.aflag < 1)  
			{
				other.punchangle_x = -20;
				self.enemy = other;  

				WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
				WriteByte(MSG_MULTICAST, TE_LIGHTNING_HAMMER);
				WriteEntity(MSG_MULTICAST, self);
				multicast(self.origin,MULTICAST_PVS);

				if(other.health&&other.solid!=SOLID_BSP&&other.movetype!=MOVETYPE_PUSH)
				{
					if (other.mass<=10)
					{
						inertia=1;
					}
					else 
					{
						inertia = other.mass/10;
					}
					other.velocity_x = other.velocity_x + self.velocity_x / inertia;
					other.velocity_y = other.velocity_y + self.velocity_y / inertia;
					other.flags(-)FL_ONGROUND;
				}
				T_Damage(other, self, self.controller, 32);
			}
		}
	}
	else if(!MetalHitSound(other.thingtype))
	{
		sound(self, CHAN_BODY, "weapons/hitwall.wav", 1, ATTN_NORM);
	}
  
	if (self.aflag < 1)
	{
		self.aflag = -1;
		self.movetype=MOVETYPE_NOCLIP;
		self.solid=SOLID_PHASE;
	}
}


void ThrowHammer ()
{
	local entity	missile;
	entity			prevmis;

	prevmis=findradius(self.origin,2000);
	while(prevmis)
	{
		if (prevmis.classname == "mjolnir" && prevmis.controller == self)
		{
			// this next line has to be here for some reason... ?  Dunno
			prevmis=prevmis.chain;
			return;
		}
		prevmis=prevmis.chain;
	}

	sound(self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
	//weapon_sound(self, "weapons/vorpswng.wav");
	missile = spawn();
	missile.owner = self;
	missile.controller = self;
	missile.classname = "mjolnir";
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	makevectors(self.v_angle);
	missile.velocity = normalize(v_forward);
	missile.angles = vectoangles(missile.velocity);
	missile.speed = 800;
	if (self.waterlevel > 2)
	{
		missile.velocity = missile.velocity * missile.speed*0.5;
	}
	else
	{
		missile.velocity = missile.velocity * missile.speed;
	}
	missile.touch = HammerTouch;
	thinktime missile : 0;
	missile.frags=TRUE;
	missile.bloodloss = time + 0.3;
	missile.lifetime = time + 3;
	sound(missile, CHAN_VOICE, "paladin/axblade.wav", 1, ATTN_NORM);
	//weapon_sound(missile, "paladin/axblade.wav");
	missile.think = ThrowHammerThink;
	setmodel(missile, "models/hamthrow.mdl");
	setsize(missile,'-1 -2 -4','1 2 4');
	setorigin(missile, self.origin+self.proj_ofs + v_forward * FL_SWIM);
//	missile.avelocity = '-500 0 0';
	missile.aflag = 0;
	missile.level= 4;
	missile.drawflags=MLS_ABSLIGHT;//Powermode?  Translucent when returning?
	missile.abslight = 1;
	missile.dmg=200;
	self.attack_finished=time + 1;

	missile.effects(+)EF_HAMMER_EFFECTS;
}

void warhammer_gone ()
{
	self.th_weapon=warhammer_gone;
	self.weaponmodel="";
	self.weaponframe=0;
}

void warhammer_throw ()
{
	self.th_weapon=warhammer_throw;
	self.wfs = advanceweaponframe($Throw1,$Throw10);
	if (self.weaponframe == $Throw7)
	{
		ThrowHammer();
	}
	else if (self.wfs==WF_LAST_FRAME)
		warhammer_gone();
}

void test_traceline ()
{
	vector	source;
	vector	dir;

	makevectors (self.v_angle);
	source = self.origin + self.proj_ofs;
	dir=normalize(v_forward);

	traceline (source, source + dir*64, TRUE, self);

//	if (trace_fraction <1.0)
//		spawntestmarker(trace_endpos);
}

void warhammer_fire (string hitdir,vector ofs)
{
	vector	source;
	vector	org,dir;
	float	damg, inertia,force,hit_dist;

	makevectors (self.v_angle);
	source = self.origin + self.proj_ofs;

//	tracearea (self.origin, self.origin + v_forward*64*self.scale, '-16 -16 -14','16 16 14',FALSE, world);
	if(self.playerclass==CLASS_DWARF)
		hit_dist = 32;
	else
		hit_dist = 64;
	traceline (source, source + v_forward*hit_dist*self.scale + ofs, FALSE, self.goalentity);
	if(!trace_ent||trace_ent==self.goalentity)
		traceline (source, source + v_forward*hit_dist*self.scale + ofs-v_up*32, FALSE, self.goalentity);
	if(!trace_ent||trace_ent==self.goalentity)
		traceline (source, source + v_forward*hit_dist*self.scale + ofs+v_up*16, FALSE, self.goalentity);

	if (trace_fraction <1.0)//&&trace_ent!=firsttarg)  
	{
		org = trace_endpos + (v_forward * 4);
	
		if (trace_ent.takedamage&&trace_ent!=self)
		{
			if(trace_ent.solid==SOLID_BSP||trace_ent.movetype==MOVETYPE_PUSH)
				inertia = 1000;
			else if(trace_ent.mass<=10)
				inertia=1;
			else
				inertia=trace_ent.mass/10;

			force=self.strength/40+0.5;
			self.goalentity=trace_ent;
			SpawnPuff (org, '0 0 0', 20,trace_ent);
	
			damg = random(15,25);
			
			if(trace_ent.thingtype==THINGTYPE_GREYSTONE||trace_ent.thingtype==THINGTYPE_BROWNSTONE||trace_ent.thingtype==THINGTYPE_WOOD_STONE)
				damg*=1.2;

			if(trace_ent.thingtype==THINGTYPE_DIRT&&self.playerclass==CLASS_DWARF)
				damg*=1.5;

			org = trace_endpos + (v_forward * -1);
			if(!MetalHitSound(trace_ent.thingtype))
			{
				sound (self, CHAN_WEAPON, "weapons/gauntht1.wav", 1, ATTN_NORM);
			}

			if(hitdir=="top")
			{
				damg=damg*2;
				if(trace_ent.classname=="player")
					trace_ent.deathtype="hammercrush";
			}

			if(inertia<100)//don't move anything more than 1000 mass
			{
				dir =normalize(trace_ent.origin - self.origin)*damg*2;

				if(hitdir=="right")
				{
					dir=dir+normalize(v_right)*40;
					trace_ent.punchangle_y=6;
				}
				else if(hitdir=="left")
				{
					dir=dir-normalize(v_right)*40;
					trace_ent.punchangle_y=-6;
				}
				trace_ent.velocity = dir*(1/inertia)*force;
				if(trace_ent.movetype==MOVETYPE_FLY)
				{
					if(trace_ent.flags&FL_ONGROUND)
						trace_ent.velocity_z=175/inertia*force;
				}
				else
					trace_ent.velocity_z = 175/inertia*force;
				trace_ent.flags(-)FL_ONGROUND;
			}
			T_Damage (trace_ent, self, self, damg);
			if(trace_ent.classname=="player")
				trace_ent.deathtype="";
		}
		else if(ofs=='0 0 0')
		{	// hit wall, add sparks?
			sound (self, CHAN_WEAPON, "weapons/hitwall.wav", 1, ATTN_NORM);
		}
	}
}

void warhammer_idle(void)
{
	self.th_weapon=warhammer_idle;
	self.weaponframe=$idle;
}

void warhammer_return (void)
{
	self.th_weapon=warhammer_return;
	self.wfs = advanceweaponframe($Return1,$Return4);
	if (self.wfs==WF_CYCLE_WRAPPED)
		warhammer_idle();
}

void warhammer_deselect (void)
{
	self.wfs = advanceweaponframe($Select9,$Select1);
	self.th_weapon=warhammer_deselect;
	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}

void warhammer_select (void)
{
//  Check to see if have warhammer
//	if(!self.?) warhammer_gone();
	self.th_weapon=warhammer_select;
	self.wfs = advanceweaponframe($Select1,$Select9);
	self.weaponmodel = "models/warhamer.mdl";
	if (self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		warhammer_idle();
	}
}

void warhammer_c (void)
{
vector ofs;
	makevectors(self.v_angle);
	self.th_weapon=warhammer_c;
	self.wfs = advanceweaponframe($LtoR1,$LtoR11);
	if (self.weaponframe == $LtoR4)
		self.weaponframe+=3;
	if (self.weaponframe == $LtoR7)
	{
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");
		self.goalentity=self;
		ofs=normalize(v_right)*30;
		warhammer_fire ("right",ofs);
	}
	else if (self.weaponframe == $LtoR8)
		warhammer_fire ("right",'0 0 0');
	else if (self.weaponframe == $LtoR9)
	{
		ofs=normalize(v_right)*-30;
		warhammer_fire ("right",ofs);
	}

	if (self.wfs==WF_CYCLE_WRAPPED)
		warhammer_return();
}

void warhammer_b (void)
{
vector ofs;
	makevectors(self.v_angle);
	self.th_weapon=warhammer_b;
	self.wfs = advanceweaponframe($RtoL1,$RtoL11);
	if (self.weaponframe == $RtoL5)
	{
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");
		self.goalentity=self;
		ofs=normalize(v_right)*-30;
		warhammer_fire ("left",ofs);
	}
	else if (self.weaponframe == $RtoL6)
		warhammer_fire ("left",'0 0 0');
	else if (self.weaponframe == $RtoL7)
	{
		ofs=normalize(v_right)*30;
		warhammer_fire ("left",ofs);
	}
	if (self.wfs==WF_CYCLE_WRAPPED)
		warhammer_select();
}	

void warhammer_a (void)
{
vector ofs;
	makevectors(self.v_angle);
	self.th_weapon=warhammer_a;
	self.wfs = advanceweaponframe($Chop1,$Chop12);
	if (self.weaponframe == $Chop7)
	{
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");
		self.goalentity=self;
		ofs=normalize(v_up)*30;
		ofs+=normalize(v_right)*15;
		warhammer_fire ("top",ofs);
	}
	else if (self.weaponframe == $Chop8)
		warhammer_fire ("top",'0 0 0');
	else if (self.weaponframe == $Chop9)
	{
		ofs=normalize(v_up)*-30;
		warhammer_fire ("top",ofs);
	}

	if (self.wfs==WF_CYCLE_WRAPPED)
		warhammer_return();
}

void Cru_Wham_Fire (void)
{
float r;
	if(self.artifact_active&ART_TOMEOFPOWER)
		warhammer_throw();
	else
	{
		self.attack_finished = time + .7;  // Attack every .7 seconds
		r = rint(random(1,3));
		if (r==1)
			warhammer_a();
		else if (r==2)
			warhammer_b();
		else if (r==3)
			warhammer_c();
	}
}

