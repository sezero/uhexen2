/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/spawn.hc,v 1.2 2007-02-07 16:57:10 sezero Exp $
 */

float ENT_WORLD					= 0;
float ENT_CHAIR					= 1;
float ENT_BARSTOOL				= 2;
float ENT_BOOKOPEN				= 3;
float ENT_BOOKCLOSED			= 4;
float ENT_TREEDEAD				= 5;
float ENT_TREE					= 6;
float ENT_TREETOP				= 7;
float ENT_CART					= 8;
float ENT_CHEST1				= 9;
float ENT_CHEST2				= 10;
float ENT_BOULDER				= 11;
float ENT_SWORD					= 12;
float ENT_BALLISTA				= 13;
float ENT_BELL					= 14;
float ENT_STATUE_MUMMYHEAD		= 15;
float ENT_POT1					= 16;
float ENT_POT2					= 17;
float ENT_POT3					= 18;
float ENT_STATUE_TUT			= 19;
float ENT_FLAG					= 20;
float ENT_STATUE_SNAKE			= 21;
float ENT_HEDGE1				= 22;
float ENT_HEDGE2				= 23;
float ENT_HEDGE3				= 24;
float ENT_FOUNTAIN				= 25;
float ENT_BUSH1					= 26;
float ENT_TOMBSTONE1			= 27;
float ENT_TOMBSTONE2			= 28;
float ENT_CAULDRON				= 29;
float ENT_SKULLSTICK			= 30;
float ENT_BEEFSLAB				= 31;
float ENT_SEAWEED				= 32;
float ENT_MUMMY					= 33;
float ENT_ARCHER				= 34;
float ENT_MUMMY_ARM				= 35;
float ENT_MUMMY_LEG				= 36;
float ENT_MUMMY_FIRE			= 37;
float ENT_MUMMY_MISSILE			= 38;
float ENT_WEAPON42_ART			= 39;
float ENT_WEAPON41_ART			= 40;
float ENT_WEAPON3_ART			= 41;
float ENT_WEAPON2_ART			= 42;
float ENT_SWIPE					= 43;
float ENT_VORP_MISSILE			= 44;
float ENT_AXE_BLADE				= 45;
float ENT_STATUE_LION			= 46;
float ENT_CORPSE2				= 47;
float ENT_CORPSE1				= 48;
float ENT_BENCH					= 49;
float ENT_FENCE					= 50;
float ENT_STATUE_ANGEL			= 51;
float ENT_SHEEP					= 52;
float ENT_BARREL				= 53;
float ENT_WEB					= 54;
float ENT_STATUE_MUMMY_HEAD		= 55;
float ENT_STATUE_MUMMY_BODY		= 56;
float ENT_GLYPH					= 57;
float ENT_FANGEL				= 58;
float ENT_STATUE_ATHENA			= 59;
float ENT_STATUE_NEPTUNE		= 60;
float ENT_BONEPILE				= 61;
float ENT_CHEST3  				= 62;
float ENT_STATUE_CAESAR			= 63;
float ENT_TELEPORT     			= 64;
float ENT_STATUE_SNAKE_COIL		= 65;
float ENT_SKULL					= 66;
float ENT_PEW					= 67;
float ENT_STATUE_OLMEC			= 68;
float ENT_STATUE_MARS			= 69;
float ENT_PLAYERHEAD			= 70;
float ENT_STATUE_KING			= 71;
float ENT_PLANT_GENERIC			= 72;
float ENT_PLANT_MESO			= 73;
float ENT_PLANT_ROME			= 74;
float ENT_FANGEL_HEAD			= 75;


float SPAWNVALU_SIZE	= 6;	// Number of fields for each entity in entity_spawnvalues array

float MASS_OFS			= 1;
float MOVETYPE_OFS		= 2;
float SOLIDTYPE_OFS		= 3;
float THINGTYPE_OFS		= 4;
float DAMAGETYPE_OFS	= 5;


