/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/weapons.hc,v 1.2 2005-03-17 18:09:38 sezero Exp $
 */
/*
*/
void (entity targ, entity inflictor, entity attacker, float damage) T_Damage;
void(entity bomb, entity attacker, float rad, entity ignore) T_RadiusDamage;
void(vector org, vector vel, float damage,entity victim) SpawnPuff;

void() UseInventoryItem;
void() ImpulseCommands;

//============================================================================

vector() wall_velocity =
{
	local vector	vel;
	
	vel = normalize (self.velocity);
	vel = normalize(vel + v_up*random(-0.5,0.5) + v_right*random(-0.5,0.5));
	vel = vel + 2*trace_plane_normal;
	vel = vel * 200;
	
	return vel;
};

/*
================
spawn_touchpuff
================
*/
void(float damage,entity victim) spawn_touchpuff =
{
vector	vel;

	vel = wall_velocity () * 0.2;
	SpawnPuff (self.origin + vel*0.01, vel, damage,victim);
};

void() T_MissileTouch =
{
	float	damg;
//	vector delta;  // Quantis never crashes

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(100,120);
	if(self.classname=="dragonspike")
		damg=damg * 0.25;
	else if(self.classname=="dragonball")
		damg=damg * 0.5;
	else if (self.classname == "mummymissile")
		damg = random(5,15);
	
	if (other.health)
	{
		T_Damage (other, self, self.owner, damg );
	}

	// don't do radius damage to the other, because all the damage
	// was done in the impact
	if(self.classname=="dragonspike")
		T_RadiusDamage (self, self.owner, 60, other);
	else if ((self.classname=="mummymissile") || (self.classname=="green_arrow") || (self.classname=="red_arrow"))
		damg = damg;  // No radius damage
	else
		T_RadiusDamage (self, self.owner, 120, other);


//	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
	self.origin = self.origin - 8*normalize(self.velocity);

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_EXPLOSION);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);

	BecomeExplosion (FALSE);
};

void() T_PhaseMissileTouch =
{
	local float	damg;
//	entity new;

	self.flags (-) FL_ONGROUND;	// So it never thinks it is touching the ground

	if (other == self.owner)
		return;		// don't explode on owner

	if ((self.enemy == other) && (other != world))  // Can't hit same enemy twice in a row but you can hit world twice
		return;

	if ((self.classname == "axeblade") || (self.classname == "powerupaxeblade"))
	{
		self.cnt +=1;
		self.velocity = self.velocity * 0.75;

		self.angles = vectoangles(self.velocity);

		sound (self, CHAN_WEAPON, "paladin/axric1.wav", 1, ATTN_NORM);
			
		if (self.goalentity)
			if (self.goalentity.classname=="ax_tail")
			{
				self.goalentity.think = axetail_run;
				self.goalentity.nextthink = time + HX_FRAME_TIME;
			}
	}

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		stopSound(self,CHAN_VOICE);
		stopSound(self,CHAN_WEAPON);
		//sound (self, CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
		//sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);

		if ((self.classname == "axeblade") || (self.classname == "powerupaxeblade"))
			remove(self.goalentity); // Remove tail
		remove(self);
		return;
	}

	if (other.health)	// Hit something that can be hurt
	{
		damg = random(30,50);
		T_Damage (other, self, self.owner, damg );
		self.counter -=1;
		self.enemy = other;
	}
	else
	{
		self.enemy = other;
		if (self.cnt <4)	// Bounce three times then die
		{
			if (self.classname == "powerupaxeblade")
				CreateBSpark (self.origin - '0 0 30');
			else
				CreateSpark (self.origin - '0 0 30');
		}
		else
			self.counter = 0;
	}	

	// Time is up
	if (self.lifetime < time)
		self.counter = 0;

	if ((other.health) || (self.counter < 1))
	{
		sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

		if (self.classname == "powerupaxeblade")
			CreateBlueExplosion (self.origin);
		else
			starteffect(CE_SM_EXPLOSION , self.origin);
	}
	else
		sound (self, CHAN_WEAPON, "paladin/axric1.wav", 1, ATTN_IDLE);

	if (self.counter < 1)
	{
		if ((self.classname == "axeblade") || (self.classname == "powerupaxeblade"))
			remove(self.goalentity); // Remove tail

		stopSound(self,CHAN_VOICE);
		//sound (self, CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
		remove(self);
	}
};

//=============================================================================

//=============================================================================

void() spike_touch;
//void() superspike_touch;


/*
===============
launch_spike
===============
*/
void(vector org, vector dir) launch_spike =
{
	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;

	newmis.angles = vectoangles(dir);
	
	newmis.touch = spike_touch;
	newmis.classname = "spike";
	newmis.think = SUB_Remove;
	newmis.nextthink = time + 6;
	setmodel (newmis, "models/spike.mdl");
	setsize (newmis, VEC_ORIGIN, VEC_ORIGIN);		
	setorigin (newmis, org);

	newmis.velocity = dir * 1000;
};



void() spike_touch =
{
//float rand;
	if (other == self.owner)
		return;

	if (other.solid == SOLID_TRIGGER)
		return;	// trigger field, do nothing

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}
	
// hit something that bleeds
	if (other.takedamage)
	{
		spawn_touchpuff (9,other);
		T_Damage (other, self, self.owner, 9);
	}
	else
	{
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		
		if (self.classname == "wizspike")
			WriteByte (MSG_BROADCAST, TE_WIZSPIKE);
		else if (self.classname == "knightspike")
			WriteByte (MSG_BROADCAST, TE_KNIGHTSPIKE);
		else
			WriteByte (MSG_BROADCAST, TE_SPIKE);
		WriteCoord (MSG_BROADCAST, self.origin_x);
		WriteCoord (MSG_BROADCAST, self.origin_y);
		WriteCoord (MSG_BROADCAST, self.origin_z);
	}

	remove(self);

};


