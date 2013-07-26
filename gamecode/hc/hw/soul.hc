/*
 * hw/soul.hc
 */

void () crusader_soul_touch =
{
	if(!other.flags2&FL_ALIVE)
		return;

	if ((other.classname == "player") && (other.playerclass==CLASS_CRUSADER))
	{
		sound (self, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
		other.super_damage_time = time + 30;
		other.super_damage = 1;
		other.super_damage_low = 0;

		self.touch = SUB_Null;
		self.think=SUB_Remove;
		thinktime self : HX_FRAME_TIME * 2;

		self.enemy.think=SUB_Remove;
		thinktime self.enemy : HX_FRAME_TIME * 2;

		sprint (other, PRINT_MEDIUM, "You now have Holy Strength!\n");
	}
	// Bad people are hurt by this
	else if ((other.classname == "player") &&
			((other.playerclass==CLASS_NECROMANCER) ||
			 (other.playerclass==CLASS_SUCCUBUS) ||
			 (other.playerclass==CLASS_ASSASSIN)))
	{
		if (self.pain_finished < time)
			T_Damage (other, self, self, 5);
		self.pain_finished = time + .2;
	}
};

void () necro_soul_touch =
{
	if(!other.flags2&FL_ALIVE)
		return;

	if ((other.classname == "player") && (other.playerclass==CLASS_NECROMANCER))
	{
		sound (self, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
		other.health += self.health;
		if (other.health>other.max_health)
			other.health = other.max_health;
		other.bluemana += self.bluemana;
		if (other.bluemana > other.max_mana)
			other.bluemana = other.max_mana;
		other.greenmana += self.greenmana;
		if (other.greenmana > other.max_mana)
			other.greenmana = other.max_mana;

		sprint (other, PRINT_MEDIUM, "You have devoured a life force!\n");

		self.touch = SUB_Null;
		self.think=SUB_Remove;
		thinktime self : HX_FRAME_TIME * 2;

		self.enemy.think=SUB_Remove;
		thinktime self.enemy : HX_FRAME_TIME * 2;
	}
	// Good people are hurt by this
	else if ((other.classname == "player") &&
			((other.playerclass==CLASS_PALADIN) ||
			 (other.playerclass==CLASS_CRUSADER)))
	{
		if (self.pain_finished < time)
			T_Damage (other, self, self, 5);
		self.pain_finished = time + .2;
	}
};

void () soul_move =
{
	vector vel;
	float p_color;

	self.velocity_z += self.hoverz;
	if (self.velocity_z > 16)
		self.hoverz = -1;
	else if (self.velocity_z < -16 )
		self.hoverz = 1;

	self.enemy.velocity_z = self.velocity_z;	// keep in sync
	self.enemy.hoverz = self.hoverz;

	if (self.classname == "soulskull")
	{
		p_color = 144;
		vel = randomv('-1.44 -1.44 -65', '1.44 1.44 -35');
		particle2(self.origin - '0 0 20',vel,vel,p_color,PARTICLETYPE_C_EXPLODE,self.health/4);
		setorigin (self.enemy, self.origin + '0 0 1');
	}
	else if (self.classname == "soulcross")
	{
		p_color = 176;
		vel = randomv('-1.44 -1.44 65', '1.44 1.44 35');
		particle2(self.origin +'0 0 25',vel,vel,p_color,PARTICLETYPE_C_EXPLODE,self.health/4);
		setorigin (self.enemy, self.origin);
	}

	self.health -= 0.1;

	self.think = soul_move;
	thinktime self : 0.1;
	if (self.sound_time < time)
	{
		self.sound_time = time + 4;
		sound (self, CHAN_VOICE, "raven/soul.wav", 1, ATTN_NORM);
	}

	if (self.health <= 0)
	{
		stopSound(self,CHAN_VOICE);
		CreateLittleWhiteFlash(self.origin);
		remove(self);
		remove(self.enemy);
		return;
	}

	if (self.health < 4)
	{
		if (self.scale > 0.2)
		{
			self.scale -= 0.02;
			self.enemy.scale -= 0.02;
		}
	}
};


void necromancer_sphere (entity ent)
{
	entity new,new2;
	float chance;

	chance = .05 + ((ent.level - 3) * .03);
	if (chance > .2)
		chance = .2;

	if (random() > chance)
		return;
	if (self.classname == "rider_death")
		return; /* won't be accessible anyway */

	droptofloor();

	new2 = spawn();
	new2.owner = new;
	new2.solid = SOLID_TRIGGER;
	new2.movetype = MOVETYPE_NOCLIP;
	setorigin (new2, self.origin + '0 0 32');
	setmodel (new2, "models/soulskul.mdl");
	setsize (new2, new2.mins , new2.maxs);
	new2.classname = "soulskull";
	new2.touch = necro_soul_touch;
	new2.think = soul_move;
	thinktime new2 : 0.1;
	new2.hoverz=1;
	new2.velocity_x = 0;
	new2.velocity_y = 0;
	new2.velocity_z = new2.hoverz;
	new2.flags=0;
	new2.drawflags(+)MLS_FULLBRIGHT;
	new2.lifespan = 15;  // Alive for 15 seconds
	new2.health = new2.lifespan * 2;
	new2.avelocity_y = 200;
	new2.scale = 1;
	sound (new, CHAN_VOICE,"raven/soul.wav", 1, ATTN_NORM);

	new = spawn();
	new.owner = new;
	new.solid = SOLID_NOT;
	new.movetype = MOVETYPE_NOCLIP;
	setorigin (new, new2.origin + '0 0 1'); // correct slight excentricity, otherwise conspicuous when scaling down
	setmodel (new, "models/soulball.mdl");
	setsize (new, new.mins , new.maxs);
	new.hoverz=1;
	new.velocity_x = 0;
	new.velocity_y = 0;
	new.velocity_z = new.hoverz;
	new.flags=0;
	new.drawflags(+)MLS_FULLBRIGHT;
	new.sound_time = 2;
	new.scale = 1;

	new2.enemy = new;
}

void crusader_sphere (entity ent)
{
	local entity new,new2;
	float chance;

	chance = .05 + ((ent.level - 3) * .03);
	if (chance > .2)
		chance = .2;

	if (random() > chance)
		return;
	if (self.classname == "rider_death")
		return; /* won't be accessible anyway */

	droptofloor();

	new2 = spawn();
	new2.owner = new2;
	new2.solid = SOLID_TRIGGER;
	new2.movetype = MOVETYPE_NOCLIP;
	setorigin (new2, self.origin + '0 0 32');
	setmodel (new2, "models/cross.mdl");
	setsize (new2, new2.mins, new2.maxs);
	new2.classname = "soulcross";
	new2.touch = crusader_soul_touch;
	new2.think = soul_move;
	thinktime new2 : 0.1;
	new2.hoverz = 1;
	new2.velocity_x = 0;
	new2.velocity_y = 0;
	new2.velocity_z = new2.hoverz;
	new2.flags=0;
	new2.drawflags(+)MLS_FULLBRIGHT;
	new2.lifespan = 15;  // Alive for 15 seconds
	new2.health = new2.lifespan * 2;
	new2.avelocity_y = 200;
	new2.scale = 1;
	sound (new, CHAN_VOICE, "raven/soul.wav", 1, ATTN_NORM);

	new = spawn();
	new.owner = new;
	new.solid = SOLID_NOT;
	new.movetype = MOVETYPE_NOCLIP;
	setorigin (new, new2.origin);
	setmodel (new, "models/goodsphr.mdl");
	setsize (new, new.mins, new.maxs);
	new.hoverz = 1;
	new.velocity_x = 0;
	new.velocity_y = 0;
	new.velocity_z = new.hoverz;
	new.flags=0;
	new.drawflags(+)MLS_ABSLIGHT;
	new.sound_time = 2;
	new.scale = 1;

	new2.enemy = new;
}

