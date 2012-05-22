/*
 * portals/fish.hc
 */

/*
==============================================================================

Q:\art\models\monsters\FISH\FISH1.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\FISH
$origin 0 0 0
$base BASE SKIN
$skin SKIN
$skin SKIN2
$flags 0

//
$frame SWIM01       SWIM02       SWIM03       SWIM04       SWIM05       
$frame SWIM06       SWIM07       SWIM08       SWIM09       SWIM10       
$frame SWIM11       SWIM12       SWIM13       SWIM14       SWIM15       
$frame SWIM16       SWIM17       SWIM18       SWIM19       SWIM20       
$frame SWIM21       SWIM22       SWIM23       SWIM24       SWIM25       
$frame SWIM26       SWIM27       SWIM28       SWIM29       SWIM30       
$frame SWIM31       SWIM32       SWIM33       SWIM34       SWIM35       
$frame SWIM36       SWIM37       SWIM38       SWIM39       SWIM40       




// Frame Code
float FISH_STAGE_MOVE = 1;
float FISH_STAGE_FOLLOW = 2;
float FISH_STAGE_BORED = 3;

void fish_hover(void);
void fish_move(void);


float fish_friends(void)
{
	entity item,test;
	float bad;

	item = findradius(self.origin, 100);
	while (item)
	{
		if (item.classname == "monster_fish" && item != self)
		{
			test = item.goalentity;
			bad = FALSE;
			while(test != world && bad != TRUE)
			{
				if (test == self) bad = TRUE;

				test = test.goalentity;
			}
			if (!bad)
			{
				self.goalentity = item;
				self.goalentity.fish_leader_count += 1;
				return TRUE;
			}
		}
		item = item.chain;
	}

	return FALSE;
}

void fish_follow(void)
{
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	if (self.goalentity == world)	/* experienced by Rugxulo */
	{
		self.think = fish_hover;
		self.monster_stage = FISH_STAGE_MOVE;
		return;
	}

	if (random() > 0.1)
		return;

	if (random() < .05)
	{
		self.monster_duration = random(250,450);
		self.monster_stage = FISH_STAGE_BORED;
		self.think = fish_hover;
		self.goalentity.fish_leader_count -= 1;
		self.goalentity = world;
		//dprint("Fish got bored\n");
		//self.drawflags (-) MLS_ABSLIGHT;
		return;	/* a return was missing here -- O.S. */
	}

	self.movedir = self.monster_last_seen - self.origin + randomv('-20 -20 -25', '20 20 25');
	if (self.goalentity.goalentity)
	{
		self.goalentity.fish_leader_count -= 1;
		self.goalentity = self.goalentity.goalentity;
		self.goalentity.fish_leader_count += 1;
	}

	self.monster_last_seen = self.goalentity.origin;

	self.count = 80 + random(20);
	self.movedir_x /= self.count;
	self.movedir_y /= self.count;
	self.movedir_z /= self.count;
	self.fish_speed = vhlen(self.movedir);

	self.think = fish_move;
}

void fish_move(void)
{
	float retval;

	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	self.ideal_yaw = vectoyaw(self.movedir);
	ChangeYaw();
	retval = walkmove(self.angles_y, self.fish_speed, FALSE);
	retval = movestep(0, 0, self.movedir_z, FALSE);
	/*
	if (retval != 2)
	{
		self.goalentity = world;
		self.monster_stage = FISH_STAGE_MOVE;
	}
	*/
	if (self.count >= 170)
		self.fish_speed *= 1.05;
	else if (self.count < 30)
		self.fish_speed *= .9;

	self.count -= 1;
	if (self.count < 1)
	{
		self.count = 0;
		if (self.monster_stage == FISH_STAGE_MOVE)
		{
			if (fish_friends())
			{
				self.monster_stage = FISH_STAGE_FOLLOW;
				self.think = fish_follow;
				self.monster_last_seen = self.goalentity.origin;
				//self.drawflags (+) MLS_ABSLIGHT;
				//self.abslight = 0;
				//self.goalentity.drawflags (+) MLS_ABSLIGHT;
				//self.goalentity.abslight = 2.5;
				//dprint("Following!\n");
			}
			else
				self.think = fish_hover;
		}
		else if (self.monster_stage == FISH_STAGE_FOLLOW)
			self.think = fish_follow;
		else if (self.monster_stage == FISH_STAGE_BORED)
			self.think = fish_hover;
	}
}

void fish_hover(void)
{
	float try;

	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SWIM01,$SWIM40);

	if (self.monster_stage == FISH_STAGE_BORED)
	{
		self.monster_duration -= 1;
		if (self.monster_duration <= 0)
			self.monster_stage = FISH_STAGE_MOVE;
	}

	if (random() < 0.02)
	{
		try = 0;
		while (try < 10)
		{
			self.movedir = randomv('-100 -100 -30', '100 100 30');
			tracearea(self.origin, self.origin + self.movedir, self.mins, self.maxs, FALSE, self);
			if (trace_fraction == 1)
			{
				self.think = fish_move;

				self.count = 170 + random(30);
				self.movedir_x /= 400;
				self.movedir_y /= 400;
				self.movedir_z /= 400;
				self.fish_speed = vhlen(self.movedir);
				try = 999;
			}
			try += 1;
		}
	}
}

void fish_die(void)
{
	remove(self);
}

/*QUAKED monster_fish (1 0 0) (-16 -16 -8) (16 16 8) 
Ambient Fish

-------------------------FIELDS-------------------------
skin:  0 = bright colored, 1 = darker colored
--------------------------------------------------------

*/
void monster_fish(void)
{
	precache_model2 ("models/fish.mdl");

	self.takedamage = DAMAGE_YES;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_FLY;
	self.flags (+) FL_SWIM | FL_MONSTER;
	self.yaw_speed = 2;
	self.hull = HULL_PLAYER;
	self.monster_stage = FISH_STAGE_MOVE;
	self.mass = 99999; // Big fishies!

//	self.drawflags (+) MLS_POWERMODE;

	setmodel (self, "models/fish.mdl");
	self.skin = 0;

	setsize (self, '-10 -10 -8', '10 10 8');
	self.health = 1;

	self.th_die = fish_die;
	self.think = fish_hover;
	thinktime self : HX_FRAME_TIME;
}