/*void() superspike_touch =
{
local float rand;
	if (other == self.owner)
		return;

	if (other.solid == SOLID_TRIGGER)
		return;	// trigger field, do nothing

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}
	
// hit something that bleeds
	if (other.takedamage)
	{
		spawn_touchpuff (18,other);
		T_Damage (other, self, self.owner, 18);
	}
	else
	{
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_SUPERSPIKE);
		WriteCoord (MSG_BROADCAST, self.origin_x);
		WriteCoord (MSG_BROADCAST, self.origin_y);
		WriteCoord (MSG_BROADCAST, self.origin_z);
	}

	remove(self);

};*/

/*
===============================================================================

PLAYER WEAPON USE

===============================================================================
*/
void() Nec_Change_Weapon;
void() Pal_Change_Weapon;
void() Cru_Change_Weapon;
void() Ass_Change_Weapon;
void() Suc_Change_Weapon;

void W_SetCurrentAmmo (void)
{
	if(self.sheep_time>time)
		return;

	wp_deselect = 0;

	self.button0=FALSE;
//	attck_cnt=0;

	if (self.oldweapon==self.weapon)		// Until we get multi skinned weapons
		return;

	if(self.attack_finished<time)
		self.attack_finished=time;

	switch (self.weapon)
	{
	case IT_WEAPON1:
		self.weaponmodel="";
		self.weaponframe = 0;

		switch(self.playerclass)
		{
		case CLASS_PALADIN:
			gauntlet_select();
			break;
		case CLASS_CRUSADER:
			warhammer_select();
			break;
		case CLASS_NECROMANCER:
			sickle_select();
			break;
		case CLASS_SUCCUBUS:
			bloodrain_select();
			break;
		default:	//CLASS_ASSASSIN
			punchdagger_select();
			break;
		}
		break;
	case IT_WEAPON2:
		self.weaponmodel="";
		self.weaponframe = 0;
	
		switch(self.playerclass)
		{
		case CLASS_PALADIN:
			vorpal_select();
			break;
		case CLASS_CRUSADER:
			icestaff_select();
			break;
		case CLASS_NECROMANCER:
			if(self.oldweapon!=IT_WEAPON3)
				magicmis_select();
			else
				magicmis_select_from_bone();
			break;
		case CLASS_SUCCUBUS:
			acidorb_select();
			break;
		default:	//CLASS_ASSASSIN
			crossbow_select();
			break;
		}
		break;
	case IT_WEAPON3:
		self.weaponmodel="";
		self.weaponframe = 0;
		switch(self.playerclass)
		{
		case CLASS_PALADIN:
			axe_select();
			break;
		case CLASS_CRUSADER:
			meteor_select();
			break;
		case CLASS_NECROMANCER:
			if(self.oldweapon!=IT_WEAPON2)
				boneshard_select();
			else
				boneshard_select_from_mmis();
			break;
		case CLASS_SUCCUBUS:
			flameorb_select();
			break;
		default:	//CLASS_ASSASSIN
			grenade_select();
			break;
		}
	break;
	case IT_WEAPON4:
		self.weaponmodel="";
		self.weaponframe = 0;

		switch(self.playerclass)
		{
		case CLASS_PALADIN:
			purifier_select();
			break;
		case CLASS_CRUSADER:
			sunstaff_select();
			break;
		case CLASS_NECROMANCER:
			ravenstaff_select();
			break;
		case CLASS_SUCCUBUS:
			lightning_select();
			break;
		default:	//CLASS_ASSASSIN
			setstaff_select();
			break;
		}
	break;
	}

//All players will have to do this eventually, to reset
//the stand, pain, run & fly functions for the different weapons

	//if(self.weapon!=self.oldweapon)
	{
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			Pal_Change_Weapon();
			break;
		case CLASS_CRUSADER:
			Cru_Change_Weapon();
			break;
		case CLASS_NECROMANCER:
			Nec_Change_Weapon();
			break;
		case CLASS_SUCCUBUS:
			Suc_Change_Weapon();
			break;
		default: //CLASS_ASSASSIN:
			Ass_Change_Weapon();
			break;
		}
		if(self.hull!=HULL_CROUCH)
			self.act_state=ACT_STAND;
	}

/*	if (self.flags2&FL_CAMERA_VIEW)
	{  // FIXME - couldn't this use oldweapon??? rather than lastweapon
		self.lastweapon=self.weaponmodel;
		self.weaponmodel="";
	}
*/

	if(self.flags2&FL_SUMMONED&&self.weapon!=IT_WEAPON3)
	{
		self.flags2(-)FL_SUMMONED;
		self.effects(-)EF_DARKLIGHT;
	}
//	if (self.deselect_time < time)
		self.oldweapon=self.weapon;	
}

