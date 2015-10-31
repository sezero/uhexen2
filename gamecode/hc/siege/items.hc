/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/items.hc,v 1.2 2007-02-07 17:01:18 sezero Exp $
 */
void() W_SetCurrentAmmo;
void() W_SetCurrentWeapon;
void() ring_touch;
void()puzzle_touch;
void (entity spawnSpot, float persists) spawnNewDmToken;
/* ALL LIGHTS SHOULD BE 0 1 0 IN COLOR ALL OTHER ITEMS SHOULD
BE .8 .3 .4 IN COLOR */



void() SUB_regen =
{
	entity	checkGuy;

	if((((self.netname == "artifact")&&
		((self.artifact_name == STR_SUPERHEALTHBOOST)||
		(self.artifact_name == STR_MANABOOST)||
		(self.artifact_name == STR_TOME)||
		(self.artifact_name == STR_INVISIBILITY)||
		(self.artifact_name == STR_POLYMORPH)||
		(self.artifact_name == STR_RINGFLIGHT)||
		(self.artifact_name == STR_INVINCIBILITY)))||
		(self.classname == "wp_weapon4_head")||
		(self.classname == "wp_weapon4_staff"))&&(shyRespawn))
	{
		checkGuy = findradius(self.origin, 384);

		while(checkGuy)
		{
			if(checkGuy.classname == "player")
			{
				self.nextthink = time + 10;
				return;
			}

			checkGuy = checkGuy.chain;
		}
	}
	self.model = self.mdl;		// restore original model
	self.solid = SOLID_TRIGGER;	// allow it to be touched again
	sound (self, CHAN_VOICE, "items/itmspawn.wav", 1, ATTN_NORM);	// play respawn sound
	setorigin (self, self.origin);
};


void ItemHitFloorWait ()
{
//	dprint("Waiting to hit\n");
	if(self.flags&FL_ONGROUND||(pointcontents(self.origin-'0 0 38')==CONTENT_SOLID&&self.velocity_z<=0))
	{
		traceline(self.origin,self.origin-'0 0 38',TRUE,self);
		self.flags(+)FL_ITEM;		// make extra wide
		self.velocity='0 0 0';
		self.solid=SOLID_TRIGGER;
		if(self.touch==puzzle_touch)
		{
			setorigin(self,trace_endpos+'0 0 28');
			setsize (self, '-8 -8 -28', '8 8 8');
		}
		else 
		{
			setorigin(self,trace_endpos+'0 0 38');
			setsize (self, '-8 -8 -38', '8 8 24');
		}
		self.nextthink=-1;
		return;
	}
	else
		thinktime self : 0.05;
}

float getBackpackSize(entity item)
{
	float val;

	val = 0;

	val += (item.cnt_torch) +
	(item.cnt_h_boost) +
	(item.cnt_sh_boost)*3 +
	(item.cnt_mana_boost)*2 +
	(item.cnt_teleport) +
	(item.cnt_tome)*4 +
	(item.cnt_summon) +
	(item.cnt_invisibility)*3 +
	(item.cnt_glyph) +
	(item.cnt_haste) +
	(item.cnt_blast) +
	(item.cnt_polymorph)*2 +
	(item.cnt_flight)*2 +
	(item.cnt_cubeofforce)*2 +
	(item.cnt_invincibility)*10 +
	(item.bluemana)/15 +
	(item.greenmana)/12 +
	(item.spawn_health) +
	(item.armor_amulet)/4 +
	(item.armor_bracer)/4 +
	(item.armor_breastplate)/4 +
	(item.armor_helmet)/4;

	return val;
}


/*
============
PlaceItem

plants the object on the floor
============
*/
void() PlaceItem =
{
	float oldz;
	float oldHull;

	self.mdl = self.model;		// so it can be restored on respawn
	self.flags(+)FL_ITEM;		// make extra wide
	self.solid = SOLID_TRIGGER;
	self.movetype = MOVETYPE_TOSS;
	setsize (self, self.mins,self.maxs);
	self.velocity = '0 0 0';
	self.origin_z = self.origin_z + 6;
	oldz = self.origin_z;
	if(!self.spawnflags&FLOATING)
	{
		oldHull=self.hull;
		self.hull = HULL_POINT;
		if(!droptofloor())
		{
			if(self.touch==puzzle_touch)
			{
				dprintv("Puzzle piece tried to fall out of level at %s\n",self.origin); 
				setorigin(self,self.wallspot);
			}
			else
			{
				dprint ("Item :");
				dprint (self.classname);
				dprint (" fell out of level at ");
				dprint (vtos(self.origin));
				dprint ("\n");
				remove(self);
			}
			return;
		}
		self.hull=oldHull;
		if(self.touch==puzzle_touch)
		{
			setorigin(self,self.origin+'0 0 16');
			setsize (self, '-4 -4 -16', '4 4 0');
		}
		else if(self.netname=="Crown")
			setsize (self, '-4 -4 0', '4 4 8');
		else
		{
			setorigin(self,self.origin+'0 0 38');
			setsize (self, '-8 -8 -38', '8 8 24');
		}
	}
	else
		self.movetype = MOVETYPE_NONE;	
};

/*
============
StartItem

Sets the clipping size and plants the object on the floor
============
*/
void() StartItem =
{
	if (self.owner)  // Spawned by the backpack function
	{
		self.movetype = MOVETYPE_PUSHPULL;
		if(self.touch==puzzle_touch)
			setsize (self, '-4 -4 -16', '4 4 0');
		else if(self.netname=="Crown")
			setsize (self, '-4 -4 0', '4 4 8');
		else
			setsize (self, '-16 -16 -38', '16 16 24');
		if(self.think!=SUB_Remove&&self.owner.classname=="player"&&self.model!="models/bag.mdl")
		{
			self.think=SUB_Remove;
			thinktime self : 30;//Go away after 30 sec if thrown by player & not a backpack
		}
	}
	else
	{//key would go here
		self.nextthink = time + 0.2;	// items start after other solids
		self.think = PlaceItem;
	}
};

