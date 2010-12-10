/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/plats.hc,v 1.3 2007-02-07 16:57:08 sezero Exp $
 */

void() newplat_center_touch;
void() newplat_go_up;
void() newplat_go_down;
void() plat_center_touch;
void() plat_outside_touch;
void() plat_trigger_use;
void() plat_go_up;
void() plat_go_down;
void() plat_crush;
float PLAT_LOW_TRIGGER = 1;

void() crusher_hit_bottom;
void() crusher_hit_top;
void() crusher_trigger_use;
void() crusher_go_up;
void() crusher_go_down;
float CRUSH_MULT   = 1;
float CRUSH_SLIDE  = 2;
float CRUSH_START_OPEN = 4;
float CRUSH_ENDPOS = 8;

float START_BOTTOM = 1;
float START_RTRN= 2;
float CONTINUE= 4;

void() train_wait;
float TRAIN_GLOW = 1;
float TRAIN_WAITTRIG = 2;
float TRAIN_RETURN = 4;

void() plat_spawn_inside_trigger =
{
	local entity	trigger;
	local vector	tmin, tmax;

   //middle trigger

	trigger = spawn();

	if (self.classname == "newplat")
		trigger.touch = newplat_center_touch;
	else
		trigger.touch = plat_center_touch;

	trigger.movetype = MOVETYPE_NONE;
	trigger.solid = SOLID_TRIGGER;
	trigger.enemy = self;
	
	tmin = self.mins + '25 25 0';
	tmax = self.maxs - '25 25 -8';
	tmin_z = tmax_z - (self.pos1_z - self.pos2_z + 8);
	if (self.spawnflags & PLAT_LOW_TRIGGER)
		tmax_z = tmin_z + 8;
	
	if (self.size_x <= 50)
	{
		tmin_x = (self.mins_x + self.maxs_x) / 2;
		tmax_x = tmin_x + 1;
	}
	if (self.size_y <= 50)
	{
		tmin_y = (self.mins_y + self.maxs_y) / 2;
		tmax_y = tmin_y + 1;
	}
	
	setsize (trigger, tmin, tmax);
};

void() plat_hit_top =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_TOP;
	self.think = plat_go_down;
	self.nextthink = self.ltime + 3;
};

void() plat_hit_bottom =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_BOTTOM;
};

void() plat_go_down =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	self.state = STATE_DOWN;
	SUB_CalcMove (self.pos2, self.speed, plat_hit_bottom);
};

void() plat_go_up =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	self.state = STATE_UP;
	SUB_CalcMove (self.pos1, self.speed, plat_hit_top);
};

void() plat_center_touch =
{
	if (other.classname != "player"&&other.movetype!=MOVETYPE_PUSHPULL)//Monsters too?
		return;
		
	if (other.health <= 0)
		return;

	self = self.enemy;
	if (self.state == STATE_BOTTOM)
		plat_go_up ();
	else if (self.state == STATE_TOP)
		self.nextthink = self.ltime + 1;	// delay going down
};

void() plat_outside_touch =
{
	if (other.classname != "player"&&other.movetype!=MOVETYPE_PUSHPULL)
		return;

	if (other.health <= 0)
		return;
		
//dprint ("plat_outside_touch\n");
	self = self.enemy;
	if (self.state == STATE_TOP)
		plat_go_down ();
};

void() plat_trigger_use =
{
	if (self.think)
		return;		// already activated
	plat_go_down();
};

void() plat_crush =
{

	T_Damage (other, self, self, 1);
	
	if (self.state == STATE_UP)
		plat_go_down ();
	else if (self.state == STATE_DOWN)
		plat_go_up ();
	else
		objerror ("plat_crush: bad self.state\n");
};

void() plat_use =
{
	self.use = SUB_Null;
	if (self.state != STATE_UP)
		objerror ("plat_use: not in up state");
	plat_go_down();
};


/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determined by the model's height.
Set "soundtype" to one of the following:
1) pulley
2) chain 
*/