float entity_spawnvalues[456] =
{  // Health    Mass    Movetype         Solid    Thingtype        DamageType
		25,		3,		0,				0,				2,				1,	// ENT_WORLD		
		25,		3,	   13,				3,				2,				1,	// ENT_CHAIR
		25,		3,	   13,				3,				2,				1,	// ENT_BARSTOOL
		20,		0,		0,				3,				1,				1,	// ENT_BOOKOPEN
		20,		0,		0,				3,				1,				1,	// ENT_BOOKCLOSED
	  1000,	99999,		0,				2,				2,				1,	// ENT_TREEDEAD
	  1000,	99999,		0,				2,			   11,				1,	// ENT_TREE
	  1000,	99999,		0,				2,			    7,				1,	// ENT_TREETOP
		25,		7,		4,				3,				2,				1,	// ENT_CART
		25,	   10,		4,				3,				2,				1,  // ENT_CHEST1
		25,	   10,		4,				3,				2,				1,	// ENT_CHEST2
		75,	  200,	   10,				2,				1,				1,	// ENT_BOULDER
		50,	    1,	    4,				3,				3,				1,	// ENT_SWORD
		 0,	99999,	    4,				3,				2,				1,	// ENT_BALLISTA
	   250,	 1000,	    0,				3,				3,				1,	// ENT_BELL
	   200,	   15,	    4,				3,				9,				1,	// ENT_STATUE_MUMMYHEAD
	    10,	  100,	    4,				3,				6,				1,	// ENT_POT1
	    10,	  100,	    4,				3,				6,				1,	// ENT_POT2
	    10,	  100,	    4,				3,				6,				1,	// ENT_POT3
	  1000,	 2000,	    4,				3,				9,				1,	// ENT_STATUE_TUT
	    50,	 1000,	    0,				3,			   15,				1,	// ENT_FLAG
	   100,	  200,	    0,				3,			    9,				1,	// ENT_STATUE_SNAKE
	    20,	  200,	    0,				3,			   11,				1,	// ENT_HEDGE1
	    20,	  200,	    0,				3,			   11,				1,	// ENT_HEDGE2
	    20,	  200,	    0,				3,			   11,				1,	// ENT_HEDGE3
	    20,	  200,	    0,				3,			    1,				1,	// ENT_FOUNTAIN
	    20,	  200,	    0,				3,			   11,				1,	// ENT_BUSH1
	    20,	  200,	    0,				3,			    1,				1,	// ENT_TOMBSTONE1
	    20,	  200,	    0,				3,			    1,				1,	// ENT_TOMBSTONE1
	    50,	   15,	    4,				3,			    3,				1,	// ENT_CAULDRON
	    20,	   15,	    0,				3,			    2,				1,	// ENT_SKULLSTICK
	    50,	   15,	    0,				3,			    4,				1,	// ENT_BEEFSLAB
	     0,	    0,	    0,				0,			    7,				1,	// ENT_SEAWEED
	   150,	    8,	    4,				3,			    4,				1,	// ENT_MUMMY
	    80,	   12,	    4,				3,			    4,				1,	// ENT_ARCHER
	     0,	    8,	   10,				0,			    4,				1,	// ENT_MUMMY_ARM
	     0,	    8,	   10,				0,			    4,				1,	// ENT_MUMMY_LEG
	     0,	    0,	    0,				1,			    5,				0,	// ENT_MUMMY_FIRE
	     0,	    0,	    9,				2,			    5,				0,	// ENT_MUMMY_MISSILE
	     0,	    0,	    0,				1,			    3,				0,	// ENT_WEAPON42_ART
	     0,	    0,	    0,				1,			    3,				0,	// ENT_WEAPON41_ART
	     0,	    0,	    0,				1,			    3,				0,	// ENT_WEAPON43_ART
	     0,	    0,	    0,				1,			    3,				0,	// ENT_WEAPON42_ART
	     0,	    0,	    0,				0,			    1,				0,	// ENT_SWIPE
	     0,	    0,	    9,				2,			    1,				0,	// ENT_VORP_MISSILE
	     0,	    0,     11,				5,			    1,				0,	// ENT_AXE_BLADE
	   100,	  200,	    0,				3,			    1,				1,	// ENT_STATUE_LION
	    20,	  200,	    4,				2,			    4,				1,	// ENT_CORPSE2
	    20,	  200,	    4,				2,			    4,				1,	// ENT_CORPSE1
	    25,	    8,	    4,				3,			    2,				1,	// ENT_BENCH
	    20,	    0,	    0,				3,			    3,				1,	// ENT_FENCE
	   150,	  500,	    0,				3,			    1,				1,	// ENT_STATUE_ANGEL
	    25,	    3,	    4,				3,			    4,				1,	// ENT_SHEEP
	    24,	   20,	   13,				3,			   12,				1,	// ENT_BARREL
	     0,	    0,	    0,				0,			   16,				0,	// ENT_WEB
	   200,	  150,	    0,				3,			    9,				1,	// ENT_STATUE_MUMMY_HEAD
	   200,	  150,	    0,				3,			    9,				1,	// ENT_STATUE_MUMMY_BODY
		 0,	    0,	    9,				2,			    1,				0,	// ENT_GLYPH
	   100,	   10,	    5,				3,			    4,				1,	// ENT_FANGEL
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_ATHENA
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_NEPTUNE
	    50,	    0,	    0,				3,			    1,				1,	// ENT_BONEPILE
		25,	   10,		4,				3,				2,				1,  // ENT_CHEST3
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_CAESAR
	     0,	    0,	    4,				0,			    1,				0,	// ENT_TELEPORT
	   200,	 9999,	    0,				3,			    1,				1,	// ENT_STATUE_SNAKE_COIL
	    10,	    3,	    0,				3,			    6,				1,	// ENT_SKULL
	    50,	    8,	    4,				3,			    2,				1,	// ENT_PEW
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_OLMEC
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_MARS
	    20,	    0,	    4,				3,			    4,				1,	// ENT_PLAYERHEAD
	   200,	    0,	    0,				3,			    1,				1,	// ENT_STATUE_KING
	    10,	 9999,	    0,				3,				7,				1,	// ENT_PLANT_GENERIC
	    10,	   20,	    4,				3,				6,				1,	// ENT_PLANT_MESO
	    10,	   50,	    4,				3,				6,				1,	// ENT_PLANT_ROME
	    10,	    3,	   10,				3,			    4,				1	// ENT_FANGEL_HEAD
}; 


