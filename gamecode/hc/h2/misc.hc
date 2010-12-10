/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/misc.hc,v 1.3 2007-02-07 16:57:07 sezero Exp $
 */

/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void info_null()
{
	remove(self);
}

float ROTATE_BREAK = 16;
/*
//============================================================================

float	POWERPOOL_HEALTH		= 1;
float	POWERPOOL_EXPERIENCE	= 2;
float	POWERPOOL_GREENMANA		= 4;
float	POWERPOOL_BLUEMANA		= 8;


void	power_pool_touch()
{
	if(time < self.ltime)
		return;
	self.cnt = self.cnt + 1;
	if(self.cnt > self.count)
		{
		self.touch = SUB_Null;
		return;
		}

	if(self.spawnflags & POWERPOOL_HEALTH)
		other.health		= other.health		+ 1;
	if(self.spawnflags & POWERPOOL_EXPERIENCE)
		other.experience	= other.experience	+ 1;
	if(self.spawnflags & POWERPOOL_GREENMANA)
		other.greenmana		= other.greenmana	+ 1;
	if(self.spawnflags & POWERPOOL_BLUEMANA)
		other.bluemana		= other.bluemana	+ 1;

	self.ltime = time + 0.15;
}

//QUAKED power_pool (0 1 0) ? HEALTH EXPERIENCE GREENMANA BLUEMANA
//Power pool.  You can pick whatever combination of benefits you would like.

void	power_pool()
{
	if(!self.spawnflags)
		{
		remove(self);
		return;
		}
	self.touch = power_pool_touch;
	self.solid = SOLID_TRIGGER;
	if(!self.count)
		self.count = 5;
}
*/
//============================================================================

//Launches a nail in a offset spread (jweier)
void launch_spread(float offset)
{
	local	vector	offang;
	local	vector	org, vec;
   local entity   mis;

	org = self.origin;

	offang = vectoangles (self.movedir - org);
	offang_y = offang_y + offset * 6;
   
   makevectors (offang);

	vec = normalize (v_forward);

	vec_z = 0;

	mis = spawn ();
	mis.owner = self;
	mis.movetype = MOVETYPE_FLYMISSILE;
	mis.solid = SOLID_BBOX;

	mis.angles = vectoangles(vec);
	
	mis.touch = spike_touch;
	mis.classname = "spike";
	mis.think = SUB_Remove;
	thinktime mis : 6;
	setmodel (mis, "models/spike.mdl");
	setsize (mis, VEC_ORIGIN, VEC_ORIGIN);		
	setorigin (mis, org);

	mis.velocity = vec * 1000;
}

//============================================================================


//============================================================================


/*QUAKED misc_fireball (0 .5 .8) (-8 -8 -8) (8 8 8)
Lava Balls
*/

void fire_fly();
void fire_touch();

void misc_fireball()
{
	precache_model ("models/lavaball.mdl");
	self.classname = "fireball";
	thinktime self : random(5);
	self.think = fire_fly;
	if (!self.speed)
		self.speed = 1000;
}

void fire_fly()
{
local entity	fireball;

	fireball = spawn();
	fireball.solid = SOLID_TRIGGER;
	fireball.movetype = MOVETYPE_TOSS;
	fireball.velocity = '0 0 1000';
	fireball.velocity=RandomVector('50 50 0');
	fireball.velocity_z = self.speed + random(200);
	fireball.classname = "fireball";
	setmodel (fireball, "models/lavaball.mdl");
	setsize (fireball, '0 0 0', '0 0 0');
	setorigin (fireball, self.origin);
	thinktime fireball : 5;
	fireball.think = SUB_Remove;
	fireball.touch = fire_touch;
	
	thinktime self : random(3,8);
	self.think = fire_fly;
}

void fire_touch()
{
	T_Damage (other, self, self, 20);
	remove(self);
}


//============================================================================

