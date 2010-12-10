/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/weather.hc,v 1.3 2007-02-07 16:57:11 sezero Exp $
 */
/*
void () dust_touch =
{
  local float numPuffs, thisPuff;
  local vector dustpos, dustvect;
  
  numPuffs = random(5, 10) + 2;
  thisPuff = 1;

  while (thisPuff < numPuffs)
  {
		dustpos_x = random(self.maxs_x, self.mins_x);
		dustpos_y = random(self.maxs_y, self.mins_y);
		dustpos_z = self.maxs_z;
   
		dustvect_x = random(10, 20);
		dustvect_y = random(10, 20);
		dustvect_z = random(-10, -1);
	
		SpawnPuff (dustpos, dustvect, 414, world);
		thisPuff += 1;
  }
};
*/

/*QUAKED weather_dust (0 1 1) ? 
Dust falls, or is stirred up within this entity.
-------------------------FIELDS-------------------------
color   - palette number of the dust's color
        default - 101
*/
/*
void() weather_dust =
{

  	self.movetype = MOVETYPE_NOCLIP;
  	self.owner = self;
  	self.solid = SOLID_NOT;
  	setsize (self, self.mins , self.maxs);
  	setorigin (self, self.origin);
  	setmodel (self, self.model);
   self.modelindex = 0;
  	self.model = "";
	        
   self.touch = dust_touch;
	self.use = dust_touch;
	
	if (!self.color) self.color=101;
};
*/
void () rain_use =
{
	dprint(" rain ");

	rain_go(self.mins,self.maxs,self.size,'125 100 0',self.color+random(8),self.counter);
	self.nextthink = time + self.wait;
};

float RAIN_STRAIGHT = 1;

/*QUAKED weather_rain (0 1 1) ? STRAIGHT NO_SPLAT
MG&RL
Rain falls within this entity.
STRAIGHT = Rain will fall straight down rather than at an angle
NO_SPLAT = Rain will not make "splats" on the ground
-------------------------FIELDS-------------------------
color   - palette number of the rain's color - will range between this color and this color + 8 unless monochrome is checked
        default - 414 (translucent blue)
counter - number of rain particles to generate every 10th of a second.
        default - 300
wait    - how often to generate rain particles
        default - .1 of a second
soundtype - 0 = rain (default)
			1 = drip (cave-type- wetness)
--------------------------------------------------------
*/
void rain_splatter ()
{//MG
vector spot1,spot2;
	spot1=self.mins;
	spot1_x+=random(self.size_x);
	spot1_y+=random(self.size_y);
	spot1_z=self.maxs_z;
	spot2=spot1;
	spot2_z=self.mins_z - 10;
	
	traceline(spot1,spot2,FALSE,self);
	if(trace_fraction<1)
	{
//		particle4(trace_endpos,10,random(408,412),PARTICLETYPE_FASTGRAV,random(5,10));
		particle4(trace_endpos,10,random(408,412),PARTICLETYPE_GRAV,random(5,10));
		thinktime self : random(0.01,0.2);
	}
	else
		thinktime self : 0;
}

void make_splatter ()
{
	newmis=spawn();
	setorigin(newmis,self.origin);
	setsize(newmis,self.mins,self.maxs);
	newmis.think=rain_splatter;
	thinktime newmis : 0;
}

void() weather_rain =
{//MG&BL
vector dir;
float splat_count;
	if (!self.soundtype)
	{
		precache_sound2("ambience/rain.wav");
		self.noise1="ambience/rain.wav";
	}
	else
	{
		precache_sound2("ambience/drip1.wav");
		self.noise1 = ("ambience/drip1.wav");
		self.think = sound_again;
		self.nextthink = time + random(5,30);
	}

  	self.movetype = MOVETYPE_NOCLIP;
  	self.owner = self;
  	self.solid = SOLID_NOT;
  	setsize (self, self.mins , self.maxs);
  	setorigin (self, self.origin);
  	setmodel (self, self.model);
   self.modelindex = 0;
  	self.model = "";
	
	if (!self.wait)
	   self.wait=0.10;
	       
	if (!self.color)
		self.color=414;

	if (!self.counter)
		self.counter=300;

	if (self.spawnflags & RAIN_STRAIGHT)
		dir = '0 0 0';
	else
		dir = '125 100 0';

	self.color=414;
	starteffect(CE_RAIN, self.mins, self.maxs, self.size, dir, self.color, self.counter, self.wait);

	if(!self.soundtype)
		ambientsound (self.origin, "ambience/rain.wav",1 , ATTN_NONE);
	else
		sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);

	if(!self.spawnflags&2)
		splat_count=10;
	while(splat_count)
	{
		make_splatter();
		splat_count-=1;
	}
	if(!self.soundtype)
		remove(self);
};

