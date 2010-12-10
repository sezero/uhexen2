/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/axe.hc,v 1.2 2007-02-07 16:57:49 sezero Exp $
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

void() T_PhaseMissileTouch =
{
	local float	damg;

	self.flags (-) FL_ONGROUND;	// So it never thinks it is touching the ground

	if (other == self.owner)
		return;		// don't explode on owner

	if ((self.enemy == other) && (other != world))  // Can't hit same enemy twice in a row but you can hit world twice
		return;

	self.cnt +=1;
//	self.velocity = self.velocity * 0.75;	// client's not interested in this happening...

	self.angles = vectoangles(self.velocity);

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		stopSound(self,0);

		remove(self);
		return;
	}

	if (other.health)	// Hit something that can be hurt
	{
		if(self.classname == "powerupaxeblade")
		{
			damg = random(25, 45);
			WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
			WriteByte (MSG_MULTICAST, TE_AXE_EXPLODE);
			WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 16);
			WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 16);
			WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 16);
			multicast(self.origin,MULTICAST_PHS_R);

			T_RadiusDamage (self, self.owner, 30.0, other);
			// get rid of it here or something?
		}
		else
		{
			damg = random(30,50);
		}
		T_Damage (other, self, self.owner, damg );
		self.counter -=1;
		self.enemy = other;
	}
	else
	{
		self.enemy = other;
		if (self.cnt <4)	// Bounce three times then die
		{
			WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
			WriteByte (MSG_MULTICAST, TE_AXE_BOUNCE);
			WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 16);
			WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 16);
			WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 16);
			multicast(self.origin,MULTICAST_PHS_R);
		}
		else
			self.counter = 0;
	}	

	// Time is up
	if (self.lifetime < time)
		self.counter = 0;

	if (self.counter < 1)
	{
		remove(self);
		return;
	}
	self.think();
};

void axeblade_gone(void)
{
	stopSound(self,0);

	if (self.skin==0)
		CreateLittleWhiteFlash(self.origin);
	else
		CreateLittleBlueFlash(self.origin);

	remove(self);
}

/*void axeblade_run (void) [ ++ 0 .. 5]
{
	self.angles = vectoangles(self.velocity);
	if (self.lifetime < time)
		axeblade_gone();
}*/

void axeblade_think(void)
{
	self.movedir = normalize(self.velocity);

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.movedir * 330.0, TRUE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_AXE);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);

	thinktime self : 0.3;

	if (self.lifetime < time)
		axeblade_gone();
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
	missile.movedir = missile.velocity;

	// PUT THIS BACK PLEASE
	missile.velocity = missile.velocity * 1100;
	
	missile.touch = T_PhaseMissileTouch;

	// Point it in the proper direction
    missile.angles = vectoangles(missile.velocity);
	missile.angles += angle_mod;

	// set missile duration
	missile.counter = 4;  // Can hurt two things before disappearing
	missile.cnt = 0;		// Counts number of times it has hit walls
	missile.lifetime = time + 2;  // Or lives for 2 seconds and then dies when it hits anything

	setorigin (missile, self.origin + self.proj_ofs  + v_forward*10 + v_right * 1);

	if (self.artifact_active & ART_TOMEOFPOWER)
	{
		missile.frags=TRUE;
		missile.classname = "powerupaxeblade";
		missile.skin = 1;
		missile.drawflags = (self.drawflags & MLS_MASKOUT)| MLS_POWERMODE;
	}
	else
		missile.classname = "axeblade";

//	thinktime missile : HX_FRAME_TIME;
//	missile.think = axeblade_run;

	missile.think = axeblade_think;
	thinktime missile : 0.3;

	missile.effects (+) EF_NODRAW;

	entity oldself;
	oldself = self;
	self = missile;

	missile.think();

	self = oldself;
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
		weapon_sound(self, "paladin/axgen.wav");

		launch_axe('0 3 0','0 0 0');    // Side
		launch_axe('0 -3 0','0 0 0');   // Side


		self.greenmana -= 8;
	}
	else if (self.greenmana >= 2)
	{
		FireMelee (WEAPON1_BASE_DAMAGE,WEAPON1_ADD_DAMAGE,64);

		if (self.greenmana >= 2)
		{
			weapon_sound(self, "paladin/axgen.wav");

			launch_axe('0 0 0','0 0 0');
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
	{
		weapon_sound(self, "weapons/vorpswng.wav");
	}

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
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");
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

