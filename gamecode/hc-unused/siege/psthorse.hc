/*
 * $Header: /HexenWorld/Siege/psthorse.hc 3     5/25/98 1:39p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\monsters\RdrFam\Horse\Final\famhorse.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RdrFam\Horse\Final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

// Horse frames
//
$frame Bcharg1      Bcharg2      Bcharg3      Bcharg4      Bcharg5      
$frame Bcharg6      Bcharg7      Bcharg8      

//
$frame BtranA1      BtranA2      BtranA3      BtranA4      BtranA5      
$frame BtranA6      BtranA7      BtranA8      BtranA9      BtranA10     
$frame BtranA11     BtranA12     BtranA13     

//
$frame BtranB1      BtranB2      BtranB3      BtranB4      BtranB5      
$frame BtranB6      BtranB7      BtranB8      BtranB9      BtranB10     
$frame BtranB11     BtranB12     

//
$frame BtranC1      BtranC2      BtranC3      BtranC4      BtranC5      
$frame BtranC6      BtranC7      BtranC8      

//
$frame BtranD1      BtranD2      BtranD3      BtranD4      BtranD5      
$frame BtranD6      BtranD7      BtranD8      

//
$frame Btrot1       Btrot2       Btrot3       Btrot4       Btrot5       
$frame Btrot6       Btrot7       Btrot8       Btrot9       Btrot10      
$frame Btrot11      Btrot12      

$framevalue 0

// Rider Frames
//
$frame Parrow1      Parrow2      Parrow3      Parrow4      Parrow5      
$frame Parrow6      

//
$frame Pcharg1      Pcharg2      Pcharg3      Pcharg4      Pcharg5      
$frame Pcharg6      Pcharg7      Pcharg8      

//
$frame Phive1       Phive2       Phive3       Phive4       Phive5       
$frame Phive6       Phive7       Phive8       Phive9       Phive10      
$frame Phive11      Phive12      Phive13      Phive14      Phive15      
$frame Phive16      Phive17      Phive18      Phive19      Phive20      
$frame Phive21      Phive22      Phive23      Phive24      Phive25      
$frame Phive26      Phive27      Phive28      Phive29      Phive30      
$frame Phive31      Phive32      Phive33      Phive34      Phive35      
$frame Phive36      Phive37      Phive38      

//
$frame PtranA1      PtranA2      PtranA3      PtranA4      PtranA5      
$frame PtranA6      PtranA7      PtranA8      PtranA9      PtranA10     
$frame PtranA11     PtranA12     PtranA13     

//
$frame PtranB1      PtranB2      PtranB3      PtranB4      PtranB5      
$frame PtranB6      PtranB7      PtranB8      PtranB9      PtranB10     
$frame PtranB11     PtranB12     

//
$frame PtranC1      PtranC2      PtranC3      PtranC4      PtranC5      
$frame PtranC6      PtranC7      PtranC8      

//
$frame PtranD1      PtranD2      PtranD3      PtranD4      PtranD5      
$frame PtranD6      PtranD7      PtranD8      

//
$frame Ptrot1       Ptrot2       Ptrot3       Ptrot4       Ptrot5       
$frame Ptrot6       Ptrot7       Ptrot8       Ptrot9       Ptrot10      
$frame Ptrot11      Ptrot12      








float fam_start[1] =
{
	$Htrot1
};

float fam_end[1] =
{
	$Htrot16
};

float fam_speed[1] =
{
	3
};

// Array to align frames
float FamRiderFrames[5] =
{
	$Ftrot1,	// Animation for fast gallop

	$FtranA1,   // Animation for start of rear
	$Frear1,	// Animation for rear
	$FtranB1,	// Animation for end of rear

	$Fscale1	// Attack Sequence #1
};

float FH_STAGE_NORMAL		= 0;
float FH_STAGE_BEGIN_REAR	= 1;
float FH_STAGE_MIDDLE_REAR	= 2;
float FH_STAGE_END_REAR		= 3;
float FH_STAGE_ATTACK		= 4;
float FH_STAGE_STANDING		= 5;

// Frame Code













void create_famrider(entity horse)
{
	entity rider;

	rider = spawn();

	rider.solid = SOLID_NOT;
	rider.movetype = MOVETYPE_NONE;
	rider.origin = horse.origin;
	rider.angles = self.angles;

	setmodel (rider, "models/boss/famrider.mdl");
	rider.skin = 0;

	horse.movechain = rider;
	rider.flags = rider.flags | FL_MOVECHAIN_ANGLE;
}

void famhorse_move(void);

void famhorse_rear(void)
{
	float retval;

	self.nextthink = time + HX_FRAME_TIME;

	if (self.monster_stage == FH_STAGE_BEGIN_REAR)
	{
		self.speed = self.speed / 1.2;
		riderpath_move(self.speed);

		retval = AdvanceFrame($HtranA1,$HtranA16);
		if (retval == AF_END) 
			self.monster_stage = FH_STAGE_MIDDLE_REAR;
		self.movechain.frame = FamRiderFrames[1] + (self.frame - $HtranA1);
	}
	else if (self.monster_stage == FH_STAGE_MIDDLE_REAR)
	{
		retval = AdvanceFrame($Hrear1,$Hrear50);
		if (retval == AF_END) 
		{
			if (self.enemy != world && random() < 0.7)
				self.monster_stage = FH_STAGE_STANDING;
			else
				self.monster_stage = FH_STAGE_END_REAR;
		}
		self.movechain.frame = FamRiderFrames[2] + (self.frame - $Hrear1);

/*		if (self.movechain.frame == FamRiderFrames[9])
		{
			if (!self.enemy)
				LocateTarget();
			
			if (self.enemy)
				Famaxe(self.enemy,'1 2 70',0);
		}*/
	}
