/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/fx.hc,v 1.1.1.1 2004-11-29 11:32:12 sezero Exp $
 */

float WHITE_PUFF	= 0;
float RED_PUFF		= 1;
float GREEN_PUFF	= 2;
float GREY_PUFF		= 3;


void CreateTeleporterBodyEffect (vector org,vector vel,float framelength)
{
	starteffect(CE_TELEPORTERBODY, org,vel,framelength);
}


void CreateTeleporterSmokeEffect (vector org,vector vel,float framelength)
{
	starteffect(CE_TELEPORTERPUFFS, org,vel,framelength);
}

// ============= SMOKE ================================

void CreateWhiteSmoke (vector org,vector vel,float framelength)
{
	starteffect(CE_WHITE_SMOKE, org,vel,framelength);
}

void CreateRedSmoke (vector org,vector vel,float framelength)
{
	starteffect(CE_RED_SMOKE, org,vel, framelength);
}

void CreateGreySmoke (vector org,vector vel,float framelength)
{
	starteffect(CE_GREY_SMOKE, org,vel, framelength);
}

void CreateGreenSmoke (vector org,vector vel,float framelength)
{
	starteffect(CE_GREEN_SMOKE, org,vel, framelength);
}

void CreateRedCloud (vector org,vector vel,float framelength)
{
	starteffect(CE_REDCLOUD, org,vel, framelength);
}
/*
void CreateFlameStream (vector org,vector vel,float framenumber)
{
	starteffect(CE_FLAMESTREAM, org,vel, framenumber);
}
*/
// ============= FLASHES ================================

void CreateLittleWhiteFlash (vector spot)
{
	starteffect(CE_SM_WHITE_FLASH,spot);
}

void CreateLittleBlueFlash (vector spot)
{
	starteffect(CE_SM_BLUE_FLASH,spot);
}

void CreateBlueFlash (vector spot) 
{
	starteffect(CE_BLUE_FLASH,spot);
}

void CreateWhiteFlash (vector spot) 
{
	starteffect(CE_WHITE_FLASH, spot);
}

void CreateYRFlash (vector spot)
{
	starteffect(CE_YELLOWRED_FLASH,spot);
}

// ============= EXPLOSIONS =============================

void CreateBlueExplosion (vector spot)
{
	starteffect(CE_BLUE_EXPLOSION,spot);
}

void CreateExplosion29 (vector spot)
{
	starteffect(CE_BG_CIRCLE_EXP,spot);
}

void CreateFireCircle (vector spot)
{
	starteffect(CE_SM_CIRCLE_EXP,spot);
}

// ============= SPARKS =============================

void CreateRedSpark (vector spot) 
{
	starteffect(CE_REDSPARK,spot);
}

void CreateGreenSpark (vector spot) 
{
	starteffect(CE_GREENSPARK,spot);
}

void CreateBSpark (vector spot) 
{
	starteffect(CE_BLUESPARK,spot);
}

void CreateSpark (vector spot)
{
	starteffect(CE_YELLOWSPARK,spot);
}

//FIXME:This should be a temp entity
void splash_run (void)
{
	float result;

	result = AdvanceFrame(0,5);

	self.nextthink = time + HX_FRAME_TIME;
	self.think = splash_run;

	if (result == AF_END)
	{
		self.nextthink = time + HX_FRAME_TIME;
		self.think = SUB_Remove;
	}
}

/*
void CreateWaterSplash (vector spot)
{
	entity newent;

	newent = spawn();
  	setmodel (newent, "models/wsplash.spr");

	setorigin (newent, spot);
	newent.movetype = MOVETYPE_NOCLIP;
	newent.solid = SOLID_NOT;
	newent.velocity = '0 0 0';
	newent.nextthink = time + 0.05;
	newent.think = splash_run;

}
*/

/*
================
SpawnPuff
================
*/
void  SpawnPuff (vector org, vector vel, float damage,entity victim)
{
	float part_color;
	float rad;

	if(victim.frozen>0)
		part_color = 406+random(8);				// Ice particles
	else if (victim.thingtype==THINGTYPE_FLESH && victim.classname!="mummy" && victim.netname != "spider")
		part_color = 256 + 8 * 16 + random(9);				//Blood red
	else if ((victim.thingtype==THINGTYPE_GREYSTONE) || (victim.thingtype==THINGTYPE_BROWNSTONE))
		part_color = 256 + 20 + random(8);			// Gray
	else if (victim.thingtype==THINGTYPE_METAL)	
		part_color = 256 + (8 * 15);			// Sparks
	else if (victim.thingtype==THINGTYPE_WOOD)	
		part_color = 256 + (5 * 16) + random(8);			// Wood chunks
	else if (victim.thingtype==THINGTYPE_ICE)	
		part_color = 406+random(8);				// Ice particles
	else if (victim.netname == "spider")	
		part_color = 256 + 183 + random(8);		// Spider's have green blood
	else
		part_color = 256 + (3 * 16) + 4;		// Dust Brown

	rad=vlen(vel);
	if(!rad)
		rad=random(10,20);
	particle4(org,rad,part_color,PARTICLETYPE_FASTGRAV,2 * damage);
}