float W_CheckNoAmmo (float check_weapon)
{
	if (check_weapon == IT_WEAPON1)
		return TRUE;
	
	switch (self.playerclass)
	{
	case CLASS_ASSASSIN:
		switch (check_weapon)
		{
		case IT_WEAPON4:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 30 && self.greenmana >= 30)
					return TRUE;
			}
			else if(self.bluemana >= 1 && self.greenmana >= 1)
					return TRUE;
		break;
		case IT_WEAPON3:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 12)
					return TRUE;
			}
			else if(self.greenmana >= 3)
					return TRUE;
			break;
		case IT_WEAPON2:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 10)
					return TRUE;
			}
			else if(self.bluemana >= 3)
					return TRUE;
		break;
		}
	break;
	case CLASS_SUCCUBUS:
		switch (check_weapon)
		{
		case IT_WEAPON4:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 2 && self.greenmana >= 2)
					return TRUE;
			}
			else if(self.bluemana >= 6 && self.greenmana >= 6)
					return TRUE;
		break;
		case IT_WEAPON3:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 10)
					return TRUE;
			}
			else if(self.greenmana >= 4)
					return TRUE;
		break;
		case IT_WEAPON2:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 8)
					return TRUE;
			}
			else if(self.bluemana >= 3)
					return TRUE;
		break;
		}
	break;
	case CLASS_CRUSADER:
		switch (check_weapon)
		{
		case IT_WEAPON4:
			if(self.bluemana >= 2 && self.greenmana >= 2)
				return TRUE;
			self.effects(-)EF_BRIGHTLIGHT;
		break;
		case IT_WEAPON3:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 20)
					return TRUE;
			}
			else if(self.greenmana >= 8)
					return TRUE;
		break;
		case IT_WEAPON2:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 10)
					return TRUE;
			}
			else if(self.bluemana >= 1)
					return TRUE;
		break;
		}
	break;
	case CLASS_NECROMANCER:
		switch (check_weapon)
		{
		case IT_WEAPON4:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 16 && self.greenmana >= 16)
					return TRUE;
			}
			else if(self.bluemana >= 8 && self.greenmana >= 8)
					return TRUE;
		break;
		case IT_WEAPON3:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 20)
					return TRUE;
			}
			else if(self.greenmana >= 1)
					return TRUE;
		break;
		case IT_WEAPON2:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 10)
					return TRUE;
			}
			else if(self.bluemana >= 2)
					return TRUE;
		break;
		}
	break;
	case CLASS_PALADIN:
		switch (check_weapon)
		{
		case IT_WEAPON4:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 8 && self.greenmana >= 8)
					return TRUE;
			}
			else if(self.bluemana >= 2 && self.greenmana >= 2)
					return TRUE;
		break;
		case IT_WEAPON3:
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 8)
					return TRUE;
			}
			else if(self.greenmana >= 2)
					return TRUE;
		break;
		case IT_WEAPON2:
/*			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.bluemana >= 4)
					return TRUE;
			}
			else if(self.bluemana >= 2) 
					return TRUE;*/
			return TRUE;
		break;
		}
	break;
	default:
		dprintf("Unknown class: %s!\n",self.playerclass);
	break;
	}
	return FALSE;
}

//=========================
// W_BestWeapon : choose the most powerful weapon the player has ammo for
//=========================
void() W_BestWeapon =
{
	if (W_CheckNoAmmo (IT_WEAPON4) && (self.items & IT_WEAPON4))
		self.weapon = IT_WEAPON4;
	else if (W_CheckNoAmmo (IT_WEAPON3) && (self.items & IT_WEAPON3))
		self.weapon = IT_WEAPON3;
	else if (W_CheckNoAmmo (IT_WEAPON2) && (self.items & IT_WEAPON2))
		self.weapon = IT_WEAPON2;
	else self.weapon = IT_WEAPON1;
};

/*
============
W_Attack

An attack impulse can be triggered now
============
*/
void()player_sheep_baa;
void() W_Attack =
{
	if (!W_CheckNoAmmo (self.weapon))
	{
		W_BestWeapon ();
		W_SetCurrentWeapon ();
		return;
	}
	
	if(self.attack_finished>time)
		return;

	if(self.sheep_time>time)
	{
		player_sheep_baa();
		return;
	}

	if(self.viewentity!=self&&self.viewentity.classname!="chasecam")
		return;

	self.show_hostile = time + 1;	// wake monsters up
	self.last_attack=time;			//In attack state

	if(self.hull==HULL_PLAYER)
		self.act_state=ACT_ATTACK;
	else
		self.act_state=ACT_CROUCH_MOVE;

	switch (self.weapon) 
	{
	case IT_WEAPON1:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			pal_gauntlet_fire();
			break;
		case CLASS_NECROMANCER:
			self.th_missile();
			break;
		case CLASS_ASSASSIN:
			Ass_Pdgr_Fire();
			break;
		case CLASS_CRUSADER:
			Cru_Wham_Fire();
			break;
		case CLASS_SUCCUBUS:
			Suc_Blrn_Fire();
			break;
		}
	break;
	case IT_WEAPON2:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			pal_vorpal_fire();
		break;
		case CLASS_ASSASSIN:
			crossbow_fire();
		break;
		case CLASS_SUCCUBUS:
			Suc_Aorb_Fire();
		break;
		case CLASS_CRUSADER:
			if(self.th_weapon==icestaff_idle)
				Cru_Ice_Fire();
		break;
		case CLASS_NECROMANCER:
			self.th_missile();
		break;
		}
	break;
	case IT_WEAPON3:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			pal_axe_fire();
		break;
		case CLASS_ASSASSIN:
			grenade_throw();
		break;
		case CLASS_SUCCUBUS:
			Suc_Forb_Fire();
		break;
		case CLASS_CRUSADER:
			Cru_Met_Attack();
		break;
		case CLASS_NECROMANCER:
			self.th_missile();
		break;
		}
	break;
	case IT_WEAPON4:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			pal_purifier_fire();
		break;
		case CLASS_ASSASSIN:
			ass_setstaff_fire();
		break;
		case CLASS_CRUSADER:
			Cru_Sun_Fire();
		break;
		case CLASS_NECROMANCER:
			ravenstaff_fire();
		break;
		case CLASS_SUCCUBUS:
			Suc_Litn_Fire();
		break;
		}
	break;
	}
};


