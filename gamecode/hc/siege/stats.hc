/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/stats.hc,v 1.1 2005-01-26 17:26:13 sezero Exp $
 */

//  min health, max health,
//  min health per level up to level 10,  min health per level up to level 10, 
//  health per level past level 10
/*
float hitpoint_table[25] =
{
	70,		85,				// Paladin
	8,		13,      4,

	65,		75,				// Crusader
	5,		10,      3,

	65,		75,				// Necromancer
	5,		10,      3,

	65,		75,				// Assassin
	5,		10,      3,

	65,		75,				// Succubus
	5,		10,      3,

	90,		100,			// Dwarf
	10,		15,      5
};
*/
float mana_table[30] =
{
//    Startup    Per Level     Past
//  min    max    min		max     10th Level
	0,		0,		0,		0, 		0,		// Paladin
	100,	100,	0,		0,	 	0, 		// Crusader
    200,	200,	0,		0, 		0,     // Necromancer
	0,		0,		0,		0,	 	0,		// Assassin
	100,	100,	0,		0,	 	0,		// Succubus
	0,		0,		0,		0, 		0		// Dwarf
};


float strength_table[12] =
{
	17,		21,		// Paladin
	11,		14,		// Crusader
	6,		10,		// Necromancer
	10,		13,		// Assassin
	12,		15,		// Succubus
	15,		20		// Dwarf
};

float intelligence_table[12] =
{
	6,		10,		// Paladin
	10,		13,		// Crusader
	15,		18,		// Necromancer
	6,		10,		// Assassin
	9,		13,		// Succubus
	8,		14		// Dwarf
};

float wisdom_table[12] =
{
	6,		10,		// Paladin
	15,		18,		// Crusader
	10,		13,		// Necromancer
	12,		15,		// Assassin
	11,		14,		// Succubus
	14,		18		// Dwarf
};

float dexterity_table[12] =
{
	10,		13,		// Paladin
	6,		10,		// Crusader
	8,		12,		// Necromancer
	15,		18,		// Assassin
	9,		13,		// Succubus
	8,		12		// Dwarf
};

void PlayerSpeed_Calc (entity who)
{
	if(who.beast_time>time)
	{
		who.hasted = 1.666;
		return;
	}

	switch (who.playerclass)
	{
	case CLASS_ASSASSIN:
		who.hasted=1;
	break;
	case CLASS_SUCCUBUS:
		who.hasted=.95;
	break;
	case CLASS_PALADIN:
		who.hasted=.9;
	break;
	case CLASS_CRUSADER:
		who.hasted=.85;
	break;
	case CLASS_NECROMANCER:
		who.hasted=.8;
	break;
	case CLASS_DWARF:
		who.hasted=1;
	break;
	}

	if (who.artifact_active & ART_HASTE)
		who.hasted *= 2.5;
	else if(who.flags2&FL2_EXCALIBUR)
		who.hasted *=1.8;


	if (who.hull==HULL_CROUCH)   // Player crouched
	{
		if(who.playerclass!=CLASS_DWARF)
			who.hasted *= .6;
		else
		{
			if(who.super_damage)
				who.hasted *= (.7 + who.super_damage/10);
			else	
				who.hasted *= .7;
		}
		if(who.sheep_time>time)
			who.hasted *= 2;//sheep were too slow
	}

	if (who.flags2&FL2_POISONED)   // Player poisoned
		who.hasted *= .8;

	if(who.siege_team==ST_DEFENDER)
		if(who.puzzle_inv1==g_keyname)//key carrier a bit slower
			who.hasted *= .8;

//	if(who.waterlevel==3)//swimming mod
//		who.hasted*=player_swim_mod[who.playerclass - 1];
}
/*
float CLASS_PALADIN					= 1;
float CLASS_CRUSADER				= 2;
float CLASS_NECROMANCER				= 3;
float CLASS_ASSASSIN				= 4;
float CLASS_SUCCUBUS				= 5;
*/

