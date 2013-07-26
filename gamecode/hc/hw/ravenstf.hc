/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/ravenstf.hc,v 1.2 2007-02-07 16:58:01 sezero Exp $
 */

// For building the model
$cd Q:\art\models\weapons\newass
$origin 0 0 0
$base BASE SKIN
$skin SKIN
$flags 0

//
$frame rootpose     

//
$frame fire1        fire2        fire3        fire4        

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     

float ravenshot_speed = 1200;

void ravenstaff_fire (void);
void ravenstaff_idle (void);
void split (void);
void raven_track(void);
void raven_flap(void);
void raven_touch (void);
void raven_track_init(void);
void ravenmissile_explode(void);

void raven_spark (void)
{
	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_HWRAVENDIE);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z - 10);
	multicast(self.origin,MULTICAST_PHS_R);
/*
	CreateWhiteSmoke(self.origin + '0 0 -10', '0 8 -10', HX_FRAME_TIME *3);
	CreateRedSmoke(self.origin + '0 0 -10', '0 0 -10', HX_FRAME_TIME *3);
	CreateWhiteSmoke(self.origin + '0 0 -10', '0 -8 -10', HX_FRAME_TIME *3);
	sound(self,CHAN_WEAPON,"raven/death.wav",1,ATTN_NORM);

	self.touch = SUB_Null;
	self.effects=EF_NODRAW;
	self.think=SUB_Remove;
	thinktime self : HX_FRAME_TIME;
//	thinktime self : HX_FRAME_TIME * 2;
*/
	remove(self);
}

void raven_death_init (void)
{
	self.raven_owner.raven_cnt -= 1;
//	dprint("Dec class: ");
//	dprint(self.raven_owner.classname);
//	dprint("\n");

	self.takedamage = DAMAGE_NO;

	traceline(self.origin,self.origin + '0 0 600',FALSE,self);
	if (trace_fraction < 1)
	{
		self.touch = raven_spark;
		self.nextthink = 0;
	}
	else
	{
		self.touch = raven_spark;
		self.think = raven_spark;
		thinktime self : 1;
	}

	self.velocity = normalize('0 0 600');
	self.velocity = self.velocity * 400;
	self.angles = vectoangles(self.velocity);
}

void raven_bounce(void)
{
	self.flags (-) FL_ONGROUND;

	self.angles = vectoangles(self.velocity);	// Flip it around to match the velocity set by the BOUNCEMISSLE code
	self.angles_y += random(-90,90);			// Change it's yaw a little
	self.angles_x = random(-20,20);			// Change it's pitch a little 

	makevectors (self.angles);
	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * 600;

	self.think = raven_flap;
	self.nextthink = time + HX_FRAME_TIME;

	self.think1 = raven_track_init;
	self.next_action = time + HX_FRAME_TIME * random(2,4);

	self.touch = raven_touch;
}

// Bite the enemy
void raven_touch (void)
{
	if ((other == self.enemy) && (other.takedamage != DAMAGE_NO))
	{
		if (other.monsterclass >= CLASS_BOSS)	// Bosses only take half damage
			T_Damage(other,self,self.owner,20);
		else
			T_Damage(other,self,self.owner,40);

		if(other.flags & FL_CLIENT)
		{
			if(random() < .5)
			{//lights out!
				stuffcmd(other, "df\n");
			}
		}
		self.damage_max += 40;
		SpawnPuff (self.origin, '0 0 -5', random(5,10),other);
		MeatChunks (self.origin,self.velocity*0.5+'0 0 20', 2,other);
//		weapon_sound(self, "weapons/gauntht1.wav");	//sigh, no weapon_sound for packed items
		sound(self,CHAN_WEAPON,"weapons/gauntht1.wav",1,ATTN_NORM);
	}

	if (self.damage_max > 250)
		raven_death_init();
	else
	{
		self.touch = SUB_Null;
		self.think = raven_bounce;
		self.nextthink = time + .05;  // Need to wait a little before flipping model to match velocity
	}

	if ((self.lifetime < time) || (self.raven_owner.raven_cnt > 6))
	{
		raven_death_init();
		return;
	}
}