void W_DeselectWeapon (void)
{
	wp_deselect = 1;

	switch (self.oldweapon)
	{
	case IT_WEAPON1:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			gauntlet_deselect();
		break;
		case CLASS_CRUSADER:
			warhammer_deselect();
		break;
		case CLASS_ASSASSIN:
			punchdagger_deselect();
		break;
		case CLASS_SUCCUBUS:
			bloodrain_deselect();
		break;
		case CLASS_NECROMANCER:
			sickle_deselect();
		break;
		}
	break;
	case IT_WEAPON2:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			vorpal_deselect();
		break;
		case CLASS_CRUSADER:
			icestaff_deselect();
		break;
		case CLASS_ASSASSIN:
			crossbow_deselect();
		break;
		case CLASS_SUCCUBUS:
			acidorb_deselect();
		break;
		case CLASS_NECROMANCER:
			if(self.weapon!=IT_WEAPON3)
				magicmis_deselect();
			else
				W_SetCurrentAmmo();
		break;
		}
	break;
	case IT_WEAPON3:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			axe_deselect();
		break;
		case CLASS_CRUSADER:
			meteor_deselect();
		break;
		case CLASS_ASSASSIN:
			grenade_deselect();
		break;
		case CLASS_NECROMANCER:
			if(self.weapon!=IT_WEAPON2)
				boneshard_deselect();
			else
				W_SetCurrentAmmo();
		break;
		case CLASS_SUCCUBUS:
			flameorb_deselect();
		break;
		}
	break;
	case IT_WEAPON4:
		switch (self.playerclass)
		{
		case CLASS_PALADIN:
			purifier_deselect();
		break;
		case CLASS_CRUSADER:
			sunstaff_deselect();
		break;
		case CLASS_ASSASSIN:
			setstaff_deselect();
		break;
		case CLASS_NECROMANCER:
			ravenstaff_deselect();
		break;
		case CLASS_SUCCUBUS:
			lightning_deselect();
		break;
		}
	break;
	default:
		W_SetCurrentAmmo();
	break;
	}
}

/*
============
W_ChangeWeapon

============
*/
void() W_ChangeWeapon =
{
	if(self.sheep_time>time)
	{
		return;
	}

	if(self.viewentity!=self&&self.viewentity.classname!="chasecam")
	{
		return;
	}

	if(self.attack_finished>time)
	{
		return;
	}

float	it, am, fl;
	
	it = self.items;
	am = 0;
	
	switch (self.impulse)
	{
	case 1:
		fl = IT_WEAPON1;
	break;
	case 2:
		fl = IT_WEAPON2;
	break;
	case 3:
		fl = IT_WEAPON3;
		if (self.bluemana < 2)
			am = 1;
	break;
	case 4:
		fl = IT_WEAPON4;
		if ((self.bluemana < 1) && (self.greenmana <1))
			am = 1;
	break;
	}

	self.impulse = 0;

	if (!(self.items & fl))
	{
		sprint (self, STR_NOCARRYWEAPON);
		return;
	}

	if(!W_CheckNoAmmo(fl))
	{
		sprint (self, STR_NOTENOUGHMANA);
		return;
	}

//
// set weapon, set ammo
//
	self.oldweapon=self.weapon;//for deselection animation
	self.weapon = fl;		
	W_SetCurrentWeapon ();
};

/*
============
CheatCommand
============
*/
void() CheatCommand =
{
	if(deathmatch||coop||skill>2)
		return;

	self.items(+)IT_WEAPON1|IT_WEAPON2|IT_WEAPON3|IT_WEAPON4|IT_WEAPON4_1|IT_WEAPON4_2;

	self.bluemana = self.max_mana;
	self.greenmana = self.max_mana;

	self.impulse = 0;

	if(self.attack_finished<time)
	{
		self.oldweapon = self.weapon;
		self.weapon = IT_WEAPON4;
		W_SetCurrentWeapon ();
	}
};

/*
============
CycleWeaponCommand

Go to the next weapon with ammo
============
*/
void() CycleWeaponCommand =
{
float	it, am;

	if(self.attack_finished>time)
		return;

	self.impulse = 0;

	self.items (+) IT_WEAPON1;
	it = self.items;

	while (1)
	{
		am = 0;

		switch (self.weapon)
		{
		case IT_WEAPON4:
			self.weapon = IT_WEAPON1;
		break;
		case IT_WEAPON1:
			self.weapon = IT_WEAPON2;
			if (self.bluemana < 1)
			{
				if (self.playerclass != CLASS_PALADIN)
					am = 1;
			}
		break;
		case IT_WEAPON2:
			self.weapon = IT_WEAPON3;
			if (self.greenmana < 1)
				am = 1;
		break;
		case IT_WEAPON3:
			self.weapon = IT_WEAPON4;
			if ((self.bluemana < 1) || (self.greenmana<1))
				am = 1;
		break;
		}
	
		if ((it & self.weapon) && am == 0)
		{
			W_SetCurrentWeapon ();
			return;
		}
	}

};

