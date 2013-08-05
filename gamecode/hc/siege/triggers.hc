/*
 * siege/triggers.hc
 */

void() button_return;
void() multi_touch;


float SPAWNFLAG_DODAMAGE = 1;
float SPAWNFLAG_QMULT = 2;
float COUNTER_ORDERED = 2;

entity stemp, otemp, s, old;

void() trigger_reactivate =
{
	self.solid = SOLID_TRIGGER;
};

//=============================================================================

float	SPAWNFLAG_NOMESSAGE = 1;
float	SPAWNFLAG_NOTOUCH = 1;

float SPAWNFLAG_MTOUCH		= 2;
float SPAWNFLAG_PUSHTOUCH	= 4;
float ALWAYS_RETURN			= 4;//for trigger_counter
//float SPAWNFLAG_ACTIVATED	= 8;
float SPAWNFLAG_REMOVE_PP	= 16;
float SPAWNFLAG_NO_PP		= 32;

// the wait time has passed, so set back up for another activation
void() multi_wait =
{
	self.check_ok=FALSE;
	if (self.max_health)
	{
		self.health = self.max_health;
		self.takedamage = DAMAGE_YES;
		self.solid = SOLID_BBOX;
	}
};


// the trigger was just touched/killed/used
// self.enemy should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void() multi_trigger =
{
//dprint("trigger fired\n");
	if (self.nextthink > time)
	{
		return;		// already been triggered
	}

	if (self.classname == "trigger_secret")
	{
		if (self.enemy.classname != "player")
			return;
		found_secrets = found_secrets + 1;
		WriteByte (MSG_ALL, SVC_FOUNDSECRET);
	}

	if (self.noise)
		sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);

// don't trigger again until reset
	self.takedamage = DAMAGE_NO;

	activator = self.enemy;

//	if (self.experience_value)
//	{
//		AwardExperience(activator,self,0);
//	}

	self.check_ok=TRUE;
	SUB_UseTargets();

	if (self.wait > 0)
	{
		self.think = multi_wait;
		thinktime self : self.wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		self.touch = self.think = self.use = SUB_Null;
		self.nextthink=-1;
/*Don't want to remove- may be checked later
		thinktime self : 0.1;
		self.think = SUB_Remove;
*/
	}
};

void() multi_killed =
{
	self.enemy = damage_attacker;
	multi_trigger();
};

float client_has_piece(entity client, string piece)
{
	if (client.puzzle_inv1 == piece ||
		client.puzzle_inv2 == piece ||
		client.puzzle_inv3 == piece ||
		client.puzzle_inv4 == piece ||
		client.puzzle_inv5 == piece ||
		client.puzzle_inv6 == piece ||
		client.puzzle_inv7 == piece ||
		client.puzzle_inv8 == piece)
		return 1;

	if (client.puzzles_cheat) // Did they cheat to get through
		return 1;

	return 0;
}

void client_remove_piece(entity client, string piece)
{
	if (!piece) return;

	if (client.puzzle_inv1 == piece) 
		client.puzzle_inv1 = string_null;
	else if (client.puzzle_inv2 == piece) 
		client.puzzle_inv2 = string_null;
	else if (client.puzzle_inv3 == piece) 
		client.puzzle_inv3 = string_null;
	else if (client.puzzle_inv4 == piece) 
		client.puzzle_inv4 = string_null;
	else if (client.puzzle_inv5 == piece) 
		client.puzzle_inv5 = string_null;
	else if (client.puzzle_inv6 == piece) 
		client.puzzle_inv6 = string_null;
	else if (client.puzzle_inv7 == piece) 
		client.puzzle_inv7 = string_null;
	else if (client.puzzle_inv8 == piece) 
		client.puzzle_inv8 = string_null;
}

float check_puzzle_pieces(entity client, float remove_pieces, float inverse)
{
	float required, has;
	entity found;

	required = has = 0;
	if (self.puzzle_piece_1)
	{
		required (+) 1;
		if (client_has_piece(client,self.puzzle_piece_1))
			has (+) 1;
	}
	if (self.puzzle_piece_2)
	{
		required (+) 2;
		if (client_has_piece(client,self.puzzle_piece_2)) 
			has (+) 2;
	}
	if (self.puzzle_piece_3)
	{
		required (+) 4;
		if (client_has_piece(client,self.puzzle_piece_3)) 
			has (+) 4;
	}
	if (self.puzzle_piece_4)
	{
		required (+) 8;
		if (client_has_piece(client,self.puzzle_piece_4)) 
			has (+) 8;
	}

	if (!inverse && required != has)
		return 0;
	else if (inverse && required == has)
		return 0;

	if (remove_pieces)
	{
		found = find(world, classname, "player");
		while (found)
		{
			client_remove_piece(found,self.puzzle_piece_1);
			client_remove_piece(found,self.puzzle_piece_2);
			client_remove_piece(found,self.puzzle_piece_3);
			client_remove_piece(found,self.puzzle_piece_4);
			found = find(found, classname, "player");
		}
	}

	return 1;
}

void() multi_use =
{
	string temp;
	float removepp, inversepp;

	if (time < self.attack_finished)
		return;

	if (self.spawnflags & SPAWNFLAG_ACTIVATED)
	{
		self.touch = multi_touch;
		return;
	}

	removepp = (self.spawnflags & SPAWNFLAG_REMOVE_PP);
	inversepp = (self.spawnflags & SPAWNFLAG_NO_PP);

	if (!check_puzzle_pieces(other,removepp,inversepp))
	{
		if (self.no_puzzle_msg && !deathmatch)
		{
			temp = getstring(self.no_puzzle_msg);
			if (!deathmatch)
				centerprint (other, temp);
			self.attack_finished = time + 2;
		}
		return;
	}

	self.enemy = activator;
	multi_trigger();
};

void() multi_touch =
{
	float removepp, inversepp;
	string temp;

	if (time < self.attack_finished)
		return;

	if(self.impulse)
		if(other.impulse!=self.impulse)
			return;

	if(!self.siege_team&&self.team)//temp fix
		self.siege_team=self.team;

	if(self.siege_team)
		if(other.siege_team!=self.siege_team)
			return;

	if(self.fail_chance)
		if(random()*100<self.fail_chance)
			return;

	if(self.inactive)
		return;

	if (self.spawnflags & SPAWNFLAG_MTOUCH)
	{
		if (!other.flags & FL_MONSTER)
			return;
	}
	else if (self.spawnflags & SPAWNFLAG_PUSHTOUCH)
	{
		if (!other.flags & FL_PUSH) 
			return;
	}
	else if (other.classname != "player")
		return;

// if the trigger has an angles field, check player's facing direction

	if (self.movedir != '0 0 0')
	{
		makevectors (other.angles);
		if (v_forward * self.movedir < 0)
			return;		// not facing the right way
	}

	if(self.gravity==666)
		if(other.classname=="player"&&!other.flags2&FL2_HARDFALL)
		{
			other.teleport_time=time+2;
			other.flags2(+)FL2_HARDFALL;
		}

	removepp = (self.spawnflags & SPAWNFLAG_REMOVE_PP);
	inversepp = (self.spawnflags & SPAWNFLAG_NO_PP);

	if (!check_puzzle_pieces(other,removepp,inversepp))
	{
		if (self.no_puzzle_msg && !deathmatch)
		{
			temp = getstring(self.no_puzzle_msg);
			if (!deathmatch)
				centerprint (other, temp);
			self.attack_finished = time + 2;
		}
		return;
	}

	self.enemy = other;

//FIXME: TEMP FIX, FIX in MAP	
	if(self.impulse)//cheap fucks
		if(self.attack_finished<time+10)
			self.attack_finished=time+10;

	multi_trigger ();
};

