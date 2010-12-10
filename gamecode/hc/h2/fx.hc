/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/fx.hc,v 1.2 2007-02-07 16:57:04 sezero Exp $
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


/*
================
SpawnPuff
================
*/
void  SpawnPuff (vector org, vector vel, float damage,entity victim)
{
	float part_color;
	float rad;

	if (victim.thingtype==THINGTYPE_FLESH && victim.classname!="mummy" && victim.netname != "spider")
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
/*
void () friction_change_touch =
{
	if (other == self.owner)
		return;

	if (other.classname == "player")
		other.friction=self.friction;

};
*/
/*QUAK-ED fx_friction_change (0 1 1) ?

Set the friction within this area.

-------------------------FIELDS-------------------------
'friction' :  this is how quickly the player will slow down after he ceases indicating movement (lets go of arrow keys).

             1       : normal friction
             >0 & <1 : slippery
             >1      : high friction
--------------------------------------------------------
*/
/*
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
*/

void() explosion_done =
{
	self.effects=EF_DIMLIGHT;
};

void() explosion_use =
{
/*
	if (self.spawnflags & FLASH)
	{
		self.effects=EF_BRIGHTLIGHT;
		self.think=p_explosion_done;
		self.nextthink= time + 1;
	}
*/
	sound (self, CHAN_BODY, self.noise1, 1, ATTN_NORM);

	particleexplosion(self.origin,self.color,self.exploderadius,self.counter);

};

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

