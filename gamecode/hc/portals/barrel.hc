/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/barrel.hc,v 1.2 2007-02-07 16:59:29 sezero Exp $
 */
/*
==============================================================================

BARRELS

==============================================================================
*/


$cd \art\models\objects\barrel\final
$base base 128 128
$skin skin
$frame resting

void barrel_go ();
void spawn_rep_barrel (vector org)
{
	entity replacement;
	replacement=spawn();
	replacement.spawnflags=self.spawnflags;
	replacement.origin=self.wallspot;
	replacement.frags=self.frags;
	replacement.classname=self.netname;
	replacement.think=barrel_go;
	replacement.flags2(+)FL_SUMMONED;
	replacement.targetname=self.targetname;
	thinktime replacement : 0.5;
}

//void	MakeExplosion(string explodemodel);
void()barrel_check_float;

/*void obj_barrel_gravity()
{
	local vector slope;

	if(self.ltime <= time)
	{
		slope = getslope(self);
		if(slope != self.dest)
		{
			self.speed		= trace_plane_normal_z * 20;
			self.ideal_yaw	= 
			self.dest		= trace_plane_normal;
		}
		self.ltime = time + 0.4;
	}

	thinktime self : 0.05;
}
*/

void float(void)
{
float x_mod, y_mod, z_mod;
vector org;

	org=self.origin;
	if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA)
	{
		if(self.velocity_x)
			self.velocity_x/=1.1;
		if(self.velocity_y)
			self.velocity_y/=1.1;

		org_z+= self.maxs_z*0.77;//float only 23% above waterlevel
		if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA)
		{
//			self.flags(+)FL_SWIM;
			self.flags(-)FL_ONGROUND;
			if(self.velocity_z<77)
				self.velocity_z=80;
			else
				self.velocity_z+=random(0,0.01);
		}
		else
		{
			self.velocity_z-=random(0,0.01);
		}
	
		if(random()<0.3)
		{
			y_mod=random(-0.15,0.15);
			if(random()<0.5)
				self.angles_y+=y_mod;
			else
				self.angles_y+=y_mod;
		}
	
		if(random()<0.3)
		{
			x_mod=random(-0.15,0.15);
			if(fabs(self.angles_x+x_mod)>10)
				self.angles_x-=x_mod;
			else
				self.angles_x+=x_mod;
		}
	
		if(random()<0.3)
		{
			z_mod=random(-0.15,0.15);
			if(fabs(self.angles_z+z_mod)>10)
				self.angles_z-=z_mod;
			else
				self.angles_z+=z_mod;
		}
		thinktime self : 0.1;
	}
	else
	{
		self.angles_z=self.angles_z=0;
		self.classname="barrel";
		self.think=barrel_check_float;
		thinktime self : 0.5;
	}
}

void barrel_check_float (void)
{
vector org;
	org=self.origin;
	if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA&&(!self.spawnflags&BARREL_SINK))
	{
		self.classname=="barrel_floating";
		self.think=float;
		thinktime self : 0;
	}
	else
	{
		self.classname="barrel";
		self.think=barrel_check_float;
		thinktime self : 0.5;
	}
}

/*
 * obj_barrel_explode() -- Blows the barrel up.
 */

void obj_barrel_explode_go()
{
entity attacker;

	if(self.enemy.flags2&FL_ALIVE)//give credit to person who blew it up
		attacker=self.enemy;
	else
		attacker=self;

	T_RadiusDamage(self, attacker, 100, self);

	sound(self, CHAN_VOICE, "weapons/explode.wav", 1, ATTN_NORM);

	particleexplosion(self.origin + '0 0 83',384,60,40);

	starteffect(CE_LG_EXPLOSION , (self.absmin+self.absmax)*0.5);

	if(self.spawnflags&BARREL_RESPAWN)
		spawn_rep_barrel(self.wallspot);

	chunk_death();
}

void obj_barrel_explode ()
{
	//delay so too many don't cause crash
	self.th_die=SUB_Null;
	self.takedamage = DAMAGE_NO;

	self.think=obj_barrel_explode_go;
	thinktime self : random()+0.05;
}

void obj_barrel_use_explode ()
{//no delay
	self.th_die=SUB_Null;
	self.takedamage = DAMAGE_NO;

	self.think=obj_barrel_explode_go;
	thinktime self : 0;
}

void()monster_rat;
void rat_spawn(float offset)
{
	newmis=spawn();
	newmis.angles_y=self.angles_y+offset*60;
	newmis.flags2(+)FL_SUMMONED;
	if(self.target)
		newmis.target=self.target;
	makevectors(newmis.angles);
	setorigin(newmis,self.origin+v_forward*16);
	newmis.think=monster_rat;
	thinktime newmis : 0;
}

