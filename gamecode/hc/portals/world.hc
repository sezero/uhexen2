/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/world.hc,v 1.1.1.1 2004-11-29 11:36:03 sezero Exp $
 */

//void() InitBodyQue;


void() main =
{
// these are just commands the the prog compiler to copy these files

//	precache_file ("progs.dat");
	precache_file ("gfx.wad");
//	precache_file ("hexen.rc");
	precache_file4 ("default.cfg");	//want this new one in pak3

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

//!!just to build the pak files!!
	precache_file4("models/buddaL.mdl");
	precache_file4("models/buddaR.mdl");

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

    precache_file4("maps/keep1.bsp");
    precache_file4("maps/keep2.bsp");
    precache_file4("maps/keep3.bsp");
    precache_file4("maps/keep4.bsp");
    precache_file4("maps/keep5.bsp");

    precache_file4("midi/tulku1.mid");
    precache_file4("midi/tulku2.mid");
    precache_file4("midi/tulku3.mid");
    precache_file4("midi/tulku4.mid");
    precache_file4("midi/tulku5.mid");
    precache_file4("midi/tulku6.mid");
    precache_file4("midi/tulku7.mid");
    precache_file4("midi/tulku8.mid");
    precache_file4("midi/tulku9.mid");
    precache_file4("midi/tulku10.mid");

    precache_file4("maps/tibet1.bsp");
    precache_file4("maps/tibet2.bsp");
    precache_file4("maps/tibet3.bsp");
    precache_file4("maps/tibet4.bsp");
    precache_file4("maps/tibet5.bsp");
    precache_file4("maps/tibet6.bsp");
    precache_file4("maps/tibet7.bsp");
    precache_file4("maps/tibet8.bsp");
    precache_file4("maps/tibet9.bsp");
    precache_file4("maps/tibet10.bsp");

	precache_file4("t9.dem");
    precache_file4("maps/thomas.bsp");
    precache_file4("maps/monsters.bsp");
//  precache_file4("maps/sheephunt.bsp");
//  precache_file4("maps/mgallmon.bsp");
};


entity	lastspawn;

//=======================
/*QUAKED worldspawn (0 0 0) ? MissionPack 
Only used for the world entity.
Set message to the string index of the level name.
Set CD to the cd track to play.
Set MIDI to the midi file name
Check MissionPack to enable the enhanced door and plat code
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

// custom map attributes
	if (self.model == "maps/eidolon.bsp"||
		self.model == "maps/rider1a.bsp"||
		self.model == "maps/meso9.bsp"||
		self.model == "maps/rider2c.bsp"||
		self.model == "maps/romeric6.bsp"||
		self.model == "maps/mgtowers.bsp")
		cvar_set ("sv_sound_distance", "0");//Small levels, don't cut sounds
	else
		cvar_set ("sv_sound_distance", "800");

	if (self.model == "maps/mgtowers.bsp")
	{
		cvar_set ("sv_gravity", "100");
		cvar_set ("r_ambient", "36");
	}
	else
		cvar_set ("sv_gravity", "800");

// the area based ambient sounds MUST be the first precache_sounds
	W_Precache ();			// get weapon precaches

	Precache_wav ();		// Precache Raven wavs 

	Precache_misc();

	if(deathmatch || coop)
	{
//		dprint("All classes precached\n");
		Precache_Paladin();
		Precache_Crusader();
		Precache_Necromancer();
		Precache_Assassin();
		Precache_Demoness();
	}
	else
	{
		switch (cl_playerclass)
		{
		case 1:
//			dprintf("Precache_Paladin cl_playerclass %s\n", cl_playerclass);
			Precache_Paladin();
		break;
		case 2:
//			dprintf("Precache_Crusader cl_playerclass %s\n", cl_playerclass);
			Precache_Crusader();
		break;
		case 3:
//			dprintf("Precache_Necromancer cl_playerclass %s\n", cl_playerclass);
			Precache_Necromancer();
		break;
		case 4:
//			dprintf("Precache_Assassin cl_playerclass %s\n", cl_playerclass);
			Precache_Assassin();
		break;
		case 5:
//			dprintf("Precache_Demoness cl_playerclass %s\n", cl_playerclass);
			Precache_Demoness();
		break;
		default:
			dprintf("cl_playerclass %s is invalid\n", cl_playerclass);
			objerror("You must choose a playerclass!\n");
		break;
		}
	}

	Precache_spr ();		// Precache Raven sprites

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

void() StartFrame =
{
	teamplay = cvar("teamplay");
	skill = cvar("skill");
	framecount = framecount + 1;
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

