/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/artifact.hc,v 1.1 2005-01-26 17:26:09 sezero Exp $
 */




/*
 * artifact_touch() -- Called when an artifact is being touched.
 *                     Awards players random amounts of whatever they represent.
 */
void() SUB_regen;
void() StartItem;
void() UseInventoryItem;
void ring_touch(void);

//getInventoryCount--returns # of items <who> has
float getInventoryCount(entity who, float itemType)
{
	if(itemType == STR_TORCH)
	{
		return who.cnt_torch;
	}
	else if(itemType == STR_HEALTHBOOST)
	{
		return who.cnt_h_boost;
	}
	else if(itemType == STR_SUPERHEALTHBOOST) // 5 limit
	{
		return who.cnt_sh_boost;
	}
	else if(itemType == STR_MANABOOST)
	{
		return who.cnt_mana_boost;
	}
	else if(itemType == STR_TELEPORT)
	{
		return who.cnt_teleport;
	}
	else if(itemType == STR_TOME)
	{
		if(tomeMode == 2)//how many tomes do i really have? um er...none?
		{
			return 0;
		}
		else
		{
			return who.cnt_tome;
		}
	}
	else if(itemType == STR_SUMMON)
	{
		return who.cnt_summon;
	}
	else if(itemType == STR_INVISIBILITY)
	{
		return who.cnt_invisibility;
	}
	else if(itemType == STR_GLYPH)
	{
		return who.cnt_glyph;
	}
	else if(itemType == STR_RINGFLIGHT)
	{
		return who.cnt_flight;
	}
	else if(itemType == STR_HASTE)
	{
		return who.cnt_haste;
	}
	else if(itemType == STR_BLAST)
	{
		return who.cnt_blast;
	}
	else if(itemType == STR_POLYMORPH)
	{
		return who.cnt_polymorph;
	}
	else if(itemType == STR_CUBEOFFORCE)
	{
		return who.cnt_cubeofforce;
	}
	else if(itemType == STR_INVINCIBILITY)
	{
		if(dmMode == DM_CAPTURE_THE_TOKEN)
		{
			if (who.gameFlags & GF_HAS_TOKEN)//how many icons can i pick up when there's just 1 on level? who knows? maybe i Can pick up 50; since there will only be the opportunity to pick up that 1, tho, i'll never know for sure. uh, never mind.
			{
				return 1;//but it's not a real invincibility...
			}
			else
			{
				return 0;
			}
		}
		else
			return who.cnt_invincibility;
	}
	return 0;
}

//rommForItem--returns # of items <who> can pick up--0 for full capacity, negative for over capacity
float roomForItem(entity who, float itemType)//F***ING S**T IS RIGHT! exotic play modes (alt tome mode, captur the icon) NEED to be handled safely by funcs that call me, cause i treat them as normal cases. so there.
{
	float slots;
	slots = 0;

	if(itemType == STR_TORCH)
	{
		slots = 5-who.cnt_torch;
	}
	else if(itemType == STR_HEALTHBOOST)
	{
		if (who.playerclass!=CLASS_CRUSADER)
			slots = 5-who.cnt_h_boost;
		else
			slots = 5-who.cnt_h_boost;
	}
	else if(itemType == STR_SUPERHEALTHBOOST) // 5 limit
	{
		if (deathmatch)
			slots = 2-who.cnt_sh_boost;
		else
			slots = 5-who.cnt_sh_boost;
	}
	else if(itemType == STR_MANABOOST)
	{
		slots = 5-who.cnt_mana_boost;
	}
	else if(itemType == STR_TELEPORT)
	{
		slots = 5-who.cnt_teleport;
	}
	else if(itemType == STR_TOME)
	{
		if(tomeMode == 2)//how many tomes can i pick up here?AAAAARRRRGGGGGGGGHHHHHHH it makes me crazy just contemplating it.
		{
			slots = 1;
		}
		else
		{
			slots = 2-who.cnt_tome;
		}
	}
	else if(itemType == STR_SUMMON)
	{
		slots = 5-who.cnt_summon;
	}
	else if(itemType == STR_INVISIBILITY)
	{
		slots = 5-who.cnt_invisibility;
	}
	else if(itemType == STR_GLYPH)
	{
		slots = 5-who.cnt_glyph;
	}
	else if(itemType == STR_RINGFLIGHT)
	{
		slots = 5-who.cnt_flight;
	}
	else if(itemType == STR_HASTE)
	{
		slots = 5-who.cnt_haste;
	}
	else if(itemType == STR_BLAST)
	{
		slots = 15-who.cnt_blast;
	}
	else if(itemType == STR_POLYMORPH)
	{
		slots = 5-who.cnt_polymorph;
	}
	else if(itemType == STR_CUBEOFFORCE)
	{
		slots = 5-who.cnt_cubeofforce;
	}
	else if(itemType == STR_INVINCIBILITY)
	{
		if(dmMode == DM_CAPTURE_THE_TOKEN)
		{
			if (who.gameFlags & GF_HAS_TOKEN)//how many icons can i pick up when there's just 1 on level? who knows? maybe i Can pick up 50; since there will only be the opportunity to pick up that 1, tho, i'll never know for sure. uh, never mind.
			{
				slots = 0;
			}
			else
			{
				slots = 1;
			}
		}
		else
			slots = 1-who.cnt_invincibility;
	}

	return slots;
}