/*
//=========================================================================

//HEALTH BOX

//=========================================================================
//
// T_Heal: add health to an entity, limiting health to max_health
// "ignore" will ignore max_health limit
//
float (entity e, float healamount, float ignore) T_Heal =
{
	if (e.health <= 0)
		return 0;
	if ((!ignore) && (e.health >= other.max_health))
		return 0;
	healamount = ceil(healamount);

	e.health = e.health + healamount;
	if ((!ignore) && (e.health >= other.max_health))
		e.health = other.max_health;
		
	if (e.health > 250)
		e.health = 250;
	return 1;
};


//QUAK-ED item_health (.3 .3 1) (0 0 0) (32 32 32) rotten megahealth
//Health box. Normally gives 25 points. Rotten box heals 5-10 points,
//megahealth will add 100 health, then rot you down to your maximum health limit, one point per second.
//-------------------------FIELDS-------------------------

//--------------------------------------------------------


float	H_ROTTEN = 1;
float	H_MEGA = 2;
void() health_touch;
void() item_megahealth_rot;


//item_megahealth - Added by aleggett for use by the item spawner.
void item_megahealth()
{
	self.touch = health_touch;
//rj	setmodel(self, "maps/b_bh100.bsp");
	self.noise = "items/r_item2.wav";
	self.healamount = 100;
	self.healtype = 2;
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	StartItem ();
}

void() health_touch =
{
	local	float amount;
	local	string	s;
	
	if (other.classname != "player"||other.model=="models/sheep.mdl")
		return;
	
	if (self.healtype == 2) // Megahealth?  Ignore max_health...
	{
		if (other.health >= 250)
			return;
		if (!T_Heal(other, self.healamount, 1))
			return;
	}
	else
	{
		if (!T_Heal(other, self.healamount, 0))
			return;
	}
	
	sprint(other, "You receive ");
	s = ftos(self.healamount);
	sprint(other, s);
	sprint(other, " health\n");
	
// health touch sound
	sound(other, CHAN_ITEM, self.noise, 1, ATTN_NORM);

	stuffcmd (other, "bf\n");
	
	self.model = string_null;
	self.solid = SOLID_NOT;

	// Megahealth = rot down the player's super health
	if (self.healtype == 2)
	{
		other.items (+) IT_SUPERHEALTH;
		self.nextthink = time + 5;
		self.think = item_megahealth_rot;
		self.owner = other;
	}
	else
	{
		if (deathmatch != 2)		// deathmatch 2 is the silly old rules
		{
			if (deathmatch)
				self.nextthink = time + 20;
			self.think = SUB_regen;
		}
	}
	
	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets
};	

void() item_megahealth_rot =
{
	other = self.owner;
	
	if (other.health > other.max_health)
	{
		other.health = other.health - 1;
		self.nextthink = time + 1;
		return;
	}

// it is possible for a player to die and respawn between rots, so don't
// just blindly subtract the flag off
	other.items (-) IT_SUPERHEALTH;
	
	if (deathmatch == 1)	// deathmatch 2 is silly old rules
	{
		self.nextthink = time + 20;
		self.think = SUB_regen;
	}
};
*/

/*
===============================================================================

WEAPONS

===============================================================================
*/

float MAX_INV = 25;


void max_playermana (void)
{
	if (other.bluemana > other.max_mana)
		other.bluemana = other.max_mana;

	if (other.greenmana > other.max_mana)
		other.greenmana = other.max_mana;
}


float(float w) RankForWeapon =
{
	if (w&IT_WEAPON4)
		return 1;
	if (w == IT_WEAPON3)
		return 2;
	if (w == IT_WEAPON2)
		return 3;
	if (w == IT_WEAPON1)
		return 4;
	return 4;
};

/*
=============
Deathmatch_Weapon

Deathmatch weapon change rules for picking up a weapon

=============
*/
void(float old, float new) NewBestWeapon =
{
float or, nr;

// change self.weapon if desired
	or = RankForWeapon (self.weapon);
	nr = RankForWeapon (new);
	if ( nr < or )
	{
		self.oldweapon = self.weapon;//for deselection animation
		if(new&IT_WEAPON4)
			self.weapon=IT_WEAPON4;
		else
			self.weapon = new;
	}
};

void() W_BestWeapon;

/*
=============
weapon_touch
=============
*/
void weapon_touch (void)
{
	float	new, old;
	entity	stemp;
	float	leave,hadweap;
	float	scaleVal, numPlayers;

	if (!other.flags & FL_CLIENT)//||other.model=="models/sheep.mdl")
		return;

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}


	if (deathmatch == 2 || coop)
	{
		if(other.items&self.items)
			return;
		else
			leave = 1;
	}
	else
		leave = 0;


	new = self.items;
	// Give player weapon and mana
	if (self.classname=="wp_weapon2")
	{
		if (other.playerclass == CLASS_PALADIN)
		{
			self.artifact_name = STR_VORPAL;
			self.netname = "wp2";
		}
		else if (other.playerclass == CLASS_CRUSADER)
		{
			self.artifact_name = STR_ICESTAFF;
			self.netname = 	"wp2";
		}
		else if (other.playerclass == CLASS_NECROMANCER)
		{
			self.artifact_name = STR_MAGICMISSILE;
			self.netname = 	"wp2";
		}
		else if (other.playerclass == CLASS_ASSASSIN)
		{
			self.artifact_name = STR_CROSSBOW;
			self.netname = 	"wp2";
		}
		else if (other.playerclass == CLASS_SUCCUBUS)
		{
			self.artifact_name = STR_ACIDORB;
			self.netname = 	"wp2";
		}

		other.bluemana += 25;		
	}
	else if (self.classname=="wp_weapon3")
	{
		if (other.playerclass == CLASS_PALADIN)
		{
			self.artifact_name = STR_AXE;
			self.netname = "wp3";
		}
		else if (other.playerclass == CLASS_CRUSADER)
		{
			self.artifact_name = STR_METEORSTAFF;
			self.netname = 	"wp3";
		}
		else if (other.playerclass == CLASS_NECROMANCER)
		{
			self.artifact_name = STR_BONESHARD;
			self.netname = 	"wp3";
		}
		else if (other.playerclass == CLASS_ASSASSIN)
		{
			self.artifact_name = STR_GRENADES;
			self.netname = 	"wp3";
		}
		else if (other.playerclass == CLASS_SUCCUBUS)
		{
			self.artifact_name = STR_FLAMEORB;
			self.netname = 	"wp3";
		}

		other.greenmana += 25;		

	}
	else if (self.classname=="wp_weapon4_head")
	{
		if (other.playerclass == CLASS_PALADIN)
		{
			self.artifact_name = STR_PURIFIER1;
			self.netname = "wp4a";
		}
		else if (other.playerclass == CLASS_CRUSADER)
		{
			self.artifact_name = STR_SUN1;
			self.netname = 	"wp4a";
		}
		else if (other.playerclass == CLASS_NECROMANCER)
		{
			self.artifact_name = STR_RAVENSTAFF1;
			self.netname = 	"wp4a";
		}
		else if (other.playerclass == CLASS_ASSASSIN)
		{
			self.artifact_name = STR_SET1;
			self.netname = 	"wp4a";
		}
		else if (other.playerclass == CLASS_SUCCUBUS)
		{
			self.artifact_name = STR_LIGHTNING1;
			self.netname = 	"wp4a";
		}

		other.bluemana += 25;		
		other.greenmana += 25;	

		if(easyFourth != 0)
		{
			other.items (+) IT_WEAPON4_2;
		}
		if (other.items & IT_WEAPON4_2)
		   new (+) IT_WEAPON4;				

	}
	else if (self.classname=="wp_weapon4_staff")
	{
		if (other.playerclass == CLASS_PALADIN)
		{
			self.artifact_name = STR_PURIFIER2;
			self.netname = "wp4b";
		}
		else if (other.playerclass == CLASS_CRUSADER)
		{
			self.artifact_name = STR_SUN2;
			self.netname = 	"wp4b";
		}
		else if (other.playerclass == CLASS_NECROMANCER)
		{
			self.artifact_name = STR_RAVENSTAFF2;
			self.netname = 	"wp4b";
		}
		else if (other.playerclass == CLASS_ASSASSIN)
		{
			self.artifact_name = STR_SET2;
			self.netname = 	"wp4b";
		}
		else if (other.playerclass == CLASS_SUCCUBUS)
		{
			self.artifact_name = STR_LIGHTNING2;
			self.netname = 	"wp4b";
		}

		other.bluemana += 25;		
		other.greenmana += 25;	

		if(easyFourth != 0)
		{
			other.items (+) IT_WEAPON4_1;
		}
		if (other.items & IT_WEAPON4_1)
		   new (+) IT_WEAPON4;				

	}
	else
		objerror ("weapon_touch: unknown classname");

	sprinti (other, PRINT_MEDIUM, STR_YOUGOTTHE);
	sprinti (other, PRINT_MEDIUM, self.artifact_name);
	sprint (other, PRINT_MEDIUM, "\n");

	sound (other, CHAN_ITEM, "weapons/weappkup.wav", 1, ATTN_NORM);  // touch sound
	stuffcmd (other, "bf\n");

	max_playermana ();	// Check mana limits

