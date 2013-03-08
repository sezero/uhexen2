/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/world.hc,v 1.6 2006-10-31 19:31:48 sezero Exp $
 */

//void() InitBodyQue;


void() main =
{
// these are just commands the the prog compiler to copy these files

//	precache_file ("progs.dat");
//	precache_file2 ("progs2.dat");
	precache_file ("gfx.wad");
//	precache_file ("hexen.rc");
	precache_file ("default.cfg");

//	precache_file ("end1.bin");
//	precache_file2 ("end2.bin");

//	precache_file ("demo1.dem");
//	precache_file ("demo2.dem");
//	precache_file ("demo3.dem");

	Precache_lmp();

// sounds loaded by C code
	precache_sound ("raven/menu1.wav");
	precache_sound ("raven/menu2.wav");
	precache_sound ("raven/menu3.wav");

	precache_sound("misc/barmovup.wav");
	precache_sound("misc/barmovdn.wav");
	precache_sound("misc/invmove.wav");
	precache_sound("misc/invuse.wav");

	precache_sound ("ambience/water1.wav");
	precache_sound ("ambience/wind2.wav");
	
// demo
	precache_file("maps/demo1.bsp");
	precache_file("maps/demo2.bsp");
	precache_file("maps/demo3.bsp");
	precache_file("maps/ravdm1.bsp");

//demo midi
	precache_file("midi/casa1.mid");	//Demo1
	precache_file("midi/casa2.mid");	//Demo2
	precache_file("midi/egyp3.mid");	//Ravdm1

// registered
	precache_file2 ("gfx/pop.lmp");
    precache_file2("maps/castle4.bsp");
    precache_file2("maps/castle5.bsp");
    precache_file2("maps/cath.bsp");
    precache_file2("maps/egypt1.bsp");
    precache_file2("maps/egypt2.bsp");
    precache_file2("maps/egypt3.bsp");
    precache_file2("maps/egypt4.bsp");
    precache_file2("maps/egypt5.bsp");
    precache_file2("maps/egypt6.bsp");
    precache_file2("maps/egypt7.bsp");
    precache_file2("maps/meso1.bsp");
    precache_file2("maps/meso2.bsp");
    precache_file2("maps/meso3.bsp");
    precache_file2("maps/meso4.bsp");
    precache_file2("maps/meso5.bsp");
    precache_file2("maps/meso6.bsp");
    precache_file2("maps/meso8.bsp");
    precache_file2("maps/meso9.bsp"); // death rider
    precache_file3("maps/rider1a.bsp"); // famine rider
    precache_file2("maps/rider2c.bsp"); // pestilence rider
    precache_file2("maps/eidolon.bsp");
    precache_file2("maps/romeric1.bsp");
    precache_file2("maps/romeric2.bsp");
    precache_file2("maps/romeric3.bsp");
    precache_file2("maps/romeric4.bsp");
    precache_file2("maps/romeric5.bsp");
    precache_file2("maps/romeric6.bsp"); // war rider
    precache_file2("maps/romeric7.bsp"); 
    precache_file2("maps/tower.bsp");
    precache_file3("maps/village1.bsp");
    precache_file3("maps/village2.bsp");
    precache_file3("maps/village3.bsp");
    precache_file3("maps/village4.bsp");
    precache_file3("maps/village5.bsp");
    precache_file2("maps/ravdm2.bsp");
    precache_file2("maps/ravdm3.bsp");
    precache_file2("maps/ravdm4.bsp");
    precache_file2("maps/ravdm5.bsp");

    precache_file3("midi/casa1.mid");
    precache_file3("midi/casa2.mid");
    precache_file3("midi/casa3.mid");
    precache_file3("midi/casa4.mid");
    precache_file2("midi/casb1.mid");
    precache_file2("midi/casb2.mid");
    precache_file2("midi/casb3.mid");
    precache_file2("midi/egyp1.mid");
    precache_file2("midi/egyp2.mid");
    precache_file2("midi/egyp3.mid");
    precache_file2("midi/meso1.mid");
    precache_file2("midi/meso2.mid");
    precache_file2("midi/meso3.mid");
    precache_file2("midi/roma1.mid");
    precache_file2("midi/roma2.mid");
    precache_file2("midi/roma3.mid");
};


entity	lastspawn;