/*
void() barrel_explode =
{
	self.takedamage = DAMAGE_NO;
	self.classname = "explo_box";
	// did say self.owner
	T_RadiusDamage (self, self, 160, world);
	sound (self, CHAN_VOICE, "weapons/explode.wav", 1, ATTN_NORM);
	particle (self.origin, '0 0 0', 75, 255);

	self.origin_z = self.origin_z + 32;
	BecomeExplosion (FALSE);
};



/*QUAK-ED misc_explobox (0 .5 .8) (0 0 0) (32 32 64)
TESTING THING
*/
/*
void() misc_explobox =
{
	local float	oldz;
	
	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;
	//rj precache_model ("maps/b_explob.bsp");
	setmodel (self, "maps/b_explob.bsp");
	precache_sound ("weapons/explode.wav");
	self.health = 20;
	self.th_die = barrel_explode;
	self.takedamage = DAMAGE_YES;

	self.origin_z = self.origin_z + 2;
	oldz = self.origin_z;
	droptofloor();
	if (oldz - self.origin_z > 250)
	{
		dprint ("item fell out of level at ");
		dprint (vtos(self.origin));
		dprint ("\n");
		remove(self);
	}
};
*/



/*QUAK-ED misc_explobox2 (0 .5 .8) (0 0 0) (32 32 64)
Smaller exploding box, REGISTERED ONLY
*
/*
void() misc_explobox2 =
{
	local float	oldz;
	
	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;
	//rj precache_model2 ("maps/b_exbox2.bsp");
	setmodel (self, "maps/b_exbox2.bsp");
	precache_sound ("weapons/explode.wav");
	self.health = 20;
	self.th_die = barrel_explode;
	self.takedamage = DAMAGE_YES;

	self.origin_z = self.origin_z + 2;
	oldz = self.origin_z;
	droptofloor();
	if (oldz - self.origin_z > 250)
	{
		dprint ("item fell out of level at ");
		dprint (vtos(self.origin));
		dprint ("\n");
		remove(self);
	}
};
*/
//============================================================================

float SPAWNFLAG_SUPERSPIKE	= 1;
float SPAWNFLAG_LASER = 2;

/*
void Laser_Touch()
{
	local vector org;
	
	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}
	
	//sound (self, CHAN_WEAPON, "enforcer/enfstop.wav", 1, ATTN_STATIC);
	org = self.origin - 8*normalize(self.velocity);

	if (other.health)
	{
		SpawnPuff (org, self.velocity*0.2, 15,other);
		T_Damage (other, self, self.owner, 15);
	}
	else
	{
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_GUNSHOT);
		WriteCoord (MSG_BROADCAST, org_x);
		WriteCoord (MSG_BROADCAST, org_y);
		WriteCoord (MSG_BROADCAST, org_z);
	}
	
	remove(self);	
}

void LaunchLaser(vector org, vector vec)
{
	local	vector	vec;
		
	vec = normalize(vec);
	
	newmis = spawn();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLY;
	newmis.solid = SOLID_BBOX;
	newmis.effects = EF_DIMLIGHT;

	setmodel (newmis, "models/javproj.mdl");
	setsize (newmis, '0 0 0', '0 0 0');		

	setorigin (newmis, org);

	newmis.velocity = vec * 600;
	newmis.angles = vectoangles(newmis.velocity);
   
	newmis.angles_y = newmis.angles_y + 30;
	
	thinktime newmis : 5;
	newmis.think = SUB_Remove;
	newmis.touch = Laser_Touch;
}
*/
void spikeshooter_use()
{

	self.enemy = other.enemy;

/*	if (self.spawnflags & SPAWNFLAG_LASER)
	{
		sound (self, CHAN_VOICE, "enforcer/enfire.wav", 1, ATTN_NORM);
		LaunchLaser (self.origin, self.movedir);
	}
	else
	{*/
		sound (self, CHAN_VOICE, "weapons/spike2.wav", 1, ATTN_NORM);
		launch_spike (self.origin, self.movedir);
		newmis.velocity = self.movedir * 500;
		if (self.spawnflags & SPAWNFLAG_SUPERSPIKE)
//	 		newmis.touch = superspike_touch;
	 		newmis.touch = spike_touch;
//	}
}

void shooter_think()
{
	spikeshooter_use ();
	thinktime self : self.wait;
	//newmis.velocity = self.velocity * 500;
}

void sprayshooter_use()
{
   sound (self, CHAN_VOICE, "weapons/spike2.wav", 1, ATTN_NORM);
   launch_spread(random(10));
}