// change to the weapon
	if(other.items&new)
		hadweap=TRUE;

	old = other.items;
	other.items (+) new;
	
	stemp = self;
	self = other;

	max_playermana();

	if(self.attack_finished<time)
	{//So you don't interrupt another selection or firing frame
		if(!deathmatch||!hadweap)	//In DM, don't switch to new weapon if already had it
		{

			NewBestWeapon (old, new);
		}

		W_SetCurrentWeapon();
	}

	self = stemp;

	if (leave)
		return;

// remove it in single player, or setup for respawning in deathmatch
	self.model = string_null;
	self.solid = SOLID_NOT;
	if (deathmatch == 1)
	{
		if(patternRunner)
		{
			scaleVal = 1.0;
		}
		else
		{
			numPlayers = countPlayers();

			scaleVal = 2.0 - (numPlayers * .125);
			if(scaleVal < .2)
			{
				scaleVal = .2;
			}
		}
		self.nextthink = time + 30*scaleVal;
	}
	self.think = SUB_regen;
	
	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets
}


/*
===============================================================================

POWERUPS

===============================================================================
*/

/*
void() powerup_touch;


void() powerup_touch =
{

	if ((other.health <= 0) || (other.classname != "player")||other.model=="models/sheep.mdl")
		return;

	sprint (other, PRINT_MEDIUM, "You got the ");
	sprint (other, PRINT_MEDIUM, self.netname);
	sprint (other, PRINT_MEDIUM, "\n");

	if (deathmatch)
	{
		self.mdl = self.model;
		
		if ((self.classname == "item_artifact_invulnerability") ||
			(self.classname == "item_artifact_invisibility"))
			self.nextthink = time + 60*5;
		else
			self.nextthink = time + 60;
		
		self.think = SUB_regen;
	}	

	sound (other, CHAN_VOICE, self.noise, 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.solid = SOLID_NOT;
	other.items (+) self.items;
	self.model = string_null;

// do the apropriate action	
	if (self.classname == "item_artifact_invulnerability")
	{
		other.invincible_time = 1;
		other.invincible_finished = time + 30;
	}
	
	if (self.classname == "item_artifact_invisibility")
	{
		other.invisible_time = 1;
		other.invisible_finished = time + 30;
	}

	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets
};
*/

void ihealth_touch(void)
{
	float	scaleVal, numPlayers;

	if ((other.classname!="player") || (other.health < 1))//||other.model=="models/sheep.mdl")
		return;

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}

	if (other.health < other.max_health)
	{
		sound (other, CHAN_VOICE, "items/itempkup.wav", 1, ATTN_NORM);
		other.health += 10;

		if (other.health > other.max_health)
			other.health = other.max_health;

		self.model = string_null;
		self.solid = SOLID_NOT;
		if (deathmatch == 1||dmMode==DM_SIEGE)
		{
			if(patternRunner)
			{
				scaleVal = 1.0;
			}
			else
			{
				numPlayers = countPlayers();

				scaleVal = 2.0 - (numPlayers * .125);
				if(scaleVal < .2)
				{
					scaleVal = .2;
				}
			}
			self.nextthink = time + RESPAWN_TIME*scaleVal;
		}
		self.think = SUB_regen;

		sprinti(other, PRINT_MEDIUM, STR_YOUHAVETHE);
		sprinti(other,PRINT_MEDIUM, self.artifact_name);
		sprint(other,PRINT_MEDIUM, "\n");

		activator = other;
		SUB_UseTargets();				// fire all targets / killtargets
	}
	if(other.flags2&FL2_POISONED)
	{
		other.flags2(-)FL2_POISONED;
		centerprint(other,"The poison has been cleansed from your blood...\n");
	}
}