void() func_plat =
{

	if (!self.t_length)
		self.t_length = 80;
	if (!self.t_width)
		self.t_width = 10;

	if (self.soundtype == 0)
		self.soundtype = 2;
// FIX THIS TO LOAD A GENERIC PLAT SOUND

	if (self.soundtype == 1)
	{
		precache_sound ("plats/pulyplt1.wav");
		precache_sound ("plats/pulyplt2.wav");
		self.noise = "plats/pulyplt1.wav";
		self.noise1 = "plats/pulyplt2.wav";
	}

	if (self.soundtype == 2)
	{
		precache_sound ("plats/chainplt1.wav");
		precache_sound ("plats/chainplt2.wav");
		self.noise = "plats/chainplt1.wav";
		self.noise1 = "plats/chainplt2.wav";
	}


	self.mangle = self.angles;
	self.angles = '0 0 0';

	self.classname = "plat";
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	setsize (self, self.mins , self.maxs);

	self.blocked = plat_crush;
	if (!self.speed)
		self.speed = 150;

// pos1 is the top position, pos2 is the bottom
	self.pos1 = self.origin;
	self.pos2 = self.origin;
	if (self.height)
		self.pos2_z = self.origin_z - self.height;
	else
		self.pos2_z = self.origin_z - self.size_z + 8;

	self.use = plat_trigger_use;

	plat_spawn_inside_trigger ();	// the "start moving" trigger	

	if (self.targetname)
	{
		self.state = STATE_UP;
		self.use = plat_use;
	}
	else
	{
		setorigin (self, self.pos2);
		self.state = STATE_BOTTOM;
	}
};

//============================================================================

void() train_next;
void() func_train_find;

void() train_blocked =
{
	if (time < self.attack_finished) return;
	self.attack_finished = time + 0.5;
	T_Damage (other, self, self, self.dmg);
};

void() train_use =
{
	if(self.wait==-1)
		self.use=SUB_Null;

	if (self.spawnflags & TRAIN_GLOW)
	{
	  self.effects = EF_BRIGHTLIGHT;
	}

	if (self.decap != 1) 
	{ 
	 self.decap = 1;
  	 if (self.think != train_next)
	 { 
	   self.think = func_train_find;
	   train_next();
	 }
	}
	else 
	{
	  if (self.spawnflags & TRAIN_RETURN) self.decap = 0;
	   else self.decap = 2;
   }
};

void() train_wait =
{
	//Check to make sure train is active
	if(self.decap!=2)
	{
		self.think = train_next;
		if(self.wait==-2)
		{
			if(self.th_die)
			{
				if(self.pausetime)
				{
					self.think=self.th_die;
					self.nextthink=self.ltime+self.pausetime;
				}
				else
				{
					self.th_die();
					return;
				}
			}
			else
			{
				if(self.pausetime)
				{
					self.think=chunk_death;
					self.nextthink=self.ltime+self.pausetime;
				}
				else
				{
					chunk_death();
					return;
				}
			}
		}
		else if(self.wait==-1)
			self.nextthink=-1;
		else if (self.wait)
		{
			self.nextthink = self.ltime + self.wait;
			sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
		}
		else self.nextthink = self.ltime + 0.1;
	}	
  if (self.decap == 0 || self.decap == 2) self.effects = 0;
};

void() train_rotate =
{
	local entity targ;
	local vector dir;

	targ = self.enemy;

	if (targ.mangle_x != 0 || targ.mangle_y != 0 || targ.mangle_z != 0)
	{
		dir = self.angles;
		dir += targ.mangle;

		SUB_CalcAngleMove (dir, self.speed, train_wait);
	}
	else 
		train_wait();
};