//  Search for an enemy
void raven_search(void)
{
	entity victim;

	self.nextthink = time + HX_FRAME_TIME;	//	Gotta flap
	self.think = raven_flap;

	victim = findradius( self.origin,1000);
	while(victim)
	{	// the controller check is for the summoned imp
		if (((victim.flags & FL_MONSTER) || (victim.flags & FL_CLIENT)) && 
			(victim.owner != self) && (victim.controller != self.owner) && (victim.health>0) && (victim!=self.owner))
		{
			if (coop && self.enemy.team == self.team) 
				victim = victim;		// Do nothing if its a player on your team.
			else
			{
				traceline(self.origin,victim.origin,TRUE,self);
				if (trace_fraction == 1.0)  
				{
					self.enemy = victim;
					self.think1 = raven_track;
					self.think1 = raven_track_init;
					self.next_action = time + .1;
					self.searchtime = 0;
					return;
				}
			}
		}
		victim = victim.chain;
	}

	self.think1 = raven_search;
	self.next_action = time + HX_FRAME_TIME * 3;

	if (self.searchtime == 0)  // Done only on birth of raven
	{
		self.searchtime = time + .5;

		self.angles_y = random(0, 360);
		self.angles_x = 0;

		makevectors (self.angles);
		self.velocity = normalize (v_forward);
		self.velocity = self.velocity * 600;
		self.last_vel = self.velocity;
	}

	if ((self.searchtime < time) || (self.lifetime < time) || (self.raven_owner.raven_cnt > 6))
		raven_death_init();
}


//
// Chase after the enemy
//
void raven_track (void)
{
	vector delta;
	vector hold_spot;

//	dprint("\n  trk:");
//	dprint(self.enemy.classname);

	// The FL_MONSTER flag gets flipped when it becomes a head
	if ((self.enemy.health <= 0) || (self.enemy == world) || (!self.enemy.flags & FL_MONSTER))
		raven_search();
	else
	{
		traceline(self.origin,self.enemy.origin,TRUE,self);
		if (trace_fraction == 1)
		{
			hold_spot = self.enemy.origin;
			hold_spot_z += self.enemy.maxs_z;  // Hit 'em on the head
			delta = hold_spot - self.origin;

			self.velocity = normalize(delta);
			self.velocity = self.velocity * 600;
			self.angles = vectoangles(self.velocity);
//			raven_check_vel();
			self.think1 = raven_track;
			self.next_action = time + HX_FRAME_TIME * 3;

			self.think = raven_flap;
			self.nextthink = time + HX_FRAME_TIME;
		}
		else
			raven_search();
	}

	if ((self.lifetime < time) || (self.raven_owner.raven_cnt > 6))
	{
		raven_death_init();
		return;
	}
}

void raven_track_init (void)
{
	vector delta;
	vector hold_spot;

	if ((self.enemy.health <= 0) || (self.enemy == world))
		raven_search();
	else
	{
		hold_spot = self.enemy.origin;
		hold_spot_z += self.enemy.maxs_z;
		delta = hold_spot - self.origin;
		self.velocity = normalize(delta);
		self.angles = vectoangles(self.velocity);

		self.idealpitch = self.angles_x;

		makevectors(self.angles);
		self.velocity = normalize(v_forward);
		self.velocity = self.velocity * 600;
		self.pitchdowntime = time + HX_FRAME_TIME *3;

		self.think = raven_track;
		self.nextthink = time;
	}
}

// Everything comes back to here
void raven_flap(void)
{
//	raven_check_vel();
	AdvanceFrame(0,7);

	if ((self.frame == 1) && (random() < .2))
	{
//		weapon_sound(self, "raven/squawk2.wav");	//doesn't work with packing!
		sound(self,CHAN_VOICE,"raven/squawk2.wav",1,ATTN_NORM);
	}

	if (self.next_action < time)
	{
		self.think = self.think1;
		self.nextthink = time;
	}
	else
	{
//		ChangeYaw();
		self.think = raven_flap;
		self.nextthink = time + HX_FRAME_TIME;
	}

	if ((self.lifetime < time) || (self.raven_owner.raven_cnt > 6))
	{
		raven_death_init();
		return;
	}
}