//adjustInventoryCount--positive numba to add, neg to subtract
void adjustInventoryCount(entity who, float itemType, float numba)
{
	float realNumba,ftemp;
	
	ftemp = roomForItem(who,itemType);

	if (numba==0)
		return;//you're trying to break this function, aren't you?

	if (numba > 0)//adding items
	{
		if (ftemp <= 0)
			return;//no room.
		if (numba > ftemp)
			realNumba = ftemp;//less room than i'd like
		else
			realNumba = numba;//enough room
	}
	else
	{
		realNumba = numba;//check right after adjustment to make sure don't go < 0
	}

	if(itemType == STR_TORCH)
	{
		who.cnt_torch+=realNumba;
		if (who.cnt_torch < 0)
			who.cnt_torch = 0;
	}
	else if(itemType == STR_HEALTHBOOST)
	{
		who.cnt_h_boost+=realNumba;
		if (who.cnt_h_boost < 0)
			who.cnt_h_boost = 0;
	}
	else if(itemType == STR_RINGFLIGHT)
	{
		who.cnt_flight+=realNumba;
		if (who.cnt_flight < 0)
			who.cnt_flight = 0;
	}
	else if(itemType == STR_SUPERHEALTHBOOST)
	{
		who.cnt_sh_boost+=realNumba;
		if (who.cnt_sh_boost < 0)
			who.cnt_sh_boost = 0;
	}
	else if(itemType == STR_MANABOOST)
	{
		who.cnt_mana_boost+=realNumba;
		if (who.cnt_mana_boost < 0)
			who.cnt_mana_boost = 0;
	}
	else if(itemType == STR_TELEPORT)
	{
		who.cnt_teleport+=realNumba;
		if (who.cnt_teleport < 0)
			who.cnt_teleport = 0;
	}
	else if(itemType == STR_TOME)
	{
		if((tomeMode == 2)&&(realNumba > 0))
		{
			who.poweredFlags(+)who.weapon;
		}
		else
		{
			who.cnt_tome+=realNumba;
			if (who.cnt_tome < 0)
				who.cnt_tome = 0;
		}
	}
	else if(itemType == STR_SUMMON)
	{
		who.cnt_summon+=realNumba;
		if (who.cnt_summon < 0)
			who.cnt_summon = 0;
	}
	else if(itemType == STR_INVISIBILITY)
	{
		who.cnt_invisibility+=realNumba;
		if (who.cnt_invisibility < 0)
			who.cnt_invisibility = 0;
	}
	else if(itemType == STR_GLYPH)
	{
		who.cnt_glyph+=realNumba;
		if (who.cnt_glyph < 0)
			who.cnt_glyph = 0;
	}
	else if(itemType == STR_HASTE)
	{
		who.cnt_haste+=realNumba;
		if (who.cnt_haste < 0)
			who.cnt_haste = 0;
	}
	else if(itemType == STR_BLAST)
	{
		who.cnt_blast+=realNumba;
		if (who.cnt_blast < 0)
			who.cnt_blast = 0;
	}
	else if(itemType == STR_POLYMORPH)
	{
		who.cnt_polymorph+=realNumba;
		if (who.cnt_polymorph < 0)
			who.cnt_polymorph = 0;
	}
	else if(itemType == STR_CUBEOFFORCE)
	{
		who.cnt_cubeofforce+=realNumba;
		if (who.cnt_cubeofforce < 0)
			who.cnt_cubeofforce = 0;
	}
	else if(itemType == STR_INVINCIBILITY)
	{
		if(dmMode == DM_CAPTURE_THE_TOKEN)
		{
			if ( (!(who.gameFlags & GF_HAS_TOKEN)) && realNumba > 0 )//fresh token
			{
				other.gameFlags (+) GF_HAS_TOKEN;
				other.effects (+) EF_BRIGHTFIELD;
			}
			else if ( (who.gameFlags & GF_HAS_TOKEN) && realNumba < 0 )//byby mr token
			{
				other.gameFlags (-) GF_HAS_TOKEN;
				other.effects (-) EF_BRIGHTFIELD;
			}
		}
		else
		{
			who.cnt_invincibility+=realNumba;
			if (who.cnt_invincibility < 0)
				who.cnt_invincibility = 0;
		}
	}

}


float countPlayers(void)
{
	num_players;
	entity lastent;

	lastent=nextent(world);
	num_players=0;
	while(lastent)
	{
		if(lastent.classname=="player")
		{
			num_players+=1;

		}
		lastent=find(lastent,classname,"player");
	}
	return num_players;
}

