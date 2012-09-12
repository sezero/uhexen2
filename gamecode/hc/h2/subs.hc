/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/subs.hc,v 1.2 2007-02-07 16:57:11 sezero Exp $
 */

float SPAWNFLAG_ACTIVATED	= 8;


void SUB_Null() {}

void SUB_Remove()
{
	remove(self);
}

/*
void spawntestmarker(vector org, float life, float skincolor)
{
	newmis=spawn_temp();
	newmis.drawflags=MLS_ABSLIGHT;
	newmis.abslight=1;
	newmis.frame=1;
	newmis.skin=skincolor;
	setmodel(newmis,"models/test.mdl");
	setorigin(newmis,org);
	newmis.think=SUB_Remove;
	if(life==-1)
		self.nextthink=-1;
	else
		thinktime newmis : life;
}*/


/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir()
{
	if(self.angles == '0 -1 0')
		self.movedir = '0 0 1';
	else if(self.angles == '0 -2 0')
		self.movedir = '0 0 -1';
	else
	{
		makevectors(self.angles);
		self.movedir = v_forward;
	}

	self.angles = '0 0 0';
}


/*
 * InitTrigger() -- Sets up the entity fields for a trigger.
 *                  If .angles is set, it is used for a one-way touch.
 *                  Use a yaw of 360 if you want a 0-degree one-way touch.
 */

void InitTrigger()
{
	if(self.angles != '0 0 0')
		SetMovedir();
	setmodel(self, self.model);       // set size and link into world
	self.solid		= SOLID_TRIGGER;
	self.movetype	= MOVETYPE_NONE;
	self.modelindex = 0;
	self.model		= "";
	if(self.spawnflags & SPAWNFLAG_ACTIVATED)
		self.inactive=TRUE;
}


/*
 * SUB_CalcMove() -- Calculates self.velocity and self.nextthink to reach
 *                   dest from self.origin traveling at speed.
 */

void(vector tdest, float tspeed, void() func) SUB_CalcMove =
{
	local vector vdestdelta;
	local float  len, traveltime;

	if(!tspeed)
		objerror("No speed is defined!");

	self.think1		= func;
	self.finaldest	= tdest;
	self.think		= SUB_CalcMoveDone;

	if(tdest == self.origin)
	{
		self.velocity = '0 0 0';
		self.nextthink = self.ltime + 0.1;
		return;
	}

	vdestdelta = tdest - self.origin;	// set destdelta to the vector needed to move
	len = vlen(vdestdelta); 			// calculate length of vector
	traveltime = len / tspeed;			// divide by speed to get time to reach dest

	if(traveltime < 0.1)
	{
		self.velocity  = '0 0 0';
		self.nextthink = self.ltime + 0.1;
		return;
	}

// set nextthink to trigger a think when dest is reached
	self.nextthink = self.ltime + traveltime;

// scale the destdelta vector by the time spent traveling to get velocity
	self.velocity = vdestdelta * (1/traveltime);	// hcc won't take vec/float	
};


/*
 * SUB_CalcMoveEnt() -- Does the same as above, but takes a specific entity.
 */
/*
void(entity ent, vector tdest, float tspeed, void() func) SUB_CalcMoveEnt =
{
	local entity stemp;

	stemp	= self;
	self	= ent;
	SUB_CalcMove(tdest, tspeed, func);
	self	= stemp;
};
*/

/*
 * SUB_CalcMoveDone -- Sets origin to the exact destination after moving.
 */

void SUB_CalcMoveDone()
{
	setorigin(self, self.finaldest);
	self.velocity = '0 0 0';
	self.nextthink = -1;
	if(self.think1)
		self.think1();
}


/*
 * SUB_CalcAngleMove() -- Calculates self.avelocity and self.nextthink to
 *                        rotate to destangle from self.angles.
 *                        The caller should make sure self.think is valid.
 */

void(vector destangle, float tspeed, void() func) SUB_CalcAngleMove =
{
	local vector destdelta;
	local float  len, traveltime;

	if(!tspeed)
		objerror("SUB_CalcAngleMove: No speed defined!");

	destdelta = destangle - self.angles;	// set destdelta to the vector needed to move
	len = vlen(destdelta);					// calculate length of vector
	traveltime = len / tspeed;				// divide by speed to get time to reach dest

// set nextthink to trigger a think when dest is reached
	self.nextthink	= self.ltime + traveltime;

// scale the destdelta vector by the time spent traveling to get velocity
	self.avelocity	= destdelta * (1 / traveltime);

	self.think1		= func;
	self.finalangle = destangle;
	self.think		= SUB_CalcAngleMoveDone;
};


