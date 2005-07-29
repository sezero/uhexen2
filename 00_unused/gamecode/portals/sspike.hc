/*
========================================================================
  
  SPREAD SPIKER (jweier)

========================================================================
*/

float sspike_INCR;

void () sspike_getSpread =
{
	local vector v1, v2;

	//TODO: search the entities for two spreads belonging to this trap

	//TODO: take the two vectors and determine the number of shots to
	//	    cover a spread of NUM_SHOTS_SPREAD shots
			
	//TODO: set sspike_INCR to reflect this increment				
};

void (vector thisShot) sspike_use =
{
	launch_spike (self.origin, thisShot);
};

void () sspike_think =
{
  //TODO: update the vector by an sspike_INCR

  sspike_use(self.angles);
  
  self.nextthink = time + self.wait;
  newmis.velocity = self.movedir * 500;
};
		
void() trap_sspikeshooter =
{
	//???
	//SetMovedir();
	
	//self.use = sspike_use;
	
	self.think = sspike_think;

	precache_sound ("weapons/spike2.wav");
};

