/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/combat.hc,v 1.1 2005-01-26 17:26:10 sezero Exp $
 */
void(vector org, vector vel, float damage, entity victim) SpawnPuff;

float MetalHitSound (float targettype)
{
//entity found;

	if(targettype==THINGTYPE_FLESH)
	{
		sound (self, CHAN_WEAPON, "weapons/met2flsh.wav", 1, ATTN_NORM);
		return TRUE;
	}
	else
	{
//		found=find(world,classname,"misc_ripples");
//		if(found)
//			starteffect(CE_RIPPLE, found.origin,'0 0 0',HX_FRAME_TIME);
		if(targettype==THINGTYPE_WOOD||targettype==THINGTYPE_DIRT)
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
	float damg;//, backstab;
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
		if(self.playerclass==CLASS_PALADIN&&self.weapon==IT_WEAPON2&&!trace_ent.flags2&FL_ALIVE)
			damage_base*=1.3;

		if(self.playerclass==CLASS_DWARF)
			if(self.weapon==IT_WEAPON2&&!trace_ent.flags2&FL_ALIVE)
			{
				if(flammable(trace_ent))
					damage_base*=2.2;
				else if(trace_ent.thingtype==THINGTYPE_DIRT)
					damage_base*=1.5;
				else
					damage_base*=1.2;
			}
			else
				damage_base*=1.2;


/*		if(trace_ent.flags2&FL_ALIVE&&self.playerclass==CLASS_ASSASSIN)//!fov(self,trace_ent,90)
		{
		vector t_vf,m_vf;
			makevectors(trace_ent.angles);
			t_vf = v_forward;
			makevectors(self.angles);
			m_vf = v_forward;
			makevectors(self.v_angle);
			if(t_vf*m_vf>0.5)//facing generally the same direction
			{
				CreateRedFlash(trace_endpos);
				damage_base=trace_ent.health*random(0.75,1.2);
				if(damage_base>100)
					damage_base = 100;
				backstab=TRUE;
			}
		}*/

		damg = random(damage_mod+damage_base,damage_base);
		SpawnPuff (org, '0 0 0', damg,trace_ent);
		T_Damage (trace_ent, self, self, damg);
/*		if(backstab)
		{
			if(!trace_ent.flags2&FL_ALIVE)
				centerprint(self,"Critical Hit Backstab!\n");
			else
				centerprint(self,"Backstab!\n");
		}*/

		if(trace_ent.thingtype==THINGTYPE_FLESH)
			sound (self, CHAN_WEAPON, "weapons/slash.wav", 1, ATTN_NORM);
		else if(!MetalHitSound(trace_ent.thingtype))
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
		if(!MetalHitSound(trace_ent.thingtype))
			sound (self, CHAN_WEAPON, "weapons/hitwall.wav", 1, ATTN_NORM);		
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_GUNSHOT);
		WriteByte (MSG_BROADCAST, 1);
		WriteCoord (MSG_BROADCAST, org_x);
		WriteCoord (MSG_BROADCAST, org_y);
		WriteCoord (MSG_BROADCAST, org_z);
	}

}
/*
 * $Log: not supported by cvs2svn $
 * 
 * 16    5/25/98 10:56p Mgummelt
 * Last version before send-off, v0.15
 * 
 * 15    5/25/98 8:38p Mgummelt
 * 
 * 14    5/25/98 1:38p Mgummelt
 * 
 * 13    5/25/98 1:30p Mgummelt
 * 
 * 12    5/06/98 11:10p Mgummelt
 * Last version before 1st beta
 * 
 * 11    5/05/98 8:33p Mgummelt
 * Added 6th playerclass for Siege only- Dwarf
 * 
 * 10    4/30/98 6:01p Mgummelt
 * 
 * 9     4/27/98 6:54p Mgummelt
 * 
 * 8     4/27/98 4:34p Mgummelt
 * Siege beta version 0.13
 * 
 * 7     4/24/98 1:31a Mgummelt
 * Siege version 0.02 4/24/98 1:31 AM
 * 
 * 6     4/23/98 5:18p Mgummelt
 * Siege version 0.01 4/23/98
 * 
 * 5     4/17/98 9:00p Mgummelt
 * 1st version of Siege- loads but totally fucked
 * 
 * 3     4/09/98 1:57p Mgummelt
 * Some experience changes
 * 
 * 2     2/09/98 3:24p Rjohnson
 * Update temp ents
 * 
 * 1     2/04/98 1:59p Rjohnson
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
