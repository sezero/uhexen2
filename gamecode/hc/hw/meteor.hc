/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/meteor.hc,v 1.2 2007-02-07 16:57:57 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\meteor\final\meteor.hc

==============================================================================
*/
// For building the model
$cd Q:\art\models\weapons\meteor\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame idle

//
$frame Select1      Select2      Select3      Select4      Select5      
$frame Select6      Select7      Select8      Select9      Select10     
$frame Select11     Select12     Select13     Select14     Select15     
$frame Select16     Select17     Select18     

//
$frame fire1     fire2     fire3     fire4     fire5     
$frame fire6     fire7     fire8     fire9     


void MeteoriteFizzle (void)
{
	CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
	remove(self);
}

void ByeByeMeteor(void)
{
	remove(self);
}

void MeteorExplode(void)
{
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_METEORHIT);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
	multicast(self.origin,MULTICAST_PHS_R);

	T_RadiusDamage (self, self.owner, 80.0, world);

	remove(self);
}

void MeteorTouch (void)
{
	if(other.controller==self.owner)
		return;

	if(other.takedamage&&other.health)
	{
		T_Damage(other,self,self.owner,self.dmg);

		if((other.flags&FL_CLIENT||other.flags&FL_MONSTER)&&other.mass<200)
		{
		vector hitdir;
			hitdir=self.o_angle*300;
			hitdir_z+=150;
			if(hitdir_z<0)
				hitdir_z=0;
			other.velocity=hitdir;
			other.flags(-)FL_ONGROUND;
		}
	}
	self.dmg=90;

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_METEORHIT);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
	multicast(self.origin,MULTICAST_PHS_R);

	T_RadiusDamage (self, self.owner, self.dmg, other);

	remove(self);
}

void MeteorThink(void)
{
	self.movedir = normalize(self.velocity);

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.movedir * 300.0, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_METEOR);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);

	thinktime self : 0.3;

	if (self.lifetime < time)
		SUB_Remove();

}


void FireMeteor (string type)
{
	vector org;
	entity meteor;

	meteor=spawn();
	setmodel(meteor,"models/tempmetr.mdl");

	meteor.th_die=MultiExplode;
	meteor.takedamage = DAMAGE_NO;
	meteor.health = 9999;
	if(self.classname=="player")
	{
		self.greenmana-=8;
		self.velocity+=normalize(v_forward)*-300;//include mass
		self.flags(-)FL_ONGROUND;
	}
	meteor.classname="meteor";
	self.punchangle_x = -6;
	weapon_sound(self, "crusader/metfire.wav");
	self.attack_finished=time + 0.7;
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	meteor.speed=1000;
	meteor.o_angle=normalize(v_forward);		
	meteor.velocity=meteor.o_angle*meteor.speed;
	meteor.veer=30;
	meteor.lifetime=time + 5;
	meteor.dmg=65;
	meteor.movetype=MOVETYPE_FLYMISSILE;
	org=self.origin+self.proj_ofs+v_forward*12;
	setsize(meteor,'0 0 0', '0 0 0');
	meteor.movedir = normalize(meteor.velocity);
	meteor.effects(+)EF_NODRAW;

	meteor.drawflags(+)MLS_FIREFLICKER;//|MLS_ABSLIGHT;


	if(self.classname=="tornato")
		meteor.owner=self.controller;
	else if(self.classname=="meteor")
		meteor.owner=self.owner;
	else
		meteor.owner=self;
	meteor.controller=self;

	meteor.solid=SOLID_BBOX;
	meteor.touch=MeteorTouch;

	setorigin(meteor,org);

	entity oldself;
	oldself = self;
	self = meteor;

	meteor.think = MeteorThink;
	meteor.think();

	self = oldself;

}

void MegaMeteorIgnite(void)
{
	float i;	
	vector startPos, endPos;
	entity hurtGuy;

	if(self.health == 6)
	{	// must've timed out or been in-air ignited...
		self.health = 5;
	}

	if(self.health == 5)
	{
		traceline(self.origin, self.origin + '0 0 -2000', FALSE, self);

		// only make meteors on the first frame
		WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
		WriteByte (MSG_MULTICAST, TE_METEOR_CRUSH);
		WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
		WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
		WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
		WriteLong (MSG_MULTICAST, 2000 * trace_fraction);
		multicast(self.origin,MULTICAST_PHS_R);

		self.effects (+) EF_NODRAW;
		self.think = MegaMeteorIgnite;

		self.velocity_x = 0;
		self.velocity_y = 0;
		self.velocity_z = 0;
	}

	i = 6; 

	while(i > 0)
	{
		startPos = self.origin;
		startPos_x += random(-80, 80);
		startPos_y += random(-80, 80);

		endPos = startPos;
		endPos_x += random(-90, 90)*4;
		endPos_y += random(-90, 90)*4;
		endPos_z += random(-1800, -1600)*4;

		//   simulate the meteor going through the air; simulate the meteor's width
		traceline (startPos, endPos, FALSE, self.owner);

//		if(trace_ent.takedamage)
//		{
//			T_Damage (trace_ent, self, self.owner, 40);
//		}
		hurtGuy=findradius(trace_endpos,90);
		while(hurtGuy)
		{
			T_Damage(hurtGuy, self, self, 50 * (1.0 - (vlen(hurtGuy.origin - trace_endpos)/90)));
			hurtGuy=hurtGuy.chain;
		}

		i-=1;
	}

	self.health -= 1;
	if(self.health == 0)
	{
		remove(self);
	}
	else
	{
		self.nextthink = time + 0.05;
	}
}