// Make sure we get a real distribution beteen
// min-max, otherwise, max will only get choosen when
// random() returns 1.0
float stats_compute(float min, float max)
{
	float value;

	value = (max-min+1)*random() + min;
	if (value > max) value = max;

	value = ceil(value);

	return value;
}

void stats_NewPlayer(entity e)
{
	float index;

	// Stats already set?
	if (e.strength) return;

	if (e.playerclass < CLASS_PALADIN || e.playerclass > CLASS_DWARF)
	{
		sprint(e,PRINT_MEDIUM, "Invalid player class ");
		sprint(e,PRINT_MEDIUM, ftos(e.playerclass));
		sprint(e,PRINT_MEDIUM, "\n");
		return;
	}

	// Calc initial health
	index = (e.playerclass - 1) * 5;
	e.health = stats_compute(hitpoint_table[index],
							 hitpoint_table[index+1]);
	e.max_health = e.health;

	// Calc initial mana
	index = (e.playerclass - 1) * 5;
	e.max_mana = stats_compute(mana_table[index],
							 mana_table[index+1]);


	index = (e.playerclass - 1) * 2;
	e.strength = stats_compute(strength_table[index],
							   strength_table[index+1]);
	e.intelligence = stats_compute(intelligence_table[index],
								   intelligence_table[index+1]);
	e.wisdom = stats_compute(wisdom_table[index],
							 wisdom_table[index+1]);
	e.dexterity = stats_compute(dexterity_table[index],
								dexterity_table[index+1]);

	e.level = 1;
//	e.experience = 0;
}


/*
================
PlayerAdvanceLevel

This routine is called (from the game C side) when a player is advanced a level
(self.level)
================
*/
void PlayerAdvanceLevel(float NewLevel)
{
	string s2;
	float OldLevel,Diff;
	float index,HealthInc,ManaInc;

	sound (self, CHAN_VOICE, "misc/comm.wav", 1, ATTN_NONE);

	OldLevel = self.level;
	self.level = NewLevel;
	Diff = self.level - OldLevel;

	if(!Diff)
	{
		return;
	}

	sprint(self,PRINT_MEDIUM, "You are now level ");
	s2 = ftos(self.level);
	sprint(self,PRINT_MEDIUM, s2);
	sprint(self,PRINT_MEDIUM, "!\n");

	if(!self.newclass)
	{
/*		if (self.playerclass == CLASS_PALADIN)

		{
		   sprint(self,PRINT_MEDIUM, "Paladin gained a level\n");
		}
		else if (self.playerclass == CLASS_CRUSADER)
		{
		   sprint(self,PRINT_MEDIUM, "Crusader gained a level\n");

			// Special ability #1, full mana at level advancement
			self.bluemana = self.greenmana = self.max_mana;

		}
		else if (self.playerclass == CLASS_NECROMANCER)
		{
		   sprint(self,PRINT_MEDIUM, "Necromancer gained a level\n");
		}
		else if (self.playerclass == CLASS_ASSASSIN)
		{
		   sprint(self,PRINT_MEDIUM, "Assassin gained a level\n");

		}
		else if (self.playerclass == CLASS_SUCCUBUS)
		{
			sprint(self,PRINT_MEDIUM,"Demoness gained a level\n");
		}
*/
		switch (self.playerclass)
		{
		case CLASS_DWARF:
		   centerprint(self, "Dwarf gained a level\n");
		break;
		case CLASS_PALADIN:
		   centerprint(self, "Paladin gained a level\n");
		break;
		case CLASS_CRUSADER:
			centerprint(self,"Crusader gained a level\n");
			// Special ability #1, full mana at level advancement
			//self.bluemana = self.greenmana = self.max_mana;
		break;
		case CLASS_NECROMANCER:
		   centerprint(self,"Necromancer gained a level\n");
		break;
		case CLASS_ASSASSIN:
		   centerprint(self,"Assassin gained a level\n");
		break;
		case CLASS_SUCCUBUS:
		   centerprint(self,"Demoness gained a level\n");
		break;
		}

	}

	if (self.playerclass < CLASS_PALADIN ||
		self.playerclass > MAX_CLASS)
		return;

	index = (self.playerclass - 1) * 5;

	// Have to do it this way in case they go up more than 1 level at a time
	while(Diff > 0)
	{
		OldLevel += 1;
		Diff -= 1;

		if (OldLevel <= MAX_LEVELS)
		{
			HealthInc = stats_compute(hitpoint_table[index+2],hitpoint_table[index+3]);
			ManaInc = stats_compute(mana_table[index+2],mana_table[index+3]);
		}
		else
		{
			HealthInc = hitpoint_table[index+4];
			ManaInc = mana_table[index+4];
		}
		self.health += HealthInc;
		self.max_health += HealthInc;

		//	An upper limit of 150 on health
		if (self.health > 150) 
			self.health = 150;

		if (self.max_health > 150)
			self.max_health = 150;


//		self.greenmana += ManaInc;
//		self.bluemana += ManaInc;
		self.max_mana += ManaInc;
		
		if(!deathmatch)
		{
			sprint(self, PRINT_LOW,"Stats: MP +");
			s2 = ftos(ManaInc);
			sprint(self, PRINT_LOW, s2);

			sprint(self, PRINT_LOW, "  HP +");
			s2 = ftos(HealthInc);
			sprint(self, PRINT_LOW, s2);
			sprint(self, PRINT_LOW, "\n");
		}
	}

	if (self.level > 2)
		self.flags(+)FL_SPECIAL_ABILITY1;

	if (self.level >5)
		self.flags(+)FL_SPECIAL_ABILITY2;

}


