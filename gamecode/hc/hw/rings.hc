/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/rings.hc,v 1.1.1.1 2004-11-29 11:29:12 sezero Exp $
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
		other.rings = other.rings | RING_WATER;
		other.ring_water = 100;
		other.ring_water_time = time + 1;

		if (other.rings_low & RING_WATER)
			other.rings_low (-) RING_WATER;

	}
	else if (self.classname == "Ring_Turning")
	{
		other.rings = other.rings | RING_TURNING;
		other.ring_turning = 100;
		other.ring_turning_time = time + 1;

		other.rings_active (+) RING_TURNING;

		if (other.rings_low & RING_TURNING)
			other.rings_low (-) RING_TURNING;
	}
	else if (self.classname == "Ring_Regeneration")
	{
		other.rings = other.rings | RING_REGENERATION;
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
	precache_model("models/ringwb.mdl");
	Ring_Init("models/ringwb.mdl",STR_RINGWATERBREATHING);
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:11  theoddone33
 * Inital import
 *
 * 
 * 4     3/21/98 8:05p Ssengele
 * 
 * 3     3/19/98 2:00p Rmidthun
 * restrict rings of flight to 5 max
 * 
 * 2     3/16/98 10:01p Ssengele
 * fixed bugs with ringoflight droppin.
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 24    9/11/97 12:02p Mgummelt
 * 
 * 23    9/02/97 2:01a Rlove
 * 
 * 22    9/01/97 9:32p Rlove
 * 
 * 21    8/31/97 8:52a Mgummelt
 * 
 * 20    8/26/97 7:45a Rlove
 * 
 * 19    8/25/97 6:01p Rlove
 * 
 * 18    8/21/97 12:00a Mgummelt
 * 
 * 17    8/20/97 11:56p Mgummelt
 * 
 * 16    8/15/97 11:40a Rlove
 * 
 * 15    8/14/97 8:00a Rlove
 * 
 * 14    8/14/97 6:42a Rlove
 * 
 * 13    7/21/97 3:03p Rlove
 * 
 * 12    7/08/97 7:07a Rlove
 * Changed bounding box on rings
 * 
 * 11    6/25/97 8:28a Rlove
 * Added ring of turning 
 * 
 * 10    6/24/97 5:44p Rlove
 * Rings of Flight and Regeneration are working
 * 
 * 9     6/24/97 3:54p Rlove
 * New ring system
 * 
 * 8     6/13/97 10:11a Rlove
 * Moved all message.hc to strings.hc
 * 
 * 7     6/04/97 8:49a Rlove
 * Added ring of absorption
 * 
 * 6     6/03/97 7:41a Rlove
 * Added ring of absorption model
 * 
 * 5     5/22/97 3:30p Mgummelt
 * 
 * 4     2/12/97 3:59p Rlove
 * Invincibility is done, changed a few things with ring of water
 * breathing and the lava death
 * 
 * 3     1/10/97 1:02p Rlove
 * 
 * 2     12/26/96 10:01a Rlove
 * Ring of Water Breathing is working, now on to the Bra of Miracles
 * 
 */