/*QUAKED trigger_multiple (.5 .5 .5) ? notouch monstertouch pushtouch deactivated remove_pp no_pp	lighttoggle lightstartlow
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
"fail_chance" - default 0 - chance that trigger may fail to fire (0 - 100%)
"impulse" - if set, will only fire if the touching entity's impulse is the name number (meant for impulse 13- the "use" impulse)
"gravity" - If "666", this trigger will make anyone falling through it take extra damage when they hit the ground.
If notouch is set, the trigger is only fired by other entities, not by touching.
If monstertouch is set, only monsters may set of the trigger
If deactivated is set, trigger will not fire until it is triggered itself
NOTOUCH has been obsoleted by trigger_relay!
soundtype
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string

Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
"netname" can be an actual string message rather than an idez to the strings.txt
*/
void() trigger_multiple =
{
	if(self.impulse == 33)//temp override
		self.impulse = 13;
	if (self.soundtype == 1)
	{
		precache_sound ("misc/secret.wav");
		self.noise = "misc/secret.wav";
	}
	else if (self.soundtype == 2)
	{
		precache_sound ("misc/comm.wav");
		self.noise = "misc/comm.wav";
	}
	else if (self.soundtype == 3)
	{
		precache_sound ("misc/trigger1.wav");
		self.noise = "misc/trigger1.wav";
	}

	if (!self.wait)
		self.wait = 0.2;
	self.use = multi_use;

	InitTrigger ();

	if (self.health)
	{
		if (self.spawnflags & SPAWNFLAG_NOTOUCH)
			objerror ("health and notouch don't make sense\n");
		self.max_health = self.health;
		self.th_die = multi_killed;
		self.takedamage = DAMAGE_YES;
		self.solid = SOLID_BBOX;
		setorigin (self, self.origin);	// make sure it links into the world
	}
	else
	{	//NOTE: was turning off touch for activate- is this necc?		
		if ( !(self.spawnflags & SPAWNFLAG_NOTOUCH))// && !(self.spawnflags & SPAWNFLAG_ACTIVATED))
		{
			self.touch = multi_touch;
		}
	}
};


/*QUAKED trigger_once (.5 .5 .5) ? notouch monstertouch pushtouch deactivated remove_pp no_pp lighttoggle  lightstartlow
Variable sized trigger. Triggers once, then removes itself.  You must set the key "target" to the name of another object in the level that has a matching
"targetname".  If "health" is set, the trigger must be killed to activate.
If notouch is set, the trigger is only fired by other entities, not by touching.
If monstertouch is set, only monsters can set of the triggers
If deactivated is set, trigger will not work until it is triggered
if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
soundtype
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string
"netname" can be an actual string message rather than an idez to the strings.txt

---------------------------------------
lighttoggle = It will toggle on/off all lights in a level with a matching .style field.
.style = Valid light styles are 33-63.

.lightvalue1 (default 0) 
.lightvalue2 (default 11)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
The lightvalue of .style will always start with the lightvalue1 of the FIRST trigger or button spawned with that .style.

.fadespeed (default 0.5) = How many seconds it will take to complete the desired lighting change

If you turn on lighttoggle, you MUST give this trigger a style value or it will turn on and off all the "normal" lights in the level (hey, maybe that's what you want!)
If you give a .style value between 0 and 32 it will change one of the preset lightstyles.
---------------------------------------

Puzzle Pieces (use the puzzle_id value from the pieces)
   puzzle_piece_1
   puzzle_piece_2
   puzzle_piece_3
   puzzle_piece_4
   no_puzzle_msg: message when player doesn't have the right pieces
*/
void() trigger_once =
{
	self.wait = -1;
	trigger_multiple();
};

/*QUAKED trigger_activate (.5 .5 .5) ? ONCE RELAY x deactivated
*/
void() trigger_activate =
{	
float temp_flags;
	temp_flags=self.spawnflags;
	self.spawnflags(-)1|2;	//Clear first two spawnflags before calling the main trigger funcs
	if (temp_flags & 1)
		trigger_once();
	else if (temp_flags & 2) 
		self.use = SUB_UseTargets;
	else trigger_multiple();
	self.touch=SUB_Null;
};

/*QUAKED trigger_deactivate (.5 .5 .5) ? ONCE RELAY x deactivated
*/
void() trigger_deactivate =
{
//Only diff is classname
	trigger_activate();
};
//=============================================================================

void () interval_use =
{
	SUB_UseTargets();
//	dprint("interval used\n");

	self.think = interval_use;
	thinktime self : self.wait;
};

/*QUAKED trigger_interval (.5 .5 .5) (-8 -8 -8) (8 8 8)
*/
void() trigger_interval =
{
	if (!self.wait)
		self.wait = 5;
//Note- next line was commented out
	InitTrigger ();

	self.use = interval_use;

	self.think = interval_use;
	if (!self.targetname)
		thinktime self : 0.1;
};

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by
other events.  It can contain killtargets, targets, delays, and 
messages.
*/
void() trigger_relay_use =
{
	SUB_UseTargets();
};

void() trigger_relay =
{
	self.use = trigger_relay_use;
};


//=============================================================================

/*QUAK-ED trigger_secret (.5 .5 .5) ?
secret counter trigger
soundtype
1)	secret
2)	beep beep
3)
4)
set "message" to text string
*/
/*
void() trigger_secret =
{
	total_secrets = total_secrets + 1;
	self.wait = -1;

	if (!self.message)
		self.message = 400;  // You found a secret area!
	if (!self.soundtype)
		self.soundtype = 1;

	if (self.soundtype == 1)
	{
		precache_sound ("misc/secret.wav");
		self.noise = "misc/secret.wav";
	}
	else if (self.soundtype == 2)
	{
		precache_sound ("misc/comm.wav");
		self.noise = "misc/comm.wav";
	}

	trigger_multiple ();
};
*/

//=============================================================================


void() counter_find_linked =
{
	entity starte, t;

	starte = self;
	t=nextent(world);

	if (self.netname == "")
		objerror("Ordered counter without a netname\n");

	self.think = SUB_Null;

	while (t != world)
	{
		self.owner = starte;

		t = find(t, netname, starte.netname);

		if(t!=world && t!=starte)
		{
			self.lockentity = t;
			self = t;
		}
	}
	self=starte;
};

void counter_return_buttons ()
{
	entity t;
	t = self.lockentity;

	while(t)
	{
		if (t.classname == "button")//Check for netname match too?
		{
			t.think = button_return;
			t.nextthink = t.ltime + 1;
		}
		t = t.lockentity;
	}
}

void() counter_use_ordered =
{
string oldtarg;
float oldmsg;
string temp;

//replace flags with aflag
	if(self.mangle)
	{
		if(
			(self.cnt==1&&other.aflag!=self.mangle_x)||
			(self.cnt==2&&other.aflag!=self.mangle_y)||
			(self.cnt==3&&other.aflag!=self.mangle_z)
		  )
				self.items = 1;//Wrong order
	}
	else if (other.aflag != self.cnt)
		self.items = 1;//Wrong order?

	self.cnt += 1;
	self.count -= 1;

	if (!self.items)
	{
		if (self.count < 0)
		{
			self.check_ok = TRUE;
			if(self.spawnflags&ALWAYS_RETURN)
				counter_return_buttons();
			if (activator.classname == "player" && (self.spawnflags & SPAWNFLAG_NOMESSAGE) == 0 &&
			    !deathmatch)
			{
				if(self.message)
					temp=getstring(self.message);
				else
					temp="Sequence completed!";
				centerprint(activator, temp);
			}
			self.enemy = activator;
			multi_trigger ();
			self.cnt = 1;
			self.count = self.frags;
			self.items = 0;
		}
	}
	else
	{
		if (self.count < 0)
		{
			self.check_ok = FALSE;
			if (activator.classname == "player" && !deathmatch) 
			{
				if (self.msg2) 
					temp = getstring(self.msg2);
				else
					temp = "Nothing seemed to happen";
				centerprint(activator, temp);
			}

			oldtarg = self.target;
			self.target = self.puzzle_id;
			oldmsg = self.message;
			self.message = FALSE;
			SUB_UseTargets();
			self.message = oldmsg;
			self.target = oldtarg;

			self.cnt = 1;
			self.count = self.frags;
			self.items = 0;

			counter_return_buttons();
		}
	}
};

void() counter_use =
{
//	local string junk;

	self.count -= 1;
	if (self.count < 0)
		return;

	if (self.count != 0)
	{
		if (activator.classname == "player" &&
		    (self.spawnflags & SPAWNFLAG_NOMESSAGE) == 0 && !deathmatch)
		{
			if (self.count >= 4)
				centerprint (activator, "There are more to go...");
			else if (self.count == 3)
				centerprint (activator, "Only 3 more to go...");
			else if (self.count == 2)
				centerprint (activator, "Only 2 more to go...");
			else
				centerprint (activator, "Only 1 more to go...");
		}
		self.check_ok=FALSE;
		return;
	}

	if (activator.classname == "player" &&
	    (self.spawnflags & SPAWNFLAG_NOMESSAGE) == 0 && !deathmatch)
	{
		centerprint(activator, "Sequence completed!");
		sound(activator,CHAN_ITEM,"misc/comm.wav",1,ATTN_NORM);
	}
	self.check_ok=TRUE;
	self.enemy = activator;
	multi_trigger ();
	self.cnt = 1;
	self.count = self.frags;
	self.items = 0;
};

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage ordered always_return deactivated
Acts as an intermediary for an action that takes multiple inputs.