void() train_next =
{
	local entity	targ;
	local vector   dir;
	float targ_speed, targ_aspeed;


	targ = find (world, targetname, self.target);
	self.target = targ.target;

	if (!self.decap && self.spawnflags & TRAIN_RETURN) 
		if (self.netname == targ.targetname) 
			 self.decap = 2;

	if (!self.target)
		objerror ("train_next: no next target");
	if (targ.wait)
		self.wait = targ.wait;
	else
		self.wait = 0;
	
	self.enemy = targ;

	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	
	if (targ.mangle_x != 0 || targ.mangle_y != 0 || targ.mangle_z != 0)
	{
		dir = self.angles;
		dir += targ.mangle;

		if(targ.speed)
			targ_speed=targ.speed;
		else
			targ_speed = self.speed;

		if(targ.anglespeed)
			targ_aspeed=targ.anglespeed;
		else
			targ_aspeed = self.anglespeed;

		if(targ.spawnflags&SYNCH)
			SUB_CalcMoveAndAngleInit (targ.origin - self.mins, targ_speed, dir, targ_aspeed, train_wait,TRUE);
		else
			SUB_CalcMoveAndAngleInit (targ.origin - self.mins, targ_speed, dir, targ_aspeed, train_wait,FALSE);
			
	}
	else
		SUB_CalcMove (targ.origin - self.mins, self.speed, train_rotate);

	if (self.spawnflags & TRAIN_WAITTRIG)
		self.decap = 2;
};

void() func_train_find =
{
	local entity	targ;

	targ = find (world, targetname, self.target);
	self.target = targ.target;
	setorigin (self, targ.origin - self.mins);
	if (!self.targetname)
	{	// not triggered, so start immediately
		self.decap = 1;
		self.nextthink = self.ltime + 0.1;
		self.think = train_next;
	}
};

/*QUAKED func_train (0 .5 .8) ? GLOW TOGGLE RETURN TRANSLUCENT
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
soundtype
1) ratchet metal

if train is only moving to one spot
"angle"    - to tell it's direction
"distance" - in pixels, how far to move
"speed" - how fast it moves between spots (default=100)
"anglespeed" - how fast it rotates to a new angle (default = 100)
"wait" - -1 will make it stop forever, -2 will make it blow up (you can put the waits on the pathcorners and it will take the wait from there.
"pausetime" - How long to wait after getting to the end of it's path before blowing up, default is 0
NOTE: If you give it a wait of -2, be sure to set the thingtype.
thingtype - type of chunks and sprites it will generate
    0 - glass
    1 - grey stone (default for trains)
    2 - wood
    3 - metal
    4 - flesh 
    5 - fire
    6 - clay
    7 - leaves
    8 - hay
    9 - brown stone
   10 - cloth
   11 - wood & leaf
   12 - wood & metal
   13 - wood stone
   14 - metal stone
   15 - metal cloth

The train will modify it's angles by whatever angles it's next path point has, so if it heads towards a path corner with an angle of '0 90 0', the train will rotate '0 90 0' on it's way to the pathpoint.  If you make the anglespeed the same as the angle, the turn should finish right as the train gets to the new spot.

NOTE: A path_corner using spawnflag "SYNCH" will make the train automatically calculate a new anglespeed based on the distance it's going and will finish the turn at the same time the move is done.

As usual, any rotating brush needs an origin brush.

"abslight" - to set the absolute light level

if TRAIN_GLOW is checked, changes to a light globe sprite and lights up an area
*/
void() func_train =
{	
	local entity targ;

	self.decap = 0;

	if (self.spawnflags & TRAIN_GLOW) {
	  self.solid = SOLID_NOT;
  	  setmodel (self, "models/s_light.spr");
	}
	else 
	{
		self.solid = SOLID_BSP;
		setmodel (self, self.model);
	}

	if (!self.speed) self.speed = 100;

	if (!self.anglespeed) self.anglespeed = 100;
	
	if (!self.target) objerror ("func_train without a target");
		
	if (!self.dmg) self.dmg = 2;

	if (self.soundtype == 1)
	{
		self.noise = ("plats/train2.wav");
		precache_sound ("plats/train2.wav");
		self.noise1 = ("plats/train1.wav");
		precache_sound ("plats/train1.wav");
	}
	else
	{
		self.noise = self.noise1 = "misc/null.wav";
		precache_sound ("misc/null.wav");
	}

	if(self.wait==-2)
	{
		if(!self.thingtype)
			self.thingtype=1;
		if(!self.th_die)
			self.th_die=chunk_death;
	}

	self.cnt = 1;
	self.movetype = MOVETYPE_PUSH;
	self.blocked = train_blocked;
	self.use = train_use;
	self.classname = "train";
   
	setsize (self, self.mins , self.maxs);
	setorigin (self, self.origin);
	
	targ = find(world, target, self.target);
	self.netname = targ.target;

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;
	if (self.spawnflags & 8)
	{
		self.drawflags(+)DRF_TRANSLUCENT;
		self.solid = SOLID_NOT;
	}
		

// start trains on the second frame, to make sure their targets have had
// a chance to spawn
	self.nextthink = self.ltime + 0.1;
	self.think = func_train_find;
};

