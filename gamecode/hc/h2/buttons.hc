/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/h2/buttons.hc,v 1.1.1.1 2004-11-29 11:41:17 sezero Exp $
 */
// button and multiple button

float SPAWNFLAG_BUTTON_ACTIVATE = 1;
float FIRE_MULTIPLE				= 4;

void() button_wait;
void() button_return;
void() pressure_use;
void() pressure_touch;

void() button_wait =
{
   self.state = STATE_TOP;
	if(self.wait==-1)
		if(!self.inactive)
			self.nextthink=-1;
		else
			self.nextthink=self.ltime+0.3;
	else
		self.nextthink = self.ltime + self.wait;
	self.think = button_return;
	activator = self.enemy;
	if (!self.inactive)
		SUB_UseTargets();
	self.frame = 1;			// use alternate textures
};

void() button_done =
{
	self.state = STATE_BOTTOM;
};

void() button_return =
{
	self.state = STATE_DOWN;
	SUB_CalcMove (self.pos1, self.speed, button_done);
	self.frame = 0;			// use normal textures
	if (self.health)
		self.takedamage = DAMAGE_NO_GRENADE;	// can be shot again
};

void() button_blocked =
{	// do nothing, just don't come all the way back out
};

void() button_fire =
{
	string s;

	if (self.inactive)
	{
		if (other.classname == "player" && self.msg2) 
		{
			s = getstring(self.msg2);
			centerprint(other, s);
		}
		return;	
	}

	if (self.state == STATE_UP)
		return;

	self.check_ok = TRUE;
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	
	self.state = STATE_UP;
	SUB_CalcMove (self.pos2, self.speed, button_wait);
};

void() button_use =
{
	self.enemy = activator;
	button_fire ();
};

void() button_touch =
{	
	if ((!other.flags&FL_PUSH)&&other.classname!="player")
		return;
	
//	if(self.inactive)
//		return;

	if (self.state == STATE_TOP) return;

	self.enemy = other;
	button_fire ();
};

void() button_killed =
{
	self.enemy = damage_attacker;
	self.health = self.max_health;
	self.takedamage = DAMAGE_NO;	// wil be reset upon return
	button_fire ();
};

/*QUAKED func_button (0 .5 .8) ? deactivated FIREONLY FIRE_MULTIPLE x x x
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again,
unless it's a pressure plate, in which case it will not return to it's position until it's not being touched anymore.
FIREONLY - has to be killed, touching won't do it.
FIRE_MULTIPLE - can be shot over and over (give it a high health)
-----------------------FIELDS-------------------------
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button can be killed and touched
"abslight" - to set the absolute light level
"soundtype"
0) steam metal
1) wooden clunk
2) metallic click
3) in-out

deactivated - button must be activated before it will work
--------------------------------------------------------
*/
void() func_button =
{
//	local float		gtemp, ftemp;

	if (self.soundtype == 0)
	{
		precache_sound ("buttons/button1.wav");
		self.noise = "buttons/button1.wav";
	}
	if (self.soundtype == 1)
	{
		precache_sound ("buttons/button2.wav");
		self.noise = "buttons/button2.wav";
	}
	if (self.soundtype == 2)
	{
		precache_sound ("buttons/button3.wav");
		self.noise = "buttons/button3.wav";
	}
	if (self.soundtype == 3)
	{
		precache_sound ("buttons/button4.wav");
		self.noise = "buttons/button4.wav";
	}
	
	SetMovedir ();

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;

	self.classname="button";
	self.movetype = MOVETYPE_PUSH;
	self.solid = SOLID_BSP;
	setmodel (self, self.model);

	self.blocked = button_blocked;
	self.use = button_use;

	if (self.health)
	{
		self.max_health = self.health;
		if(self.spawnflags&FIRE_MULTIPLE)
			self.th_pain = button_use;//for multiple uses
		self.th_die = button_killed;
		self.takedamage = DAMAGE_NO_GRENADE;
	}
	
	if (!self.spawnflags & 2)
	{
		if (!self.health) self.health = 10;
		self.touch = button_touch;
	}

	if (!self.speed)
		self.speed = 40;
	if (!self.wait)
		self.wait = 1;
	if (!self.lip)
		self.lip = 4;
	
	//If activatable, set usable flags off
	if(self.spawnflags&SPAWNFLAG_BUTTON_ACTIVATE)
	   self.inactive=TRUE;
	else self.inactive=FALSE;

	self.state = STATE_BOTTOM;

	self.pos1 = self.origin;
	self.pos2 = self.pos1 + self.movedir*(fabs(self.movedir*self.size) - self.lip);

	self.ltime = time;
};