nomessage = it will print "1 more.. " etc when triggered and "sequence complete" when finished.
ordered = things must be triggered in order to make the counter go off
always_return = Buttons will pop back to ready position even if successful (default is that they stay down once correct combination is found)

 - The triggers that trigger the counter need to be ordered using the "aflag" field
 - The first trigger is 1, second is 2, etc.
 - If a trigger is hit out of order, the counter resets
 - Triggers need a name in their netname function, the same name must be in the counter triggers netname fields (the target of the counter should NOT have a netname field, only the things triggering the counter)
 - Count must still be the number of triggers until the counter fires, minus 1 (don't ask why)

"wait" = how long to wait after successful before giving it another try.  Default is -1, meaning it works once and shut off.  If you specify a wait time, the trigger will become a multiple trigger.
"mangle" = This entity has the ability to have a non-sequential sequence of numbers as a combination using mangle.
The format is like a vector, for example, if you want the counter (ordered) to work only if the cnt order of 3, 5, 7 is used, enter the value "3 5 7" (no quotes).
A trigger_combination_assign trigger can pass it's "mangle" value to trigger_counter when it uses it.
This way you can have a number of different possible combinations that could be used and only one wouldbe right (depending, say, on which path the player took).
The values can be as high as you like (okay, from 1 to 65336), so you can have any number of buttons in this puzzle.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and shut off, unless you specify a wait time.
*/
void() trigger_counter =
{
	if(!self.wait)
		self.wait = -1;

	if (!self.count)
		self.count = 2;

	if(self.spawnflags&8)
		self.inactive=TRUE;

//used for the ordered trigger
	self.items = 0;
	self.cnt = 1;
	self.frags = self.count;

	if (self.spawnflags & COUNTER_ORDERED)
	{
		self.use = counter_use_ordered;
		thinktime self : 0.1;
		self.think = counter_find_linked;
	}
	else
		self.use = counter_use;
};

/*QUAKED trigger_combination_assign (.5 .5 .5) ? notouch monstertouch pushtouch deactivated remove_pp no_pp lighttoggle lightstartlow
This will pass it's "mangle" field to it's target- meant for use with an ordered trigger_counter.
It will pass the "mangle" but not USE the counter (it WILL use other targets normally, however).
Otherwise, it behaves just like any other trigger.
Giving it a wait of -1 will make it only work once.
*/
void trigger_combination_assign ()
{
	trigger_multiple();
}

/*QUAKED trigger_counter_reset (.5 .5 .5) ? notouch monstertouch pushtouch deactivated remove_pp no_pp lighttoggle lightstartlow
This will reset a trigger_counter to start counting again as if it hasn't been used yet.  Useful for when you want a counter to count more than once but the counting can be interrupted.
It will reset the counter but not USE the counter (it WILL use other targets normally, however).
Otherwise, it behaves just like any other trigger.
Giving it a wait of -1 will make it only work once.
*/
void trigger_counter_reset ()
{
	trigger_multiple();
}

void() check_find_linked =
{
entity starte, t;

	starte = self;
	t=nextent(world);

	if (self.netname == "")
		objerror("Check trigger without a netname\n");

	self.think = SUB_Null;

	while (t != world)
	{
		t = find(t, netname, starte.netname);

		if(t!=world&&t!=starte)
		{
		//	dprint(t.classname);
		//	dprint(" added to trigger_check chain\n");
			self.check_chain = t;
			self = t;
			self.owner = starte;
		}
	}
	self=starte;
};

void check_use ()
{
entity t;
float failed;

	t=self.check_chain;
	while(t)
	{
		if (!t.check_ok)
		{
		//	dprint(t.classname);
		//	dprint(" failed!\n");
			failed = TRUE;
		}
		t = t.check_chain;
	}

	if (!failed && !self.check_ok)
	{
	//	dprint("Trigger_check: all passed\n");
		self.check_ok = TRUE;
		SUB_UseTargets();
	}
	else if (failed && self.check_ok)
	{
	//	dprint("Failed but check okay, now i'm not check_ok\n");
		self.check_ok = FALSE;
		SUB_UseTargets();
	}
}

/*QUAKED trigger_check (.5 .5 .5) ? 
Checks to see if its child entities are active, and if they are, it triggers

netname = the name to check for its child entities.  Like the trigger_counter, each
			 entity that this checks must share its netname.  

You do not need to specify how many children the trigger has
*/
void() trigger_check =
{
	self.use = check_use;
	thinktime self : 0.1;
	self.think = check_find_linked;
};

/*
==================================================================================

trigger_quake

==================================================================================
*/

void() quake_shake_next =
{
entity player;
	if (self.spawnflags & SPAWNFLAG_DODAMAGE)
		T_Damage (self.enemy, self, self, self.dmg);

	player = find(world, classname, "player");

	if (!player)
		return;

	player.punchangle=RandomVector('5 4 4');
	if(player.flags&FL_ONGROUND)
	{
		player.velocity+=RandomVector('25 25 0');
		player.velocity_z+=random(100,200);
		player.flags(-)FL_ONGROUND;
	}

	self.think = quake_shake_next;
	thinktime self : 0.1;

	if (self.lifespan < time) 
	{
		self.nextthink = -1;
		self.wait = -1;
	}
	else
		thinktime self : 0.1;
};

//Isn't this a great function name?
void() quake_shake =
{
	sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NONE);
	sound(self,CHAN_AUTO,"fx/quake.wav",1,ATTN_NONE);

	self.think = quake_shake_next;
	thinktime self : 0.1;

	SUB_UseTargets();

	if (!self.spawnflags & SPAWNFLAG_QMULT)
		self.wait = -1;
};

void() quake_use =
{
	if (self.nextthink >= time||self.nextthink<0)
		return;

	self.think = quake_shake;
	self.lifespan+=time;
	if(!self.spawnflags&2)
		self.use=SUB_Null;
	thinktime self : self.wait;
};

/*QUAKED trigger_quake (3 26 0) (-10 -10 -10) (10 10 10) dodamage multiple
Earthquake effect

Sorry some of the entity names are screwy, but it saves space

damage default = 5;
lifespan default = 2;
wait default = 1;

dodamage = inflict damage on player

"items" radius of quake
"dmg" damage done to victim
"lifespan" duration of the quake
"target" name of trigger to target (for other effects)
"targetname" set this if you want something else to trigger the trigger
"wait" delay before the quake goes off
*/
void() trigger_quake =
{
	self.use = quake_use;
	if (!self.wait)
		self.wait = 1;
	if (!self.dmg)
		self.dmg = 5;
	if (!self.lifespan)
		self.lifespan = 2;

	InitTrigger ();

	self.touch = SUB_Null;
};

/*
==============================================================================

TELEPORT TRIGGERS

==============================================================================
*/

float	PLAYER_ONLY	= 1;
float	SILENT = 2;

void() play_teleport =
{
	local	float v;
	local	string tmpstr;

	v = random(5);
	if (v < 1)
		tmpstr = "misc/teleprt1.wav";
	else if (v < 2)
		tmpstr = "misc/teleprt2.wav";
	else if (v < 3)
		tmpstr = "misc/teleprt3.wav";
	else if (v < 4)
		tmpstr = "misc/teleprt4.wav";
	else
		tmpstr = "misc/teleprt5.wav";

	sound (self, CHAN_VOICE, tmpstr, 1, ATTN_NORM);
	remove (self);
};

void(vector org) spawn_tfog =
{
	s = spawn ();
	s.origin = org;
	thinktime s : 0.05;
	s.think = play_teleport;

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_TELEPORT);
	WriteCoord (MSG_BROADCAST, org_x);
	WriteCoord (MSG_BROADCAST, org_y);
	WriteCoord (MSG_BROADCAST, org_z);
};


