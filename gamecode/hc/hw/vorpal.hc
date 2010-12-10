/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/vorpal.hc,v 1.3 2007-03-18 08:11:07 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\vorpal\final\vorpal.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\vorpal\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

$frame SwdRoot      
//

// FRAME 2 - 23
$frame 2ndSwd1      2ndSwd2      2ndSwd3      2ndSwd4      2ndSwd5      
$frame 2ndSwd9      2ndSwd10     
$frame 2ndSwd11     2ndSwd13     2ndSwd14     2ndSwd15     
$frame 2ndSwd16     2ndSwd18     2ndSwd19     2ndSwd21     
$frame 2ndSwd22     2ndSwd23     2ndSwd24     2ndSwd25     2ndSwd26     
$frame 2ndSwd27     2ndSwd28
     
//
// FRAME 24 - 36
$frame 3rdSwd1      3rdSwd2            
$frame 3rdSwd9         
$frame 3rdSwd11     3rdSwd12     3rdSwd13     3rdSwd14     3rdSwd15     
$frame 3rdSwd16     3rdSwd17     
$frame 3rdSwd22     3rdSwd23     3rdSwd24     

// FRAME 113 - 131
$frame 6thSwd13     6thSwd14     6thSwd15     
$frame 6thSwd16     6thSwd17


float VORP_BASE_DAMAGE			= 15;
float VORP_ADD_DAMAGE			= 10;
float VORP_PWR_BASE_DAMAGE		= 50;
float VORP_PWR_ADD_DAMAGE		= 30;
float VORP_RADIUS				= 150;

float VORP_PUSH					= 5;

/*
============
vorpmissile_touch - vorpmissile hit something. Death to the infidel!
============
*/
void vorpmissile_touch (void)
{
	float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		stopSound(self,0);
		remove(self);
		return;
	}

	damg = random(20,40);

	if (self.classname == "halfvorpmissile")
	{
		damg = damg * .5;
	}

	if (other.health)
		T_Damage (other, self, self.owner, damg );

	T_RadiusDamage (self, self.owner, 80.0, self.owner);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_SWORD_EXPLOSION);
	WriteCoord(MSG_MULTICAST, self.origin_x - self.movedir_x * -8);
	WriteCoord(MSG_MULTICAST, self.origin_y - self.movedir_y * -8);
	WriteCoord(MSG_MULTICAST, self.origin_z - self.movedir_z * -8);
	WriteEntity(MSG_MULTICAST, self.owner);
	multicast(self.origin,MULTICAST_PHS_R);

	stopSound(self,0);
	remove(self);
}

void vorpMissileThink(void)
{
	self.movedir = normalize(self.velocity);

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.movedir * 360.0, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_SWORD_SHOT);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);

	thinktime self : 0.3;

	if (self.lifetime < time)
		SUB_Remove();
}


/*
============
launch_vorpal_missile - create and launch vorpal missile
============
*/
void launch_vorpal_missile(void)
{
	entity missile;

	missile = spawn ();

	CreateEntityNew(missile,ENT_VORP_MISSILE,"models/vorpshot.mdl",SUB_Null);

	missile.owner = self;
	missile.classname = "vorpalmissile";
			
	// set missile speed	
	makevectors (self.v_angle);
	missile.velocity = normalize(v_forward);
	missile.velocity = missile.velocity * 1200;
	missile.movedir = normalize(missile.velocity);
	
	missile.frags=TRUE;
	missile.touch = vorpmissile_touch;
	missile.angles = vectoangles(missile.velocity);
	missile.angles_x += 180;
	//missile.drawflags=MLS_ABSLIGHT;
	//missile.abslight=0.5;

	//missile.effects (+) EF_SWORD_EFFECT;
	missile.effects (+) EF_NODRAW;

//	setorigin (missile, self.origin + v_up * 40);
	setorigin (missile, self.origin + self.proj_ofs);
//	missile.scale = .5;
	missile.scale = 1;

	if (self.bluemana < 4)
		missile.classname = "halfvorpmissile";

//	thinktime missile : HX_FRAME_TIME;

	missile.think = vorpMissileThink;
	missile.lifetime = time + 2;
	thinktime missile : .3;

	entity oldself;
	oldself = self;
	self = missile;

	missile.think();

	self = oldself;
}