/*QUAK-ED misc_teleporttrain (0 .5 .8) (-8 -8 -8) (8 8 8)
This is used for the final bos
*/
/*
void() misc_teleporttrain =
{	
	if (!self.speed)
		self.speed = 100;
	if (!self.target)
		objerror ("func_train without a target");

	self.cnt = 1;
	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_PUSH;
	self.blocked = train_blocked;
	self.use = train_use;
	self.avelocity = '100 200 300';

	self.noise = ("misc/null.wav");
	precache_sound ("misc/null.wav");
	self.noise1 = ("misc/null.wav");
	precache_sound ("misc/null.wav");

	precache_model2 ("models/teleport.mdl");
	setmodel (self, "models/teleport.mdl");
	setsize (self, self.mins , self.maxs);
	setorigin (self, self.origin);

// start trains on the second frame, to make sure their targets have had
// a chance to spawn
	self.nextthink = self.ltime + 0.1;
	self.think = func_train_find;
};
*/

void() newplat_hit_bottom =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_BOTTOM;
	self.lifetime = time + self.wait;
	if (((self.spawnflags & START_RTRN) && !(self.spawnflags & START_BOTTOM)) ||
		(self.spawnflags & CONTINUE))
	{
		self.nextthink = self.ltime + self.wait;
		self.think=newplat_go_up;
	}
	setorigin (self.enemy, self.origin);	
};

void() newplat_hit_top =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_TOP;
	self.lifetime = time + self.wait;

	if (((self.spawnflags & START_RTRN) && (self.spawnflags & START_BOTTOM)) ||
		(self.spawnflags & CONTINUE))
	{
		self.nextthink = self.ltime + self.wait;
		self.think=newplat_go_down;
	}

	setorigin (self.enemy, self.origin);	
};

void() newplat_trigger_use =
{
	if (self.think)
		return;		// already activated

	if ((self.state==STATE_MOVING) || (self.lifetime > time))
 		return;

	if (self.state == STATE_BOTTOM)
		newplat_go_up ();
	else
		newplat_go_down ();
};



void() newplat_calc_down =
{
	self.state=STATE_MOVING;
	SUB_CalcMove (self.pos2, self.speed, newplat_hit_bottom);
};

void() newplat_go_down =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	newplat_calc_down();
};

void() newplat_calc_up =
{
	self.state=STATE_MOVING;
	SUB_CalcMove (self.pos1, self.speed, newplat_hit_top);
};

void() newplat_go_up =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	newplat_calc_up();
};

void() newplat_crush =
{
	T_Damage (other, self, self, 1);
	if (self.velocity_z < 0)
		newplat_calc_down ();
	else if (self.velocity_z > 0)
		newplat_calc_up();
	else
		objerror ("newplat_crush: bad self.state\n");
};