/*--------------------
Create one raven
----------------------*/
void create_raven(void)
{
	entity missile;
	vector spot1, spot2;

	missile = spawn ();
	missile.frags=TRUE;
	missile.owner = self.owner;

	missile.raven_owner = self.raven_owner;	//used to decrement count correctly

	missile.movetype = MOVETYPE_BOUNCEMISSILE;
	missile.solid = SOLID_BBOX;
	missile.takedamage = DAMAGE_NO;

	// set missile speed
	makevectors (self.v_angle);
	missile.velocity = normalize (v_forward);
	missile.velocity = missile.velocity * 600;
	missile.angles = vectoangles(missile.velocity);
	missile.searchtime = 0;
	missile.yaw_speed = 50;

	setmodel (missile, "models/ravproj.mdl");
	setsize (missile, '-8 -8 8', '8 8 8');

	// had to remove the offset on origin
	setorigin (missile, self.origin + self.proj_ofs - v_forward * 14);

	missile.touch = raven_touch;
	missile.lifetime = time + 4 + random();
	missile.classname = "bird_missile";
//	sound(missile,CHAN_VOICE,"raven/ravengo.wav",1,ATTN_NORM);

	// Find an enemy
	makevectors(self.v_angle);
	spot1 = self.origin + self.proj_ofs;
	spot2 = spot1 + (v_forward*600); // Look ahead
	traceline(spot1,spot2,FALSE,self);
	missile.th_die=raven_death_init;

	// We have a victim in sights
	if ((trace_ent!=world) && 
		(trace_ent.flags & FL_MONSTER) && (trace_ent.owner != self) && (trace_ent.health>0))
	{
		missile.enemy = trace_ent;

		missile.nextthink = time + HX_FRAME_TIME;
		missile.think = raven_flap;

		missile.next_action = time + .01;
		missile.think1 = raven_track;
		missile.think1 = raven_track_init;
	}
	else
	{
		missile.nextthink = time + .01;
		missile.think = raven_search;
	}
}

void ravenmissile_explode (void)
{
local vector vel;

	makevectors (self.v_angle);
	vel = normalize (v_forward);
	vel = vel * 600;

	create_raven();
	create_raven();
	create_raven();

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_HWRAVENEXPLODE);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	multicast(self.origin,MULTICAST_PHS_R);
//	sound(self,CHAN_VOICE,"raven/ravengo.wav",1,ATTN_NORM);

/*
	CreateWhiteSmoke(self.origin + '0 0  0','0 0 8',HX_FRAME_TIME * 3);
	CreateWhiteSmoke(self.origin + '0 0  5','0 0 8',HX_FRAME_TIME * 3);
	CreateWhiteSmoke(self.origin + '0 0 10','0 0 8',HX_FRAME_TIME * 3);
*/
	remove(self);
}

void ravenmissile_touch (void)
{
	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if (other.health)
	{
//		sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
		starteffect(CE_SM_EXPLOSION2 , self.origin);
		self.enemy = other;
		T_Damage(other,self,self,10);
	}
	ravenmissile_explode();
}

void ravenmissile_puff (void)
{
	makevectors(self.angles);

	if (self.lifetime < time)
		ravenmissile_explode();
	else
	{
		thinktime self : HX_FRAME_TIME * 3;
		self.think = ravenmissile_puff;
	}
}