/*
============
Smite that which is directly in front of you
============
*/
void vorpal_melee (void)
{
	vector	source;
	vector	org,dir;
	float damg;
	float no_flash,inertia;

	makevectors (self.v_angle);
	source = self.origin+self.proj_ofs;
	traceline (source, source + v_forward*64, FALSE, self);  // Straight in front

	self.enemy = world;

	if (trace_fraction == 1.0)	// Anything right in front ?
	{
		traceline (source, source + v_forward*88 - (v_up * 30), FALSE, self);  // 30 down
	
		if (trace_fraction == 1.0)  
		{
			traceline (source, source + v_forward*88 + v_up * 30, FALSE, self);  // 30 up
		
			if (trace_fraction == 1.0)  
				return;
		}
	}
	
	org = trace_endpos + (v_forward * 2);

	if (trace_ent.takedamage) 	// It can be hurt
	{
		SpawnPuff (org, '0 0 0', 20,trace_ent);
		self.enemy = trace_ent;

		if(!trace_ent.mass)
			inertia=1;
		else if(trace_ent.mass<=5)
			inertia=trace_ent.mass;
		else
			inertia=trace_ent.mass/10;

		no_flash = 0;
		if ((self.bluemana >= 4) && (self.artifact_active & ART_TOMEOFPOWER)) // Tome of power melee damage
		{
			damg = 40 + random(30);
			damg += damg * .25;
		}
		else if (self.bluemana >= 2)
		{
			damg = 30 + random(20);
			if (trace_ent.flags & FL_MONSTER)  // Only monsters make it use mana
				self.bluemana -= 2;
		}
		else 
		{
			no_flash =1;
			damg = 20 + random(10);
		}

		org = trace_endpos + (v_forward * -1);

		if (!no_flash)
			CreateLittleWhiteFlash(org);

		if (!MetalHitSound(trace_ent.thingtype))
			sound (self, CHAN_WEAPON, "weapons/vorpht1.wav", 1, ATTN_NORM);

		dir =  trace_ent.origin - self.origin;
		if(trace_ent.solid!=SOLID_BSP&&trace_ent.movetype!=MOVETYPE_PUSH)
		{
			trace_ent.velocity = dir * VORP_PUSH*(1/inertia);
			if(trace_ent.movetype==MOVETYPE_FLY)
			{
				if(trace_ent.flags&FL_ONGROUND)
					trace_ent.velocity_z=80/inertia;
			}
			else
				trace_ent.velocity_z = 80/inertia;
			trace_ent.flags (-) FL_ONGROUND;
		}
		T_Damage (trace_ent, self, self, damg);
	}
	else	// hit wall
	{
		sound (self, CHAN_WEAPON, "weapons/vorpht2.wav", 1, ATTN_NORM);
		org = trace_endpos + (v_forward * -1);
		org += '0 0 10';
		CreateWhiteSmoke(org,'0 0 2',HX_FRAME_TIME);
	}
}

/*
============
Deflect missiles
============
*/
/*
void vorpal_downmissile (void)
{
	vector  source,dir;
	entity  victim;
	float chance;
	entity hold;

	if (!self.artifact_active & ART_TOMEOFPOWER)
		return;

	victim = findradius( self.origin, 150);
	while(victim)
	{
		if ((victim.movetype == MOVETYPE_FLYMISSILE) && (victim.owner != self))
		{
			victim.owner = self;
			chance = random();
			dir = victim.origin + (v_forward * -1);
			CreateLittleWhiteFlash(dir);
			sound (self, CHAN_WEAPON, "weapons/vorpturn.wav", 1, ATTN_NORM);
			if (chance < 0.9)  // Deflect it
			{
				victim.v_angle = self.v_angle + randomv('-180 -180 -180', '180 180 180'); 

				makevectors (victim.v_angle);
				victim.velocity = v_forward * 1000;
			}
			else  // reflect missile
				victim.velocity = '0 0 0' - victim.velocity;
		}
		victim = victim.chain;
	}
}
*/

/*
============
Fire Vorpal sword in normal mode
============
*/
void vorpal_normal_fire (void)
{
	entity  victim;
	float damg,damage_flg;

	vorpal_melee ();

	if (self.bluemana<2)   // Not enough mana to fire it
		return;

	damage_flg = 0;
	victim = findradius( self.origin, 100);
	while(victim)
	{	// Enemy would be the one that took direct melee damage so don't hurt him twice
		if ((victim.health) && (victim!=self) && (victim!=self.enemy))
		{
			traceline (self.origin + self.owner.view_ofs, victim.origin, FALSE, self);  // 30 up

			if (trace_ent == victim)
			{
				damage_flg = 1;

				//sound (self, CHAN_WEAPON, "weapons/vorpblst.wav", 1, ATTN_NORM);
				weapon_sound(self, "weapons/vorpblst.wav");

				CreateWhiteSmoke(victim.origin + '0 0 30','0 0 8',HX_FRAME_TIME);

				damg = VORP_BASE_DAMAGE + random(VORP_ADD_DAMAGE);
				T_Damage (victim, self, self, damg);
			}
		}

		victim = victim.chain;
	}

	if (trace_ent.flags & FL_MONSTER)  // Only monsters make it use mana
		self.bluemana -= 2;

}

