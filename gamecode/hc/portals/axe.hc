/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/axe.hc,v 1.2 2007-02-07 16:59:29 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\axe\final\axe.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\axe\final
$origin 10 -10 10
$base BASE skin
$skin skin
$flags 0

$frame AxeRoot1

$frame 1stAxe1      1stAxe2      1stAxe3      1stAxe4      1stAxe5
$frame 1stAxe6      1stAxe7      1stAxe8      
$frame 1stAxe11     1stAxe12     1stAxe14     
$frame 1stAxe15     1stAxe17     1stAxe18          
$frame 1stAxe21     1stAxe22     1stAxe23
$frame 1stAxe25     1stAxe27     


float AXE_DAMAGE			= 24;
float AXE_ADD_DAMAGE		= 6;

void() T_PhaseMissileTouch;

void axeblade_gone(void)
{
	stopSound(self,CHAN_VOICE);
	stopSound(self,CHAN_WEAPON);
	//sound (self, CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
	//sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);

	if (self.skin==0)
		CreateLittleWhiteFlash(self.origin);
	else
		CreateLittleBlueFlash(self.origin);

	remove(self.goalentity);
	remove(self);
}

void axeblade_run (void) [ ++ 0 .. 5]
{
//dvanceFrame(0,5);
	if (self.lifetime < time)
		axeblade_gone();
}


void axetail_run (void)
{
	if(!self.owner)
		remove(self);
	else
	{
		self.origin = self.owner.origin;
		self.velocity = self.owner.velocity;
		self.owner.angles = vectoangles(self.velocity);
		self.angles = self.owner.angles;
		self.origin = self.owner.origin;
	}
}


void launch_axtail (entity axeblade)
{
	local entity tail;

	tail = spawn ();
	tail.movetype = MOVETYPE_NOCLIP;
	tail.solid = SOLID_NOT;
	tail.classname = "ax_tail";
	setmodel (tail, "models/axtail.mdl");
	setsize (tail, '0 0 0', '0 0 0');		
	tail.drawflags (+)DRF_TRANSLUCENT;

	tail.owner = axeblade;
	tail.origin = tail.owner.origin;
	tail.velocity = tail.owner.velocity;
    tail.angles = tail.owner.angles;

	axeblade.goalentity = tail;

}


void launch_axe (vector dir_mod,vector angle_mod)
{
	entity missile;

	self.attack_finished = time + 0.4;

	missile = spawn ();

	CreateEntityNew(missile,ENT_AXE_BLADE,"models/axblade.mdl",SUB_Null);

	missile.owner = self;
	missile.classname = "ax_blade";
		
	// set missile speed	
	makevectors (self.v_angle + dir_mod);
	missile.velocity = normalize(v_forward);
	missile.velocity = missile.velocity * 900;
	
	missile.touch = T_PhaseMissileTouch;

	// Point it in the proper direction
    missile.angles = vectoangles(missile.velocity);
	missile.angles += angle_mod;

	// set missile duration
	missile.counter = 4;  // Can hurt two things before disappearing
	missile.cnt = 0;		// Counts number of times it has hit walls
	missile.lifetime = time + 2;  // Or lives for 2 seconds and then dies when it hits anything

	setorigin (missile, self.origin + self.proj_ofs  + v_forward*10 + v_right * 1);

//sound (missile, CHAN_VOICE, "paladin/axblade.wav", 1, ATTN_NORM);

	if (self.artifact_active & ART_TOMEOFPOWER)
	{
		missile.frags=TRUE;
		missile.classname = "powerupaxeblade";
		missile.skin = 1;
		missile.drawflags = (self.drawflags & MLS_MASKOUT)| MLS_POWERMODE;
	}
	else
		missile.classname = "axeblade";

	missile.lifetime = time + 2;
	thinktime missile : HX_FRAME_TIME;
	missile.think = axeblade_run;

	launch_axtail(missile);

}


/*
================
axeblade_fire
================
*/
void() axeblade_fire =
{
	if ((self.artifact_active & ART_TOMEOFPOWER) && (self.greenmana >= 8))
	{
		FireMelee (50,25,64);
		sound (self, CHAN_WEAPON, "paladin/axgenpr.wav", 1, ATTN_NORM);

//		makevectors (self.v_angle);
//		CreateLittleBlueFlash(self.origin + v_forward*40 + v_up * 42);

		launch_axe('0 0 0','0 0 0');	// Middle

		launch_axe('0 5 0','0 0 0');    // Side
		launch_axe('0 -5 0','0 0 0');   // Side

//		launch_axe('5 0 0','0 0 0');	// Top
//		launch_axe('-5 0 0','0 0 0');	// Bottom

		self.greenmana -= 8;
	}
	else if (self.greenmana >= 2)
	{
		FireMelee (WEAPON1_BASE_DAMAGE,WEAPON1_ADD_DAMAGE,64);

		if (self.greenmana >= 2)
		{
			sound (self, CHAN_WEAPON, "paladin/axgen.wav", 1, ATTN_NORM);

	//		makevectors (self.v_angle);
	//		CreateLittleWhiteFlash(self.origin + self.proj_ofs + v_forward*24 + v_right * 2);

			launch_axe('0 0 0','0 0 300');
			self.greenmana -= 2;
		}
	}

};

void axe_ready (void)
{
	self.th_weapon=axe_ready;
	self.weaponframe = $AxeRoot1;
}

void axe_select (void)
{
	self.wfs = advanceweaponframe($1stAxe18,$1stAxe3);
	if (self.weaponframe == $1stAxe14)
		sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);

	self.weaponmodel = "models/axe.mdl";
	self.th_weapon=axe_select;
	self.last_attack=time;

	if (self.wfs == WF_LAST_FRAME)
	{
		self.attack_finished = time - 1;
		axe_ready();
	}
}

void axe_deselect (void)
{
	self.wfs = advanceweaponframe($1stAxe18,$1stAxe3);
	self.th_weapon=axe_deselect;
	self.oldweapon = IT_WEAPON3;

	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}


void axe_a (void)
{

	self.wfs = advanceweaponframe($1stAxe1,$1stAxe25);
	self.th_weapon = axe_a;

	// These frames are used during selection animation
	if ((self.weaponframe >= $1stAxe2) && (self.weaponframe <= $1stAxe4))
		self.weaponframe +=1;
	else if ((self.weaponframe >= $1stAxe6) && (self.weaponframe <= $1stAxe7))
		self.weaponframe +=1;

	if (self.weaponframe == $1stAxe15)
	{
		sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		axeblade_fire();
	}

	if (self.wfs == WF_LAST_FRAME)
		axe_ready();
}

void pal_axe_fire(void)
{
	axe_a ();

	if (self.artifact_active & ART_TOMEOFPOWER)
  		self.attack_finished = time + .7;
	else
  		self.attack_finished = time + .35;
}

