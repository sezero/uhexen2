/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/icemace.hc,v 1.2 2007-02-07 16:57:06 sezero Exp $
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
		AwardExperience(forwhom,loser,loser.experience_value);
		loser.experience_value=0;
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
		if(other.flags&FL_COLDHEAL)//Had to take out cold heal, so cold resist
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
			if(loser.flags&FL_COLDHEAL)
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

