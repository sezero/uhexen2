/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/icemace.hc,v 1.1 2005-01-26 17:26:11 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\icestaff\final\icestaff.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\icestaff\final
$origin 10 -10 10
$base BASE skin
$skin skin
$skin skin2
$flags 0

//
$frame fire1a     fire1b    fire1c
$frame fire2a     fire2b    fire2c
$frame fire3a     fire3b    fire3c
$frame fire4a     fire4b    fire4c
$frame fire5a     fire5b    fire5c

//
$frame idle1      idle2      idle3      idle4      idle5      
$frame idle6      idle7      idle8      idle9      idle10     
$frame idle11     idle12     idle13     idle14     idle15     
$frame idle16

//
$frame power1      power2      power3      power4      power5      
$frame power6      power7      power8      power9

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     

void() IceCubeThink =
{
	if(self.freeze_time<time&&skill<3)
	{
		self.th_die();
		return;
	}

	if(self.colormap>144)
	{
		self.colormap-=1;
		self.abslight-=0.05;
	}
	else
	{
		self.colormap=0;
		self.abslight=0.5;
		self.skin=GLOBAL_SKIN_ICE;
	}

	if(random()<0.2&&random()<0.2)
		sound(self,CHAN_AUTO,"misc/drip.wav",1,ATTN_NORM);

	self.think=IceCubeThink;
	thinktime self : 0.1;
};

void (entity loser,entity forwhom) SnowJob=
{
//FIXME: Make gradual- person slows down then stops
entity oself;
	if(loser.solid==SOLID_BSP)
		return;

	if(coop)
		if(loser.classname=="player"&&forwhom.classname=="player")
			return;

	if(teamplay)
		if(loser.team==forwhom.team)
			return;

	if(loser.flags&FL_MONSTER&&loser.monsterclass>=CLASS_BOSS)
	{
        T_Damage(loser,self,forwhom,10);
		return;
	}

	sound(loser,CHAN_BODY,"crusader/frozen.wav",1,ATTN_NORM);
	loser.frozen=50;
    loser.oldskin=loser.skin;
	if(loser.classname!="player")
	{
		loser.colormap=159;		
		loser.thingtype=THINGTYPE_ICE;
		loser.freeze_time=time+5;
		if(loser.scale==0)
			loser.scale = 1;
		loser.lifetime=loser.scale;
		loser.o_angle=loser.mins;
		loser.v_angle=loser.maxs;
		loser.enemy=forwhom;
        loser.oldthink=loser.think;
        loser.think=IceCubeThink;
        thinktime loser : 0;
        loser.touch=SUB_Null;
		loser.th_pain=SUB_Null;
		loser.wait = time + 3;
		if(loser.angles_x==0&&loser.angles_z==0)
			loser.drawflags(+)SCALE_ORIGIN_BOTTOM;
		loser.oldmovetype=loser.movetype;
		loser.movetype=MOVETYPE_TOSS;
        loser.health=1;
		loser.deathtype="ice melt";
		loser.th_die=shatter;
//		AwardExperience(forwhom,loser,loser.experience_value);
		oself=self;
		self=loser;
		SUB_UseTargets();
		self=oself;
	}
	else
	{
		loser.credit_enemy=forwhom;//give credit to freezer
		loser.artifact_active(+)ARTFLAG_FROZEN;
		loser.colormap=159;		
		loser.thingtype=THINGTYPE_ICE;
        loser.o_angle=loser.v_angle;
        loser.pausetime = time + 10;
        loser.attack_finished = time + 10;
//Temp -turns screen blue
        loser.health=1;
		thinktime loser : 10;
//Prevent interruption? loser.th_pain=SUB_Null;
	}
    loser.flags(-)FL_FLY;
    loser.flags(-)FL_SWIM;
	if(loser.flags&FL_ONGROUND)
		loser.last_onground=time;
    loser.flags(-)FL_ONGROUND;
//need to be able to reverse this...
	loser.oldtouch=loser.touch;
	loser.touch=obj_push;
	loser.drawflags(+)DRF_TRANSLUCENT|MLS_ABSLIGHT;
	loser.abslight=1;
};

