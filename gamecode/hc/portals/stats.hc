/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/stats.hc,v 1.3 2007-02-07 16:59:37 sezero Exp $
 */

// ExperienceValues for each level indicate the minimum at which
// you can become that level.  The 11th entry indicates the amount
// of experience needed for each level past 10


float ExperienceValues[55] =
{
	// Paladin
	 945,			// Level 2
	2240,			// Level 3
	5250,			// Level 4
	10150,			// Level 5
	21000,			// Level 6
	39900,			// Level 7
	72800,			// Level 8
	120400,			// Level 9
	154000,			// Level 10
	210000,			// Level 11
	210000,			// Required amount for each level afterwards

	// Crusader
	911,			// Level 2
	2160,			// Level 3
	5062,			// Level 4
	9787,			// Level 5
	20250,			// Level 6
	38475,			// Level 7
	70200,			// Level 8
	116100,			// Level 9
	148500,			// Level 10
	202500,			// Level 11
	202500,			// Required amount for each level afterwards

	// Necromancer
	823,			// Level 2
	1952,			// Level 3
	4575,			// Level 4
	8845,			// Level 5
	18300,			// Level 6
	34770,			// Level 7
	63440,			// Level 8
	104920,			// Level 9
	134200,			// Level 10
	183000,			// Level 11
	183000,			// Required amount for each level afterwards

	// Assassin
	675,			// Level 2
	1600,			// Level 3
	3750,			// Level 4
	7250,			// Level 5
	15000,			// Level 6
	28500,			// Level 7
	52000,			// Level 8
	86000,			// Level 9
	110000,			// Level 10
	150000,			// Level 11
	150000,			// Required amount for each level afterwards

	// Succubus
	 871,			// Level 2
	2060,			// Level 3
	4822,			// Level 4
	9319,			// Level 5
	19278,			// Level 6
	36626,			// Level 7
	66804,			// Level 8
	110494,			// Level 9
	141334,			// Level 10
	192700,			// Level 11
	192700			// Required amount for each level afterwards
};

//  min health, max health,
//  min health per level up to level 10,  min health per level up to level 10, 
//  health per level past level 10
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
	5,		10,      3
};

float mana_table[25] =
{
//    Startup    Per Level     Past
//  min    max    min		max     10th Level
	84,		94,		6,		9, 		1,		// Paladin
	88,		98,		7,		10, 	2, 	// Crusader
    96,	   106,		10,		12, 	4,     // Necromancer
	92,	   102,		9,		11, 	3,		// Assassin
	90,	   100,		8,		11, 	3		// Succubus
};


float strength_table[10] =
{
	15,		18,		// Paladin
	12,		15,		// Crusader
	6,		10,		// Necromancer
	10,		13,		// Assassin
	11,		14		// Succubus
};

float intelligence_table[10] =
{
	6,		10,		// Paladin
	10,		13,		// Crusader
	15,		18,		// Necromancer
	6,		10,		// Assassin
	9,		13		// Succubus
};

float wisdom_table[10] =
{
	6,		10,		// Paladin
	15,		18,		// Crusader
	10,		13,		// Necromancer
	12,		15,		// Assassin
	11,		14		// Succubus
};

float dexterity_table[10] =
{
	10,		13,		// Paladin
	6,		10,		// Crusader
	8,		12,		// Necromancer
	15,		18,		// Assassin
	9,		13		// Succubus
};

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

	if (e.playerclass < CLASS_PALADIN || e.playerclass > CLASS_SUCCUBUS)
	{
		sprint(e,"Invalid player class ");
		sprint(e,ftos(e.playerclass));
		sprint(e,"\n");
		return;
	}

	// Calc initial health
	index = (e.playerclass - 1) * 5;
	e.health = stats_compute(hitpoint_table[index], hitpoint_table[index+1]);
	e.max_health = e.health;

	// Calc initial mana
	index = (e.playerclass - 1) * 5;
	e.max_mana = stats_compute(mana_table[index], mana_table[index+1]);

	index = (e.playerclass - 1) * 2;
	e.strength = stats_compute(strength_table[index], strength_table[index+1]);
	e.intelligence = stats_compute(intelligence_table[index], intelligence_table[index+1]);
	e.wisdom = stats_compute(wisdom_table[index], wisdom_table[index+1]);
	e.dexterity = stats_compute(dexterity_table[index], dexterity_table[index+1]);

	e.level = 1;
	e.experience = 0;
}