/*-----------------------------------------
	redblast - the red flash sprite
  -----------------------------------------*/
void(vector spot) CreateRedFlash =
{
	starteffect(CE_RED_FLASH,spot);
};

void() DeathBubblesSpawn;

void () flash_remove =
{
	remove(self);
};

void GenerateTeleportSound (entity center)
{
string telesnd;
float r;
	r=rint(random(4))+1;
	if(r==1)
		telesnd="misc/teleprt1.wav";
	else if(r==2)
		telesnd="misc/teleprt2.wav";
	else if(r==3)
		telesnd="misc/teleprt3.wav";
	else if(r==4)
		telesnd="misc/teleprt4.wav";
	else
		telesnd="misc/teleprt5.wav";
	sound(center,CHAN_AUTO,telesnd,1,ATTN_NORM);
}

void GenerateTeleportEffect(vector spot1,float teleskin)
{
	entity sound_ent;

	if (self.attack_finished > time)
		return;

	sound_ent = spawn();
	setorigin(sound_ent,spot1);
	GenerateTeleportSound(sound_ent);
	sound_ent.think = SUB_Remove;
	thinktime sound_ent : 2;

	CreateTeleporterBodyEffect (spot1,'0 0 0',teleskin);  // 3rd parameter is the skin

	CreateTeleporterSmokeEffect (spot1,'0 0 0',HX_FRAME_TIME);
	CreateTeleporterSmokeEffect (spot1 + '0 0 64','0 0 0',HX_FRAME_TIME);

//	GenerateTeleportSound(newent);
//	if (self.scale < 0.11)
//	{
//		particle4(self.origin + '0 0 40',random(5,10),20,PARTICLETYPE_FASTGRAV,random(20,30));
//		particle4(self.origin + '0 0 40',random(5,10),250,PARTICLETYPE_FASTGRAV,random(20,30));
//		remove(self);
//	}

}

void smoke_generator_use(void)
{
	self.use = smoke_generator_use;
	self.nextthink = time + HX_FRAME_TIME;
	if (!self.wait)
		self.wait = 2;	
	self.owner = other;

	if (self.lifespan)
		self.lifetime = time + self.lifespan;

}

void smoke_generator_run(void)
{
	if (self.thingtype == WHITE_PUFF)
		CreateWhiteSmoke(self.origin, '0 0 8', HX_FRAME_TIME *3);
	else if (self.thingtype == RED_PUFF)
		CreateRedSmoke(self.origin, '0 0 8', HX_FRAME_TIME *3);
	else if (self.thingtype == GREEN_PUFF)
		CreateRedSmoke(self.origin, '0 0 8', HX_FRAME_TIME *3);
	else if (self.thingtype == GREY_PUFF)
		CreateGreySmoke(self.origin, '0 0 8', HX_FRAME_TIME *3);

	self.nextthink = time + random(self.wait);
	self.think = smoke_generator_run;

	if ((self.lifespan) && (self.lifetime < time))
		remove(self);
}

/*QUAKED fx_smoke_generator (0 1 1) (-8 -8 -8) (8 8 8)
Generates smoke puffs
-------------------------FIELDS-------------------------
wait - how often it should generate smoke (default 2)
thingtype - type of smoke to generate
 0 - white puff       (fire place)
 1 - red              (lava)
 2 - green            (slime)
 3 - grey             (oil)

lifespan - fill this in and it will only puff for this long
--------------------------------------------------------
*/
void() fx_smoke_generator =
{

	setmodel(self, "models/null.spr");

	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_NONE;

	setsize (self,'0 0 0' , '0 0 0');

	self.th_die = SUB_Remove;

	if (!self.targetname)	//	Not targeted by anything so puff away
		self.nextthink = time + HX_FRAME_TIME;

	self.use = smoke_generator_use;

	if (!self.wait)
		self.wait = 2;


	self.think = smoke_generator_run;
};