float BOX_SIZE	= 2;	// Number of fields for each entity in entity_box array
float BMAX_OFS	= 1;

vector entity_box[152] =
{   // Min             Maxs
	'   0    0    0',	'  0   0   0',			// ENT_WORLD
	' -10  -10    0',	' 10  10  40',			// ENT_CHAIR
	' -10  -10    0',	' 10  10  32',			// ENT_BARSTOOL
	'  -8   -8    0',	'  8   8  10',			// ENT_BOOKOPEN
	'  -8   -8    0',	'  8   8  10',			// ENT_BOOKCLOSED
	' -42  -42    0',	' 42  42 160',			// ENT_TREEDEAD
	' -32  -32  -16',	' 32  32 210',			// ENT_TREE
	'-140 -140  -16',	'140 140 120',			// ENT_TREETOP
	' -36  -32  -10',	' 36  75  64',			// ENT_CART
	' -16  -16  -0',	' 16  16  32',			// ENT_CHEST1
	' -16  -16  -0',	' 16  16  32',			// ENT_CHEST2
	' -24  -24  -16',	' 24  24  16',			// ENT_BOULDER
	' -16  -16   -8',	' 16  16   8',			// ENT_SWORD
	' -45  -45    0',	' 45  45  60',			// ENT_BALLISTA
	'-100 -100 -205',	'100 100   8',			// ENT_BELL
	' -24  -24    0',	' 24  24  50',			// ENT_STATUE_MUMMYHEAD
	' -24  -24    0',	' 24  24  60',			// ENT_POT1
	' -16  -16    0',	' 16  16  40',			// ENT_POT2
	' -16  -16    0',	' 16  16  40',			// ENT_POT3
	' -36  -36    0',	' 36  36 248',			// ENT_STATUE_TUT
	' -16  -16    0',	' 16  16 160',			// ENT_FLAG
	' -16  -16    0',	' 16  16  80',			// ENT_STATUE_SNAKE
	' -24  -24    0',	' 24  24  80',			// ENT_HEDGE1
	' -24  -24    0',	' 24  24  80',			// ENT_HEDGE2
	' -24  -24    0',	' 24  24 120',			// ENT_HEDGE3
	' -24  -24    0',	' 24  24  80',			// ENT_FOUNTAIN
	' -16  -16    0',	' 16  16  40',			// ENT_BUSH1
	' -24  -24    0',	' 24  24  60',			// ENT_TOMBSTONE1
	' -16  -16    0',	' 16  16  40',			// ENT_TOMBSTONE2
	' -16  -16    0',	' 16  16  40',			// ENT_CAULDRON
	' -16  -16    0',	' 16  16  40',			// ENT_SKULLSTICK
	' -16  -16    0',	' 16  16  40',			// ENT_BEEFSLAB
	'  -8   -8    0',	'  8   8  32',			// ENT_SEAWEED
	' -16  -16    0',	' 16  16  50',			// ENT_MUMMY
	' -16  -16    0',	' 16  16  50',			// ENT_ARCHER
	'   0    0    0',	'  0   0   0',			// ENT_MUMMY_ARM
	'   0    0    0',	'  0   0   0',			// ENT_MUMMY_LEG
	'  -8   -8    0',	'  8   8  16',			// ENT_MUMMY_FIRE
	'   0    0    0',	'  0   0   0',			// ENT_MUMMY_MISSILE
	'  -8   -8  -44',	'  8   8  20',			// ENT_WEAPON42_ART
	'  -8   -8  -44',	'  8   8  20',			// ENT_WEAPON41_ART
	'  -8   -8  -44',	'  8   8  20',			// ENT_WEAPON3_ART
	'  -8   -8  -44',	'  8   8  20',			// ENT_WEAPON2_ART
	'   0    0    0',	'  0   0   0',			// ENT_SWIPE
	'   0    0    0',	'  0   0   0',			// ENT_VORP_MISSILE
	'   0    0    0',	'  0   0   0',			// ENT_AXE_BLADE
	' -56  -14    0',	' 56  14  60',			// ENT_STATUE_LION
	' -32  -16    0',	' 32  16  10',			// ENT_CORPSE2
	' -32  -16    0',	' 32  16  10',			// ENT_CORPSE1
	' -10  -30    0',	' 10  30  40',			// ENT_BENCH
	' -10  -26    0',	' 10  26  70',			// ENT_FENCE
	' -60  -40    0',	' 60  40 120',			// ENT_STATUE_ANGEL
	' -16  -16    0',	' 16  16  32',			// ENT_SHEEP
	' -13  -13    0',	' 13  13  36',			// ENT_BARREL
	' -25  -25  -25',	' 25  25  25',			// ENT_WEB
	' -26  -26    0',	' 26  26  30',			// ENT_STATUE_MUMMY_HEAD
	' -26  -26    0',	' 26  26 130',			// ENT_STATUE_MUMMY_BODY
	'   0    0    0',	'  0   0   0',			// ENT_GLYPH
	' -14  -14  -41',	' 14  14  23',			// ENT_FANGEL
	' -30  -30    0',	' 30  30  90',			// ENT_STATUE_ATHENA
	' -30  -30    0',	' 30  30 100',			// ENT_STATUE_NEPTUNE
	' -10  -10    0',	' 10  10  10',			// ENT_BONEPILE
	' -16  -16    0',	' 16  16  32',			// ENT_CHEST3
	' -16  -16    0',	' 16  16  90',			// ENT_STATUE_CAESAR
	'   0    0    0',	'  0   0   0',			// ENT_TELEPORT
	' -44  -44    0',	' 44  44  90',			// ENT_STATUE_SNAKE_COIL
	'  -8   -8    0',	'  8   8  16',			// ENT_SKULL
	'  -16 -40   0',	' 16  40  50',			// ENT_PEW
	' -40  -40    0',	' 40  40 130',			// ENT_STATUE_OLMEC
	' -30  -30    0',	' 30  30  80',			// ENT_STATUE_MARS
	'  -8   -8    0',	'  8   8  16',			// ENT_PLAYERHEAD_PALADIN
	' -30  -30    0',	' 30  30 120',			// ENT_STATUE_KING
	' -10  -10    0',	' 10  10  20',			// ENT_PLANT_GENERIC
	' -10  -10    0',	' 10  10  40',			// ENT_PLANT_MESO
	' -24  -24    0',	' 24  24  90',			// ENT_PLANT_ROME
	'  -8   -8    0',	'  8   8  16'			// ENT_FANGEL_SKULL
};

