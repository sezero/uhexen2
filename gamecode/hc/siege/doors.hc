/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/doors.hc,v 1.3 2007-02-07 17:00:37 sezero Exp $
 */

float DOOR_START_OPEN	= 1;
float REVERSE			= 2;
float DOOR_DONT_LINK	= 4;
float DOOR_TOGGLE		= 8;
float DOOR_SLIDE		= 16;
float DOOR_NORMAL		= 32;
float DOOR_REMOVE_PP	= 64;
float DOOR_NO_PP		= 128;

/*
Doors are similar to buttons, but can spawn a fat trigger field around them
to open without a touch, and they link together to form simultanious
double/quad doors.
 
Door.owner is the master door.  If there is only one door, it points to itself.
If multiple doors, all will point to a single one.

Door.enemy chains from the master door through all doors linked in the chain.
*/

void door_hit_bottom();
void door_hit_top();

/*
===========================================================================
door_slide
===========================================================================
*/

void door_slide_next()
{
	local vector vdestdelta, odelta;
	local float  len, tspeed;//, len2;

	tspeed = self.speed;

	if(!tspeed)
		objerror("No speed is defined!");

	// Make sure we're not already at the destination
	if(self.finaldest == self.origin)
		{
		self.velocity = '0 0 0';
		if(self.state == STATE_DOWN)
			self.think = door_hit_bottom;
	    else if(self.state == STATE_UP)
			self.think = door_hit_top;
    	self.nextthink = self.ltime + 0.1;
		return;
		}

	vdestdelta	= self.finaldest - self.origin;	// Set destdelta to the vector needed to move
	odelta		= self.origin - self.pos1;
	len			= vlen(odelta);					// Get the length of the vector

	// If the length is this small, just stop
	if(vlen(vdestdelta) < 0.1) 
	{
		if(self.state == STATE_DOWN)
			door_hit_bottom();
		else if(self.state == STATE_UP)
			door_hit_top();
		else
			dprint("Bad door state!\n");
		return;
	}

	self.nextthink = self.ltime + 0.1;
	self.think = door_slide_next;
	
	tspeed = ((self.speed - (len / 10)) / 20);

	if(tspeed < 2)
	{
		if(self.state == STATE_DOWN)
			SUB_CalcMove(self.finaldest, self.speed, door_hit_bottom);
		else if(self.state == STATE_UP)
			SUB_CalcMove(self.finaldest, self.speed, door_hit_top);
		else
			dprint("Bad door state!\n");
		return;
	}

	self.velocity = vdestdelta * tspeed;
}


void door_slide(vector tdest)
{
	self.finaldest = tdest;
	door_slide_next();
}


/*
===========================================================================
door_crash
===========================================================================
*/

void door_crash_next()
{
	local vector vdestdelta, nextvect, testvect;
	local vector tdest;
	local float  len, nextlen, testlen;
	local float  tspeed;

	tdest  = self.finaldest;
	tspeed = self.speed;

	if(!tspeed)
		objerror("No speed is defined!");

	if(tdest == self.origin)
	{
		self.velocity = '0 0 0';	
		if(self.state == STATE_DOWN)
			self.think = door_hit_bottom;
		else if(self.state == STATE_UP)
			self.think = door_hit_top;
		else
			dprint("Bad door state!\n");
    	self.nextthink = self.ltime + 0.1;
		return;
	}

	// set destdelta to the vector needed to move
	vdestdelta	= self.finaldest - self.origin;
	nextvect	= self.pos2 - self.origin;
	testvect	= self.pos2 - self.finaldest;

   // calculate length of vector
	len			= vlen (vdestdelta);
	nextlen 		= vlen(nextvect) + 1;
	testlen		= vlen(testvect);

	if(len < 0.1 || nextlen > testlen)
	{
		door_hit_bottom;
		return;
	}
	else 
	{
		self.velocity = vdestdelta * (nextlen / len) * 4;
		nextvect = self.origin + self.velocity;
		nextlen = vlen(nextvect);
		if(nextlen >= testlen * 2)
		{
			SUB_CalcMove(self.finaldest, self.speed * (len / (len / 3)), door_hit_bottom);
			return;
		}
	}

	self.nextthink = self.ltime + 0.1;
	self.think = door_crash_next;
}


void door_crash(vector tdest)
{
	self.finaldest = tdest;
	door_crash_next();
}

/*
=============================================================================
THINK FUNCTIONS
=============================================================================
*/

void door_go_down();
void door_go_up();

void door_damage()
{
	T_Damage (other, self, self, self.dmg);
}

void door_blocked()
{
//FIXME: Rotating doors sem to think they're being blocked
//	even if they're rotating down and the object is above them
	if(self.wait>-2&&self.strength<=0)
		if(self.dmg==666)
		{
			if(other.classname=="player"&&other.flags2&FL_ALIVE)
			{
				other.decap=TRUE;
				T_Damage (other, self, self, other.health+300);
			}
			else
				T_Damage (other, self, self, other.health+50);
		}
		else
		{
			T_Damage (other, self, self, self.dmg);
		}
			
	
//Rotating doors rotating around a x or z axis push you up and in the direction they're turning
/*	if(self.strength==2)
	{
		other.flags(-)FL_ONGROUND;
		other.velocity=normalize(self.origin-(other.absmin+other.absmax)*0.5)*100;
	}
	else*/ if(other.flags&FL_ONGROUND&&(self.movedir_x||self.movedir_z)&&self.strength==1)//&&other.origin_z>self.origin_z
	{
		other.flags(-)FL_ONGROUND;
		other.velocity_z+=self.speed*2;
		other.velocity_x-=self.speed*self.movedir_x;
		other.velocity_y-=self.speed*self.movedir_z;
	}
	else
	{
		other.flags(-)FL_ONGROUND;
		other.velocity_z+=10;
	}
	
// if a door has a negative wait, it would never come back if blocked,
// so let it just squash the object to death real fast
	if (other.health>0)
		if (self.wait >= 0)
		{
			if (self.state == STATE_DOWN)
				door_go_up ();
			else
				door_go_down ();
		}
}