void (vector org) fx_flash =
{
	local entity newent;

	newent = spawn();
  	setmodel (newent, "models/s_bubble.spr");

//	setmodel (newent, newent.model);
//	newent.modelindex = 0;
//	newent.model = "";        


	setorigin (newent, org + '0 0 24');
	newent.movetype = MOVETYPE_NOCLIP;
	newent.solid = SOLID_NOT;
	newent.velocity = '0 0 0';
	newent.nextthink = time + 0.5;
	newent.think = flash_remove;
	newent.classname = "bubble";

   newent.effects = EF_BRIGHTLIGHT;

	setsize (newent, '-8 -8 -8', '8 8 8');

};

void () friction_change_touch =
{
	if (other == self.owner)
		return;

	if (other.classname == "player")
		other.friction=self.friction;

};

/*QUAKED fx_friction_change (0 1 1) ?
ONLY WORKS ON PLAYERS
Set the friction within this area.

-------------------------FIELDS-------------------------
'friction' :  this is how quickly the player will slow down after he ceases indicating movement (lets go of arrow keys).

             1       : normal friction
             >0 & <1 : slippery
             >1      : high friction

default = 0

Player's Friction will be reset when they leave the brush's bounds
--------------------------------------------------------
*/

void() fx_friction_change =
{
	self.movetype = MOVETYPE_NONE;
	self.owner = self;
	self.solid = SOLID_TRIGGER;
	setorigin (self, self.origin);
	setmodel (self, self.model);
	self.modelindex = 0;
	self.model = "";

	setsize (self, self.mins , self.maxs);

	self.touch = friction_change_touch;
};


void () gravity_change_touch =
{
	if (other == self.owner||other.gravity==self.gravity)
		return;
	dprintf("Changing gravity to %s\n",self.gravity);
//	if (other.classname == "player")
		other.gravity=other.standard_grav=self.gravity;

};

/*QUAKED fx_gravity_change (0 1 1) ?

Set the gravity within this area.

-------------------------FIELDS-------------------------
'gravity' :  this is how quickly the player will fall

             100       : 1 G
             >0 & <100 : low grav
             >100      : high grav
--------------------------------------------------------
*/

void() fx_gravity_change =
{
	self.gravity/=100;
	self.movetype = MOVETYPE_NONE;
	self.owner = self;
	self.solid = SOLID_TRIGGER;
	setorigin (self, self.origin);
	setmodel (self, self.model);
	self.modelindex = 0;
	self.model = "";

	setsize (self, self.mins , self.maxs);

	self.touch = gravity_change_touch;
};

/*
void() explosion_done =
{
	self.effects=EF_DIMLIGHT;
};
*/

/*
void() explosion_use =
{
	if (self.spawnflags & FLASH)
	{
		self.effects=EF_BRIGHTLIGHT;
		self.think=p_explosion_done;
		self.nextthink= time + 1;
	}
	sound (self, CHAN_BODY, self.noise1, 1, ATTN_NORM);
	particleexplosion(self.origin,self.color,self.exploderadius,self.counter);

};
*/

/*QUAK-ED fx_particle_explosion (0 1 1) ( -5 -5 -5) (5 5 5) FLASH
 Gives off a spray of particles like an explosion.
-------------------------FIELDS-------------------------
 FLASH will cause a brief flash of light.

 "color" is the color of the explosion. Particle colors dim as they move away from the center point.

 color values :
   31 - white
   47 - light blue
   63 - purple
   79 - light green
   85 - light brown
  101 - red  (default)
  117 - light blue
  133 - yellow
  149 - green
  238 - red to orange
  242 - purple to red
  246 - green to purple
  250 - blue - green
  254 - yellow to blue

 "exploderadius" is the distance the particles travel before disappearing. 1 - 10  (default 5)

 "soundtype" the type of sound made during explosion
  0 - no sound
  1 - rocket explosion   (default)
  2 - grenade shoot

  "counter" the number of particles to create
  1 - 1024
  512 (default)

--------------------------------------------------------
*/
/*
void() fx_particle_explosion =
{
	self.effects=0;
	self.use=explosion_use;

	self.movetype = MOVETYPE_NOCLIP;
	self.owner = self;
	self.solid = SOLID_NOT;
	setorigin (self, self.origin);
	setmodel (self, self.model);
	setsize (self, self.mins , self.maxs);

	// Explosion color
	if ((!self.color) || (self.color>254))
		self.color=101;

	// Explosion sound is what type????
	if (self.soundtype>2)
		self.soundtype=0;
	else if (!self.soundtype)
		self.soundtype=1;

	if (self.soundtype==0)
		self.noise1 = ("misc/null.wav");
	else if (self.soundtype==1)
		self.noise1 = ("weapons/explode.wav");
	else if (self.soundtype==2)
		self.noise1 = ("weapons/grenade.wav");

	self.exploderadius = 10 - self.exploderadius;  // This is backwards in builtin function

	// Explosion radius
	if ((self.exploderadius<1) || (self.exploderadius>10))
		self.exploderadius=5;

	// Particle count
	if ((self.counter<1) || (self.counter>1024))
		self.counter=512;

};
*/

