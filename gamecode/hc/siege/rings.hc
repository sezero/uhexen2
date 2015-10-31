/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/rings.hc,v 1.2 2007-02-07 17:01:22 sezero Exp $
 */
void player_fly(void);
void Ring_Init(string modelname,float name);

void ring_touch (void)
{
	float		amount;
	entity holdself;

	if ((other.classname != "player") || (other.health <= 0))
		return;

	// Was it thrown out by player just a frame of two ago?????
	if (self.owner == other && self.artifact_ignore_owner_time > time)
		return;
	if (self.artifact_ignore_time > time) 
		return;
	
	// take appropriate action
	if ((self.classname == "Ring_Flight") && (deathmatch))
	{
		if(other.cnt_flight > 4)
		{
			return;
		}
	}

	amount = random();
	if (amount < 0.5)
	{
		sprinti (other, PRINT_MEDIUM, STR_YOUPOSSESS);
		sprinti (other, PRINT_MEDIUM, self.artifact_name);
	}
	else
	{
		sprinti (other, PRINT_MEDIUM, STR_YOUHAVEACQUIRED);
		sprinti (other, PRINT_MEDIUM, self.artifact_name);
	}

	sprint (other,PRINT_MEDIUM, "\n");

	if ((deathmatch||(self.classname == "Ring_Flight"&&!self.owner))&&(self.artifact_respawn))
	{
		self.mdl = self.model;
		self.nextthink = time + 60;
		self.think = SUB_regen;
	}

	sound (other, CHAN_VOICE, "items/ringpkup.wav", 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.solid = SOLID_NOT;
	self.model = string_null;

	// take appropriate action
	if ((self.classname == "Ring_Flight") && (deathmatch))
	{
		other.cnt_flight += 1;
	}
	else if (self.classname == "Ring_Flight") 
	{
		if(other.rings_active&RING_FLIGHT)//add time to current ring
		{
			other.ring_flight = 100;
			other.ring_flight_time = time + 1;
		}
		else
		{
			other.rings(+)RING_FLIGHT;
			other.ring_flight = 100;
			other.ring_flight_time = time + 1;
			holdself = self;
			self = other;
			player_fly();
			self = holdself;
			other.rings_active (+) RING_FLIGHT;
		}
		other.rings_low (-) RING_FLIGHT;
	}
	else if (self.classname == "Ring_WaterBreathing")
	{
		other.rings (+) RING_WATER;
		other.ring_water = 100;
		other.ring_water_time = time + 1;

		if (other.rings_low & RING_WATER)
			other.rings_low (-) RING_WATER;

	}
	else if (self.classname == "Ring_Turning")
	{
		other.rings (+) RING_TURNING;
		other.ring_turning = 100;
		other.ring_turning_time = time + 1;

		other.rings_active (+) RING_TURNING;

		if (other.rings_low & RING_TURNING)
			other.rings_low (-) RING_TURNING;
	}
	else if (self.classname == "Ring_Regeneration")
	{
		other.rings (+) RING_REGENERATION;
		other.ring_regeneration = 100;
		other.rings_active (+) RING_REGENERATION;

		if (other.rings_low & RING_REGENERATION)
			other.rings_low (-) RING_REGENERATION;
	}

	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets

	if(!self.artifact_respawn)
	{
		remove(self);
	}
}

/*QUAKED Ring_WaterBreathing (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
Ring of Water Breathing
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void Ring_WaterBreathing (void)
{
	if(self.origin=='600 -2088 156')
	{
		self.classname=="Climbing Boots";
		art_climb();
	}
	else
	{
		precache_model("models/ringwb.mdl");
		Ring_Init("models/ringwb.mdl",STR_RINGWATERBREATHING);
	}
}

/*QUAKED Ring_Flight (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
Ring of Flight
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void Ring_Flight (void)
{
	precache_model("models/ringft.mdl");
	Ring_Init("models/ringft.mdl",STR_RINGFLIGHT);
}

/*QUAKED Ring_Regeneration (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
Ring of Regeneration
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void Ring_Regeneration (void)
{
	precache_model("models/ringre.mdl");
	Ring_Init("models/ringre.mdl",STR_RINGREGENERATION);
}

/*QUAKED Ring_Turning (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
Ring of Turning
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void Ring_Turning (void)
{
	precache_model("models/ringtn.mdl");
	Ring_Init("models/ringtn.mdl",STR_RINGTURNING);
}

void Ring_Init(string modelname,float name)
{
	setmodel(self, modelname);

	self.netname = "Ring";
	self.artifact_name = name;

	self.artifact_respawn = deathmatch;

	setsize(self,'0 0 0','0 0 0');
	self.hull=HULL_POINT;
	self.touch = ring_touch;

	StartItem();
}