void spawn_instant_health(void)
{
	self.touch = ihealth_touch;
	setmodel (self, "models/i_hboost.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.classname = "item_health";
	self.artifact_name = STR_INSTANTHEALTH;
	self.netname = "blue health";
	StartItem ();
}

/*QUAKED item_health (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING
Player is given 10 health instantly
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_health (void)
{
	spawn_instant_health();
}

void mana_touch(void)
{
	float	scaleVal, numPlayers;

	if ((other.classname!="player") || (other.health < 1))//||other.model=="models/sheep.mdl")
		return;

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}

	if (self.owner == other && self.artifact_ignore_owner_time > time)
		return;
	if (self.artifact_ignore_time > time) 
		return;

	if ((self.classname == "item_mana_green") && (other.greenmana >= other.max_mana))
		return;

	if ((self.classname == "item_mana_blue") && (other.bluemana >= other.max_mana))
		return;

	if ((self.classname == "item_mana_both") && (other.bluemana >= other.max_mana) && (other.greenmana >= other.max_mana))
		return;

	sprinti(other, PRINT_MEDIUM, STR_YOUHAVETHE);
	sprinti(other,PRINT_MEDIUM, self.artifact_name);
	sprint(other,PRINT_MEDIUM, "\n");

	sound (other, CHAN_VOICE, "items/itempkup.wav", 1, ATTN_NORM);
	stuffcmd (other, "bf\n");

	if (self.classname == "item_mana_green")
		other.greenmana += self.count;
	else if (self.classname == "item_mana_blue")
		other.bluemana += self.count;
	else 
	{
		other.greenmana += self.count;
		other.bluemana += self.count;
	}

	max_playermana();

	self.model = string_null;
	self.solid = SOLID_NOT;
	self.think = SUB_regen;

	activator = other;
	SUB_UseTargets();				// fire all targets / killtargets
	if (!self.owner && (deathmatch == 1||dmMode==DM_SIEGE))	//if it has an owner, it was dropped when he was killed so don't respawn
	{
		if(patternRunner)
		{
			scaleVal = 1.0;
		}
		else
		{
			numPlayers = countPlayers();

			scaleVal = 2.0 - (numPlayers * .125);
			if(scaleVal < .2)
			{
				scaleVal = .2;
			}
		}
		self.nextthink = time + RESPAWN_TIME*scaleVal;
	}
	else
		remove(self);
}

void spawn_item_mana_green(float amount)
{
	setmodel (self, "models/i_gmana.mdl");
	self.touch = mana_touch;
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.classname = "item_mana_green";
	self.artifact_name = STR_GREENMANA;
	self.netname = "gmana";
	self.count=amount;
	StartItem ();
}

/*QUAKED item_mana_green (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING BIG
Player is given 15 green mana instantly
BIG = 30 mana.
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void item_mana_green (void)
{
	if(self.spawnflags&2)
	{
		self.drawflags(+)SCALE_ORIGIN_CENTER|MLS_POWERMODE;
		self.scale=2;
		spawn_item_mana_green(30);
	}
	else
		spawn_item_mana_green(15);
}


void spawn_item_mana_blue(float amount)
{
	self.touch = mana_touch;
	setmodel (self, "models/i_bmana.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.classname = "item_mana_blue";
	self.count=amount;
	self.artifact_name = STR_BLUEMANA;
	self.netname = "bmana";
	StartItem ();
}

/*QUAKED item_mana_blue (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING BIG
Player is given 15 blue mana instantly
BIG = 30
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_mana_blue (void)
{
	if(self.spawnflags&2)
	{
		self.drawflags(+)SCALE_ORIGIN_CENTER|MLS_POWERMODE;
		self.scale=2;
		spawn_item_mana_blue(30);
	}
	else
		spawn_item_mana_blue(15);
}

void spawn_item_mana_both(float amount)
{
	self.touch = mana_touch;
	setmodel (self, "models/i_btmana.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.classname = "item_mana_both";
	self.count=amount;
	self.artifact_name = STR_COMBINEDMANA;
	self.netname = "cmana";
	StartItem ();
}

/*QUAKED item_mana_both (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING BIG
Player is given 15 green and 10 blue mana instantly
BIG = 30 each
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_mana_both (void)
{
	if(self.spawnflags&2)
	{
		self.drawflags(+)SCALE_ORIGIN_CENTER|MLS_POWERMODE;
		self.scale=2;
		spawn_item_mana_both(30);
	}
	else
		spawn_item_mana_both(15);
}



/*
===============================================================================

ARMOR

===============================================================================
*/

void armor_touch(void)
{
	float	scaleVal, numPlayers;

	if((other.classname != "player") || (other.health <= 0))//||other.model=="models/sheep.mdl")
	{
		return;
	}

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}

	if(self.classname == "item_armor_amulet")
	{
		other.armor_amulet = 20;
	}
	else if(self.classname == "item_armor_bracer")
	{
		other.armor_bracer = 20;
	}
	else if(self.classname == "item_armor_breastplate")
	{
		other.armor_breastplate = 20;
	}
	else if(self.classname == "item_armor_helmet")
	{
		other.armor_helmet = 20;
	}

	self.solid = SOLID_NOT;
	self.model = string_null;
	if(deathmatch == 1||dmMode==DM_SIEGE)
	{
		if(patternRunner)
		{
			scaleVal = 1.0;
		}
		else
		{
			numPlayers = countPlayers();

			scaleVal = 2.0 - (numPlayers * .125);
			if(scaleVal < .2)
			{
				scaleVal = .2;
			}
		}
		self.nextthink = time + RESPAWN_TIME*scaleVal;
	}
	self.think = SUB_regen;

	sprinti(other, PRINT_MEDIUM, STR_YOUHAVETHE);
	sprinti(other, PRINT_MEDIUM, self.artifact_name);
	sprint(other, PRINT_MEDIUM, "\n");

	sound(other, CHAN_ITEM, "items/armrpkup.wav", 1, ATTN_NORM);
	stuffcmd(other, "bf\n");

	activator = other;
	SUB_UseTargets();
}