void() newplat_center_touch =
{

	if ((other.classname != "player"&&other.movetype!=MOVETYPE_PUSHPULL) || (other.health <= 0))
		return;
	
	self = self.enemy;
	if ((self.state==STATE_MOVING) || (self.lifetime > time))
 		return;

	if (self.state == STATE_BOTTOM)
		newplat_go_up ();
	else
		newplat_go_down ();
};

void() newplat_spawn_inside_trigger =
{
	local entity	trigger;

   //middle trigger
	trigger = spawn();

	trigger.touch = newplat_center_touch;

	trigger.movetype = MOVETYPE_PUSH;
	trigger.solid = SOLID_TRIGGER;
	trigger.enemy = self;
	self.enemy = trigger;
	
	setsize (trigger, self.mins,self.maxs);
};

/*QUAKED func_newplat (0 .5 .8) ? START_BOTTOM STRT_RTRN CONTINUE
speed	default 150

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determined by the model's height.

Set "soundtype" to one of the following:
1) base fast
2) chain slow

START_BOTTOM - where plat starts at
if checked plat starts at the bottom of it's movement

START_RTRN - if check will return plat to start position.

CONTINUE - plat will never stop moving


height - distance plat moves up or down

wait - amount of time plat waits before moving (default 3)


*/
void() func_newplat =
{

	if (!self.t_length)
		self.t_length = 80;
	if (!self.t_width)
		self.t_width = 10;

	if (self.soundtype == 0)
		self.soundtype = 2;

	if (self.soundtype == 1)
	{
		precache_sound ("plats/pulyplt1.wav");
		precache_sound ("plats/pulyplt2.wav");
		self.noise = "plats/pulyplt1.wav";
		self.noise1 = "plats/pulyplt2.wav";
	}

	if (self.soundtype == 2)
	{
		precache_sound ("plats/chainplt1.wav");
		precache_sound ("plats/chainplt2.wav");
		self.noise = "plats/chainplt1.wav";
		self.noise1 = "plats/chainplt2.wav";
	}



	self.mangle = self.angles;
	self.angles = '0 0 0';

	self.classname = "newplat";
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	setsize (self, self.mins , self.maxs);

	if (!self.speed)
		self.speed = 150;

	if (!self.wait)
		self.wait = 3;

// pos1 is the top position, pos2 is the bottom
	self.pos1 = self.origin;
	self.pos2 = self.origin;

	if (self.spawnflags & START_BOTTOM)
		self.state=STATE_BOTTOM;
	else
		self.state=STATE_TOP;

   if (self.state==STATE_BOTTOM)
	{
		self.pos1_z = self.origin_z + self.height;
		self.pos2_z = self.origin_z;
	}
	else
	{
		self.pos1_z = self.origin_z;
		self.pos2_z = self.origin_z - self.height;
	}

	self.use = newplat_trigger_use;
	self.blocked = newplat_crush;

	newplat_spawn_inside_trigger ();	//set the "start moving" trigger	

};


/*
===============================================================================

FUNC_CRUSHER

===============================================================================
*/

void() crusher_slide_next =
{
   local vector	vdestdelta;
   local float		len, tspeed;

	tspeed = self.speed;

	if (!tspeed) objerror("No speed defined!");

	//Make sure we're not already at the destination
	if (self.finaldest == self.origin)
	{
		self.velocity = '0 0 0';
		
      if (self.state == STATE_DOWN) self.think = crusher_hit_bottom;
	    else if (self.state == STATE_UP) self.think = crusher_hit_top;

    	self.nextthink = self.ltime + 0.1;
		
		return;
	}
		
	//Set destdelta to the vector needed to move
	vdestdelta = self.finaldest - self.origin;
	
   //Get the length of the vector
	len = vlen (vdestdelta);
	
	//If the length is this small, just stop
	if (len < 1) 
	{
	  if (self.state == STATE_DOWN) crusher_hit_bottom();
	   else if (self.state == STATE_UP) crusher_hit_top();
		 else dprint("NO STATE\n");
	  return;
	}
   		
	self.nextthink = self.ltime + 0.1;
	self.think = crusher_slide_next;
	
	self.velocity = vdestdelta * ((len / (len / 3)) / (self.speed / 100));
};