//=======================
/*QUAKED worldspawn (0 0 0) ? MissionPack Siege
Only used for the world entity.
Set message to the string index of the level name.
Set CD to the cd track to play.
Set MIDI to the midi file name
Check "MissionPack" to enable the enhanced door and plat code
Check "Siege" to enable Siege code
-------------------------FIELDS-------------------------
target = sheep enables the Sheep Hunter code
worldtype - determines which skin obj_pots use
0 - castle
1 - egypt
2 - meso
3 - roman
--------------------------------------------------------
*/
//=======================
void() worldspawn =
{
	lastspawn = world;

	if(self.spawnflags&WSF_SIEGE)
		dmMode=DM_SIEGE;
	newsiege = TRUE;
//	newsiege = FALSE;
	gamestarted = FALSE;

//	InitBodyQue ();

// custom map attributes
/*	don't want hardcoded values for hexenworld
	if (self.model == "maps/mgtowers.bsp")
	{
		cvar_set ("sv_gravity", "100");
		cvar_set ("r_ambient", "36");
	}
	else
		cvar_set ("sv_gravity", "800");
*/

// the area based ambient sounds MUST be the first precache_sounds

// player precaches	
	W_Precache ();			// get weapon precaches

	Precache_wav ();		// Precache Raven wavs 

	Precache_Id_mdl ();		// Precache Id models

	Precache_mdl ();		// Precache Raven models
	Precache_Demoness ();		// Precache Demoness

	// FIXME: these need to be added to Precache_mdl
	precache_model("models/schunk1.mdl");
	precache_model("models/schunk2.mdl");
	precache_model("models/schunk3.mdl");
	precache_model("models/schunk4.mdl");

	Precache_spr ();		// Precache Raven sprites

	Precache_hw();

//
// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
//

	// 0 normal
	lightstyle(0, "m");

	// 1 FLICKER (first variety)
	lightstyle(1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	lightstyle(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	lightstyle(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
//lightstyle(4, "mamamamamama");
	lightstyle(4, "zzzzzzzzzzzz");

	// 5 GENTLE PULSE 1
	lightstyle(5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 TORCHLIGHT (second variety)
	lightstyle(EF_TORCHLIGHT, "knmonqnmolm");
	
	// 7 CANDLE (second variety)
	lightstyle(7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	lightstyle(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	lightstyle(9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	lightstyle(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	lightstyle(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");


	// Light styles 25-30 are Model Light Styles

	// 25: MLS_FULLBRIGHT(1)
	lightstyle(25, "z");
	// 26: MLS_POWERMODE(2)
	lightstyle(26, "2jklmnooppqqrrrrqqppoonmlkj");
	// 27: MLS_TORCH(3)
	lightstyle(27, "wvtwyzxvtxwyzwv");
	// 28: MLS_FIREFLICKER(4)
	lightstyle(28, "jilghmnohjljhfpjnjgpohkjhioomh");
	// 29: MLS_CRYSTALGOLEM(5)
	lightstyle(29, "2jklmnooppqqrrrrqqppoonmlkj");


	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	lightstyle(63, "a");

	StringsInit(); // Initialize strings -- temporary
};

void (entity spawnSpot, float persists) spawnNewDmToken;

void UpdateCTT(void)
{
	entity	token, curPlayer, spawnSpot, tokenHolder;
	float	tokenInUse = 0;
	vector	tokenLine, curAngs;

	token = find(world, netname, "artifact");
	while(token != world)
	{	// get rid of leftover old icons - this is highly inappropriate...
		if(token.artifact_name == STR_INVINCIBILITY)
		{
			if(token.classname != "dmMode1_token")
			{
				remove(token);
			}
			//curPlayer = token; // store it away so the search will work correctly
		}
		token = find(token, netname, "artifact");

		//remove(curPlayer);
	}

	curPlayer=find(world, classname, "player");
	while((curPlayer != world)&&(tokenInUse == 0))
	{
		if(curPlayer.gameFlags & GF_HAS_TOKEN)
		{
			tokenInUse = 1;
			tokenHolder = curPlayer;
		}
		curPlayer = find(curPlayer, classname, "player");
	}

	if(tokenInUse == 0)
	{
		token = find(world,classname,"dmMode1_token");
		if(!token)
		{	// couldn't find a token in the world or in an inventory...
			spawnSpot = SelectSpawnPoint();
			spawnNewDmToken(spawnSpot, 1);
		}
		else
		{
			tokenHolder = token;
		}
	}

	curPlayer = find(world, classname, "player");
	while(curPlayer != world)
	{
		if(curPlayer.gameFlags & GF_HAS_TOKEN)
		{
			curPlayer.targDist = 0;
		}
		else
		{	// update this player's targeting stuff...
			tokenLine = tokenHolder.origin - curPlayer.origin;
			curAngs = vectoangles(tokenLine);

			curPlayer.targDist = vlen(tokenLine) / 8.0;
			curPlayer.targAng = curAngs_y * 256.0 / 360.0;
			curPlayer.targPitch = curAngs_x * 256.0 / 360.0;
		}
		curPlayer = find(curPlayer, classname, "player");
	}
}

void GetRandomTarget(entity source)
{
	entity curPlayer;
	float	playerCount = 0;
	float	nextChoice, curCount;

	curPlayer = find(world, classname, "player");
	while(curPlayer != world)
	{
		if(!(curPlayer.gameFlags & GF_HAS_LEFT_HUNTER))
		{
			playerCount += 1;
		}
		curPlayer = find(curPlayer, classname, "player");
	}

	while((source.targetPlayer == world)||(source.targetPlayer == source)||(source.targetPlayer.gameFlags & GF_HAS_LEFT_HUNTER))
	{
		curCount = 0;
		nextChoice = rint(random(0, playerCount - 1));
		curPlayer = find(curPlayer, classname, "player");
		
		while(curPlayer.gameFlags & GF_HAS_LEFT_HUNTER)
		{	// make sure the first one is valid...
			curPlayer = find(curPlayer, classname, "player");
		}
		while(curCount < nextChoice)
		{
			curPlayer = find(curPlayer, classname, "player");
			if(!(curPlayer.gameFlags & GF_HAS_LEFT_HUNTER))
			{
				curCount += 1;
			}
		}
		source.targetPlayer = curPlayer;
	}
}

void UpdateHunter(void)
{	
	entity curPlayer;
	vector	destLine, curAngs;
	float playerCount;

	// update the target info...
	curPlayer = find(world, classname, "player");
	while(curPlayer != world)
	{
		if(!(curPlayer.gameFlags & GF_HAS_LEFT_HUNTER))
		{
			if(curPlayer.targetPlayer != world)
			{
				destLine = curPlayer.targetPlayer.origin - curPlayer.origin;
				curAngs = vectoangles(destLine);

				curPlayer.targDist = vlen(destLine) / 8.0;
				curPlayer.targAng = curAngs_y * 256.0 / 360.0;
				curPlayer.targPitch = curAngs_x * 256.0 / 360.0;
			}
			playerCount += 1;
		}
		curPlayer = find(curPlayer, classname, "player");
	}

	// make certain everyone has a target...
	if(playerCount > 1)
	{
		curPlayer = find(world, classname, "player");
		while(curPlayer != world)
		{
			if(!(curPlayer.gameFlags & GF_HAS_LEFT_HUNTER))
			{
				if(curPlayer.targetPlayer == world)
				{
					GetRandomTarget(curPlayer);
					centerprint2(curPlayer, "Your new target is ",curPlayer.targetPlayer.netname);
				}
			}
			curPlayer = find(curPlayer, classname, "player");
		}
	}
}

void() StartFrame =
{
	teamplay = cvar("teamplay");
	skill = cvar("skill");
	damageScale = cvar("damagescale");
	meleeDamScale = cvar("meleeDamScale");
	shyRespawn = cvar("shyRespawn");
	spartanPrint = cvar("spartanPrint");
	manaScale = cvar("manascale");
	tomeMode = cvar("tomemode");
	tomeRespawn = cvar("tomerespawn");
	w2Respawn = cvar("w2respawn");
	altRespawn = cvar("altrespawn");
	fixedLevel = cvar("fixedlevel");
	autoItems = cvar("autoitems");
	dmMode = cvar("dmmode");
	easyFourth = cvar("easyfourth");
	patternRunner = cvar("patternrunner");
	framecount = framecount + 1;

	if(dmMode == DM_CAPTURE_THE_TOKEN)
	{
		UpdateCTT();
	}
	else if(dmMode == DM_HUNTER)
	{
		UpdateHunter();
	}
};

/*
==============================================================================

BODY QUE

==============================================================================
*/
/*
entity	bodyque_head;

void() bodyque =
{	// just here so spawn functions don't complain after the world
	// creates bodyques
};

void() InitBodyQue =
{
	local entity	e;
	
	bodyque_head = spawn();
	bodyque_head.classname = "bodyque";
	bodyque_head.owner = spawn();
	bodyque_head.owner.classname = "bodyque";
	bodyque_head.owner.owner = spawn();
	bodyque_head.owner.owner.classname = "bodyque";
	bodyque_head.owner.owner.owner = spawn();
	bodyque_head.owner.owner.owner.classname = "bodyque";
	bodyque_head.owner.owner.owner.owner = bodyque_head;
};


// make a body que entry for the given ent so the ent can be
// respawned elsewhere
void(entity ent) CopyToBodyQue =
{
	bodyque_head.angles = ent.angles;
	bodyque_head.model = ent.model;
	bodyque_head.modelindex = ent.modelindex;
	bodyque_head.frame = ent.frame;
	bodyque_head.colormap = ent.colormap;
	bodyque_head.movetype = ent.movetype;
	bodyque_head.velocity = ent.velocity;
	bodyque_head.flags = 0;
	setorigin (bodyque_head, ent.origin);
	setsize (bodyque_head, ent.mins, ent.maxs);
	bodyque_head = bodyque_head.owner;
};*/