void spawn_item_armor_helmet(void)
{
	setmodel (self, "models/i_helmet.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.touch = armor_touch;
	self.artifact_name = STR_ARMORHELMET;
	self.netname = "helm";

	StartItem ();
}

/*QUAKED item_armor_helmet (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_armor_helmet (void)
{
	spawn_item_armor_helmet();
}

void spawn_item_armor_breastplate (void)
{
	setmodel (self, "models/i_bplate.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.touch = armor_touch;
	self.artifact_name = STR_ARMORBREASTPLATE;
	self.netname = "brplate";

	StartItem ();
}

/*QUAKED item_armor_breastplate (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_armor_breastplate (void)
{
	spawn_item_armor_breastplate();
}

void spawn_item_armor_bracer(void)
{
	setmodel (self, "models/i_bracer.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.touch = armor_touch;
	self.artifact_name = STR_ARMORBRACER;
	self.netname = "armlet";

	StartItem ();
}

/*QUAKED item_armor_bracer (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_armor_bracer (void)
{
	spawn_item_armor_bracer();
}

void spawn_item_armor_amulet(void)
{
	setmodel (self, "models/i_amulet.mdl");
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.touch = armor_touch;
	self.netname = "amulet";
	self.artifact_name = STR_ARMORAMULET;

	StartItem ();
}

/*QUAKED item_armor_amulet (0 .5 .8) (-8 -8 -45) (8 8 20) FLOATING
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void item_armor_amulet (void)
{
	spawn_item_armor_amulet();
}


/*
===============================================================================

PLAYER BACKPACKS

===============================================================================
*/

void GetPuzzle2(entity item, entity person, string which);

void BackpackTouch(void)
{
	string	s;
	float	old, new;
	float	ItemCount;
	float	itemRoom;
	float	removeMe,old_cnt;
		
	if (other.classname != "player")//||other.model=="models/sheep.mdl")
		return;

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}

	if (other.health <= 0)
		return;
	if (self.owner == other && self.artifact_ignore_owner_time > time)
		return;
	if (self.artifact_ignore_time > time) 
		return;

	removeMe = TRUE;
	ItemCount = 0;

	if (self.cnt_torch > 0)
	{
		itemRoom = roomForItem(other,STR_TORCH);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_torch)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_torch;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_TORCH, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_TORCH);
			if (itemRoom > 1)	// Plural
				sprint(other,PRINT_MEDIUM, "es");
			self.cnt_torch = self.cnt_torch - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_h_boost > 0)
	{
		itemRoom = roomForItem(other,STR_HEALTHBOOST);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_h_boost)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_h_boost;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_HEALTHBOOST, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_HEALTHBOOST);
			if (itemRoom > 1)	// Plural
				sprint(other,PRINT_MEDIUM, "s");
			self.cnt_h_boost = self.cnt_h_boost - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_sh_boost > 0)
	{
		itemRoom = roomForItem(other,STR_SUPERHEALTHBOOST);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_sh_boost)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_sh_boost;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_SUPERHEALTHBOOST, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_SUPERHEALTHBOOST);
			if (itemRoom > 1)	// Plural
				sprint(other,PRINT_MEDIUM, "s");
			self.cnt_sh_boost = self.cnt_sh_boost - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_mana_boost > 0)
	{
		itemRoom = roomForItem(other,STR_MANABOOST);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_mana_boost)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_mana_boost;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_MANABOOST, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_MANABOOST);
			else
				sprint(other,PRINT_MEDIUM, "Kraters of Might");
			self.cnt_mana_boost = self.cnt_mana_boost - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_teleport > 0)
	{
		itemRoom = roomForItem(other,STR_TELEPORT);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_teleport)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_teleport;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_TELEPORT, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_TELEPORT);
			if (itemRoom > 1)	// Plural
				sprint(other,PRINT_MEDIUM, "s");
			self.cnt_teleport = self.cnt_teleport - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_tome > 0)
	{
		itemRoom = roomForItem(other,STR_TOME);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_tome)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_tome;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_TOME, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_TOME);
			else
				sprint(other,PRINT_MEDIUM, "Tomes of Power");
			self.cnt_tome = self.cnt_tome - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_summon > 0)
	{
		itemRoom = roomForItem(other,STR_SUMMON);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_summon)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_summon;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_SUMMON, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_SUMMON);
			else
				sprint(other,PRINT_MEDIUM, "Stones of Summoning");
			self.cnt_summon = self.cnt_summon - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_flight > 0)
	{
		itemRoom = roomForItem(other,STR_RINGFLIGHT);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_flight)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_flight;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_RINGFLIGHT, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_RINGFLIGHT);
			else
				sprint(other,PRINT_MEDIUM, "Rings of Flight");
			self.cnt_flight = self.cnt_flight - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_glyph > 0)
	{
		itemRoom = roomForItem(other,STR_GLYPH);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_glyph)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_glyph;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_GLYPH, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_GLYPH);
			else
				sprint(other,PRINT_MEDIUM, "Glyphs Of The Ancients");
			self.cnt_glyph = self.cnt_glyph - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}

	if (self.cnt_haste > 0)
	{
		itemRoom = roomForItem(other,STR_HASTE);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_haste)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_haste;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_HASTE, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_HASTE);
			self.cnt_haste = self.cnt_haste - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_blast > 0)
	{
		itemRoom = roomForItem(other,STR_BLAST);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_blast)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_blast;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_BLAST, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_BLAST);
			else
				sprint(other,PRINT_MEDIUM, "Discs of Repulsion");
			self.cnt_blast = self.cnt_blast - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_polymorph > 0)
	{
		itemRoom = roomForItem(other,STR_POLYMORPH);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_polymorph)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_polymorph;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_POLYMORPH, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_POLYMORPH);
			else
				sprint(other,PRINT_MEDIUM, "Seals of the Ovinomancer");
			self.cnt_polymorph = self.cnt_polymorph - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_invisibility > 0)
	{
		itemRoom = roomForItem(other,STR_INVISIBILITY);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_invisibility)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_invisibility;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_INVISIBILITY, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_INVISIBILITY);
			if (itemRoom > 1)
				sprint(other,PRINT_MEDIUM, "s");
			self.cnt_invisibility = self.cnt_invisibility - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_cubeofforce > 0)
	{
		itemRoom = roomForItem(other,STR_CUBEOFFORCE);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_cubeofforce)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_cubeofforce;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_CUBEOFFORCE, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			sprinti(other,PRINT_MEDIUM, STR_CUBEOFFORCE);
			if (itemRoom > 1)
				sprint(other,PRINT_MEDIUM, "s");
			self.cnt_cubeofforce = self.cnt_cubeofforce - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.cnt_invincibility > 0)
	{
		itemRoom = roomForItem(other,STR_INVINCIBILITY);

		if (itemRoom > 0)
		{
			if (itemRoom < self.cnt_invincibility)
			{
				removeMe = FALSE;
			}
			else
			{
				itemRoom = self.cnt_invincibility;
			}

			if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
			else sprint (other, PRINT_MEDIUM, "You get ");
			ItemCount += 1;

			adjustInventoryCount(other, STR_INVINCIBILITY, itemRoom);
			s = ftos(itemRoom);
			sprint(other,PRINT_MEDIUM, s);
			sprint(other,PRINT_MEDIUM, " ");
			if (itemRoom == 1)
				sprinti(other,PRINT_MEDIUM, STR_INVINCIBILITY);
			else
				sprint(other,PRINT_MEDIUM, "Icons of the Defender");
			self.cnt_invincibility = self.cnt_invincibility - itemRoom;
		}
		else
		{
			removeMe = FALSE;
		}
	}
	if (self.bluemana > 0)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.bluemana += self.bluemana;

		s = ftos(self.bluemana);
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_BLUEMANA);

		self.bluemana = 0;
	}
	if (self.greenmana > 0)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.greenmana += self.greenmana;

		s = ftos(self.greenmana);
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_GREENMANA);

		self.greenmana = 0;
	}
	if (self.cnt_arrows > 0&&other.cnt_arrows<100)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		old_cnt = other.cnt_arrows;
		other.cnt_arrows+= self.cnt_arrows;
		if(other.cnt_arrows>100)
		{
			self.cnt_arrows=other.cnt_arrows - 100;
			other.cnt_arrows = 100;
		}
		else
			self.cnt_arrows = 0;

		s = ftos(other.cnt_arrows-old_cnt);
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_ARROWS);
	}
	if (self.cnt_grenades > 0&&other.cnt_grenades<45)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		old_cnt = other.cnt_grenades;
		other.cnt_grenades+= self.cnt_grenades;
		if(other.cnt_grenades>100)
		{
			self.cnt_grenades=other.cnt_grenades - 100;
			other.cnt_grenades = 100;
		}
		else
			self.cnt_grenades = 0;

		s = ftos(other.cnt_grenades-old_cnt);
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_GRENADES);
	}
	
	if (self.armor_amulet)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.armor_amulet = self.armor_amulet;
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_ARMORAMULET);

		self.armor_amulet = 0;
	}

	if (self.armor_bracer)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.armor_bracer = self.armor_bracer;
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_ARMORBRACER);

		self.armor_bracer = 0;
	}

	if (self.armor_breastplate)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.armor_breastplate = self.armor_breastplate;
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_ARMORBREASTPLATE);

		self.armor_breastplate = 0;
	}

	if (self.armor_helmet)
	{
		if (ItemCount) sprint(other,PRINT_MEDIUM, ", ");
		else sprint (other, PRINT_MEDIUM, "You get ");
		ItemCount += 1;

		other.armor_helmet = self.armor_helmet;
		sprint(other,PRINT_MEDIUM, s);
		sprint(other,PRINT_MEDIUM, " ");
		sprinti(other,PRINT_MEDIUM, STR_ARMORHELMET);

		self.armor_helmet = 0;
	}

	if (!ItemCount)
	{
		if (removeMe)
		{
			sprint(other,PRINT_MEDIUM, "You get...Nothing!");
		}
		else
		{
			8;
//			sprint(other,PRINT_MEDIUM, "...Nothing you have room to carry!");
		}
	}