void() tdeath_touch =
{
	if (other == self.owner)
		return;

// frag anyone who teleports in on top of an invincible player
	if (other.classname == "player")
	{
		if (self.owner.classname != "player")
		{	// other monsters explode themselves
			T_Damage (self.owner, self, self.owner, 50000);
			return;
		}

		if (other.artifact_active&ART_INVINCIBILITY)
		{
			if(self.owner.artifact_active&ART_INVINCIBILITY)
			{
				self.classname = "teledeath4";
				other.deathtype=self.owner.deathtype=self.classname;
				remove_invincibility(other);
				remove_invincibility(self.owner);
				T_Damage (other, self, self.owner, 50000);
			}
			else
				self.classname = "teledeath2";
			other=self.owner;
		}

		if ((coop&&teamplay&&self.owner.classname=="player")||
			(deathmatch&&teamplay&&other.siege_team==self.owner.siege_team)
			)
			self.classname = "teledeath3";
	}

	if (other.health&&(!self.frags||other.flags2&FL_ALIVE))
	{
		other.deathtype=self.classname;
		T_Damage (other, self, self.owner, 50000);
	}
};


void(vector org, entity death_owner, float alive_only_tf) spawn_tdeath =
{
entity	death;

	death = spawn();
	death.classname = "teledeath";
	death.movetype = MOVETYPE_NONE;
	death.solid = SOLID_TRIGGER;
	death.angles = '0 0 0';
	setsize (death, death_owner.mins - '1 1 1', death_owner.maxs + '1 1 1');
	setorigin (death, org);
	death.touch = tdeath_touch;
	thinktime death : 0.2;
	death.think = SUB_Remove;
	death.owner = death_owner;
	if(alive_only_tf)
		death.frags = TRUE;

	force_retouch = 2;		// make sure even still objects get hit
};

void teleport_effect_delay ()
{
	GenerateTeleportEffect(self.enemy.origin,0);
	self.attack_finished=time+0.5;

	//commented this for chaos device hangin-around
	if (self.classname == "teleportcoin2")
	{
		self.think = SUB_Remove;
		self.nextthink = time + HX_FRAME_TIME;
	}
}

float DEFENDER = 32;
float ATTACKER = 64;
float AUTOTEAM = 128;
void() teleport_touch =
{
entity	t,arrivedeffect;
vector	org;
float poof_speed;
float no_throw;

//	dprint("Teleporter touched ");
//	dprint(other.classname);
//	dprint("\n");
/*	if(dmMode==DM_SIEGE)
		if(self.siege_team)
		{
			countPlayers();
			if(num_players<max_players)
			{
				bprint(PRINT_HIGH,"Waiting for all players to join, only ");
				bprinti(PRINT_HIGH,num_players);
				bprint(PRINT_HIGH," are in, need ");
				bprinti(PRINT_HIGH,max_players);
				bprint(PRINT_HIGH," to start\n");
				return;//Everyone must be in before you can begin
			}
		}*/

	if (other.classname == "tripwire")//no teleport tripmines!
	{
		T_Damage (other, self, self, 200 );
		return;
	}
	if (other.classname == "chain_head")//no teleport tripmines!
	{
		T_Damage (other.owner, self, self, 200 );
		return;
	}

	if(self.inactive)
		return;

	if(self.siege_team)
		if(other.siege_team!=self.siege_team)
			return;

	if (self.spawnflags & PLAYER_ONLY)
	{
		if (other.classname != "player")
			return;
	}

// Don't teleport world geometry
	if (other.solid == SOLID_BSP||other.solid==SOLID_TRIGGER||other.safe_time>time)/*teleport*/
		return;

	if(self.spawnflags&DEFENDER)
		become_defender(other);
	else if(self.spawnflags&ATTACKER)
		become_attacker(other);
	else if(self.spawnflags&AUTOTEAM)
		become_either(other);

	SUB_UseTargets ();

// put a tfog where the player was UNLESS silent is checked (jweier)
	if (!self.spawnflags & SILENT)
		GenerateTeleportEffect(other.origin,0);

	if (self.classname != "teleportcoin")
	{
		if(self.spawnflags&16)
			t = SelectSpawnPoint ();
		else
		{
			t = find (world, targetname, self.target);

			while(t!=world&&t.classname!="info_teleport_destination")
				t = find (t, targetname, self.target);
		}
		if (!t)
			objerror ("couldn't find target");
	}
	else
		t = self.goalentity;

// spawn a tfog flash in front of the destination
	if(t.avelocity!='0 0 0')
		t.mangle=t.angles;

	if(self.spawnflags&16||t.spawnflags&1||self.classname=="teleportcoin")
		no_throw=TRUE;
	else
		no_throw=FALSE;

	if(!no_throw)
	{
		makevectors (t.mangle);
		org = t.origin + 32 * v_forward;
	}
	else
		org=t.origin;

	spawn_tdeath(t.origin, other,FALSE);

// move the player and lock him down for a little while
	if (!other.health&&other.size!='0 0 0')
	{//Exclude projectiles!
		other.origin = t.origin;
		if(!no_throw)	//In case you don't want to push them in a certain dir
			other.velocity = (v_forward * other.velocity_x) + (v_forward * other.velocity_y);
		return;
	}

	if((t.spawnflags&2||self.spawnflags&16)&&other.classname=="player")
		other.velocity='0 0 0';//Kill all player's velocity

	setorigin (other, t.origin);

	if (!self.spawnflags & SILENT)
	{
		//adding condition--else part is new--for chaos device hangin-around
		if (self.classname != "teleportcoin")
		{
			self.enemy=other;
			self.think=teleport_effect_delay;
			thinktime self : 0.05;
		}
		else
		{
			arrivedeffect=spawn();
			arrivedeffect.enemy=other;
			arrivedeffect.think=teleport_effect_delay;
			thinktime arrivedeffect : 0.05;
			arrivedeffect.classname="teleportcoin2";
			thinktime self : 0;
			self.think=SUB_Remove;
		}
	}
	other.safe_time = time + 0.7;/*teleport*/

	if(!no_throw)
	{
		other.angles = t.mangle;
		other.fixangle = 1;		// turn this way immediately
		if(other.classname!="player"&&other.velocity!='0 0 0')
			poof_speed = vlen(other.velocity);
		else
			poof_speed = 300;
		other.velocity = v_forward * poof_speed;
	}

	other.flags(-)FL_ONGROUND;

	UpdateMissileVelocity(other);
};

/*QUAKED info_teleport_destination (.5 .5 .5) (-8 -8 -8) (8 8 32) NO_THROW kill_velocity
This is the destination marker for a teleporter.  It should have a "targetname" field with the same value as a teleporter's "target" field.

NO_THROW = won't throw the entity it teleports in the direction (angles) it's facing
kill_velocity = players will come out the other side with no velocity

=====FIELDS=====
"angles" - Will turn player this way and push him in this direction unless the NO_THROW spawnflag is on.
================
*/
void() info_teleport_destination =
{
// this does nothing, just serves as a target spot
	if(self.avelocity!='0 0 0')
		self.movetype	= MOVETYPE_NOCLIP;
	self.mangle = self.angles;
	self.angles = '0 0 0';
	self.model = "";
	self.origin = self.origin + '0 0 27';
	if (!self.targetname)
		dprint("error- no targetname on teleport dest");
};

void teleport_shut_off ()
{
	self.touch=SUB_Null;
	self.think = SUB_Null;
	thinktime self : 0;
}

void() teleport_use =
{
//	if(self.inactive)
//		return;

//	dprint("Teleporter used\n");
	self.touch = teleport_touch;
	thinktime self : 0.2;
	force_retouch = 2;		// make sure even still objects get hit
	self.think = teleport_shut_off;
};

/*QUAKED trigger_teleport (.5 .5 .5) ? PLAYER_ONLY SILENT inactive inactive CHAOS Defender Attacker AutoTeam
Any object touching this will be transported to the corresponding info_teleport_destination entity. You must set the "target" field, and create an object with a "targetname" field that matches.

SILENT = No effect or sound
CHAOS = Will act like a Chaos device- teleports you to a start spot somewhere on the map
COOL DESIGN IDEA: If you like, you can use a trigger_message_transfer to change the target of the teleporter so it can go different places at different times.

If the trigger_teleport has a targetname, it will only teleport entities when it has been fired.
*/
void() trigger_teleport =
{
vector o;

	InitTrigger ();
	if(!self.targetname)
		self.touch = teleport_touch;
	else
		self.use = teleport_use;
//	self.touch = teleport_touch;
//	self.use = teleport_use;

	// find the destination 
	if (self.target==""&&!self.spawnflags&16)
		dprint ("Error - no target on trigger_teleport");

	if (!(self.spawnflags & SILENT))
	{
		precache_sound ("ambience/newhum1.wav");
		o = (self.mins + self.maxs)*0.5;
		ambientsound (o, "ambience/newhum1.wav",0.5 , ATTN_STATIC);
	}

	if (self.spawnflags & 4)
		self.inactive = TRUE;
};


