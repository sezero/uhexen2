/*
==============================================================================

Q:\art\models\weapons\spllbook\spllbook.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\spllbook
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame normal02     normal03     normal04     normal05     normal06     
$frame normal07     normal08     normal09     normal10     normal11     
$frame normal12     normal13

//
$frame idlean12     idlean13     idlean14     idlean15     idlean16     
$frame idlean17     idlean18     idlean19     idlean20     idlean21     
$frame idlean22     idlean23     idlean24     idlean25     idlean26     
$frame idlean27     idlean28     idlean29     idlean30     idlean31     
$frame idlean32     idlean33     idlean34     idlean35     

//
$frame idlebn36     idlebn37     idlebn38     idlebn39     idlebn40     
$frame idlebn41     idlebn42     idlebn43     idlebn44     idlebn45     
$frame idlebn46     idlebn47     idlebn48     idlebn49     idlebn50     
$frame idlebn51     idlebn52     idlebn53     idlebn54     idlebn55     
$frame idlebn56     idlebn57     idlebn58     idlebn59     


//
$frame select80     select81     select82     select83     select84     
$frame select85     select86     select87     select88     select89     
$frame select90     select91     select92     select93     select94     
$frame select95     select96     select97     select98     select99     


void BloodMissileTouch (void)
{
	if(pointcontents(self.origin)==CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	self.movedir=normalize(self.velocity);

	if(!other.takedamage)
	{
		if(self.frags)
		{
			//counter for number of bounces
			sound (self, CHAN_WEAPON, "succubus/brnbounce.wav", 1, ATTN_NORM);
			if(self.dmg>=2)
				self.dmg-=1;
			starteffect(CE_BRN_BOUNCE,self.origin - self.movedir*8 - '0 0 6','0 0 0',HX_FRAME_TIME);//CreateRedFlash(self.origin);
			self.health += 45;
			thinktime self : 0;
			return;
		}
		else
		{
			starteffect(CE_BLDRN_EXPL, self.origin-self.movedir*6,'0 0 0', HX_FRAME_TIME);
			sound (self, CHAN_WEAPON, "succubus/brnwall.wav", 1, ATTN_NORM);
			remove(self);
			return;
		}
	}

	spawn_touchpuff(self.dmg,other);
	T_Damage(other,self,self.owner,self.dmg);
	starteffect(CE_BLDRN_EXPL, self.origin-self.movedir*6,'0 0 0', HX_FRAME_TIME);
	sound(self,CHAN_AUTO,"succubus/brnhit.wav",1,ATTN_NORM);

	remove(self);
}

void BloodMissileFade ()
{
	self.angles=vectoangles(self.velocity);
	if(self.health>30)
	{
		self.health-=60;
		thinktime self : 0.3;
		if(self.dmg>=1.75)
			self.dmg-=0.75;
	}
	else
	{
		remove(self);
		return;
	}

	traceline(self.origin, self.origin + self.movedir * 240, FALSE, newmis);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_BLOODRAIN);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 240.0);
	WriteByte (MSG_MULTICAST, self.health);
	multicast(self.origin,MULTICAST_PVS);
}

void FireBloodMissile (float offset)
{
	float f_dist;

	makevectors(self.v_angle);

	self.effects(+)EF_MUZZLEFLASH;
	newmis=spawn();
	newmis.classname="blood missile";
	newmis.owner=self;
	newmis.effects(+) EF_NODRAW;
	f_dist=8;
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		newmis.health=250;
		newmis.dmg=random(40,60);
		f_dist=16;
		newmis.solid=SOLID_PHASE;

		newmis.movetype=MOVETYPE_BOUNCEMISSILE;
		newmis.frags=TRUE;
	}
	else
	{
		newmis.dmg=random(15,22);
		newmis.health=90;
		newmis.frags=FALSE;
		newmis.movetype=MOVETYPE_FLYMISSILE;
	}

	newmis.solid=SOLID_BBOX;
	newmis.touch=BloodMissileTouch;

	newmis.speed=800;
	newmis.velocity=normalize(v_forward)*newmis.speed;// + spread;
	newmis.movedir=normalize(newmis.velocity);
	newmis.angles=vectoangles(newmis.velocity);

	setmodel(newmis,"models/sucwp1p.mdl");
	setsize(newmis,'0 0 0','0 0 0');

	setorigin(newmis,self.origin+self.proj_ofs+v_forward*f_dist-v_right*12 + v_right*(offset*3) - '0 0 6');
	weapon_sound(self, "succubus/brnfire.wav");

	newmis.think=BloodMissileFade;
	thinktime newmis : 0.3;

	traceline(newmis.origin, newmis.origin + newmis.movedir * 240, FALSE, newmis);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_BLOODRAIN);
	WriteCoord (MSG_MULTICAST, newmis.origin_x);
	WriteCoord (MSG_MULTICAST, newmis.origin_y);
	WriteCoord (MSG_MULTICAST, newmis.origin_z);
	WriteByte (MSG_MULTICAST, newmis.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, newmis.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 240.0);
	WriteByte (MSG_MULTICAST, newmis.health);
	multicast(self.origin,MULTICAST_PVS);
}

void  blrn_power(float offset)
{
	if(self.attack_finished>time)
		return;

	FireBloodMissile(offset);
	self.attack_finished=time+0.7;
	self.punchangle_x=3;
}

void  blrn_normal()
{
	if(self.attack_finished>time)
		return;

	FireBloodMissile(0);
	self.attack_finished=time+0.4;
}










/*======================
ACTION
select
deselect
ready loop
relax loop
fire once
fire loop
ready to relax(after short delay)
relax to ready(Fire delay?  or automatic if see someone?)
=======================*/


void()bloodrain_ready;
void() Suc_Blrn_Fire;

void bloodrain_fire (void)
{
	if(self.button0&&self.weaponframe==$normal07 &&!self.artifact_active&ART_TOMEOFPOWER)
		self.weaponframe=$normal07;
	else
		self.wfs = advanceweaponframe($normal02,$normal13);
	if(self.weaponframe<=$normal07)
		self.weaponframe+=1;
	self.th_weapon=bloodrain_fire;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED)
		bloodrain_ready();
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
//		if(self.weaponframe>=$normal07 && self.weaponframe<=$normal09)
//			blrn_power((self.weaponframe - $normal08) *3);
		if(self.weaponframe==$normal07)
			blrn_power(0);

	}
	else if(self.weaponframe==$normal07)
		blrn_normal();
}

void() Suc_Blrn_Fire =
{
	self.weaponframe_cnt=0;
	bloodrain_fire();

	thinktime self : 0;
};

void bloodrain_jellyfingers ()
{
	self.wfs = advanceweaponframe($idlebn36,$idlebn59);
	self.th_weapon=bloodrain_jellyfingers;
	if(self.wfs==WF_CYCLE_WRAPPED)
		bloodrain_ready();
}

void bloodrain_ready (void)
{
	self.wfs = advanceweaponframe($idlean12,$idlean35);
	if(random()<0.1&&self.weaponframe==$idlean35)
		self.th_weapon=bloodrain_jellyfingers;
	else
		self.th_weapon=bloodrain_ready;
}


void bloodrain_select (void)
{
	self.wfs = advanceweaponframe($select80,$select99);
	self.weaponmodel = "models/sucwp1.mdl";
	self.th_weapon=bloodrain_select;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		bloodrain_ready();
	}
}

void bloodrain_deselect (void)
{
	self.wfs = advanceweaponframe($select99,$select80);
	self.th_weapon=bloodrain_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}