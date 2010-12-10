/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/famhorse.hc,v 1.2 2007-02-07 16:57:01 sezero Exp $
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
$frame Hrear1       Hrear2       Hrear3       Hrear4       Hrear5       
$frame Hrear6       Hrear7       Hrear8       Hrear9       Hrear10      
$frame Hrear11      Hrear12      Hrear13      Hrear14      Hrear15      
$frame Hrear16      Hrear17      Hrear18      Hrear19      Hrear20      
$frame Hrear21      Hrear22      Hrear23      Hrear24      Hrear25      
$frame Hrear26      Hrear27      Hrear28      Hrear29      Hrear30      
$frame Hrear31      Hrear32      Hrear33      Hrear34      Hrear35      
$frame Hrear36      Hrear37      Hrear38      Hrear39      Hrear40      
$frame Hrear41      Hrear42      Hrear43      Hrear44      Hrear45      
$frame Hrear46      Hrear47      Hrear48      Hrear49      Hrear50      

//
$frame HtranA1      HtranA2      HtranA3      HtranA4      HtranA5      
$frame HtranA6      HtranA7      HtranA8      HtranA9      HtranA10     
$frame HtranA11     HtranA12     HtranA13     HtranA14     HtranA15     
$frame HtranA16     

//
$frame HtranB1      HtranB2      HtranB3      HtranB4      HtranB5      
$frame HtranB6      HtranB7      HtranB8      HtranB9      HtranB10     
$frame HtranB11     HtranB12     HtranB13     HtranB14     HtranB15     
$frame HtranB16     

//
$frame Htrot1       Htrot2       Htrot3       Htrot4       Htrot5       
$frame Htrot6       Htrot7       Htrot8       Htrot9       Htrot10      
$frame Htrot11      Htrot12      

$framevalue 0
//----------------------------------------------------------------
// Rider Frames
//----------------------------------------------------------------
$frame Frear1       Frear2       Frear3       Frear4       Frear5       
$frame Frear6       Frear7       Frear8       Frear9       Frear10      
$frame Frear11      Frear12      Frear13      Frear14      Frear15      
$frame Frear16      Frear17      Frear18      Frear19      Frear20      
$frame Frear21      Frear22      Frear23      Frear24      Frear25      
$frame Frear26      Frear27      Frear28      Frear29      Frear30      
$frame Frear31      Frear32      Frear33      Frear34      Frear35      
$frame Frear36      Frear37      Frear38      Frear39      Frear40      
$frame Frear41      Frear42      Frear43      Frear44      Frear45      
$frame Frear46      Frear47      Frear48      Frear49      Frear50      

//
$frame Fscale1      Fscale2      Fscale3      Fscale4      Fscale5      
$frame Fscale6      Fscale7      Fscale8      Fscale9      Fscale10     
$frame Fscale11     Fscale12     Fscale13     Fscale14     Fscale15     
$frame Fscale16     Fscale17     Fscale18     Fscale19     Fscale20     
$frame Fscale21     Fscale22     Fscale23     Fscale24     Fscale25     
$frame Fscale26     Fscale27     Fscale28     Fscale29     Fscale30     
$frame Fscale31     Fscale32     Fscale33     Fscale34     Fscale35     
$frame Fscale36     Fscale37     Fscale38     Fscale39     Fscale40     
$frame Fscale41     Fscale42     Fscale43     Fscale44     Fscale45     
$frame Fscale46     Fscale47     Fscale48     Fscale49     Fscale50     
$frame Fscale51     Fscale52     Fscale53     Fscale54     Fscale55     
$frame Fscale56     Fscale57     Fscale58     Fscale59     Fscale60     
$frame Fscale61     Fscale62     Fscale63     Fscale64     Fscale65     
$frame Fscale66     Fscale67     Fscale68     Fscale69     Fscale70     

//
$frame FtranA1      FtranA2      FtranA3      FtranA4      FtranA5      
$frame FtranA6      FtranA7      FtranA8      FtranA9      FtranA10     
$frame FtranA11     FtranA12     FtranA13     FtranA14     FtranA15     
$frame FtranA16     