/*-----------------------------------
	Inter-Level Teleport - aleggett
  -----------------------------------*/
/*
void teleport_newmap_touch()
{
	if(other.classname != "player" || other.health <= 0 ||
	   other.solid != SOLID_SLIDEBOX)
		return;
	stuffcmd(self, self.target);
}
*/

/*QUAK-ED trigger_teleport_newmap (.5 .5 .5)
Any player touching this will be transported to the map named in .target.
.target uses the syntax:

	map e1m1

or corresponding to any other levelname.
*/
/*
void trigger_teleport_newmap()
{
	InitTrigger();
	if(!self.target)
		objerror("no target map");
	self.touch = teleport_newmap_touch;
}
*/

/*
==============================================================================

trigger_setskill

==============================================================================
*/
/*
void() trigger_skill_touch =
{
	string temp;

	if (other.classname != "player")
		return;

	temp = getstring(self.message);
	cvar_set ("skill", temp);
};
*/

/*QUAKED trigger_setskill (.5 .5 .5) ?
sets skill level to the value of "message".
Only used on start map.
*/
/*
void() trigger_setskill =
{
	InitTrigger ();
	self.touch = trigger_skill_touch;
};
*/

/*
==============================================================================

ONLY REGISTERED TRIGGERS

==============================================================================
*/
/*
void() trigger_onlyregistered_touch =
{
	if (other.classname != "player")
		return;
	if (self.attack_finished > time)
		return;

	self.attack_finished = time + 2;
	if (cvar("registered"))
	{
		self.message = "";
		SUB_UseTargets ();
		remove (self);
	}
	else
	{
		if (self.message != "" && !deathmatch)
		{
			centerprint (other, self.message);
			sound (other, CHAN_BODY, "misc/comm.wav", 1, ATTN_NORM);
		}
	}
};
*/

/*QUAK-ED trigger_onlyregistered (.5 .5 .5) ?
Only fires if playing the registered version, otherwise prints the message
*/
/*
void() trigger_onlyregistered =
{
	precache_sound ("misc/comm.wav");
	InitTrigger ();
	self.touch = trigger_onlyregistered_touch;
};
*/

//============================================================================

void() hurt_on =
{
	self.solid = SOLID_TRIGGER;
	self.nextthink = -1;
};

void() hurt_touch =
{
	if (other.takedamage)
	{
		self.solid = SOLID_NOT;
		T_Damage (other, self, self, self.dmg);
		self.think = hurt_on;
		thinktime self : 1;
	}
};

/*QUAKED trigger_hurt (.5 .5 .5) ?
Any object touching this will be hurt
set dmg to damage amount
defalt dmg = 5
*/
void() trigger_hurt =
{
	InitTrigger ();
	self.touch = hurt_touch;
	if (!self.dmg)
		self.dmg = 1000;
};

//============================================================================

//============================================================================

float PUSH_ONCE = 1;

void trigger_push_gone (void)
{
	remove(self);
}

void() trigger_push_touch =
{
	if(self.inactive)
		return;

	if(self.spawnflags&2)
		if(other.flags&FL_ONGROUND)
			return;

	if (other.movetype&&other.solid!=SOLID_BSP)//health>0?
	{
		other.velocity = self.movedir * self.speed;
		UpdateMissileVelocity(other);
		if(other.movedir!='0 0 0')
			other.movedir=self.movedir;
		if ((other.classname == "player") && (other.flags & FL_ONGROUND))
		{
			sound (other, CHAN_AUTO, "ambience/windpush.wav", 1, ATTN_NORM);//MAKE OPTIONAL
			other.flags (-) FL_ONGROUND;
		}
		other.safe_time=time+0.5;//so they don't take impact damage from push brushes
	}
	if (self.spawnflags & PUSH_ONCE)
		remove(self);
};

void trigger_push_turn_on ()
{
	self.use = trigger_push_gone;
	self.touch = trigger_push_touch;
}

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE no_pickup x INACTIVE

Pushes the player in the direction set by angles
When used while "on", removes it.

PUSH_ONCE - will go away after one use.
no_pickup - will not lift player off the ground- they have to jump first to be lifted
INACTIVE - Must be turned on by a trigger_activate before it can be used
-------------------------FIELDS-------------------------
Angles - the direction to push
Speed - how hard to push (default 500)
If you target it, it waits to be triggered to turn on- next use will remove it.
--------------------------------------------------------
*/
void() trigger_push =
{
	if(self.angles=='0 0 0')
		self.movedir='1 0 0';

	InitTrigger ();

	precache_sound ("ambience/windpush.wav");

	if(self.targetname)
	{
		self.use = trigger_push_turn_on;
	}
	else
	{
		self.use = trigger_push_gone;
		self.touch = trigger_push_touch;
	}
	if (!self.speed)
		self.speed = 500;
};


//============================================================================

void() trigger_monsterjump_touch =
{
	if ( other.flags & (FL_MONSTER | FL_FLY | FL_SWIM) != FL_MONSTER )
		return;

// set XY even if not on ground, so the jump will clear lips
	other.velocity_x = self.movedir_x * self.speed;
	other.velocity_y = self.movedir_y * self.speed;

	if ( !(other.flags & FL_ONGROUND) )
		return;

	other.flags(-)FL_ONGROUND;

	other.velocity_z = self.height;

	if(self.cnt)
		self.cnt-=1;
	else if(self.wait=-1)
		self.touch=SUB_Null;

	if(other.th_jump)
	{
		other.think=other.th_jump;
		thinktime other : 0;
	}
};

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/
void() trigger_monsterjump =
{
	if(!self.cnt)
		self.cnt=4;
	if (!self.speed)
		self.speed = 200;
	if (!self.height)
		self.height = 200;
	if (self.angles == '0 0 0')
		self.angles = '0 360 0';
	InitTrigger ();
	self.touch = trigger_monsterjump_touch;
};

/*
void() trigger_magicfield_touch =
{
	if (other.classname == "grenade")
		other.velocity = self.speed * self.movedir * 10;
	else if (other.health > 0)
	{
		if (other.artifact_active & ART_TOMEOFPOWER)
			return;
		else other.velocity = self.speed * self.movedir * 10;

		if (other.classname == "player" && !deathmatch)
		{
			makevectors(other.angles);
			SpawnPuff(other.origin + (v_forward * random(160)), '0 0 -10', 101,other);
			SpawnPuff(other.origin + (v_forward * random(160)), '5 5 0', 101,other);
			SpawnPuff(other.origin + (v_forward * random(160)), '0 0 10', 101,other);
			centerprint(other, "You must have the Tome of Power\n");
		}
	}
	if (self.spawnflags & PUSH_ONCE)
		remove(self);
};
*/

/*QUAK-ED trigger_magicfield (.5 .5 .5) ? 
Denies player access without a certain item
*/
/*
void() trigger_magicfield =
{
	InitTrigger ();
	self.touch = trigger_magicfield_touch;
	if (!self.speed)
		self.speed = 100;
};
*/


/*
==============================================================================

trigger_crosslevel

==============================================================================
*/

void() trigger_crosslevel_use =
{
	if(other.classname=="trigger_check")
	{
		if(!other.check_ok)
		{
			self.check_ok=FALSE;
			if (self.spawnflags & 1)
				serverflags(-)SFL_CROSS_TRIGGER_1;
			if (self.spawnflags & 2)
				serverflags(-)SFL_CROSS_TRIGGER_2;
			if (self.spawnflags & 4)
				serverflags(-)SFL_CROSS_TRIGGER_3;
			if (self.spawnflags & 8)
				serverflags(-)SFL_CROSS_TRIGGER_4;
			if (self.spawnflags & 16)
				serverflags(-)SFL_CROSS_TRIGGER_5;
			if (self.spawnflags & 32)
				serverflags(-)SFL_CROSS_TRIGGER_6;
			if (self.spawnflags & 64)
				serverflags(-)SFL_CROSS_TRIGGER_7;
			if (self.spawnflags & 128)
				serverflags(-)SFL_CROSS_TRIGGER_8;
			return;
		}
	}
	if (self.spawnflags & 1)
		serverflags(+)SFL_CROSS_TRIGGER_1;
	if (self.spawnflags & 2)
		serverflags(+)SFL_CROSS_TRIGGER_2;
	if (self.spawnflags & 4)
		serverflags(+)SFL_CROSS_TRIGGER_3;
	if (self.spawnflags & 8)
		serverflags(+)SFL_CROSS_TRIGGER_4;
	if (self.spawnflags & 16)
		serverflags(+)SFL_CROSS_TRIGGER_5;
	if (self.spawnflags & 32)
		serverflags(+)SFL_CROSS_TRIGGER_6;
	if (self.spawnflags & 64)
		serverflags(+)SFL_CROSS_TRIGGER_7;
	if (self.spawnflags & 128)
		serverflags(+)SFL_CROSS_TRIGGER_8;
	self.check_ok=TRUE;
	SUB_UseTargets();
	self.solid = SOLID_NOT;
};

