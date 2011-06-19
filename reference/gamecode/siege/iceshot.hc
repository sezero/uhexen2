void() rain_use;

void() IceCubeThink =
{
	if((self.maxs_z-self.mins_z)>5)
        if(pointcontents(self.origin-'0 0 23')==CONTENT_LAVA||self.frozen<=0)
        {
				if((self.maxs_z-self.mins_z)<25)
					self.small=TRUE;
                self.frozen=FALSE;
                self.think=self.oldthink;
                self.nextthink=time;
                self.skin=self.oldskin;
				self.touch=self.oldtouch;
				self.movetype=self.oldmovetype;
				self.drawflags-=DRF_TRANSLUCENT;
				return;
        }

        if(pointcontents(self.origin)==CONTENT_WATER||pointcontents(self.origin)==CONTENT_SLIME)
        {
		        self.frozen=self.frozen - 1;
				self.scale -= 0.01;
		}

        if((!self.flags&FL_ONGROUND)&&pointcontents(self.origin+'0 0 -24')!=CONTENT_SOLID)
            self.frags=TRUE;
        if (self.frags&&(self.flags&FL_ONGROUND))
            T_Damage(self,world,self.enemy,self.frags*10);
		self.frozen=self.frozen - 0.2;
		if(self.wait<=time)
		{
			self.scale -= 0.007;
			self.mins = self.o_angle * (self.scale/self.lifetime);
			self.maxs = self.v_angle * (self.scale/self.lifetime);
			setsize(self,self.mins,self.maxs);
			droptofloor();
		}
		if((self.maxs_z-self.mins_z)<=5||self.scale<=0.07)
		{
//			AwardExperience(self.enemy,self,0);
			remove(self);
		}
        self.think=IceCubeThink;
        self.nextthink=time+0.1;
};

void (entity loser,entity forwhom) SnowJob=
{
	sound(loser,CHAN_AUTO,"weapons/frozen.wav",1,ATTN_NORM);
	loser.frozen=50;
    loser.oldskin=loser.skin;
    loser.skin=105;
	if(loser.classname!="player")
	{
		if(loser.scale==0)
			loser.scale = 1;
		loser.lifetime=loser.scale;
		loser.o_angle=loser.mins;
		loser.v_angle=loser.maxs;
		loser.enemy=forwhom;
        loser.oldthink=loser.think;
        loser.think=IceCubeThink;
        loser.nextthink=time;
//      loser.nextthink=time+30;
        loser.touch=SUB_Null;
//Prevent interruption? loser.th_pain=SUB_Null;                        
		loser.wait = time + 10;
		if(loser.angles_x==0&&loser.angles_z==0)
			loser.drawflags+=SCALE_ORIGIN_BOTTOM;
		loser.oldmovetype=loser.movetype;
		loser.movetype=MOVETYPE_PUSHPULL;
        loser.health=1;
	}
	else
	{
        loser.o_angle=loser.v_angle;
        loser.pausetime = time + 20;
        loser.attack_finished = time + 20;
//Temp -turns screen blue
        loser.items+= IT_QUAD;
        loser.health=1;
		loser.nextthink=time + 20;
//Prevent interruption? loser.th_pain=SUB_Null;
	}
	if(loser.flags&FL_FLY)
        loser.flags = loser.flags - FL_FLY;
	if (loser.flags & FL_SWIM)
        loser.flags = loser.flags - FL_SWIM;
	if(loser.flags&FL_ONGROUND)
        loser.flags = loser.flags - FL_ONGROUND;
//need to be able to reverse this...
		loser.oldtouch=loser.touch;
		loser.touch=obj_push;
		loser.drawflags+=DRF_TRANSLUCENT;
};

void() FreezeTouch=
{
     if(other.takedamage&&other.health&&(!other.frozen)&&(!other.flags&FL_COLDRESIST)&&(!other.flags&FL_COLDHEAL))
     {
        if((!other.frozen)&&other.health>12)
                T_Damage(other,self,self.owner,10);
        if(random()<0.2)
			SnowJob(other,self.owner);
	}
	else if(other.flags&FL_COLDHEAL)
		other.health=other.health+5;
	else
		T_RadiusDamage(self,self.owner,30,self.owner);
	self.touch=SUB_Null;
	self.deathtype="ice shatter";
	shatter();
};