void(vector tdest) crusher_slide =
{
    self.finaldest = tdest;
	 crusher_slide_next();
};

void() crusher_hit_top =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_TOP;

	if (self.spawnflags & CRUSH_MULT) 
		return;
	
   if (!self.level)
   {
    self.think = crusher_go_down;
	 self.nextthink = self.ltime + 1;
	}
	else 
		self.nextthink = -1;
};

void() crusher_hit_bottom =
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
	self.state = STATE_BOTTOM;
	if (self.level && self.spawnflags & CRUSH_ENDPOS) return;
	self.think = crusher_go_up;
	self.nextthink = self.ltime + 1;
};

void() crusher_go_down =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	self.state = STATE_DOWN;
	if (self.spawnflags & CRUSH_SLIDE) crusher_slide(self.pos2);
    else SUB_CalcMove (self.pos2, self.speed, crusher_hit_bottom);
};

void() crusher_go_up =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	self.state = STATE_UP;
	if (self.spawnflags & CRUSH_SLIDE) crusher_slide(self.pos1);
	 else SUB_CalcMove (self.pos1, self.speed, crusher_hit_top);
};

void() crusher_trigger_use =
{
	if (!self.level) 
	 self.level = TRUE;
   else 
	  self.level = FALSE;

//HCC doesn't like these together in one statement
	if (self.think) 
	 if (self.spawnflags & CRUSH_MULT)
	 	return;		// already activated
	 	
	crusher_go_down();
};

void() crusher_crush =
{
	//Crusher does not return like a plat, it just keeps on crushin'
	T_Damage (other, self, self, self.dmg);
};

void() crusher_use =
{
	if (!self.level) 
	 self.level = TRUE;
   else 
    self.level = FALSE;

	crusher_go_down();
};

/*QUAKED func_crusher (0 .5 .8) ? multiple slide start_open end_open
speed	default 150
dmg default 10

If not targetname is given, crushers will start working immediatly

Crushers are always drawn in the extended position, so they will light correctly.

start_open = start in open position
multiple = go once, return, and wait to be triggered again
slide = slide move (like doors)
end_open = stop in the position opposite what they were drawn in

"lip" same as doors
"speed" speed of the crusher
"wait pause until going in the other direction
"targetname" if set, no trigger is needed (use with multiple)
"dmg" damage the crusher does to a victim

Set "soundtype" to one of the following:
1) base fast
2) chain slow
3) Guillotine
*/

void() func_crusher =
{
	SetMovedir();

	if (self.soundtype == 0) self.soundtype = 2;

	if (self.soundtype == 1)
	{
		precache_sound ("plats/plat1.wav");
		precache_sound ("plats/plat2.wav");
		self.noise = "plats/plat1.wav";
		self.noise1 = "plats/plat2.wav";
	}
	else if (self.soundtype == 2)
	{
		precache_sound ("plats/medplat1.wav");
		precache_sound ("plats/medplat2.wav");
		self.noise = "plats/medplat1.wav";
		self.noise1 = "plats/medplat2.wav";
	}
	else if (self.soundtype == 3)
	{
		precache_sound3 ("plats/guiltin1.wav");
		precache_sound3 ("plats/guiltin2.wav");
		self.noise = "plats/guiltin1.wav";
		self.noise1 = "plats/guiltin2.wav";
	}

	self.mangle = self.angles;
	self.angles = '0 0 0';

	self.classname = "crusher";
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	setsize (self, self.mins , self.maxs);

	self.level = TRUE;

	if (!self.dmg) self.dmg = 10;

	self.blocked = crusher_crush;
	
	if (!self.speed) self.speed = 150;

	self.pos1 = self.origin;
	self.pos2 = self.pos1 + self.movedir*(fabs(self.movedir*self.size) - self.lip);

	if (self.spawnflags & CRUSH_START_OPEN)
	{
	   setorigin (self, self.pos2);
		self.pos2 = self.pos1;
		self.pos1 = self.origin;
	}

	self.use = crusher_trigger_use;

	if (self.targetname)
	{
		self.state = STATE_UP;
		self.use = crusher_use;
	}
	else
	{
		setorigin (self, self.pos2);
		self.state = STATE_BOTTOM;
		self.nextthink = self.ltime + 0.1;
		self.think = crusher_use;
	}
};