// Jump ahead one level
void player_level_cheat()
{
	float index;

	index = (self.playerclass - 1) * (MAX_LEVELS+1);

	if (self.level > MAX_LEVELS)
		index += MAX_LEVELS - 1;
	else
		index += self.level - 1;

	self.experience = ExperienceValues[index];

	if (self.level > MAX_LEVELS)
		self.experience += (self.level - MAX_LEVELS) * ExperienceValues[index+1];

	PlayerAdvanceLevel(self.level+1);
}

void player_experience_cheat(void)
{
	AwardExperience(self,self,350);
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

	sprint(self,"You are now level ");
	s2 = ftos(self.level);
	sprint(self,s2);
	sprint(self,"!\n");

	if(!self.newclass)
	{
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
		   centerprint(self, "Paladin gained a level\n");
		break;
		case CLASS_CRUSADER:
			centerprint(self,"Crusader gained a level\n");
			// Special ability #1, full mana at level advancement
			// Pa3PyX: only do this at level 3 and on, and also
			//	   fully heal, to make this more useful, and
			//	   consistent with the manual
			if (self.level > 2) {
				self.bluemana = self.greenmana = self.max_mana;
				if (self.health < self.max_health) {
					self.health = self.max_health;
				}
			}
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
		self.playerclass > CLASS_SUCCUBUS)
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

		self.greenmana += ManaInc;
		self.bluemana += ManaInc;
		self.max_mana += ManaInc;

		sprint(self, "Stats: MP +");
		s2 = ftos(ManaInc);
		sprint(self, s2);

		sprint(self, "  HP +");
		s2 = ftos(HealthInc);
		sprint(self, s2);
		sprint(self, "\n");
	}

	if (self.level > 2)
		self.flags(+)FL_SPECIAL_ABILITY1;

	if (self.level >5)
		self.flags(+)FL_SPECIAL_ABILITY2;
}


float FindLevel(entity WhichPlayer)
{
	float Chart;
	float Amount,Position,Level;

	if (WhichPlayer.playerclass < CLASS_PALADIN ||
		WhichPlayer.playerclass > CLASS_SUCCUBUS)
		return WhichPlayer.level;

	Chart = (WhichPlayer.playerclass - 1) * (MAX_LEVELS+1);

	Level = 0;
	Position=0;
	while(Position < MAX_LEVELS && Level == 0)
	{
		if (WhichPlayer.experience < ExperienceValues[Chart+Position])
			Level = Position+1;

		Position += 1;
	}

	if (!Level)
	{
		Amount = WhichPlayer.experience - ExperienceValues[Chart + MAX_LEVELS - 1];
		Level = ceil(Amount / ExperienceValues[Chart + MAX_LEVELS]) + MAX_LEVELS;
	}

	return Level;
}


