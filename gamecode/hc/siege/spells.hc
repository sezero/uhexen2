entity check_legal_spell_target_infront ()
{
	traceline(self.origin+self.view_ofs,self.origin+self.view_ofs+v_forward*128,FALSE,self);
	if(trace_ent.flags2&FL_ALIVE)
		return trace_ent;
	else
		return self;
}

void health_spell_touch ()
{
//sfx
	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.flags2&FL_ALIVE)
	{
		other.health+=5;
		if(other.health>other.max_health&&other.max_health)
			other.health=other.max_health;
	}
	CreateBSpark(self.origin);
	remove(self);
}

void heal ()
{
entity targ_ent,health_spell;
	if(self.bluemana<5)
	{
		centerprint(self,"Not enough mana for this spell (5)");
		return;
	}

	targ_ent=check_legal_spell_target_infront();
	makevectors(self.v_angle);
	if(targ_ent==self)
	{
		if(self.health==self.max_health)
		{
			centerprint(self,"You are already at your full health!\n");
			return;
		}
		//sound
		//graphic
		self.health+=5;
		if(self.health>self.max_health)
			self.health=self.max_health;
	}
	else
	{
		health_spell=spawn();
		health_spell.solid=SOLID_BBOX;
		health_spell.movetype=MOVETYPE_FLYMISSILE;
		health_spell.touch=health_spell_touch;
		health_spell.owner=self;
		health_spell.velocity=v_forward*700;
		health_spell.angles=vectoangles(v_forward);
		setmodel(health_spell,"models/shardice.mdl");
		setsize(health_spell,'0 0 0','0 0 0');
		setorigin(health_spell,self.origin+self.proj_ofs+v_forward*8);
	}
	self.attack_finished=time+0.5;
	self.bluemana-=5;
}

void poison_spell_touch ()
{
	if(other.classname!="player") // added by KS
	{
		self.think = SUB_Remove;
		self.nextthink = time+0.1;
		return;
	}

	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.flags&FL_CLIENT)
	{
		spawn_poison(other,self.owner,60);//poisoned for a whole minute!
		PlayerSpeed_Calc (other);
		centerprint(other,"You have been poisoned!\n");
	}
	CreateGreenSpark(self.origin);
	remove(self);
}

void poison ()
{
entity poison_spell;
	if(self.greenmana<15)
	{
		centerprint(self,"Not enough mana for this spell (need 15)");
		return;
	}

	makevectors(self.v_angle);
	poison_spell=spawn();
	poison_spell.solid=SOLID_BBOX;
	poison_spell.movetype=MOVETYPE_FLYMISSILE;
	poison_spell.touch=poison_spell_touch;
	poison_spell.owner=self;
	poison_spell.velocity=v_forward*700;
	poison_spell.angles=vectoangles(v_forward);
	setmodel(poison_spell,"models/sucwp2p.mdl");
	setsize(poison_spell,'0 0 0','0 0 0');
	setorigin(poison_spell,self.origin+self.proj_ofs+v_forward*8);
	self.attack_finished=time+1;
	self.greenmana-=15;
}

void speed_spell_touch ()
{
	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.flags&FL_CLIENT)
	{
		Use_Haste(other);
		centerprint(other,"You are endowed with the speed of Mercury!\n");
	}
	CreateBSpark(self.origin);
	remove(self);
}

void mercury_speed ()
{
entity targ_ent,speed_spell;
	if(self.bluemana<70)
	{
		centerprint(self,"Not enough mana for this spell (need 70)");
		return;
	}

	targ_ent=check_legal_spell_target_infront();
	makevectors(self.v_angle);
	if(targ_ent==self)
	{
		if (self.artifact_active & ART_HASTE)
		{
			centerprint(self,"You are already Mercurially Fast!\n");
			return;
		}
		//sound
		//graphic
		centerprint(self,"You are endowed with the speed of Mercury!\n");
		Use_Haste(self);
	}
	else
	{
		speed_spell=spawn();
		speed_spell.solid=SOLID_BBOX;
		speed_spell.movetype=MOVETYPE_FLYMISSILE;
		speed_spell.touch=speed_spell_touch;
		speed_spell.owner=self;
		speed_spell.velocity=v_forward*700;
		speed_spell.angles=vectoangles(v_forward);
		setmodel(speed_spell,"models/shardice.mdl");
		setsize(speed_spell,'0 0 0','0 0 0');
		setorigin(speed_spell,self.origin+self.proj_ofs+v_forward*8);
	}
	self.attack_finished=time+1;
	self.bluemana-=70;
}

void berzerker_spell_touch ()
{
	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.flags2&FL_ALIVE&&!other.super_damage)
	{
		//sound
		//graphic
		other.super_damage=4;
		other.super_damage_time=time+15;
		other.colormap=140;
		if(other.flags&FL_CLIENT)
			centerprint(other,"You are endowed with Holy Strength!\n");
	}
	CreateBSpark(self.origin);
	remove(self);
}

void berzerker ()
{
entity targ_ent,berzerker_spell;
	if(self.bluemana<50)
	{
		centerprint(self,"Not enough mana for this spell(50)");
		return;
	}

	targ_ent=check_legal_spell_target_infront();
	makevectors(self.v_angle);
	if(targ_ent==self)
	{
		if(self.super_damage)
		{
			centerprint(self,"You already have Holy Strength!\n");
			return;
		}
		//sound
		//graphic
		centerprint(self,"You are endowed with Holy Strength!\n");
		self.super_damage=4;
		self.super_damage_time=time+15;
		self.colormap=140;
	}
	else
	{
		berzerker_spell=spawn();
		berzerker_spell.solid=SOLID_BBOX;
		berzerker_spell.movetype=MOVETYPE_FLYMISSILE;
		berzerker_spell.touch=berzerker_spell_touch;
		berzerker_spell.owner=self;
		berzerker_spell.velocity=v_forward*700;
		berzerker_spell.angles=vectoangles(v_forward);
		setmodel(berzerker_spell,"models/shardice.mdl");
		setsize(berzerker_spell,'0 0 0','0 0 0');
		setorigin(berzerker_spell,self.origin+self.proj_ofs+v_forward*8);
	}
	self.attack_finished=time+1;
	self.bluemana-=50;
}

void invis_spell_touch ()
{
	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.flags2&FL_ALIVE)
	{
		//sound
		//graphic
		if(other.flags&FL_CLIENT)
		{
			UseInvisibility(other);
			centerprint(other,"You are shrouded in The Cloak of Perseus!\n");
		}
		else
		{
			other.invisible_time = time + 999;
			other.effects(+)EF_NODRAW|EF_DIMLIGHT;
			other.oldskin=other.skin;
			other.skin=101;
			other.drawflags(+)DRF_TRANSLUCENT;
		}
	}
	CreateBSpark(self.origin);
	remove(self);
}

void Invisibility ()
{
entity targ_ent,invis_spell;
	if(self.bluemana<80)
	{
		centerprint(self,"Not enough mana for this spell (need 80)");
		return;
	}

	targ_ent=check_legal_spell_target_infront();
	makevectors(self.v_angle);
	if(targ_ent==self)
	{
		if (self.artifact_active & ART_INVISIBILITY)
		{
			centerprint(self,"You are already invisible!\n");
			return;
		}
		//sound
		//graphic
		centerprint(self,"You are shrouded in The Cloak of Perseus!\n");
		UseInvisibility(self);
	}
	else
	{
		invis_spell=spawn();
		invis_spell.solid=SOLID_BBOX;
		invis_spell.movetype=MOVETYPE_FLYMISSILE;
		invis_spell.touch=invis_spell_touch;
		invis_spell.owner=self;
		invis_spell.velocity=v_forward*700;
		invis_spell.angles=vectoangles(v_forward);
		setmodel(invis_spell,"models/shardice.mdl");
		setsize(invis_spell,'0 0 0','0 0 0');
		setorigin(invis_spell,self.origin+self.proj_ofs+v_forward*8);
	}
	self.attack_finished=time+1;
	self.bluemana-=80;
}

void Wrath_Of_God ()
{
float num_strikes;
entity found;
	if(self.bluemana<self.max_mana)
	{
		centerprint(self,"You must have full mana to cast this spell\n");
		return;
	}
	self.bluemana=0;
	found=findradius(self.origin,1024);
	while(found!=world&&num_strikes<16)
	{	
		if(found.flags2&FL_ALIVE&&found.playerclass!=CLASS_DWARF)
		{
			if(infront(found))
				if(visible(found))
				{
					if(found.flags&FL_CLIENT)
						centerprint(found,"You have been smitten by the Wrath of God!!!\n");
					traceline(found.origin,found.origin+'0 0 2000',TRUE,self);
					do_lightning(self,num_strikes,0,4,trace_endpos,found.origin,2000,TE_STREAM_LIGHTNING);
					num_strikes+=1;
				}
		}
		found=found.chain;
	}
	starteffect(CE_FLOOR_EXPLOSION , self.origin);
	T_Damage(self,self,self,5000);
}


void beast_charge ()
{
	if(self.act_state!=ACT_ATTACK)
		self.act_state=ACT_ATTACK;
}

void UnBeast (entity targ)
{
entity oself;
	setmodel(targ,targ.init_model);
	targ.frame=0;
	targ.act_state=ACT_STAND;
	targ.beast_time=0;
	targ.scale=1;
	self.drawflags(-)SCALE_ORIGIN_CENTER;
	targ.greenmana=0;
	if(targ.health>targ.max_health)
		targ.health=targ.max_health;

	sound(targ,CHAN_VOICE,"eidolon/growl.wav",1,ATTN_NORM);
	particleexplosion((targ.absmin+targ.absmax)*0.5,random(144,159),targ.absmax_z-targ.absmin_z,50);
	GenerateTeleportEffect(targ.origin,1);

	targ.oldweapon = FALSE;
	targ.weapon = IT_WEAPON1;

	oself=self;
	self=targ;
	restore_weapon();
	SetModelAndThinks();
	self=oself;

	if(targ.playerclass!=CLASS_DWARF)
	{
		setsize (targ, '-16 -16 0', '16 16 56');	
		targ.view_ofs='0 0 50';
		targ.hull=HULL_PLAYER;
	}
	else
	{
		setsize (targ, '-16 -16 0', '16 16 28');	
		targ.view_ofs='0 0 22';
		targ.hull=HULL_CROUCH;
	}
	PlayerSpeed_Calc(targ);
}

float cant_fit (vector bmins,vector bmaxs)
{
float	o_hull;
vector	o_origin,o_mins,o_maxs;
	o_hull=self.hull;
	o_origin = self.origin;
	o_mins = self.mins;
	o_maxs=self.maxs;
	setmodel(self,"models/yakman.mdl");
	setorigin(self,self.origin + '0 0 -1' * bmins_z);//pop up 50
	setsize(self,bmins,bmaxs);
	self.hull=HULL_GOLEM;
	tracearea(self.origin,self.origin+'0 0 2',bmins,bmaxs,FALSE,self);
	if(trace_fraction<1||trace_startsolid||trace_allsolid)
	{
		setorigin(self,o_origin);
		setmodel(self,self.init_model);
		setsize(self,o_mins,o_maxs);
		self.hull=o_hull;
		return TRUE;
	}

	return FALSE;
}

