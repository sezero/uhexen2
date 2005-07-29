void trap_death_fireball_use ();
void trap_death_fireball_wait ();
	
void death_fireball_touch ()
{
	local float damg;
	
	sound (self, CHAN_WEAPON, "misc/combust.wav", 1, ATTN_NORM);
	
	damg = (12 + random(1,10));
	
	if (other.health)
		T_Damage (other, self, self.owner, damg );
	
	T_RadiusDamage (self, self.owner, damg, other);

	self.origin = self.origin - 8*normalize(self.velocity);
	
	remove(self);
}

void trap_death_fireball_wait ()
{
	self.think = trap_death_fireball_use;
	thinktime self : random(0.1, 2);
}

void death_fireball_think ()
{
	if ((vlen(self.velocity) < 50))
	{
		dprint("KABLOEY!\n");
		MeteoriteFizzle();
		return;
	}

	particle4(self.origin,75,random(272,288),PARTICLETYPE_FIREBALL,random(5,10));
	self.think=death_fireball_think;
	thinktime self : 0.1;
}

void trap_death_fireball_use ()
{
	entity fireball;
	vector vec;

	makevectors(self.angles);
	vec = v_up;
	vec_x += random(-0.2,0.2);
	vec_y += random(-0.2,0.2);
	
	sound (self, CHAN_WEAPON, "raven/littorch.wav", 1, ATTN_NORM);
	fireball = spawn ();
	fireball.movetype = MOVETYPE_BOUNCE;
	fireball.solid = SOLID_BBOX;
	fireball.speed=random(600, 1000);
	fireball.velocity=vec*fireball.speed;
	fireball.touch = death_fireball_touch;
	fireball.dmg=self.dmg;
	fireball.owner = self;
	fireball.angles = vectoangles (fireball.velocity);
	fireball.think = death_fireball_think;
	thinktime fireball : 0.1;
	self.last_attack=time;
	setmodel (fireball, "models/dthball.mdl");
	setsize (fireball, '0 0 0', '0 0 0');     
	setorigin (fireball,self.origin);
}


/*QUAKED trap_death_fireball (1 0.3 0) (0 0 0) (16 16 16)
-------------------------FIELDS-------------------------
.wait = How long to wait between firings (default 0.5)
.dmg = How much damage to do with each shot (default 10)
--------------------------------------------------------
*/
void () trap_death_fireball =
{
	precache_sound("imp/fireball.wav");
	precache_model("models/dthball.mdl");
	if(!self.wait)
		self.wait=0.5;
	if(!self.dmg)
		self.dmg=10;
	self.use = trap_death_fireball_wait;
};

 