/*
============
Fire Vorpal sword in Power Up mode
============
*/
void vorpal_tome_fire (void)
{
	vorpal_melee ();
	
	if (self.bluemana>=4)
	{
		launch_vorpal_missile();
		self.bluemana -=4;
	}
}

/*
============
Decide if vorpal sword is in Normal or Powerup mode
============
*/
void vorpal_fire (void)
{
	if (self.artifact_active & ART_TOMEOFPOWER)
		vorpal_tome_fire();
	else
		vorpal_normal_fire();
}

/*
============
vorpal_ready - vorpal sword is in ready position
============
*/
void vorpal_ready (void) 
{
	self.weaponframe = $SwdRoot;
	self.th_weapon=vorpal_ready;
}

/*
============
vorpal_twitch - vorpal sword twitches 
============
*/
void vorpal_twitch (void)
{
	self.wfs = advanceweaponframe($2ndSwd1,$2ndSwd28);
	self.th_weapon = vorpal_twitch;

	if (self.weaponframe == $2ndSwd11)	// Frame 48
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");
	else if (self.weaponframe == $2ndSwd18)	// Frame 55
		//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		weapon_sound(self, "weapons/vorpswng.wav");

	if (self.wfs == WF_LAST_FRAME)
		vorpal_ready();
}

// Rotate model around player
void moveswipe(void) [++0 .. 6]
{ 
	vector org;

	makevectors (self.owner.v_angle);
	org = self.owner.origin + self.owner.view_ofs - '0 0 25';
	setorigin (self, org);
	self.angles = self.owner.v_angle;
	self.angles_y += 180;

	self.think = moveswipe;
	thinktime self : HX_FRAME_TIME;

	if (self.frame>5)		
		self.think = SUB_Remove;
}


void SpawnSwipe(void)
{
	entity swipe;
	vector org;

	swipe = spawn ();

	CreateEntityNew(swipe,ENT_SWIPE,"models/vorpswip.mdl",SUB_Null);

	makevectors (self.v_angle);
	swipe.angles = self.v_angle;
	swipe.angles_y += 180;

	org = self.origin + self.view_ofs - '0 0 25';
	setorigin (swipe, org);
	
	swipe.counter =0;
	swipe.owner = self;

	swipe.velocity = '0 0 0';
	swipe.touch = SUB_Null;
	swipe.drawflags(+)DRF_TRANSLUCENT;

	swipe.think = moveswipe;
	thinktime swipe : HX_FRAME_TIME;

}

void vorpal_a (void)
{
	self.wfs = advanceweaponframe($3rdSwd1,$3rdSwd24);
	self.th_weapon = vorpal_a;

	if (self.weaponframe == $3rdSwd2)	// Frame 80
	{
		if (self.artifact_active & ART_TOMEOFPOWER)
			//sound (self, CHAN_WEAPON, "weapons/vorppwr.wav", 1, ATTN_NORM);
			weapon_sound(self, "weapons/vorppwr.wav");
		else
			//sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
			weapon_sound(self, "weapons/vorpswng.wav");
		
	}
	else if (self.weaponframe == $3rdSwd9)	// Frame 84
		vorpal_fire ();

	if ((self.weaponframe == $3rdSwd9) && (self.bluemana>=2))
		SpawnSwipe();

	if (self.wfs == WF_LAST_FRAME)
		self.th_weapon=vorpal_ready;
}


/*
============
vorpal_select - vorpal sword was just chosen
============
*/
void vorpal_select (void)
{
	self.wfs = advanceweaponframe($2ndSwd5,$2ndSwd1);

	self.weaponmodel = "models/vorpal.mdl";
	self.th_weapon=vorpal_select;
	self.last_attack=time;

	if (self.wfs == WF_LAST_FRAME)
	{
		self.attack_finished = time - 1;
		vorpal_twitch();
	}
}

/*
============
vorpal_deselect - vorpal sword was just un-chosen
============
*/
void vorpal_deselect (void)
{
	self.wfs = advanceweaponframe($6thSwd13,$6thSwd17);
	self.th_weapon=vorpal_deselect;
	self.oldweapon = IT_WEAPON2;

	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();

}

void pal_vorpal_fire(void)
{
	vorpal_a ();

	self.attack_finished = time + 0.3;
}