void () weather_lightning_use =
{
	local vector p1,p2;
	local entity targ;

	if (self.classname == "weather_sunbeam_start")
		sound(self,CHAN_WEAPON,"crusader/sunhum.wav",1,ATTN_NORM);
	
	if (!self.target)
	{
		dprint("No target for lightning\n");
		return;
	}
	
	targ = find (world, targetname, self.target);  // Get ending point
	
	if (!targ)
	{
		dprint("No target for beam effect\n");
		return;
	}

/*	if(!self.aflag&&self.spawnflags&2)
	{
		self.aflag=TRUE;
		if(random()<0.5)
			sound (self,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
		else
			sound (self,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
	}*/

	
	p1 = self.origin;
	p2 = targ.origin;
	p1+=normalize(p2-p1)*15;	//So beam is drawn at startpoint
				
	if(self.classname=="weather_lightning_start")
		do_lightning (self,1,0,4,p1,p2,10);

	else if(self.classname=="weather_sunbeam_start")
	{
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_STREAM_SUNSTAFF1);
		WriteEntity (MSG_BROADCAST, self);
		WriteByte (MSG_BROADCAST, 0);
		WriteByte (MSG_BROADCAST, 4);
	
		WriteCoord (MSG_BROADCAST, p1_x);
		WriteCoord (MSG_BROADCAST, p1_y);
		WriteCoord (MSG_BROADCAST, p1_z);

		WriteCoord (MSG_BROADCAST, p2_x);
		WriteCoord (MSG_BROADCAST, p2_y);
		WriteCoord (MSG_BROADCAST, p2_z);
		
		LightningDamage (p1, p2, self, 10,"sunbeam");
	}
	else if(self.classname=="fx_colorbeam_start")
	{
//FIXME: make a temp ent colored beam where you can pass the color
//	Uses 1 model, but several diff. color skins
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_STREAM_COLORBEAM);	//beam type
		WriteEntity (MSG_BROADCAST, self);				//owner
		WriteByte (MSG_BROADCAST, 0);					//tag + flags
		WriteByte (MSG_BROADCAST, 4);					//time
		WriteByte (MSG_BROADCAST, self.color);			//color
	
		WriteCoord (MSG_BROADCAST, p1_x);
		WriteCoord (MSG_BROADCAST, p1_y);
		WriteCoord (MSG_BROADCAST, p1_z);

		WriteCoord (MSG_BROADCAST, p2_x);
		WriteCoord (MSG_BROADCAST, p2_y);
		WriteCoord (MSG_BROADCAST, p2_z);
	}
	else
		return;

	if (self.lifetime > time )  // Not done living
		thinktime self : 0.2;
  	else if (self.wait>-1)  // constantly running lightning needs to be reset
  	{
		thinktime self : self.wait;
		self.think = weather_lightning_use;
		self.lifetime = self.lifespan + self.nextthink;
		self.aflag=FALSE;
  	}
	else
		self.nextthink=-1;
};