/*
======================================
void stats_NewClass(entity e)
MG
Used when doing a quick changeclass
======================================
*/
void stats_NewClass(entity e)
{
entity oself;
float index,newlevel;

	if (e.playerclass < CLASS_PALADIN || e.playerclass > MAX_CLASS)
	{
		sprint(e,PRINT_MEDIUM, "Invalid player class ");
		sprint(e,PRINT_MEDIUM, ftos(e.playerclass));
		sprint(e,PRINT_MEDIUM, "\n");
		return;
	}

	// Calc initial health
	index = (e.playerclass - 1) * 5;
	e.health = stats_compute(hitpoint_table[index],
							 hitpoint_table[index+1]);
	e.max_health = e.health;

	// Calc initial mana
	index = (e.playerclass - 1) * 5;
	e.max_mana = stats_compute(mana_table[index],
							 mana_table[index+1]);

	index = (e.playerclass - 1) * 2;
	e.strength = stats_compute(strength_table[index],
							   strength_table[index+1]);
	e.intelligence = stats_compute(intelligence_table[index],
								   intelligence_table[index+1]);
	e.wisdom = stats_compute(wisdom_table[index],
							 wisdom_table[index+1]);
	e.dexterity = stats_compute(dexterity_table[index],
								dexterity_table[index+1]);

	//Add level diff stuff
	if(dmMode==DM_SIEGE)
		newlevel = 6;
	e.level=1;
	if(newlevel>1)
	{
		oself=self;
		self=e;
		PlayerAdvanceLevel(newlevel);
		self=oself;
	}
}

/*
======================================
drop_level
MG
Used in deathmatch where you don't
lose all exp, just enough to drop you
down one level.
======================================
*/