/*void FreezeDie()
{
	// FIXME - get all of this stuff over
	//particleexplosion(self.origin,14,10,10);
	//particle2(self.origin,'-10 -10 -10','10 10 10',145,14,5);
	//if(self.movechain!=world)
	//	remove(self.movechain);
	remove(self);
}*/

void remove_artflag ()
{
	if(self.enemy.frozen<=0)
	{
		self.enemy.artifact_active(-)self.artifact_active;
	}
	remove(self);
}

void() FreezeTouch=
{
	float hitType;
	float removeBall;

	removeBall = 0;

	if(other==self.owner)
		return;

	//starteffect(CE_ICE_HIT,self.origin-self.movedir*8);
    if(other.takedamage&&other.health&&other.frozen<=0&&other.thingtype==THINGTYPE_FLESH)//FIXME: Thingtype_flesh only
	{	// hit someone who MIGHT be frozen by this attack - should always be players
	    //sound (self, CHAN_BODY, "crusader/icehit.wav", 1, ATTN_NORM);

		hitType = 0;

		if(other.freeze_time<=time)
		{
			other.frozen=0;
		}
		other.freeze_time=time+2.5;
		other.frozen-=1.5;
		if(other.classname=="player")
		{
			other.artifact_active(+)ARTFLAG_FROZEN;
			newmis=spawn();
			newmis.enemy=other;
			newmis.artifact_active=ARTFLAG_FROZEN;
			newmis.think=remove_artflag;
			thinktime newmis : 0.3;
		}


		if(other.flags&FL_COLDHEAL)//Had to take out cold heal, so cold resist
		{
	        T_Damage(other,self,self.owner,9);
		}
		else if((other.health<=10||(other.classname=="player"&&other.frozen<=-7&&other.health<200))&&other.solid!=SOLID_BSP&&!other.artifact_active&ART_INVINCIBILITY&&other.thingtype==THINGTYPE_FLESH&&other.health<100)
		{
			SnowJob(other,self.owner);
		}
		else
		{
	        T_Damage(other,self,self.owner,20);
		}
		removeBall = 1;
	}
	else if(other.frozen<=0)
	{	// hit a wall, I think
	    //sound (self, CHAN_BODY, "crusader/icewall.wav", 1, ATTN_NORM);
		hitType = 1;
		T_RadiusDamage(self,self.owner,30,self.owner);
		self.touch=SUB_Null;
		//shatter();
		//remove(self);
		removeBall = 1;
	}
	else
	{	
		hitType = 1;
		//sound(self,CHAN_BODY,"misc/tink.wav",1,ATTN_NORM);
	}
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_ICEHIT);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
	WriteByte (MSG_MULTICAST, hitType);
	multicast(self.origin,MULTICAST_PVS);

	if(removeBall)
	{
		remove(self);
	}
};

void iceballThink(void)
{
	self.movedir = normalize(self.velocity);

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.movedir * 360.0, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_ICESHOT);
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

void()FireFreeze=
{
	
//vector    dir;
	self.bluemana-=3;
	self.attack_finished=time + 0.2;
	self.punchangle_x= -1;
	makevectors (self.v_angle);
    //sound (self, CHAN_WEAPON, "crusader/icefire.wav", 1, ATTN_NORM);
	weapon_sound(self, "crusader/icefire.wav");
	newmis = spawn ();
    newmis.owner = self;
	newmis.movetype = MOVETYPE_BOUNCEMISSILE;
	newmis.solid = SOLID_BBOX;
	
	newmis.th_die=shatter;
	newmis.deathtype="ice shatter";
	
	newmis.touch = FreezeTouch;
    newmis.classname = "snowball";
    newmis.speed = 1200;
	newmis.movedir=normalize(v_forward);
	newmis.angles = vectoangles(self.movedir);
    newmis.velocity = newmis.movedir * newmis.speed;
	newmis.scale = newmis.scale * 1.5;
	newmis.lifetime = time + 4;

    setmodel (newmis, "models/iceshot1.mdl");

    setsize (newmis, '0 0 0', '0 0 0');
    setorigin (newmis, self.origin+self.proj_ofs + v_forward*8);

	//newmis.effects(+)EF_ICEBALL_EFFECT;
	newmis.effects(+)EF_NODRAW;

	entity oldself;
	oldself = self;
	self = newmis;

	newmis.think = iceballThink;
	newmis.think();

	self = oldself;
};

