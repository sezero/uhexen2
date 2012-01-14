/*
 * hw/weapons.hc
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

void BecomeExplosion (float explodetype)
{
	if (explodetype)
	{
		if(explodetype==CE_FLOOR_EXPLOSION)
			starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 64');
		else
			starteffect(explodetype , self.origin);
	}
	else
	{
		if(self.classname=="acidblob")
			starteffect(CE_ACID_EXPL,self.origin - self.movedir*8,'0 0 0',HX_FRAME_TIME*2);
		else if (self.flags2&FL_SMALL)
			starteffect(CE_SM_EXPLOSION , self.origin);
		else if(self.flags&FL_ONGROUND)
			starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 64');
		else
			starteffect(CE_LG_EXPLOSION , self.origin);
	}

	if(self.classname=="multigrenade")
	{//Let sounds play here
		self.effects=EF_NODRAW;
		self.velocity='0 0 0';
		self.movetype=MOVETYPE_NONE;
		self.think=SUB_Remove;
		thinktime self : 3;
	}
	else
		remove(self);
}

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
				if(self.bluemana >= 4 && self.greenmana >= 4)
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
			if(self.artifact_active&ART_TOMEOFPOWER)
			{
				if(self.greenmana >= 35 && self.bluemana >= 35)
					return TRUE;
				self.effects(-)EF_BRIGHTLIGHT;
			}
			else if(self.bluemana >= 2 && self.greenmana >= 2)
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

/*float	test_weapon;

	test_weapon = IT_WEAPON4;
	while((!W_CheckNoAmmo (test_weapon)||(!self.items&test_weapon)))
	{
		if(test_weapon==IT_WEAPON2)
			test_weapon=IT_WEAPON1;
		else
			test_weapon -= 1;
	}
	self.weapon=test_weapon; */


/*
	test_weapon = IT_WEAPON4;
	while((!W_CheckNoAmmo (test_weapon)||(!self.items&test_weapon)))
	{
		if(test_weapon==IT_WEAPON2)
			test_weapon=IT_WEAPON1;
		else
			test_weapon/=2;
	}
	self.weapon=test_weapon; */
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
		sprinti (self, PRINT_MEDIUM,STR_NOCARRYWEAPON);
		return;
	}

	if(!W_CheckNoAmmo(fl))
	{
		sprinti (self, PRINT_MEDIUM,STR_NOTENOUGHMANA);
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

/*
	if(self.attack_finished<time)
	{
		self.oldweapon = self.weapon;
		self.weapon = IT_WEAPON4;
		W_SetCurrentWeapon ();
	}
*/
};

/*
============
CycleWeaponCommand

Go to the next weapon with ammo
============
*/
void() CycleWeaponCommand =
{
	float	am,fl;
	if(self.attack_finished>time)
		return;
	self.impulse = 0;
	self.items (+) IT_WEAPON1;
	am = 1;
	fl = self.weapon;
	while (am)
	{
		switch (fl)
		{
		case IT_WEAPON1:
			fl = IT_WEAPON2;
		break;
		case IT_WEAPON2:
			fl = IT_WEAPON3;
		break;
		case IT_WEAPON3:
			fl = IT_WEAPON4;
		break;
		case IT_WEAPON4:
			fl = IT_WEAPON1;
		break;
		}
		if ((self.items & fl) && ((W_CheckNoAmmo(fl))||((self.playerclass == CLASS_PALADIN)&&(fl==IT_WEAPON2))))
		{
			am = 0;
		}
	}
	self.weapon = fl;

	W_SetCurrentWeapon ();
	return;
};

/*
============
CycleWeaponReverseCommand

Go to the prev weapon with ammo
============
*/
void() CycleWeaponReverseCommand =
{
	float	am,fl;
	if(self.attack_finished>time)
		return;
	self.impulse = 0;
	self.items (+) IT_WEAPON1;
	am = 1;
	fl = self.weapon;
	while (am)
	{
		switch (fl)
		{
		case IT_WEAPON1:
			fl = IT_WEAPON4;
		break;
		case IT_WEAPON2:
			fl = IT_WEAPON1;
		break;
		case IT_WEAPON3:
			fl = IT_WEAPON2;
		break;
		case IT_WEAPON4:
			fl = IT_WEAPON3;
		break;
		}
		if ((self.items & fl) && ((W_CheckNoAmmo(fl))||((self.playerclass == CLASS_PALADIN)&&(fl==IT_WEAPON2))))
		{
			am = 0;
		}
	}
	self.weapon = fl;
	W_SetCurrentWeapon ();
	return;
};

/*
============
ServerflagsCommand

Just for development
============
*/
/*
void() ServerflagsCommand =
{
	serverflags = serverflags * 2 + 1;
};
*/


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
		if(tomeMode == 1)
		{
			if(self.artifact_active & ART_TOMEOFPOWER)
			{
				self.artifact_active(-)ART_TOMEOFPOWER;
				self.tome_time = 0;
			}
			else
			{
				self.artifact_active(+)ART_TOMEOFPOWER;
				self.tome_time = time + 1200000; // arbitrary fairly large number
			}
		}
		if(tomeMode == 2)
		{
			if(self.poweredFlags & self.weapon)
			{
				if(self.artifact_active & ART_TOMEOFPOWER)
				{
					self.artifact_active(-)ART_TOMEOFPOWER;
					self.tome_time = 0;
				}
				else
				{
					self.artifact_active(+)ART_TOMEOFPOWER;
					self.tome_time = time + 1200000; // arbitrary fairly large number
				}				
			}
		}
		return;
	}

	if(tomeMode == 2)
	{
		self.artifact_active(-)ART_TOMEOFPOWER;
		self.tome_time = 0;
	}

	self.attack_finished=time + 999;
	W_DeselectWeapon ();
}

