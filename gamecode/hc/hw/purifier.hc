/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/purifier.hc,v 1.3 2007-03-18 08:11:07 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\purifier\final\purifier.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\purifier\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame Rootpose     
//
$frame 1Lshot1      1Lshot2      1Lshot3      

//
$frame 1Rshot1      1Rshot2      1Rshot3      

//
$frame 2Lshot1      2Lshot2      2Lshot3      

//
$frame 2Rshot1      2Rshot2      2Rshot3      

//
$frame 3Rshot1      3Rshot2      3Rshot3      

//
$frame bigshot1     bigshot2     bigshot3     bigshot4     bigshot5     
$frame bigshot6     bigshot7     bigshot8     bigshot9     

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12          


//==================================================================


void purifier_ready (void);

void pmissile_gone(void)
{
	stopSound(self,0);
	//sound (self, CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
	//sound (self, CHAN_WEAPON, "misc/null.wav", 1, ATTN_NORM);
	remove(self);
}

/*
============
pmissile2_touch - missile2 hit something. Hurt it bad
============
*/
void pmissile2_touch (void)
{
	float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		pmissile_gone();
		return;
	}

	// don't do radius damage to the other, because all the damage
	// was done in the impact
	//damg = random(150,200);
	damg = random(60,110);
	if (other.health)
	{
		T_Damage (other, self, self.owner, damg );
		if((other.flags&FL_CLIENT||other.flags&FL_MONSTER)&&other.mass<200)
		{
			vector hitdir;

			hitdir=self.movedir*300;
			hitdir_z+=150;
			if(hitdir_z<0)
				hitdir_z=0;
			other.velocity=hitdir;
			other.flags(-)FL_ONGROUND;
		}
	}

	damg = random(120,160);
	T_RadiusDamage (self, self.owner, damg, other);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_PURIFY2_EXPLODE);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
	multicast(self.origin,MULTICAST_PHS_R);

	remove(self);
}

/*
============
pmissile2_puff - create smoke ring behind missile
============
*/
void pmissile2_puff(void)
{
	self.nextthink = time + .3;
	self.think = pmissile2_puff;

	if(time>self.lifetime - 1.7)	//Don't start tracking until it's been in the world 1/3 of a second
	{
		HomeThink();
		self.angles=vectoangles(self.velocity);
	}

	if (self.lifetime < time)	// Kill missile if it's time is up
		pmissile_gone();

	self.movedir = normalize(self.velocity);
	self.angles = vectoangles(self.movedir);

	// 300 is how far this thing travels in .3 seconds...
	traceline(self.origin, self.origin + self.movedir * 300.0, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_PURIFY2_MISSILE);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);
}

/*
============
launch_pmissile2 - create and launch power up missile
============
*/
void launch_pmissile2 (void)
{
	local entity missile;

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.frags=TRUE;

	missile.classname = "purimissile";
			
	// set missile speed	
	makevectors (self.v_angle);
	missile.velocity = normalize(v_forward);
    missile.velocity = missile.velocity * 1000;
	
	missile.touch = pmissile2_touch;
	missile.angles = vectoangles(missile.velocity);

	//sound (self, CHAN_VOICE, "paladin/purfireb.wav", 1, ATTN_NORM);
	weapon_sound(self, "paladin/purfireb.wav");

	setmodel (missile, "models/drgnball.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

//	setorigin (missile, self.origin + v_forward*10 + v_right * 1 + v_up * 40);
	setorigin (missile, self.origin + self.proj_ofs + v_forward*10);

	missile.effects=EF_BRIGHTLIGHT;
	missile.nextthink = time + .3;
	missile.think = pmissile2_puff;
	missile.lifetime = time + 2;
//Homing stuff-------------------
	missile.veer=FALSE;	//No random wandering
	missile.turn_time=3;//Lower the number, tighter the turn
	missile.speed=1000;	//Speed
	missile.ideal_yaw=TRUE;//Only track things in front
//End homing stuff-------------------
	self.greenmana -= 8;
	self.bluemana -= 8;

	//missile.effects(+)EF_PURIFY2_EFFECT;
	missile.effects(+)EF_NODRAW;

	entity oldself;
	oldself = self;
	self = missile;

	missile.think();

	self = oldself;
}


/*
============
purifier_tomefire - firing animation when in power up mode
============
*/
void purifier_tomefire (void)
{
	self.wfs = advanceweaponframe($bigshot1,$bigshot9);
	self.th_weapon=purifier_tomefire;
	if(self.weaponframe==$bigshot2)
	{
		self.punchangle_x= -4;
		launch_pmissile2 ();
		if(self.classname=="player")
		{
			self.velocity+=normalize(v_forward)*-200;//include mass
			self.flags(-)FL_ONGROUND;
		}
		self.attack_finished = time + 1.0;
	}
	else if(self.wfs==WF_CYCLE_WRAPPED)
			purifier_ready();
}

/*
============
launch_pmissile1 - create and launch normal missile
============
*/
void launch_pmissile1 (void)
{
	vector	startSpot;
	float	damg;

	makevectors (self.v_angle);

	startSpot = self.origin + self.proj_ofs + v_forward * 6;

	if ((self.cnt==1) || (self.cnt==3))
	{
		startSpot += v_right * 10;
	}
	else if ((self.cnt==0) || (self.cnt==2))
	{
		startSpot -= v_right * 10;
	}

	traceline(startSpot, startSpot + v_forward * 2000, FALSE, self);

	damg = random(8,16);
	if (trace_ent.health)
	{
		T_Damage (trace_ent, self, self, damg );
	}

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_PURIFY1_EFFECT);
	WriteCoord (MSG_MULTICAST, startSpot_x);
	WriteCoord (MSG_MULTICAST, startSpot_y);
	WriteCoord (MSG_MULTICAST, startSpot_z);
	WriteByte (MSG_MULTICAST, (self.v_angle_y/360.0)*256.0);
	WriteByte (MSG_MULTICAST, (self.v_angle_x/360.0)*-256.0);
	WriteShort(MSG_MULTICAST, (trace_fraction * 2000));
	multicast(self.origin,MULTICAST_PVS);

	self.greenmana -= 1;
	self.bluemana -= 1;

	self.cnt += 1;

	if (self.cnt > 3)
	{
		self.cnt =0;
	}
}