void sprayshooter_think()
{
	sprayshooter_use ();
	thinktime self : self.wait;
}

/*QUAKED trap_spikeshooter_spray (0 .5 .8) (-8 -8 -8) (8 8 8)
When triggered, fires a spike in the direction set in QuakeEd.
*/
void trap_spikeshooter_spray()
{
	SetMovedir ();
	self.use = sprayshooter_use;
	precache_sound ("weapons/spike2.wav");

	if (self.wait == 0)
		self.wait = 1;
	self.nextthink = self.nextthink + self.wait + self.ltime;
	self.think = sprayshooter_think;
}

/*QUAKED trap_spikeshooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser
When triggered, fires a spike in the direction set in QuakeEd.
Laser is only for REGISTERED.
*/

void trap_spikeshooter()
{
	SetMovedir ();
	self.use = spikeshooter_use;
/*	if (self.spawnflags & SPAWNFLAG_LASER)
	{
//		precache_model2 ("models/laser.mdl");
		
		//precache_sound2 ("enforcer/enfire.wav");
		//precache_sound2 ("enforcer/enfstop.wav");
	}
	else*/
		precache_sound ("weapons/spike2.wav");
}


/*QUAKED trap_shooter (0 .5 .8) (-8 -8 -8) (8 8 8) superspike laser
Continuously fires spikes.
"wait" time between spike (1.0 default)
"nextthink" delay before firing first spike, so multiple shooters can be stagered.
*/
void trap_shooter()
{
	trap_spikeshooter ();
	
	if (self.wait == 0)
		self.wait = 1;
	self.nextthink = self.nextthink + self.wait + self.ltime;
	self.think = shooter_think;
}

void () trap_lightning_track =
{
	local vector p1,p2;
	local entity targ;
	local float len;
	
	targ = find (world, classname, "player");  // Get ending point
	
	if (!targ)
	{
		dprint("No target for lightning");
		return;
	}

	if (targ.health <= 0) 
	{
		self.nextthink = -1;
		return;
	}

	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	
	p1 = self.origin;
	p2 = targ.origin;

	len = vlen(p2 - p1);

	traceline(p1, p2, TRUE, self);

	if (len >= self.aflag || trace_fraction < 1)
	{
		if (self.wait == -1 || self.spawnflags & 2)
			self.nextthink = -1;
		else if (self.wait == 1)
			thinktime self : random(self.wait,self.wait+2);
		else 
			thinktime self : self.wait;

		return;
	}
				
	do_lightning (self,1,0,4, p1, p2, self.dmg);

	fx_flash (p2);		// Flash of light

	self.think = trap_lightning_track;
	
	if (self.wait == -1 || self.spawnflags & 2)
		self.nextthink = -1;
	else if (self.wait == 1)
		thinktime self : random(self.wait,self.wait+2);
	else 
		thinktime self : self.wait;
};

void () trap_lightning_use =
{
	local vector p1,p2;
	local entity targ;

	if (!self.target)
	{
		dprint("No target for lightning");
		return;
	}
	
	targ = find (world, targetname, self.target);  // Get ending point
	
	if (!targ)
	{
		dprint("No target for lightning");
		return;
	}

	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	
	p1 = self.origin;
	p2 = targ.origin;
				
	WriteByte (MSG_ALL, SVC_TEMPENTITY);
	WriteByte (MSG_ALL, TE_LIGHTNING1);
	WriteEntity	(MSG_ALL, self);
	
	WriteCoord (MSG_ALL, p1_x);
	WriteCoord (MSG_ALL, p1_y);
	WriteCoord (MSG_ALL, p1_z);

	WriteCoord (MSG_ALL, p2_x);
	WriteCoord (MSG_ALL, p2_y);
	WriteCoord (MSG_ALL, p2_z);

	LightningDamage (p1, p2, self, self.dmg,"lightning");

	fx_flash (p2);		// Flash of light
};