/*
-------------------------------------------------------------------------
*/
float pressure_weight_check ()
{
vector org;
float len, totalmass;
entity head;
	org = (self.absmax + self.absmin)*0.5;
	len = vlen(self.absmax - self.absmin)*0.66;	

	head = findradius(org, len);

	while (head)
	{
		if(head!=self)
		{
			if(head.flags2&FL_ALIVE)
				totalmass += head.mass*10;
			else
				totalmass += head.mass;
/*			if(head.netname)
				dprint(head.netname);
			else
				dprint(head.classname);
			dprint(" weight added\n");
*/		}
		head = head.chain;
	}

/*	dprint("Measured mass: ");
	dprint(ftos(totalmass));
	dprint("\n");
	dprint("Required mass: ");
	dprint(ftos(self.mass));
	dprint("\n");
*/
	if (totalmass >= self.mass)
		return TRUE;
	else
		return FALSE;
}

float pressure_bounds_check ()
{
vector org1,org2,org3,org4, center,found_bottom;
float radius;
entity found;

	org1_z=org2_z=org3_z=org4_z=self.absmax_z+3;
	org1_x = self.absmin_x;
	org1_y = self.absmin_y;

	org2_x = self.absmin_x;
	org2_y = self.absmax_y;

	org3_x = self.absmax_x;
	org3_y = self.absmin_y;

	org4_x = self.absmax_x;
	org4_y = self.absmax_y;

	center=(self.absmax+self.absmin)*0.5;
	center_z=self.absmax_z;
	radius=fabs(self.absmax_x-center_x);

	found=findradius(center,radius);
	while(found)
	{
		found_bottom=(found.absmin+found.absmax)*0.5;
		found_bottom_z=found.absmin_z;
		if(found!=self)
			if(found_bottom_x>self.absmin_x&&found_bottom_x<self.absmax_x)
				if(found_bottom_y>self.absmin_y&&found_bottom_y<self.absmax_y)
					if(found_bottom_z>=self.absmax_z - 3&&found_bottom_z<=self.absmax_z+7)
						return TRUE;
					else
						dprint("Not right height\n");
				else
					dprint("Not right y\n");
			else
				dprint("Not right x\n");
		found=found.chain;
	}
	return FALSE;
}

void() pressure_wait =
{
float tripped;
	tripped=TRUE;
	if(!pressure_bounds_check())
		tripped=FALSE;
		
	if (!pressure_weight_check())
		tripped=FALSE;

	if(tripped)
	{
		self.check_ok = TRUE;
		self.nextthink = self.ltime + 0.05;
	}
	else
	{
		self.check_ok = FALSE;
		pressure_use();
		
		self.touch = pressure_touch;
		SUB_CalcMove(self.pos1, self.speed, SUB_Null);
	}
};

void() pressure_fire =
{
	sound (self, CHAN_VOICE, self.noise, 1, ATTN_NORM);

	self.state = STATE_UP;
	self.touch = SUB_Null;
	
	SUB_UseTargets();
	
	SUB_CalcMove (self.pos2, self.speed, pressure_wait);
};

void() pressure_use =
{
	self.enemy = activator;
	pressure_fire ();
};

void() pressure_touch =
{
	if(other==world)
		return;

entity found;
float inbounds,enough_weight;

	if(pressure_bounds_check())
		inbounds=TRUE;
		
	if (pressure_weight_check())
		enough_weight=TRUE;

	if(inbounds)
	{
		if(enough_weight)
		{
			self.check_ok = TRUE;
			self.touch = SUB_Null;
			pressure_use();
		}
		else
		{
			if(self.pain_finished<time&&other!=self.goalentity)
			if(other.classname=="player")
			{
				centerprint(other,"Looks like it needs more weight...\n");
				self.pain_finished=time+1;
			}
			else
			{
				found=findradius(other.origin,128);
				while(found)
				{
					if(found.classname=="player")
					{
						centerprint(found,"Looks like it needs more weight...\n");
						self.pain_finished=time+1;
						found=world;
					}
					else
						found=found.chain;
				}
			}
		}
	}
	self.goalentity=other;
};

void() pressure_blocked = 
{
float tripped;
	tripped=TRUE;
	if(!pressure_bounds_check())
		tripped=FALSE;
		
	if (!pressure_weight_check())
		tripped=FALSE;

	if(tripped)
	{
		self.check_ok = TRUE;
		self.touch = SUB_Null;
		pressure_use();
		return;
	}
};

