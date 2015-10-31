/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/artifact.hc,v 1.2 2007-02-07 16:57:49 sezero Exp $
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
//	if(other.classname != "player"||other.model=="models/sheep.mdl")
	if(other.classname != "player")
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

	if ((dmMode == DM_CAPTURE_THE_TOKEN) && (other.gameFlags & GF_HAS_TOKEN))
	{
		return;
	}

	if (roomForItem(other,self.artifact_name)<=0)
		return;

	adjustInventoryCount(other,self.artifact_name,1);

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
	else
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
			if(scaleVal < .2)
			{
				scaleVal = .2;
			}
		}

		if(self.artifact_name==STR_INVINCIBILITY)
			thinktime self : 240;
		else if(self.artifact_name==STR_INVISIBILITY)
			thinktime self : 90*scaleVal;
		else
			thinktime self : 60*scaleVal;
		self.think = SUB_regen;
	}

	sound(other, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
	stuffcmd(other, "bf\n");
	self.solid = SOLID_NOT;
//	other.items (+) self.items;
	self.model = string_null;

	activator = other;
	SUB_UseTargets(); // Fire all targets / killtargets

	if(!self.artifact_respawn)
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
	else if (artifact == ARTIFACT_INVISIBILITY)
		GenerateArtifactModel("models/a_invis.mdl",STR_INVISIBILITY,respawnflag);
	else if (artifact == ARTIFACT_INVINCIBILITY)
		GenerateArtifactModel("models/a_invinc.mdl",STR_INVINCIBILITY,respawnflag);
	else if (artifact == ARTIFACT_CUBEOFFORCE)
		GenerateArtifactModel("models/a_cube.mdl",STR_CUBEOFFORCE,respawnflag);
	else if (artifact == ARTIFACT_SUMMON)
		GenerateArtifactModel("models/a_summon.mdl",STR_SUMMON,respawnflag);
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
	self.torchtime = time + 7;
	self.torchthink = KillTorch;
}


void FullTorch()
{
	sound (self, CHAN_BODY, "raven/fire1.wav", 1, ATTN_NORM);
	self.effects(+)EF_TORCHLIGHT;
	self.torchtime = time + 23;
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
		self.cnt_torch		-= 1;
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
	spawn_artifact(ARTIFACT_INVISIBILITY,RESPAWN);
}

/*
void spawn_art_sword_and_crown(void)
{
	setmodel(self, "models/xcalibur.mdl");
	self.netname = "Sword";
	self.touch	 = artifact_touch;
	setsize (self, '-8 -8 -44', '8 8 20');

	StartItem();
}
*/
/*QUAK-ED art_sword_and_crown (.0 .0 .5) (-8 -8 -44) (8 8 20) FLOATING
Artifact for Sword and Crown
-------------------------FIELDS-------------------------
None
--------------------------------------------------------
*/
/*void art_sword_and_crown()
{
	precache_model2("models/xcalibur.mdl");
	self.artifact_respawn = deathmatch;
	spawn_art_sword_and_crown();
}
*/

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