/*
 * $Log: not supported by cvs2svn $
 * 
 * 7     3/13/98 1:51p Mgummelt
 * Fixed friction_change entity to work,  made checkbottom use the hull
 * mins/maxs for it's checks, not the bounding box's.
 * 
 * 6     2/12/98 5:55p Jmonroe
 * remove unreferenced funcs
 * 
 * 5     1/26/98 6:18p Mgummelt
 * 
 * 59    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 57    9/29/97 4:36p Rlove
 * 
 * 56    9/29/97 3:00p Rlove
 * 
 * 55    9/25/97 2:31p Rlove
 * 
 * 54    9/01/97 12:48a Jweier
 * 
 * 53    8/31/97 8:52a Mgummelt
 * 
 * 52    8/23/97 7:15p Rlove
 * 
 * 51    8/22/97 4:28p Rlove
 * 
 * 50    8/22/97 11:10a Rlove
 * Added red cloud
 * 
 * 49    8/19/97 2:28p Rlove
 * 
 * 48    8/19/97 2:20p Rlove
 * 
 * 47    8/17/97 3:06p Mgummelt
 * 
 * 46    8/05/97 12:04p Rlove
 * 
 * 45    7/30/97 3:33p Mgummelt
 * 
 * 44    7/28/97 7:50p Mgummelt
 * 
 * 43    7/28/97 1:51p Mgummelt
 * 
 * 42    7/26/97 1:11p Rlove
 * New snake stuff
 * 
 * 41    7/25/97 6:34p Rlove
 * 
 * 40    7/25/97 3:32p Mgummelt
 * 
 * 39    7/24/97 4:06p Rlove
 * 
 * 38    7/24/97 3:53p Rlove
 * 
 * 37    7/22/97 8:10a Rlove
 * Gave smoke puffs a life span
 * 
 * 36    7/21/97 4:03p Mgummelt
 * 
 * 35    7/21/97 4:02p Mgummelt
 * 
 * 34    7/21/97 3:03p Rlove
 * 
 * 33    7/21/97 11:45a Mgummelt
 * 
 * 32    7/17/97 4:54p Rlove
 * 
 * 31    7/17/97 1:53p Rlove
 * 
 * 30    7/15/97 12:28p Mgummelt
 * 
 * 29    7/07/97 5:54p Rlove
 * Spiders bleed green now
 * 
 * 28    6/18/97 6:32p Mgummelt
 * 
 * 27    6/12/97 12:14p Rlove
 * Added red and green sparks
 * 
 * 25    6/03/97 10:26a Rlove
 * 
 * 24    6/03/97 9:02a Rlove
 * 
 * 23    6/03/97 9:00a Rlove
 * Added fx_smoke_generator entity
 * 
 * 22    5/30/97 12:01p Rlove
 * New blue explosion
 * 
 * 21    5/30/97 11:41a Rjohnson
 * Removed the message field of the starteffect
 * 
 * 20    5/28/97 10:45a Rlove
 * Moved sprite effects to client side - smoke, explosions, and flashes.
 * 
 * 19    5/27/97 4:50p Rjohnson
 * Added white smoke puff as a client effect
 * 
 * 18    5/27/97 7:58a Rlove
 * New thingtypes of GreyStone,BrownStone, and Cloth.
 * 
 * 17    5/23/97 4:17p Rlove
 * Getting rid of Quake sounds
 * 
 * 16    5/22/97 10:28a Rlove
 * Added fire circle fx
 * 
 * 15    5/19/97 12:01p Rlove
 * New sprites for axe
 * 
 * 14    5/19/97 11:29a Rlove
 * 
 * 13    5/19/97 8:58a Rlove
 * Adding sprites and such to the axe.
 * 
 * 12    5/16/97 1:52p Rlove
 * 
 * 11    5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 10    5/15/97 1:33p Rlove
 * 
 * 9     5/13/97 2:26p Rlove
 * 
 * 8     5/12/97 11:06a Rlove
 * 
 * 6     5/12/97 10:31a Rlove
 * 
 * 5     4/24/97 2:21p Mgummelt
 * 
 * 4     11/19/96 11:40a Rlove
 * Particle explosion entity
 * 
 * 3     11/18/96 3:30p Rlove
 * added fx_flash entity
 * 
 * 2     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */
