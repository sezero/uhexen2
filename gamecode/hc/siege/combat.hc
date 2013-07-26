/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/combat.hc,v 1.2 2007-02-07 17:00:36 sezero Exp $
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
		if(self.playerclass == CLASS_PALADIN)
		{
			if(self.weapon == IT_WEAPON2 && !(trace_ent.flags2&FL_ALIVE))
				damage_base*=1.3;
		}
		else if(self.playerclass == CLASS_DWARF)
		{
			if(self.weapon == IT_WEAPON2 && !(trace_ent.flags2&FL_ALIVE))
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
		}
		/*
		else if(self.playerclass == CLASS_ASSASSIN && (trace_ent.flags2&FL_ALIVE)) //!fov(self,trace_ent,90)
		{
		vector t_vf,m_vf;
			makevectors(trace_ent.angles);
			t_vf = v_forward;
			makevectors(self.angles);
			m_vf = v_forward;
			makevectors(self.v_angle);
			if(t_vf * m_vf > 0.5) //facing generally the same direction
			{
				CreateRedFlash(trace_endpos);
				damage_base=trace_ent.health * random(0.75,1.2);
				if(damage_base>100)
					damage_base = 100;
				backstab=TRUE;
			}
		}
		*/

		damg = random(damage_mod+damage_base,damage_base);
		SpawnPuff (org, '0 0 0', damg,trace_ent);
		T_Damage (trace_ent, self, self, damg);
		/*
		if(backstab)
		{
			if(!(trace_ent.flags2 & FL_ALIVE))
				centerprint(self,"Critical Hit Backstab!\n");
			else
				centerprint(self,"Backstab!\n");
		}
		*/

		if(trace_ent.thingtype==THINGTYPE_FLESH)
			sound (self, CHAN_WEAPON, "weapons/slash.wav", 1, ATTN_NORM);
		else if(!MetalHitSound(trace_ent.thingtype))
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

