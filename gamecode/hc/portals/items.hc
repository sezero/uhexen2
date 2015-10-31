/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/items.hc,v 1.2 2007-02-07 16:59:34 sezero Exp $
 */
void() W_SetCurrentAmmo;
void() W_SetCurrentWeapon;
void() ring_touch;
void()puzzle_touch;
/* ALL LIGHTS SHOULD BE 0 1 0 IN COLOR ALL OTHER ITEMS SHOULD
BE .8 .3 .4 IN COLOR */



void() SUB_regen =
{
	self.model = self.mdl;		// restore original model
	self.solid = SOLID_TRIGGER;	// allow it to be touched again
	sound (self, CHAN_VOICE, "items/itmspawn.wav", 1, ATTN_NORM);	// play respawn sound
	setorigin (self, self.origin);
};

/*
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
*/

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
			dprint ("Item :");
			dprint (self.classname);
			dprint (" fell out of level at ");
			dprint (vtos(self.origin));
			dprint ("\n");
			remove(self);
			return;
		}
		self.hull=oldHull;
		if(self.touch==puzzle_touch)
		{
			if(self.puzzle_id=="scept"&&mapname=="egypt5")
			{
				setorigin(self,self.origin+'0 0 28');
				setsize (self, '-1 -1 -28', '1 1 0');
			}
			else
			{
				setorigin(self,self.origin+'0 0 28');
				setsize (self, '-8 -8 -28', '8 8 8');
			}
		}
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
			setsize (self, '-8 -8 -28', '8 8 8');
		else 
			setsize (self, '-16 -16 -38', '16 16 24');
		if(self.think!=SUB_Remove&&self.owner.classname=="player"&&self.model!="models/bag.mdl")
		{
			self.think=SUB_Remove;
			thinktime self : 30;//Go away after 30 sec if thrown by player & not a backpack
		}
	}
	else
	{
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

void max_ammo2 (entity AddTo, entity AddFrom)
{
	// FIXME: I assume the max will be different between classes and levels

	if (AddTo.cnt_torch + AddFrom.cnt_torch > MAX_INV)
		AddFrom.cnt_torch = MAX_INV - AddTo.cnt_torch;
	if (AddTo.cnt_h_boost + AddFrom.cnt_h_boost > MAX_INV)
		AddFrom.cnt_h_boost = MAX_INV - AddTo.cnt_h_boost;
	if (AddTo.cnt_sh_boost + AddFrom.cnt_sh_boost > MAX_INV)
		AddFrom.cnt_sh_boost = MAX_INV - AddTo.cnt_sh_boost;
	if (AddTo.cnt_mana_boost + AddFrom.cnt_mana_boost > MAX_INV)
		AddFrom.cnt_mana_boost = MAX_INV - AddTo.cnt_mana_boost;
	if (AddTo.cnt_teleport + AddFrom.cnt_teleport > MAX_INV)
		AddFrom.cnt_teleport = MAX_INV - AddTo.cnt_teleport;
	if (AddTo.cnt_tome + AddFrom.cnt_tome > MAX_INV)
		AddFrom.cnt_tome = MAX_INV - AddTo.cnt_tome;
	if (AddTo.cnt_summon + AddFrom.cnt_summon > MAX_INV)
		AddFrom.cnt_summon = MAX_INV - AddTo.cnt_summon;
	if (AddTo.cnt_invisibility + AddFrom.cnt_invisibility > MAX_INV)
		AddFrom.cnt_invisibility = MAX_INV - AddTo.cnt_invisibility;
	if (AddTo.cnt_glyph + AddFrom.cnt_glyph > MAX_INV)
		AddFrom.cnt_glyph = MAX_INV - AddTo.cnt_glyph;
	if (AddTo.cnt_haste + AddFrom.cnt_haste > MAX_INV)
		AddFrom.cnt_haste = MAX_INV - AddTo.cnt_haste;
	if (AddTo.cnt_blast + AddFrom.cnt_blast > MAX_INV)
		AddFrom.cnt_blast = MAX_INV - AddTo.cnt_blast;
	if (AddTo.cnt_polymorph + AddFrom.cnt_polymorph > MAX_INV)
		AddFrom.cnt_polymorph = MAX_INV - AddTo.cnt_polymorph;
	if (AddTo.cnt_flight + AddFrom.cnt_flight > MAX_INV)
		AddFrom.cnt_flight = MAX_INV - AddTo.cnt_flight;
	if (AddTo.cnt_cubeofforce + AddFrom.cnt_cubeofforce > MAX_INV)
		AddFrom.cnt_cubeofforce = MAX_INV - AddTo.cnt_cubeofforce;
	if (AddTo.cnt_invincibility + AddFrom.cnt_invincibility > MAX_INV)
		AddFrom.cnt_invincibility = MAX_INV - AddTo.cnt_invincibility;

	if (AddTo.bluemana + AddFrom.bluemana > AddTo.max_mana)
		AddFrom.bluemana = AddTo.max_mana - AddTo.bluemana;

	if (AddTo.greenmana + AddFrom.greenmana > AddTo.max_mana)
		AddFrom.greenmana = AddTo.max_mana - AddTo.greenmana;
}

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
		if(new&IT_WEAPON4)
			self.weapon=IT_WEAPON4;
		else
			self.weapon = new;
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

	if (!other.flags & FL_CLIENT||other.model=="models/sheep.mdl")
		return;

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
		switch(other.playerclass)
		{
		case CLASS_PALADIN:
			self.netname = STR_VORPAL;
			break;
		case CLASS_CRUSADER:
			self.netname = 	STR_ICESTAFF;
			break;
		case CLASS_NECROMANCER:
			self.netname = 	STR_MAGICMISSILE;
			break;
		case CLASS_SUCCUBUS:
			self.netname = 	STR_ACIDORB;
			break;
		default:	//CLASS_ASSASSIN
			self.netname = 	STR_CROSSBOW;
			break;
		}

		other.bluemana += 25;		
	}
	else if (self.classname=="wp_weapon3")
	{
		switch(other.playerclass)
		{
		case CLASS_PALADIN:
			self.netname = STR_AXE;
			break;
		case CLASS_CRUSADER:
			self.netname = 	STR_METEORSTAFF;
			break;
		case CLASS_NECROMANCER:
			self.netname = 	STR_BONESHARD;
			break;
		case CLASS_SUCCUBUS:
			self.netname = 	STR_FLAMEORB;
			break;
		default:	//CLASS_ASSASSIN
			self.netname = 	STR_GRENADES;
			break;
		}
		other.greenmana += 25;		

	}
	else if (self.classname=="wp_weapon4_head")
	{
		switch(other.playerclass)
		{
		case CLASS_PALADIN:
			self.netname = STR_PURIFIER1;
			break;
		case CLASS_CRUSADER:
			self.netname = 	STR_SUN1;
			break;
		case CLASS_NECROMANCER:
			self.netname = 	STR_RAVENSTAFF1;
			break;
		case CLASS_SUCCUBUS:
			self.netname = 	STR_LIGHTNING1;
			break;
		default:	//CLASS_ASSASSIN
			self.netname = 	STR_SET1;
			break;
		}

		other.bluemana += 25;		
		other.greenmana += 25;	

		if (other.items & IT_WEAPON4_2)
		   new (+) IT_WEAPON4;				

	}
	else if (self.classname=="wp_weapon4_staff")
	{
		switch(other.playerclass)
		{
		case CLASS_PALADIN:
			self.netname = STR_PURIFIER2;
			break;
		case CLASS_CRUSADER:
			self.netname = 	STR_SUN2;
			break;
		case CLASS_NECROMANCER:
			self.netname = 	STR_RAVENSTAFF2;
			break;
		case CLASS_SUCCUBUS:
			self.netname = 	STR_LIGHTNING2;
			break;
		default:	//CLASS_ASSASSIN
			self.netname = 	STR_SET2;
			break;
		}

		other.bluemana += 25;		
		other.greenmana += 25;	

		if (other.items & IT_WEAPON4_1)
		   new (+) IT_WEAPON4;				

	}
	else
		objerror ("weapon_touch: unknown classname");

	sprint (other, STR_YOUGOTTHE);
	sprint (other, self.netname);
	sprint (other, "\n");

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
		self.oldweapon = self.weapon;
		if(!deathmatch||!hadweap)	//In DM, don't switch to new weapon if already had it
			NewBestWeapon (old, new);

		W_SetCurrentWeapon();
	}

	self = stemp;

	if (leave)
		return;

