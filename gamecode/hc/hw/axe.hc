/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/axe.hc,v 1.1.1.1 2004-11-29 11:25:00 sezero Exp $
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:00  theoddone33
 * Inital import
 *
 * 
 * 14    4/10/98 10:58a Nalbury
 * moved some stuff from the phs to the pvs...
 * 
 * 13    4/07/98 9:07p Nalbury
 * No more reliable junk...
 * 
 * 12    4/04/98 5:47a Nalbury
 * Tweaked ALOT of damage amounts.  It's getting there...
 * 
 * 11    3/30/98 7:07a Nalbury
 * Cleaned up the movement for several projectiles...
 * 
 * 10    3/27/98 1:34p Mgummelt
 * Adding PHS_OVERRIDE_R channel flag and ATTN_LOOP to all door and plat
 * sounds, replacing all "null.wav" sounds with stopSound.
 * 
 * 9     3/23/98 5:04p Nalbury
 * 
 * 8     3/19/98 3:04p Nalbury
 * Only throw two axes now.
 * 
 * 6     3/17/98 1:08a Nalbury
 * weakened powered up axe a bit and made it a bit more net friendly.
 * 
 * 5     3/13/98 4:10p Nalbury
 * Adjusted powered up purifier and axe speed.
 * 
 * 4     3/12/98 12:21p Nalbury
 * Using weapon sound instead of regular sound.
 * 
 * 3     3/06/98 6:45a Nalbury
 * net optimizations
 * 
 * 2     3/04/98 3:00p Nalbury
 * Added some axe stuff...
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 54    8/26/97 7:38a Mgummelt
 * 
 * 53    8/13/97 3:39p Rlove
 * 
 * 52    8/08/97 6:21p Mgummelt
 * 
 * 51    8/06/97 2:51p Rlove
 * 
 * 50    7/31/97 11:41a Mgummelt
 * 
 * 49    7/25/97 11:22a Mgummelt
 * 
 * 48    7/24/97 3:53p Rlove
 * 
 * 47    7/21/97 3:03p Rlove
 * 
 * 46    7/15/97 2:33p Mgummelt
 * 
 * 45    7/15/97 2:30p Mgummelt
 * 
 * 44    7/14/97 6:11p Rlove
 * 
 * 43    7/12/97 1:31p Rlove
 * 
 * 42    7/12/97 9:09a Rlove
 * Reworked Assassin Punch Dagger
 * 
 * 41    6/20/97 10:16a Rlove
 * 
 * 40    6/18/97 1:41p Mgummelt
 * 
 * 39    6/16/97 11:42a Rlove
 * aim has to be given the origin of where the firing takes place at.
 * 
 * 38    6/16/97 8:05a Rlove
 * 
 * 37    6/09/97 11:20a Rlove
 * 
 * 36    6/09/97 10:13a Rlove
 * 
 * 35    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 * 34    6/02/97 9:55a Rlove
 * Changed where firing is done
 * 
 * 33    5/31/97 6:53a Rlove
 * New and hopefully improved ax
 * 
 * 29    5/27/97 10:57a Rlove
 * Took out old Id sound files
 * 
 * 26    5/19/97 12:01p Rlove
 * New sprites for axe
 * 
 * 25    5/19/97 8:58a Rlove
 * Adding sprites and such to the axe.
 * 
 * 24    5/16/97 1:52p Rlove
 * 
 * 23    5/15/97 1:33p Rlove
 * 
 * 22    5/12/97 11:06a Rlove
 * 
 * 21    5/12/97 10:31a Rlove
 * 
 * 19    5/07/97 11:03a Rlove
 * 
 * 16    5/05/97 5:40p Rlove
 * 
 * 14    4/21/97 9:16a Rlove
 * Tried new axe animations
 * 
 * 13    4/16/96 11:51p Mgummelt
 * 
 * 12    4/16/97 8:38a Rlove
 * Corrected axe loop when selected after purifier
 * 
 * 11    4/16/97 7:59a Rlove
 * Removed references to ammo_  fields
 * 
 * 10    4/15/97 10:14a Rlove
 * Changed cleric to crusader
 * 
 * 9     4/14/97 5:04p Rlove
 * 
 * 6     4/04/97 5:40p Rlove
 * 
 * 5     3/19/97 7:44a Rlove
 * Doors no longer open when an axe blade is near
 * 
 * 4     3/18/97 5:09p Rlove
 * 
 * 3     3/17/97 12:40p Rlove
 * New axe is in.
 * 
 * 2     3/10/97 8:30a Rlove
 * Added the axe weapon
 * 
 * 1     3/07/97 2:03p Rlove
 */