void BecomeBeast ()
{
	if(cant_fit('-48 -48 -50','48 48 50'))
	{
		self.attack_finished = time + 1;
		centerprint(self,"Not enough room to Become Beast here!\n");
		return;
	}
	self.frame=0;
	self.act_state=ACT_YAK_HOWL;
	self.weaponmodel = string_null;
	self.teleport_time=time+2;
	sound(self,CHAN_VOICE,"eidolon/roar.wav",1,ATTN_NONE);
	self.pain_finished=time+3;
	self.beast_time=time+120;
	self.drawflags(+)SCALE_ORIGIN_CENTER;
	self.scale=1.33;
	self.th_weapon=beast_charge;
	self.greenmana-=200;
	self.health=self.health/self.max_health * 999;
	self.view_ofs='0 0 80';
	self.mass=500;
	self.hasted = 1.666;
	self.think=player_frames;
	thinktime self : 0;
	self.attack_finished=time+2;
	self.weapon=IT_WEAPON1;
//	if(dmMode==DM_SIEGE&&self.siege_team==ST_ATTACKER)
//		self.skin=2;
}	

void dispel_spell_touch ()
{
	if(other.playerclass==CLASS_DWARF)
		return;

	if(other.classname=="monster_imp_lord")
	{
		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_TELEPORT_LINGER);
		WriteCoord (MSG_BROADCAST, other.origin_x);
		WriteCoord (MSG_BROADCAST, other.origin_y);
		WriteCoord (MSG_BROADCAST, other.origin_z);
		WriteCoord (MSG_BROADCAST, 3.0);
		stopSound(other,0);
		remove(other);
		return;
	}

	if(other.flags2&FL_ALIVE)
	{
		if(other.sheep_time>time&&other.flags2&FL2_EXCALIBUR&&self.owner.siege_team==ST_DEFENDER)
			self.owner.experience+=30;//Unsheeped DoC

		if(other.flags2&FL2_POISONED)
			other.flags2(-)FL2_POISONED;
		other.sheep_time =
		other.beast_time =
		other.super_damage_time =
		other.haste_time =
		other.tome_time =
		other.ring_regen_time =
		other.ring_water_time =
		other.ring_flight_time = 0;
		if(other.invisible_time>time)
		{
			other.invisible_time = 0;
			other.flags(-)EF_NODRAW|EF_LIGHT;
			other.drawflags(-)DRF_TRANSLUCENT;
			other.skin=self.oldskin;
		}
		if(other.flags2&FL_CLIENT)
			centerprint(other,"You have been cleansed of magic!\n");
	}
	CreateBSpark(self.origin);
	remove(self);
}

void DispelMagic ()
{
entity targ_ent,dispel_spell;
	if(self.bluemana<30)
	{
		centerprint(self,"Not enough mana for this spell (need 30)");
		return;
	}

	targ_ent=check_legal_spell_target_infront();
	makevectors(self.v_angle);
	if(targ_ent==self)
	{
		if(self.flags2&FL2_POISONED)
			self.flags2(-)FL2_POISONED;
		self.sheep_time =
		self.beast_time =
		self.super_damage_time =
		self.haste_time =
		self.tome_time =
		self.ring_regen_time =
		self.ring_water_time =
		self.ring_flight_time = 0;
		if(self.invisible_time>time)
		{
			self.invisible_time = 0;
			self.flags(-)EF_NODRAW|EF_LIGHT;
			self.drawflags(-)DRF_TRANSLUCENT;
			self.skin=self.oldskin;
		}

		centerprint(self,"You are cleansed of magic!\n");
		return;
	}
	else
	{
		dispel_spell=spawn();
		dispel_spell.solid=SOLID_BBOX;
		dispel_spell.movetype=MOVETYPE_FLYMISSILE;
		dispel_spell.touch=dispel_spell_touch;
		dispel_spell.owner=self;
		dispel_spell.velocity=v_forward*700;
		dispel_spell.angles=vectoangles(v_forward);
		setmodel(dispel_spell,"models/shardice.mdl");
		setsize(dispel_spell,'0 0 0','0 0 0');
		setorigin(dispel_spell,self.origin+self.proj_ofs+v_forward*8);
	}
	self.attack_finished=time+1;
	self.bluemana-=30;
}
