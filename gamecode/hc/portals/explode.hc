void() CB_BoltStick;
void FireMeteor (string type);
void FireAcidBlob (string type);

void()BlowUp=
{
	if(self.dmg<2.5&&self.scale>=0.1)
	{
		self.v_angle=RandomVector('180 180 180');
		self.scale=self.dmg;
		if(self.enemy)
			if(self.enemy.health>0&&self.enemy.flags2&FL_ALIVE)//Stay with enemy;
				setorigin(self,self.enemy.origin+self.view_ofs);
	    T_RadiusDamage (self, self.owner, self.dmg*100, world);
		self.dmg += 0.1;
		self.think=BlowUp;
		thinktime self : 0.025;
	}
	else
	{
		self.think=SUB_Remove;
		thinktime self : 0;
	}
};

void() SprayFire=
{
local entity fireballblast;
	sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NORM);
	fireballblast=spawn();
	fireballblast.enemy=self.enemy;
	fireballblast.movetype=MOVETYPE_NOCLIP;
	fireballblast.owner=self.owner;
	fireballblast.classname="fireballblast";
	fireballblast.solid=SOLID_NOT;
	fireballblast.drawflags(+)MLS_ABSLIGHT|SCALE_TYPE_UNIFORM|SCALE_ORIGIN_CENTER;
	fireballblast.abslight= 0.5;
	fireballblast.scale=0.1;
	setmodel(fireballblast,"models/blast.mdl");
	setsize(fireballblast,'0 0 0','0 0 0');
	setorigin(fireballblast,self.origin);
//	fireballblast.effects=EF_BRIGHTLIGHT;
	fireballblast.dmg=0.1;
	fireballblast.avelocity='50 50 50';
	fireballblast.think=BlowUp;
	thinktime fireballblast : 0;
	remove(self);
};

void BecomeExplosion (float explodetype)
{
	if (explodetype)
	{
		if(explodetype==CE_FLOOR_EXPLOSION)
			starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 64');
		else
			starteffect(explodetype , self.origin);
	}
	else
	{
		if(self.classname=="acidblob")
			starteffect(CE_ACID_EXPL,self.origin - self.movedir*8,'0 0 0',HX_FRAME_TIME*2);
		else if (self.flags2&FL_SMALL)
			starteffect(CE_SM_EXPLOSION , self.origin);
		else if(self.flags&FL_ONGROUND)
			starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 64');
		else
			starteffect(CE_LG_EXPLOSION , self.origin);
	}

	if(self.classname=="multigrenade")
	{//Let sounds play here
		self.effects=EF_NODRAW;
		self.velocity='0 0 0';
		self.movetype=MOVETYPE_NONE;
		self.think=SUB_Remove;
		thinktime self : 3;
	}
	else
		remove(self);
}

void SmallExplosion (void)
{
	sound(self,CHAN_AUTO,"weapons/explode.wav",0.5,ATTN_NORM);
	BecomeExplosion(CE_SM_EXPLOSION);
}

void DarkExplosion ()
{
entity ignore;
	if(self.classname=="timebomb")
	{
		sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
		ignore=self.owner;
	}
	else if(self.classname=="pincer")
	{
		sound(self,CHAN_BODY,"weapons/explode.wav",1,ATTN_NORM);
		ignore=self.owner;
	}
	else
	{
		if(self.controller.classname=="multigrenade")
			sound(self.controller,CHAN_BODY,"weapons/explode.wav",1,ATTN_NORM);
		else
			sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
		ignore=world;
	}

	T_RadiusDamage (self, self.owner, self.dmg, ignore);

	if(self.classname=="minigrenade"&&random()<0.5)
		BecomeExplosion(FALSE);
	else if(self.classname=="flaming arrow")
	{
		starteffect(CE_XBOW_EXPLOSION,self.origin);
		remove(self);
	}
	else
	{
		starteffect(CE_NEW_EXPLOSION,self.origin);
		remove(self);
	}
}

