/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/punchdgr.hc,v 1.2 2007-02-07 16:57:09 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\pnchdagr\final\punchdgr.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\pnchdagr\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

// Frame 1
$frame rootdag      
//
// Frame 2 - 9
$frame attacka1      attacka3      
$frame attacka7           
$frame attacka12     attacka13     attacka15     
$frame attacka16     attacka19     

// Frame 10 - 21
$frame attackb5
$frame attackb6      attackb7	   
$frame attackb16     attackb17     attackb18     attackb19     attackb20     
$frame attackb25
$frame attackb26     attackb27     attackb29     

// Frame 22 - 33
$frame attackc3      attackc4      attackc5      
$frame attackc6      attackc7           
$frame attackc11     attackc12     attackc13          
$frame attackc16     attackc18     attackc19     attackc20     

// Frame 34 - 
$frame attackd2      attackd4      attackd5      
$frame attackd6      attackd10     
$frame attackd11     attackd13     attackd14     attackd15     
$frame attackd16     attackd17     attackd20   attackd21     

$frame  f1  f2  f3  f4  f5  f6  f7  f8


// Frame Code
void() Ass_Pdgr_Fire;


void fire_punchdagger ()
{
	vector	source;
	vector	org,dir;
	float damg, inertia;
	float damage_mod;
	float damage_base;
	float c_level;
	damage_mod = 10;

	makevectors (self.v_angle);
	source = self.origin + self.proj_ofs;
	traceline (source, source + v_forward*64, FALSE, self);
	if (trace_fraction == 1.0)  
	{
		traceline (source, source + v_forward*64 - (v_up * 30), FALSE, self);  // 30 down	
		if (trace_fraction == 1.0)  
		{
			traceline (source, source + v_forward*64 + v_up * 30, FALSE, self);  // 30 up
			if (trace_fraction == 1.0)  
				return;
		}
	}
	
	org = trace_endpos + (v_forward * 4);

	if (trace_ent.takedamage)
	{

		//FIXME:Add multiplier for level and strength
		if (trace_ent.flags2&FL_ALIVE && !infront_of_ent(self,trace_ent) && self.playerclass==CLASS_ASSASSIN &&
              self.weapon==IT_WEAPON1 && self.level >5)
		{
			c_level = self.level;
			if (c_level > 10)
				c_level = 10;

			if (random(1,10)<=(c_level - 4))
			{
				damage_base = WEAPON1_PWR_BASE_DAMAGE;
				damage_mod = WEAPON1_PWR_ADD_DAMAGE;
				CreateRedFlash(trace_endpos);
				centerprint(self,"Critical Hit Backstab!\n");
				AwardExperience(self,trace_ent,10);
				damage_base*=random(2.5,4);
			}
		}

		else if (self.artifact_active & ART_TOMEOFPOWER)
		{
			damage_base = WEAPON1_PWR_BASE_DAMAGE;
			damage_mod = WEAPON1_PWR_ADD_DAMAGE;

			CreateWhiteFlash(org);
		
			if(trace_ent.mass<=10)
				inertia=1;
			else
				inertia=trace_ent.mass/10;

			if ((trace_ent.hull != HULL_BIG) && (inertia<1000) && (trace_ent.classname != "breakable_brush"))
			{
				if (trace_ent.mass < 1000)
				{
					dir =  trace_ent.origin - self.origin;
					trace_ent.velocity = dir * WEAPON1_PUSH*(1/inertia);
					if(trace_ent.movetype==MOVETYPE_FLY)
					{
						if(trace_ent.flags&FL_ONGROUND)
							trace_ent.velocity_z=200/inertia;
					}
					else
						trace_ent.velocity_z = 200/inertia;
					trace_ent.flags(-)FL_ONGROUND;
				}
			}	
		}
		else
		{
			damage_base = WEAPON1_BASE_DAMAGE;
			damage_mod = WEAPON1_ADD_DAMAGE;
		}

		damg = random(damage_mod + damage_base,damage_base);
		SpawnPuff (org, '0 0 0', damg,trace_ent);
		T_Damage (trace_ent, self, self, damg);

		if (!MetalHitSound(trace_ent.thingtype))
			sound (self, CHAN_WEAPON, "weapons/slash.wav", 1, ATTN_NORM);
	}
	else
	{	// hit wall
		sound (self, CHAN_WEAPON, "weapons/hitwall.wav", 1, ATTN_NORM);
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_GUNSHOT);
		WriteCoord (MSG_BROADCAST, org_x);
		WriteCoord (MSG_BROADCAST, org_y);
		WriteCoord (MSG_BROADCAST, org_z);

		if (self.artifact_active & ART_TOMEOFPOWER)
			CreateWhiteFlash(org);
		else
		{
			org = trace_endpos + (v_forward * -1) + (v_right * 15);
			org -= '0 0 26';
			CreateSpark (org);
		}
	}
}