// remove it in single player, or setup for respawning in deathmatch
	self.model = string_null;
	self.solid = SOLID_NOT;
	if (deathmatch == 1)
		self.nextthink = time + 30;
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

	sprint (other, "You got the ");
	sprint (other, self.netname);
	sprint (other,"\n");

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

	if ((other.classname!="player") || (other.health < 1)||other.model=="models/sheep.mdl")
		return;

	if (other.health < other.max_health)
	{
		sound (other, CHAN_VOICE, "items/itempkup.wav", 1, ATTN_NORM);
		other.health += 10;

		if (other.health > other.max_health)
			other.health = other.max_health;

		self.model = string_null;
		self.solid = SOLID_NOT;
		if (deathmatch == 1)
			self.nextthink = time + RESPAWN_TIME;
		self.think = SUB_regen;

		sprint(other, STR_YOUHAVETHE);
		sprint(other,self.netname);
		sprint(other,"\n");

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
	self.netname = STR_INSTANTHEALTH;
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
	if ((other.classname!="player") || (other.health < 1)||other.model=="models/sheep.mdl")
		return;

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

	sprint(other, STR_YOUHAVETHE);
	sprint(other,self.netname);
	sprint(other,"\n");

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
	if (!self.owner && deathmatch == 1||world.target=="sheep")
		self.nextthink = time + RESPAWN_TIME;
	else
		remove(self);	//test this!
}

