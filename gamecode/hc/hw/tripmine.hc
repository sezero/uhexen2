/*
================================
TRIPMINE.HC
MG

Assassin's Glyph
================================
*/

void() FireExplosion=
{
//	entity boommissile;
	float damaged;

	if (!self)//er, just in case self is screwed, git outta here.
		return;

	self.takedamage=DAMAGE_NO;
	self.th_die = SUB_Null;

	if(self.goalentity.classname=="chain_head")
	{
		if (self.goalentity.wrq_effect_id != -1)
		{
			endeffect(MSG_ALL,self.goalentity.wrq_effect_id);
			self.goalentity.wrq_effect_id=-1;
		}

		WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
		WriteByte (MSG_MULTICAST, TE_LINE_EXPLOSION);
		WriteCoord (MSG_MULTICAST, self.origin_x);
		WriteCoord (MSG_MULTICAST, self.origin_y);
		WriteCoord (MSG_MULTICAST, self.origin_z);
		WriteCoord (MSG_MULTICAST, self.goalentity.origin_x);
		WriteCoord (MSG_MULTICAST, self.goalentity.origin_y);
		WriteCoord (MSG_MULTICAST, self.goalentity.origin_z);
		multicast((self.origin+self.goalentity.origin)*0.5,MULTICAST_PHS);


		traceline (self.origin+normalize(self.v_angle*8), self.goalentity.origin, FALSE, self.goalentity);
		damaged = 0;
		while (trace_fraction <1.0)
		{
			if (trace_ent)//but how can world be in the way of the tripchain?
			{
				if (trace_ent.teleport_time < time)
				{
					T_Damage(trace_ent,self,self.controller,random(120,150));
				}
			}
			damaged += 1;
			traceline (trace_endpos, self.goalentity.origin, FALSE, trace_ent);
			if ((trace_ent == world)||(trace_ent == self.goalentity)||(damaged > 4))
			{
				trace_fraction = 1.0;
			}
		}

//	if(pointcontents(self.origin)==CONTENT_SOLID||self.bloodloss<=0)
//	{
//		if(self.goalentity.classname=="chain_head")
//		{
			remove(self.goalentity);
//		}
//		remove(self);
//	}

	}

//	boommissile=spawn();
//	boommissile.owner=self.controller;
//	boommissile.movetype=MOVETYPE_NOCLIP;
//	boommissile.solid=SOLID_NOT;
//	boommissile.velocity=self.v_angle*3;
//	boommissile.think=ShaBoom;
//	boommissile.nextthink=time;
//	boommissile.goalentity=self.goalentity;
//	boommissile.bloodloss=100;
//	boommissile.classname="tripwire";
//	setorigin(boommissile,self.origin);
//	setmodel (boommissile, "models/bg_expld.spr");

	MultiExplode();
};

void () LaserTripped =
{
	self.takedamage=DAMAGE_NO;
	sound(self,CHAN_AUTO,"misc/warning.wav",1,ATTN_NORM);
    self.effects=EF_BRIGHTLIGHT;
	FireExplosion();
//    self.think=FireExplosion;
  //  self.nextthink=time + 0.1;
};

void() LaserThink =
{
        if(!self.aflag)
        {
	        sound (self, CHAN_BODY, "doors/baddoor.wav", 1, ATTN_NORM);
			self.controller=self.owner;
            self.owner=self;
            self.skin=1;
            self.aflag=TRUE;
        }
        traceline (self.origin+normalize(self.v_angle*8), self.goalentity.origin, FALSE, self.goalentity);
		if((vlen(self.origin-self.goalentity.origin)>640)||
			(trace_fraction <1.0&&
			(trace_ent.velocity!='0 0 0'||trace_ent.health>0||trace_ent.takedamage)))
		{
			LaserTripped();
				return;
		}
        else 
			self.think=LaserThink;
        self.nextthink=time;
		if(self.lifetime<time||self.tripwire_cnt<self.controller.tripwire_cnt - 3)
			self.think=self.th_die;
};

void ChainDie (void)
{
	self.takedamage=DAMAGE_NO;
	self.th_die = SUB_Null;
	if (self.wrq_effect_id != -1)
	{
		endeffect(MSG_ALL,self.wrq_effect_id);
		self.wrq_effect_id=-1;
	}
	if(self.owner)
	{
		self.owner.think=MultiExplode;
		thinktime self.owner : 0;
	}
	MultiExplode();
}

