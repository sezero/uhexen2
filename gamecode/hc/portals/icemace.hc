/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/icemace.hc,v 1.1.1.1 2004-11-29 11:32:49 sezero Exp $
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

void chain_remove ()
{
	if(self.movechain.movechain!=world)
		remove(self.movechain.movechain);
	if(self.movechain!=world)
		remove(self.movechain);
	remove(self);
}

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

	if(loser.flags&FL_GODMODE)
		return;

	loser.frozen=50;
    loser.oldskin=loser.skin;
	if(loser.classname!="player")
	{
		if(loser.gravity!=loser.standard_grav)
			loser.gravity=loser.standard_grav;
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
		AwardExperience(forwhom,loser,loser.experience_value);
		loser.experience_value=	loser.init_exp_val = 0;
		oself=self;
		self=loser;
		SUB_UseTargets();
		self=oself;
	}
	else
	{
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

void FreezeDie()
{
	particleexplosion(self.origin,14,10,10);
	particle2(self.origin,'-10 -10 -10','10 10 10',145,14,5);
	if(self.movechain!=world)
		remove(self.movechain);
	remove(self);
}

void remove_artflag ()
{
	if(self.enemy.frozen<=0)
		self.enemy.artifact_active(-)self.artifact_active;
	remove(self);
}

void() FreezeTouch=
{
	if(other==self.owner)
		return;

	starteffect(CE_ICE_HIT,self.origin-self.movedir*8);
    if(other.takedamage&&other.health&&other.frozen<=0&&other.thingtype==THINGTYPE_FLESH)//FIXME: Thingtype_flesh only
	{
	    sound (self, CHAN_BODY, "crusader/icehit.wav", 1, ATTN_NORM);
		if(other.frozen<=0)
		{
			if(other.freeze_time<=time)
				other.frozen=0;
			other.freeze_time=time+2.5;
			other.frozen-=1;
			if(other.classname=="player")
			{
				other.artifact_active(+)ARTFLAG_FROZEN;
				newmis=spawn();
				newmis.enemy=other;
				newmis.artifact_active=ARTFLAG_FROZEN;
				newmis.think=remove_artflag;
				thinktime newmis : 0.1;
			}
		}
		if(other.flags2&FL2_COLDHEAL)//Had to take out cold heal, so cold resist
	        T_Damage(other,self,self.owner,3);
		else if((other.health<=10||(other.classname=="player"&&other.frozen<=-5&&other.health<200))&&other.solid!=SOLID_BSP&&!other.artifact_active&ART_INVINCIBILITY&&other.thingtype==THINGTYPE_FLESH&&other.health<100)
			SnowJob(other,self.owner);
		else
	        T_Damage(other,self,self.owner,10);
		self.think=FreezeDie;
		thinktime self : 0;
	}
	else if(other.frozen<=0)
	{
	    sound (self, CHAN_BODY, "crusader/icewall.wav", 1, ATTN_NORM);
		T_RadiusDamage(self,self.owner,30,self.owner);
		self.touch=SUB_Null;
		shatter();
	}
	else
		sound(self,CHAN_BODY,"misc/tink.wav",1,ATTN_NORM);
};

void()FireFreeze=
{
//vector    dir;
	self.bluemana-=1;
	self.attack_finished=time + 0.05;
	self.punchangle_x= -1;
	makevectors (self.v_angle);
    sound (self, CHAN_WEAPON, "crusader/icefire.wav", 1, ATTN_NORM);
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
    newmis.velocity = newmis.movedir * newmis.speed;
	newmis.angles = vectoangles(newmis.velocity);
	newmis.avelocity=randomv('-600 -600 -600','600 600 600');

    setmodel (newmis, "models/iceshot1.mdl");
	newmis.drawflags=MLS_ABSLIGHT;
	newmis.abslight=0.5;
	newmis.think=chain_remove;
	thinktime newmis : 3;

    setsize (newmis, '0 0 0', '0 0 0');
    setorigin (newmis, self.origin+self.proj_ofs + v_forward*8);

entity corona;
	corona=spawn();
	newmis.movechain=corona;
	corona.movetype=MOVETYPE_NOCLIP;
	corona.avelocity=randomv('-600 -600 -600','600 600 600');
	corona.drawflags=DRF_TRANSLUCENT|MLS_ABSLIGHT;
	corona.abslight=0.5;
	corona.scale=2;
	corona.think=SUB_Remove;
	thinktime corona : 2;
	setmodel(corona,"models/iceshot2.mdl");
	setsize(corona,'0 0 0','0 0 0');
	setorigin(corona,newmis.origin);
};

/*
void shard_hit (void)
{
	if(other.classname=="blizzard shard")
		return;

	if(other.takedamage&&other.health&&other!=self.owner)
		T_Damage(other,self,self.owner,50*self.scale);
	sound(self,CHAN_AUTO,"crusader/icewall.wav",0.1,ATTN_NORM);
	particleexplosion(self.origin,14,20,5);
//	particle2(self.origin,'-10 -10 -10','10 10 10',145,14,5);
	remove(self);
}
*/

/*
void FireShard (void)
{
local vector org,dir;
	
		newmis=spawn();
		newmis.movetype=MOVETYPE_BOUNCE;
		newmis.solid=SOLID_TRIGGER;
		newmis.owner=self.owner;

		dir_x=random(50,100);
		dir_y=random(50,100);
		dir_z=random(-250,-180);

		org_x= random(-84,-8);
		org_y= random(-84,-8);
		if(org_x<64)
			org_z=64+org_x;
		else if(org_y<64)
			org_z=64+org_y;
		org_x+=self.origin_x;
		org_y+=self.origin_y;
		org_z+=self.origin_z+64;
		traceline(self.origin,org,TRUE,self);
		org=trace_endpos;

		newmis.velocity=dir;
		newmis.angles=vectoangles(newmis.velocity)+'90 0 0';
		newmis.scale=random(0.05,0.55);
		newmis.skin=0;
		newmis.frame=0;
		newmis.touch=shard_hit;

		setmodel(newmis,"models/shard.mdl");
		setsize(newmis,'0 0 0','0 0 0');
		setorigin(newmis,org);
}
*/	
void() blizzard_think=
{
entity loser;
vector dir, top, bottom, beam_angle;
float beam_count; 
	if(self.lifetime<time||self.blizzcount<self.owner.blizzcount - 1)
	{
		remove(self);
		return;
	}

//	if(self.pain_finished<=time)
//		self.effects=EF_NODRAW;

	self.color=random(15);
	self.color=rint(self.color)+9*16+256;
	dir_x=random(-100,100);
	dir_y=random(-100,100);
	top = '0 0 128';
	top_x=top_y=self.level+1;
	rain_go(self.origin-top,self.origin+top,top*2,dir,self.color,77);
	if(self.t_width<time)
	{
		sound(self,CHAN_WEAPON,"crusader/blizzard.wav",1,ATTN_NORM);
		self.t_width=time + 0.7;
	}

	if(self.level<128)
		self.level+=8;

	makevectors(self.angles);
	beam_count=6;
	while(beam_count)
	{
		beam_angle=self.angles;
		beam_angle_y=self.angles_y+60*(6 - beam_count);
		makevectors(beam_angle);
		traceline(self.origin,self.origin+v_forward*self.level,TRUE,self);
		dir=trace_endpos-v_forward*8;
		traceline(dir,dir+'0 0 128',TRUE,self);
		top=trace_endpos;
		traceline(dir,dir-'0 0 128',TRUE,self);
		bottom=trace_endpos;

		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_STREAM_ICECHUNKS);
		WriteEntity (MSG_BROADCAST, self);
		WriteByte (MSG_BROADCAST, beam_count+STREAM_ATTACHED);
		WriteByte (MSG_BROADCAST, 2);
		WriteCoord (MSG_BROADCAST, top_x);
		WriteCoord (MSG_BROADCAST, top_y);
		WriteCoord (MSG_BROADCAST, top_z);
		WriteCoord (MSG_BROADCAST, bottom_x);
		WriteCoord (MSG_BROADCAST, bottom_y);
		WriteCoord (MSG_BROADCAST, bottom_z);

		beam_count-=1;
		traceline(top,bottom,FALSE,self.owner);
		if(trace_ent.takedamage&&trace_ent.frozen<=0&&trace_ent.solid!=SOLID_BSP)
		{
			trace_ent.frozen-=1;
			if((trace_ent.frozen<-5||trace_ent.health<=10)&&!trace_ent.artifact_active&ART_INVINCIBILITY&&trace_ent.thingtype==THINGTYPE_FLESH&&trace_ent.health<100)
				SnowJob(trace_ent,self.owner);
			else
				T_Damage(trace_ent,self,self.owner,10);
		}
	}
	if(random()<0.3)
		self.velocity=randomv('-200 -200 0','200 200 0');
	self.avelocity_y=500;
	self.flags(-)FL_ONGROUND;

	loser=findradius(self.origin,self.level);
	while(loser)
	{
		if(loser.takedamage&&loser.health&&loser.frozen<=0&&loser!=self.owner&&loser.solid!=SOLID_BSP)
			if(loser.flags2&FL2_COLDHEAL)
				T_Damage(loser,self,self.owner,1);
			else
			{
				if(random()<0.1)
					loser.frozen-=1;
				if((loser.frozen<-5||loser.health<15)&&loser.classname!="mjolnir"&&!loser.artifact_active&ART_INVINCIBILITY&&loser.thingtype==THINGTYPE_FLESH&&loser.health<100)
					SnowJob(loser,self.owner);
				else
					T_Damage(loser,self,self.owner,5);
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

	if(other.frozen<=0&&!other.artifact_active&ART_INVINCIBILITY&&other.thingtype==THINGTYPE_FLESH&&other.health<100)
		SnowJob(other,self.owner);

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

	self.touch=SUB_Null;

	self.velocity=randomv('-200 -200 0','200 200 0');
  	self.movetype = MOVETYPE_FLY;
  	self.solid = SOLID_NOT;
	self.classname="blizzard";
//	self.effects=EF_NODRAW;
	self.angles_y=other.angles_y;
	self.avelocity_y=500;
	
	self.lifetime=time + 10;
	self.think=blizzard_think;
	thinktime self : 0;
	
	setmodel(self,"models/null.spr");
	setsize(self,'-32 -32 -32','32 32 32');
	self.hull=HULL_GOLEM;
	if(other.takedamage)
		setorigin(self,(other.absmax+other.absmin)*0.5);
//	self.pain_finished=time + 3;
//	self.effects = EF_BRIGHTLIGHT;
//	starteffect(18, self.origin);

//	traceline(self.origin,self.origin + '0 0 -64',TRUE,self);
//	setorigin(self,trace_endpos + '0 0 64');
//	shatter();
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
	self.attack_finished=time + 1;
	self.bluemana-=10;
    sound (self, CHAN_WEAPON, "crusader/blizfire.wav", 1, ATTN_NORM);
	newmis=spawn();
	newmis.owner=self;
	self.blizzcount+=1;
	newmis.blizzcount=self.blizzcount;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.touch=make_blizzard;

	newmis.velocity=normalize(v_forward)*1000;
	newmis.effects=EF_MUZZLEFLASH;
	
//	newmis.think=sparkle;
//	thinktime newmis : 0;

//	setmodel(newmis,"models/null.spr");
	newmis.drawflags(+)SCALE_ORIGIN_CENTER;//DRF_TRANSLUCENT
	setmodel(newmis,"models/ball.mdl");
//	newmis.effects(+)EF_NODRAW;
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
 * 8     3/18/98 3:49p Mgummelt
 * Last minute original game fixes, doors, eidolon, icemace, rats.
 * 
 * 7     3/03/98 7:31p Mgummelt
 * 
 * 6     2/18/98 4:59p Mgummelt
 * 
 * 5     2/12/98 5:55p Jmonroe
 * remove unreferenced funcs
 * 
 * 4     2/06/98 7:06p Mgummelt
 * 
 * 3     1/26/98 6:18p Mgummelt
 * 
 * 97    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
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