void door_blocked_mp()
{
float do_dmg;

/*
		dprint("Blocked Door: \n");
		dprint(other.classname);
		dprint("\n");
		dprintv("Velocity: %s\n",self.velocity);
		dprintv("Avelocity: %s\n",self.avelocity);
*/
//FIXME: Rotating doors seem to think they're being blocked
//	even if they're rotating down and the object is above them
//	if(self.classname=="door_rotating")
//		self.nextthink+=HX_FRAME_TIME;//self.wait?

/*	dprint("door blocked\n");
	dprintf("dmg = %s\n",self.dmg);
	dprintf("strength = %s\n",self.strength);
	dprintf("wait = %s\n",self.wait);
	dprint("other = ");
	dprint(other.classname);
	dprint("\n");*/
	if(self.dmg==-1)
	{
		if(other.classname=="player" && other.flags2&FL_ALIVE)
		{
			if (self.wait >= 0)
			{
				if (self.state == STATE_DOWN)
					door_go_up ();
				else
					door_go_down ();
			}
			return;
		}
		else
			do_dmg=2;
	}
	else
		do_dmg=self.dmg;

//	dprintf("Door dmg = %s\n",do_dmg);
	if(self.wait>-2)//&&self.strength<=0)
	{
		if(do_dmg==666)
		{
			if(other.classname=="player"&&other.flags2&FL_ALIVE)
			{
				other.decap=TRUE;
				T_Damage (other, self, self, other.health+300);
			}
			else
				T_Damage (other, self, self, other.health+50);
		}
		else
		{
//			dprintf("crushing- %s\n",do_dmg);
			T_Damage (other, self, self, do_dmg);//FIXME: Rotating doors get stuck open and never try to return
		}
	}
//	else
//		dprint("Door wait <= -2\n");

//Rotating doors rotating around a x or z axis push you up and in the direction they're turning
/*	if(self.strength==2)
	{
		other.flags(-)FL_ONGROUND;
		other.velocity=normalize(self.origin-(other.absmin+other.absmax)*0.5)*100;
	}
	else*/

/*NOT in MP maps?
	if(other.flags&FL_ONGROUND&&(self.movedir_x||self.movedir_z)&&self.strength==1)//&&other.origin_z>self.origin_z
	{//This is not neccessary anymore
		other.flags(-)FL_ONGROUND;
		other.velocity_z+=self.speed*2;
		other.velocity_x-=self.speed*self.movedir_x;
		other.velocity_y-=self.speed*self.movedir_z;
	}
	else
	{
		other.flags(-)FL_ONGROUND;
		other.velocity_z+=10;
	}
*/	
// if a door has a negative wait, it would never come back if blocked,
// so let it just squash the object to death real fast
	if (other.health>0)
		if (self.wait >= 0)//&&self.wait!=1.5)
		{
			if (self.state == STATE_DOWN)
			{
	//			dprint("Going up...\n");
				door_go_up ();
			}
			else
			{
	//			dprint("Going down...\n");
				door_go_down ();
			}
		}
}


void door_hit_top()
{
	self.velocity = '0 0 0';
	sound (self,CHAN_UPDATE+PHS_OVERRIDE_R, self.noise1, 1, ATTN_NORM);
	self.effects(-)EF_UPDATESOUND;
	self.state = STATE_TOP;

	if (self.spawnflags & DOOR_TOGGLE)
			return;		// don't come down automatically

	if(self.wait== -2)
		self.th_die();
	else if(self.wait== -1)
		self.nextthink = -1;
	else
	{
		self.think = door_go_down;
		self.nextthink = self.ltime + self.wait;
	}
}

void door_hit_bottom()
{
	self.velocity = '0 0 0';
	sound (self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise1, 1, ATTN_NORM);
	self.effects(-)EF_UPDATESOUND;
	self.state = STATE_BOTTOM;
}

void door_go_down()
{
string hold_target;
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	if(!self.thingtype && self.max_health)
	{
		self.takedamage = DAMAGE_YES;
		self.health = self.max_health;
	}

	self.state = STATE_DOWN;

	if(self.classname == "door")
	{
//		dprintv("rotation: %s\n",self.v_angle);
		if(self.spawnflags & DOOR_SLIDE)
			door_slide(self.pos1);
		else if(self.spawnflags & DOOR_NORMAL)
			if(self.v_angle!='0 0 0')
				if(self.speed)
					if(self.anglespeed)
						SUB_CalcMoveAndAngleInit (self.pos1, self.speed, self.o_angle, self.anglespeed, door_hit_bottom,FALSE);
					else
						SUB_CalcMoveAndAngleInit (self.pos1, self.speed, self.o_angle, self.anglespeed, door_hit_bottom,TRUE);
				else
					SUB_CalcAngleMove(self.o_angle, self.anglespeed, door_hit_bottom);
			else
				SUB_CalcMove(self.pos1, self.speed, door_hit_bottom);
		else
			door_crash(self.pos1); 
	}
	else if (self.classname == "door_rotating")
		SUB_CalcAngleMove(self.pos1, self.speed, door_hit_bottom);

	if(self.close_target!="")
	{//Use second target when closing
		hold_target=self.target;
		self.target=self.close_target;
		SUB_UseTargets();
		self.target=hold_target;
	}
}


void new_movedir (vector movin,float dir)
{
	self.movedir = movin;

	// check for clockwise rotation
	if (dir<0)
		self.movedir = self.movedir * -1;

	self.pos1 = self.angles;
	self.pos2 = self.angles + self.movedir * self.dflags;
}