/*QUAKED func_pressure (0 .5 .8) ? ACTIVATE
-----------------------FIELDS-------------------------
"mass"		amount of mass a plate must have on it to fire-this can be cumulative so you have to put more than one thing on pressure plate to work (default 0)
NOTE:
Player mass ~= 60-80
Normal barrel mass = 75
Normal barrel mass = 85
Indestructible barrel mass = 95
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"soundtype"
0) steam metal
1) wooden clunk
2) metallic click
3) in-out
--------------------------------------------------------
*/
void func_pressure (void)
{
//local float		gtemp, ftemp;

	if (self.soundtype == 0)
	{
		precache_sound ("buttons/airbut1.wav");
		self.noise = "buttons/airbut1.wav";
	}
	if (self.soundtype == 1)
	{
		precache_sound ("buttons/switch21.wav");
		self.noise = "buttons/switch21.wav";
	}
	if (self.soundtype == 2)
	{
		precache_sound ("buttons/switch02.wav");
		self.noise = "buttons/switch02.wav";
	}
	if (self.soundtype == 3)
	{
		precache_sound ("buttons/switch04.wav");
		self.noise = "buttons/switch04.wav";
	}
	
	SetMovedir ();

	self.movetype = MOVETYPE_PUSH;
	self.solid = SOLID_BSP;
	setmodel (self, self.model);

	self.blocked = pressure_blocked;
	self.use = pressure_use;
	self.touch = pressure_touch;
	self.ltime = time;

	if (self.health)
	{
		self.max_health = self.health;
		self.th_die = button_killed;
		self.takedamage = DAMAGE_NO_GRENADE;
	}
	else
		self.touch = pressure_touch;

	if (!self.speed)
		self.speed = 40;
	if (!self.wait)
		self.wait = 1;
	if (!self.lip)
		self.lip = 4;

	//If activatable, set usable flags off
	if(self.spawnflags&SPAWNFLAG_BUTTON_ACTIVATE)
		self.inactive=TRUE;
//	else
//		self.inactive=FALSE;

	self.state = STATE_BOTTOM;

	self.pos1 = self.origin;
	self.pos2 = self.pos1 + self.movedir*(fabs(self.movedir*self.size) - self.lip);
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 55    8/13/97 5:46p Mgummelt
 * 
 * 54    7/23/97 7:04p Mgummelt
 * 
 * 53    7/21/97 4:03p Mgummelt
 * 
 * 52    7/21/97 4:02p Mgummelt
 * 
 * 51    7/17/97 2:17p Mgummelt
 * 
 * 50    7/14/97 4:42p Mgummelt
 * 
 * 49    7/14/97 2:39p Rlove
 * 
 * 48    7/11/97 4:37p Mgummelt
 * 
 * 47    7/10/97 7:02p Mgummelt
 * 
 * 46    7/09/97 3:54p Rjohnson
 * Fix for messages
 * 
 * 45    7/09/97 12:03p Rjohnson
 * Added msg2 to the global text file
 * 
 * 44    7/03/97 12:47p Mgummelt
 * 
 * 43    6/25/97 9:23p Mgummelt
 * 
 * 42    6/18/97 4:00p Mgummelt
 * 
 * 41    6/15/97 5:10p Mgummelt
 * 
 * 40    6/11/97 4:06p Mgummelt
 * 
 * 39    6/06/97 4:08p Mgummelt
 * 
 * 38    6/06/97 10:58a Rjohnson
 * Fix for lights
 * 
 * 37    6/05/97 8:16p Mgummelt
 * 
 * 36    6/04/97 1:04p Jweier
 * 
 * 35    6/01/97 5:09a Mgummelt
 * 
 * 34    5/30/97 5:03p Rjohnson
 * Added abslight to buttons
 * 
 * 33    5/27/97 6:42p Mgummelt
 * 
 * 32    5/23/97 5:03p Jweier
 * 
 * 31    5/22/97 3:30p Mgummelt
 * 
 * 30    5/20/97 6:21p Jweier
 * 
 * 29    5/20/97 6:15p Jweier
 * 
 * 28    5/15/97 6:31p Jweier
 * 
 * 27    5/15/97 6:27p Jweier
 * 
 * 26    5/11/97 7:30a Mgummelt
 * 
 * 25    5/09/97 5:54p Jweier
 * 
 * 24    5/07/97 4:11p Jweier
 * 
 * 23    5/02/97 8:06p Mgummelt
 * 
 * 22    5/02/97 6:17p Jweier
 * 
 * 21    5/01/97 6:50p Mgummelt
 * 
 * 20    5/01/97 5:46p Jweier
 * 
 * 19    4/30/97 6:36p Jweier
 * 
 * 18    4/30/97 6:34p Jweier
 * 
 * 17    4/30/97 5:36p Jweier
 * 
 * 16    4/26/97 12:56p Mgummelt
 * 
 * 15    4/26/97 12:52p Mgummelt
 * 
 * 14    4/26/97 12:51p Jweier
 * 
 * 13    4/15/97 7:08p Jweier
 * activate/deactivate
 * 
 * 12    4/05/97 5:45p Mgummelt
 * 
 * 11    4/05/97 4:00p Mgummelt
 * 
 * 10    4/04/97 8:08p Mgummelt
 * 
 * 9     3/22/97 4:47p Jweier
 * 
 * 8     3/21/97 9:38a Rlove
 * Created CHUNK.HC and MATH.HC, moved brush_die to chunk_death so others
 * can use it.
 * 
 * 7     3/18/97 6:37p Jweier
 * Button needs to be fixed to handle PUSH_TOUCH
 * 
 * 6     3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 5     11/21/96 8:34a Rlove
 * Corrected the button name to FUNC_BUTTON
 * 
 * 4     11/20/96 4:43p Rlove
 * Corrected func_door naming problem
 * 
 * 3     11/18/96 3:29p Rlove
 * Changed sounds variable to soundtype
 * 
 * 2     11/11/96 1:12p Rlove
 * Added Source Safe stuff
 */