void barrel_die ()
{
	self.solid=SOLID_NOT;
	if(random()<0.3||self.target!=""||self.classname=="monster_ratnest")
	{
	float r;
		r=rint(random(3,6));
		while(r>0)
		{
			rat_spawn(r);
			r-=1;
		}
	}
	if(self.spawnflags&BARREL_RESPAWN)
		spawn_rep_barrel(self.wallspot);
	if(self.classname!="monster_ratnest")
		chunk_death();
	else
		remove(self);
}

/*
 * obj_barrel_slide() -- Slides a barrel in the direction given in self.movedir.
 */
/*
void obj_barrel_slide()
{
	local entity victim;
	local float  direction;
//old if function, direction was null
//	if(walkmove(direction, self.count) == FALSE)
	if(!walkmove(self.cnt, self.count, FALSE))
		{
//Is this supposed to push something else it hits?
		victim = findradius(self.origin, self.count + 38);
		while(victim)
			{
			if(victim.movetype != MOVETYPE_NONE && victim != self)
				{
				direction = vectoyaw(victim.origin - self.origin);
				if(self.cnt <= 60 && direction >= 300)
					direction -= 360;
				if(direction > self.cnt - 60 &&
				   direction < self.cnt + 60)
					victim.velocity += (victim.origin - self.origin) * self.count;
				}
			victim = victim.chain;
			}
		}

//	if(!self.spawnflags & BARREL_DOWNHILL)
//		self.count -= 0.7;
//	if(self.count < 0.5)
//		self.think = obj_barrel_gravity;
//What is this supposed to do?
//	obj_barrel_gravity();
//	reduce self.count to simulate friction, right?
	self.count = self.count - 1;
	if(self.count<=0)
	{
		self.think=SUB_Null;
		self.nextthink = -1;
	}
	else
		thinktime self : 0.05;
}
*/

void obj_barrel_roll (void)
{
	self.v_angle_y=self.angles_y;
	self.v_angle_x=self.v_angle_z=0;
	makevectors(self.v_angle);
	self.velocity-=self.movedir*self.speed;
	self.movedir=normalize(v_forward);
	self.speed/=1.01;
	self.anglespeed=self.speed/7.7;
	if(self.speed<1&&self.speed>-1)
	{
		self.velocity = '0 0 0';
		self.think = SUB_Null;
		self.nextthink = -1;
	}
	else
	{
	vector dircheck;
//	float hitcheck;
//Rolling sound
		if(self.flags&FL_ONGROUND)
		{
			self.flags(-)FL_ONGROUND;
			self.last_onground=time;
			self.level=TRUE;
		}
		else if(self.level)
		{
			self.level=FALSE;
			self.last_onground = time - 1;
			self.speed/=2;//slow down if go off cliff, looks weird otherwise
		}
		

		self.velocity+=self.movedir*self.speed;
		self.angles_x-=self.anglespeed;
		self.think=obj_barrel_roll;
		thinktime self : 0.05;

		dircheck=normalize(self.velocity);
		makevectors(self.angles);
		traceline(self.origin,self.origin+dircheck*39,FALSE,self);
		if(trace_fraction==1)
		{
			traceline(self.origin+v_right*16,self.origin+v_right*16+dircheck*39,FALSE,self);
			if(trace_fraction==1)
				traceline(self.origin-v_right*16,self.origin-v_right*16+dircheck*39,FALSE,self);
		}
		if(trace_fraction<1)
		{
			sound(self,CHAN_AUTO,"fx/thngland.wav",1,ATTN_NORM);		// landing thud
			self.speed*=-0.25;
			self.last_onground=time - 1;
			obj_fly_hurt(trace_ent);
			self.velocity=dircheck*self.speed;
		}
	}			
}


/*
 * obj_barrel_shoot() -- Called when a barrel is shot.
 */
void obj_barrel_shoot()
{
	sound(self,CHAN_AUTO,"fx/thngland.wav",1,ATTN_NORM);		// landing thud

//	if(self.aflag)
//		return;
/*
	self.movedir = normalize(self.origin - self.enemy.origin);
	self.velocity=self.velocity + self.movedir*self.count;
	self.avelocity_y=random(100,300);
	self.velocity_z=self.velocity_z + 50;
	if(self.velocity_z>150)
		self.velocity_z=150;
	self.flags(-)FL_ONGROUND;
*/
}


/*
 * obj_barrel_use() -- Called when a barrel is triggered.
 */

void barrel_drop ()
{
	self.movetype=MOVETYPE_FLYMISSILE;
//	setorigin(self,self.origin - '0 0 20');
	self.flags(-)FL_PUSH|FL_ONGROUND;
	self.velocity='0 0 -200';
	self.touch=self.th_die;
}

void obj_barrel_use()
{
//	if(other.movetype == MOVETYPE_STEP)
//		obj_pull();
//	else
		obj_barrel_explode();
}