void spawn_item_mana_green(float amount)
{
	setmodel (self, "models/i_gmana.mdl");
	self.touch = mana_touch;
	setsize (self, '0 0 0', '0 0 0');
	self.hull=HULL_POINT;
	self.classname = "item_mana_green";
	self.netname = STR_GREENMANA;
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
	self.netname = STR_BLUEMANA;
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
	self.netname = STR_COMBINEDMANA;
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
	if((other.classname != "player") || (other.health <= 0)||other.model=="models/sheep.mdl")
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
	if(deathmatch == 1)
	{
		self.nextthink = time + RESPAWN_TIME;
	}
	self.think = SUB_regen;

	sprint(other, STR_YOUHAVETHE);
	sprint(other, self.netname);
	sprint(other, "\n");

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
	self.netname = STR_ARMORHELMET;

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
	self.netname = STR_ARMORBREASTPLATE;

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
	self.netname = STR_ARMORBRACER;

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
	self.netname = STR_ARMORAMULET;

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

//void GetPuzzle2(entity item, entity person, string which);

void BackpackTouch(void)
{
	string	s;
	float	old, new;
	float	ItemCount;
		
	if (other.classname != "player"||other.model=="models/sheep.mdl")
		return;
	if (other.health <= 0)
		return;
	if (self.owner == other && self.artifact_ignore_owner_time > time)
		return;
	if (self.artifact_ignore_time > time) 
		return;

	ItemCount = 0;
	sprint (other, "You get ");

	max_ammo2 (other,self);

	if (self.cnt_torch > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_torch += self.cnt_torch;

		s = ftos(self.cnt_torch);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_TORCH);
		if (self.cnt_torch > 1)	// Plural
			sprint(other,"es");
	}
	if (self.cnt_h_boost > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_h_boost += self.cnt_h_boost;

		s = ftos(self.cnt_h_boost);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_HEALTHBOOST);
		if (self.cnt_h_boost > 1)	// Plural
			sprint(other,"s");
	}
	if (self.cnt_sh_boost > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_sh_boost += self.cnt_sh_boost;

		s = ftos(self.cnt_sh_boost);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_SUPERHEALTHBOOST);
		if (self.cnt_sh_boost > 1)	// Plural
			sprint(other,"s");
	}
	if (self.cnt_mana_boost > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_mana_boost += self.cnt_mana_boost;

		s = ftos(self.cnt_mana_boost);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_mana_boost == 1)
			sprint(other,STR_MANABOOST);
		else
			sprint(other,"Kraters of Might");
	}
	if (self.cnt_teleport > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_teleport += self.cnt_teleport;

		s = ftos(self.cnt_teleport);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_TELEPORT);
		if (self.cnt_teleport > 1)	// Plural
			sprint(other,"s");
	}
	if (self.cnt_tome > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_tome += self.cnt_tome;

		s = ftos(self.cnt_tome);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_tome == 1)
			sprint(other,STR_TOME);
		else
			sprint(other,"Tomes of Power");
	}
	if (self.cnt_summon > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_summon += self.cnt_summon;

		s = ftos(self.cnt_summon);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_summon == 1)
			sprint(other,STR_SUMMON);
		else
			sprint(other,"Stones of Summoning");

	}
	if (self.cnt_flight > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_flight += self.cnt_flight;

		s = ftos(self.cnt_flight);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_flight == 1)
			sprint(other,STR_RINGFLIGHT);
		else
			sprint(other,"Rings of Flight");

	}
	if (self.cnt_glyph > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_glyph += self.cnt_glyph;

		s = ftos(self.cnt_glyph);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_glyph == 1)
			sprint(other,STR_GLYPH);
		else
			sprint(other,"Glyphs Of The Ancients");
	}

	if (self.cnt_haste > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_haste += self.cnt_haste;

		s = ftos(self.cnt_haste);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_HASTE);
	}
	if (self.cnt_blast > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_blast += self.cnt_blast;

		s = ftos(self.cnt_blast);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_blast == 1)
			sprint(other,STR_BLAST);
		else
			sprint(other,"Discs of Repulsion");
	}
	if (self.cnt_polymorph > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_polymorph += self.cnt_polymorph;

		s = ftos(self.cnt_polymorph);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_polymorph == 1)
			sprint(other,STR_POLYMORPH);
		else
			sprint(other,"Seals of the Ovinomancer");
	}
	if (self.cnt_invisibility > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_invisibility += self.cnt_invisibility;

		s = ftos(self.cnt_invisibility);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_INVISIBILITY);
		if (self.cnt_polymorph > 1)
			sprint(other,"s"); 
	}
	if (self.cnt_cubeofforce > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_cubeofforce += self.cnt_cubeofforce;

		s = ftos(self.cnt_cubeofforce);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_CUBEOFFORCE);
		if (self.cnt_cubeofforce > 1)
			sprint(other,"s"); 

	}
	if (self.cnt_invincibility > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.cnt_invincibility += self.cnt_invincibility;

		s = ftos(self.cnt_invincibility);
		sprint(other,s);
		sprint(other," ");
		if (self.cnt_invincibility == 1)
			sprint(other,STR_INVINCIBILITY);
		else
			sprint(other,"Icons of the Defender");
	}
	if (self.bluemana > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.bluemana += self.bluemana;

		s = ftos(self.bluemana);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_BLUEMANA);
	}
	if (self.greenmana > 0)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.greenmana += self.greenmana;

		s = ftos(self.greenmana);
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_GREENMANA);
	}
	
	if (self.armor_amulet)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.armor_amulet = self.armor_amulet;
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_ARMORAMULET);
	}

	if (self.armor_bracer)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.armor_bracer = self.armor_bracer;
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_ARMORBRACER);
	}

	if (self.armor_breastplate)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.armor_breastplate = self.armor_breastplate;
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_ARMORBREASTPLATE);
	}

	if (self.armor_helmet)
	{
		if (ItemCount) sprint(other,", ");
		ItemCount += 1;

		other.armor_helmet = self.armor_helmet;
		sprint(other,s);
		sprint(other," ");
		sprint(other,STR_ARMORHELMET);
	}

	if (!ItemCount) sprint(other,"...Nothing!");