void door_go_up()
{
	if(self.state == STATE_UP)		/* Already going up */
	{
//		dprint("UP: Tried to go up while already going up\n");
		return;
	}

	if(self.state == STATE_TOP) 	/* Reset top wait time */
	{
		self.nextthink = self.ltime + self.wait;
//		dprint("TOP: Tried to go up while already at top\n");
		return;
	}

	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	self.state = STATE_UP;

	if(self.classname == "door")
	{
		if(self.spawnflags & DOOR_NORMAL)
		{
			if(self.v_angle!='0 0 0')
			{
				if(self.speed)
				{
					if(self.anglespeed)
						SUB_CalcMoveAndAngleInit (self.pos2, self.speed, self.v_angle, self.anglespeed, door_hit_top,FALSE);
					else
						SUB_CalcMoveAndAngleInit (self.pos2, self.speed, self.v_angle, self.anglespeed, door_hit_top,TRUE);
				}
				else
					SUB_CalcAngleMove(self.v_angle, self.anglespeed, door_hit_top);
			}
			else
				SUB_CalcMove(self.pos2, self.speed, door_hit_top);
		}
		else
			door_slide(self.pos2);
	}
	else if(self.classname == "door_rotating") 
		SUB_CalcAngleMove(self.pos2, self.speed, door_hit_top);

	SUB_UseTargets();
}


/*
=============================================================================

ACTIVATION FUNCTIONS

=============================================================================
*/

void door_fire()
{
entity 	oself;
entity	starte;

//	if(self.wait<=-1 || self.wait==1.5)//not supposed to return
//			if(self.velocity!='0 0 0'||self.avelocity!='0 0 0')//Moving
//				return;

	if (self.owner != self)
		objerror ("door_fire: self.owner != self");

	self.no_puzzle_msg = 0;

// play use key sound

	self.message = 0;		// no more message
	oself = self;

	if (self.spawnflags & DOOR_TOGGLE)
	{
		if (self.state == STATE_UP || self.state == STATE_TOP)
		{
			starte = self;
			do
			{
				door_go_down ();
				self = self.enemy;
			} while ( (self != starte) && (self != world) );
			self = oself;
			return;
		}
	}
	
// trigger all paired doors
	starte = self;
	do
	{
		door_go_up ();
		self = self.enemy;
	} while ( (self != starte) && (self != world) );
	self = oself;
}


/*
 * door_use() -- Called whenever a door is opened.
 */

void door_use()
{
entity oself;


	/*
	dprint("Door Used by: ");
	dprint(other.classname);
	dprint("\n");
	dprint("Door's Activator: ");
	dprint(activator.classname);
	dprint("\n");
*/

	if(self.inactive)
	{
	//	dprint("Door not active\n");
		return;
	}

	self.message = 0;			// door messages are for touch only
	self.owner.message = 0;
	self.enemy.message = 0;
	oself = self;
	self = self.owner;
	door_fire ();
	self = oself;
}


// defined in triggers.hc
float check_puzzle_pieces(entity client, float remove_pieces, float inverse);

/*
 * door_trigger_touch() -- Called when someone touches a door.
 */

void door_trigger_touch()
{
	entity door;
	string temp;
	float removepp, inversepp;

//	if(!other.flags2&FL_ALIVE)
//		return;

	if(!other.flags&FL_CLIENT&&!other.flags&FL_MONSTER)
		return;

	if(other.flags&FL_MONSTER&&world.spawnflags&MISSIONPACK)
		return;

	if(time < self.attack_finished)
		return;

	door = self;
	self = self.owner;
	if(!deathmatch)
	{
		removepp = (self.spawnflags & DOOR_REMOVE_PP);
		inversepp = (self.spawnflags & DOOR_NO_PP);

		if (!check_puzzle_pieces(other,removepp,inversepp))
		{
			if (self.no_puzzle_msg && !deathmatch) 
			{
				temp = getstring(self.no_puzzle_msg);
				centerprint (other, temp);
				door.attack_finished = time + 2;
			}
			return;
		}
	}
	self.attack_finished = time + 1;
	activator	= other;
	door_use();
}


/*
 * door_killed() -- Used to open doors that open when shot.
 */

void door_killed()
{
	local entity oself;
	
	oself			= self;
	self			= self.owner;
	self.health		= self.max_health;
	self.takedamage = DAMAGE_NO;		// wil be reset upon return
	door_use();
	self			= oself;
}


/*
 * door_touch() -- Prints messages and opens key doors.
 */

void door_touch()
{
	string temp;
	float removepp, inversepp;

//	dprint("Door hit!\n");
//	if(!other.flags2&FL_ALIVE)
//		return;

	if(self.siege_team)
		if(other.siege_team!=self.siege_team)
			return;

	if(!other.flags&FL_CLIENT&&!other.flags&FL_MONSTER)
		return;

	if(other.flags&FL_MONSTER&&world.spawnflags&MISSIONPACK)
		return;

	if(self.dmg==666&&(self.velocity!='0 0 0'||self.avelocity!='0 0 0'))
	{
		if(other.classname=="player"&&other.flags2&FL_ALIVE)
		{
			other.decap=TRUE;
			T_Damage (other, self, self, other.health+300);
		}
		else
			T_Damage (other, self, self, other.health+50);
	}

	if(self.owner.attack_finished > time)
		return;

	if (self.owner)
		self.owner.attack_finished = time + 2;

	if(self.owner.message != 0 && !deathmatch && self.owner != world)
	{
		temp = getstring(self.owner.message);
		centerprint (other, temp);
		sound (other, CHAN_UPDATE+PHS_OVERRIDE_R, "misc/comm.wav", 1, ATTN_NORM);
	}

// key door stuff

	if (!self.puzzle_piece_1 && !self.puzzle_piece_2 && !self.puzzle_piece_3 && !self.puzzle_piece_4)
		return;

// FIXME: blink key on player's status bar
	
	removepp = (self.spawnflags & DOOR_REMOVE_PP);
	inversepp = (self.spawnflags & DOOR_NO_PP);

	if (!check_puzzle_pieces(other,removepp,inversepp))
	{
		if (self.no_puzzle_msg && !deathmatch)
		{
			temp = getstring(self.no_puzzle_msg);
			centerprint (other, temp);
		}
		return;
	}

	if(self.puzzle_piece_1 == g_keyname)
		other.experience += 500;//opened throne door!

	self.touch = SUB_Null;
	if (self.enemy) 
		self.enemy.touch = SUB_Null;	// get paired door
	door_use ();
}



