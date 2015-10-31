/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/artifact.hc,v 1.2 2007-02-07 16:56:55 sezero Exp $
 */




/*
 * artifact_touch() -- Called when an artifact is being touched.
 *                     Awards players random amounts of whatever they represent.
 */
void() SUB_regen;
void() StartItem;
void ring_touch(void);

void artifact_touch()
{
	float amount;

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

	// Take appropriate action
	if(self.netname == STR_TORCH)
	{
		if ((other.cnt_torch + 1)  > 15)
			return;	
		else
			other.cnt_torch += 1;		
	}
	else if(self.netname == STR_HEALTHBOOST)   // 25 limit
	{
		if ((other.cnt_h_boost + 1)  > 30||(other.playerclass!=CLASS_CRUSADER&&other.cnt_h_boost + 1  > 15))
			return;	
		else
			other.cnt_h_boost += 1; 
	}
	else if(self.netname == STR_SUPERHEALTHBOOST) // 5 limit
	{
		if (deathmatch&&(other.cnt_sh_boost + 1) > 2)
			return;	
		else if ((other.cnt_sh_boost + 1) > 5)
			return;	
		else
			other.cnt_sh_boost += 1; 
	}
	else if(self.netname == STR_MANABOOST)
	{
		if ((other.cnt_mana_boost + 1) > 15)
			return;	
		else
			other.cnt_mana_boost += 1; 
	}
	else if(self.netname == STR_TELEPORT)
	{
		if ((other.cnt_teleport + 1) > 15)
			return;	
		else
			other.cnt_teleport += 1;
	}
	else if(self.netname == STR_TOME)
	{
		if ((other.cnt_tome + 1)  > 15)
			return;	
		else
			other.cnt_tome += 1;
	}
	else if(self.netname == STR_SUMMON)
	{
		if ((other.cnt_summon + 1) > 15)
			return;	
		else
			other.cnt_summon += 1;
	}
	else if(self.netname == STR_INVISIBILITY)
	{
		if ((other.cnt_invisibility + 1) > 15)
			return;	
		else
			other.cnt_invisibility += 1;
	}
	else if(self.netname == STR_GLYPH)
	{
		if(other.playerclass==CLASS_CRUSADER)
		{
			if ((other.cnt_glyph + 5) > 50)
				return;	
			else	
				other.cnt_glyph += 5;
		}
		else
		{
			if ((other.cnt_glyph + 1) > 15)
				return;	
			else	
				other.cnt_glyph += 1;
		}
	}
	else if(self.netname == STR_HASTE)
	{
		if ((other.cnt_haste + 1)  > 15)
			return;	
		else
			other.cnt_haste += 1;
	}
	else if(self.netname == STR_BLAST)
	{
		if ((other.cnt_blast + 1)  > 15)
			return;	
		else
			other.cnt_blast += 1;
	}
	else if(self.netname == STR_POLYMORPH)
	{
		if ((other.cnt_polymorph + 1)  > 15)
			return;	
		else
			other.cnt_polymorph += 1;
	}
	else if(self.netname == STR_FLIGHT)
	{
		if ((other.cnt_flight + 1)  > 15)
			return;	
		else
			other.cnt_flight += 1;
	}
	else if(self.netname == STR_CUBEOFFORCE)
	{
		if ((other.cnt_cubeofforce + 1)  > 15)
			return;	
		else
			other.cnt_cubeofforce += 1;
	}
	else if(self.netname == STR_INVINCIBILITY)
	{
		if ((other.cnt_invincibility + 1)  > 15)
			return;	
		else
			other.cnt_invincibility += 1;
	}
	/*
	else if(self.classname == "art_sword_and_crown")
	{
		centerprint(other,"You are victorious!\n");
		bprint(other.netname);
		bprint(" has captured the Crown!\n");
	}
	*/

	amount = random();
	if (amount < 0.5)
	{
		sprint (other, STR_YOUPOSSESS);
		sprint (other, self.netname);
	}
	else
	{
		sprint (other, STR_YOUHAVEACQUIRED);
		sprint (other, self.netname);
	}

	sprint (other,"\n");

	if (self.artifact_respawn)
	{
		self.mdl = self.model;
		if(self.netname==STR_INVINCIBILITY)
			thinktime self : 120;
		else if(self.netname==STR_INVISIBILITY)
			thinktime self : 90;
		else
			thinktime self : 60;
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
void GenerateArtifactModel(string modelname,string art_name,float respawnflag) 
{
	if (respawnflag)	// Should this thing respawn
		self.artifact_respawn = deathmatch;
	setmodel(self, modelname);
	self.netname = art_name;

	if (modelname == "models/ringft.mdl")
	{
		self.netname = "Ring of Flight";
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
	else if (artifact == ARTIFACT_FLIGHT)
		GenerateArtifactModel("models/ringft.mdl",STR_FLIGHT,respawnflag);
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

	if(self.health<-100)
		self.health=1;
	else if(self.health<0)
		self.health+=100;
	else if (self.health < 899)
		self.health = self.health + 100;
	else if (self.health > 999)
		self.health = 999;

	self.cnt_sh_boost -= 1;
	self.artifact_flags(+)AFL_SUPERHEALTH;   // Show the health is in use
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