/*
 * $Log: not supported by cvs2svn $
 * 
 * 26    6/01/98 2:49a Mgummelt
 * 
 * 25    5/27/98 6:08p Mgummelt
 * 
 * 24    5/25/98 10:56p Mgummelt
 * Last version before send-off, v0.15
 * 
 * 23    5/25/98 8:38p Mgummelt
 * 
 * 22    5/25/98 1:39p Mgummelt
 * 
 * 21    5/25/98 1:31p Mgummelt
 * 
 * 20    5/07/98 11:19p Mgummelt
 * Almost ready for release- fix some dwarf-related compatibility problems
 * 
 * 19    5/06/98 11:10p Mgummelt
 * Last version before 1st beta
 * 
 * 18    5/05/98 8:33p Mgummelt
 * Added 6th playerclass for Siege only- Dwarf
 * 
 * 17    4/30/98 6:01p Mgummelt
 * 
 * 16    4/24/98 1:31a Mgummelt
 * Siege version 0.02 4/24/98 1:31 AM
 * 
 * 15    4/23/98 5:19p Mgummelt
 * Siege version 0.01 4/23/98
 * 
 * 14    4/17/98 9:00p Mgummelt
 * 1st version of Siege- loads but totally fucked
 * 
 * 12    4/10/98 7:36p Mgummelt
 * 
 * 11    4/10/98 7:34p Mgummelt
 * Attempting to fix crazy super experience gain, capped level at 20, no
 * matter what
 * 
 * 10    4/09/98 1:57p Mgummelt
 * Some experience changes
 * 
 * 9     4/09/98 1:39p Ssengele
 * put in extra check to make sure i'm not awarding experience to world,
 * award exper. to right fella in t_damage for lava, frozen damage.
 * 
 * 8     4/08/98 7:10p Mgummelt
 * Added in script-side support for new spartanPrint option- only sends
 * chat and relevant death messages to clients
 * 
 * 7     4/07/98 9:59p Mgummelt
 * took out a couple sprints and made it so can't die from falling damage
 * 
 * 6     4/01/98 4:43p Rmidthun
 * fixed level dropping message
 * 
 * 5     3/30/98 2:12a Rmidthun
 * capped exp to 1 billion max, just in case
 * 
 * 4     3/29/98 6:47p Rmidthun
 * 
 * 3     3/26/98 4:58p Mgummelt
 * Fixed droplevel, added playerspeedcalc, added succubus stats
 * 
 * 10    3/17/98 11:02a Mgummelt
 * 
 * 9     3/16/98 6:21p Jweier
 * 
 * 8     3/13/98 3:02a Mgummelt
 * 
 * 7     3/12/98 11:06p Jmonroe
 * change ifs to switch
 * 
 * 6     2/24/98 6:39p Mgummelt
 * 
 * 5     2/13/98 11:16a Jmonroe
 * changed succubus to demoness
 * 
 * 4     1/21/98 12:12p Jweier
 * made level up more apparent
 * 
 * 27    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 24    9/10/97 11:40p Mgummelt
 * 
 * 23    9/10/97 7:51p Mgummelt
 * 
 * 22    9/10/97 7:08p Mgummelt
 * 
 * 21    9/03/97 7:49p Mgummelt
 * 
 * 20    8/15/97 3:59p Rlove
 * 
 * 19    8/11/97 4:35p Rlove
 * 
 * 18    8/09/97 10:51a Rlove
 * 
 * 17    7/26/97 8:39a Mgummelt
 * 
 * 16    7/25/97 11:45a Mgummelt
 * 
 * 15    7/25/97 11:12a Mgummelt
 * 
 * 14    7/25/97 11:10a Mgummelt
 * 
 * 13    7/14/97 2:29p Rlove
 * 
 * 12    7/08/97 5:17p Rlove
 * 
 * 11    7/03/97 10:07a Rlove
 * 
 * 10    6/30/97 3:33p Rlove
 * 
 * 9     6/30/97 9:41a Rlove
 * 
 * 8     6/20/97 9:25a Rlove
 * 
 * 7     6/20/97 9:12a Rlove
 * New mana system added
 * 
 * 6     6/06/97 2:52p Rlove
 * Artifact of Super Health now functions properly
 * 
 * 5     5/15/97 1:15p Rjohnson
 * Added the appriate experience tables and hitpoint advancement for level
 * gains
 * 
 * 4     5/15/97 11:43a Rjohnson
 * Stats updates
 * 
 * 3     5/14/97 4:12p Rjohnson
 * Minor fix from C-side conversion
 * 
 * 2     5/14/97 3:36p Rjohnson
 * Inital stats implementation
 * 
 * 1     5/13/97 2:23p Rjohnson
 * Initial Version
 */