void spawn_barrel(float barrel_type)
{
	self.frags=barrel_type;
	if(!self.flags2&FL_SUMMONED)
	{
		precache_model("models/barrel.mdl");
		if(barrel_type==BARREL_NORMAL)
			precache_model("models/rat.mdl");
	}

	CreateEntityNew(self,ENT_BARREL,"models/barrel.mdl",chunk_death);

	if(self.spawnflags&ON_SIDE)
	{
		self.aflag=1;
		self.frame=1;
		self.v_angle=self.angles;
		setsize(self, '-18 -13 -13','18 13 13');
		self.hull=HULL_CROUCH;
	}
	else
		setsize(self, '-13 -13 0','13 13 36');

	if (self.scale)
		self.mass *=self.scale;

	self.netname=self.classname;
	self.classname = "barrel";

	self.flags(+)FL_PUSH;
	self.touch		= obj_push;
	self.th_pain	= obj_barrel_shoot;

	if (barrel_type==BARREL_NORMAL)
	{
		self.th_die	= barrel_die;
		self.skin=0;
	}
	else if (barrel_type==BARREL_INDESTRUCTIBLE)
	{
		self.health	= 999999;
		self.th_die	= SUB_Null;
		self.skin=1;
	}
	else if (barrel_type==BARREL_EXPLODING)
	{
		self.th_die	= obj_barrel_explode;
		self.skin=2;
	}

	if(!self.spawnflags&BARREL_SINK)
	{
		self.think=barrel_check_float;
		thinktime self : 0;
	}

	if(pointcontents(self.origin)!=CONTENT_EMPTY&&(!self.spawnflags&BARREL_SINK))
	{
		self.classname="barrel_floating";
		self.think=float;
		thinktime self : 0;
	}
	else if(!self.flags2&FL_SUMMONED&&!self.spawnflags&BARREL_NO_DROP)
		droptofloor();

	if(self.spawnflags&DROP_USE)
	{
		self.movetype=MOVETYPE_NONE;
		self.use=barrel_drop;
		if(barrel_type==BARREL_EXPLODING)
			self.th_die=obj_barrel_use_explode;
	}
	else if(self.targetname)
		if(barrel_type==BARREL_EXPLODING)
			self.use=obj_barrel_use_explode;
		else
			self.use=self.th_die;

	if(self.spawnflags&BARREL_RESPAWN)
		self.wallspot=self.origin;
}


/*QUAKED obj_barrel (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK DROP_USE RESPAWN

A barrel, just a plain old barrel
-------------------------FIELDS-------------------------
.health  - How hard it is to smash
           Default: 25
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Sinks in water
DROP_USE - Barrel has no gravity until used, then will drop
RESPAWN - Barrel will RESPAWN at it's initial origin when it is destroyed
--------------------------------------------------------
*/
void obj_barrel(void)
{
	if(self.spawnflags&DROP_USE)
		self.spawnflags(+)BARREL_NO_DROP;
	precache_sound("misc/squeak.wav");
	spawn_barrel(BARREL_NORMAL);
	self.mass = 75;
}


/*QUAKED obj_barrel_indestructible (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK DROP_USE RESPAWN

A barrel you just can't break
-------------------------FIELDS-------------------------
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Sinks in water
DROP_USE - Barrel has no gravity until used, then will drop
RESPAWN - Barrel will RESPAWN at it's initial origin when it is destroyed
--------------------------------------------------------
*/
void obj_barrel_indestructible(void)
{
	if(self.spawnflags&DROP_USE)
		self.spawnflags(+)BARREL_NO_DROP;
	spawn_barrel(BARREL_INDESTRUCTIBLE);
	self.mass = 95;
}


/*QUAKED obj_barrel_exploding (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK DROP_USE RESPAWN
An exploding barrel with red XXX on the side
WARNING!:  Putting too many exploding barrels next to each other will cause a crash, there is no way around this, so if it happens, it's to be considered a Designer error!  
Putting them in lines and chains seems to be ok, as long as you don't stack them or group them too closely, more than 4 in a tight group is probably pushing it.
-------------------------FIELDS-------------------------
.health  - How hard it is to blow up
           Default: 25
.targetname - set a targetname on a barrel and it will not delay when it explodes (default is arandom delay to prevent too many explosions at once)
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Sinks in water
DROP_USE - Barrel has no gravity until used, then will drop
RESPAWN - Barrel will RESPAWN at it's initial origin when it is destroyed
--------------------------------------------------------
*/
void obj_barrel_exploding(void)
{
	if(self.spawnflags&DROP_USE)
		self.spawnflags(+)BARREL_NO_DROP;
	spawn_barrel(BARREL_EXPLODING);
	self.mass = 85;
}


void barrel_go(void)
{
	if(self.classname=="obj_barrel_normal")
	{
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_NORMAL);
		self.mass = 75;
	}
	else if(self.classname=="obj_barrel_indestructible")
	{
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_INDESTRUCTIBLE);
		self.mass = 95;
	}
	else if(self.classname=="obj_barrel_exploding")
	{
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_EXPLODING);
		self.mass = 85;
	}
}