/*
============
purifier_rapidfire? - different rapid fire animations
============
*/
void purifier_rapidfire2R (void)
{
	self.wfs = advanceweaponframe($2Rshot1,$2Rshot3);
	self.th_weapon=purifier_rapidfire2R;

	if (self.weaponframe == $2Rshot3)
		self.punchangle_x= random(-3);

	if (self.attack_finished <= time&&self.button0)
		launch_pmissile1();

	if (self.wfs==WF_CYCLE_WRAPPED)
		purifier_ready();
}

void purifier_rapidfire2L (void)
{
	self.wfs = advanceweaponframe($2Lshot1,$2Lshot3);
	self.th_weapon=purifier_rapidfire2L;

	if (self.weaponframe == $1Lshot3)
		self.punchangle_x= random(-3);

	if (self.attack_finished <= time&&self.button0)
		launch_pmissile1();

	if (self.wfs==WF_CYCLE_WRAPPED)
		purifier_ready();
}

void purifier_rapidfire1R (void)
{
	self.wfs = advanceweaponframe($1Rshot1,$1Rshot3);
	self.th_weapon=purifier_rapidfire1R;

	if (self.weaponframe == $1Rshot3)
		self.punchangle_x= random(-3);

	if (self.attack_finished <= time&&self.button0)
		launch_pmissile1();

	if (self.wfs==WF_CYCLE_WRAPPED)
		purifier_ready();
}

void purifier_rapidfire1L (void)
{
	self.wfs = advanceweaponframe($1Lshot1,$1Lshot3);
	self.th_weapon=purifier_rapidfire1L;

	if (self.weaponframe == $1Lshot3)
		self.punchangle_x= random(-3);

	if (self.attack_finished <= time&&self.button0)
		launch_pmissile1();

	if (self.wfs==WF_CYCLE_WRAPPED)
		purifier_ready();
}

/*
============
purifier_rapidfire - choose which rapid fire animation to use
============
*/
void purifier_rapidfire (void)
{
	
	if (self.counter ==0)
		purifier_rapidfire1L();
	else if (self.counter ==1)
		purifier_rapidfire1R();
	else if (self.counter ==2)
		purifier_rapidfire2L();
	else if (self.counter ==3)
		purifier_rapidfire2R();

	self.counter += 1;
	self.attack_finished = time + .14;

	if (self.counter > 3)
		self.counter =0;
}

/*
============
purifier_fire - shoot purifier.
============
*/
void() pal_purifier_fire =
{
	if ((self.artifact_active & ART_TOMEOFPOWER) &&
		(self.greenmana >= 8) && (self.bluemana >= 8))
		purifier_tomefire();
	else if ((self.greenmana >= 2) && (self.bluemana >= 2))
		purifier_rapidfire();

	self.nextthink=time;
};


/*
============
purifier_ready - just sit there until fired
============
*/
void purifier_ready (void)
{
	self.weaponframe = $Rootpose;
	self.wfs = $Rootpose;
	self.th_weapon=purifier_ready;
}

/*
============
purifier_deselect - purifier was just unchosen. Remove from view
============
*/
void purifier_deselect (void)
{
	self.wfs = advanceweaponframe($Select12,$Select1);
	self.th_weapon=purifier_deselect;
	self.oldweapon = IT_WEAPON4;

	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}

/*
============
purifier_select - purifier was just chosen. Bring into view
============
*/
void purifier_select (void)
{
	self.wfs = advanceweaponframe($select1,$select12);
	self.weaponmodel = "models/purifier.mdl";
	self.th_weapon=purifier_select;
	self.counter = 0;

	if (self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		purifier_ready();
	}
}

