/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/punchdgr.hc,v 1.1.1.1 2004-11-29 11:35:07 sezero Exp $
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

			if ((trace_ent.hull != HULL_SCORPION) && (inertia<1000) && (trace_ent.classname != "breakable_brush"))
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
/*
 * $Log: not supported by cvs2svn $
 * 
 * 2     2/20/98 4:39p Jmonroe
 * removed unused variables
 * 
 * 33    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 31    9/18/97 5:10p Rlove
 * 
 * 30    8/31/97 12:02a Mgummelt
 * 
 * 29    8/26/97 7:38a Mgummelt
 * 
 * 28    8/22/97 2:54p Jweier
 * 
 * 27    8/12/97 6:09p Mgummelt
 * 
 * 26    8/08/97 6:21p Mgummelt
 * 
 * 25    7/22/97 7:32a Rlove
 * 
 * 24    7/21/97 3:03p Rlove
 * 
 * 23    7/17/97 6:53p Mgummelt
 * 
 * 22    7/12/97 9:09a Rlove
 * Reworked Assassin Punch Dagger
 * 
 * 20    7/09/97 11:16a Rlove
 * 
 * 19    7/07/97 2:59p Mgummelt
 * 
 * 18    6/30/97 5:38p Mgummelt
 * 
 * 17    6/18/97 7:23p Mgummelt
 * 
 * 16    6/17/97 8:16p Mgummelt
 * 
 * 15    6/17/97 2:55p Rlove
 * 
 * 14    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 * 13    5/05/97 11:41a Rlove
 * New root position 
 * 
 * 12    4/18/97 11:44a Rlove
 * changed advanceweaponframe to return frame state
 * 
 * 11    4/17/97 1:28p Rlove
 * added new built advanceweaponframe
 * 
 * 10    4/16/96 11:52p Mgummelt
 * 
 * 9     4/12/96 8:56p Mgummelt
 * 
 * 8     4/11/96 1:51p Mgummelt
 * 
 * 7     4/10/96 2:50p Mgummelt
 * 
 * 6     4/09/97 2:41p Rlove
 * New Raven weapon sounds
 * 
 * 5     4/04/97 5:40p Rlove
 * 
 * 4     3/31/97 4:10p Rlove
 * Removed old animations
 * 
 * 3     3/31/97 6:38a Rlove
 * New punchdagger animations are operational
 * 
 * 2     3/21/97 9:38a Rlove
 * Created CHUNK.HC and MATH.HC, moved brush_die to chunk_death so others
 * can use it.
 * 
 * 1     3/20/97 4:01p Rlove
 */