/*--------------------
Launch all ravens
----------------------*/
void launch_superraven (void)
{
	entity newmis;

	self.attack_finished = time + 1.0;
	self.raven_cnt += 3;	//considered from time of missile creation

	makevectors(self.v_angle);

	newmis = spawn();
	setmodel (newmis, "models/birdmsl2.mdl");
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
	newmis.takedamage = DAMAGE_NO;
	newmis.owner = self;
	newmis.raven_owner = self;
	setsize (newmis, '0 0 0', '0 0 0');

	newmis.velocity = normalize (v_forward);
	newmis.velocity = newmis.velocity * 600;
	newmis.angles = vectoangles(newmis.velocity);
	setorigin(newmis, self.origin + self.proj_ofs  + v_forward*10);

	newmis.touch = ravenmissile_touch;
	newmis.lifetime = time + .5;
	newmis.avelocity_z = 1000;
	newmis.scale = .40;
	thinktime newmis : HX_FRAME_TIME * 3;
	newmis.think = ravenmissile_puff;

	self.punchangle_x= random(-3);
	//TODO: kick
}


void ravenshot_touch (void)
{
	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	T_Damage (other, self, self.owner, self.dmg );

//	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	starteffect(CE_SM_EXPLOSION2 , self.origin);

	remove(self);
}

void create_raven_shot2(vector location,float add_yaw,float nexttime,float rotate,void() nextfunc)
{
	entity missile;
	vector holdangle;

	missile = spawn ();
	missile.owner = self.owner;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.solid = DAMAGE_YES;
	missile.dmg = 50;

// set missile speed
	missile.angles = self.angles;

	holdangle = self.angles;
//	holdangle_z = 0;
	holdangle_x = 0 - holdangle_x;
	holdangle_y += add_yaw;
	makevectors (holdangle);
	missile.velocity = normalize (v_forward);
	missile.velocity = missile.velocity * ravenshot_speed;

	if (rotate)
		missile.avelocity_z = 1000;
	else
		missile.avelocity_z = -1000;

	missile.touch = ravenshot_touch;

	setmodel (missile, "models/vindsht1.mdl");
	setsize (missile, '0 0 0', '0 0 0');
	setorigin (missile, location);

	missile.classname = "set_missile";

	thinktime missile : nexttime;
	missile.think = nextfunc;
}

void create_raven_shot1(vector location,float nexttime,void() nextfunc,vector fire_angle)
{
	entity missile;

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;

// set missile speed
	makevectors (fire_angle);
	missile.velocity = normalize (v_forward);
	missile.velocity = missile.velocity * ravenshot_speed;

	missile.avelocity_z = 1000; 

	missile.angles = vectoangles(missile.velocity);
	missile.dmg = 90;

	missile.touch = ravenshot_touch;

	setmodel (missile, "models/vindsht1.mdl");
	setsize (missile, '0 0 0', '0 0 0');
	setorigin (missile, location);

	missile.classname = "set_missile";

	thinktime missile : nexttime;

	missile.think = nextfunc;
}

void missle_straight(void)
{
	vector holdangles;

	holdangles = self.angles;
//	holdangles_z = 0;
	holdangles_x = 0 - holdangles_x;
	makevectors (holdangles);

	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * ravenshot_speed;

	return;
}

void missle_straight1(void)
{
	vector holdangles;

	holdangles = self.angles;
//	holdangles_z = 0;
	holdangles_x = 0 - holdangles_x;
	makevectors (holdangles);

	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * ravenshot_speed;

	create_raven_shot2(self.origin,-6,.2,1,missle_straight);
	self.dmg = 50;

	starteffect(CE_HWSPLITFLASH, self.origin);
	self.think = missle_straight;
	thinktime self : .05;
}

void missle_straight2(void)
{
	vector holdangles;

	holdangles = self.angles;
//	holdangles_z = 0;
	holdangles_x = 0 - holdangles_x;
	makevectors (holdangles);

	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * ravenshot_speed;

	create_raven_shot2(self.origin,6,.2,1,missle_straight);
	self.dmg = 50;

	starteffect(CE_HWSPLITFLASH, self.origin);
	self.think = missle_straight;
	thinktime self : .05;
}