//
$frame FtranB1      FtranB2      FtranB3      FtranB4      FtranB5      
$frame FtranB6      FtranB7      FtranB8      FtranB9      FtranB10     
$frame FtranB11     FtranB12     FtranB13     FtranB14     FtranB15     
$frame FtranB16     

//
$frame Ftrot1       Ftrot2       Ftrot3       Ftrot4       Ftrot5       
$frame Ftrot6       Ftrot7       Ftrot8       Ftrot9       Ftrot10      
$frame Ftrot11      Ftrot12      Ftrot13      Ftrot14      Ftrot15      
$frame Ftrot16      


float fam_start[1] =
{
	$Htrot1
};

float fam_end[1] =
{
	$Htrot12
};

float fam_speed[1] =
{
	5.5  // Normal speed
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
float FH_STAGE_LAUGHING		= 6;
float FH_STAGE_ATTACK2		= 7;

float pulltime;
float looktime;
float hurttime;

void famhorse_move(void);
void famhorse_rear(void);

void do_fambeam (entity lowner,float tag, float lflags, float duration, vector spot1, vector spot2)
{
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_STREAM_FAMINE);
	WriteEntity (MSG_BROADCAST, lowner);
	WriteByte (MSG_BROADCAST, tag+lflags);
	WriteByte (MSG_BROADCAST, duration);
	
	WriteCoord (MSG_BROADCAST, spot1_x);
	WriteCoord (MSG_BROADCAST, spot1_y);
	WriteCoord (MSG_BROADCAST, spot1_z);

	WriteCoord (MSG_BROADCAST, spot2_x);
	WriteCoord (MSG_BROADCAST, spot2_y);
	WriteCoord (MSG_BROADCAST, spot2_z);
}

void famine_pain(void)
{
	float chance,rear_chance;

	rear_chance = self.health / self.max_health;

	if (rear_chance < .50)
		rear_chance = .5;

	if (self.monster_stage == FH_STAGE_NORMAL)
	{
		// Force a new gallop frame in
		self.frame = fam_start[self.rider_gallop_mode];

		chance = random();

		if (chance > rear_chance) // Like the Lone Ranger but he disappears
		{
			self.think = famhorse_rear;
			self.monster_stage = FH_STAGE_BEGIN_REAR;
		}
		else if (chance < 0.20)  // Told himself a joke
		{
			self.monster_stage = FH_STAGE_LAUGHING;
			self.movechain.frame = $FtranB1;
		}
		else if (chance < 0.60)  // Missile attack
		{
			self.monster_stage = FH_STAGE_ATTACK2;
			self.movechain.frame = $Fscale1;
		}
	}
}

void famine_blinkin(void) 
{
	thinktime self : HX_FRAME_TIME;
	self.think = famine_blinkin;

	self.scale += 0.10;
	self.movechain.scale += 0.10;

	if (self.scale >= 1)
	{
		self.scale=1;
		self.movechain.scale=1;

		thinktime self : HX_FRAME_TIME;
		self.think = famhorse_rear;
	}
}

// Find a point to start at close to the player
vector famine_pointcheck(void)
{
	entity search;
	float diff, holddiff;
	vector newspot,holdpos;

	search = find(world, classname, "rider_path");
	diff = 99999;
	while(search != world)
	{
		holddiff = vlen(search.origin - self.enemy.origin);
		
		if (holddiff < diff)
		{
			traceline (search.origin, search.origin - '0 0 600' , FALSE, self);			

			holdpos=trace_endpos;
			tracearea (holdpos,holdpos + '0 0 40','-55 -55 -24', '55 55 150',FALSE,self);
			if (trace_fraction == 1.0 && trace_ent == world)		// Check no one is standing where monster wants to be
			{
				diff = holddiff;
				newspot= holdpos;
				self.path_current = search;
			}
		}

		search = find(search, classname, "rider_path");
	}
	return (newspot);
}


/*-----------------------------------------
	famine_init - looking for a spot to come back in
  -----------------------------------------*/
