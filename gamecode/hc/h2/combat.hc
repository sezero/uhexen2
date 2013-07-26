/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/combat.hc,v 1.3 2007-02-07 16:56:59 sezero Exp $
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
		if(self.playerclass == CLASS_ASSASSIN && self.weapon == IT_WEAPON1)
		{
		  if(self.level > 5) /* Pa3PyX: this ability starts at clvl 6 */ {
		    if((trace_ent.flags2 & FL_ALIVE) && !infront_of_ent(self, trace_ent) &&
							random(1, 10) < self.level) {
			CreateRedFlash(trace_endpos);
			damage_base*=random(2.5,4);
			backstab=TRUE;
		    }
		  }
		}

		damg = random(damage_mod+damage_base,damage_base);
		SpawnPuff (org, '0 0 0', damg,trace_ent);
		T_Damage (trace_ent, self, self, damg);
		if(!(trace_ent.flags2 & FL_ALIVE) && backstab)
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
			if ((trace_ent.flags & FL_MONSTER) || (trace_ent.flags & FL_CLIENT))
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