void() trigger_crosslevel_touch =
{
	if (other.classname != "player")
		return;
	activator = other;
	trigger_crosslevel_use();
};

/*QUAKED trigger_crosslevel (.5 .5 .5) ? trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.
*/
void() trigger_crosslevel =
{
	if (((self.spawnflags & 1) && (serverflags & SFL_CROSS_TRIGGER_1)) ||
		((self.spawnflags & 2) && (serverflags & SFL_CROSS_TRIGGER_2)) ||
		((self.spawnflags & 4) && (serverflags & SFL_CROSS_TRIGGER_3)) ||
		((self.spawnflags & 8) && (serverflags & SFL_CROSS_TRIGGER_4)) ||
		((self.spawnflags & 16) && (serverflags & SFL_CROSS_TRIGGER_5)) ||
		((self.spawnflags & 32) && (serverflags & SFL_CROSS_TRIGGER_6)) ||
		((self.spawnflags & 64) && (serverflags & SFL_CROSS_TRIGGER_7)) ||
		((self.spawnflags & 128) && (serverflags & SFL_CROSS_TRIGGER_8)))
	{
		self.solid = SOLID_NOT;
		self.flags(+)FL_ARCHIVE_OVERRIDE;
		return;
	}
	InitTrigger ();
	self.inactive = FALSE;
	self.touch = trigger_crosslevel_touch;
	self.use = trigger_crosslevel_use;
};

/*QUAKED trigger_crosslevel_target (.5 .5 .5) ? trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  It is OK to check multiple triggers.  Delay, target and killtarget also work.
*/
void() trigger_crosslevel_target_think =
{
entity found;
	found=find(world,classname,"player");
	if(!found)
	{
//		bprint("Postponing check\n");
		thinktime self : 3;
	}
	else if (((self.spawnflags & 1) && (serverflags & SFL_CROSS_TRIGGER_1)) ||
		((self.spawnflags & 2) && (serverflags & SFL_CROSS_TRIGGER_2)) ||
		((self.spawnflags & 4) && (serverflags & SFL_CROSS_TRIGGER_3)) ||
		((self.spawnflags & 8) && (serverflags & SFL_CROSS_TRIGGER_4)) ||
		((self.spawnflags & 16) && (serverflags & SFL_CROSS_TRIGGER_5)) ||
		((self.spawnflags & 32) && (serverflags & SFL_CROSS_TRIGGER_6)) ||
		((self.spawnflags & 64) && (serverflags & SFL_CROSS_TRIGGER_7)) ||
		((self.spawnflags & 128) && (serverflags & SFL_CROSS_TRIGGER_8)))
	{
		activator = world;
		self.check_ok=TRUE;
		SUB_UseTargets();
	}
	else
		self.check_ok=FALSE;
};

void() trigger_crosslevel_target =
{
	self.think = trigger_crosslevel_target_think;
//FIXME: temporarily lenghtened this so I could use the addserverflags impulse
//	thinktime self : 0.5;
	thinktime self : 3;
	self.solid = SOLID_NOT;
	self.flags(+)FL_ARCHIVE_OVERRIDE;
};

/*QUAK-ED trigger_deathtouch (.5 .5 .5)

Kills anything that has a matching targetname and touches it.

th_die = Set this if you want the object to have a specific death, defaults to SUB_Remove.

If it is SUB_Remove, it will execute the th_die of the object, if it has one.
If the object doesn't have a th_die, but it has health, it will execute chunk_death.
If it doesn't have health, it will just be removed.

FIXME: Solid_bsp's don't seem to touch this
*/
/*
void trigger_deathtouch_touch (void)
{
	if(other.targetname!=self.target)
		return;

	other.targetname="";//so i don't keep on killing it

	if(self.th_die)
		other.think=self.th_die;
	else if(other.th_die)
		other.think=other.th_die;
	else if(other.health)
		other.think=chunk_death;
	else
		other.think=SUB_Remove;
	thinktime other : 0.05;
}

void trigger_deathtouch (void)
{
	InitTrigger ();
	self.touch = trigger_deathtouch_touch;
}
*/

void GetPuzzle(entity item, entity person)
{
	if (!person.puzzle_inv1)
		person.puzzle_inv1 = item.puzzle_id;
	else if (!person.puzzle_inv2)
		person.puzzle_inv2 = item.puzzle_id;
	else if (!person.puzzle_inv3)
		person.puzzle_inv3 = item.puzzle_id;
	else if (!person.puzzle_inv4)
		person.puzzle_inv4 = item.puzzle_id;
	else if (!person.puzzle_inv5)
		person.puzzle_inv5 = item.puzzle_id;
	else if (!person.puzzle_inv6)
		person.puzzle_inv6 = item.puzzle_id;
	else if (!person.puzzle_inv7)
		person.puzzle_inv7 = item.puzzle_id;
	else if (person.puzzle_inv8)
		person.puzzle_inv8 = item.puzzle_id;
	else
		dprint("No room for puzzle piece!\n");
}

void GetPuzzle2(entity item, entity person, string which)
{
	item.puzzle_id = which;
}