void famine_blinkin_init (void)
{
	vector holdspot;
	holdspot = famine_pointcheck();
	setorigin(self,holdspot);

	setmodel (self, "models/boss/famhorse.mdl");
	setsize (self, '-40 -40 0', '40 40 100');
	self.hull = HULL_POINT;
	self.solid = SOLID_SLIDEBOX;
	self.takedamage = DAMAGE_YES;

	setmodel (self.movechain, "models/boss/famrider.mdl");
	self.movechain.flags (+) FL_MOVECHAIN_ANGLE;

	walkmove(self.angles_y, .01, TRUE);		// You have to move it a little bit to make it solid

	riderpath_findnext();  // Find the next path point to turn the horse in that direction

	self.ideal_yaw = vectoyaw(self.path_current.origin - self.origin);
	self.angles_y = self.ideal_yaw;

	sound (self, CHAN_BODY, "skullwiz/blinkin.wav", 1, ATTN_NORM);
	CreateRedCloud (self.origin + '0 0 40','0 0 0',HX_FRAME_TIME);

	thinktime self : HX_FRAME_TIME;
	self.think = famine_blinkin;
}

/*-----------------------------------------
	famine_blinkout - blink out
  -----------------------------------------*/
void famine_blinkout(void) 
{
	thinktime self : HX_FRAME_TIME / 2;
	self.think = famine_blinkout;

	self.scale -= 0.10;
	self.movechain.scale -= 0.10;

	if ((self.scale > 0.79) && (self.scale < 0.89))
	{
		sound (self, CHAN_BODY, "skullwiz/blinkout.wav", 1, ATTN_NORM);
		CreateRedCloud (self.origin + '0 0 40','0 0 0',HX_FRAME_TIME);
	}

	if (self.scale < 0.10)
	{
		setmodel(self,string_null);
		setmodel(self.movechain,string_null);
		thinktime self : random(0.5,3);		// Reappear when
		self.think = famine_blinkin_init;
	}
}

void famine_blinkout_init(void)
{
	self.takedamage = DAMAGE_NO;  // So t_damage won't force him into another state 
	self.solid = SOLID_NOT;
	self.scale = 1;
	self.drawflags (+) SCALE_TYPE_XYONLY;

	self.movechain.takedamage = DAMAGE_NO;  // So t_damage won't force him into another state 
	self.movechain.scale = 1;
	self.movechain.drawflags (+) SCALE_TYPE_XYONLY;

	famine_blinkout();	
}

void famine_missile_touch(void)
{
	float damg;

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(8,16);

	T_Damage (other, self, self.owner, damg );

	self.origin = self.origin - 8 * normalize(self.velocity) - '0 0 40';
	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	CreateRedSpark (self.origin); 

	remove(self);

}


void famine_missile_think (void)
{
	if (self.lifetime < time)
	{
		sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
		CreateRedSpark (self.origin); 
		remove(self);
	}
	else
	{
		HomeThink();
		self.angles = vectoangles(self.velocity);
		self.think=famine_missile_think;
		thinktime self : HX_FRAME_TIME;
	}
}

void famine_missile(float dir)
{
	entity newmis;
	vector diff;

	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
		
    setmodel (newmis, "models/famshot.mdl");
	setsize (newmis, '0 0 0', '0 0 0');		
	setorigin (newmis, self.origin + '0 0 120');

	newmis.angles = self.angles;

	diff = (self.enemy.origin + self.enemy.view_ofs) - self.origin + '0 0 120' ;
	newmis.velocity = normalize(diff);
	newmis.angles = vectoangles(newmis.velocity);

	if (dir == 0)
		newmis.angles_y -= 25;
	else if (dir == 2)
		newmis.angles_y += 25;
	makevectors (newmis.angles);
	

	newmis.velocity = normalize(v_forward);
	if (dir ==1)
		newmis.speed=800;	//Speed
	else
		newmis.speed=700;	//Speed
	newmis.classname = "faminemissile";
	newmis.angles = vectoangles(newmis.velocity);

	newmis.veer=FALSE;	//No random wandering
	newmis.turn_time=5;	//Lower the number, tighter the turn
	newmis.ideal_yaw=FALSE;//Only track things in front
	newmis.lifetime = time + 2;
	newmis.think=famine_missile_think;
	thinktime newmis : 0;

	newmis.touch = famine_missile_touch;
}