/*	else
	{
		self.scale = getBackpackSize(self) * 4.0 * 0.01;
		if(self.scale > 2.4)self.scale = 2.4;
		if(self.scale < .3)self.scale = .3;
	}*/

//	if the player was using his best weapon, change up to the new one if better

	new = self.items;
	if (!new)
		new = other.weapon;
	old = other.items;
	other.items (+) new;

//	change weapons

	if (ItemCount || removeMe)
	{
		sprint (other, PRINT_MEDIUM, "\n");

	//	backpack touch sound
		sound (other, CHAN_ITEM, "weapons/ammopkup.wav", 1, ATTN_NORM);
		stuffcmd (other, "bf\n");
	}

//	remove the backpack, change self to the player
	if (removeMe)
	{
		remove(self);
	}

	self = other;

//	change to the weapon
	if (!deathmatch)
		self.weapon = new;
	else
		NewBestWeapon (old, new);

	W_SetCurrentWeapon ();
}

void MonsterDropStuff(void)
{
	float chance;

	if(!self.flags&FL_MONSTER)
		return;

	if (self.monsterclass < CLASS_GRUNT)
		return;

	// Grunts drop only instant items
	if (self.monsterclass == CLASS_GRUNT)
	{
		if (random() < .15) // %15 chance he'll drop something	
		{
			chance = random();
			if (chance < .25)
				self.greenmana = 10;
			else if (chance < .50)
				self.bluemana = 10;
			else if (chance < .75)
			{
				self.greenmana = 10;
				self.bluemana = 10;
			}
			else
			{
				self.spawn_health = 1;
			}
		}
	}

	// Henchmen drop instant items or lesser artifacts
	else if (self.monsterclass == CLASS_HENCHMAN)
	{
		if (random() < .15) // %15 chance he'll drop something	
		{
			chance = random();

			if (chance < .08)
				self.greenmana = 10;
			else if (chance < .16)
				self.bluemana = 10;
			else if (chance < .24)
			{
				self.greenmana = 10;
				self.bluemana = 10;
			}
			else if (chance < .32)
			{
				self.spawn_health = 1;
			}
			else if (chance < .40)
				self.cnt_torch = 1;
			else if (chance < .48)
				self.cnt_h_boost = 1;
			else if (chance < .56)
				self.cnt_mana_boost = 1;
			else if (chance < .64)
				self.cnt_teleport = 1;
			else if (chance < .72)
				self.cnt_tome = 1;
			else if (chance < .80)
				self.cnt_haste = 1;
			else if (chance < .90)
				self.cnt_blast = 1;
		}		
	}
	// Leaders drop armor or artifacts
	else if (self.monsterclass == CLASS_LEADER)
	{		
		if (random() < .15) // %15 chance he'll drop something	
		{
			chance = random();
		
			 if (chance < .05)
				self.cnt_torch = 1;
			else if (chance < .10)
				self.cnt_h_boost = 1;
			else if (chance < .15)
				self.cnt_sh_boost = 1;
			else if (chance < .20)
				self.cnt_mana_boost = 1;
			else if (chance < .25)
				self.cnt_teleport = 1;
			else if (chance < .30)
				self.cnt_tome = 1;
			else if (chance < .35)
				self.cnt_summon = 1;
			else if (chance < .40)
				self.cnt_invisibility = 1;
			else if (chance < .45)
				self.cnt_glyph = 1;
			else if (chance < .50)
				self.cnt_haste = 1;
			else if (chance < .55)
				self.cnt_blast = 1;
			else if (chance < .60)
				self.cnt_polymorph = 1;
			else if (chance < .65)
				self.cnt_cubeofforce = 1;
			else if (chance < .70)
				self.cnt_invincibility = 1;
			else if (chance < .75)
				self.armor_amulet = 20;
			else if (chance < .80)
				self.armor_bracer = 20;
			else if (chance < .85)
				self.armor_breastplate = 20;
			else
				self.armor_helmet = 20;
		}
	}

	DropBackpack();
}



void spawn_weapon2 (void)
{
	entity newEnt;
	
	newEnt = spawn();

	CreateEntityNew(newEnt,ENT_WEAPON2_ART,"models/w_l2_c1.mdl",SUB_Null);
	newEnt.hull=HULL_POINT;
	newEnt.classname="wp_weapon2";

	newEnt.flags(+)FL_CLASS_DEPENDENT;
	newEnt.touch = weapon_touch;	
	newEnt.items=IT_WEAPON2;

	newEnt.movetype = MOVETYPE_PUSHPULL;
	setsize (newEnt, '-16 -16 -38', '16 16 24');

	newEnt.think=SUB_Remove;
	thinktime newEnt : 30;

	newEnt.origin = self.origin;
	newEnt.origin_z += 32;
	newEnt.velocity_x = random(-50, 50);
	newEnt.velocity_y = random(-50, 50);
	newEnt.velocity_z = 300;
	newEnt.flags(-)FL_ONGROUND;
}