void reset_solid (void)
{

}

void rot_mov_dmg (void)
{
	if(other==world)
		return;

	if(other.classname=="player")
	{
		self.solid=SOLID_TRIGGER;
		self.think=reset_solid;
		thinktime self : 0.1;
	}

	if(other.takedamage)
	{
		if(self.noise1)
			sound(self,CHAN_VOICE,self.noise1,1,ATTN_NORM);
		self.pain_finished=time+self.wait;
		T_Damage(other,self,self.owner,self.dmg);
	}
}

void rot_mov_snd (void)
{
	if(self.pain_finished<=time)
	{
		sound(self,CHAN_VOICE,self.noise,1,ATTN_NORM);
		self.pain_finished=time+self.wait;
	}
	self.think=rot_mov_snd;
	thinktime self : self.wait;
}

void rot_mov_activate (void)
{
	if(self.dmg)
		self.touch=rot_mov_dmg;

	if(!self.avelocity)
		self.avelocity=self.o_angle;

	if(self.noise)
		if(!self.wait)
			objerror ("func_rotating_movechain: sound, but no delay time");
		else
		{
			self.think=rot_mov_snd;
			thinktime self : 0;
		}
}

float NOANGLECHAIN = 1;

/*QUAKED func_rotating_movechain (0 .5 .8) ? NOANGLECHAIN

Only one other object in the world should have the same netname.  If not, it will find and use only the first one it finds!

If you're making multiple sawblades, for instance, label the mover and the rotater "sawblade1" for the first one, "sawblade2" for the second, and so on.

If you give it a targetname, it will wait to be activated, this can be seperate from the object it's attached to.

It will not do damage until it's been activated.

NOANGLECHAIN = Setting this flag will stop it from modifying it's angles by the owner's angles, but will still movechain.
dmg	= How much damage it should do when it touches.
noise = Noise it should make, if any, be sure to set the wait time
noise1 = noise it should make when it hits something
wait = Length of the sound so it knows when to loop it.
avelocity = The direction and speed it should spin: pitch yaw roll (this is relative to it's own axis, not the world)
netname = the name of the object it's linked to, that object must have a matching netname!!!


Needs something to tell it to stop?

A way to make it die at the end of a path or if triggered again?

Maybe make movechain_angle optional spawnflag?

What do YOU think?  We'd like to know...
*/
void func_rotating_movechain (void)
{
	if(!self.netname)
		objerror ("func_rotating_movechain has no netname");

	self.owner=find(world,netname,self.netname);
	if(self.owner.classname=="world")
		objerror ("func_rotating_movechain has no owner!");

	self.solid=SOLID_TRIGGER;
	self.movetype=MOVETYPE_NOCLIP;

	setmodel(self,self.model);
	setsize(self,self.mins,self.maxs);
	setorigin(self,self.origin);
	//dprint(vtos(self.avelocity));

	self.owner.movechain=self;
	if(!self.spawnflags&NOANGLECHAIN)
		self.flags+=FL_MOVECHAIN_ANGLE;
	
	if(self.targetname)
	{
		self.use=rot_mov_activate;
		self.o_angle=self.avelocity;
		self.avelocity='0 0 0';
	}
	else
	{
		self.think=rot_mov_activate;
		thinktime self : 3;//wait a few secs for board to start
	}
}