/*QUAKED trap_lightning (0 1 1) (-8 -8 -8) (8 8 8) TRACK ONCE
Generates a bolt of lightning which ends at the weather_lightning_end that is the target
-------------------------FIELDS-------------------------
noise  - sound generated when lightning appears
      1 - no sound
      2 - lightning (default)

wait - time between shots
aflag - radius limiter
target - be sure to give this a target fx_lightning_end to hit
dmg - damage this bolt does
--------------------------------------------------------
*/
void () trap_lightning =
{
	self.movetype = MOVETYPE_NOCLIP;
	self.owner = self;
	self.solid = SOLID_NOT;
	setorigin (self,self.origin);
	setmodel (self,self.model);
	setsize (self,self.mins, self.maxs);

	if (!self.noise)
		self.noise = "raven/lightng1.wav"; 

	if (!self.dmg)
		self.dmg = 10;

	if (!self.wait)
		self.wait = 1;

	if (!self.aflag)
		self.aflag = 500;

	self.ltime = time;

	self.noise = "raven/lightng1.wav"; 
	precache_sound ("raven/lightng1.wav");

	if (self.spawnflags & 1) 
		self.use = trap_lightning_track;
	else
		self.use = trap_lightning_use;		// For triggered lightning
};


/*===============================================================================


===============================================================================
*/


//void make_bubbles();
void bubble_remove();
void bubble_bob();

/*QUAK-ED air_bubbles (0 .5 .8) (-8 -8 -8) (8 8 8)

testing air bubbles
*/
/*
void air_bubbles()
{
	if (deathmatch)
	{
		remove (self);
		return;
	}
	precache_model ("models/s_bubble.spr");
	thinktime self : 1;
	self.think = make_bubbles;
}


void make_bubbles()
{
local entity	bubble;

	bubble = spawn_temp();
	setmodel (bubble, "models/s_bubble.spr");
	setorigin (bubble, self.origin);
	bubble.movetype = MOVETYPE_NOCLIP;
	bubble.solid = SOLID_NOT;
	bubble.velocity = '0 0 15';
	thinktime bubble : 0.5;
	bubble.think = bubble_bob;
	bubble.touch = bubble_remove;
	bubble.classname = "bubble";
	bubble.frame = 0;
	bubble.cnt = 0;
	setsize (bubble, '-8 -8 -8', '8 8 8');
	thinktime self : random(0.5,1.5);
	self.think = make_bubbles;
}
*/
void() bubble_split =
{
local entity	bubble;
	bubble = spawn_temp();
	setmodel (bubble, "models/s_bubble.spr");
	setorigin (bubble, self.origin);
	bubble.movetype = MOVETYPE_NOCLIP;
	bubble.solid = SOLID_NOT;
	bubble.velocity = self.velocity;
	thinktime bubble : 0.5;
	bubble.think = bubble_bob;
	bubble.touch = bubble_remove;
	bubble.classname = "bubble";
	bubble.frame = 1;
	bubble.cnt = 10;
	setsize (bubble, '-8 -8 -8', '8 8 8');
	self.frame = 1;
	self.cnt = 10;
	if (self.waterlevel != 3)
		remove (self);
};

void() bubble_remove =
{
	if (other.classname == self.classname)
	{
//		dprint ("bump");
		return;
	}
	remove(self);
};


void() bubble_bob =
{
local float		rnd1, rnd2, rnd3;

local float waterornot;
	waterornot=pointcontents(self.origin);
	if (waterornot!=CONTENT_WATER&&waterornot!=CONTENT_SLIME)
		remove(self);
	self.cnt = self.cnt + 1;
	if (self.cnt == 4)
		bubble_split();
	if (self.cnt == 20)
		remove(self);

	rnd1 = self.velocity_x + random(-10,10);
	rnd2 = self.velocity_y + random(-10,10);
	rnd3 = self.velocity_z + random(10,20);

	if (rnd1 > 10)
		rnd1 = 5;
	if (rnd1 < -10)
		rnd1 = -5;
		
	if (rnd2 > 10)
		rnd2 = 5;
	if (rnd2 < -10)
		rnd2 = -5;
		
	if (rnd3 < 10)
		rnd3 = 15;
	if (rnd3 > 30)
		rnd3 = 25;
	
	self.velocity_x = rnd1;
	self.velocity_y = rnd2;
	self.velocity_z = rnd3;
		
	thinktime self : 0.5;
	self.think = bubble_bob;
};


/*~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>
~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~<~>~*/