/*	else if (self.monster_stage == FH_STAGE_STANDING)
	{
		retval = AdvanceFrame($HaxeD1,$HaxeD15);
		if (retval == AF_END) 
		{
			if (random() < 0.5)
				self.monster_stage = FH_STAGE_END_REAR;
		}

		self.movechain.frame = FamRiderFrames[10] + (self.frame - $HaxeD1);

		if (!self.enemy)
			LocateTarget();
		if (self.enemy)
		{
			if (self.movechain.frame == FamRiderFrames[11])
				waraxe(self.enemy,'3 3 82',1);
			else if (self.movechain.frame == WarRiderFrames[12])
				waraxe(self.enemy,'-3 3 70',1);
			else if (self.movechain.frame == WarRiderFrames[13])
				waraxe(self.enemy,'-1 3 75',1);
		}
	}*/
	else if (self.monster_stage == FH_STAGE_END_REAR)
	{
		retval = AdvanceFrame($HtranB1,$HtranB16);
		if (retval == AF_END) 
		{
			self.think = famhorse_move;
			self.monster_stage = FH_STAGE_NORMAL;
			self.speed = fam_speed[self.rider_gallop_mode];
		}
		else
			self.speed = self.speed * 1.3;

		riderpath_move(self.speed);

		self.movechain.frame = FamRiderFrames[3] + (self.frame - $HtranB1);
	}
}

void famhorse_move(void)
{
	float retval;

	self.think = famhorse_move;
	self.nextthink = time + HX_FRAME_TIME;

//	dprintv("origin %s   ",self.origin);
//	dprintv("rider %s\n",self.movechain.origin);

	retval = AdvanceFrame(fam_start[self.rider_gallop_mode],fam_end[self.rider_gallop_mode]);
	
	if (!self.path_current)
	{
		riderpath_init();
	}

	riderpath_move(self.speed);
	if (retval == AF_BEGINNING)
	{
		retval = fabs(self.rider_y_change);

		// Force a new gallop frame in
		self.frame = fam_start[self.rider_gallop_mode];

		if (retval < 1 && random() < 0.1)
		{
			self.think = famhorse_rear;
			self.monster_stage = FH_STAGE_BEGIN_REAR;
		}
/*		else if (self.rider_gallop_mode == 0)
		{
			if (!self.enemy)
				LocateTarget();
			
			if (self.enemy)
			{
				self.monster_stage = FH_STAGE_ATTACK;
			}
		}*/

//		dprintf("Gallop is %s",self.rider_gallop_mode);
//		dprintf("   y_change is %s\n",self.rider_y_change);
	}

	if (self.monster_stage == FH_STAGE_ATTACK)
	{
		if (self.rider_gallop_mode == 0)
		{
			self.movechain.frame = FamRiderFrames[4] + 
				(self.frame - fam_start[self.rider_gallop_mode]);
/*			if (self.movechain.frame == FamRiderFrames[7])
				waraxe(self.enemy,'3 3 82',1);
			else if (self.movechain.frame == WarRiderFrames[8])
			{
				waraxe(self.enemy,'-3 3 70',1);
//				waraxe(self.enemy,'0 3 70',1);
			}*/
		}
	}
	else
	{
		self.movechain.frame = FamRiderFrames[self.rider_gallop_mode] + 
			(self.frame - fam_start[self.rider_gallop_mode]);
	}

	// make sure we use the last attack frame before we go out of the mode
	if (retval == AF_END)
	{
		self.monster_stage = FH_STAGE_NORMAL;
	}

	if (fabs(fam_speed[self.rider_gallop_mode] - self.speed) < 0.2)
		self.speed = fam_speed[self.rider_gallop_mode];
	else if (fam_speed[self.rider_gallop_mode] > self.speed) 
		self.speed += 0.2;
	else
		self.speed -= 0.2;
}







void rider_death();

/*QUAKED rider_famine (1 0 0) (0 0 0) (50 50 50)
Famine rider monster.  You must place rider_path entites
on the map.  The rider will first proceed to the 
rider_path point with a path_id of 1.
-------------------------FIELDS-------------------------
--------------------------------------------------------

*/
void() rider_famine =
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2 ("models/boss/famhorse.mdl");
	precache_model2 ("models/boss/famrider.mdl");
    precache_model2 ("models/boss/shaft.mdl");
    precache_model2 ("models/boss/circle.mdl");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.yaw_speed = 2;

	setmodel (self, "models/boss/famhorse.mdl");
	self.skin = 0;

	setsize (self, '-55 -55 0', '55 55 100');
	self.health = 1;
	self.takedamage=DAMAGE_YES;
	self.rider_gallop_mode = 0;
	self.speed = fam_speed[self.rider_gallop_mode];
	self.rider_path_distance = 200;
	self.monster_stage = FH_STAGE_NORMAL;
	self.mass = 30000;

	create_famrider(self);

	self.th_die = rider_die;
	self.think = famhorse_move;
	self.nextthink = time + 0.2; // wait for path points to spawn

//	warhorse_die();
};