/*
=============================================================================

SPAWNING FUNCTIONS

=============================================================================
*/


entity spawn_field(vector fmins, vector fmaxs, entity door)
{//FIXME: THIS ENTITY NEEDS TO REMOVE ITSELF IF IT'S OWNER IS
	//REMOVED!
entity	trigger;
vector	t1, t2;
	
	trigger = spawn();
	trigger.movetype = MOVETYPE_NONE;
	trigger.solid = SOLID_TRIGGER;
	trigger.owner = door;
	trigger.touch = door_trigger_touch;

	t1 = fmins;
	t2 = fmaxs;
//	if (door.classname == "door") 
//	{
//		if(self.v_angle!='0 0 0')
//		{
		  t1 += door.origin;
		  t2 += door.origin;
		  setsize (trigger, t1 - '60 60 8', t2 + '60 60 8');
/*		}
		else
			setsize (trigger, t1 - '60 60 8', t2 + '60 60 8');
	}
   else if (door.classname == "door_rotating")
	{
	  t1 += door.origin;
	  t2 += door.origin;
	  setsize (trigger, t1 - '60 60 8', t2 + '60 60 8');
	}
*/	return (trigger);
}

float EntitiesTouching(entity e1, entity e2)
{
	local vector e1max, e1min, e2max, e2min;

	//Rotating door's mins and maxs aren't based on their world positions,
	//so the origin needs to be applied to make sure they are checking their
	//real positions

	if (e1.classname == "door_rotating"||(e1.classname=="door"&&e1.v_angle!='0 0 0'))
	{
		e1max = e1.maxs + e1.origin;
		e1min = e1.mins + e1.origin;
		e2max = e2.maxs + e2.origin;
		e2min = e2.mins + e2.origin;
	}
	else
	{
		e1max = e1.maxs;
		e1min = e1.mins;
		e2max = e2.maxs;
		e2min = e2.mins;
	}

	if (e1min_x > e2max_x)
		return FALSE;
	if (e1min_y > e2max_y)
		return FALSE;
	if (e1min_z > e2max_z)
		return FALSE;
	if (e1max_x < e2min_x)
		return FALSE;
	if (e1max_y < e2min_y)
		return FALSE;
	if (e1max_z < e2min_z)
		return FALSE;
	return TRUE;
}


/*
 * LinkDoors()
 */

void LinkDoors()
{
entity	t, starte;
vector	cmins, cmaxs;

	if (self.enemy)
		return;		// already linked by another door

	if (self.spawnflags & 4)
	{
		self.owner = self.enemy = self;
		return;		// don't want to link this door
	}

   cmins = self.mins;
   cmaxs = self.maxs;

	starte = self;
	t = self;
	
	loop /*do*/ {
		self.owner = starte;			// master door

		if (!self.thingtype && self.health)
			starte.health = self.health;
		if (self.targetname)
			starte.targetname = self.targetname;
		if (self.message != 0)
			starte.message = self.message;

		t = find (t, classname, self.classname);	
		if (!t)
		{
			self.enemy = starte;		// make the chain a loop

		// shootable, fired, or key doors just needed the owner/enemy links,
		// they don't spawn a field

			self = self.owner;

			if (!self.thingtype && self.health)
				return;
			if (self.targetname)
				return;
			if (self.puzzle_piece_1 != string_null || 
				self.puzzle_piece_2 != string_null || 
				self.puzzle_piece_3 != string_null || 
				self.puzzle_piece_4 != string_null)
				return;

			self.owner.trigger_field = spawn_field(cmins, cmaxs, self.owner);

			return;
		}

		if (EntitiesTouching(self,t))
		{
			if (t.enemy)
				objerror ("cross connected doors");
			
			self.enemy = t;
			self = t;

			if (t.mins_x < cmins_x)
				cmins_x = t.mins_x;
			if (t.mins_y < cmins_y)
				cmins_y = t.mins_y;
			if (t.mins_z < cmins_z)
				cmins_z = t.mins_z;
			if (t.maxs_x > cmaxs_x)
				cmaxs_x = t.maxs_x;
			if (t.maxs_y > cmaxs_y)
				cmaxs_y = t.maxs_y;
			if (t.maxs_z > cmaxs_z)
				cmaxs_z = t.maxs_z;
		}
	} /*while (1);*/
}