/*
============
CycleWeaponReverseCommand

Go to the prev weapon with ammo
============
*/
void() CycleWeaponReverseCommand =
{
	local	float	it, am;
	
	it = self.items;
	self.impulse = 0;

	while (1)
	{
		am = 0;

		switch (self.weapon)
		{
		case IT_WEAPON4:
			self.weapon = IT_WEAPON3;
			if (self.greenmana < 1)
				am = 1;
		break;
		case IT_WEAPON3:
			self.weapon = IT_WEAPON2;
		break;
		case IT_WEAPON2:
			self.weapon = IT_WEAPON1;
		break;
		case IT_WEAPON1:
			self.weapon = IT_WEAPON4;
			if ((self.bluemana < 1) && (self.greenmana<1))
				am = 1;
		break;
		}
	
		if ( (it & self.weapon) && am == 0)
		{
			W_SetCurrentWeapon ();
			return;
		}
	}

};

/*
============
ServerflagsCommand

Just for development
============
*/
void() ServerflagsCommand =
{
	serverflags = serverflags * 2 + 1;
};


/*
============
W_WeaponFrame

Called every frame so impulse events can be handled as well as possible
============
*/
void() W_WeaponFrame =
{
	ImpulseCommands ();

	if (time < self.attack_finished)
		return;

// check for attack
	if (self.button0)
	{
		W_Attack ();
	}
};

/*
========
	ClassChangeWeapon - Player is changing class so change weapon model to match. Called from C code
========
*/
void ClassChangeWeapon(void)
{
	self.drawflags(-)MLS_ABSLIGHT|DRF_TRANSLUCENT;

	if(self.sheep_time>time)
		return;

	self.weaponframe = 0;
	if (self.playerclass==CLASS_PALADIN)
	{
		switch (self.weapon)
		{
		case  IT_WEAPON1:
			self.th_weapon=gauntlet_select;
			self.weaponmodel = "models/gauntlet.mdl";
		break;
		case IT_WEAPON2:
			self.th_weapon=vorpal_select;
			self.weaponmodel = "models/vorpal.mdl";
		break;
		case IT_WEAPON3:
			self.th_weapon=axe_select;
			self.weaponmodel = "models/axe.mdl";
		break;
		case IT_WEAPON4:
			self.th_weapon=purifier_select;
			self.weaponmodel = "models/purifier.mdl";
		break;
		}
	}
	else if (self.playerclass==CLASS_CRUSADER)
	{
		switch (self.weapon)
		{
		case IT_WEAPON1:
			self.th_weapon=warhammer_select;
			self.weaponmodel = "models/warhamer.mdl";
		break;
		case IT_WEAPON2:
			self.th_weapon=icestaff_select;
			self.weaponmodel = "models/icestaff.mdl";
		break;
		case IT_WEAPON3:
			self.th_weapon=meteor_select;
			self.weaponmodel = "models/meteor.mdl";
		break;
		case IT_WEAPON4:
			self.th_weapon=sunstaff_select;
			self.weaponmodel = "models/sunstaff.mdl";
		break;
		}
	}
	else if (self.playerclass==CLASS_NECROMANCER)
	{
		switch (self.weapon)
		{
		case IT_WEAPON1:
			self.th_weapon=sickle_select;
			self.weaponmodel = "models/sickle.mdl";
		break;
		case IT_WEAPON2:
			self.th_weapon=sickle_select;
			self.weaponmodel = "models/spllbook.mdl";  // FIXME: still need these models
		break;
		case IT_WEAPON3:
			self.th_weapon=sickle_select;
			self.weaponmodel = "models/spllbook.mdl";
		break;
		case IT_WEAPON4:
			self.th_weapon=ravenstaff_select;
			self.weaponmodel = "models/ravenstf.mdl";
		break;
		}
	}
	else if (self.playerclass==CLASS_ASSASSIN)
	{
		switch (self.weapon)
		{
		case IT_WEAPON1:
			self.th_weapon=punchdagger_select;
			self.weaponmodel = "models/punchdgr.mdl";
		break;
		case IT_WEAPON2:
			self.th_weapon=crossbow_select;
			self.weaponmodel = "models/crossbow.mdl";
		break;
		case IT_WEAPON3:
			self.th_weapon=grenade_select;
			self.weaponmodel = "models/v_assgr.mdl";
		break;
		case IT_WEAPON4:
			self.th_weapon=setstaff_select;
			self.weaponmodel = "models/scarabst.mdl";
		break;
		}
	}
	else if (self.playerclass==CLASS_SUCCUBUS)
	{
		switch (self.weapon)
		{
		case IT_WEAPON1:
			self.th_weapon=bloodrain_select;
			self.weaponmodel = "models/sucwp1.mdl";
		break;
		case IT_WEAPON2:
			self.th_weapon=acidorb_select;
			self.weaponmodel = "models/sucwp2.mdl";
		break;
		case IT_WEAPON3:
			self.th_weapon=flameorb_select;
			self.weaponmodel = "models/sucwp3.mdl";
		break;
		case IT_WEAPON4:
			self.th_weapon=lightning_select;
			self.weaponmodel = "models/sucwp4.mdl";
		break;
		}
	}
//FIXME: take off all timed effects, lighting tinting, drawflags,
//	power-ups, etc.  Reset max health, abilities, etc.
	SetModelAndThinks();
	self.act_state=ACT_STAND;
}


