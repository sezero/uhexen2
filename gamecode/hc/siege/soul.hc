/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/soul.hc,v 1.2 2007-02-07 17:01:24 sezero Exp $
 */

// Possible improvement: Make model shrink before disappearing

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
		((other.playerclass==CLASS_PALADIN) || (other.playerclass==CLASS_CRUSADER)))
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

	if (self.classname == "soulskull") 
	{
		p_color = 144;
		vel = randomv('-1.44 -1.44 -65', '1.44 1.44 -35');
		particle2(self.origin - '0 0 20',vel,vel,p_color,PARTICLETYPE_C_EXPLODE,self.health/4);
	}

	else if (self.classname == "soulcross")
	{	
		p_color = 176;
		vel = randomv('-1.44 -1.44 65', '1.44 1.44 35');
		particle2(self.origin +'0 0 25',vel,vel,p_color,PARTICLETYPE_C_EXPLODE,self.health/4);
	}

	setorigin (self.enemy, self.origin);	// Move the ball with it

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
		//sound (self, CHAN_VOICE, "misc/null.wav", 1, ATTN_NORM);
		remove(self);
		remove(self.enemy);
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

	new2 = spawn();

	new2.owner = new;
	new2.solid = SOLID_TRIGGER;
	new2.movetype = MOVETYPE_FLY;

	droptofloor();
	setorigin (new2, self.origin + '0  0 32');
	setmodel (new2, "models/soulskul.mdl");
	new2.classname = "soulskull";
	setsize (new2, new2.mins , new2.maxs);
	new2.think = soul_move;
	thinktime new2 : 0.1;
	new2.hoverz=1;
	new2.velocity_z=new.hoverz;
	new2.touch = necro_soul_touch;
	new2.flags=0;
	new2.lifespan = 15;  // Alive for 15 seconds
	new2.health = new2.lifespan * 2;

	new2.avelocity_y = 200;
	sound (new, CHAN_VOICE,"raven/soul.wav", 1, ATTN_NORM);


	new = spawn();
	new.owner = new;
	new.solid = SOLID_TRIGGER;
	new.movetype = MOVETYPE_FLY;

	setmodel (new, "models/soulball.mdl");

	setsize (new, new.mins , new.maxs);
	new.flags=0;
	setorigin (new, new2.origin);

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

	new2 = spawn();

	new2.owner = new2;
	new2.solid = SOLID_TRIGGER;
	new2.movetype = MOVETYPE_FLY;

	droptofloor();
	setorigin (new2, self.origin + '0  0 32');
	setmodel (new2, "models/cross.mdl");

	setsize (new2, new2.mins , new2.maxs);
	new2.classname = "soulcross";
	new2.touch = crusader_soul_touch;
	new2.think = soul_move;
	thinktime new2 : 0.1;
	new2.hoverz=1;
	new2.velocity_z=new.hoverz;
	new2.flags=0;
	new2.lifespan = 15;  // Alive for 15 seconds
	new2.health = new2.lifespan * 2;

	new2.avelocity_y = 200;
	sound (new, CHAN_VOICE, "raven/soul.wav", 1, ATTN_NORM);


	new = spawn();

	new.owner = new;
	new.solid = SOLID_TRIGGER;
	new.movetype = MOVETYPE_FLY;

	setorigin (new, new2.origin);
	setmodel (new, "models/goodsphr.mdl");

	new.drawflags (+) MLS_ABSLIGHT;

	setsize (new, new.mins , new.maxs);
	new.hoverz=1;
	new.velocity_z=new.hoverz;
	new.flags=0;
	new.lifespan = 15;  // Alive for 15 seconds
	new.health = new.lifespan * 2;
	new.sound_time = 2;

	new2.enemy = new;
}