void door_sounds(void)
{
	self.noise3 = "doors/baddoor.wav";

	if (self.soundtype == 0)			// No sound
	{
		self.noise1 = "misc/null.wav";
		self.noise2 = "misc/null.wav";
		self.noise4 = "misc/null.wav";
	}
	else if (self.soundtype == 1)		// Big Metal door, swinging
	{
		precache_sound ("doors/gatestop.wav");
		precache_sound ("doors/gateswng.wav");
		precache_sound ("doors/gatestrt.wav");

		self.noise1 = "doors/gatestop.wav";
		self.noise2 = "doors/gateswng.wav";
		self.noise4 = "doors/gatestrt.wav";
	}
	else if (self.soundtype == 2)		// Big Stone Door, sliding
	{
		precache_sound ("doors/doorstop.wav");
		precache_sound ("doors/stonslid.wav");
		precache_sound ("doors/dorstart.wav");

		self.noise1 = "doors/doorstop.wav";
		self.noise2 = "doors/stonslid.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 3)		// Big Wood Door, Swinging
	{
		precache_sound ("doors/swngstop.wav");
		precache_sound ("doors/wdswngbg.wav");
		precache_sound ("doors/dorstart.wav");

		self.noise1 = "doors/swngstop.wav";
		self.noise2 = "doors/wdswngbg.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 4)		// Normal Wood Door, Swinging
	{
		precache_sound ("doors/swngstop.wav");
		precache_sound ("doors/wdswngsm.wav");
		precache_sound ("doors/dorstart.wav");

		self.noise1 = "doors/swngstop.wav";
		self.noise2 = "doors/wdswngsm.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 5)		// Big Wood Door, Sliding
	{
		precache_sound ("doors/swngstop.wav");
		precache_sound ("doors/woodslid.wav");
		precache_sound ("doors/dorstart.wav");

		self.noise1 = "doors/swngstop.wav";
		self.noise2 = "doors/woodslid.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 6)		// Drawbridge, Falling and crushing innocent peasants who are 
                                        // basically slave labor, who toil away their lives so that the rich upperclass
                                        // can keep busy inbreeding with each other. Damn the aristocracy! Freedom to
                                        // the common man! Burn the castle! Death to the tyrants! 
	{
		precache_sound ("doors/doorstop.wav");
		precache_sound ("doors/drawmove.wav");
		precache_sound ("doors/drawstrt.wav");

		self.noise1 = "doors/doorstop.wav";
		self.noise2 = "doors/drawmove.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 7)		// Rotating Walkway
	{
		precache_sound ("doors/doorstop.wav");
		precache_sound ("doors/stonslid.wav");
		precache_sound ("doors/dorstart.wav");

		self.noise1 = "doors/doorstop.wav";
		self.noise2 = "doors/stonslid.wav";
		self.noise4 = "doors/dorstart.wav";
	}
	else if (self.soundtype == 8)		// Big Metal door, sliding
	{
		precache_sound ("doors/mtlstop.wav");
		precache_sound ("doors/mtlslide.wav");
		precache_sound ("doors/mtlstart.wav");

		self.noise1 = "doors/mtlstop.wav";
		self.noise2 = "doors/mtlslide.wav";
		self.noise4 = "doors/mtlstart.wav";
	}
	else if (self.soundtype == 9)		// Pendulum
	{
		precache_sound2 ("doors/penstop.wav");
		precache_sound2 ("doors/penswing.wav");
		precache_sound2 ("doors/penstart.wav");

		self.noise1 = "doors/penstop.wav";
		self.noise2 = "doors/penswing.wav";
		self.noise4 = "doors/penstart.wav";
	}
}


void door_rotate_incr_done()
{
//	if(self.strength==2)
//		cvar_set ("sv_gravity", "800");
}

void() door_rotate_incr =
{
vector newvect;
	if(self.strength==2)
		cvar_set ("sv_gravity", "100");

	self.dflags=self.flags;
	self.flags=0;

	if(self.v_angle!='0 0 0')
	{
		if(random()>0.5&&self.v_angle_x!=0)
		{
			self.cnt=self.dflags=self.v_angle_x;
			new_movedir('1 0 0',self.cnt);
		}
		else if(random()>0.5&&self.v_angle_y!=0)
		{
			self.cnt=self.dflags=self.v_angle_y;
			new_movedir('0 1 0',self.cnt);
		}
		else if(self.v_angle_z!=0)
		{
			self.cnt=self.dflags=self.v_angle_z;
			new_movedir('0 0 1',self.cnt);
		}
	}
	newvect = self.movedir * self.cnt;

	SUB_CalcAngleMove(self.angles + newvect, self.speed, door_rotate_incr_done);
};

/*QUAKED func_door (0 .5 .8) ? START_OPEN REVERSE DOOR_DONT_LINK TOGGLE SLIDE NORMAL_MOVE remove_pp no_pp
NOTE: Doors can now be activated and deactivated with the appropriate triggers.
"inactive" set to 1 to start the door deactivated.

if two doors touch, they are assumed to be connected and operate as a unit.

-----------------------FIELDS-------------------------
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).

Key doors are always wait -1.

"message" is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"	determines the opening direction
"level" how far (in map units) to move in the specified angle- overrides default movement that is size of door
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health" if set, door must be shot open
"speed"	movement speed (100 default), -1 will not move, just rotate
"wait" wait before returning (3 default, -1 = never return)
"lip" lip remaining at end of move (8 default)
"dmg" damage to inflict when blocked (2 default)  If you make it 
	666, it will gib anything it touches, and behead players.
	-1 it will hurt other things, but not players
"close_target" secondary target to fire when door closes
"ondeath_target" - only for breakable doors- will activate this target only when it dies

ROTATING DOORS: MUST HAVE AN ORIGIN BRUSH
"v_angle" Angle to turn, in: pitch yaw roll, '0 0 0' will not rotate, just move (default = '0 0 0')
"anglespeed" how quickly to turn in that direction.  no anglespeed will force it to choose one that will synch the completion of the rotation iwth the completion of the move.  (default = 0)
"strength" When set to 1, it will throw something if it gets in the way

"soundtype"
0) no sound
1) Big metal door, swinging
2) Big stone door, sliding
3) Big wood door, swinging
4) Normal wood door, swinging
5) Big wood door, sliding
6) Drawbridge
7) Rotating walkway
8) Big metal door, sliding
9) Pendulum swinging

Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
--------------------------------------------------------

*/
void func_door()
{
float movedist, num_axes;
	door_sounds();

	SetMovedir ();
	// check for clockwise rotation
	if (self.spawnflags & REVERSE)
	{
		self.movedir = self.movedir * -1;
		self.v_angle = self.v_angle * -1;
	}

	self.max_health = self.health;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.classname = self.netname = "door";

	self.use = door_use;

	if (self.abslight)
		self.drawflags (+) MLS_ABSLIGHT;
	
	if (self.speed==-1)
		self.speed=0;
	else if(!self.speed)
		self.speed = 100;

	if (!self.wait)
		self.wait = 3;
	if (!self.lip)
		self.lip = 8;
	if(world.spawnflags&MISSIONPACK)
		self.blocked = door_blocked_mp;
	else
		self.blocked = door_blocked;
	if (!self.dmg)
//		self.dmg = -1;
		self.dmg = 2;

	self.pos1 = self.origin;
//	dprintf("Worldspawn.spawnflags = %s\n",world.spawnflags);
	if(world.spawnflags&MISSIONPACK)
	{
//		dprint("Using new door code\n");
		if(self.level)
			movedist = self.level - self.lip;
		else
		{
			//was: movedist=fabs(self.movedir*self.size) - self.lip;
//			dprintv("Door movedir = %s\n",self.movedir);
//			dprintv("Door size = %s\n",self.size);
			num_axes=0;
			movedist=0;
			if(fabs(self.movedir_x)>0.001)
			{
//				dprint("X axis\n");
				movedist+=fabs(self.movedir_x*self.size_x);
				num_axes+=1;
			}
			if(fabs(self.movedir_y)>0.001)
			{
//				dprint("Y axis\n");
				movedist+=fabs(self.movedir_y*self.size_y);
				num_axes+=1;
			}
			if(fabs(self.movedir_z)>0.001)
			{
//				dprint("Z axis\n");
				movedist+=fabs(self.movedir_z*self.size_z);
				num_axes+=1;
			}
			movedist=movedist/num_axes - self.lip;
//			dprintf("Door movedist = %s\n",movedist);
		}
		self.pos2 = self.pos1 + self.movedir*movedist;
	}
	else
	{
//		dprint("Using old door code\n");
		if(self.level)
			self.pos2 = self.pos1 + self.movedir*(self.level - self.lip);
		else
			self.pos2 = self.pos1 + self.movedir*(fabs(self.movedir*self.size) - self.lip);
	}
//	dprintv("Pos1: %s\n",self.pos1);
//	dprintv("Pos2: %s\n",self.pos2);
	if(self.v_angle!='0 0 0')
	{
		self.o_angle=self.angles;
		self.v_angle+=self.angles;
	}

	if(self.wait== -2||self.strength==1)
	{
		self.th_die = chunk_death;
//		self.takedamage = DAMAGE_YES;
		if (!self.health)
		{
			if ((self.thingtype == THINGTYPE_GLASS) || (self.thingtype == THINGTYPE_CLEARGLASS))
				self.max_health = self.health = 25;
			else if ((self.thingtype == THINGTYPE_GREYSTONE) || (self.thingtype == THINGTYPE_BROWNSTONE)||self.thingtype==THINGTYPE_DIRT)
				self.max_health = self.health = 75;
			else if (self.thingtype == THINGTYPE_WOOD)
				self.max_health = self.health = 50;
			else if (self.thingtype == THINGTYPE_METAL)
				self.max_health = self.health = 100;
			else if (self.thingtype == THINGTYPE_FLESH)
				self.max_health = self.health = 30;
			else
				self.max_health = self.health = 25;
		}
	}

// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
// but spawn in the open position
	if (self.spawnflags & DOOR_START_OPEN)
	{
		if(self.v_angle!='0 0 0')
		{
			self.angles = self.v_angle;
			self.v_angle = self.o_angle;
			self.o_angle = self.angles;
		}
		setorigin (self, self.pos2);
		self.pos2 = self.pos1;
		self.pos1 = self.origin;
	}

	self.state = STATE_BOTTOM;

	if (self.health)
	{
		self.takedamage = DAMAGE_YES;
		if(self.strength!=1)
			self.th_die =self.th_pain= door_killed;
	}
	
	if (self.puzzle_piece_1 != string_null || 
		self.puzzle_piece_2 != string_null || 
		self.puzzle_piece_3 != string_null || 
		self.puzzle_piece_4 != string_null)
		self.wait = -1;
		
	self.touch = door_touch;

// LinkDoors can't be done until all of the doors have been spawned, so
// the sizes can be detected properly.
	self.think = LinkDoors;
	self.nextthink = self.ltime + 0.1;

	if (self.cnt)
	{
		self.touch = SUB_Null;
		self.use  = door_rotate_incr;
	}
}


/*QUAKED func_door_smashing (0 .5 .8) ? START_OPEN x DOOR_DONT_LINK TOGGLE SLIDE NORMAL_MOVE remove_pp no_pp
NOTE: Doors can now be activated and deactivated with the appropriate triggers.
"inactive" set to 1 to start the door deactivated.

if two doors touch, they are assumed to be connected and operate as a unit.

-----------------------FIELDS-------------------------
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).

Key doors are always wait -1.

"message" is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"	determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"	movement speed (100 default)
"wait" wait before returning (3 default, -1 = never return)
"lip" lip remaining at end of move (8 default)
"dmg" damage to inflict when blocked (2 default)
"close_target" secondary target to fire when door closes


"soundtype"
0) no sound
1) Big metal door, swinging
2) Big stone door, sliding
3) Big wood door, swinging
4) Normal wood door, swinging
5) Big wood door, sliding
6) Drawbridge
7) Rotating walkway
8) Big metal door, sliding
9) Pendulum swinging



Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
--------------------------------------------------------
void func_door_smashing()
{
	door_sounds();

	SetMovedir ();

	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.classname = "door";

	if(world.spawnflags&MISSIONPACK)
		self.blocked = door_blocked_mp;
	else
		self.blocked = door_blocked;
	self.use = door_use;
	
	if (!self.speed)
		self.speed = 100;
	if (!self.wait)
		self.wait = 3;
	if (!self.lip)
		self.lip = 8;
	if (!self.dmg)
		self.dmg = 2;

	self.pos1 = self.origin;
	self.pos2 = self.pos1 + self.movedir*(fabs(self.movedir*self.size) - self.lip);

// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
// but spawn in the open position
	if (self.spawnflags & DOOR_START_OPEN)
	{
		setorigin (self, self.pos2);
		self.pos2 = self.pos1;
		self.pos1 = self.origin;
	}

	self.state		= STATE_BOTTOM;
	self.takedamage = DAMAGE_YES;
	self.th_die		= chunk_death;

	if(self.soundtype == 1)
		{
		self.thingtype  = THINGTYPE_GREYSTONE;
		self.max_health = self.health = 75;
		}
	else if(self.soundtype == 4)
		{
		self.thingtype  = THINGTYPE_METAL;
		self.max_health = self.health = 100;
		}
	else {
		self.thingtype  = THINGTYPE_WOOD;
		self.max_health = self.health = 50;
		}

	self.touch = door_touch;

// LinkDoors can't be done until all of the doors have been spawned, so
// the sizes can be detected properly.
	self.think = LinkDoors;
	self.nextthink = self.ltime + 0.1;
}*/



/*
=============================================================================

SECRET DOORS

=============================================================================
*/

void fd_secret_move1();
void fd_secret_move2();
void fd_secret_move3();
void fd_secret_move4();
void fd_secret_move5();
void fd_secret_move6();
void fd_secret_done();

float SECRET_OPEN_ONCE = 1;		// stays open
float SECRET_1ST_LEFT = 2;		// 1st move is left of arrow
float SECRET_1ST_DOWN = 4;		// 1st move is down from arrow
float SECRET_NO_SHOOT = 8;		// only opened by trigger
float SECRET_YES_SHOOT = 16;	// shootable even if targeted


void fd_secret_use()
{
	local float temp;

	self.health = 10000;

	// exit if still moving around...
	if(self.origin != self.oldorigin)
		return;

	if(self.inactive)
		return;

	self.message = 0;		// no more message

	SUB_UseTargets();				// fire all targets / killtargets

	if(!self.spawnflags & SECRET_NO_SHOOT)
	{
		self.th_pain = SUB_Null;
		self.takedamage = DAMAGE_NO;
	}
	self.velocity = '0 0 0';

	// Make a sound, wait a little...
	
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise1, 1, ATTN_NORM);
	self.effects(-)EF_UPDATESOUND;
	self.nextthink = self.ltime + 0.1;

	temp = 1 - (self.spawnflags & SECRET_1ST_LEFT);	// 1 or -1
	makevectors(self.mangle);
	
	if(!self.t_width)
	{
		if (self.spawnflags & SECRET_1ST_DOWN)
			self. t_width = fabs(v_up * self.size);
		else
			self. t_width = fabs(v_right * self.size);
	}
		
	if (!self.t_length)
		self. t_length = fabs(v_forward * self.size);

	if (self.spawnflags & SECRET_1ST_DOWN)
		self.dest1 = self.origin - v_up * self.t_width;
	else
		self.dest1 = self.origin + v_right * (self.t_width * temp);
		
	self.dest2 = self.dest1 + v_forward * self.t_length;
	SUB_CalcMove(self.dest1, self.speed, fd_secret_move1);
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
}


/* Wait after the first movement... */

void fd_secret_move1()
{
	self.nextthink = self.ltime + 1;
	self.think = fd_secret_move2;
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise3, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
}


/* Start moving sideways with sound... */

void fd_secret_move2()
{
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	SUB_CalcMove(self.dest2, self.speed, fd_secret_move3);
}


/* Wait here until it's time to go back... */

void fd_secret_move3()
{
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise3, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	if(!self.spawnflags & SECRET_OPEN_ONCE)
	{
		self.nextthink = self.ltime + self.wait;
		self.think = fd_secret_move4;
	}
}


/* Move backward... */

void fd_secret_move4()
{
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	SUB_CalcMove(self.dest1, self.speed, fd_secret_move5);		
}


/* Wait for one second... */

void fd_secret_move5()
{
	self.nextthink = self.ltime + 1;
	self.think = fd_secret_move6;
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise3, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
}

void fd_secret_move6()
{
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise2, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
	SUB_CalcMove(self.oldorigin, self.speed, fd_secret_done);
}

void fd_secret_done()
{
	if (!self.targetname || self.spawnflags&SECRET_YES_SHOOT)
	{
		self.health = 10000;
		self.takedamage = DAMAGE_YES;
		self.th_pain = fd_secret_use;	
	}
	sound(self, CHAN_UPDATE+PHS_OVERRIDE_R, self.noise3, 1, ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND;
}

void secret_blocked()
{
	if (time < self.attack_finished)
		return;
	self.attack_finished = time + 0.5;
	T_Damage (other, self, self, self.dmg);
}


/*
 * secret_touch() -- Prints messages.
 */

void secret_touch()
{
	string s;

	if (other.classname != "player")
		return;

	if (self.attack_finished > time)
		return;

	self.attack_finished = time + 2;
	
	if (self.message)
	{
		s = getstring(self.message);
		centerprint (other, s);
		sound (other, CHAN_BODY, "misc/comm.wav", 1, ATTN_NORM);
	}
}


/*QUAKED func_door_secret (0 .5 .8) ? open_once 1st_left 1st_down no_shoot always_shoot x remove_pp no_pp
NOTE: Doors can now be activated and deactivated with the appropriate triggers.
"inactive" set to 1 to start the door deactivated.

Basic secret door. Slides back, then to the side. Angle determines direction.
-----------------------FIELDS-------------------------
wait  = # of seconds before coming back
1st_left = 1st move is left of arrow
1st_down = 1st move is down from arrow
always_shoot = even if targeted, keep shootable
t_width = override WIDTH to move back (or height if going down)
t_length = override LENGTH to move sideways
"dmg"		damage to inflict when blocked (2 default)
"close_target" secondary target to fire when door closes

If a secret door has a targetname, it will only be opened by it's botton or trigger, not by damage.

"soundtype"
0) no sound
1) Big metal door, swinging
2) Big stone door, sliding
3) Big wood door, swinging
4) Normal wood door, swinging
5) Big wood door, sliding
6) Drawbridge
7) Rotating walkway
8) Big metal door, sliding
9) Pendulum swinging


Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
--------------------------------------------------------
*/

void func_door_secret()
{
	door_sounds();

	if (!self.dmg)
		self.dmg = 2;
		
	// Magic formula...
	self.mangle = self.angles;
	self.angles = '0 0 0';
	if(world.spawnflags&MISSIONPACK)
	{
		if(self.mangle=='0 -1 0')
		{
			self.mangle='-90 0 0';
		}
		else if(self.mangle=='0 -2 0')
		{
			self.mangle='90 0 0';
		}
	}

	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	self.classname = self.netname = "door";
	setmodel (self, self.model);
	setorigin (self, self.origin);	
	
	self.touch = secret_touch;
	self.blocked = secret_blocked;
	self.speed = 50;
	self.use = fd_secret_use;
	if ( !self.targetname || self.spawnflags&SECRET_YES_SHOOT)
	{
		self.health = 10000;
		self.takedamage = DAMAGE_YES;
		self.th_pain = fd_secret_use;
		self.th_die = fd_secret_use;
	}
	self.oldorigin = self.origin;
	if (!self.wait)
		self.wait = 5;		// 5 seconds before closing
}

/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE DOOR_DONT_LINK remove_pp no_pp TOGGLE X_AXIS Y_AXIS
NOTE: Doors can now be activated and deactivated with the appropriate triggers.
"inactive" set to 1 to start the door deactivated.

if two doors touch, they are assumed to be connected and operate as  
a unit.

TOGGLE causes the door to wait in both the start and end states for  
a trigger event.

START_OPEN causes the door to move to its destination when spawned,  
and operate in reverse.  It is used to temporarily or permanently  
close off an area when triggered (not useful for touch or  
takedamage doors).

Key doors are always wait -1.

You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

REVERSE will cause the door to rotate in the opposite direction.

"flags" is how many degrees the door will be rotated.
"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
a "dmg" of -1 will make it only hurt non-players
"flags2" will damage the object that touches it
"strength" When set to 1, it will throw something if it gets in the way
"close_target" secondary target to fire when door closes

"soundtype"
0) no sound
1) Big metal door, swinging
2) Big stone door, sliding
3) Big wood door, swinging
4) Normal wood door, swinging
5) Big wood door, sliding
6) Drawbridge
7) Rotating walkway
8) Big metal door, sliding
9) Pendulum swinging