void AwardExperience(entity ToEnt, entity FromEnt, float Amount)
{
	float AfterLevel;
	float IsPlayer;
	entity SaveSelf;
	float index,test40,test80,diff,index2,totalnext,wis_mod;

	if (!Amount) return;

	if(ToEnt.deadflag>=DEAD_DYING)
		return;

	IsPlayer = (ToEnt.classname == "player");

	if (FromEnt != world && Amount == 0.0)
	{
		Amount = FromEnt.experience_value;
	}

	if (ToEnt.level <4)
		Amount *= .5;

	if (ToEnt.playerclass == CLASS_PALADIN)
		Amount *= 1.4;
	else if (ToEnt.playerclass == CLASS_CRUSADER)
		Amount *= 1.35;
	else if (ToEnt.playerclass == CLASS_NECROMANCER)
		Amount *= 1.22;

	wis_mod = ToEnt.wisdom - 11;
	Amount+=Amount*wis_mod/20;//from .75 to 1.35

	ToEnt.experience += Amount;

	if (IsPlayer)
	{
		AfterLevel = FindLevel(ToEnt);

//		dprintf("Total Experience: %s\n",ToEnt.experience);

		if (ToEnt.level != AfterLevel)
		{
			SaveSelf = self;
			self = ToEnt;

			PlayerAdvanceLevel(AfterLevel);

			self = SaveSelf;
		}

	// Crusader Special Ability #1: award full health at 40% and 80% of levels experience
		// O.S.: this is supposed to be for level3+
		if (ToEnt.playerclass == CLASS_CRUSADER && ToEnt.level > 2)
		{
			index = (ToEnt.playerclass - 1) * (MAX_LEVELS+1);
			if ((ToEnt.level - 1) > MAX_LEVELS)
				index += MAX_LEVELS;
			else
				index += ToEnt.level - 1;

		//	if (ToEnt.level == 1)
		//	{
		//		test40 = ExperienceValues[index] * .4;
		//		test80 = ExperienceValues[index] * .8;
		//	}
		//	else
			if ((ToEnt.level - 1) <= MAX_LEVELS)
			{
				index2 = index - 1;
				diff = ExperienceValues[index] - ExperienceValues[index2]; 
				test40 = ExperienceValues[index2] + (diff * .4);
				test80 = ExperienceValues[index2] + (diff * .8);
			}
			else // Past MAX_LEVELS
			{
				totalnext = ExperienceValues[index - 1];   // index is 1 past MAXLEVEL at this point
				totalnext += ((ToEnt.level - 1) - MAX_LEVELS) * ExperienceValues[index];

				test40 = totalnext + (ExperienceValues[index] * .4);
				test80 = totalnext + (ExperienceValues[index] * .8);
			}

			if (((ToEnt.experience - Amount) < test40) && (ToEnt.experience> test40))
				ToEnt.health = ToEnt.max_health;
			else if (((ToEnt.experience - Amount) < test80) && (ToEnt.experience> test80))
				ToEnt.health = ToEnt.max_health;
		}
	}
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

	if (e.playerclass < CLASS_PALADIN || e.playerclass > CLASS_SUCCUBUS)
	{
		sprint(e,"Invalid player class ");
		sprint(e,ftos(e.playerclass));
		sprint(e,"\n");
		return;
	}

	// Calc initial health
	index = (e.playerclass - 1) * 5;
	e.health = stats_compute(hitpoint_table[index], hitpoint_table[index+1]);
	e.max_health = e.health;

	// Calc initial mana
	index = (e.playerclass - 1) * 5;
	e.max_mana = stats_compute(mana_table[index], mana_table[index+1]);

	index = (e.playerclass - 1) * 2;
	e.strength = stats_compute(strength_table[index], strength_table[index+1]);
	e.intelligence = stats_compute(intelligence_table[index], intelligence_table[index+1]);
	e.wisdom = stats_compute(wisdom_table[index], wisdom_table[index+1]);
	e.dexterity = stats_compute(dexterity_table[index], dexterity_table[index+1]);

	//Add level diff stuff
	newlevel = FindLevel(e);
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

void drop_level (entity loser,float number)
{
float pos,lev_pos,new_exp,mana_dec,health_dec,dec_pos;
	if(loser.classname!="player")
		return;

	if(loser.level-number<1)
	{//would drop below level 1, set to level 1
		loser.experience=0;
		dec_pos = (loser.playerclass - 1) * 5;
		loser.max_health= hitpoint_table[dec_pos];
		loser.max_mana = mana_table[dec_pos];
		if(loser.health>loser.max_health)
			loser.health=loser.max_health;
		if(loser.bluemana>loser.max_mana)
			loser.bluemana=loser.max_mana;
		if(loser.greenmana>loser.max_mana)
			loser.greenmana=loser.max_mana;
		return;
	}

	pos = (loser.playerclass - 1) * (MAX_LEVELS+1);
	if(loser.level-number>1)
	{
		loser.level-=number;
		lev_pos+=loser.level - 2;
		if(lev_pos>9)//last number in that char's 
		{
			new_exp=ExperienceValues[pos+10];
			loser.experience=new_exp+new_exp*(lev_pos - 9);
		}
		else
			loser.experience = ExperienceValues[pos+lev_pos];
	}
	else
	{
		loser.level=1;
		loser.experience=0;
	}

	if (loser.level <= 2)
		loser.flags(-)FL_SPECIAL_ABILITY1;

	if (loser.level <=5)
		loser.flags(-)FL_SPECIAL_ABILITY2;

	dec_pos = (loser.playerclass - 1) * 5;
	health_dec = hitpoint_table[dec_pos+4];
	mana_dec = mana_table[dec_pos+4];

	loser.max_health -= health_dec *number;
	if(loser.health>loser.max_health)
		loser.health=loser.max_health;

	loser.max_mana -= mana_dec *number;
	if(loser.bluemana>loser.max_mana)
		loser.bluemana=loser.max_mana;
	if(loser.greenmana>loser.max_mana)
		loser.greenmana=loser.max_mana;
}