void spawn_weapon3 (void)
{
	entity newEnt;

	newEnt = spawn();
	
	CreateEntityNew(newEnt,ENT_WEAPON3_ART,"models/w_l3_c1.mdl",SUB_Null);
	newEnt.hull=HULL_POINT;
	newEnt.classname="wp_weapon3";

	newEnt.flags(+)FL_CLASS_DEPENDENT;
	newEnt.touch = weapon_touch;	
	newEnt.items=IT_WEAPON3;

	newEnt.movetype = MOVETYPE_PUSHPULL;
	//setsize (newEnt, '-16 -16 -38', '16 16 24');
	setsize (newEnt, '-16 -16 -38', '16 16 24');

	newEnt.think=SUB_Remove;
	thinktime newEnt : 30;

	newEnt.origin = self.origin;
	newEnt.origin_z += 32;
	newEnt.velocity_x = random(-50, 50);
	newEnt.velocity_y = random(-50, 50);
	newEnt.velocity_z = 300;
	newEnt.flags(-)FL_ONGROUND;
	
}

void spawn_weapon4_head (void)
{
	entity newEnt;

	newEnt = spawn();
	
	CreateEntityNew(newEnt,ENT_WEAPON41_ART,"models/w_l41_c1.mdl",SUB_Null);
	newEnt.hull=HULL_POINT;
	newEnt.classname="wp_weapon4_head";

	newEnt.flags(+)FL_CLASS_DEPENDENT;
	newEnt.touch = weapon_touch;	
	newEnt.items=IT_WEAPON4_1;

	newEnt.movetype = MOVETYPE_PUSHPULL;
	setsize (newEnt, '-16 -16 -38', '16 16 24');

	newEnt.think=SUB_Remove;
	thinktime newEnt : 30;

	newEnt.origin = self.origin;
	newEnt.origin_z += 32;
	newEnt.velocity_x = random(-50, 50);
	newEnt.velocity_y = random(-50, 50);
	newEnt.velocity_z = 300;
	newEnt.flags(-)FL_ONGROUND;
}

void spawn_weapon4_staff (void)
{
	entity newEnt;

	newEnt = spawn();

	CreateEntityNew(newEnt,ENT_WEAPON42_ART,"models/w_l42_c1.mdl",SUB_Null);
	newEnt.hull=HULL_POINT;
	newEnt.classname="wp_weapon4_staff";

	newEnt.flags(+)FL_CLASS_DEPENDENT;
	newEnt.touch = weapon_touch;	
	newEnt.items=IT_WEAPON4_2;

	newEnt.movetype = MOVETYPE_PUSHPULL;
	setsize (newEnt, '-16 -16 -38', '16 16 24');

	newEnt.think=SUB_Remove;
	thinktime newEnt : 30;

	newEnt.origin = self.origin;
	newEnt.origin_z += 32;
	newEnt.velocity_x = random(-50, 50);
	newEnt.velocity_y = random(-50, 50);
	newEnt.velocity_z = 300;
	newEnt.flags(-)FL_ONGROUND;
}

/*
===============
DropBackpack
===============
*/
void DropBackpack(void)
{
	entity item;//,old_self;
	float total;
	float dropThreshhold;

/*	if(altRespawn)
	{
		dropThreshhold = .5;
	}
	else
	{
		dropThreshhold = -1;
	}*/

/*	if((dmMode == DM_CAPTURE_THE_TOKEN)&&(self.gameFlags & GF_HAS_TOKEN))
	{
		self.gameFlags (-) GF_HAS_TOKEN;
		//bprint(PRINT_MEDIUM, self.netname);
		//bprint(PRINT_MEDIUM, " dropped the Icon...\n");
		//bcenterprint(self.netname);
		//bcenterprint(" dropped the Icon...\n");
		bcenterprint2(self.netname, " dropped the Icon...\n");
		spawnNewDmToken(self, 0);
	}*/

	item = spawn();

	total = 0;



	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_torch > 3)
			total += item.cnt_torch = 3;
		else
			total += item.cnt_torch = self.cnt_torch;

		self.cnt_torch=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_h_boost > 3)
			total += item.cnt_h_boost = 3;
		else
			total += item.cnt_h_boost = self.cnt_h_boost;

		self.cnt_h_boost=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_sh_boost > 3)
			total += item.cnt_sh_boost = 3;
		else
			total += item.cnt_sh_boost = self.cnt_sh_boost;

		self.cnt_sh_boost=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_mana_boost > 3)
			total += item.cnt_mana_boost = 3;
		else
			total += item.cnt_mana_boost = self.cnt_mana_boost;

		self.cnt_mana_boost=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_teleport > 3)
			total += item.cnt_teleport = 3;
		else
			total += item.cnt_teleport = self.cnt_teleport;

		self.cnt_teleport=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		self.cnt_tome=0;
	}


	if(random(0,1) > dropThreshhold)
	{
	    self.cnt_summon=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		self.cnt_invisibility=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if(self.playerclass==CLASS_CRUSADER)
			self.cnt_glyph=rint(self.cnt_glyph/5);
		if (self.cnt_glyph > 3)
			total += item.cnt_glyph = 3;
		else
			total += item.cnt_glyph = self.cnt_glyph;

		self.cnt_glyph=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		self.cnt_haste=0;
	}


	if(random(0,1) > dropThreshhold)
	{
	    self.cnt_blast=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		self.cnt_polymorph=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		self.cnt_flight=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_cubeofforce > 3)
			total += item.cnt_cubeofforce = 3;
		else
			total += item.cnt_cubeofforce = self.cnt_cubeofforce;

		self.cnt_cubeofforce=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.cnt_invincibility > 3)
			total += item.cnt_invincibility = 3;
		else
			total += item.cnt_invincibility = self.cnt_invincibility;

		self.cnt_invincibility=0;
	}

	if(random(0,1) > dropThreshhold)
	{
		// Full armor on this body?
		if (self.armor_amulet==20)
		{
			total += 1;
			item.armor_amulet = self.armor_amulet;
		}
		self.armor_amulet=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.armor_bracer==20)
		{
			total += 1;
			item.armor_bracer = self.armor_bracer;
		}
		self.armor_bracer=0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.armor_breastplate==20)
		{
			total += 1;
			item.armor_breastplate = self.armor_breastplate;
		}
		self.armor_breastplate = 0;
	}


	if(random(0,1) > dropThreshhold)
	{
		if (self.armor_helmet==20)
		{
			total += 1;
			item.armor_helmet = self.armor_helmet;
		}
		self.armor_helmet = 0;
	}

/*	if (self.puzzle_inv1)
	{
		item.puzzle_inv1 = self.puzzle_inv1;
		total = 999;
	}
	if (self.puzzle_inv2)
	{
		item.puzzle_inv2 = self.puzzle_inv2;
		total = 999;
	}
	if (self.puzzle_inv3)
	{
		item.puzzle_inv3 = self.puzzle_inv3;
		total = 999;
	}
	if (self.puzzle_inv4)
	{
		item.puzzle_inv4 = self.puzzle_inv4;
		total = 999;
	}
	if (self.puzzle_inv5)
	{
		item.puzzle_inv5 = self.puzzle_inv5;
		total = 999;
	}
	if (self.puzzle_inv6)
	{
		item.puzzle_inv6 = self.puzzle_inv6;
		total = 999;
	}
	if (self.puzzle_inv7)
	{
		item.puzzle_inv7 = self.puzzle_inv7;
		total = 999;
	}
	if (self.puzzle_inv8)
	{
		item.puzzle_inv8 = self.puzzle_inv8;
		total = 999;
	}
*/

	// Any mana or instant health 	
