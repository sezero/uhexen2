/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/combat.hc,v 1.2 2005-02-23 08:11:25 sezero Exp $
 */
void(vector org, vector vel, float damage, entity victim) SpawnPuff;

float MetalHitSound (float targettype)
{
	if(targettype==THINGTYPE_FLESH)
	{
		sound (self, CHAN_WEAPON, "weapons/met2flsh.wav", 1, ATTN_NORM);
		return TRUE;
	}
	else if(targettype==THINGTYPE_WOOD)
	{
		sound (self, CHAN_WEAPON, "weapons/met2wd.wav", 1, ATTN_NORM);
		return TRUE;
	}
	else if(targettype==THINGTYPE_METAL)
	{
		sound (self, CHAN_WEAPON, "weapons/met2met.wav", 1, ATTN_NORM);
		return TRUE;
	}
	else if(targettype==THINGTYPE_BROWNSTONE||targettype==THINGTYPE_GREYSTONE)
	{
		sound (self, CHAN_WEAPON, "weapons/met2stn.wav", 1, ATTN_NORM);
		return TRUE;
	}
	return FALSE;
}


/*
================
FireMelee
================
*/
void FireMelee (float damage_base,float damage_mod,float attack_radius)
{
	vector	source;
	vector	org;
	float damg, backstab;
	float chance,point_chance;

	makevectors (self.v_angle);
	source = self.origin+self.proj_ofs;
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
	//	if(trace_ent.flags2&FL_ALIVE&&!infront_of_ent(self,trace_ent)&&self.playerclass==CLASS_ASSASSIN&&self.weapon==IT_WEAPON1&&random(1,10)<self.level)
	//	Pa3PyX: this ability starts at clvl 6
		if(trace_ent.flags2 & FL_ALIVE && !infront_of_ent(self, trace_ent) && self.playerclass == CLASS_ASSASSIN && self.weapon == IT_WEAPON1 && random(1, 10) < self.level && self.level > 5)
		{
			CreateRedFlash(trace_endpos);
			damage_base*=random(2.5,4);
			backstab=TRUE;
		}

		damg = random(damage_mod+damage_base,damage_base);
		SpawnPuff (org, '0 0 0', damg,trace_ent);
		T_Damage (trace_ent, self, self, damg);
		if(!trace_ent.flags2&FL_ALIVE&&backstab)
		{
			centerprint(self,"Critical Hit Backstab!\n");
			AwardExperience(self,trace_ent,10);
		}

		if(trace_ent.thingtype==THINGTYPE_FLESH)
			sound (self, CHAN_WEAPON, "weapons/slash.wav", 1, ATTN_NORM);
		else 
			sound (self, CHAN_WEAPON, "weapons/hitwall.wav", 1, ATTN_NORM);

		// Necromancer stands a chance of vampirically stealing health points
		if (self.playerclass == CLASS_NECROMANCER) 
		{
			if  ((trace_ent.flags & FL_MONSTER) || (trace_ent.flags & FL_CLIENT))	
			{
				chance = self.level * .05;

				if (chance > random())
				{
					point_chance = self.level;
					point_chance *= random();
					if (point_chance < 1)
						point_chance = 1;

					sound (self, CHAN_BODY, "weapons/drain.wav", 1, ATTN_NORM);

					self.health += point_chance;
					if (self.health>self.max_health)
						self.health = self.max_health;
				}
			}
		}
	}
	else
	{	// hit wall
		sound (self, CHAN_WEAPON, "weapons/hitwall.wav", 1, ATTN_NORM);
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_GUNSHOT);
		WriteCoord (MSG_BROADCAST, org_x);
		WriteCoord (MSG_BROADCAST, org_y);
		WriteCoord (MSG_BROADCAST, org_z);
	}

}
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/29 11:31:18  sezero
 * Initial import
 *
 * 
 * 36    10/28/97 1:00p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 34    7/12/97 9:09a Rlove
 * Reworked Assassin Punch Dagger
 * 
 * 32    7/07/97 2:59p Mgummelt
 * 
 * 31    6/30/97 6:14p Rlove
 * 
 * 30    6/30/97 5:30p Rlove
 * 
 * 29    6/28/97 6:32p Mgummelt
 * 
 * 28    6/26/97 4:46p Rlove
 * 
 * 27    6/26/97 4:39p Rlove
 * Special ability #2 for Necromancer: Vampiric Drain
 * 
 * 26    6/18/97 6:08p Mgummelt
 * 
 * 25    5/28/97 8:13p Mgummelt
 * 
 * 24    5/27/97 9:40a Rlove
 * Took out super_damage and radsuit fields
 * 
 * 23    5/12/97 10:31a Rlove
 * 
 * 22    5/06/97 1:29p Mgummelt
 * 
 * 21    4/25/97 8:31p Mgummelt
 * 
 * 20    4/24/97 2:15p Mgummelt
 * 
 * 19    4/12/96 8:55p Mgummelt
 * 
 * 18    4/09/97 2:41p Rlove
 * New Raven weapon sounds
 * 
 * 17    4/04/97 5:40p Rlove
 * 
 * 16    3/31/97 6:45a Rlove
 * Moved most of COMBAT.HC over to DAMAGE.HC
 * 
 */
