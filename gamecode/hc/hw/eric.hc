void()trap_fireball_use;
void trap_fireball_wait ()
{
	self.use = trap_fireball_use;
	self.think=SUB_Null;
	self.nextthink=-1;
}

void fireball_think ()
{
	particle4(self.origin,3,random(160,176),PARTICLETYPE_FIREBALL,random(10,20));
	self.think=fireball_think;
	thinktime self : 0.1;
}

void trap_fireball_use ()
{
entity fireball;
vector vec;
float dot;

	if(self.spawnflags&1)
	{
		self.think=trap_fireball_wait;
		thinktime self : 0;
	}
	else
	{
		self.use=trap_fireball_wait;
	    self.think = trap_fireball_use;
		thinktime self : self.wait;
	}

	if(self.goalentity)
		self.enemy=self.goalentity;
	else
		self.enemy = find (world, classname, "player");

	if((visible(self.enemy)&&self.enemy!=world)||self.goalentity==self.enemy)
	{
		vec=normalize((self.enemy.absmin+self.enemy.absmax)*0.5-self.origin);
		makevectors(self.angles);
		dot = v_forward*vec;
		if(dot>0.6||self.goalentity==self.enemy)
		{
			sound (self, CHAN_WEAPON, "imp/fireball.wav", 1, ATTN_NORM);
			fireball = spawn ();
			fireball.movetype = MOVETYPE_FLYMISSILE;
			fireball.solid = SOLID_BBOX;
			fireball.speed=1000;
			fireball.velocity=vec*fireball.speed;
	        fireball.touch = fireballTouch;
			fireball.dmg=self.dmg;
			fireball.owner = self;
	        fireball.angles = vectoangles (fireball.velocity);
			fireball.think = fireball_think;
			thinktime fireball : 0.05;
			self.last_attack=time;
			setmodel (fireball, "models/drgnball.mdl");
			setsize (fireball, '0 0 0', '0 0 0');     
			setorigin (fireball,self.origin);
		}
	}
}


void locate_first_target ()
{
	self.goalentity=find(world,targetname,self.target);
	if(!self.goalentity)
		dprint("ERROR: Targeted Fireball can't find target\n");
}

/*QUAKED trap_fireball (1 0.3 0) (0 0 0) (16 16 16) TRIGGER_ONLY
New item for QuakeEd
It works!  It really works!
If TRIGGER_ONLY is turned on, it will fire once each time it's triggered (used)
otherwise, each time it's used, it's turned on or off.
If it's targetted to something, it will fire at that rather than tracking the player.
-------------------------FIELDS-------------------------
.wait = How long to wait between firings (default 0.5)
.dmg = How much damage to do with each shot (default 10)
--------------------------------------------------------
*/
void () trap_fireball =
{
	precache_sound2("imp/fireball.wav");
	precache_model2("models/drgnball.mdl");
	if(!self.wait)
		self.wait=0.5;
	if(!self.dmg)
		self.dmg=10;
	if(self.target)
	{
		self.think=locate_first_target;
		thinktime self : 0.5;
	}

	self.use = trap_fireball_use;
};

 