void famine_missilespawner(void)
{
	CreateRedFlash(self.origin + '0 0 120');
	sound (self, CHAN_WEAPON, "famine/shot.wav", 1, ATTN_NORM);
	famine_missile(0);
	famine_missile(1);
	famine_missile(2);
}

/* ---------------------------
	Create the rider
   ---------------------------*/
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
	rider.flags (+) FL_MOVECHAIN_ANGLE;
}

/* ---------------------------
	The horse is rearing
   ---------------------------*/
void famhorse_rear(void)
{
	float retval;

	thinktime self : HX_FRAME_TIME;

	if (self.monster_stage == FH_STAGE_BEGIN_REAR)
	{
		retval = AdvanceFrame($HtranA1,$HtranA16);

		if (self.frame > $HtranA10)
			self.speed = 1;

		riderpath_move(self.speed);

		if (self.frame == $HtranA8)
			sound (self, CHAN_VOICE, "famine/whinny.wav", 1, ATTN_NORM);

		if (retval == AF_END) 
			self.monster_stage = FH_STAGE_MIDDLE_REAR;
		self.movechain.frame = FamRiderFrames[1] + (self.frame - $HtranA1);
	}
	else if (self.monster_stage == FH_STAGE_MIDDLE_REAR)
	{
		retval = AdvanceFrame($Hrear1,$Hrear50);

		if (self.frame == $Hrear10)
		{
			famine_blinkout_init();
			return;
		}		

		if (self.enemy)
		{
			if (self.enemy.health > 0)
			{
				if (self.frame == $Hrear30) 
					famine_missilespawner();
			}
		}
	
		if (retval == AF_END) 
		{
			if (self.enemy != world && random() < 0.7)
				self.monster_stage = FH_STAGE_STANDING;
			else
				self.monster_stage = FH_STAGE_END_REAR;
		}
		self.movechain.frame = FamRiderFrames[2] + (self.frame - $Hrear1);

	}
	else if (self.monster_stage == FH_STAGE_STANDING)
	{
		if (random() < 0.5)
			self.monster_stage = FH_STAGE_END_REAR;
	}
	else if (self.monster_stage == FH_STAGE_END_REAR)
	{
		retval = AdvanceFrame($HtranB1,$HtranB16);

		if (self.frame < $HtranB11)
			self.speed =0;
		else if (self.frame < $HtranB13)
			self.speed = fam_speed[self.rider_gallop_mode] * .5;
		else 
			self.speed = fam_speed[self.rider_gallop_mode];

		if (retval == AF_END) 
		{
			self.think = famhorse_move;
			self.monster_stage = FH_STAGE_NORMAL;
		}

		riderpath_move(self.speed);

		self.movechain.frame = FamRiderFrames[3] + (self.frame - $HtranB1);
	}
}

void check_remove ()
{
	thinktime self : 1;
	if(pulltime<time)
		remove(self);
}

/* ---------------------------
	The horse is running around
   ---------------------------*/