void () lightning_init =
{
	if (self.lifetime > time )  // Being triggered right now
		return; 

	self.think = weather_lightning_use;
	self.lifetime = self.lifespan + time;
	if(self.spawnflags&2)
	{
		if(random()<0.5)
			sound (self,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
		else
			sound (self,CHAN_AUTO,"crusader/lghtn1.wav",1,ATTN_NORM);
		self.aflag=TRUE;
	}
	weather_lightning_use ();  // Make it run

};


/*QUAKED weather_lightning_start (0 1 1) (-8 -8 -8) (8 8 8) STARTOFF thunder_sound
Generates a bolt of lightning which ends at the weather_lightning_end that is the target

STARTOFF - if clicked the lightning becomes triggerable, will run constantly once triggered. Wait will still tell how long to wait between strikes
thunder_sound - will randomly play lightning strike sounds when it fires
-------------------------FIELDS-------------------------
noise  - sound generated when lightning appears
      1 - no sound
      2 - lightning (default)

target - be sure to give this a target fx_lightning_end to hit

wait   - amount of time between strikes
      -1   lightning is triggerable.  
      > -1 lightning is constantly running.
      default -1

lifespan - amount of time lightning bolt will exist.
--------------------------------------------------------
*/
void () weather_lightning_start =
{

	self.movetype = MOVETYPE_NOCLIP;
	self.owner = self;
	self.solid = SOLID_NOT;
	setorigin (self,self.origin);
	setmodel (self,self.model);
	setsize (self,self.mins, self.maxs);

	if(self.classname=="weather_lightning_start")
	{
		precache_model("models/stlghtng.mdl");
		if (!self.noise)
		{
			self.noise = "raven/lightng1.wav"; 
			precache_sound("raven/lightng1.wav");
		}
		if(self.spawnflags&2)
		{
			self.noise="";
			precache_sound2("crusader/lghtn1.wav");
		}
	}
	else if (self.classname == "weather_sunbeam_start" && self.noise == "2")
	{
		dprint("Shazzaam!\n");
		self.noise = "crusader/lghtn1.wav";
		precache_sound("crusader/lghtn1.wav");
	}
	
	self.use = lightning_init;		// For triggered lightning
	self.think = weather_lightning_use;

	if (!self.wait)	// Set wait to default
		self.wait = 5;

	if ((self.wait>-1) && (!self.spawnflags & 1))   // Constantly running lightning
	{
		thinktime self : self.wait;
		self.lifetime = time + self.lifespan + self.wait;
	}
};

/*QUAKED weather_lightning_end (0 1 1) (-8 -8 -8) (8 8 8)
Where lightning from weather_lightning_start will hit.
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void () weather_lightning_end =
{
	self.movetype = MOVETYPE_NOCLIP;
	self.owner = self;
	self.solid = SOLID_NOT;
	setsize (self,self.mins, self.maxs);
	setorigin (self,self.origin);
};


/*QUAKED weather_sunbeam_start (0 1 1) (-8 -8 -8) (8 8 8) STARTOFF
Generates a ray of sunlight which ends at the weather_sunbeam_end that is the target

STARTOFF - if clicked the sunbeam becomes triggerable, will run constantly once triggered. Wait will still tell how long to wait between strikes
-------------------------FIELDS-------------------------
noise  - sound generated when lightning appears
      1 - no sound (default)
      2 - lightning 

target - be sure to give this a target fx_lightning_end to hit

wait   - amount of time between strikes
      -1   sunbeam is triggerable.  
      > -1 sunbeam is constantly running.
      default -1

lifespan - amount of time sunbeam will exist.
--------------------------------------------------------
*/
void () weather_sunbeam_start =
{
	precache_model ("models/stsunsf1.mdl");
	precache_model ("models/stsunsf2.mdl");
	precache_model ("models/stsunsf3.mdl");
	precache_model ("models/stsunsf4.mdl");

	self.noise = "crusader/lghtn1.wav";

	weather_lightning_start();
};

/*QUAKED weather_sunbeam_end (0 1 1) (-8 -8 -8) (8 8 8)
Where sunbeam from weather_sunbeam_start will hit.
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void () weather_sunbeam_end =
{
	weather_lightning_end();
};

/*QUAKED fx_colorbeam_start (0 1 1) (-8 -8 -8) (8 8 8) STARTOFF
Generates a ray of colored light which ends at the fx_colorbeam_end that is the target

STARTOFF - if clicked the colorbeam becomes triggerable, will run constantly once triggered. Wait will still tell how long to wait between strikes
-------------------------FIELDS-------------------------
noise  - sound generated when lightning appears
      1 - no sound (default)
      2 - lightning 

target - be sure to give this a target fx_lightning_end to hit

wait   - amount of time between strikes
      -1   beam is triggerable.  
      > -1 beam is constantly running.
      default -1

color - color of the beam
	0 = red (default)
	1 = blue
	2 = green
	3 = white
	4 = yellow

lifespan - amount of time beam will exist.
--------------------------------------------------------
*/
/*
void () fx_colorbeam_start =
{
	precache_model("models/stclrbm.mdl");
	weather_lightning_start();
};
*/

/*QUAKED fx_colorbeam_end (0 1 1) (-8 -8 -8) (8 8 8)
Where colorbeam from weather_colorbeam_start will hit.
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void () fx_colorbeam_end =
{
	weather_lightning_end();
};
*/