void artifact_touch()
{
	float amount;
	float numPlayers;
	float scaleVal;
	entity	oldself;
	float oldInv;
//	string printnum;

	if(self.artifact_name == STR_TELEPORT&&self.owner!=world)
	{//Thrown teleport coin opens a portal
		if(other.classname=="player"&&self.owner!=other)
		{
			other.flags2(+)FL_TORNATO_SAFE;
			oldself = spawn();

			oldself.goalentity = SelectSpawnPoint ();

			oldself.classname = "teleportcoin";
			oldself.inactive = FALSE;
			oldself.think = teleport_coin_run;
			oldself.nextthink = time + .01;
			oldself.spawnflags =0;
			oldself.enemy = other;

			//added this for chaos device hangin-around
			setorigin (oldself, other.origin);
			oldself.movetype = MOVETYPE_NONE;
			oldself.solid = SOLID_TRIGGER;
			oldself.takedamage = DAMAGE_NO;
			setsize(oldself,'-16 -16 0','16 16 56');
			remove(self);
			return;
		}
		else if(self.flags&FL_ONGROUND)
		{
			oldself = spawn();
			oldself.goalentity = SelectSpawnPoint ();

			oldself.classname = "teleportcoin";
			oldself.inactive = FALSE;
			oldself.think = teleport_coin_run;
			oldself.nextthink = time + .01;
			oldself.spawnflags =0;

			//added this for chaos device hangin-around
			setorigin (oldself, self.origin);
			oldself.movetype = MOVETYPE_NONE;
			oldself.solid = SOLID_TRIGGER;
			oldself.takedamage = DAMAGE_NO;
			setsize(oldself,'-16 -16 0','16 16 56');
			remove(self);
			return;
		}
	}
	if(other.classname != "player"||other.model=="models/sheep.mdl")
	{ // Only players can take artifacts
		return;
	}
	if(other.health <= 0)
	{ // Player is dead
		return;
	}

	if (self.owner == other && self.artifact_ignore_owner_time > time)
		return;

	if (self.artifact_ignore_time > time) 
		return;

	if(self.artifact_name == STR_INVINCIBILITY&&other.siege_team!=ST_DEFENDER)
		return;

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}


	if(self.artifact_name == STR_GLYPH)
	{
		if ((other.cnt_glyph + 1) > 10||other.playerclass!=CLASS_ASSASSIN)
			return;	
		else	
			other.cnt_glyph += 1;
	}
	else if(self.artifact_name  == STR_GRENADES)
	{
		if ((other.cnt_grenades + 15) > 45||other.playerclass!=CLASS_SUCCUBUS)
			return;	
		else	
			other.cnt_grenades += 15;
	}
	else if(self.artifact_name  == STR_ARROWS)
	{
		if ((other.cnt_arrows + 25) > 100||other.playerclass==CLASS_CRUSADER||other.playerclass==CLASS_DWARF)
			return;	
		else	
			other.cnt_arrows += 25;
	}
	else if(self.artifact_name  == STR_CLIMB)
	{
		if (other.playerclass!=CLASS_ASSASSIN||other.flags2&FL2_WALLCLIMB)
			return;	
		other.flags2(+)FL2_WALLCLIMB;
		other.artifact_active(+)ART_CLIMB;
		centerprint(other,"You have the climbing boots!\n");
		sound(other, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
		stuffcmd(other, "bf\n");
		self.solid = SOLID_NOT;
		self.model = string_null;
		thinktime self : 10;
		self.think = SUB_regen;
		activator = other;
		SUB_UseTargets(); // Fire all targets / killtargets
		return;
	}
	else if(self.classname == "art_sword_and_crown")
	{
		if(other.siege_team==ST_DEFENDER)
			return;

		if(other.siege_team==ST_ATTACKER)
		{
			other.experience+=1000;//yahoo!
			bprint(PRINT_HIGH,other.netname);
			bprint(PRINT_HIGH," has captured the ");
			bprint(PRINT_HIGH,self.netname);
			bprint(PRINT_HIGH,"!\n");
			centerprint_all_clients("The Crown has been Siezed!!!\n",other);
			centerprint(other,"You have captured the Crown!\n");
			end_siege_game (ST_DEFENDER,WP_ATTCROWN);
		}
	}
	else
	{
		if (roomForItem(other,self.artifact_name)<=0)
			return;
	
		adjustInventoryCount(other,self.artifact_name,1);
	}

	if(autoItems)
	{
		if((self.artifact_name == STR_INVISIBILITY)||
			(self.artifact_name == STR_HASTE)||
			(self.artifact_name == STR_CUBEOFFORCE)||
			(self.artifact_name == STR_SUPERHEALTHBOOST)||
			(self.artifact_name == STR_MANABOOST)||
			((self.artifact_name == STR_INVINCIBILITY)&&(dmMode != DM_CAPTURE_THE_TOKEN)))
		{
			oldself = self;
			self = other;
			oldInv = self.inventory;

			if(oldself.artifact_name == STR_INVISIBILITY)
			{
				self.inventory = INV_INVISIBILITY;
			}
			else if(oldself.artifact_name == STR_HASTE)
			{
				self.inventory = INV_HASTE;
			}
			else if(oldself.artifact_name == STR_CUBEOFFORCE)
			{
				self.inventory = INV_CUBEOFFORCE;
			}
			else if(oldself.artifact_name == STR_SUPERHEALTHBOOST)
			{
				self.inventory = INV_SUPER_HP_BOOST;
			}
			else if(oldself.artifact_name == STR_MANABOOST)
			{
				self.inventory = INV_MANA_BOOST;
			}
			else if(oldself.artifact_name == STR_INVINCIBILITY)
			{
				self.inventory = INV_INVINCIBILITY;
			}


			UseInventoryItem();

			self.cnt_invisibility = 0;
			self.cnt_haste = 0;
			self.cnt_cubeofforce = 0;
			self.cnt_sh_boost = 0;
			self.cnt_mana_boost = 0;
			self.cnt_invincibility = 0;

			self.inventory = oldInv;
			self = oldself;
		}
	}

	if((self.artifact_name == STR_INVINCIBILITY)&&(dmMode == DM_CAPTURE_THE_TOKEN))
	{
		//bcenterprint(other.netname);
		//bcenterprint(" has acquired the Icon!!!\n");
		bcenterprint2(other.netname, " has acquired the Icon!!!\n");

		//bprint(PRINT_MEDIUM, other.netname);
		//bprint(PRINT_MEDIUM, " has acquired the Icon!!!\n");
	}
	else if(self.artifact_name&&self.classname == "art_sword_and_crown")
	{
		amount = random();
		if (amount < 0.5)
		{
			sprinti (other, PRINT_MEDIUM, STR_YOUPOSSESS);
			sprinti (other, PRINT_MEDIUM, self.artifact_name);
		}
		else
		{
			sprinti (other, PRINT_MEDIUM, STR_YOUHAVEACQUIRED);
			sprinti (other, PRINT_MEDIUM, self.artifact_name);
		}

		sprint (other,PRINT_MEDIUM, "\n");
	}

	if (self.artifact_respawn)
	{
		self.mdl = self.model;

		numPlayers = countPlayers();

		if(patternRunner)
		{
			scaleVal = 1.0;
		}
		else
		{
			scaleVal = 2.0 - (numPlayers * .125);
			if(scaleVal < 1)
			{
				scaleVal = 1;
			}
		}

		if(self.artifact_name==STR_INVINCIBILITY)
			thinktime self : 300;
		else if(self.artifact_name==STR_INVISIBILITY)
			thinktime self : 90*scaleVal;
		else if(self.artifact_name==STR_TORCH)
			thinktime self : 10*scaleVal;
		else
			thinktime self : 60*scaleVal;
		self.think = SUB_regen;
	}

	sound(other, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
	stuffcmd(other, "bf\n");
	self.solid = SOLID_NOT;
//	other.items = other.items | self.items;
	self.model = string_null;

	activator = other;
	SUB_UseTargets(); // Fire all targets / killtargets

	if(!self.artifact_respawn||self.artifact_name==STR_INVINCIBILITY)
	{
		remove(self);
	}
}


void Artifact_Cheat(void)
{
	self.cnt_sh_boost = 20;
	self.cnt_summon = 20;
	self.cnt_glyph = 20;
	self.cnt_blast = 20;
	self.cnt_polymorph = 20;
	self.cnt_flight = 20;
	self.cnt_cubeofforce = 20;
	self.cnt_invincibility = 20;
	self.cnt_invisibility = 20;
	self.cnt_haste = 20;
	self.cnt_mana_boost = 20;
	self.cnt_sh_boost = 20;
	self.cnt_h_boost = 20;
	self.cnt_teleport = 20;
	self.cnt_tome = 20;
	self.cnt_torch = 20;
}


/*-----------------------------------------
	GenerateArtifactModel - generate the artifact 
  -----------------------------------------*/
void GenerateArtifactModel(string modelname,float art_name,float respawnflag) 
{
	if (respawnflag)	// Should this thing respawn
		self.artifact_respawn = deathmatch;

	setmodel(self, modelname);
	self.artifact_name = art_name;
	self.netname = "artifact";

	if (modelname == "models/ringft.mdl")
	{
		self.netname = "RingofFlight";
		self.classname = "Ring_Flight";
		self.touch	 = ring_touch;
	}
	else if (modelname != "models/a_xray.mdl")
		self.touch	 = artifact_touch;
	setsize (self, '0 0 0', '0 0 0');

	StartItem();
}


/*-----------------------------------------
	spawn_artifact - decide which artifact to spawn
  -----------------------------------------*/
void spawn_artifact (float artifact,float respawnflag)
{
	if (artifact == ARTIFACT_HASTE)
		GenerateArtifactModel("models/a_haste.mdl",STR_HASTE,respawnflag);
	else if (artifact == ARTIFACT_POLYMORPH)
		GenerateArtifactModel("models/a_poly.mdl",STR_POLYMORPH,respawnflag);
	else if (artifact == ARTIFACT_GLYPH)
		GenerateArtifactModel("models/a_glyph.mdl",STR_GLYPH,respawnflag);
	else if (artifact == ARTIFACT_GRENADES)
		GenerateArtifactModel("models/w_l3_c4.mdl",STR_GRENADES,respawnflag);
	else if (artifact == ARTIFACT_ARROWS)
		GenerateArtifactModel("models/arrows.mdl",STR_ARROWS,respawnflag);
	else if (artifact == ARTIFACT_INVISIBILITY)
		GenerateArtifactModel("models/a_invis.mdl",STR_INVISIBILITY,respawnflag);
	else if (artifact == ARTIFACT_INVINCIBILITY)
		GenerateArtifactModel("models/a_invinc.mdl",STR_INVINCIBILITY,respawnflag);
	else if (artifact == ARTIFACT_CUBEOFFORCE)
		GenerateArtifactModel("models/a_cube.mdl",STR_CUBEOFFORCE,respawnflag);
//	else if (artifact == ARTIFACT_SUMMON)
//		GenerateArtifactModel("models/a_summon.mdl",STR_SUMMON,respawnflag);
	else if (artifact == ARTIFACT_TOME)
		GenerateArtifactModel("models/a_tome.mdl",STR_TOME,respawnflag);
	else if (artifact == ARTIFACT_TELEPORT)
		GenerateArtifactModel("models/a_telprt.mdl",STR_TELEPORT,respawnflag);
	else if (artifact == ARTIFACT_MANA_BOOST)
		GenerateArtifactModel("models/a_mboost.mdl",STR_MANABOOST,respawnflag);
	else if (artifact == ARTIFACT_BLAST)
		GenerateArtifactModel("models/a_blast.mdl",STR_BLAST,respawnflag);
	else if (artifact == ARTIFACT_TORCH)
		GenerateArtifactModel("models/a_torch.mdl",STR_TORCH,respawnflag);
	else if (artifact == ARTIFACT_HP_BOOST)
		GenerateArtifactModel("models/a_hboost.mdl",STR_HEALTHBOOST,respawnflag);
	else if (artifact == ARTIFACT_SUPER_HP_BOOST)
		GenerateArtifactModel("models/a_shbost.mdl",STR_SUPERHEALTHBOOST,respawnflag);
	else if (artifact == ARTIFACT_CLIMB)
		GenerateArtifactModel("models/a_climb.mdl",STR_CLIMB,respawnflag);
//	else if (artifact == ARTIFACT_FLIGHT)//what is this artifact supposed to be?!?!?!
//		GenerateArtifactModel("models/ringft.mdl",STR_FLIGHT,respawnflag);
}


/*
====================================================================================================

SUPER HP BOOST

====================================================================================================
*/

void DecrementSuperHealth()
{
	float wait_time,over,decr_health;

	if (self.health > self.max_health)
	{
		if (self.health<200)
		{
			wait_time = 2;
			decr_health = 1;
		}
		else if (self.health<400)  // Vary rate of update time
		{
			decr_health = 1;
			over = 200 - (self.health - 200);
			wait_time = over/400;
			if (wait_time < .10)
				wait_time = .10;
		}
		else						// Vary the amount of the decrement
		{
			wait_time = .10;
			over = self.health - 400;
			decr_health = over * .016;
			decr_health = ceil(decr_health);
			if (decr_health < 2)
				decr_health = 2;
		}
		
		self.health = self.health - decr_health;

		self.healthtime = time + wait_time;
	}
	else  // All done, get rid of it
		self.artifact_flags (-) AFL_SUPERHEALTH;  

}


void use_super_healthboost()
{
	self.healthtime = time + .05;

	if(self.health<0)
		self.health=100;
	else if (self.health < 899)
		self.health = self.health + 100;
	else if (self.health > 999)
		self.health = 999;

	self.cnt_sh_boost -= 1;
	self.artifact_flags(+)AFL_SUPERHEALTH;   // Show the health is in use

	if(self.flags2&FL2_POISONED)
	{
		self.flags2(-)FL2_POISONED;
		centerprint(self,"The poison has been cleansed from your blood...\n");
	}
}


/*QUAKED art_SuperHBoost (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for the Super Health Boost
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_SuperHBoost()
{
	spawn_artifact(ARTIFACT_SUPER_HP_BOOST,RESPAWN);
}




/*
====================================================================================================

HP BOOST

====================================================================================================
*/

void use_healthboost()
{
	if(self.health >= self.max_health)
	{ // Already at max health
		return;
	}
	self.cnt_h_boost -= 1;
	self.health += 25;
  	if(self.health > self.max_health)
	{
  		self.health = self.max_health;
	}

	if(self.flags2&FL2_POISONED)
	{
		self.flags2(-)FL2_POISONED;
		centerprint(self,"The poison has been cleansed from your blood...\n");
	}
}


/*QUAKED art_HealthBoost (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for the Health Boost
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_HealthBoost()
{
	spawn_artifact(ARTIFACT_HP_BOOST,RESPAWN);
}




/*
====================================================================================================

The TORCH

====================================================================================================
*/

/*QUAKED art_torch (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for the torch
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_torch()
{
	spawn_artifact(ARTIFACT_TORCH,RESPAWN);
}


void KillTorch()
{
	if(!self.artifact_active&ART_INVISIBILITY)
		self.effects(-)EF_DIMLIGHT;   // Turn off lights
	self.artifact_flags(-)AFL_TORCH;  // Turn off torch flag
	if(self.model=="models/a_torch.mdl")
		remove(self);
	else
	{
		self.cnt_torch	-= 1;
		if (self.cnt_torch < 0)
			self.cnt_torch = 0;
	}
}

void DouseTorch()//Never called?!
{
	sound (self, CHAN_BODY, "raven/douse.wav", 1, ATTN_IDLE);
	self.torchtime = 0;
	KillTorch();
}

void DimTorch()
{
	sound (self, CHAN_BODY, "raven/kiltorch.wav", 1, ATTN_IDLE);

	self.effects(-)EF_TORCHLIGHT;
	if(self.classname=="player")
		self.torchtime = time + 15;
	else
		self.torchtime = time + 3;
	self.torchthink = KillTorch;
}


void FullTorch()
{
	sound (self, CHAN_BODY, "raven/fire1.wav", 1, ATTN_NORM);
	self.effects(+)EF_TORCHLIGHT;
	if(self.classname=="player")
		self.torchtime = time + 45;
	else
		self.torchtime = time + 7;
	self.torchthink = DimTorch;
}


/*
============
TorchBurn

============
*/
void UseTorch()
{
	if((self.effects!=EF_DIMLIGHT) && (self.effects!=EF_TORCHLIGHT))
	{
		sound (self, CHAN_WEAPON, "raven/littorch.wav", 1, ATTN_NORM);

		self.effects(+)EF_DIMLIGHT;   // set player to emit light
		self.torchtime		= time + 1;
		self.torchthink		= FullTorch;
		self.artifact_flags (+) AFL_TORCH;   // Show the torch is in use
		//self.cnt_torch		-= 1;// not until it goes out or is thrown
	}
}


/*QUAKED art_blastradius (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Blast Radius
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_blastradius()
{
	spawn_artifact(ARTIFACT_BLAST,RESPAWN);
}



void UseManaBoost()
{
	self.bluemana  = self.max_mana;
	self.greenmana = self.max_mana;

	self.cnt_mana_boost -= 1;
}


/*QUAKED art_manaboost (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Mana Boost
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_manaboost()
{
	spawn_artifact(ARTIFACT_MANA_BOOST,RESPAWN);
}


/*QUAKED art_teleport (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Teleportation
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_teleport()
{
	remove(self);
	return;
	spawn_artifact(ARTIFACT_TELEPORT,RESPAWN);
}


/*QUAKED art_tomeofpower (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Tome of Power
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_tomeofpower()
{
	spawn_artifact(ARTIFACT_TOME,RESPAWN);
}


/*QUAKED art_summon (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Summoning
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_summon()
{
	spawn_artifact(ARTIFACT_SUMMON,RESPAWN);
}

/*QUAKED art_glyph (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Glyph of the Ancients
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_glyph()
{
	spawn_artifact(ARTIFACT_GLYPH,RESPAWN);
}


/*QUAKED ammo_arrows (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void ammo_arrows ()
{
	precache_model("models/arrows.mdl");
	spawn_artifact(ARTIFACT_ARROWS,RESPAWN);
}

/*QUAKED ammo_grenades (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void ammo_grenades ()
{
	precache_model("models/w_l3_c4.mdl");
	spawn_artifact(ARTIFACT_GRENADES,RESPAWN);
}

/*QUAKED art_haste (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Haste
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_haste()
{
	spawn_artifact(ARTIFACT_HASTE,RESPAWN);
}


/*QUAKED art_polymorph (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Polymorph
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_polymorph()
{
	spawn_artifact(ARTIFACT_POLYMORPH,RESPAWN);
}

/*QUAKED art_cubeofforce (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Cube Of Force
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_cubeofforce()
{
	if(skill>2)
	{
		remove(self);
		return;
	}
	spawn_artifact(ARTIFACT_CUBEOFFORCE,RESPAWN);
}


/*QUAKED art_invincibility (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Invincibility
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_invincibility()
{
	spawn_artifact(ARTIFACT_INVINCIBILITY,RESPAWN);
}

/*QUAKED art_invisibility (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Invisibility
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_invisibility()
{
	self.classname = "Ring_WaterBreathing";
	Ring_WaterBreathing();
//	spawn_artifact(ARTIFACT_INVISIBILITY,RESPAWN);
}

/*QUAKED art_climb (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for wallclimbing
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void art_climb()
{
	precache_model("models/a_climb.mdl");
	spawn_artifact(ARTIFACT_CLIMB,RESPAWN);
}

void spawn_art_sword_and_crown(void)
{
	self.effects=EF_BRIGHTLIGHT;
	setmodel(self, self.mdl);
	if(!self.netname)
		self.netname = "Crown";
	self.touch	 = artifact_touch;
	setsize (self, '-8 -8 0', '8 8 8');

	StartItem();
}

/*QUAKED art_sword_and_crown (.0 .0 .5) (-8 -8 0) (8 8 20) FLOATING
Artifact for Sword and Crown
-------------------------FIELDS-------------------------
"mdl" set this if you want to use your own model- this includes path and extension- for example: "models/xcalibur.mdl"
"netname" set this if you want to give your winning Siege Piece it's own name when you capture it- for example: "Excalibur"
--------------------------------------------------------
*/
void art_sword_and_crown()
{
	self.spawnflags(+)1;
	if(!self.mdl)
		self.mdl = "models/crown.mdl";
	precache_model2(self.mdl);
	self.artifact_respawn = deathmatch;
	spawn_art_sword_and_crown();
}

void item_spawner_use(void)
{
	DropBackpack();
}

/*QUAKED item_spawner (.0 .0 .5) (-8 -8 -44) (8 8 20) 
Generic item spawner
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
void item_spawner()
{
	setmodel(self, self.model);       // set size and link into world
	self.solid		= SOLID_NOT;
	self.movetype	= MOVETYPE_NONE;
	self.modelindex = 0;
	self.model		= "";
	self.effects = EF_NODRAW;
	
	self.use = item_spawner_use;
}

void tokenDrop()
{
	//bprint(PRINT_MEDIUM, "Teleporting the Icon to a new location...\n");
	bcenterprint2("","Teleporting the Icon to a new location...\n");
	remove(self);
}

void spawnNewDmToken(entity spawnSpot, float persists)
{
	entity oldself, newGuy;

	oldself = self;

	newGuy = spawn();
	newGuy.origin = spawnSpot.origin + '0 0 40';
	self = newGuy;

	spawn_artifact(ARTIFACT_INVINCIBILITY,NO_RESPAWN);	
	newGuy.classname = "dmMode1_token";

	if(!persists)
	{
		newGuy.nextthink = time + 10.0;
		newGuy.think = tokenDrop;
		newGuy.velocity_x = random(-200,200);
		newGuy.velocity_y = random(-200,200);
		newGuy.velocity_z = 300;
	}
	newGuy.effects (+) EF_BRIGHTFIELD;

	self = oldself;
}


float stealRandomItem(entity from, entity to)
{
	entity item;
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
		return 3;
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 31    5/25/98 1:38p Mgummelt
 * 
 * 30    5/11/98 2:36p Mgummelt
 * 
 * 29    5/07/98 11:19p Mgummelt
 * Almost ready for release- fix some dwarf-related compatibility problems
 * 
 * 28    5/06/98 11:10p Mgummelt
 * Last version before 1st beta
 * 
 * 27    5/05/98 8:33p Mgummelt
 * Added 6th playerclass for Siege only- Dwarf
 * 
 * 26    5/01/98 4:02p Mgummelt
 * Siege version 0.2 5/1/98 4:02PM
 * 
 * 25    4/24/98 1:31a Mgummelt
 * Siege version 0.02 4/24/98 1:31 AM
 * 
 * 24    4/23/98 5:18p Mgummelt
 * Siege version 0.01 4/23/98
 * 
 * 23    4/17/98 9:00p Mgummelt
 * 1st version of Siege- loads but totally fucked
 * 
 * 21    3/29/98 10:04p Rmidthun
 * added curing of poison by using health boost and mystic urn
 * 
 * 20    3/29/98 4:41p Rmidthun
 * Added teleport coin auto-use when thrown
 * 
 * 19    3/29/98 3:50p Nalbury
 * Made certain items only respawn when noone is around...
 * 
 * 18    3/26/98 8:14p Ssengele
 * assassin's powered up punchdagger steals items.
 * 
 * 17    3/26/98 1:31p Ssengele
 * inventory limits enforced with backpacks, now backpacks stick around if
 * they contain stuff you can't pick up.
 * 
 * 16    3/24/98 6:20p Nalbury
 * Added easyfourth and patternrunner server options...
 * 
 * 15    3/24/98 12:49p Ssengele
 * mystic urn should more consistently save peoples--was just adding 100
 * to folks' health if it was <0, with min of 1 end result; now, if health
 * is <0, urn will set it to 100.
 * 
 * 14    3/23/98 1:27p Nalbury
 * Added in autoItems stuff...
 * 
 * 13    3/22/98 4:56a Nalbury
 * Added a little target blob...
 * 
 * 12    3/21/98 8:05p Ssengele
 * 
 * 11    3/21/98 12:09p Nalbury
 * tweaked capture the icon thingy
 * 
 * 10    3/20/98 5:08p Nalbury
 * fixed a centerprint thingy
 * 
 * 9     3/20/98 4:17p Nalbury
 * Added capture the icon ... made it so sheep could still pick up
 * everything...
 * 
 * 8     3/19/98 1:18p Nalbury
 * Added electricty to axes...  increased max repulsion discs to 15...
 * sheep player's can now pick up items...
 * 
 * 7     3/18/98 3:56p Rmidthun
 * reduced number of items that can be carried
 * 
 * 6     3/16/98 10:01p Ssengele
 * fixed bugs with ringoflight droppin.
 * 
 * 5     3/13/98 12:17p Nalbury
 * made getting the invincibility less frequent.
 * 
 * 4     3/13/98 12:14p Nalbury
 * altered respawn time based on the number of players.
 * 
 * 3     3/12/98 3:50p Nalbury
 * Added in tomeMode's 1 and 2.
 * 
 * 2     3/06/98 6:45a Nalbury
 * Modified crusaders glyph
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 75    9/10/97 8:00p Mgummelt
 * 
 * 74    9/02/97 2:01a Rlove
 * 
 * 73    9/01/97 6:45p Mgummelt
 * 
 * 72    9/01/97 1:35a Mgummelt
 * 
 * 70    8/29/97 11:14p Mgummelt
 * 
 * 69    8/26/97 2:26a Mgummelt
 * 
 * 68    8/25/97 6:37p Rlove
 * 
 * 67    8/25/97 6:01p Rlove
 * 
 * 66    8/23/97 7:15p Rlove
 * 
 * 65    8/20/97 3:44p Mgummelt
 * 
 * 64    8/16/97 5:46p Mgummelt
 * 
 * 63    8/06/97 10:11p Mgummelt
 * 
 * 62    7/28/97 12:31p Rlove
 * Health doesn't count down as fast
 * 
 * 61    7/24/97 6:14p Rlove
 * Artifacts can no longer be used if the current one is still in use.
 * 
 * 60    7/24/97 3:53p Rlove
 * 
 * 59    7/24/97 11:29a Mgummelt
 * 
 * 58    7/24/97 3:26a Mgummelt
 * 
 * 57    7/21/97 3:03p Rlove
 * 
 * 56    7/19/97 2:30a Bgokey
 * 
 * 55    7/17/97 2:17p Mgummelt
 * 
 * 54    7/14/97 1:00p Mgummelt
 * 
 * 53    7/08/97 3:09p Rlove
 * 
 * 52    7/08/97 7:00a Rlove
 * 
 * 51    7/07/97 2:58p Mgummelt
 * 
 * 50    7/07/97 10:56a Mgummelt
 * 
 * 49    6/28/97 2:43p Rlove
 * 
 * 48    6/26/97 4:44p Rjohnson
 * Cheat gives you all artifacts
 * 
 * 47    6/20/97 9:43a Rlove
 * New mana system
 * 
 * 46    6/19/97 7:51a Rlove
 * 
 * 45    6/18/97 4:00p Mgummelt
 * 
 * 44    6/17/97 10:26a Rlove
 * 
 * 43    6/16/97 11:49p Bgokey
 * 
 * 42    6/16/97 6:40p Bgokey
 * 
 * 41    6/16/97 4:01p Rlove
 * 
 * 40    6/16/97 3:37p Rlove
 * Fixed the cheat so not everything is given (E3)
 * 
 * 39    6/16/97 3:00p Rlove
 * 
 * 38    6/13/97 10:11a Rlove
 * Moved all message.hc to strings.hc
 * 
 * 37    6/09/97 7:26a Rlove
 * Torch Artifact bit was being turned off without being checked if it was
 * on.
 * 
 * 36    6/06/97 2:52p Rlove
 * Artifact of Super Health now functions properly
 * 
 * 35    6/04/97 8:16p Mgummelt
 * 
 * 34    6/03/97 7:59a Rlove
 * Change take_art.wav to artpkup.wav
 * 
 * 33    5/27/97 3:52p Rjohnson
 * Added a generic item spawner
 * 
 * 32    5/23/97 12:22p Bgokey
 * 
 * 31    5/22/97 3:30p Mgummelt
 * 
 * 30    5/19/97 5:29p Rlove
 * 
 * 29    5/19/97 4:35p Rlove
 * 
 * 28    5/19/97 8:58a Rlove
 * Adding sprites and such to the axe.
 * 
 * 27    5/11/97 8:54p Mgummelt
 * 
 * 26    4/28/97 10:17a Rlove
 * New artifacts and items
 * 
 * 25    4/24/97 10:00p Rjohnson
 * Fixed problem with precache and spawning artifacts
 * 
 * 24    4/24/97 2:53p Rjohnson
 * Added backpack functionality and spawning of objects
 * 
 * 23    4/21/97 4:44p Rlove
 * Changed bounding box and color for artifacts
 * 
 * 22    4/15/97 8:46a Rlove
 * Weapon pick ups are working better.  Instant health is also working
 * 
 * 21    4/09/96 7:33p Mgummelt
 * 
 * 20    4/09/96 7:31p Mgummelt
 * 
 * 19    4/04/97 5:40p Rlove
 * 
 * 18    3/29/97 1:02p Aleggett
 * 
 * 17    2/13/97 3:22p Rlove
 * Blast Radius
 * 
 * 16    2/12/97 10:17a Rlove
 * 
 * 15    2/07/97 1:34p Rlove
 * Artifact of invincibility
 * 
 * 14    2/04/97 3:37p Rlove
 * Rewrote super health, made the code tighter
 * 
 * 13    2/04/97 3:05p Rlove
 * Rewrote the torch, doesn't use an entity anymore (what was I thinking?)
 * 
 * 12    12/18/96 3:14p Rlove
 * Mana system started
 * 
 * 11    12/18/96 11:50a Rlove
 * Changes for Health and Super Health use
 * 
 * 10    12/18/96 8:56a Rlove
 * Inventory screen is operational now
 * 
 * 9     12/16/96 12:42p Rlove
 * New gauntlets, artifacts, and inventory
 * 
 * 1     12/13/96 3:46p Rlove
 *
 * 8     12/09/96 4:02p Rlove
 * Inventory now keeps track of what the hero has
 *
 * 7     12/09/96 10:34a Rlove
 *
 * 6     12/02/96 8:47a Rlove
 * Added mana and blast radius artifacts
 *
 * 5     11/12/96 2:39p Rlove
 * Updates for the inventory system. Torch, HP boost and Super HP Boost.
 *
 * 4     11/11/96 1:03p Rlove
 * Put in Source Safe stuff
 */