/*QUAK-ED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
/*
void viewthing()
{
	self.movetype = MOVETYPE_NONE;
	self.solid = SOLID_NOT;
//	precache_model ("models/player.mdl");
//	setmodel (self, "models/player.mdl");
}
*/


/*
==============================================================================

SIMPLE BMODELS

==============================================================================
*/

void() func_wall_use =
{	// change to alternate textures
	self.frame = 1 - self.frame;
};

/*QUAKED func_wall (0 .5 .8) ? TRANSLUCENT
This is just a solid wall if not inhibitted
TRANSLUCENT - makes it see-through
abslight = how bright to make it
*/
void func_wall()
{
	self.angles = '0 0 0';
	self.movetype = MOVETYPE_PUSH;	// so it doesn't get pushed by anything
	self.solid = SOLID_BSP;
	self.classname="solid wall";
	self.use = func_wall_use;
	setmodel (self, self.model);
	if(self.spawnflags&1)
		self.drawflags=DRF_TRANSLUCENT;
	if(self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;
}


/*QUAKED func_illusionary (0 .5 .8) ? TRANSLUCENT LIGHT
A simple entity that looks solid but lets you walk through it.
*/
void func_illusionary()
{
	if (self.spawnflags & 1) 
		self.drawflags (+) DRF_TRANSLUCENT;

	if (self.abslight)
		self.drawflags (+) MLS_ABSLIGHT;

	if (self.spawnflags & 2)
		self.drawflags (+) MLS_ABSLIGHT;

	self.classname="illusionary wall";
	self.angles = '0 0 0';
	self.movetype = MOVETYPE_NONE;
	self.solid = SOLID_NOT;
	setmodel (self, self.model);
	makestatic (self);
}

//============================================================================

/*
void noise_think()
{
	thinktime self : 0.5;
	//sound (self, 1, "enforcer/enfire.wav", 1, ATTN_NORM);
	//sound (self, 2, "enforcer/enfstop.wav", 1, ATTN_NORM);
//	sound (self, 3, "enforcer/sight1.wav", 1, ATTN_NORM);
//	sound (self, 4, "enforcer/sight2.wav", 1, ATTN_NORM);
//	sound (self, 5, "enforcer/sight3.wav", 1, ATTN_NORM);
//	sound (self, 6, "enforcer/sight4.wav", 1, ATTN_NORM);
//	sound (self, 7, "enforcer/pain1.wav", 1, ATTN_NORM);
}
*/

/*QUAKED misc_noisemaker (1 0.5 0) (-10 -10 -10) (10 10 10)
For optimzation testing, starts a lot of sounds.
*/
/*void misc_noisemaker()
{
	//precache_sound2 ("enforcer/enfire.wav");
	//precache_sound2 ("enforcer/enfstop.wav");
//	precache_sound2 ("enforcer/sight1.wav");
//	precache_sound2 ("enforcer/sight2.wav");
//	precache_sound2 ("enforcer/sight3.wav");
//	precache_sound2 ("enforcer/sight4.wav");
//	precache_sound2 ("enforcer/pain1.wav");
//	precache_sound2 ("enforcer/pain2.wav");
//	precache_sound2 ("enforcer/death1.wav");
//	precache_sound2 ("enforcer/idle1.wav");

	thinktime self : random(0.1,1.1);
	self.think = noise_think;
}
*/

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS BREAK GRADUAL TOGGLE_REVERSE KEEP_START
You need to have an origin brush as part of this entity.  The  
center of that brush will be the point around which it is rotated. 
It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.
BREAK makes the brush breakable
REVERSE will cause the it to rotate in the opposite direction.
GRADUAL will make it slowly speed up and slow down.
TOGGLE_REVERSE will make it go in the other direction next time it's triggered
KEEP_START means it will return to it's starting position when turned off

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)
"lifetime" this will make it stop after a while, then start up again after "wait".  Default is staying on.
"wait" if it has a lifetime, this is how long it will wait to start up again.  default is 3 seconds.
"thingtype" type of brush (if breakable - default is wood)
"health" (if breakable - default is 25)
"abslight" - to set the absolute light level
"anglespeed" - If using GRADUAL, this will determine how fast the speed up and down will occur.  1 will be very slow, 100 will be instant.  (Default is 10)