void()FreezeThink=
{
        if((pointcontents(self.origin)==CONTENT_WATER&&random()<0.3)||pointcontents(self.origin)==CONTENT_LAVA||(pointcontents(self.origin)==CONTENT_SLIME&&random()<0.5)||self.wait<time)
            {
            if(pointcontents(self.origin)==CONTENT_LAVA)
                    sound (self, CHAN_WEAPON, "player/slimbrn2.wav", 1, ATTN_NORM);
            remove(self);
            }
        self.think=FreezeThink;
        self.nextthink=time + 0.5;
};

void()FireFreeze=
{
	local vector    dir;
		makevectors (self.v_angle);
        dir = normalize(v_forward);
        sound (self, CHAN_WEAPON, "hknight/hit.wav", 1, ATTN_NORM);
		newmis = spawn ();
        newmis.owner = self;
		newmis.movetype = MOVETYPE_FLYMISSILE;
		newmis.solid = SOLID_BBOX;
		newmis.angles = vectoangles(dir);
	
		newmis.touch = FreezeTouch;
        newmis.classname = "snowball";
		newmis.think = SUB_Remove;
		newmis.nextthink = time + 6;
        newmis.speed = 1200;
        setmodel (newmis, "models/iceshot.mdl");
        newmis.avelocity='-200 200 -200';
        newmis.think=FreezeThink;
        newmis.nextthink=time + 0.5;
        newmis.wait=time + 3;

        setsize (newmis, VEC_ORIGIN, VEC_ORIGIN);
        setorigin (newmis, self.origin + v_forward*8+'0 0 16');
        newmis.velocity = dir * newmis.speed;
};

void() blizzard_think=
{
local entity loser;
	if(self.lifetime<time)
		remove(self);
	rain_go(self.mins,self.maxs,self.size,self.color,self.counter);
	if(self.pain_finished<time)
	{
		sound(self,CHAN_WEAPON,"test/blizzard.wav",1,ATTN_NORM);
		self.pain_finished=time + 0.7;
	}
	loser=findradius(self.origin,128);
	while(loser)
	{
		if(loser.takedamage&&loser.health&&!loser.frozen&&(!loser.flags&FL_COLDRESIST))
			if(loser.flags&FL_COLDHEAL)
				loser.health+=3;
			else
			{
				T_Damage(loser,self,self.owner,5);
				if(random()<0.2)
					SnowJob(loser,self.owner);
			}
		loser=loser.chain;
	}
	self.nextthink = time + 0.1;
};

void() make_blizzard =
{
local entity blizzard;
	blizzard=spawn();
  	blizzard.movetype = MOVETYPE_NOCLIP;
  	blizzard.solid = SOLID_NOT;
	blizzard.classname="blizzard";
  	blizzard.owner = self.owner;
	blizzard.modelindex = 0;
	
	blizzard.color=150;
	blizzard.lifetime=time + 30;
	blizzard.think=blizzard_think;
	blizzard.nextthink = time;

	blizzard.counter=500;

  	setmodel (blizzard, "progs/null.spr");
  	setsize (blizzard, '-64 -64 -36', '64 64 128');
  	setorigin (blizzard, self.origin);

	shatter();
};

void()sparkle=
{
	particle(self.origin,'0 0 0',crandom()*255,random()*7+3);
	self.think = sparkle;
	self.nextthink = time+0.01;
};

void FireBlizzard (void)
{
	self.attack_finished=time + 1;
	newmis=spawn();
	newmis.owner=self;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.touch=make_blizzard;

	newmis.velocity=normalize(v_forward)*1000;
	newmis.effects=EF_MUZZLEFLASH;
	
	newmis.think=sparkle;
	newmis.nextthink=time;

	setmodel(newmis,"progs/null.spr");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+v_forward*16+'0 0 16');
}