void ChainTouch(void)
{
//	if(other==self.owner.owner)
//		return;
	if(other==self.owner)
		return;

	self.touch=SUB_Null;
	if(other.takedamage)
	{
		T_Damage(other,self,self.owner.controller,random(50,70));
	    self.owner.think=LaserTripped;
		self.owner.nextthink=time;
		if (self.wrq_effect_id != -1)
		{
			endeffect(MSG_ALL,self.wrq_effect_id);
			self.wrq_effect_id=-1;
		}
	}
	else
	{
		sound(self,CHAN_BODY,"weapons/met2stn.wav",1,ATTN_NORM);
		self.movetype=MOVETYPE_NONE;
		self.velocity='0 0 0';
        self.health=25;
        self.takedamage=DAMAGE_YES;
		self.th_die=ChainDie;
		setsize(self,'-3 -3 -6','3 3 6');

		if (self.wrq_effect_id != -1)
		{
			endeffect(MSG_ALL,self.wrq_effect_id);
		}
		self.wrq_effect_id = starteffect(CE_TRIPMINESTILL, self.owner.origin, self.origin);
	}
}

void ChainThink (void)
{
//	DrawLinks();
/*	traceline(self.origin,self.view_ofs,FALSE,self);
	if((vlen(self.origin-self.owner.origin)>640&&self.movetype==MOVETYPE_FLYMISSILE)||
//		self.movetype=MOVETYPE_BOUNCE;
	(trace_fraction<1))
	{
	    self.owner.think=LaserTripped;
		self.owner.nextthink=time;
		if (self.wrq_effect_id != -1)
		{
			endeffect(MSG_ALL,self.wrq_effect_id);
			self.wrq_effect_id=-1;
		}
//		remove(self);//dis fux up tempent--i'll get removed anyway
		self.movetype=MOVETYPE_NONE;
		self.velocity='0 0 0';
        self.takedamage=DAMAGE_NO;
		self.touch=SUB_Null;
		self.nextthink = time+10;
		return;
	}
	self.nextthink=time+0.05;*/
	self.nextthink=time+5;
}

void FireChain (void)
{
	vector dir;
	sound(self,CHAN_BODY,"assassin/chain.wav",1,ATTN_NORM);
	dir=normalize(self.v_angle);
	newmis=spawn();
	newmis.classname="chain_head";
	newmis.owner=self;
	self.goalentity=newmis;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.velocity=dir*750;
	newmis.touch=ChainTouch;
	newmis.think=ChainThink;
	newmis.drawflags=MLS_POWERMODE;
	newmis.nextthink=time;
	newmis.view_ofs=self.origin;
	newmis.angles=vectoangles(newmis.velocity);
//	setmodel(newmis,"models/twspike.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+dir*8);

	newmis.wrq_effect_id = starteffect(CE_TRIPMINE, self.origin, newmis.velocity);
}

void TripArmed ()
{
	FireChain();
    self.think=LaserThink;
	thinktime self : 0;
	self.lifetime=time+60;
}

void() TripTouch =
{
	if(other.takedamage&&other.health&&(other.classname=="player"||other.movetype)&&other!=self.owner)
		MultiExplode();
	else
	{
		sound(self,CHAN_WEAPON,"buttons/switch04.wav",1,ATTN_NORM);
		self.v_angle='0 0 0' - self.velocity;
		self.angles = vectoangles (self.v_angle);
		self.movetype=MOVETYPE_NONE;
		self.velocity='0 0 0';
		self.avelocity= '0 0 0';
		setsize(self,'-3 -3 -6','3 3 6');
		self.hull=HULL_POINT;
		self.touch=SUB_Null;
		self.think=TripArmed;
		thinktime self : 0.5;
	}
};

void TripDie (void)
{
	self.takedamage=DAMAGE_NO;
	self.th_die = SUB_Null;
	if(self.goalentity.classname=="chain_head")
	{
		if (self.goalentity.wrq_effect_id != -1)
		{
			endeffect(MSG_ALL,self.goalentity.wrq_effect_id);
			self.goalentity.wrq_effect_id=-1;
		}
		remove(self.goalentity);
	}
	MultiExplode();
}

void() Use_Tripwire =
{
	entity missile;

	missile=spawn();
	missile.owner=self;

	missile.movetype=MOVETYPE_FLYMISSILE;
	missile.solid=SOLID_BBOX;
	missile.health=25;
	missile.dmg=50;
	missile.takedamage=DAMAGE_YES;

	missile.th_die=TripDie;
	missile.classname="tripwire";
	makevectors(self.v_angle);
	missile.touch=TripTouch;
	missile.think=SUB_Null;
	missile.nextthink= -1;
	self.tripwire_cnt+=1;
	missile.tripwire_cnt=self.tripwire_cnt;

	missile.velocity=normalize(v_forward)*500;
	missile.angles = vectoangles ('0 0 0' - missile.velocity);
	missile.avelocity_z = 300;

	setmodel (missile, "models/glyphwir.mdl");
	setorigin(missile,self.origin+self.proj_ofs+v_forward*4);
	setsize (missile, '0 0 0','0 0 0');
};