"abslight" - to set the absolute light level

Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
*/

void func_door_rotating()
{
vector	vec;

	if(self.targetname=="opendraw")
	{
		self.wait = -1;
		self.dmg=10000;
	}

	self.dflags=self.flags;//don't ask
	self.flags=0;

	// set the axis of rotation
	if (self.spawnflags & 64)
		self.movedir = '0 0 1';
	else if (self.spawnflags & 128)
		self.movedir = '1 0 0';
	else
		self.movedir = '0 1 0';

	// check for clockwise rotation
	if (self.spawnflags & 2)
		self.movedir = self.movedir * -1;

	// CHEAT hack to get the puzzle piece flags stored in the 
	// same area, without re-arranging the fields so that the
	// designers don't complain
	self.spawnflags (-) 192;
	if (self.spawnflags & 8) 
		self.spawnflags (+) DOOR_REMOVE_PP;
	if (self.spawnflags & 16) 
		self.spawnflags (+) DOOR_NO_PP;

	if (self.spawnflags & 32) 
		self.spawnflags (+) DOOR_TOGGLE;
	
	self.pos1 = self.angles;
	self.pos2 = self.angles + self.movedir * self.dflags;

	self.max_health = self.health;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.classname = "door_rotating";
	self.netname = "door";

	if (self.abslight)
		self.drawflags (+) MLS_ABSLIGHT;

	if (!self.speed)
		self.speed = 100;
	if (self.wait==0)
		self.wait = 3;
	if (!self.dmg)
//		self.dmg = -1;
		self.dmg = 2;
	if(self.wait== -2||self.strength==1)
	{
		self.th_die = chunk_death;
		if(self.strength==1)
			self.takedamage = DAMAGE_YES;
		if (!self.health)
		{
			if ((self.thingtype == THINGTYPE_GLASS) || (self.thingtype == THINGTYPE_CLEARGLASS))
				self.max_health = self.health = 25;
			else if ((self.thingtype == THINGTYPE_GREYSTONE) || (self.thingtype == THINGTYPE_BROWNSTONE)||self.thingtype==THINGTYPE_DIRT)
				self.max_health = self.health = 75;
			else if (self.thingtype == THINGTYPE_WOOD)
				self.max_health = self.health = 50;
			else if (self.thingtype == THINGTYPE_METAL)
				self.max_health = self.health = 100;
			else if (self.thingtype == THINGTYPE_FLESH)
				self.max_health = self.health = 30;
			else
				self.max_health = self.health = 25;
		}
	}

	door_sounds();

// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
// but spawn in the open position
	if (self.spawnflags & DOOR_START_OPEN)
	{
		self.angles = self.pos2;
		vec = self.pos2;
		self.pos2 = self.pos1;
		self.pos1 = vec;
		self.movedir = self.movedir * -1;
	}

	self.state = STATE_BOTTOM;

	self.touch = door_touch;
	if(world.spawnflags&MISSIONPACK)
		self.blocked = door_blocked_mp;
	else
		self.blocked = door_blocked;
	self.use = door_use;

	if (self.puzzle_piece_1 != string_null || 
		self.puzzle_piece_2 != string_null || 
		self.puzzle_piece_3 != string_null || 
		self.puzzle_piece_4 != string_null)
		self.wait = -1;
		
// LinkDoors can't be done until all of the doors have been spawned, so
// the sizes can be detected properly.

	self.think = LinkDoors;
	self.nextthink = self.ltime + 0.1;

	if (self.cnt)
	{
		self.touch = SUB_Null;
		self.use  = door_rotate_incr;
	}

	if (self.flags2)
	{
		self.touch = door_damage;
		self.flags2=FALSE;
	}
}