void MegaMeteorTouch(void)
{
	if(other == self.owner)
	{
		return;
	}

	if((other != world)&&(self.health != 6))
	{
		return;
	}

	if(self.health == 6)
	{
		self.health = 5;
		self.velocity_x = 0;
		self.velocity_y = 0;
		self.velocity_z = 1600;
		self.lifetime = time + 0.5;
		self.flags(-)FL_ONGROUND;
	}
	else
	{
		MegaMeteorIgnite();
	}

}

void MegaMeteorThink(void)
{
	if(self.lifetime < time)
	{
		self.health = 5;
		MegaMeteorIgnite();
		return;
		//self.health = 5;
	}

/*	if((self.owner.button0)&&(self.lifetime - time < 4.7))
	{	// if the owner tries firing again and the projectile's been around for at least .3 of a second...
		self.health = 5;
	}*/

	if(self.health == 5)
	{
		self.velocity_x = 0;
		self.velocity_y = 0;
		self.velocity_z = 1600;
		self.flags(-)FL_ONGROUND;
	}

	self.movedir = normalize(self.velocity);

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.velocity*.3, TRUE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_MEGAMETEOR);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);

	thinktime self : 0.3;

}

void FireMeteorTornado(void)
{
	vector org;
	entity meteor;

	meteor=spawn();
	setmodel(meteor,"models/tempmetr.mdl");
	meteor.scale = 2.3;

	if(self.classname=="player")
	{
		self.greenmana-=16;
		self.velocity+=normalize(v_forward)*-100;//include mass
		self.flags(-)FL_ONGROUND;
	}

	meteor.classname="meteor";
	self.punchangle_x = -6;
	weapon_sound(self, "crusader/metfire.wav");
	self.attack_finished=time + 1.5;
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	meteor.speed=1600;
	meteor.o_angle=normalize(v_forward);		
	meteor.velocity=meteor.o_angle*meteor.speed;
	meteor.lifetime=time + 5;
	meteor.th_die=MegaMeteorIgnite;
	meteor.nextthink = time + 0.1;
	meteor.dmg=60;
	meteor.movetype=MOVETYPE_FLYMISSILE;
	meteor.health = 6;

	setsize(meteor,'0 0 0', '0 0 0');
	meteor.movedir = normalize(meteor.velocity);
//	meteor.effects(+)EF_BRIGHTLIGHT|EF_TORNADO_EFFECT;
	meteor.effects(+)EF_NODRAW;
	meteor.drawflags(+)MLS_FIREFLICKER|DRF_TRANSLUCENT;//|MLS_ABSLIGHT;

	meteor.owner=self;
	//meteor.solid=SOLID_PHASE;
	meteor.solid=SOLID_BBOX;
	meteor.touch=MegaMeteorTouch;

	org=self.origin+self.proj_ofs+v_forward*12;
	setorigin(meteor,org);


	entity oldself;
	oldself = self;
	self = meteor;

	meteor.think = MegaMeteorThink;
	meteor.think();

	self = oldself;
}

void()meteor_ready_loop;
void() Cru_Met_Attack;

void meteor_power_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire9);
	self.th_weapon=meteor_power_fire;
	if(self.weaponframe==$fire2 && self.attack_finished<=time)
	{
		FireMeteorTornado();
	}

	if(self.wfs==WF_CYCLE_WRAPPED)
	{
			self.last_attack=time;
			meteor_ready_loop();
	}
}

void meteor_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire9);
	self.th_weapon=meteor_fire;

	if((!self.button0||self.attack_finished>time)&&self.wfs==WF_CYCLE_WRAPPED)
	{
		self.last_attack=time;
		meteor_ready_loop();
	}
	else if(self.weaponframe==$fire1 &&self.attack_finished<=time)
			FireMeteor("meteor");
}

void() Cru_Met_Attack =
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		self.th_weapon=meteor_power_fire;
	else
		self.th_weapon=meteor_fire;
	thinktime self : 0;
};

void meteor_ready_loop (void)
{
	self.weaponframe = $idle;
	self.th_weapon=meteor_ready_loop;
}

void meteor_select (void)
{
//go to ready loop, not relaxed?
	self.wfs = advanceweaponframe($Select1,$Select16);
	self.weaponmodel = "models/meteor.mdl";
	self.th_weapon=meteor_select;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		meteor_ready_loop();
	}
}

void meteor_deselect (void)
{
	self.wfs = advanceweaponframe($Select16,$Select1);
	self.th_weapon=meteor_deselect;

	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