void() blizzard_think=
{
	entity loser;
	vector dir, top;

	if(self.lifetime<time)
	{
		remove(self);
		return;
	}

	self.color=random(15);
	self.color=rint(self.color)+9*16+256;
	dir_x=random(-100,100);
	dir_y=random(-100,100);
	top = '0 0 128';
	top_x=top_y=self.level+1;

	if(self.level<128)
		self.level+=8;

	makevectors(self.angles);

	self.flags(-)FL_ONGROUND;

	loser=findradius(self.origin,self.level);
	while(loser)
	{
		if(loser.takedamage&&loser.health&&loser.frozen<=0&&loser!=self.owner&&loser.solid!=SOLID_BSP)
			if(loser.flags&FL_COLDHEAL)
				T_Damage(loser,self,self.owner,1);
			else
			{
				loser.frozen-=1.2;

				if((loser.frozen<-7||loser.health<15)&&loser.classname!="mjolnir"&&!loser.artifact_active&ART_INVINCIBILITY&&loser.thingtype==THINGTYPE_FLESH&&loser.health<100)
				{
					SnowJob(loser,self.owner);
				}
				else
				{
					T_Damage(loser,self,self.owner,12);
				}
			}
		loser=loser.chain;
	}
	thinktime self : 0.1;
};

void() make_blizzard =
{
//FIXME: check all sides to make correct size,
//so won't go through walls.  Use tracelines and
//use this info to set origin, size, and radius
//length.
//local vector sizeright,sizeleft,sizefront,sizeback,sizebottom,sizetop;
//Note: Limit to 2 per player.  If 3rd made, erase first.
//Sound
	entity found;
	entity newShot;
	float curShot = 0;

	makevectors(self.angles);

	if(other.frozen<=-2&&!other.artifact_active&ART_INVINCIBILITY&&other.thingtype==THINGTYPE_FLESH&&other.health<100)
	{
		if(other.frozen <= -2)
		{
			SnowJob(other,self.owner);
		}
		else
		{
			other.frozen -= 4;
		}
	}

	found=findradius(self.origin,256);
	while(found)
	{
		if(found.flags&FL_CLIENT&&found!=self.owner)
		{
			found.artifact_active(+)ARTFLAG_DIVINE_INTERVENTION;
			found.divine_time = time + HX_FRAME_TIME;
		}
		found=found.chain;
	}

	while(curShot < 2)
	{
		newShot = spawn();
		newShot.origin = self.origin;
		newShot.touch=SUB_Null;

		if(curShot == 0)
		{
			newShot.velocity = normalize(v_right)*180;
		}
		else
		{
			newShot.velocity = normalize(v_right)*-180;
		}
  		newShot.movetype = MOVETYPE_FLY;
  		newShot.solid = SOLID_NOT;
		newShot.classname="blizzard";
		newShot.angles_y=other.angles_y;

		newShot.effects (+) EF_ICESTORM_EFFECT;

		newShot.lifetime=time + 3;
		newShot.think=blizzard_think;
		thinktime newShot : 0;

		newShot.owner = self.owner;

		setmodel(newShot,"models/null.spr");
		setsize(newShot,'-48 -48 -32','48 48 32');
		newShot.hull=HULL_GOLEM;
		if(other.takedamage)
		{
			setorigin(newShot,(other.absmax+other.absmin)*0.5);
		}

		newShot.drawflags(+)SCALE_ORIGIN_CENTER;//DRF_TRANSLUCENT

		curShot += 1;
	}
	remove(self);
};

void()sparkle=
{
//	particle(self.origin,'0 0 0',random(-255,255),random(3,10));
//	particleexplosion(self.origin,152+random(7),10,10);
	particle2(self.origin,'-3 -3 -3','3 3 3',144+random(15),2,20);
	particle2(self.origin,'-1 -1 -1','1 1 1',24+random(7),3,10);
	self.think = sparkle;
	thinktime self : 0.01;
};