void famhorse_move(void)
{
	float retval,chance,rear_chance,damage,diff2;
	vector diff,hold_velocity;
	entity newent;
	vector holdvel, holdangles,spot1;

	self.think = famhorse_move;
	thinktime self : HX_FRAME_TIME;

	// Advance horse frame
	retval = AdvanceFrame(fam_start[self.rider_gallop_mode],fam_end[self.rider_gallop_mode]);

	if (self.frame == $Htrot6 || self.frame == $Htrot11)
	{
		chance = random();
		if (chance < .33)
			sound (self, CHAN_VOICE, "famine/clop1.wav", 1, ATTN_NORM);
		else if (chance < .66)
			sound (self, CHAN_VOICE, "famine/clop2.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "famine/clop3.wav", 1, ATTN_NORM);
	}
	
	if (!self.path_current)  // Look for a path to follow
		riderpath_init();

	riderpath_move(self.speed);

	if ((!self.enemy) || (self.enemy.health <= 0))
	{
		if (!FindTarget(TRUE))
			self.enemy = world;
		self.goalentity = self.enemy;
	}

	if ((retval == AF_BEGINNING) && (self.monster_stage == FH_STAGE_NORMAL) && (self.enemy!=world))
	{
		if (random() < .05)
			sound(self,CHAN_AUTO,"famine/snort.wav",1,ATTN_NORM);
			
		retval = fabs(self.rider_y_change);

		// Force a new gallop frame in
		self.frame = fam_start[self.rider_gallop_mode];

		rear_chance = self.health / self.max_health;

		if (rear_chance < .50)
			rear_chance = .5;

		chance = random();

		if (chance > rear_chance) // Like the Lone Ranger but he disappears
		{
			self.think = famhorse_rear;
			self.monster_stage = FH_STAGE_BEGIN_REAR;
		}
		else if (chance < 0.05)  // Told himself a joke
		{
			self.monster_stage = FH_STAGE_LAUGHING;
			self.movechain.frame = $FtranB1;
		}
		// Find an enemy??
		else if ((self.rider_gallop_mode == 0) && (looktime < time))
		{			
			traceline (self.origin  + '0 0 120',self.enemy.origin , FALSE, self);			
			
			if (self.enemy)
			{
				if (self.enemy == trace_ent)
				{
					chance = random();

					if (chance < .30)	// The famous pull attack
					{
						pulltime = time + random(2,5);
						self.monster_stage = FH_STAGE_ATTACK;
						self.movechain.frame = $Fscale1;
						sound(self,CHAN_WEAPON,"famine/pull.wav",1,ATTN_NORM);
					}
					else if (chance < .70)		// Missile attack
					{
						self.monster_stage = FH_STAGE_ATTACK2;
						self.movechain.frame = $Fscale1;
					}
				}
			}
		}
	}

	// Is rider attacking player
	if (self.monster_stage == FH_STAGE_ATTACK)
	{
		if ((self.movechain.frame == $Fscale9) && (self.enemy.health > 0))
		{
			newent = spawn();
			setorigin (newent, self.origin + '0 0 120');
			setmodel (newent, "models/soulball.mdl");
			self.controller = newent;
			self.controller.think=check_remove;
			thinktime self.controller : 0;
		}

		if ((self.movechain.frame == $Fscale10) && (self.enemy.health > 0))
		{
			diff = self.enemy.origin - self.origin + '0 0 120';
			holdvel = normalize(diff);
			holdangles = vectoangles(holdvel);

			// traceline in front and behind because the beam is wide
			makevectors (holdangles);
			spot1 = self.origin + '0 0 120' + v_right * 25;
			traceline (spot1,self.enemy.origin , FALSE, self);			
			if (trace_ent != self.enemy)
			{
				pulltime = 0;
			}
			else
			{
				spot1 = self.origin + '0 0 120' - v_right * 25;
				traceline (spot1,self.enemy.origin , FALSE, self);			
				if (trace_ent != self.enemy)
					pulltime = 0;
			}

			if (pulltime)
			{
				self.movechain.drawflags(+)MLS_ABSLIGHT;
				self.movechain.abslight = .5;
				do_fambeam (self,1,STREAM_ATTACHED, 1, self.origin + '0 0 120', self.enemy.origin);
				self.enemy.velocity = '0 0 0';

				setorigin (self.controller, self.origin + '0 0 120');

				diff = self.enemy.origin - self.origin;
				hold_velocity = normalize(diff);
				hold_velocity = hold_velocity * -250;
				self.enemy.velocity = self.enemy.velocity + hold_velocity; 
				if(vlen(self.enemy.velocity)>500)
					self.enemy.velocity=normalize(self.enemy.velocity)*500;
				self.enemy.flags (-) FL_ONGROUND;
			}

			if (hurttime < time)
			{			
				diff2 = vlen(self.enemy.origin - self.origin);
				damage =  300/diff2;
				if (damage < 2)
					damage=2;
				else if (damage >8)
					damage = 8;

				T_Damage (self.enemy, self, self, damage);
				if (self.enemy.health <= 0)
				{
					if(self.controller)
						remove(self.controller);
					sound(self,CHAN_WEAPON,"misc/null.wav",1,ATTN_NORM);
				}

				hurttime = time + 1;
			}

			if (pulltime < time)
			{
				if(self.controller)
					remove(self.controller);
				sound(self,CHAN_WEAPON,"misc/null.wav",1,ATTN_NORM);

				self.movechain.drawflags(-)MLS_ABSLIGHT;
				self.movechain.frame += 50;				
				looktime = time + random(5,10);
			}
		}	
		else
		{
			self.movechain.frame += 1;
		}

		if (self.movechain.frame >= $Fscale70)
			self.monster_stage = FH_STAGE_NORMAL;
	}
	else if (self.monster_stage == FH_STAGE_ATTACK2)
	{
		if (self.movechain.frame == $Fscale10) 
		{
			famine_missilespawner();
			self.movechain.frame += 50;				
		}
	
		self.movechain.frame += 1;

		if (self.movechain.frame >= $Fscale70)
			self.monster_stage = FH_STAGE_NORMAL;
	}

	else if (self.monster_stage == FH_STAGE_LAUGHING)
	{
		self.movechain.frame += 1;

		if (self.movechain.frame == $FtranB6)
			sound (self.movechain, CHAN_VOICE, "famine/laugh.wav", 1, ATTN_NORM);

		else if (self.movechain.frame >= $FtranB16)
			self.monster_stage = FH_STAGE_NORMAL;
	}
}

/*QUAKED rider_famine (1 0 0)  (-55 -55 -24) (55 55 150) TRIGGER_WAIT
Famine rider monster.  You must place rider_path entites
on the map.  The rider will first proceed to the 
rider_path point with a path_id of 1.
-------------------------FIELDS-------------------------
map: next map to go to when you kill the rider
target: start spot on the next map
--------------------------------------------------------
*/
void rider_famine(void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model3 ("models/boss/famhorse.mdl");
	precache_model3 ("models/boss/famrider.mdl");

	precache_model3 ("models/famshot.mdl");

	precache_sound3 ("famine/die.wav");
	precache_sound3 ("famine/laugh.wav");
	precache_sound3 ("famine/whinny.wav");
	precache_sound3 ("famine/pull.wav");
	precache_sound3 ("famine/shot.wav");
	precache_sound3 ("famine/snort.wav");
	precache_sound3 ("famine/clop1.wav");
	precache_sound3 ("famine/clop2.wav");
	precache_sound3 ("famine/clop3.wav");
	precache_sound3 ("misc/null.wav");
	precache_sound3 ("raven/blast.wav");
	precache_sound3 ("skullwiz/blinkout.wav");
	precache_sound3 ("skullwiz/blinkin.wav");

	rider_init();

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.thingtype = THINGTYPE_FLESH;
	self.yaw_speed = 2;

	setmodel (self, "models/boss/famhorse.mdl");
	self.skin = 0;

	setsize (self, '-40 -40 0', '40 40 100');
	self.health = self.max_health = 2200;
	self.th_pain = famine_pain;
	self.rider_gallop_mode = 0;
	self.speed = fam_speed[self.rider_gallop_mode];
	self.rider_path_distance = 75;
	self.monster_stage = FH_STAGE_NORMAL;
	self.mass = 30000;
	self.flags (+) FL_MONSTER;
	self.flags2 (+) FL_ALIVE;
	self.monsterclass = CLASS_BOSS;
	self.yaw_speed = 10;
	self.experience_value = 500;

	create_famrider(self);
	self.attack_finished = 0;
	self.hull = HULL_POINT;
	looktime =0;
	hurttime = 0;

	self.noise="famine/die.wav";
	self.th_save = famhorse_move;
	self.think = multiplayer_health;
	thinktime self : 1;
}