void puzzle_touch(void)
{
	local entity	stemp;
	local float	amount;

	if(self.t_width>time&&other==self.enemy)
		return;//last owner can't pick up again for 5 seconds

	if (other.classname != "player")
		return;

	if (other.health <= 0)  // Dead players can't pick stuff up
		return;

	if (other.puzzle_inv1 == self.puzzle_id ||
		other.puzzle_inv2 == self.puzzle_id ||
		other.puzzle_inv3 == self.puzzle_id ||
		other.puzzle_inv4 == self.puzzle_id ||
		other.puzzle_inv5 == self.puzzle_id ||
		other.puzzle_inv6 == self.puzzle_id ||
		other.puzzle_inv7 == self.puzzle_id ||
		other.puzzle_inv8 == self.puzzle_id)
		return;

	thinktime self : 0;
	self.think = SUB_Remove;

	amount = random();
	if (amount < 0.5)
	{
		sprinti (other, PRINT_MEDIUM, STR_YOUPOSSESS);
		sprint (other, PRINT_MEDIUM, self.netname);
	}
	else
	{
		sprinti (other, PRINT_MEDIUM, STR_YOUHAVEACQUIRED);
		sprint (other, PRINT_MEDIUM, self.netname);
	}
	other.pain_target=self.netname;

	sprint (other,PRINT_MEDIUM, "\n");

	other.flags2(+)FL2_HASKEY;
	WriteTeam (SVC_HASKEY,other);

	GetPuzzle(self, other);

	sound (other, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");

	if (coop)
		return;

	self.solid = SOLID_NOT;
	self.model = string_null;

/*	if (coop)
	{
		self.mdl = self.model;
		thinktime self : 60;
		self.think = SUB_regen;
	}*/

	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets
}

void puzzle_use(void)
{
	entity found;
	float num_found;

	self.effects(-)EF_NODRAW;
	self.solid = SOLID_TRIGGER;
	self.use = SUB_Null;
	self.touch = puzzle_touch;

	setorigin(self,self.origin);

	num_found = 0;

	if (self.spawnflags & 4)
	{
		found = find(world, classname, "player");
		while (found)
		{
			if (vlen(found.origin-self.origin) < 200)
			{
				num_found += 1;
				other = found;
				self.touch();
			}
			found = find(found, classname, "player");
		}
	}

	if (num_found == 1 && !coop)
	{
		remove(self);
	}
	else
	{
		StartItem();
	}
}

/*QUAKED puzzle_piece (1 .6 0) (-8 -8 -28) (8 8 8) SPAWN FLOATING AUTO_GET SIEGEKEY
Puzzle Piece

IMPORTANT!  All puzzle_piece models need to be in the subdirectory "models/puzzle/"!!!
SIEGEKEY - This puzzle_piece is intended as the key to the throneroom on a siege map
-------------------------FIELDS-------------------------
puzzle_id: the name that identifies the piece
           (this should 5 characters or less- no extensions- for example, "cskey.mdl" is just "cskey")
mdl: do not set unless you are using it for a Siege Key- set this as the path from your Hexen directory to the model you will be using- for example "models/puzzle/cskey.mdl"
netname: the name the player sees when picked up
--------------------------------------------------------
*/
void puzzle_piece(void)
{
//RICK: Added floating spawnflag as per Brian R.'s request
	if(!self.puzzle_id)
	{
		dprint("Warning!  No 'puzzle_id' field on puzzle_piece!!!\n");
		return;
	}

	if(!self.flags2&FL2_REPLACEMENT)
	{
		precache_sound("items/artpkup.wav");
		precache_puzzle_model(self.puzzle_id);
	}
	setpuzzlemodel(self,self.puzzle_id);
	if(self.spawnflags&8||self.puzzle_id=="cskey")//temp fix
	{
		g_keyname=self.puzzle_id;
		g_keymdl=self.model;
	}

	self.noise = "items/artpkup.wav";

	if (self.spawnflags & 1)
	{
		setsize (self, '-4 -4 -16', '4 4 0');
//		setsize (self, '-8 -8 -8', '8 8 16');
		self.spawnflags(-)1;
		self.solid = SOLID_NOT;
		self.effects(+)EF_NODRAW;
		self.use = puzzle_use;
	}
	else
	{
		setsize(self,'0 0 0','0 0 0');
		self.hull=HULL_POINT;
		self.solid = SOLID_BBOX;
		self.touch = puzzle_touch;
		self.think=StartItem;
		thinktime self : 0;
	}
	if(self.spawnflags&2)
		self.spawnflags=1;

	if ((self.puzzle_id == "glass") || (self.puzzle_id == "lens"))
		self.drawflags (+) DRF_TRANSLUCENT;

	if(!self.flags2&FL2_REPLACEMENT)
		spawn_key_tracker();
}

void DropPuzzlePiece(float throwkey)
{
entity newpuzz;
	newpuzz=spawn();
	setpuzzlemodel(newpuzz,self.puzzle_id);
	newpuzz.noise = "items/artpkup.wav";

	setsize(newpuzz,'0 0 0','0 0 0');
	newpuzz.hull=HULL_POINT;
	newpuzz.solid = SOLID_BBOX;
	newpuzz.touch = puzzle_touch;
	newpuzz.t_width=time+5;
	newpuzz.enemy=self;
	newpuzz.think=StartItem;
	thinktime newpuzz : 0;

	if ((self.puzzle_id == "glass") || (self.puzzle_id == "lens"))
		newpuzz.drawflags (+) DRF_TRANSLUCENT;

	if(throwkey)
	{
		makevectors(self.angles);
		setorigin(newpuzz,self.origin+'0 0 18'+v_forward*8);//make it easier to place on ledges
	}
	else
		setorigin(newpuzz,self.origin+'0 0 18');
	newpuzz.wallspot=newpuzz.origin;
	newpuzz.classname="puzzle_piece";
	newpuzz.netname = self.pain_target;
	newpuzz.puzzle_id=self.puzzle_id;
	self.puzzle_id="";
	self.flags2(-)FL2_HASKEY;
	WriteTeam(SVC_NONEHASKEY,self);
}
/*
void DropPuzzlePiece(float throwkey)
{
entity newpuzz;
vector throw_org;
//	dprint("Dropped puzzle piece\n");
	throw_org = self.origin+self.maxs_z*'0 0 1' - '0 0 6';
	if(throwkey)
	{
		makevectors(self.v_angle);
		traceline(throw_org,throw_org + v_forward * 32,FALSE,self);
		if (trace_fraction < 1||v_forward_z<-0.2)
			throwkey=FALSE;
	}

	newpuzz=spawn();
	setpuzzlemodel(newpuzz,self.puzzle_id);
	newpuzz.noise = "items/artpkup.wav";

	setsize(newpuzz,'0 0 0','0 0 0');
	newpuzz.hull=HULL_POINT;
	newpuzz.solid = SOLID_BBOX;
	newpuzz.touch = puzzle_touch;
	newpuzz.t_width=time+5;
	newpuzz.enemy=self;
	if(throwkey)
	{
		newpuzz.velocity=v_forward*200;
		newpuzz.movetype=MOVETYPE_TOSS;
		newpuzz.mdl = self.model;		// so it can be restored on respawn
		newpuzz.flags(+)FL_ITEM;		// make extra wide
		newpuzz.solid = SOLID_BBOX;
		newpuzz.flags(+)FL_IGNORESIZEOFS;
		newpuzz.hull=HULL_POINT;
	}
	else
	{
		newpuzz.think=StartItem;
		thinktime newpuzz : 0;
	}

	if ((self.puzzle_id == "glass") || (self.puzzle_id == "lens"))
		newpuzz.drawflags (+) DRF_TRANSLUCENT;

	setorigin(newpuzz,throw_org);
	newpuzz.wallspot=newpuzz.origin;
	newpuzz.classname="puzzle_piece";
	newpuzz.netname = self.pain_target;
	newpuzz.puzzle_id=self.puzzle_id;
	self.puzzle_id="";
	self.flags2(-)FL2_HASKEY;
	WriteTeam(SVC_NONEHASKEY,self);
}
*/

void puzzle_static_use(void)
{
	setpuzzlemodel(self,self.puzzle_id);

	/*if (!droptofloor())
	{
		dprint ("Static Puzzle Piece fell out of level at ");
		dprint (vtos(self.origin));
		dprint ("\n");
		remove(self);
		return;
	}*/

	SUB_UseTargets();

	if (self.lifespan)
	{
		thinktime self : self.lifespan;
		self.think = SUB_Remove;
	}
}

/*QUAKED puzzle_static_piece (1 .6 0) (-8 -8 -8) (8 8 8)
Puzzle Static Piece
-------------------------FIELDS-------------------------
puzzle_id: the name of the model to be created
lifespan: how long the puzzle piece should be around
--------------------------------------------------------
*/
void puzzle_static_piece(void)
{
	precache_puzzle_model(self.puzzle_id);
	setmodel(self, self.model);
	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_NONE;
	setsize (self, '0 0 0', '0 0 0');

	self.use = puzzle_static_use;
}

void reset_mangle (void)
{
	SUB_CalcAngleMove(self.mangle,10,SUB_Null);
}

void() control_return =
{	
	if(self.goalentity.classname!="catapult")
	{
		self.goalentity.oldthink=SUB_Null;
		self.goalentity.think=reset_mangle;
		thinktime self.goalentity : 0;
	}

	if(self.check_ok)
	{
//		other.weaponmodel.drawflags(-)DRF_TRANSLUCENT;
//		other.weaponmodel.abslight=0;
		self.enemy.oldweapon=0;
		self.enemy.th_weapon=W_SetCurrentAmmo;
		self.check_ok = FALSE;
		self.enemy=world;
	}
};

void() catapult_ready;
void() control_touch =
{
vector org, dir; 
float fire_range;
	if (other.classname != "player")
		return;

	if (other.beast_time>time)
		return;

	if (self.enemy != world && other != self.enemy)
		return;

	if(self.goalentity.health<=0&&self.health)
	{
		self.think=SUB_Remove;
		thinktime self : 0;
		return;
	}

	other.attack_finished=time+0.1;
	if(other.weaponmodel!="models/xhair.mdl");
	{
		other.weaponmodel="models/xhair.mdl";
		other.weaponframe = 0;
		other.th_weapon=SUB_Null;
		self.check_ok = TRUE;
	}

	if(self.enemy!=other)
		centerprint(other,"You're in control!\n");

	self.enemy = other;
	self.goalentity.enemy = self;

	makevectors(self.enemy.v_angle);
	if(self.goalentity.classname=="catapult")
	{
		if(self.enemy.angles_y<self.goalentity.angles_y+5&&self.enemy.angles_y>self.goalentity.angles_y - 5)
			self.goalentity.angles_y=self.enemy.angles_y;
		if(self.goalentity.think==catapult_ready)
		{
			if(self.enemy.button0)
			{
				self.goalentity.think=self.goalentity.th_weapon;
				thinktime self.goalentity : 0;
			}
		}
	}
	else
	{
		org=self.enemy.origin+self.enemy.proj_ofs;
		dir=normalize(v_forward);
		traceline(org,org+dir*10000,FALSE,self.enemy);
		org=self.goalentity.origin+self.goalentity.proj_ofs;

		fire_range=vlen(org-trace_endpos);
		if(fire_range>128)
		{
			dir=normalize(trace_endpos-org);
			if(trace_ent.health&&trace_ent.origin!='0 0 0')//Many breakable brishes have no origin
				self.goalentity.goalentity=trace_ent;
			else
				self.goalentity.goalentity=world;
			self.goalentity.view_ofs=trace_endpos;
			dir=vectoangles(dir);
			self.goalentity.angles=dir;
			self.goalentity.angles_z=dir_z/10;

			if(self.goalentity.think!=self.goalentity.th_weapon)
			{
				if(self.enemy.button0&&self.goalentity.th_weapon!=SUB_Null)
				{
//					self.goalentity.oldthink = control_return;
					self.goalentity.think=self.goalentity.th_weapon;
					thinktime self.goalentity : 0;
				}
//				else
//				{
//					self.goalentity.think = control_return;
//					thinktime self.goalentity : 0.1;
//				}
			}
		}
	}
	self.think = control_return;
	thinktime self : 0.5;
};

/*QUAKED trigger_control (.5 .5 .5) ?

Takes over a ballista when the player is inside of it
*/
void trigger_control_find_target (void)
{
	if (!self.target)
		objerror("Nothing to control!\n");

	self.goalentity = find(world, targetname, self.target);

	if(self.goalentity.takedamage)
		self.health=TRUE;

	if (!self.goalentity)
		objerror("Could not find target\n");
	else if(self.goalentity.classname=="catapult"||self.goalentity.classname=="obj_catapult2")
	{
		self.goalentity.movechain=self;
		self.flags(+)FL_MOVECHAIN_ANGLE;
		self.movetype=MOVETYPE_NOCLIP;
	}
	else
		self.goalentity.mangle = self.goalentity.angles;
}

void() trigger_control =
{
	self.enemy = world;
	self.touch = control_touch;
	self.ltime = time;
	InitTrigger();
	self.think=trigger_control_find_target;
	thinktime self : 1;
};

/*QUAK-ED trigger_no_friction (.5 .5 .5)
Takes FL_ONGROUND flag off anything
*/
/*
void trigger_no_fric_touch (void)
{
	other.flags(-)FL_ONGROUND;
}

void trigger_no_friction (void)
{
	InitTrigger();
	self.touch = trigger_no_fric_touch;
}
*/

/*QUAKED trigger_attack (.5 .5 .5) ?
Checks to see if a player touching it has tried to fire.
*/
void trigger_attack_touch (void)
{
	if(other.classname!="player")
		return;

	if(other.last_attack+0.3>=time)
	{
		SUB_UseTargets();
		remove(self);
	}
}

void trigger_attack (void) 
{
	InitTrigger();
	self.touch=trigger_attack_touch;
}

/*QUAKED trigger_message_transfer (.5 .5 .5) ?
Special case- will player it's message and transfer it's activating trigger's next target to it's target.
Does NOT activate it's target, only transfers the name to the activating trigger
These triggers also cannot be deactivated by touch
===================
FEILDS
.message = A message to display when used.
*/
void trigger_message_transfer_use ()
{
	string temp;

	temp = getstring(self.message);
	if (!deathmatch)
		centerprint(activator, temp);
	other.nexttarget=self.target;
}

void trigger_message_transfer ()
{
	InitTrigger();
	self.use=trigger_message_transfer_use;
}


float PUSH_SCALAR = 100;//push triggers' multiplier on dist from.
void push_trigger_touch ()
{
vector push_vec,save_vel;
entity oself;
float o_spd;
	if(!self.owner)
	{
		self.touch=SUB_Null;
		self.think=SUB_Remove;
		thinktime self: 0;
		return;
	}
	else if(self.owner.health<=0)
	{
		self.touch=SUB_Null;
		self.think=SUB_Remove;
		thinktime self: 0;
		return;
	}

	if(other.classname!="player")
		return;

	if(infront_of_ent(self.owner,other))
	{
		makevectors(other.angles);
		push_vec=v_forward;
	}
	else
		push_vec = normalize(self.owner.origin-other.origin);

	save_vel = other.velocity;
	o_spd=vhlen(save_vel);
	if(o_spd<PUSH_SCALAR)
		other.velocity = push_vec * PUSH_SCALAR;
	else
		other.velocity = push_vec * o_spd;

	oself = self;
	self=self.owner;
	if(self.touch!=SUB_Null)
		self.touch();
	self=oself;

	other.velocity = save_vel;
}

void push_trigger_think ()
{
	setorigin(self,self.owner.origin);
	self.think=push_trigger_think;
	thinktime self : 0.05;
}

void push_trigger_init()
{
vector add_mins,add_maxs;
	InitTrigger();
	add_mins_x=add_mins_y=self.level*-1;
	add_mins_z=2;
	add_maxs=add_maxs*-1;
	setsize(self,self.owner.mins+'-3 -3 1',self.owner.maxs+'3 3 -1');
	setorigin(self,self.owner.origin);
	self.touch=push_trigger_touch;
	self.think=push_trigger_think;
	thinktime self : 0.05;
}

void spawn_push_trigger(float extra)
{
	self.trigger_field=spawn();
	self.trigger_field.level=extra;
	self.trigger_field.owner=self;
	self.trigger_field.think=push_trigger_init;
	thinktime self.trigger_field : 0;
}

/*QUAKED trigger_start_timer (.5 .5 .5) ?
When activated, will set the timelimit (if there is one) to time + timelimit- this way any time lost getting ready to set up is saved.
*/
void start_timer ()
{
float timelimit;
string printnum;
entity found,oself;
	g_init_timelimit = cvar("timelimit");
	timelimit =g_init_timelimit * 60;
	if(timelimit)
	{
		dprintf("updated timelimit from %s to ",timelimit);
		timelimit+=time;
		printnum = ftos(timelimit/60);
		cvar_set("timelimit",printnum);
		dprintf("%s, time is: ",timelimit);
		dprintf("%s\n",time);
	}
	updateSiegeInfo();
	found = find(world,classname,"ambient_lightfader");//start fading lights
	if(found)
	{
		if(found.use!=SUB_Null)
		{
			oself=self;
			self=found;
			self.use();
			self=oself;
		}
	}

	//start mana gen
	found=find(world,classname,"player");
	while(found)
	{
		found.last_time=time;
		found=find(found,classname,"player");
	}
	gamestarted = TRUE;

	remove(self);
}

void trigger_start_timer ()
{
	self.use=start_timer;
}

void trigger_hardfall ()
{
//vector newmins,newmaxs,org;
/*	InitTrigger();

	org=(self.absmin+self.absmax)*0.5;
	newmins=self.absmin-org;
	newmaxs=self.absmax-org;
	newmins_x+=-128;
	newmins_y+=-128;
	newmaxs_x+=128;
	newmaxs_y+=128;
//	setsize(self,newmins,newmaxs);
	self.absmin=newmins+org;
	self.absmax=newmaxs+org;
*/
	if(!self.siege_team)
		self.siege_team=ST_DEFENDER;
	self.gravity=666;
	trigger_multiple();
}

void trigger_become_defender_touch ()
{
	if(other.classname!="player"||other.siege_team==ST_DEFENDER)
		return;
	other.siege_team=ST_DEFENDER;
	setsiegeteam(other,ST_DEFENDER);
	other.last_time = time;
	other.skin=0;
	other.health=other.max_health;
	other.bluemana=other.greenmana=0;
}

/*QUAKED trigger_become_defender (.5 .5 .5) ?
Makes a player choose the defender's side in siege
*/
void trigger_become_defender ()
{
	InitTrigger();
	self.touch=trigger_become_defender_touch;
}

void trigger_become_attacker_touch ()
{
	if(other.classname!="player"||other.siege_team==ST_ATTACKER)
		return;
	other.siege_team=ST_ATTACKER;
	setsiegeteam(other,ST_ATTACKER);
	other.last_time = time;
	other.health=other.max_health;
	other.skin=1;
	other.bluemana=other.greenmana=0;
}

/*QUAKED trigger_become_attacker (.5 .5 .5) ?
Makes a player choose the attacker's side in siege
*/
void trigger_become_attacker ()
{
	InitTrigger();
	self.touch=trigger_become_attacker_touch;
}