void punchdagger_idle(void)
{
	self.th_weapon=punchdagger_idle;
	self.weaponframe=$rootdag;
}


void () punchdagger_d =
{
	self.th_weapon=punchdagger_d;
	self.wfs = advanceweaponframe($attackd2,$attackd21);

	if (self.weaponframe == $attackd5)
		sound (self, CHAN_WEAPON, "weapons/gaunt1.wav", 1, ATTN_NORM);

	if (self.weaponframe == $attackd10)
		fire_punchdagger();

	if(self.wfs==WF_CYCLE_WRAPPED)
		punchdagger_idle();
};


void () punchdagger_c =
{
	self.th_weapon=punchdagger_c;
	self.wfs = advanceweaponframe($attackc3,$attackc20);

	if (self.weaponframe == $attackc7)
		sound (self, CHAN_WEAPON, "weapons/gaunt1.wav", 1, ATTN_NORM);

	if (self.weaponframe == $attackc11)
		fire_punchdagger();

	if(self.wfs==WF_CYCLE_WRAPPED)
		punchdagger_idle();
};

void () punchdagger_b =
{
	self.th_weapon=punchdagger_b;
	self.wfs = advanceweaponframe($attackb5,$attackb29);

	if (self.weaponframe == $attackb6)
		sound (self, CHAN_WEAPON, "weapons/gaunt1.wav", 1, ATTN_NORM);

	if (self.weaponframe == $attackb18)
		fire_punchdagger();

	if(self.wfs==WF_CYCLE_WRAPPED)
		punchdagger_idle();
};

void () punchdagger_a =
{
	self.th_weapon=punchdagger_a;
	self.wfs = advanceweaponframe($attacka1,$attacka19);

	if (self.weaponframe == $attacka7)
		sound (self, CHAN_WEAPON, "weapons/gaunt1.wav", 1, ATTN_NORM);

	if (self.weaponframe == $attacka13)
		fire_punchdagger();

	if(self.wfs==WF_CYCLE_WRAPPED)
		punchdagger_idle();
};

float r2;


void Ass_Pdgr_Fire (void)
{
	self.attack_finished = time + .5;  // Attack every .7 seconds
//	r2 = rint(random(1,4));
	if (r2==1)
		punchdagger_a();
	else if (r2==2)
		punchdagger_b();
	else if (r2==3)
		punchdagger_c();
	else
		punchdagger_d();
	r2+=1;
	if (r2>4)
		r2=1;
}

void punchdagger_select (void)
{
	self.th_weapon=punchdagger_select;
	self.wfs = advanceweaponframe($f8,$f1);
	self.weaponmodel = "models/punchdgr.mdl";
	if(self.wfs==WF_CYCLE_STARTED)
		sound(self,CHAN_WEAPON,"weapons/unsheath.wav",1,ATTN_NORM);
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		punchdagger_idle();
	}
}

void punchdagger_deselect (void)
{
	self.th_weapon=punchdagger_deselect;
	self.wfs = advanceweaponframe($f1,$f8);
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();

}