/*	if (self.puzzle_inv1) GetPuzzle2(self, other, self.puzzle_inv1);
	if (self.puzzle_inv2) GetPuzzle2(self, other, self.puzzle_inv2);
	if (self.puzzle_inv3) GetPuzzle2(self, other, self.puzzle_inv3);
	if (self.puzzle_inv4) GetPuzzle2(self, other, self.puzzle_inv4);
	if (self.puzzle_inv5) GetPuzzle2(self, other, self.puzzle_inv5);
	if (self.puzzle_inv6) GetPuzzle2(self, other, self.puzzle_inv6);
	if (self.puzzle_inv7) GetPuzzle2(self, other, self.puzzle_inv7);
	if (self.puzzle_inv8) GetPuzzle2(self, other, self.puzzle_inv8);
*/

//	if the player was using his best weapon, change up to the new one if better

	new = self.items;
	if (!new)
		new = other.weapon;
	old = other.items;
	other.items (+) new;

//	change weapons
	sprint (other, "\n");

//	backpack touch sound
	sound (other, CHAN_ITEM, "weapons/ammopkup.wav", 1, ATTN_NORM);
	stuffcmd (other, "bf\n");

//	remove the backpack, change self to the player
	remove(self);
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

	if(!self.flags&FL_MONSTER)
		return;

	if (self.monsterclass < CLASS_GRUNT)
		return;

	DropBackpack();
}