thingtype - type of chunks and sprites it will generate
    0 - glass (default)
    1 - stone
    2 - wood
    3 - metal
    4 - flesh 
    
health - amount of damage item can take.  Default is based on thingtype
   glass -  25
   stone -  75
   wood -   50
   metal - 100
   flesh -  30
*/
void() rotating_use;
void() rotating_touch;
void rotate_wait (void)
{
	thinktime self : 10000000000;
}

void rotate_reset (void)
{
	if(self.wait)
	{
		self.think=rotating_use;
		thinktime self : self.wait;
	}
	else
	{
		self.think=SUB_Null;
		self.nextthink=-1;
	}
}

void rotate_wait_startpos (void)
{
	if(self.angles==self.o_angle)
	{
		self.avelocity='0 0 0';
		rotate_reset();
	}
	else
		thinktime self : 0.05;
}

void rotate_slowdown (void)
{
	self.level-=(self.speed/self.anglespeed);
	if((self.dmg==-1||self.dmg==666)&&self.level<100)
		self.touch=SUB_Null;

	if(self.level<1||(self.level<=self.speed/self.anglespeed&&self.spawnflags&KEEP_START))
	{
		if(self.spawnflags&KEEP_START)
		{
			self.think=rotate_wait_startpos;
			thinktime self : 0;
		}
		else		
		{
			self.avelocity='0 0 0';
			rotate_reset();
		}
	}
	else 
	{
		self.avelocity=self.movedir*self.level;
		self.think=rotate_slowdown;
		thinktime self : 0.01;
	}
}

void rotate_startup (void)
{
	self.level+=(self.speed/self.anglespeed);
	if((self.dmg==-1||self.dmg==666)&&self.level>=100&&self.touch==SUB_Null)
		self.touch=rotating_touch;

	if(self.pain_finished<=time&&self.lifetime)
	{
		self.think=rotating_use;
		thinktime self : 0;
		return;
	}

	if(self.level<self.speed)
	{
		self.avelocity = self.movedir * self.level;
		self.think=rotate_startup;
		thinktime self : 0.01;
	}
	else 
	{
		//dprint("reached max speed\n");
		self.level = self.speed;
		self.avelocity = self.movedir * self.speed;
		if(self.pain_finished>time&&self.lifetime)
		{
			self.think=rotating_use;
			thinktime self : self.pain_finished;
			return;
		}
		else
		{
			self.think=rotate_wait;
			thinktime self : 10000000000;
		}
	}
}

void rotating_use()
{
	if (self.avelocity != '0 0 0')
	{
		if(!self.spawnflags&GRADUAL)
		{
			self.avelocity='0 0 0';
			rotate_reset();
		}
		else if(self.think==rotate_slowdown)
			return;
		else
		{
			sound (self, CHAN_VOICE, self.noise2, 1, ATTN_NORM);
			self.think=rotate_slowdown;
			thinktime self : 0;
		}
	}
	else
	{
		if(self.lifetime)
			self.pain_finished=time+self.lifetime;
		if(self.spawnflags&TOGGLE_REVERSE)
			self.movedir= self.movedir*-1;
		if(!self.spawnflags&GRADUAL)
		{
			self.avelocity = self.movedir * self.speed;
			self.think=rotating_use;
			thinktime self : 10000000000;
		}
		else
		{
			sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
			self.think=rotate_startup;
			thinktime self : 0;
		}
	}
}

void rotating_damage (entity chopped_liver)
{
	if(self.dmg==666)
	{
		if(chopped_liver.classname=="player"&&chopped_liver.flags2&FL_ALIVE)
		{
			chopped_liver.decap=TRUE;
			T_Damage (chopped_liver, self, self, chopped_liver.health+300);
		}
		else
			T_Damage (chopped_liver, self, self, chopped_liver.health+50);
	}
	else if(self.dmg==-1&&chopped_liver.health)
	{
	float damg;
		chopped_liver.deathtype="chopped";
		damg=vlen(self.avelocity);
		T_Damage (chopped_liver, self, self, damg);
	}
}
	
void rotating_touch()
{
	if(!other.takedamage)
		return;
	rotating_damage(other);		
}