/*
 * SUB_CalcAngleMoveEnt() -- Does the same as above, but takes a specific entity.
 */
/*
void(entity ent, vector destangle, float tspeed, void() func) SUB_CalcAngleMoveEnt =
{
	local entity stemp;

	stemp	= self;
	self	= ent;
	SUB_CalcAngleMove(destangle, tspeed, func);
	self	= stemp;
};
*/

/*
 * SUB_CalcAngleMoveDone() -- After rotating, set .angle to exact final angle.
 */

void SUB_CalcAngleMoveDone()
{
	self.angles = self.finalangle;
	self.avelocity = '0 0 0';
	self.nextthink = -1;
	if (self.think1)
		self.think1();
}

/*
 * SUB_CalcMoveAndAngleDone-Set angle and position in final states.
 */

void SUB_CalcMoveAndAngleDone(void)
{
	setorigin(self, self.finaldest);
	self.angles = self.finalangle;
	self.velocity = self.avelocity = '0 0 0';
	self.nextthink = -1;
	if (self.think1)
		self.think1();
}

void()SUB_CalcAngleOnlyDone;

void SUB_CalcMoveOnlyDone(void)
{
	setorigin(self, self.finaldest);
	self.velocity = '0 0 0';
	self.movetime=0;
	if(self.angletime>0)
	{
		self.think=SUB_CalcAngleOnlyDone;
		self.nextthink=self.ltime+self.angletime;
	}
	else
		SUB_CalcMoveAndAngleDone();
}

void SUB_CalcAngleOnlyDone(void)
{
	self.angles = self.finalangle;
	self.avelocity = '0 0 0';
	self.angletime = 0;
	if(self.movetime>0)
	{
		self.think=SUB_CalcMoveOnlyDone;
		self.nextthink=self.ltime+self.movetime;
	}
	else
		SUB_CalcMoveAndAngleDone();
}

/*
====================================================================
SUB_CalcMoveAndAngle()
MG!!!

Calculates self.velocity and self.nextthink to reach dest from
self.origin traveling at speed, does same with angle simultaneously.
====================================================================
*/

void SUB_CalcMoveAndAngle (float synchronize)
{
vector vdestdelta, destdelta;
float  len, alen;
//MOVE
	if(!self.speed)
		objerror("No speed is defined!");

	if(self.finaldest == self.origin)
	{
		self.velocity = '0 0 0';
		self.movetime = 0;
	}
	else
	{
		vdestdelta = self.finaldest - self.origin;	// set destdelta to the vector needed to move
		len = vlen(vdestdelta); 			// calculate length of vector
		self.movetime = len / self.speed;			// divide by speed to get time to reach dest

		if(self.movetime < 0.1)
		{
			self.velocity  = '0 0 0';
			self.movetime = 0.1;
		}
		// scale the destdelta vector by the time spent traveling to get velocity
		self.velocity = vdestdelta * (1/self.movetime);	// hcc won't take vec/float	
	}


//ANGLE

	if(self.angles==self.finalangle)
	{	
			self.avelocity = '0 0 0';
			self.angletime = 0;
	}
	else
	{
		destdelta = self.finalangle - self.angles;	// set destdelta to the vector needed to move
		alen = vlen(destdelta);					// calculate length of vector
		if(!synchronize)
		{
			if(!self.anglespeed)
				objerror("SUB_CalcAngleMove: No speed defined!");
		}
		else
			self.anglespeed=self.speed/len*alen;				//
		self.angletime = alen / self.anglespeed;				// divide by speed to get time to reach dest
// scale the destdelta vector by the time spent traveling to get velocity
		self.avelocity	= destdelta * (1 / self.angletime);
	}
//	if(synchronize&&self.angletime!=self.movetime)
//		dprint("Whoops!\n");


// set nextthink to trigger a think when dest is reached

	if(self.movetime<=0)
		self.movetime=self.angletime;
	if(self.angletime<=0)
		self.angletime=self.movetime;

	if(self.movetime>self.angletime)
	{
		self.movetime-=self.angletime;
		self.think = SUB_CalcAngleOnlyDone;
		self.nextthink=self.ltime+self.angletime;
	}
	else if(self.movetime<self.angletime)
	{
		self.angletime-=self.movetime;
		self.think = SUB_CalcMoveOnlyDone;
		self.nextthink=self.ltime+self.movetime;
	}
	else
	{
		self.think = SUB_CalcMoveAndAngleDone;
		self.nextthink=self.ltime+self.movetime;
	}
}