void split (void)
{
	vector holdangles;

	// RIGHT SIDE
	create_raven_shot2(self.origin,-5,.30,0,missle_straight1);

	// LEFT SIDE
	create_raven_shot2(self.origin,5,.30,0,missle_straight2);

	starteffect(CE_HWSPLITFLASH, self.origin);

	self.dmg = 50;
	holdangles = self.angles;
//	holdangles_z = 0;
	holdangles_x = 0 - holdangles_x;
	makevectors (holdangles);

	self.velocity = normalize (v_forward);
	self.velocity = self.velocity * ravenshot_speed;
}

void launch_set (vector dir_mod)
{
	self.attack_finished = time + 1.0;

	create_raven_shot1(self.origin + self.proj_ofs + v_forward*14,0.05,split,self.v_angle);
}


void ravenstaff_power (void)
{
	self.wfs=advanceweaponframe($fire1,$fire4);
	self.th_weapon=ravenstaff_power;

	if (self.weaponframe==$fire1)
	{
		self.punchangle_x = -4;
		launch_superraven();
		self.greenmana -= 16;
		self.bluemana -= 16;
	}
	else if(self.weaponframe == $fire4)
	{
		self.weaponframe = $fire4;
		self.th_weapon=ravenstaff_idle;
	}

	thinktime self : HX_FRAME_TIME;
}

void ravenstaff_normal (void)
{
	self.wfs=advanceweaponframe($fire1,$fire4);
	self.th_weapon=ravenstaff_normal;

	if(self.weaponframe==$fire3)
	{
		self.punchangle_x = -4;
		launch_set('0 0 0');
		self.greenmana -= 8;
		self.bluemana -= 8;
	}

	else if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.weaponframe = $rootpose;
		self.th_weapon=ravenstaff_idle;
	}

	thinktime self : HX_FRAME_TIME;
}


void ravenstaff_fire (void)
{
	vector holdvelocity;

	if ((self.artifact_active & ART_TOMEOFPOWER) &&
		(self.greenmana >= 16) && (self.bluemana >= 16))
	{
//		dprintf("Raven count %s\n", self.raven_cnt);
		if(self.raven_cnt<3)	//this way it won't waste so much mana
		{
			weapon_sound(self, "raven/rfire2.wav");
//			sound (self, CHAN_WEAPON, "raven/rfire2.wav", 1, ATTN_NORM);
			stuffcmd (self, "bf\n");
			ravenstaff_power();
		}
		else
		{
			self.attack_finished = time + 0.5;
		}
	}
	else if ((self.greenmana >= 8) && (self.bluemana >= 8))
	{
		stuffcmd (self, "bf\n");
		makevectors(self.v_angle);
		holdvelocity = normalize(v_right);
		holdvelocity = holdvelocity * 10;
		starteffect(CE_TELESMK1, self.origin + self.proj_ofs  + v_forward * 14,holdvelocity,HX_FRAME_TIME * 3);
//		starteffect(CE_TELESMK1, self.origin + self.proj_ofs  + v_forward * 14,holdvelocity * -1,HX_FRAME_TIME * 3);
//		sound (self, CHAN_WEAPON, "raven/rfire1.wav", 1, ATTN_NORM);
		ravenstaff_normal();
	}
}

/*
============
ravenstaff_ready - just sit there until fired
============
*/
void ravenstaff_idle (void)
{
	self.weaponframe= $rootpose;
	self.th_weapon=ravenstaff_idle;
}

void ravenstaff_select (void)
{
	self.wfs=advanceweaponframe($select1,$select12);
	self.weaponmodel=("models/ravenstf.mdl");
	self.th_weapon=ravenstaff_select;
	if(self.weaponframe==$select12)
	{
		self.attack_finished = time - 1;
		ravenstaff_idle();
	}
}

void ravenstaff_deselect (void)
{
	self.wfs=advanceweaponframe($select12,$select1);
	self.th_weapon=ravenstaff_deselect;
	thinktime self : HX_FRAME_TIME;

	self.oldweapon = IT_WEAPON4;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