void rotating_blocked (void)
{
	if(!other.takedamage)
		return;

	rotating_damage(other);		

	if(other.health>100&&!other.flags2&FL_ALIVE)//allow for blockage
	{
		self.avelocity='0 0 0';
		self.level=0;
		self.touch=SUB_Null;
		self.think=rotating_use;
		thinktime self : self.wait;
	}
}

void func_rotating()
{
	// set the axis of rotation
	if (self.spawnflags & 4)
		self.movedir = '0 0 1';
	else if (self.spawnflags & 8)
		self.movedir = '1 0 0';
	else
		self.movedir = '0 1 0';

	// check for reverse rotation
	if (self.spawnflags & 2)
		self.movedir = self.movedir * -1;

	if(self.spawnflags&TOGGLE_REVERSE)
		self.movedir = self.movedir * -1;

	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	self.classname="rotating non-door";
	setorigin (self, self.origin);	
	setmodel (self, self.model);

	self.use = rotating_use;
	self.blocked = rotating_blocked;
	self.touch=SUB_Null;

	if (!self.speed)
		self.speed = 100;

	if(!self.anglespeed)
		self.anglespeed = 10;

	if (self.dmg==0)
		self.dmg = 2;

	if(self.lifetime)
		if(!self.wait)
			self.wait=3;

//	self.noise1 = "doors/hydro1.wav";
//	self.noise2 = "doors/hydro2.wav";

//	precache_sound ("doors/hydro1.wav");
//	precache_sound ("doors/hydro2.wav");

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;

	if (self.spawnflags & ROTATE_BREAK)
	{
	  if (!self.thingtype)
	   self.thingtype = THINGTYPE_WOOD;
	  if (!self.health)
	  {
		if ((self.thingtype == THINGTYPE_GLASS) || (self.thingtype == THINGTYPE_CLEARGLASS))
			self.health = 25;
		else if ((self.thingtype == THINGTYPE_GREYSTONE) || (self.thingtype == THINGTYPE_BROWNSTONE))
			self.health = 75;
		else if (self.thingtype == THINGTYPE_WOOD)
			self.health = 50;
		else if (self.thingtype == THINGTYPE_METAL)
			self.health = 100;
		else if (self.thingtype == THINGTYPE_FLESH)
			self.health = 30;
		else
			self.health = 25;
	  }
	  self.takedamage = DAMAGE_YES;
	  self.th_die = chunk_death;
	}

	if(self.spawnflags&KEEP_START)
		self.o_angle=self.angles;

	if (self.spawnflags & 1)
		self.use();

	if (self.flags2)
	{
		self.touch = rotating_touch;
		self.flags2=FALSE;
	}
}

/*QUAK-ED trigger_fan_blow (0 .5 .8) ? 
Will blow anything in the direction of the func_rotating it's targeted by.
Note that clockwise rotation pulls you towards it, counterclockwise pushes you away- func_rotating design should match this.
To use, target this trigger with with the func_rotating (do NOT target the func_rotating with the trigger!!!).
Then place this trigger so that it covers both front and back of the "fan" and extend it as far as you want it to have influence.
*/