void FireBlizzard (void)
{
	self.attack_finished=time + 1.2;
	self.bluemana-=10;
    //sound (self, CHAN_WEAPON, "crusader/blizfire.wav", 1, ATTN_NORM);
	weapon_sound(self, "crusader/blizfire.wav");
	newmis=spawn();
	newmis.owner=self;
	self.blizzcount+=1;
	newmis.blizzcount=self.blizzcount;
	//newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.movetype = MOVETYPE_TOSS;
	newmis.solid=SOLID_BBOX;
	newmis.touch=make_blizzard;

	newmis.velocity=normalize(v_forward)*1000;
	newmis.velocity=newmis.velocity + v_up*100;
	newmis.movedir = normalize(newmis.velocity);
	newmis.angles = vectoangles(newmis.movedir);
	newmis.effects=EF_MUZZLEFLASH;
	
	newmis.drawflags(+)SCALE_ORIGIN_CENTER;//DRF_TRANSLUCENT
	setmodel(newmis,"models/ball.mdl");

	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs+v_forward*16);
	newmis.scale = .1;
}

/*======================
ACTION
select
deselect
ready loop
relax loop
fire once
fire loop
ready to relax(after short delay)
relax to ready(Fire delay?  or automatic if see someone?)
=======================*/

void() Cru_Ice_Fire;
void icestaff_idle (void)
{
	self.th_weapon=icestaff_idle;
	self.wfs = advanceweaponframe($idle1,$idle16);
}

void icestaff_select (void)
{
	self.th_weapon=icestaff_select;
	self.wfs = advanceweaponframe($select1,$select10);
	self.weaponmodel = "models/icestaff.mdl";
	if (self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		icestaff_idle();
	}
}

void icestaff_deselect (void)
{
	self.wfs = advanceweaponframe($select10,$select1);
	self.th_weapon=icestaff_deselect;

	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}


void()icestaff_shard;
void icestaff_blizzard (void)
{
	self.th_weapon=icestaff_blizzard;
	self.wfs = advanceweaponframe($power1 , $power9);
	if(self.wfs==WF_CYCLE_WRAPPED)
		icestaff_idle();
	else if(self.weaponframe==$power1)
		if(self.artifact_active&ART_TOMEOFPOWER)
			FireBlizzard();
		else
			icestaff_shard();
}

void icestaff_f1 (void)
{
	self.th_weapon=icestaff_f1;
	self.wfs = advanceweaponframe($fire1a,$fire1c);
	if (self.wfs==WF_CYCLE_WRAPPED)
		if(!self.button0)
			icestaff_idle();
		else
			icestaff_shard();
	else if (self.attack_finished<=time&&self.weaponframe==$fire1a)
		if(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=10)
			icestaff_blizzard();
		else if(!self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=1)
			FireFreeze();
		else
			icestaff_idle();
}

void icestaff_f2 (void)
{
	self.th_weapon=icestaff_f2;
	self.wfs = advanceweaponframe($fire2a,$fire2c);
	if (self.wfs==WF_CYCLE_WRAPPED)
		if(!self.button0)
			icestaff_idle();
		else
			icestaff_shard();
	else if (self.attack_finished<=time&&self.weaponframe==$fire2a)
		if(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=10)
			icestaff_blizzard();
		else if(!self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=1)
			FireFreeze();
		else
			icestaff_idle();
}

void icestaff_f3 (void)
{
	self.th_weapon=icestaff_f3;
	self.wfs = advanceweaponframe($fire3a,$fire3c);
	if (self.wfs==WF_CYCLE_WRAPPED)
		if(!self.button0)
			icestaff_idle();
		else
			icestaff_shard();
	else if (self.attack_finished<=time&&self.weaponframe==$fire3a)
		if(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=10)
			icestaff_blizzard();
		else if(!self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=1)
			FireFreeze();
		else
			icestaff_idle();
}

void icestaff_f4 (void)
{
	self.th_weapon=icestaff_f4;
	self.wfs = advanceweaponframe($fire4a,$fire4c);
	if (self.wfs==WF_CYCLE_WRAPPED)
		if(!self.button0)
			icestaff_idle();
		else
			icestaff_shard();
	else if (self.attack_finished<=time&&self.weaponframe==$fire4a)
		if(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=10)
			icestaff_blizzard();
		else if(!self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=1)
			FireFreeze();
		else
			icestaff_idle();
}