void(vector org) spawn_tfog;

void ScaleBoundingBox(float newscale, entity subject,float mass_flag)
{

	if (!newscale)
		return;

	if (newscale > 2.5)
	{
		dprint("Scale can't be bigger than 2.5");
		newscale = subject.scale;
	}
	else if (newscale < 0.01)
	{
		dprint("Scale can't be less than .01");
		newscale = subject.scale;
	}
		
	// FIXME: need to add a change in hulls too
	
	self.mins=self.orgnl_mins;
	self.maxs=self.orgnl_maxs;

	self.mins = self.mins * newscale;
	self.maxs = self.maxs * newscale;
	setsize (subject, self.mins, self.maxs);

	if (!mass_flag)	// User didn't specify mass so scale mass
		self.mass = self.mass * newscale;
}

void CreateEntityNew(entity subject,float entity_id,string modelname,void() death_func)
{
	float index1,index2;
	vector hold_vect;
	float mass_flag;

	setmodel(subject, modelname);

	index1 = SPAWNVALU_SIZE * entity_id;

	if (!subject.health)
		subject.health = entity_spawnvalues[index1];
 
	if (!subject.mass)
	{
		mass_flag = 0;
		subject.mass = entity_spawnvalues[index1 + MASS_OFS];
	}
	else
		mass_flag = 1;

	subject.movetype = entity_spawnvalues[index1 + MOVETYPE_OFS];
	subject.solid = entity_spawnvalues[index1 + SOLIDTYPE_OFS];
	subject.thingtype = entity_spawnvalues[index1 + THINGTYPE_OFS];
	subject.takedamage = entity_spawnvalues[index1 + DAMAGETYPE_OFS];


	index2 = BOX_SIZE * entity_id;


	if ((self.angles_y == 0) || (self.angles_y == 180)) // Facing East/West
	{
		subject.orgnl_mins = entity_box[index2];
		subject.orgnl_maxs = entity_box[index2 + BMAX_OFS];
	}
	else
	{
		hold_vect = entity_box[index2];
		subject.orgnl_mins_x = hold_vect_y;
		subject.orgnl_mins_y = hold_vect_x;
		subject.orgnl_mins_z = hold_vect_z;

		hold_vect = entity_box[index2 + BMAX_OFS];
		subject.orgnl_maxs_x = hold_vect_y;
		subject.orgnl_maxs_y = hold_vect_x;
		subject.orgnl_maxs_z = hold_vect_z;
	}

	setsize (subject, subject.orgnl_mins, subject.orgnl_maxs);

	if (subject.scale)
		ScaleBoundingBox(subject.scale, subject,mass_flag);

	if ((subject.maxs_x - subject.mins_x ) >= 32)
		subject.hull = 2;

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;

	subject.th_die = death_func;

	if(subject.proj_ofs=='0 0 0')
		subject.proj_ofs='0 0 24';
}

void Create_Missile (entity spawner, vector spot1,vector spot2,string missilemodel, 
	string missilename,float missileskin,float missilespeed,void() touchfunct)
{
vector diff;
	
	newmis = spawn ();
	newmis.owner = spawner;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
	newmis.skin = missileskin;
		
	makevectors (spawner.angles);

	diff = normalize(spot2 - spot1);
	diff+=aim_adjust(self.enemy);

	newmis.velocity = diff * missilespeed;
	newmis.classname = missilename;
	newmis.angles = vectoangles(newmis.velocity);

	newmis.touch = touchfunct;

	setmodel (newmis,missilemodel);
	setsize (newmis, '0 0 0', '0 0 0');		
	setorigin (newmis, spot1);

	newmis.think = SUB_Remove;
	newmis.nextthink = time + 2.5;
}