void W_SetCurrentWeapon (void) 
{
	if(self.attack_finished>time)
	{
		return;
	}

	if(self.sheep_time>time)
	{
		return;
	}

	self.button0=FALSE;
//	attck_cnt=0;

	if (self.oldweapon==self.weapon)		// Until we get multi skinned weapons
	{
		return;
	}

	self.attack_finished=time + 999;
	W_DeselectWeapon ();
}
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/29 11:36:16  sezero
 * Initial import
 *
 * 
 * 25    3/20/98 7:42p Mgummelt
 * 
 * 24    3/17/98 4:23p Mgummelt
 * 
 * 23    3/17/98 4:22p Mgummelt
 * 
 * 22    3/16/98 8:31p Jweier
 * 
 * 21    3/14/98 6:37p Mgummelt
 * 
 * 20    3/13/98 3:27a Mgummelt
 * Replaced all sounds that played a null.wav with stopSound commands
 * 
 * 19    3/11/98 6:21p Mgummelt
 * 
 * 18    3/06/98 4:55p Mgummelt
 * 
 * 17    3/06/98 12:35a Jmonroe
 * made caserange work, switched some more things
 * 
 * 16    3/04/98 4:24p Mgummelt
 * 
 * 15    3/02/98 1:19a Jmonroe
 * added dm map cycling for keep and tibet.
 * reduced code size by changing to switch
 * 
 * 14    2/20/98 6:15p Jmonroe
 * removed unused variables
 * 
 * 13    2/18/98 2:57p Jweier
 * 
 * 12    2/12/98 5:55p Jmonroe
 * remove unreferenced funcs
 * 
 * 11    2/12/98 2:48p Mgummelt
 * 
 * 10    2/11/98 10:56p Mgummelt
 * 
 * 9     2/08/98 4:28p Mgummelt
 * 
 * 8     2/06/98 3:47p Mgummelt
 * 
 * 215   10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 213   9/29/97 3:00p Rlove
 * 
 * 212   9/16/97 11:45a Rlove
 * 
 * 211   9/11/97 9:05a Mgummelt
 * 
 * 210   9/09/97 3:58p Mgummelt
 * 
 * 209   9/09/97 2:26p Mgummelt
 * 
 * 208   9/01/97 6:01p Rlove
 * 
 * 207   9/01/97 3:26p Mgummelt
 * 
 * 206   8/31/97 4:03p Mgummelt
 * 
 * 205   8/31/97 3:57p Mgummelt
 * 
 * 204   8/31/97 2:36p Mgummelt
 * 
 * 203   8/31/97 11:38a Mgummelt
 * To which I say- shove where the sun don't shine- sideways!  Yeah!
 * How's THAT for paper cut!!!!
 * 
 * 202   8/31/97 8:52a Mgummelt
 * 
 * 201   8/30/97 6:58p Mgummelt
 * 
 * 200   8/29/97 11:02p Mgummelt
 * 
 * 199   8/29/97 4:17p Mgummelt
 * Long night
 * 
 * 198   8/28/97 2:42p Mgummelt
 * 
 * 197   8/27/97 10:22p Mgummelt
 * 
 * 196   8/27/97 7:06p Rjohnson
 * Fix for weapons
 * 
 * 195   8/26/97 6:01p Mgummelt
 * 
 * 194   8/26/97 7:38a Mgummelt
 * 
 * 193   8/26/97 2:27a Mgummelt
 * 
 * 192   8/26/97 12:03a Mgummelt
 * 
 * 191   8/25/97 11:32p Mgummelt
 * 
 * 190   8/25/97 4:06p Rlove
 * 
 * 189   8/25/97 2:27p Rlove
 * 
 * 188   8/25/97 1:09a Mgummelt
 * 
 * 187   8/24/97 7:25p Rlove
 * 
 * 186   8/21/97 4:04p Rlove
 * 
 * 185   8/20/97 11:22a Rlove
 * 
 * 184   8/19/97 12:57p Mgummelt
 * 
 * 183   8/19/97 10:04a Rjohnson
 * Removed camera stuff
 * 
 * 182   8/17/97 5:34p Mgummelt
 * 
 * 181   8/17/97 3:06p Mgummelt
 * 
 * 180   8/16/97 7:57a Rlove
 * Moved ImpulseCommands in WeaponFrame
 * 
 * 179   8/12/97 10:58p Mgummelt
 * 
 * 178   8/01/97 9:48p Mgummelt
 * 
 * 177   8/01/97 6:14p Mgummelt
 * 
 * 176   7/30/97 11:17p Mgummelt
 * 
 * 175   7/30/97 11:14p Mgummelt
 * 
 * 174   7/30/97 3:33p Mgummelt
 * 
 * 173   7/29/97 3:46p Mgummelt
 * 
 * 172   7/29/97 10:52a Mgummelt
 * 
 * 170   7/26/97 8:39a Mgummelt
 * 
 * 169   7/25/97 4:23p Mgummelt
 * 
 * 168   7/25/97 4:20p Mgummelt
 * 
 * 167   7/21/97 4:04p Mgummelt
 * 
 * 166   7/21/97 4:02p Mgummelt
 * 
 * 165   7/21/97 12:35p Mgummelt
 * 
 * 164   7/21/97 12:11p Mgummelt
 * 
 * 163   7/21/97 11:46a Mgummelt
 * 
 * 162   7/19/97 9:53p Mgummelt
 * 
 * 161   7/18/97 11:06a Mgummelt
 * 
 * 160   7/17/97 6:53p Mgummelt
 * 
 * 159   7/16/97 8:12p Mgummelt
 * 
 * 158   7/15/97 9:19p Mgummelt
 * 
 * 157   7/15/97 3:27p Mgummelt
 * 
 * 156   7/15/97 3:19p Mgummelt
 * 
 * 155   7/15/97 2:42p Mgummelt
 * 
 * 154   7/12/97 10:56a Rlove
 * 
 * 153   7/09/97 11:54a Mgummelt
 * 
 * 152   7/03/97 5:58p Mgummelt
 * 
 * 151   7/03/97 4:52p Mgummelt
 * 
 * 150   7/03/97 4:46p Mgummelt
 * 
 * 149   7/03/97 10:07a Rlove
 * 
 * 148   7/02/97 2:42p Rlove
 * 
 * 147   7/01/97 2:36p Rlove
 * Fixed solid_phase problem
 * 
 * 146   6/27/97 5:37p Mgummelt
 * 
 * 145   6/26/97 7:36a Rlove
 * Changed Vindictus to Ravenstaff
 * 
 * 144   6/24/97 5:44p Rlove
 * Rings of Flight and Regeneration are working
 * 
 * 143   6/24/97 7:48a Rlove
 * 
 * 142   6/23/97 3:14p Mgummelt
 * 
 * 141   6/20/97 9:43a Rlove
 * New mana system
 * 
 * 140   6/19/97 3:42p Mgummelt
 * 
 * 139   6/19/97 3:39p Mgummelt
 * 
 * 138   6/19/97 3:33p Rjohnson
 * Fix
 * 
 * 137   6/19/97 3:28p Rjohnson
 * Removed crandom()
 * 
 * 136   6/18/97 7:51p Mgummelt
 * 
 * 135   6/18/97 5:30p Mgummelt
 * 
 * 134   6/16/97 5:14p Rlove
 * 
 * 133   6/16/97 4:01p Rlove
 * 
 * 132   6/16/97 2:35p Rlove
 * 
 * 131   6/16/97 2:12p Mgummelt
 * 
 * 130   6/16/97 12:04p Rjohnson
 * Fixed a bug with cycling weapons
 * 
 * 129   6/13/97 10:51a Rlove
 * Moved some precache code
 * 
 * 128   6/12/97 8:55p Mgummelt
 * 
 * 127   6/12/97 12:13p Rlove
 * Archer arrows generate red or green sparks
 * 
 * 126   6/09/97 11:20a Rlove
 * 
 * 125   6/05/97 9:30a Rlove
 * Weapons now have deselect animations
 * 
 * 124   6/03/97 10:48p Mgummelt
 * 
 * 123   6/02/97 9:55a Rlove
 * Changed where firing is done
 * 
 * 121   5/31/97 9:22a Rlove
 * Newer, faster, better
 * 
 * 119   5/30/97 8:40p Rjohnson
 * Removed a message field
 * 
 * 118   5/30/97 12:01p Rlove
 * New blue explosion
 * 
 * 117   5/30/97 9:00a Rlove
 * New axe blade effect
 * 
 * 116   5/29/97 12:26p Mgummelt
 * 
 * 115   5/28/97 8:13p Mgummelt
 * 
 * 114   5/28/97 11:12a Rlove
 * 
 * 113   5/28/97 10:45a Rlove
 * Moved sprite effects to client side - smoke, explosions, and flashes.
 * 
 * 112   5/23/97 1:29p Rlove
 * 
 * 111   5/23/97 6:59a Rlove
 * Added new cheatmode field
 * 
 * 110   5/22/97 7:07p Mgummelt
 * 
 * 109   5/22/97 6:30p Mgummelt
 * 
 * 108   5/21/97 11:18a Mgummelt
 * 
 * 106   5/20/97 9:32p Mgummelt
 * 
 * 105   5/19/97 4:35p Rlove
 * 
 * 104   5/19/97 12:01p Rlove
 * New sprites for axe
 * 
 * 103   5/19/97 11:27a Mgummelt
 * 
 * 102   5/19/97 8:58a Rlove
 * Adding sprites and such to the axe.
 * 
 * 101   5/16/97 1:52p Rlove
 * 
 * 100   5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 99    5/15/97 5:05a Mgummelt
 * 
 * 98    5/15/97 12:30a Mgummelt
 * 
 * 97    5/13/97 8:36a Rlove
 * 
 * 96    5/12/97 1:42p Rlove
 * 
 * 95    5/12/97 10:31a Rlove
 * 
 * 94    5/11/97 7:30a Mgummelt
 * 
 * 93    5/09/97 2:46p Rlove
 * 
 * 92    5/07/97 3:40p Mgummelt
 * 
 * 91    5/07/97 11:03a Rlove
 * 
 * 90    5/05/97 10:09p Mgummelt
 * 
 * 89    5/05/97 5:40p Rlove
 * 
 * 88    5/05/97 10:29a Mgummelt
 * 
 * 87    5/03/97 8:49a Rlove
 * 
 * 86    5/02/97 8:33a Rlove
 * Weapon models change when you change classes
 * 
 * 85    5/02/97 8:05a Rlove
 * 
 * 84    5/01/97 8:52p Mgummelt
 * 
 * 83    5/01/97 10:22a Rlove
 * 
 * 82    4/30/97 5:02p Mgummelt
 * 
 * 81    4/28/97 6:55p Mgummelt
 * 
 * 80    4/26/97 6:23p Mgummelt
 * 
 * 79    4/24/97 8:49p Mgummelt
 * 
 * 78    4/24/97 3:51p Mgummelt
 * 
 * 77    4/24/97 2:22p Mgummelt
 * 
 * 76    4/21/97 8:48p Mgummelt
 * 
 * 75    4/21/97 6:15p Mgummelt
 * 
 * 74    4/21/97 5:20p Rlove
 * New mummy missile
 * 
 * 73    4/21/96 1:34p Mgummelt
 * 
 * 72    4/21/97 12:30p Mgummelt
 * 
 * 71    4/21/97 10:51a Rlove
 * Changed sprite explosion for phased missiles
 * 
 * 70    4/21/97 9:16a Rlove
 * Tried new axe animations
 * 
 * 69    4/18/97 2:24p Rlove
 * Changed vorpal sword over to new weapon code
 * 
 * 68    4/18/97 12:45p Mgummelt
 * 
 * 67    4/17/97 9:12p Mgummelt
 * 
 * 66    4/17/97 2:51p Mgummelt
 * 
 * 65    4/16/96 11:52p Mgummelt
 * 
 * 64    4/16/97 4:34p Mgummelt
 * 
 * 63    4/16/97 4:22p Mgummelt
 * 
 * 62    4/16/97 8:38a Rlove
 * Corrected axe loop when selected after purifier
 * 
 * 61    4/16/97 7:59a Rlove
 * Removed references to ammo_  fields
 * 
 * 60    4/15/97 10:14a Rlove
 * Changed cleric to crusader
 * 
 * 59    4/14/96 5:12p Mgummelt
 * 
 * 58    4/14/97 5:04p Rlove
 * 
 * 57    4/14/96 3:47p Mgummelt
 * 
 * 56    4/14/96 2:36p Mgummelt
 * 
 * 55    4/14/97 9:36a Rlove
 * self.th_missile and self..th_melee were stopping .nextthink for weapons
 * so I commented them out.
 * 
 * 54    4/13/97 4:09p Mgummelt
 * 
 * 53    4/13/96 4:05p Mgummelt
 * 
 * 52    4/13/96 3:31p Mgummelt
 * 
 * 51    4/12/96 8:56p Mgummelt
 * 
 * 50    4/12/97 3:25p Rlove
 * New stuff
 * 
 * 49    4/12/96 9:02a Mgummelt
 * 
 * 48    4/11/97 7:33p Mgummelt
 * 
 * 47    4/11/97 7:30p Mgummelt
 * 
 * 46    4/11/97 2:36p Rlove
 * Got rid of test weapon
 * 
 * 45    4/11/97 12:32a Mgummelt
 * 
 * 44    4/10/96 5:08p Mgummelt
 * 
 * 43    4/10/97 2:14p Rlove
 * Some tweaking of gauntlets and vorpal sword.
 * 
 * 42    4/09/97 2:49p Mgummelt
 * 
 * 41    4/09/97 2:44p Mgummelt
 * 
 * 40    4/09/97 2:40p Rlove
 * 
 * 39    4/09/97 1:21p Mgummelt
 * 
 * 38    4/09/97 1:13p Mgummelt
 * 
 * 37    4/09/97 12:17p Mgummelt
 * 
 * 36    4/09/97 11:11a Mgummelt
 * 
 * 35    4/09/97 11:09a Mgummelt
 * 
 * 34    4/09/97 11:06a Mgummelt
 * 
 * 33    4/08/97 5:29p Rlove
 * 
 * 32    4/05/97 5:55p Mgummelt
 * 
 * 31    4/04/97 9:36p Mgummelt
 * 
 * 30    4/04/97 5:40p Rlove
 * 
 * 29    4/01/97 2:44p Rlove
 * New weapons, vorpal sword and purifier
 * 
 * 28    3/31/97 4:18p Rlove
 * New punch dagger animations
 * 
 * 27    3/31/97 7:29a Rlove
 * Added damage to SpawnPuff
 * 
 * 26    3/31/97 6:38a Rlove
 * New punchdagger animations are operational
 * 
 * 25    3/21/97 9:38a Rlove
 * Created CHUNK.HC and MATH.HC, moved brush_die to chunk_death so others
 * can use it.
 * 
 * 24    3/20/97 4:01p Rlove
 * Added mummy, medusa for Brian R.
 * 
 * 23    3/19/97 7:44a Rlove
 * Doors no longer open when an axe blade is near
 * 
 * 22    3/18/97 5:09p Rlove
 * 
 * 21    3/17/97 3:01p Rlove
 * Added sprite explosion
 * 
 * 20    3/17/97 2:01p Rlove
 * Bounce missiles were sticking on the floor and that was a bad thing
 * 
 * 19    3/17/97 12:40p Rlove
 * New axe is in.
 * 
 * 18    3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 17    3/10/97 8:30a Rlove
 * Added the axe weapon
 * 
 * 16    3/04/97 8:06a Rlove
 * Added first frames of a couple new weapons
 * 
 * 15    2/28/97 3:33p Rlove
 * New crossbow and sickle weapon models
 * 
 * 14    2/27/97 2:54p Rlove
 * Added new gauntlet model
 * 
 * 13    2/12/97 3:57p Rjohnson
 * Added code to allow the insertion of a test weapon and to use it easily
 * 
 * 12    2/07/97 1:37p Rlove
 * Artifact of Invincibility
 * 
 * 11    1/29/97 7:50a Rlove
 * Adding crossbow weapon
 * 
 * 10    1/22/97 7:24a Rlove
 * The shotgun wouldn't stop firing, now it does
 * 
 * 9     12/30/96 8:31a Rlove
 * The javelin weapon
 * 
 * 8     12/18/96 8:56a Rlove
 * Shotgun works again
 * 
 * 7     12/16/96 3:50p Rlove
 * New gauntlet stuff
 * 
 * 6     12/16/96 12:42p Rlove
 * New gauntlets, artifacts, and inventory
 * 
 * 1     12/13/96 3:47p Rlove
 * 
 * 5     11/12/96 2:39p Rlove
 * Updates for the inventory system. Torch, HP boost and Super HP Boost.
 * 
 * 4     11/11/96 1:23p Rlove
 * Added Source Safe stuff
 */

