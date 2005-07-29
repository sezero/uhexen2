void(vector org) SpawnFlameAt;

void () FireDie =
{
    T_RadiusDamage (self, self.owner, (self.dmg - 1)*125+25, world);

	self.origin = self.origin - 8*normalize(self.velocity);

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_EXPLOSION);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);

	BecomeExplosion (FALSE);
        remove(self);
};

void() FireDamage =
{
local float damagemult;
		if(self.trigger_field.skin>110)
			self.wait=0;
		else self.wait = self.wait - 1;

        if (self.trigger_field.classname == "player" && self.owner.classname != "player")
                self.owner = self.trigger_field;

        if (self.trigger_field.waterlevel > 2)
            if(self.trigger_field.watertype == CONTENT_LAVA)
            {    
				if(self.trigger_field.classname=="player")
				{
					sprint(self.owner,self.trigger_field.netname);
			        sprint(self.owner," jumped out of the frying pan into the fire!\n");
				}
			    self.trigger_field.effects = 0;
				self.trigger_field.onfire = 0;
				remove(self);
//	            FireDie();
            }
            else
            {    
				if(self.trigger_field.classname=="player")
				{
		            sound (self.owner, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
					sprint(self.owner,self.trigger_field.netname);
				}
			    sprint(self.owner," saved his ass by jumping in the water!\n");
				self.trigger_field.effects = 0;
	            self.trigger_field.onfire = 0;
		        remove(self);
            }
                
       if ((self.waterlevel > 2) || (self.watertype == CONTENT_WATER) || (self.watertype == CONTENT_SLIME) || (pointcontents(self.origin) == CONTENT_WATER) || (pointcontents(self.origin) == CONTENT_SLIME))
       {
            sound (self.owner, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
            sprint(self.owner,"Fireball fizzled underwater\n");
            self.trigger_field.effects = 0;
            self.trigger_field.onfire = 0;
            remove(self);
       }

//        if (self.aflag != 5)
//		{
		//Non-explosive death
                if ((self.wait < 1) || (self.trigger_field.health<=0))
                {
                        self.trigger_field.effects = 0;
                        self.trigger_field.onfire = 0;
                        remove(self);
                }
//        }
//        else if (self.wait < 1 || self.trigger_field.health <= 6)
//        {
		//explosive fireball death
//                self.dmg = 2;
//                self.trigger_field.onfire = 0;
//                self.trigger_field.effects = 0;
//                FireDie();
//        }
		if(self.trigger_field.onfire>0.5)
			self.trigger_field.onfire=0.5;//Max out damage at 0.5 per think
        if (!self.trigger_field.flags2&FL2_FIRERESIST)
        {
				if(self.aflag>0.1)
					damagemult=self.aflag;
				else 
					damagemult=0.1;		
                if(self.trigger_field.flags2&FL2_FIREHEAL)                
                    self.trigger_field.health=self.trigger_field.health+2*damagemult*self.trigger_field.onfire;
                else 
					T_Damage (self.trigger_field, self, self.owner, damagemult*self.trigger_field.onfire);
        }

		if(self.trigger_field.health<=17)
			if(self.trigger_field.thingtype==THINGTYPE_WOOD)
			{
				self.trigger_field.skin=111;
				self.trigger_field.deathtype="burnt crumble";
			}
			else if(self.trigger_field.thingtype==THINGTYPE_FLESH)
			{
				self.trigger_field.skin=112;
				self.trigger_field.deathtype="burnt crumble";
			}

		//FIXME: need 3 different fire models to put on them,
		//randomly choose one & scale it to their size, add more
		// if they're more on fire.  No more than 3.  Only one
		//fire sound, from "invisible" one doing actual damage.
		//Use movechain function to link flames to target
        if(random()<0.5)
	        SpawnFlameAt (self.origin);//This is causing an edict overflow hard crash...

        self.origin = self.trigger_field.origin + '0 0 6';
        self.think = FireDamage;
		thinktime self : 0.1;
};

void() FireTouch2 =
{
	self.trigger_field.effects = EF_BRIGHTLIGHT;
	if(self.trigger_field.onfire<=0)
		self.trigger_field.onfire = 1;
	self.think=FireDamage;
	thinktime self : 0;
};

void() FireTouch =
{
	local float     damg;

	if (other == self.owner)
		return;         // don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}
		
        if (other.takedamage&&other.thingtype!=1&&other.thingtype!=0&&other.thingtype!=3&&other.skin<111)
		{
				if(other.onfire)
				{
					other.onfire=other.onfire+0.1;
					remove(self);
				}
                else
					other.onfire=0.1;
				sound (self, CHAN_WEAPON, "misc/combust.wav", 1, ATTN_NORM);
                self.trigger_field = other;
                self.origin = other.origin + '0 0 6';

                if (other.classname == "player" || self.aflag == 5)
                        self.wait = 180;
                else
                        self.wait = 40;
                
                setsize (self, '0 0 0', '0 0 0');            

                self.think=FireTouch2;
                thinktime self : 0;
				self.movetype = MOVETYPE_NOCLIP;
				self.velocity = '0 0 0' ;
				self.avelocity = '0 0 1000';
                if ((other.waterlevel < 2) && (other.classname == "player"))
	                sprint(other, "You're burning up!\n");
				return;
		}
        else if (self.movetype == MOVETYPE_FLYMISSILE)
			remove(self);
//                FireDie();
        sound (self, CHAN_WEAPON, "misc/fburn_sm.wav", 0.75, ATTN_STATIC);  // bounce sound
		SpawnPuff(self.origin,'0 0 0', 10,other);
//        spawn_touchblood (10);
        SpawnFlameAt (self.origin);
		if (self.velocity == '0 0 0')
			self.avelocity = '0 0 0';
        if (self.aflag == 5)
        {
                self.trigger_field=self.enemy;
				thinktime self : 0.05;
                self.think = FireTouch2;
        }
};

void () FireThink =
{

	if (pointcontents(self.origin)==CONTENT_WATER||pointcontents(self.origin)==CONTENT_SLIME||(self.waterlevel > 2)) 
		FireFizzle();
	self.attack_finished = self.attack_finished + 0.1;
	if (self.attack_finished > 5)
//		if(self.model=="models/lavaball.spr")
//			FireDie();
//		else
			remove(self);
	thinktime self : 0.1;
	self.think = FireThink;
};

void(entity loser) SpawnFlameOn =
{
if(pointcontents(loser.origin)<-2)
        return;
//Used by player, archvile, and stickmine
local entity fire;
        if (pointcontents(self.origin) < -2)
        {
            sound (self, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
            return;
        }
        fire = spawn ();
        fire.controller = self;
		fire.movetype = MOVETYPE_NOCLIP;
        fire.solid = SOLID_NOT;
		fire.velocity = '0 0 0' ;
		fire.avelocity = '0 0 1000';
        fire.classname = "fire";
        fire.dmg = 1;
        fire.aflag = 0.25;
        setmodel (fire, "models/null.spr");
		fire.effects=EF_NODRAW;
        setsize (fire, '0 0 0', '0 0 0');        
		if(self.classname=="fireballblast"||self.classname=="flaming arrow")
		{
                fire.enemy = loser;
                setorigin (fire, (loser.absmax+loser.absmin)*0.5);
				fire.owner=self;
		}
        else
        {
                setorigin (fire, (self.enemy.absmax+self.enemy.absmin)*0.5);
                fire.enemy = self.enemy;
                fire.owner = self.controller;
        }
		sound (fire, CHAN_WEAPON, "misc/combust.wav", 1, ATTN_NORM);
		if(!loser.onfire)
			loser.onfire=0.1;
		else
		{
			loser.onfire+=0.1;
			remove(fire);
			return;
		}
        fire.trigger_field = loser;

        if (loser.classname == "player")
	        fire.wait = 120;
        else
            fire.wait = 40;
        thinktime fire : 0.1;
        fire.think=FireTouch2;
};

void(vector org) SpawnFlameAt =
{
if(pointcontents(self.origin)<-2)
        return;
local entity    fireflame;
local float     xorg, yorg, zorg;
        fireflame = spawn();
        setmodel (fireflame, "models/flame2.mdl");
        fireflame.movetype = MOVETYPE_FLY;
        fireflame.solid = SOLID_NOT;
        fireflame.classname = "missile";
		fireflame.drawflags(+)MLS_ABSLIGHT;
		fireflame.abslight=0.5;
        fireflame.frame = rint(random());
        setsize (fireflame, '-2 -2 -2', '1 1 1');
        xorg = random(-15,15);
        yorg = random(-15,15);
        zorg = random(-25,25);
        setorigin (fireflame, org + v_forward * xorg + v_right * yorg + v_up * zorg);
        fireflame.velocity_x += random(-40,40);
        fireflame.velocity_y += random(-40,40);
        fireflame.velocity_z += random(300);
        fireflame.avelocity = '0 0 0';
        if (random() < 0.3)
			sound (self, CHAN_WEAPON, "misc/fburn_sm.wav", 1, ATTN_NORM);
        thinktime fireflame :  0.5;    // remove after half second
        fireflame.think = SUB_Remove;
};

/*
================
FlameTouch
================
*/
void () FlameTouch =
{
        local   float   rn;
//Scale size relative to other.size?
    if (other.takedamage)
	{
		if(other.flags2&FL2_FIREHEAL)
			other.health=other.health+1;
		else if (!other.flags2&FL2_FIRERESIST)
			T_Damage(other,self,self.owner,5);
		if(other.thingtype==THINGTYPE_FLESH||other.thingtype==THINGTYPE_WOOD)
        {
			//set on fire
                rn = random();
                // 50% chance
                if (rn <= 0.5||other.onfire>=1)
                        {
                        if(other.onfire)
                                other.onfire = other.onfire + 0.1;
                        remove(self);
                        }
                other.onfire = other.onfire + 0.1;
                if(self.owner.movetype==MOVETYPE_NONE)
                        self.wait = 20;
                else self.wait = 60;
                self.aflag = 0.1;
                self.trigger_field = other;
                self.think = FireTouch2;
                thinktime self : 0;
                self.solid = SOLID_NOT;
                setmodel (self,"models/flame2.mdl");
        }
        else if(self.netname=="firespike")
        {
				//FIXME:else sprite
                remove(self);
        }
	}
	else
        {
                if(self.netname=="firespike")
                {
                        if(other.thingtype==THINGTYPE_FLESH||other.thingtype==THINGTYPE_WOOD)
							SpawnFlameAt(self.origin);
						//FIXME:else sprite
                        remove(self);
                }
                else
                {
                        self.velocity = '0 0 0';
                        self.velocity_z = random(24,48);
                }
        }
};




/*
================
W_FireFlame
================
*/
void(float offset) W_FireFlame =
{
        local   entity flame;
        local   float rn;

    if(self.movetype!=MOVETYPE_NONE)
    {
        if (self.waterlevel > 2)
        {
//                makevectors (self.v_angle);

                rn = random();
                if (rn < 0.5)
                        sound (self, CHAN_WEAPON, "player/swim1.wav", 1, ATTN_NORM);
                else
                        sound (self, CHAN_WEAPON, "player/swim2.wav", 1, ATTN_NORM);
				flame=spawn();
				flame.owner=flame;
				setorigin(flame,self.origin+self.proj_ofs + v_forward * 32 + v_up * random(8,16));
				flame.think=DeathBubblesSpawn;
				thinktime flame : 0;
                return;
        }

        // Take away a shell
//        self.currentammo = self.ammo_shells = self.ammo_shells - 1;
    }
    else if((random()>=0.1&&random()>=0.2)||(!other.takedamage))
        return;
        sound (self, CHAN_WEAPON, "paladin/purfire.wav", 1, ATTN_NORM);

        flame = spawn ();
        flame.owner = self;
        flame.movetype = MOVETYPE_FLYMISSILE;
        flame.solid = SOLID_BBOX;
        flame.classname = "fire";
		flame.drawflags(+)MLS_ABSLIGHT;
		flame.abslight=0.5;
// set flame speed    

		if(self.classname=="player"&&self.playerclass==CLASS_PALADIN&&self.weapon==IT_WEAPON4)
	    {
			self.punchangle_x= -1;
		    setmodel (flame, "models/purfir1.mdl");
			flame.netname="firespike";
		}
        else
			setmodel (flame, "models/flame2.mdl");
        setsize (flame, '0 0 0', '0 0 0');            

        if(self.movetype==MOVETYPE_NONE)
        {
                flame.velocity='0 0 100';
                setorigin (flame, self.origin + '0 0 16');
        }
        else
        {
                makevectors (self.v_angle);
                flame.velocity = normalize(v_forward)*700 + v_up*random(32) + v_right * random(0-offset,offset);
                setorigin (flame, self.origin+self.proj_ofs + v_forward * 32 + v_up * random(-8,8));
        }
		if(flame.netname=="firespike")
			flame.angles=vectoangles(flame.velocity);

//        flame.effects = 6;
        flame.touch = FlameTouch;
	
        flame.think = FireThink;
		thinktime flame : 0;
		self.attack_finished=time + 0.1;
};