/*
 * SUB_CalcMoveAndAngleInit-Set movement values and call main function
 */
void(vector tdest, float tspeed, vector destangle, float aspeed,void() func,float synchronize) SUB_CalcMoveAndAngleInit =
{
	self.finaldest = tdest;
	self.speed = tspeed;
	self.finalangle = destangle;
	self.anglespeed = aspeed;
	self.think1 = func;
	SUB_CalcMoveAndAngle(synchronize);
};

/*
 * DelayThink()
 */

void DelayThink()
{
	activator = self.enemy;
	SUB_UseTargets();
	remove(self);
}


/*
 * SUB_UseTargets() --
 *
 * The global "activator" should be set to the entity that initiated the firing.
 *
 * If self.delay is set, a DelayedUse entity will be created that will actually
 * do the SUB_UseTargets after that many seconds have passed.
 *
 * Centerprints any self.message to the activator.
 *
 * Removes all entities with a targetname that match self.killtarget,
 * and removes them, so some events can remove other triggers.
 *
 * Search for (string)targetname in all entities that
 * match (string)self.target and call their .use function
 */

void SUB_UseTargets()
{
entity t, stemp, otemp, act;
string s;

//
// check for a delay
//
	if(self.delay)
	{
		// create a temp object to fire at a later time
		t = spawn();
		t.classname = "DelayedUse";
		thinktime t : self.delay;
		t.think = DelayThink;
		t.enemy = activator;
		t.message = self.message;
		t.killtarget = self.killtarget;
		t.target = self.target;
		return;
	}

//
// print the message
//
	if(activator.classname == "player" && self.message != 0)
	{
		s = getstring(self.message);
		centerprint (activator, s);
		if(!self.noise)
			sound (activator, CHAN_VOICE, "misc/comm.wav", 1, ATTN_NORM);
	}

//
// kill the killtargets
//
	if(self.killtarget)
	{
		t = world;
		do
		{
			t = find(t, targetname, self.killtarget);
			if(t!=world)
				remove(t);
		}
		while(t!=world);
	}

//
// fire targets
//
	self.style=0;
	if (self.target)
	{
		act = activator;
		t = world;
		loop /*do*/ {
			t = find (t, targetname, self.target);
			if (!t)
			{
				if(self.nexttarget!=""&&self.target!=self.nexttarget)
					self.target=self.nexttarget;
				return;
			}
			if(t.style>=32&&t.style!=self.style)
			{
				self.style=t.style;
				if(self.classname=="breakable_brush")
					lightstylestatic(self.style,0);
				else
					lightstyle_change(t);
			}
			stemp = self;
			otemp = other;
			self = t;
			other = stemp;
			if (other.classname == "trigger_activate")
				if (!self.inactive) 
					self.inactive = TRUE;
				else
					self.inactive = FALSE;
			else if (other.classname == "trigger_deactivate")
				self.inactive = TRUE;
			else if (other.classname == "trigger_combination_assign"&& self.classname=="trigger_counter")
				self.mangle = other.mangle;
			else if (other.classname == "trigger_counter_reset"&& self.classname=="trigger_counter")
			{
				self.cnt = 1;
				self.count = self.frags;
				self.items = 0;
			}
			else if (self.use != SUB_Null&&!self.inactive)
			{	//Else here because above trigger types should not use it's target
				if (self.use)
					self.use ();
			}
			self = stemp;
			other = otemp;
			activator = act;
		} /*while (1);*/
	}
}


/*
 * SUB_AttackFinished() -- Gets ready to finish the attack.  In Nightmare
 *                         mode, all attack_finished times become 0, and
 *                         some monsters refire twice automatically.
 */

void SUB_AttackFinished(float normal)
{
	self.cnt = 0;		// refire count for nightmare
	if(skill != 3)
		self.attack_finished = time + normal;
}


/*
 * SUB_CheckRefire() -- Decides whether or not for monster to refire.
 */
/*
float visible(entity targ);

void (void() thinkst) SUB_CheckRefire =
{
	if(skill != 3)
		return;
	if(self.cnt == 1)
		return;
	if(!visible (self.enemy))
		return;
	self.cnt = 1;
	self.think = thinkst;
};
*/

