/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/meteor.hc,v 1.1.1.1 2004-11-29 11:27:59 sezero Exp $
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:07  theoddone33
 * Inital import
 *
 * 
 * 32    4/10/98 10:58a Nalbury
 * moved some stuff from the phs to the pvs...
 * 
 * 31    4/09/98 2:07p Nalbury
 * Made powered up meteor staff easier to use and powered up icemace a bit
 * less potent...
 * 
 * 30    4/07/98 9:07p Nalbury
 * No more reliable junk...
 * 
 * 29    4/04/98 4:22p Nalbury
 * tweaked lightning some...
 * 
 * 28    4/04/98 5:47a Nalbury
 * Tweaked ALOT of damage amounts.  It's getting there...
 * 
 * 27    4/04/98 3:04a Nalbury
 * fixed up powered up meteor a biit...
 * 
 * 26    4/02/98 2:37p Ssengele
 * specifically set takedamage to no, although that didn't seem to help a
 * lot with the xbolts; also set health to 9999; something dies with
 * multiexplode as its th_die, and i want to make sure the meteors aren't
 * causing the other infrequent stack overflow.  only touched the
 * unpowered meteors.
 * 
 * 25    4/01/98 6:32p Nalbury
 * improved megamissile speed...
 * 
 * 24    3/31/98 5:27p Nalbury
 * Removed some unused flags.
 * 
 * 23    3/30/98 12:25a Nalbury
 * Did some blood rain stuff...
 * 
 * 22    3/27/98 4:09p Nalbury
 * Upped damage, reload time for powered up version...
 * 
 * 21    3/27/98 1:13a Nalbury
 * Got the meteor storm where I want it - still needs to be balanced...
 * 
 * 20    3/26/98 8:00p Nalbury
 * decreased powered sunstaff damage;  changed meteor effect
 * 
 * 19    3/23/98 5:33p Nalbury
 * Increased meteor staff damage.
 * 
 * 18    3/23/98 5:04p Nalbury
 * 
 * 17    3/12/98 7:49p Nalbury
 * Decreased damage for meteors and tornades; decreased tornado's range
 * 
 * 16    3/12/98 1:48p Nalbury
 * modified sound stuff.
 * 
 * 15    3/12/98 11:41a Nalbury
 * tornados can now go up stairs and stuff.
 * 
 * 14    3/10/98 3:00p Nalbury
 * Changed the amount tornados lift.
 * 
 * 13    3/09/98 5:11p Nalbury
 * changed purifier around
 * 
 * 12    3/09/98 2:41p Nalbury
 * Fixed more gib stuff
 * 
 * 11    3/07/98 2:49a Nalbury
 * worked on player gib stuff
 * 
 * 10    3/05/98 4:41p Nalbury
 * 
 * 9     3/05/98 2:58p Nalbury
 * Adjusted meteor stuff
 * 
 * 8     3/04/98 5:53a Nalbury
 * Removed unneeded code...  Added cost to tornados.
 * 
 * 7     3/02/98 11:01p Nalbury
 * Added more functionality to the tornados
 * 
 * 6     3/02/98 4:09p Nalbury
 * Messed around with the tornado alot...
 * 
 * 5     2/23/98 6:45p Nalbury
 * Really prevent the meteors from being in the wall...
 * 
 * 4     2/23/98 6:25p Nalbury
 * pulled ice shards and meteors out of wall
 * 
 * 3     2/20/98 4:22p Nalbury
 * Cheapened meteor stuff
 * 
 * 2     2/18/98 12:07p Nalbury
 * Changed meteors to temp ents.
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 77    9/11/97 12:02p Mgummelt
 * 
 * 76    9/02/97 7:54p Mgummelt
 * 
 * 75    9/01/97 8:18p Mgummelt
 * 
 * 74    9/01/97 7:09a Mgummelt
 * 
 * 73    9/01/97 5:58a Mgummelt
 * 
 * 72    9/01/97 5:13a Mgummelt
 * 
 * 71    9/01/97 3:08a Mgummelt
 * 
 * 70    8/31/97 6:55p Mgummelt
 * 
 * 69    8/30/97 6:58p Mgummelt
 * 
 * 68    8/30/97 3:05p Mgummelt
 * 
 * 67    8/28/97 2:41p Mgummelt
 * 
 * 66    8/26/97 6:01p Mgummelt
 * 
 * 65    8/26/97 7:38a Mgummelt
 * 
 * 64    8/26/97 2:26a Mgummelt
 * 
 * 63    8/25/97 11:41p Mgummelt
 * 
 * 62    8/25/97 4:15p Mgummelt
 * 
 * 61    8/22/97 5:15p Mgummelt
 * 
 * 60    8/12/97 6:10p Mgummelt
 * 
 * 59    8/08/97 6:21p Mgummelt
 * 
 * 58    8/07/97 10:30p Mgummelt
 * 
 * 57    8/06/97 10:19p Mgummelt
 * 
 * 56    8/04/97 8:03p Mgummelt
 * 
 * 55    7/30/97 10:43p Mgummelt
 * 
 * 54    7/29/97 5:44p Mgummelt
 * 
 * 53    7/28/97 7:50p Mgummelt
 * 
 * 52    7/28/97 1:51p Mgummelt
 * 
 * 51    7/26/97 8:39a Mgummelt
 * 
 * 50    7/24/97 4:06p Rlove
 * 
 * 49    7/24/97 3:53p Rlove
 * 
 * 48    7/24/97 3:26a Mgummelt
 * 
 * 47    7/21/97 4:04p Mgummelt
 * 
 * 46    7/21/97 4:02p Mgummelt
 * 
 * 45    7/21/97 11:45a Mgummelt
 * 
 * 44    7/19/97 9:53p Mgummelt
 * 
 * 43    7/18/97 3:55p Mgummelt
 * 
 * 42    7/15/97 8:31p Mgummelt
 * 
 * 41    7/14/97 9:30p Mgummelt
 * 
 * 40    7/10/97 7:21p Mgummelt
 * 
 * 39    7/09/97 6:31p Mgummelt
 * 
 * 38    7/01/97 3:30p Mgummelt
 * 
 * 37    7/01/97 2:21p Mgummelt
 * 
 * 36    6/30/97 5:38p Mgummelt
 * 
 * 35    6/23/97 4:50p Mgummelt
 * 
 * 34    6/18/97 7:06p Mgummelt
 * 
 * 33    6/18/97 4:19p Mgummelt
 * 
 * 32    6/18/97 4:00p Mgummelt
 * 
 * 31    6/16/97 4:00p Mgummelt
 * 
 * 30    6/15/97 5:10p Mgummelt
 * 
 * 29    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 * 28    6/04/97 8:16p Mgummelt
 * 
 * 27    6/03/97 12:35p Mgummelt
 * 
 * 26    5/31/97 3:59p Mgummelt
 * 
 * 25    5/28/97 8:24p Mgummelt
 * 
 * 23    5/23/97 2:54p Mgummelt
 * 
 * 22    5/22/97 7:05p Mgummelt
 * 
 * 21    5/22/97 2:50a Mgummelt
 * 
 * 20    5/20/97 9:36p Mgummelt
 * 
 * 19    5/19/97 11:36p Mgummelt
 * 
 * 18    5/19/97 12:06p Mgummelt
 * 
 * 17    5/17/97 8:45p Mgummelt
 * 
 * 16    5/16/97 11:27p Mgummelt
 * 
 * 15    5/15/97 8:28p Mgummelt
 * 
 * 14    5/15/97 2:44p Mgummelt
 * 
 * 13    5/15/97 5:04a Mgummelt
 * 
 * 12    5/15/97 3:45a Mgummelt
 * 
 * 10    5/12/97 10:37a Rlove
 * 
 * 9     5/06/97 1:29p Mgummelt
 * 
 * 8     5/05/97 10:09p Mgummelt
 * 
 * 7     5/05/97 4:48p Mgummelt
 * 
 * 6     4/28/97 6:53p Mgummelt
 * 
 * 5     4/25/97 8:32p Mgummelt
 * 
 * 4     4/24/97 2:21p Mgummelt
 * 
 * 3     4/21/97 12:31p Mgummelt
 * 
 * 2     4/17/97 1:45p Mgummelt
 * 
 * 1     4/17/97 12:13p Mgummelt
 */