void trigger_find_owner (void)
{
entity found;
	found=find(world,target,self.targetname);
	if(found==world)
		remove(self);
	else
		self.owner=found;
}
/*
void trigger_fan_blow_touch (void)
{
vector blowdir, org;
float blowhard, blowdist;
	if(other==self.owner)
		return;

	if(self.owner.origin=='0 0 0')
		org=(self.owner.absmin+self.owner.absmax)*0.5;
	else
		org=self.owner.origin;

	if(self.owner.avelocity_x!=0)
	{
//FIXME: Need to cheat here?  dilute avelocity?
		blowhard=self.owner.avelocity_x/3;
		blowdir = '0 1 0';
		blowdist = fabs(org_y - other.origin_y);
	}
	else if(self.owner.avelocity_y!=0)
	{
		blowhard = self.owner.avelocity_y;
		blowdir = '0 0 1';
		blowdist = fabs(org_z - other.origin_z);
	}
	else if(self.owner.avelocity_z!=0)
	{
//FIXME: Need to cheat here?  dilute avelocity?
		blowhard=self.owner.avelocity_z/3;
		blowdir = '1 0 0';
		blowdist = fabs(org_x - other.origin_x);
	}
	else 
		return;
	if(blowdist<100)
		blowdist=0;

	if(blowhard>0)
	{
		blowhard-=blowdist;
		if(blowhard<=0)
			return;
	}
	else
	{
		blowhard+=blowdist;
		if(blowhard>=0)
			return;
	}
	blowhard/=10;

//FIXME: Factor in mass?
	blowdir*=blowhard;
//FIXME: Need to cheat here?  Pull down much faster?
	if(blowdir_z<0)
		blowdir*=10;
//FIXME: Will actually slow someone down if their already moving in this direction!
	if(other.velocity!=blowdir)
		other.velocity+=blowdir;
//	if(!other.flags&FL_ONGROUND)
//	{
//		if(pointcontents(other.origin)==CONTENT_EMPTY&&other.movetype!=MOVETYPE_FLY&&other.movetype!=MOVETYPE_FLYMISSILE&&other.movetype!=MOVETYPE_BOUNCEMISSILE)
//			other.velocity_z-=60;//FIXME:  calculate in gravity too
//	}
//	else
	if(other.flags&FL_ONGROUND)
	{
		other.flags(-)FL_ONGROUND;
		if(other.velocity_z==0)
			other.velocity_z+=7;
	}
}

void trigger_fan_blow (void)
{
	InitTrigger();
	self.touch=trigger_fan_blow_touch;
	self.think=trigger_find_owner;
	thinktime self : 0.1;
}
*/

void() angletrigger_done =
{
	self.level = FALSE;
};

void() angletrigger_blocked =
{
	T_Damage (other, self, self, self.dmg);
};

void() angletrigger_use =
{
vector newvect;

	if (self.level)
		return;
	else
		self.level = TRUE;

	if(self.angles_x>=360)
		self.angles_x-=360;
	else if(self.angles_x<=-360)
		self.angles_x+=360;
	if(self.angles_y>=360)
		self.angles_y-=360;
	else if(self.angles_y<=-360)
		self.angles_y+=360;
	if(self.angles_z>=360)
		self.angles_z-=360;
	else if(self.angles_z<=-360)
		self.angles_z+=360;
	newvect = self.movedir * self.cnt;

	if (self.angles + newvect == self.mangle) 
	{
		self.check_ok = TRUE;
		SUB_UseTargets();
	}
	else if (self.check_ok)
	{
		self.check_ok = FALSE;
		SUB_UseTargets();
	}
	
	SUB_CalcAngleMove(self.angles + newvect, self.speed, angletrigger_done);
};

/*QUAKED func_angletrigger (0 .5 .8) ? REVERSE X_AXIS Y_AXIS
Rotates at certain intervals, and fires off when a set angle is met

mangle = desired angle to trigger at (relative to the world!)
cnt	 = degrees to turn each move
dmg	 = damage if blocked
*/

void() func_angletrigger =
{
	
	// set the axis of rotation
	if (self.spawnflags & 2)
		self.movedir = '0 0 1';
	else if (self.spawnflags & 4)
		self.movedir = '1 0 0';
	else
		self.movedir = '0 1 0';

	// check for clockwise rotation
	if (self.spawnflags & 1)
		self.movedir = self.movedir*-1;

	self.pos1 = self.angles;
	self.pos2 = self.angles + self.movedir * self.cnt;

	self.max_health = self.health;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.classname = "angletrigger";

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;

	if (!self.speed)
		self.speed = 100;
	if (self.wait==0)
		self.wait = 1;
	if (!self.dmg)
		self.dmg = 2;
	
/*
	precache_sound ("doors/hydro1.wav");
	precache_sound ("doors/hydro2.wav");
	precache_sound ("doors/basetry.wav");
	precache_sound ("doors/baseuse.wav");
	self.noise2 = "doors/hydro1.wav";
	self.noise1 = "doors/hydro2.wav";
	self.noise3 = "doors/basetry.wav";
	self.noise4 = "doors/baseuse.wav";
*/
	self.blocked = angletrigger_blocked;
	self.use = angletrigger_use;
	
	if (!self.targetname)
		self.touch = angletrigger_use;

	self.inactive = FALSE;	
};