/*	if(altRespawn)
	{
		self.bluemana *= .5;
		self.greenmana *= .5;
		self.spawn_health *= .5;

		item.bluemana = self.bluemana;
		item.greenmana = self.greenmana;
		item.spawn_health = self.spawn_health;

		// These will all currently respawn :[

		if(self.items & IT_WEAPON4)
		{
			self.items(-)IT_WEAPON4|IT_WEAPON4_1|IT_WEAPON4_2;
			
			if(altRespawn)
			{
				if(random(0,1) < .5)
				{
					spawn_weapon4_staff();
				}
				else
				{
					spawn_weapon4_head();
				}
			}

		}
		if(self.items & IT_WEAPON3)
		{
			if((random(0,1) > .5)||(self.weapon == IT_WEAPON3))
			{
				self.items(-)IT_WEAPON3;

				spawn_weapon3();
			}
		}
		if(self.items & IT_WEAPON2)
		{
			if((random(0,1) > .5)||(self.weapon == IT_WEAPON2))
			{
				self.items(-)IT_WEAPON2;

				spawn_weapon2();
			}
		}
	}
	else
	{*/
//		item.bluemana = self.bluemana;
//		item.greenmana = self.greenmana;
		item.cnt_arrows=self.cnt_arrows;
		item.cnt_grenades=self.cnt_grenades;
		item.spawn_health = self.spawn_health;

		self.cnt_arrows = 0;
		self.cnt_grenades = 0;
		self.bluemana=0;
		self.greenmana=0;
		self.spawn_health=0;
//	}

//	total = 1;
//	item.cnt_tome = 1;

	if (!total && !item.cnt_grenades&&!item.cnt_arrows&&!item.bluemana && !item.greenmana && !item.spawn_health) 
	{	// Nothing to put in the backpack
		remove(item);
		return;
	}
	else
		total=TRUE;

	setorigin(item,self.origin);
	item.origin = self.origin + '0 0 40';
	item.flags(+)FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	item.owner = self;
	item.artifact_ignore_owner_time = time + 2;
	item.artifact_ignore_time = time + 0.1;

//Always throw BP in Siege
/*	if ((total == 1 && !item.bluemana && !item.greenmana && !item.spawn_health) ||
	    (total == 0 &&  item.bluemana && !item.greenmana && !item.spawn_health)  ||
		(total == 0 && !item.bluemana &&  item.greenmana && !item.spawn_health)  ||
	    (total == 0 && !item.bluemana && !item.greenmana &&  item.spawn_health))
	{	// throw out the individual item
		item.velocity_z = 200;
//		item.velocity_x = random(-20,20);
//		item.velocity_y = random(-20,20);

		old_self = self;
		self = item;

		if (item.cnt_torch)
		{
			spawn_artifact(ARTIFACT_TORCH,NO_RESPAWN);
		}
		else if (item.cnt_h_boost)
		{
			spawn_artifact(ARTIFACT_HP_BOOST,NO_RESPAWN);
		}
		else if (item.cnt_sh_boost)
		{
			spawn_artifact(ARTIFACT_SUPER_HP_BOOST,NO_RESPAWN);
		}
		else if (item.cnt_mana_boost)
		{
			spawn_artifact(ARTIFACT_MANA_BOOST,NO_RESPAWN);
		}
		else if (item.cnt_teleport)
		{
			spawn_artifact(ARTIFACT_TELEPORT,NO_RESPAWN);
		}
		else if (item.cnt_tome)
		{
			spawn_artifact(ARTIFACT_TOME,NO_RESPAWN);
		}
		else if (item.cnt_summon)
		{
			spawn_artifact (ARTIFACT_SUMMON,NO_RESPAWN);
		}
		else if (item.cnt_invisibility)
		{
			spawn_artifact (ARTIFACT_INVISIBILITY,NO_RESPAWN);
		}
		else if (item.cnt_glyph)
		{
			spawn_artifact (ARTIFACT_GLYPH,NO_RESPAWN);
		}
		else if (item.cnt_haste)
		{
			spawn_artifact (ARTIFACT_HASTE,NO_RESPAWN);
		}
		else if (item.cnt_blast)
		{
			spawn_artifact(ARTIFACT_BLAST,NO_RESPAWN);
		}
		else if (item.cnt_polymorph)
		{
			spawn_artifact (ARTIFACT_POLYMORPH,NO_RESPAWN);
		}
		else if (item.cnt_flight)
		{
			spawn_artifact (ARTIFACT_FLIGHT,NO_RESPAWN);
		}
		else if (item.cnt_cubeofforce)
		{
			spawn_artifact (ARTIFACT_CUBEOFFORCE,NO_RESPAWN);
		}
		else if (item.cnt_invincibility)
		{
			spawn_artifact (ARTIFACT_INVINCIBILITY,NO_RESPAWN);
		}
		//can never happen
//		else if ((item.bluemana) && (item.greenmana))
//		{
//			spawn_item_mana_both(self.bluemana);
//		}
//
		else if (item.bluemana)
		{
			spawn_item_mana_blue(self.bluemana);
		}
		else if (item.greenmana)
		{
			spawn_item_mana_green(self.greenmana);
		}
		else if (item.spawn_health)
		{
			spawn_instant_health();
		}
		else if (item.armor_amulet)
		{
			spawn_item_armor_amulet();
		}
		else if (item.armor_bracer)
		{
			spawn_item_armor_bracer();
		}
		else if (item.armor_breastplate)
		{
			spawn_item_armor_breastplate();
		}
		else if (item.armor_helmet)
		{
			spawn_item_armor_helmet();
		}
		else
		{
			dprint("Bad backpack!");
			remove(item);
			self = old_self;
			return;
		}

		self = old_self;
	}
	else
	{*/
		item.velocity_z = 300;

		setmodel (item, "models/bag.mdl");
		setsize (item, '-16 -16 -45', '16 16 10');
		item.hull=HULL_POINT;
		item.touch = BackpackTouch;
	
		item.nextthink = time + 120;	// remove after 2 minutes
		item.think = SUB_Remove;

//		item.scale = getBackpackSize(item) * 4.0 * 0.01;
//		if(item.scale > 2.4)item.scale = 2.4;
//		if(item.scale < .15)item.scale = .15;

		if (!total)
		{
			remove(item);
			return;
		}
//	}
}

