/*
===============================================================================
LIGHTNING.HC
MG

Lightning and Sunbeam effects, and thunderstorm
===============================================================================
*/
void smolder_think ()
{
vector ofs;
	ofs_x=random(-10,10);
	ofs_y=random(-10,10);
	particle(self.origin+ofs, '0 0 100', random(272,288), random(1,10));
	if(random()<0.1&&random()<0.5)
		CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
	thinktime self : 0.1;
	if(time>self.lifetime)
		remove(self);
}

void smolder (vector org)
{
//	starteffect(CE_SMOLDER,org);
	return;//Magical Network-Friendly Code!
	newmis=spawn();
	setorigin(newmis,org);
	newmis.effects=EF_NODRAW;
	newmis.lifetime=time+7;
	newmis.think=smolder_think;
	thinktime newmis : 0;
}

/*
void shock_think()
{
	if (self.skin ==0)
		self.skin = 1;
	else
		self.skin = 0;
	self.scale-=0.1;

	thinktime self : 0.05;
	if(time>self.lifetime||self.scale<=0.1)
		remove(self);
}

void spawnshockball (vector org)
{
	newmis=spawn();
	newmis.drawflags(+)MLS_TORCH;
	setmodel (newmis, "models/vorpshok.mdl");
	setorigin(newmis, org);
	newmis.lifetime=time+1;
	newmis.angles_z=90;
	newmis.think=shock_think;
	thinktime newmis : 0;
	newmis.scale=2.5;
}
*/

/*
=================
LightningDamage
=================
*/

void (vector endpos) ThroughWaterZap =
{
	entity waterloser, attacker;
	float damg;
	waterloser = spawn();
	setorigin (waterloser, endpos);
	if(self.classname=="mjolnir")
		damg=128;
	else
		damg=666*2;
	attacker=self;
	if(self.classname!="player")
	{
		if(self.owner.classname=="player")
			attacker=self.owner;
		else if(self.controller.classname=="player")
			attacker=self.controller;
	}
	T_RadiusDamageWater (waterloser, attacker, damg,self);
	remove (waterloser);
};

void (vector startpos) ThroughWater =
{
vector endpos;
float mover;
	mover = 600;
    while (mover)
    {
        mover = mover - 10;
        endpos = startpos + v_forward * mover;
        if (pointcontents(endpos) == CONTENT_WATER || pointcontents(endpos) == CONTENT_SLIME)
                ThroughWaterZap(endpos);
        else if (pointcontents(endpos) == CONTENT_SOLID)
                return;
    }
};

void do_lightning_dam (entity from, float damage, string type)
{
vector loser_org;
	if((trace_ent.classname=="buddha_shield"||trace_ent.classname=="monster_buddha")&&from.classname=="monster_buddha"&&type=="lightning")
		return;
	
	if((trace_ent.classname=="monster_eidolon"||trace_ent.classname=="obj_chaos_orb")&&type=="lightning")
		return;

	particle (trace_endpos, '0 0 100', 225, damage*4);
	if(type=="lightning")
//		spawnshockball((trace_ent.absmax+trace_ent.absmin)*0.5);
		starteffect(CE_LSHOCK,(trace_ent.absmax+trace_ent.absmin)*0.5);
	loser_org=trace_ent.origin;
    T_Damage (trace_ent, from, from, damage);
	if(trace_ent.health<=0)
		smolder(loser_org);
	if(type=="lightning")
		sound(trace_ent,CHAN_AUTO,"misc/lighthit.wav",1,ATTN_NORM);
	else
		sound(trace_ent,CHAN_AUTO,"crusader/sunhit.wav",1,ATTN_NORM);
}