void() MultiExplode =
{
float nummeteorites;
//FIXME: For some reason, the light casting effects in Hex2
//are a lot more costly than they were in Quake...
	if(self.classname=="stickmine")
	{
		SprayFire();
		return;
	}

	T_RadiusDamage (self, self.owner, self.dmg, world);

	if(self.classname=="meteor"||self.classname=="acidblob"||self.classname=="monster_pentacles")
	{
		nummeteorites=random(3,10);
		if(deathmatch||coop)
		{
			if(self.classname=="acidblob")
				starteffect(CE_CHUNK, self.origin, THINGTYPE_ACID,'0 0 600', nummeteorites);
			else
				starteffect(CE_CHUNK, self.origin, THINGTYPE_METEOR,'0 0 600', nummeteorites);
		}
		else while(nummeteorites>0)
		{
			if(self.classname=="acidblob")//FIXME: Need wet acid explode sound
			{
				if(nummeteorites==1)
					sound(self,CHAN_BODY,"succubus/blobexpl.wav",1,ATTN_NORM);
				FireAcidBlob("aciddrop");
			}
			else
				FireMeteor("minimeteor");
			nummeteorites =nummeteorites - 1;
		}
	}

	if(self.flags2&FL_SMALL)
        SmallExplosion();
    else
    {
//FIXME:  This causes a crash if removed, and sometimes the explosions are spawned at the world origin!
//		if(self.classname!="minigrenade"||(self.classname=="flaming arrow"&&random()<0.5))
//		{
			WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
			WriteByte (MSG_BROADCAST, TE_EXPLOSION);
			WriteCoord (MSG_BROADCAST, self.origin_x);
			WriteCoord (MSG_BROADCAST, self.origin_y);
			WriteCoord (MSG_BROADCAST, self.origin_z);
//		}
		BecomeExplosion(FALSE);
    }
};

void() SuperGrenadeExplode;
void() GrenadeTouch2 =
{
	if (other == self.owner)
		return;         // don't explode on owner

    if(other.owner==self.owner&&other.classname==self.classname&&self.classname=="minigrenade")
        return;

	if (other.takedamage==DAMAGE_YES)//let them damage b_models on impact?
	{
		T_Damage(other,self,self.owner,self.dmg);
		self.dmg/=2;
        if(self.classname=="multigrenade")
	        self.think=SuperGrenadeExplode;
        else if(self.classname=="minigrenade"||self.classname=="flaming arrow")
			self.think=DarkExplosion;
		else if(self.classname=="poison grenade")
		{
			self.think=SpewPoison;
		}
		else
			self.think=MultiExplode;
		thinktime self : 0;
	}
	else
	{
		sound (self, CHAN_WEAPON, "assassin/gbounce.wav", 1, ATTN_NORM);  // bounce sound
		if (self.velocity == '0 0 0')
			self.avelocity = '0 0 0';
	}
};

void() StickMineTouch =
{
        if(other==self.owner)
                return;

		self.skin=1;
vector stickdir;
		self.touch=SUB_Null;
		if(other.takedamage)
		{
	        sound(self,CHAN_WEAPON,"weapons/met2flsh.wav",1,ATTN_NORM);
			T_Damage(other,self,self.owner,3);
			if(other.solid!=SOLID_BSP)
			{
//				makevectors(self.angles);
				stickdir=other.origin+normalize(self.origin-other.origin)*12;
//Modify height for shorter or taller models like imp, golem, spider, etc.
				if(other.classname=="player")
//Put it right below view of player
					stickdir_z=other.origin_z+other.proj_ofs_z + 1;
				else if(other.classname=="monster_spider")
					stickdir_z=(self.origin_z+(other.origin_z+other.size_z*0.2)*3)*0.25;
				else stickdir_z=(self.origin_z+(other.origin_z+other.size_z*0.6)*3)*0.25;
				setorigin(self,stickdir);
				SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,other);
			}
		}
		else
		{
			setorigin(self,self.origin+normalize(self.velocity)*-3);
	        sound(self,CHAN_WEAPON,"weapons/met2stn.wav",1,ATTN_NORM);
			SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,world);
		}

        self.velocity='0 0 0';
        self.movetype=MOVETYPE_NOCLIP;
        self.solid=SOLID_NOT;
        self.touch=SUB_Null;
        self.wait=time + 1;
		self.health=other.health;
        if(other.takedamage)
        {
            self.enemy=other;
            self.view_ofs=(self.origin-other.origin);
            self.o_angle=(self.angles-self.enemy.angles);
			self.think=CB_BoltStick;
	        thinktime self : 0;
        }
        else
        {
			self.enemy=world;
			self.movetype=MOVETYPE_NONE;
			self.think=MultiExplode;
	        thinktime self : 0.5;
        }
};

void() Use_Fireball =
{
	self.attack_finished=time + 1;//So you can't have a ton of them
	makevectors(self.v_angle);
//sound
entity missile;
    missile=spawn();
    missile.owner=self;
    missile.classname="stickmine";
    missile.movetype=MOVETYPE_BOUNCE;
    missile.solid=SOLID_BBOX;
	missile.touch=StickMineTouch;
	missile.dmg=50;

	missile.velocity=normalize(v_forward)*700 +v_up*200;
	missile.avelocity=RandomVector('300 300 300');
	missile.lifetime=time+60;

	setmodel (missile, "models/glyphwir.mdl");
	setsize(missile,'0 0 0','0 0 0');
	setorigin(missile,self.origin+self.proj_ofs+v_forward*16);
	missile.think=MultiExplode;
	thinktime missile : 10;
};