float RandomMonsterGoodies ()
{
float chance;
float it_total;	

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
			it_total+=1;
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
			it_total+=1;
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
			else if (chance < .25&&!(world.spawnflags&MISSIONPACK))
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
			it_total+=1;
		}
	}
	return it_total;
}

/*
===============
DropBackpack
===============
*/
void DropBackpack(void)
{
entity item,old_self;
float total;

	item = spawn();

	if(self.playerclass==CLASS_CRUSADER)
		self.cnt_glyph=rint(self.cnt_glyph/5);
	total = 0;

	if (self.cnt_torch > 3)
		total += item.cnt_torch = 3;
	else
		total += item.cnt_torch = self.cnt_torch;

	if (self.cnt_h_boost > 3)
		total += item.cnt_h_boost = 3;
	else
		total += item.cnt_h_boost = self.cnt_h_boost;

	if (self.cnt_sh_boost > 3)
		total += item.cnt_sh_boost = 3;
	else
		total += item.cnt_sh_boost = self.cnt_sh_boost;

	if (self.cnt_mana_boost > 3)
		total += item.cnt_mana_boost = 3;
	else
		total += item.cnt_mana_boost = self.cnt_mana_boost;

	if (self.cnt_teleport > 3)
		total += item.cnt_teleport = 3;
	else
		total += item.cnt_teleport = self.cnt_teleport;

	if (self.cnt_tome > 3)
		total += item.cnt_tome = 3;
	else
		total += item.cnt_tome = self.cnt_tome;

	if (self.cnt_summon > 3)
		total += item.cnt_summon = 3;
	else
		total += item.cnt_summon = self.cnt_summon;

	if (self.cnt_invisibility > 3)
		total += item.cnt_invisibility = 3;
	else
		total += item.cnt_invisibility = self.cnt_invisibility;

	if (self.cnt_glyph > 3)
		total += item.cnt_glyph = 3;
	else
		total += item.cnt_glyph = self.cnt_glyph;

	if (self.cnt_haste > 3)
		total += item.cnt_haste = 3;
	else
		total += item.cnt_haste = self.cnt_haste;

	if (self.cnt_blast > 3)
		total += item.cnt_blast = 3;
	else
		total += item.cnt_blast = self.cnt_blast;

	if (self.cnt_polymorph > 3)
		total += item.cnt_polymorph = 3;
	else
		total += item.cnt_polymorph = self.cnt_polymorph;

	if (self.cnt_flight > 3)
		total += item.cnt_flight = 3;
	else
		total += item.cnt_flight = self.cnt_flight;

	if (self.cnt_cubeofforce > 3)
		total += item.cnt_cubeofforce = 3;
	else
		total += item.cnt_cubeofforce = self.cnt_cubeofforce;

	if (self.cnt_invincibility > 3)
		total += item.cnt_invincibility = 3;
	else
		total += item.cnt_invincibility = self.cnt_invincibility;

	// Full armor on this body?
	if (self.armor_amulet==20)
	{
		total += 1;
		item.armor_amulet = self.armor_amulet;
	}

	if (self.armor_bracer==20)
	{
		total += 1;
		item.armor_bracer = self.armor_bracer;
	}

	if (self.armor_breastplate==20)
	{
		total += 1;
		item.armor_breastplate = self.armor_breastplate;
	}

	if (self.armor_helmet==20)
	{
		total += 1;
		item.armor_helmet = self.armor_helmet;
	}

	// Any mana or instant health 	
	item.bluemana = self.bluemana;
	item.greenmana = self.greenmana;
	item.spawn_health = self.spawn_health;


	if (!total && !item.bluemana && !item.greenmana && !item.spawn_health) 
		if(self.classname!="player")
			total=RandomMonsterGoodies();

	if (!total && !item.bluemana && !item.greenmana && !item.spawn_health) 
	{	// Nothing to put in the backpack
		remove(item);
		return;
	}

	setorigin(item,self.origin);
	item.origin = self.origin + '0 0 40';
	item.flags(+)FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	item.owner = self;
	item.artifact_ignore_owner_time = time + 2;
	item.artifact_ignore_time = time + 0.1;

	if ((total == 1 && !item.bluemana && !item.greenmana && !item.spawn_health) ||
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
		//this could never happen
		/*else if ((item.bluemana) && (item.greenmana))
		{
			spawn_item_mana_both(self.bluemana);
		}
		*/
		//these items could respawn in dmatch!
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
	{
		item.velocity_z = 300;
//		item.velocity_x = random(-20,20);
//		item.velocity_y = random(-20,20);

		setmodel (item, "models/bag.mdl");
		setsize (item, '-16 -16 -45', '16 16 10');
		item.hull=HULL_POINT;
		item.touch = BackpackTouch;
	
		item.nextthink = time + 120;	// remove after 2 minutes
		item.think = SUB_Remove;

		if (!total)
		{
			remove(item);
			return;
		}
	}

	self.cnt_torch=0;
    self.cnt_h_boost=0;
    self.cnt_sh_boost=0;
    self.cnt_mana_boost=0;
    self.cnt_teleport=0;
    self.cnt_tome=0;
    self.cnt_summon=0;
    self.cnt_invisibility=0;
    self.cnt_glyph=0;
    self.cnt_haste=0;
    self.cnt_blast=0;
    self.cnt_polymorph=0;
    self.cnt_flight=0;
    self.cnt_cubeofforce=0;
    self.cnt_invincibility=0;

	self.armor_amulet=0;
	self.armor_bracer=0;
	self.armor_breastplate = 0;
	self.armor_helmet = 0;
	self.bluemana=0;
	self.greenmana=0;
	self.spawn_health=0;
}