void(vector p1, vector p2, entity from, float damage,string type) LightningDamage =
{
entity	e1, e2;// swap;
vector	f;
float	inertia;//absorb;
	f = p2 - p1;
	normalize (f);
	f_x = 0 - f_y;
	f_y = f_x;
	f_z = 0;
	f = f*16;

	e1 = e2 = world;

	traceline (p1, p2, FALSE, self);

        if(type=="lightning"&&(pointcontents(trace_endpos) == CONTENT_WATER || pointcontents(trace_endpos) == CONTENT_SLIME))
			ThroughWaterZap(trace_endpos);
        else if(type=="lightning"&&(trace_ent.watertype == CONTENT_WATER || trace_ent.watertype == CONTENT_SLIME))
			T_RadiusDamageWater (self, self, 666*2,self);
		else if(self.classname=="mjolnir"&&trace_ent==self.controller)
			bprint("");
        else if (trace_ent.takedamage)
		{
			if (trace_ent.mass<=10)
				inertia=1;
			else 
				inertia = trace_ent.mass/10;
			do_lightning_dam(from,damage,type);
            if (self.classname=="mjolnir"&&(trace_ent.flags&FL_ONGROUND)&&type=="lightning")
            {
	            trace_ent.velocity_z = trace_ent.velocity_z + 400/inertia;
	            trace_ent.flags(-)FL_ONGROUND;
            }
		}
        else if(type=="lightning") 
			ThroughWater(p1);

        e1 = trace_ent;
		traceline (p1 + f, p2 + f, FALSE, self);
		if(self.classname=="mjolnir"&&trace_ent==self.controller)
			bprint("");
        else if(trace_ent != e1 && trace_ent.takedamage)
			do_lightning_dam(from,damage,type);

		e2 = trace_ent;
		traceline (p1 - f, p2 - f, FALSE, self);
		if(self.classname=="mjolnir"&&trace_ent==self.controller)
			bprint("");
        else if (trace_ent != e1 && trace_ent != e2 && trace_ent.takedamage)
			do_lightning_dam(from,damage,type);
};

void do_lightning (entity lowner,float tag, float lflags, float duration, vector spot1, vector spot2, float ldamg,float te_type)
{
vector damage_dir;
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, te_type);
	WriteEntity (MSG_BROADCAST, lowner);
	WriteByte (MSG_BROADCAST, tag+lflags);
	WriteByte (MSG_BROADCAST, duration);
	
	WriteCoord (MSG_BROADCAST, spot1_x);
	WriteCoord (MSG_BROADCAST, spot1_y);
	WriteCoord (MSG_BROADCAST, spot1_z);

	WriteCoord (MSG_BROADCAST, spot2_x);
	WriteCoord (MSG_BROADCAST, spot2_y);
	WriteCoord (MSG_BROADCAST, spot2_z);

	if(ldamg)
	{
		if(self.owner.classname=="player")
			lowner=self.owner;
		else if(self.controller.classname=="player")
			lowner=self.controller;	

		damage_dir=normalize(spot2-spot1);
		LightningDamage (spot1-damage_dir*15, spot2+damage_dir*15, lowner, ldamg,"lightning");
	}
}