void icestaff_f5 (void)
{
	self.th_weapon=icestaff_f5;
	self.wfs = advanceweaponframe($fire5a,$fire5c);
	if (self.wfs==WF_CYCLE_WRAPPED)
		if(!self.button0)
			icestaff_idle();
		else
			icestaff_shard();
	else if (self.attack_finished<=time&&self.weaponframe==$fire5a)
		if(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=10)
			icestaff_blizzard();
		else if(!self.artifact_active&ART_TOMEOFPOWER&&self.bluemana>=1)
			FireFreeze();
		else
			icestaff_idle();
}

void icestaff_shard (void)
{
float r;
	r=rint(random(4)) + 1;
	if(r==1)
		icestaff_f1();
	else if(r==2)
		icestaff_f2();
	else if(r==3)
		icestaff_f3();
	else if(r==4)
		icestaff_f4();
	else
		icestaff_f5();
}

void Cru_Ice_Fire (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		icestaff_blizzard();
	else
		icestaff_shard();
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 19    6/01/98 2:49a Mgummelt
 * 
 * 18    5/25/98 1:38p Mgummelt
 * 
 * 17    4/30/98 6:01p Mgummelt
 * 
 * 15    4/10/98 10:58a Nalbury
 * moved some stuff from the phs to the pvs...
 * 
 * 14    4/09/98 2:07p Nalbury
 * Made powered up meteor staff easier to use and powered up icemace a bit
 * less potent...
 * 
 * 13    4/07/98 9:07p Nalbury
 * No more reliable junk...
 * 
 * 12    3/30/98 7:07a Nalbury
 * Cleaned up the movement for several projectiles...
 * 
 * 11    3/29/98 6:47p Rmidthun
 * 
 * 10    3/23/98 5:04p Nalbury
 * 
 * 9     3/12/98 12:21p Nalbury
 * Using weapon sound instead of regular sound.
 * 
 * 8     3/05/98 4:41p Nalbury
 * 
 * 7     3/05/98 2:58p Nalbury
 * Ice mace balanced more
 * 
 * 6     3/04/98 5:52a Nalbury
 * Changed behaviour of powered up icemace.
 * 
 * 5     2/27/98 5:05p Nalbury
 * Added ice mace flag
 * 
 * 4     2/23/98 6:25p Nalbury
 * pulled ice shards and meteors out of wall
 * 
 * 3     2/20/98 11:56a Nalbury
 * Fixed up Ice Mace
 * 
 * 2     2/18/98 6:23p Nalbury
 * Made icemace better
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 95    9/25/97 4:06p Mgummelt
 * 
 * 94    9/19/97 8:47a Rlove
 * 
 * 93    9/11/97 12:02p Mgummelt
 * 
 * 92    9/02/97 7:54p Mgummelt
 * 
 * 91    9/01/97 8:18p Mgummelt
 * 
 * 90    8/31/97 2:36p Mgummelt
 * 
 * 89    8/31/97 11:38a Mgummelt
 * To which I say- shove where the sun don't shine- sideways!  Yeah!
 * How's THAT for paper cut!!!!
 * 
 * 88    8/31/97 3:42a Mgummelt
 * 
 * 87    8/31/97 3:41a Mgummelt
 * 
 * 86    8/30/97 6:58p Mgummelt
 * 
 * 85    8/28/97 8:49p Mgummelt
 * 
 * 84    8/27/97 7:07p Mgummelt
 * 
 * 83    8/26/97 6:01p Mgummelt
 * 
 * 82    8/26/97 7:38a Mgummelt
 * 
 * 81    8/25/97 11:32p Mgummelt
 * 
 * 80    8/25/97 4:42p Mgummelt
 * 
 * 79    8/22/97 5:15p Mgummelt
 * 
 * 78    8/19/97 8:14p Mgummelt
 * 
 * 77    8/19/97 12:57p Mgummelt
 * 
 * 76    8/19/97 12:22a Mgummelt
 * 
 * 75    8/17/97 3:45p Mgummelt
 * 
 * 74    8/16/97 4:26p Mgummelt
 * 
 * 73    8/15/97 11:27p Mgummelt
 * 
 * 72    8/15/97 4:25p Mgummelt
 * 
 * 71    8/12/97 6:10p Mgummelt
 * 
 * 70    8/09/97 1:49a Mgummelt
 * 
 * 69    8/08/97 6:21p Mgummelt
 * 
 * 68    8/08/97 3:34p Mgummelt
 * 
 * 67    8/05/97 4:10p Mgummelt
 * 
 * 66    8/04/97 6:21p Mgummelt
 * 
 * 65    7/31/97 11:23p Mgummelt
 * 
 * 64    7/31/97 12:57a Mgummelt
 * 
 * 63    7/31/97 12:33a Mgummelt
 * 
 * 62    7/30/97 11:22p Mgummelt
 * 
 * 61    7/30/97 10:43p Mgummelt
 * 
 * 60    7/30/97 8:27p Mgummelt
 * 
 * 59    7/30/97 3:33p Mgummelt
 * 
 * 58    7/28/97 7:50p Mgummelt
 * 
 * 57    7/28/97 1:51p Mgummelt
 * 
 * 56    7/26/97 8:38a Mgummelt
 * 
 * 55    7/26/97 2:17a Mgummelt
 * 
 * 54    7/25/97 10:19p Mgummelt
 * 
 * 53    7/25/97 8:39p Mgummelt
 * 
 * 52    7/25/97 4:03p Mgummelt
 * 
 * 51    7/25/97 3:43p Mgummelt
 * 
 * 50    7/25/97 3:32p Mgummelt
 * 
 * 49    7/25/97 2:56p Mgummelt
 * 
 * 48    7/24/97 12:34p Mgummelt
 * 
 * 47    7/24/97 12:32p Mgummelt
 * 
 * 46    7/24/97 3:26a Mgummelt
 * 
 * 45    7/22/97 7:36p Bgokey
 * 
 * 44    7/22/97 11:47a Mgummelt
 * 
 * 43    7/21/97 7:03p Mgummelt
 * 
 * 42    7/21/97 4:03p Mgummelt
 * 
 * 41    7/21/97 4:02p Mgummelt
 * 
 * 40    7/19/97 9:53p Mgummelt
 * 
 * 39    7/16/97 10:57p Mgummelt
 * 
 * 38    7/15/97 8:30p Mgummelt
 * 
 * 37    7/14/97 9:30p Mgummelt
 * 
 * 36    7/10/97 7:21p Mgummelt
 * 
 * 35    7/09/97 6:31p Mgummelt
 * 
 * 34    7/01/97 3:30p Mgummelt
 * 
 * 33    7/01/97 2:21p Mgummelt
 * 
 * 32    6/30/97 5:38p Mgummelt
 * 
 * 31    6/19/97 4:03p Rjohnson
 * removed crandom()
 * 
 * 30    6/18/97 6:39p Mgummelt
 * 
 * 29    6/18/97 4:00p Mgummelt
 * 
 * 28    6/16/97 4:00p Mgummelt
 * 
 * 27    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 * 26    5/30/97 10:04p Mgummelt
 * 
 * 25    5/28/97 8:13p Mgummelt
 * 
 * 24    5/22/97 2:50a Mgummelt
 * 
 * 23    5/19/97 11:36p Mgummelt
 * 
 * 22    5/16/97 11:28p Mgummelt
 * 
 * 21    5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 20    5/15/97 5:05a Mgummelt
 * 
 * 19    5/08/97 5:47p Mgummelt
 * 
 * 18    5/07/97 3:40p Mgummelt
 * 
 * 17    5/06/97 1:29p Mgummelt
 * 
 * 16    5/05/97 4:48p Mgummelt
 * 
 * 15    5/02/97 8:06p Mgummelt
 * 
 * 14    4/28/97 6:53p Mgummelt
 * 
 * 13    4/28/97 1:10p Mgummelt
 * 
 * 12    4/25/97 8:32p Mgummelt
 * 
 * 11    4/24/97 2:21p Mgummelt
 * 
 * 10    4/21/97 8:47p Mgummelt
 * 
 * 9     4/21/97 6:15p Mgummelt
 * 
 * 8     4/21/97 12:31p Mgummelt
 * 
 * 7     4/18/97 8:22p Mgummelt
 * 
 * 6     4/18/97 5:24p Mgummelt
 * 
 * 5     4/18/97 11:44a Rlove
 * changed advanceweaponframe to return frame state
 * 
 * 4     4/17/97 9:12p Mgummelt
 * 
 * 3     4/17/97 2:50p Mgummelt
 * 
 * 2     4/14/96 3:52p Mgummelt
 * 
 * 1     4/14/96 3:51p Mgummelt
 */