void(float max_strikes, float damg) CastLightning =
{
//Not working, I want 3 seperate beams, when get that, drop damage to 10
vector  org, dir,tospot;
float number_strikes;

	self.effects(+)EF_MUZZLEFLASH;

	if(max_strikes==0)
		max_strikes=1;
	while(max_strikes>number_strikes)
	{
		if(random()<0.7)
			sound(self.enemy,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
		else
			sound(self.enemy,CHAN_AUTO,"crusader/lghtn2.wav",1,ATTN_NORM);
		if(self.enemy.solid==SOLID_BSP&&self.enemy.origin=='0 0 0')
			org=(self.enemy.absmin+self.enemy.absmax)*0.5;
		else
		{
			org=self.enemy.origin;
			org_z += 0.5*self.enemy.maxs_z;
		}
		dir=org;
		dir_x+= random(-300,300);
		dir_y+= random(-300,300);
		dir_z+= 500;
		traceline(org,dir,TRUE,self);
		tospot=org;
		org=trace_endpos;
		do_lightning (self,number_strikes,0,4,org,tospot,damg,TE_STREAM_LIGHTNING);
		number_strikes+=1;
	}
};

void()rolling_thunder;
void thunder_clear ()
{
	self.owner.aflag-=1;
	remove(self);
}

void thunder_sound ()
{
float sound_vol;
	sound_vol=self.lightvalue2/25;
	if(sound_vol>1)
		sound_vol=1;
	else if(sound_vol<0)
		sound_vol=0.1;
	sound (self, CHAN_VOICE, "ambience/thunder1.wav", sound_vol, ATTN_NORM);
	thinktime self : 5;
	self.think=thunder_clear;
}

void spawn_thunder ()
{
	self.aflag+=1;
	newmis=spawn();
	self.angles_y=random(360);
	makevectors(self.angles);
	setorigin(newmis,self.origin+v_forward*self.lightvalue2*10);
	newmis.owner=self;
	newmis.lightvalue2=self.lightvalue2;
	newmis.think=thunder_sound;
	thinktime newmis : 2.5 - self.lightvalue2/10;
}

void flash_wait ()
{
	lightstylestatic(self.style,self.lightvalue1);
	self.think=rolling_thunder;
	thinktime self : 0;
}

void lightning_strike (void)
{
vector org,tospot, lightn_dir;
float dist, num_branches;
	dist=random(self.frags);
	self.angles_y=random(360);
	makevectors(self.angles);
	traceline(self.origin,self.origin+v_forward*dist,TRUE,self);
	org=trace_endpos;
	tospot=org-'0 0 1000';
	traceline(org,tospot,TRUE,self);
	tospot=trace_endpos;
	tospot_x+=random(-100,100);
	tospot_y+=random(-100,100);
	dist=vlen(tospot-org);

	newmis=spawn();
	setorigin(newmis,org);
	if(random()<0.5)
		sound(newmis,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
	else
		sound(newmis,CHAN_AUTO,"crusader/lghtn2.wav",1,ATTN_NORM);
	newmis.think=SUB_Remove;
	thinktime newmis : 3;

	num_branches = rint(random(3,7));
	while(num_branches)
	{
		self.level+=1;
		if(self.level>=8)
			self.level=0;

		if (self.lockentity.classname == "monster_buddha")
			do_lightning (self.lockentity,self.level,STREAM_ATTACHED,4,org,tospot,50,TE_STREAM_LIGHTNING);
		else
			do_lightning (self,self.level,STREAM_ATTACHED,4,org,tospot,10000,TE_STREAM_LIGHTNING);

		lightn_dir=normalize(tospot-org);
		org=org + lightn_dir*random(num_branches+dist/10,num_branches+dist/5);//Include trace_fraction?
		tospot=org-'0 0 1000';
		traceline(trace_endpos,tospot,TRUE,self);
		tospot=trace_endpos;
		if(random()<0.5)
			tospot_x+=random(125,375);
		else
			tospot_x-=random(125,375);
		if(random()<0.5)
			tospot_y+=random(125,375);
		else
			tospot_y-=random(125,375);
		
		/*if(trace_fraction<0.01)
		{
			dprint("not into ground\n");
			num_branches=0;
		}
		else*/
			num_branches-=1;
	}		
}

void rolling_thunder (void)
{
	if(random(100)>=self.wait)
	{
		if(self.spawnflags&1&&random(100)<self.dmg)
		{
			self.lightvalue2=25;
			lightning_strike();
		}
		else
		{
		float lightburst;
			lightburst=random(0.1,1.1)*(25 - self.lightvalue1);
			self.lightvalue2=self.lightvalue1+lightburst;
			if(self.aflag<=3)
				spawn_thunder();
		}
		lightstylestatic(self.style,self.lightvalue2);
		self.think=flash_wait;
		thinktime self : random(0.3);
	}
	else
	{
		self.think=rolling_thunder;
		thinktime self : random(0.1,1.1);
	}
}

void thunder_use (void)
{
	if(self.spawnflags&1)
	{
		self.lightvalue2=25;
		lightning_strike();
	}
	else
	{
	float lightburst;
		lightburst=random(0.1,1.1)*(25 - self.lightvalue1);
		self.lightvalue2=self.lightvalue1+lightburst;
		spawn_thunder();
	}
	lightstylestatic(self.style,self.lightvalue2);
	self.think=flash_wait;
	thinktime self : random(0.3);
}

/*QUAKED light_thunderstorm (0 1 0) (-8 -8 -8) (8 8 8) LIGHTNING
Default light value is 300
This will create a light source that will occaisionally flash with light followed shortly by an ambient thunder sound

.wait = the shorter the wait, the more active the thunderstorm.  Valid values for this are 1 - 100.  100 is a stupid value because it will never thunder!  Default is 33
.dmg = how often lighting strikes, 0 will be the equvalent of never, 100 will be very frequent.  default is 10
.lightvalue1 = the light value the storm should always return to.  Valid range is 0 - 25, 25 being the brightest.  Default is 11 (about 300 brightness)
.frags = The radius in which lightning can stike from this entity. (default=1000)

The LIGHTNING spawnflag will make it cast random lightning strikes

NOTE: These MUST be targeted.  They will never actually be used by the trigger, but targeting is required to give them a distince lightstyle
Targeting can be used to link several thunderstorms together so they all use the same lightstyle (they'll all flash at the same time)
*/
void light_thunderstorm()
{
		if(self.targetname)
			self.use=thunder_use;
		else
		{
			remove(self);
			return;
		}
		precache_sound3("ambience/thunder1.wav");
		precache_sound ("crusader/lghtn1.wav");
		precache_sound ("crusader/lghtn2.wav");
		
		if(!self.frags)
			self.frags=1000;
		if(!self.wait)
			self.wait=33;
		if(!self.lightvalue1)
			self.lightvalue1=11;
		if(!self.dmg&&self.spawnflags&1)
			self.dmg=10;
//			self.dmg=100;
		lightstylestatic(self.style,self.lightvalue1);
		self.think=rolling_thunder;
		thinktime self : 0;
}
