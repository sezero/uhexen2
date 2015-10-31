/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/object.hc,v 1.3 2007-02-07 16:57:08 sezero Exp $
 */


float SPAWNFLAG_BALLISTA_TRACK = 1;

/*
 * obj_push() -- Allows players to push objects when they walk toward them.
 */
void() Missile_Arc;
void(float vol) sheep_sound;
void() obj_barrel_roll;
void()float;
void()sheep_trot;

void obj_fly_hurt (entity loser)
{//MG
//FIXME: Check for sky
//dprint("hit\n");
float magnitude,my_mass;
	if(self.frozen>0&&other.classname=="snowball")
		return;

	if(self.classname=="player")
		my_mass=self.mass;
	else if(!self.mass)
		my_mass = 1;
	else if(self.mass<=10)
		my_mass=10;
	else
		my_mass = self.mass/10;

	magnitude=vlen(self.velocity)*my_mass/10;
	if(pointcontents(self.absmax)==CONTENT_WATER)//FIXME: or other watertypes
		magnitude/=3;							//water absorbs 2/3 velocity

	if(self.classname=="barrel"&&self.aflag)//rolling barrels are made for impacts!
		magnitude*=3;

	if(self.frozen>0&&magnitude<300&&self.flags&FL_ONGROUND&&loser==world&&self.velocity_z<-20&&self.last_onground+0.3<time)
		magnitude=300;
/*
dprint("\n");
dprint(self.classname);
dprint(" hit-> ");
dprint(loser.classname);
dprint("\n");
dprint(vtos(self.velocity));
dprint("\n");
dprint("Magnitude: ");
dprint(ftos(magnitude));
dprint("\n");
dprint("Air time: ");
dprint(ftos(time-self.last_onground));
dprint("\n");
dprint("Time since last impact: ");
dprint(ftos(time-self.last_impact));
dprint("\n");
*/
  if(self.last_onground+0.3<time||(self.last_onground+0.1<time&&loser.thingtype>=THINGTYPE_GLASS))
  {
	vector dir1, dir2;
	float force,dot;
		if(loser.thingtype>=THINGTYPE_GLASS)
			magnitude*=2;

		if(magnitude>=100&&loser.takedamage&&loser.classname!="catapult"&&loser!=world)
		{

			dir1=normalize(self.velocity);
			if(loser.origin=='0 0 0')
				dir2=dir1;
			else
				dir2=normalize(loser.origin-self.origin);
	
			dot= dir1*dir2;
	
		    if(dot >= 0.2)
				force=dot;		
			else
				force=0;

			force*=magnitude/50;

			if(pointcontents(loser.absmax)==CONTENT_WATER||(self.classname=="barrel"&&self.aflag))//FIXME: or other watertypes
				force/=3;							//water absorbs 2/3 velocity

			if(self.flags&FL_MONSTER&&loser==world)
				force/=2;

			if(self.frozen>0&&force>10)
				force=10;

			if((force>=1&&loser.classname!="player")||force>=10)
			{
/*				dprint("Damage other (");
				dprint(loser.classname);
				dprint("): ");
				dprint(ftos(force));
				dprint("\n");
*/				T_Damage (loser, self, self, force);  
			}
		}

		if(self.classname!="monster_mezzoman"&&self.netname!="spider")//Cats always land on their feet
			if((magnitude>=100+self.health&&self.classname!="player")||magnitude>=700)//health here is used to simulate structural integrity
			{
				if(self.classname=="player"&&self.flags&FL_ONGROUND&&magnitude<1000)
				{
					//allow for some lenience on high falls
					magnitude/=2;
					if(self.absorb_time>=time)//crouching on impact absorbs 1/2 the damage
						magnitude/=2;
				}
				magnitude/=40;
				magnitude=magnitude - force/2;//If damage other, subtract half of that damage off of own injury
				if(magnitude>=1)
				{
//FIXME: Put in a thingtype impact sound function
/*					dprint("Damage self (");
					dprint(self.classname);
					dprint("): ");
					dprint(ftos(magnitude));
					dprint("\n");
*/
					if(self.classname=="player_sheep"&&self.flags&FL_ONGROUND&&self.velocity_z>-50)
						return;
					T_Damage(self,world,world,magnitude);
				}
			}

	self.last_impact=time;
	if(self.flags&FL_ONGROUND)
		self.last_onground=time;
	}
}

void obj_push()
{//MG
vector pushdir,pushangle;
float ontop,pushed,inertia,force,walkforce;

	if(other.solid==SOLID_PHASE||other.movetype==MOVETYPE_FLYMISSILE||other.movetype==MOVETYPE_BOUNCEMISSILE)
		return;

	if(self.classname=="barrel"&&pointcontents(self.origin)!=CONTENT_EMPTY&&(!self.spawnflags&BARREL_SINK))
	{
		self.classname="barrel_floating";
		self.think=float;
		self.nextthink=time;
	}

	if(self.classname=="player_sheep"&&other.classname=="catapult")
		self.spawnflags(+)1;	//Sheep won't move once on catapult

	if(self.last_impact + 0.1<=time)
		obj_fly_hurt(other);

//	if (other.classname != "player"&&!(other.flags&FL_MONSTER)&&(!other.flags&FL_PUSH)&&other.movetype!=MOVETYPE_PUSHPULL)
//		return;

	if(other!=world&&other.absmin_z >= self.origin_z+self.maxs_z - 5&&other.velocity_z<1)
	{		
		if(!other.frozen&&
			(
			 (!other.flags2&FL_ALIVE&&other.flags&FL_MONSTER)||
			 (self.flags&FL_MONSTER&&self.model!="models/spider.mdl"&&self.model!="models/scorpion.mdl")
			)
		  )
		{
			makevectors(other.angles);
			v_forward_z=1;
			other.velocity=v_forward*300;
			other.flags(-)FL_ONGROUND;
		}
		if(other.flags&FL_CLIENT&&!other.frozen)
			ontop = FALSE;
		else
		{
			ontop=TRUE;
//			other.flags(+)FL_ONGROUND;
		}
	}

	if(self.flags&FL_MONSTER)
	{
		if(other!=world&&self.absmin_z >= other.absmax_z - 3&&self.velocity_z<1&&other.movetype!=MOVETYPE_FLYMISSILE&&other.movetype!=MOVETYPE_BOUNCE&&other.movetype!=MOVETYPE_BOUNCEMISSILE)
			self.flags(+)FL_ONGROUND;
		if(self.frozen<=0&&!self.artifact_active&ARTFLAG_STONED)
			return;
	}

	if(!other.velocity)
		return;

	if (self.impulse == 20)
		return;

	if(!self.mass)
		inertia=1;
	if(self.mass<=30)
		inertia=self.mass/3;
	else
		inertia=self.mass/33;

	if(other.strength)
		force=vlen(other.velocity)*(other.strength/40+0.5);
	else
		force=vlen(other.velocity);

	if(pointcontents(self.origin)==CONTENT_WATER||pointcontents(self.origin)==CONTENT_SLIME||pointcontents(self.origin)==CONTENT_LAVA)
		force/=3;

//FIXME: mass should determine how fast the object can be pushed
//	if (self.mass > 199*force)	// Too heavy to move
	if (self.mass >= 1000)	// Too heavy to move
		return;

//So you can push frozen guys around before they melt
	if(self.frozen>0)
	{
		self.freeze_time=time+10;
		self.wait=time + 10;
	}

	if(ontop)
		return;	

	walkforce=force/inertia/40;//20 is the frame time...
	if(self.classname=="barrel"&&self.aflag)
	{
		vector dir1, dir2;
		float dot_forward,dot_right;
		self.angles_z=0;
		self.v_angle_x=self.v_angle_z=0;
		makevectors(self.v_angle);

		if(ontop)
			dir1=normalize(other.velocity);
		else
			dir1=normalize(self.origin-other.origin);

		dir2=normalize(v_forward);
		dir1_z=dir2_z=0;

		dot_forward= dir1*dir2;

		self.enemy=other;
	    if(dot_forward>=0.9)
		{
			self.movedir=dir2;//test
			self.movedir_z=0;
			self.speed += force/inertia;
			if(self.speed>other.strength*300)
				self.speed=other.strength*300;
			traceline(self.origin,self.origin+dir2*48,FALSE,self);
			if(trace_fraction==1.0)
			{
				self.velocity=self.movedir*self.speed;
				self.think=obj_barrel_roll;
				self.nextthink=time;
			}
		}
		else if(dot_forward<=-0.9)
		{
			self.movedir=dir2;//test
			self.movedir_z=0;
			self.speed += force/inertia*-1;
			if(self.speed<other.strength*-300)
				self.speed=other.strength*-300;
			traceline(self.origin,self.origin+dir2*-48,FALSE,self);
			if(trace_fraction==1.0)
			{
				self.velocity=self.movedir*self.speed;
				self.think=obj_barrel_roll;
				self.nextthink=time;
			}
		}
		else
		{
			dir1=normalize(other.velocity);
			dir2=normalize(v_right);
			dot_right= dir1*dir2;
		    if(dot_right >0.2)
			{
			    if(dot_forward >0.1)
				{
					self.angles_y-=walkforce*10;
				}
				else if(dot_forward<-0.1)
				{
					self.angles_y+=walkforce*10;
				}
			}
			else if(dot_right<-0.2)
			{
			    if(dot_forward >0.1)
				{
					self.angles_y+=walkforce*10;
				}
				else if(dot_forward<-0.1)
				{
					self.angles_y-=walkforce*10;
				}
			}
			self.v_angle_y=self.angles_y;
		}
	}
	else
	{
		pushdir=normalize(other.velocity);
		pushangle=vectoangles(pushdir);

		pushed=FALSE;
		walkforce=force/inertia/20;//20 is the frame time...
		if(!walkmove(pushangle_y,walkforce,FALSE))//FIXME: check mass
		{
			if(other.absmax_z<=self.origin_z+self.mins_z*0.75)
				pushdir_z*=2;
			self.velocity=(pushdir*force*2*(1/inertia)+self.velocity)*0.5;
			if(self.flags&FL_ONGROUND)
			{
				if(self.velocity_z<0)
					self.velocity_z=0;
				self.flags-=FL_ONGROUND;
			}
			if(self.velocity)
				pushed=TRUE;
		}
		else
			pushed=TRUE;
	
		if(pushed&&self.classname!="barrel_floating")
		{
			if(self.pain_finished<=time)
			{
				if(self.classname=="player_sheep")
				{
					sheep_sound(.75);
					if(!infront(other)&&random()<0.5)
//FIXME- find current think and set transition to run
						self.think=sheep_trot;
				}
				else if(self.thingtype==THINGTYPE_WOOD)
				{
					sound(self,CHAN_VOICE,"misc/pushwood.wav",1,ATTN_NORM);		
					self.pain_finished=time + 1.041;
				}
				else if ((self.thingtype==THINGTYPE_GREYSTONE) || (self.thingtype==THINGTYPE_BROWNSTONE))
				{
					sound(self,CHAN_VOICE,"misc/pushston.wav",1,ATTN_NORM);
					self.pain_finished=time + .711;
				}
				else if(self.thingtype==THINGTYPE_METAL)
				{
					sound(self,CHAN_VOICE,"misc/pushmetl.wav",1,ATTN_NORM);
					self.pain_finished=time + .835;
				}
			}
		}
	}
}



/*QUAKED obj_chair (0.3 0.1 0.6) (-10 -10 -5) (10 10 40)
A wooden chair. 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void obj_chair()
{
	precache_model("models/chair.mdl");

	CreateEntityNew(self,ENT_CHAIR,"models/chair.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
}


/*QUAKED obj_barstool (0.3 0.1 0.6) (-10 -10 -5) (10 10 32)
A bar stool - Drinks on the house!
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void obj_barstool()
{
	precache_model3("models/stool.mdl");

	CreateEntityNew(self,ENT_BARSTOOL,"models/stool.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
}


/*QUAKED obj_tree (0.3 0.1 0.6) (-42 -42 0) (42 42 160)
A tree that has no leaves
-------------------------FIELDS-------------------------
health : 1000
--------------------------------------------------------
*/
void obj_tree()
{
	precache_model2("models/tree.mdl");
	CreateEntityNew(self,ENT_TREEDEAD,"models/tree.mdl",chunk_death);
}

void tree2_death (void)
{
	self.owner.nextthink = time + .01;
	self.owner.think = chunk_death;
	chunk_death();
}

/*QUAKED obj_tree2 (0.3 0.1 0.6) (-140 -140 -16) (140 140 220)
A tree with a round top of leaves 
-------------------------FIELDS-------------------------
health : 1000
--------------------------------------------------------
*/
void obj_tree2()
{
	entity top;

	precache_model("models/tree2.mdl");
	CreateEntityNew(self,ENT_TREE,"models/tree2.mdl",tree2_death);

	top = spawn();
	top.scale = self.scale;

	CreateEntityNew(top,ENT_TREETOP,top.model,tree2_death);

	top.origin = self.origin;

	if (self.scale)		// Move top according to scale
		top.origin_z += top.scale * 104;
	else 
		top.origin_z += 104; 

	top.health = self.health;
	top.classname = "tree2top";

	top.owner = self;
	self.owner = top;

}

/*QUAKED obj_bench (0.3 0.1 0.6) (-30 -30 0) (30 30 40)
A wooden bench 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void obj_bench()
{
	precache_model3("models/bench.mdl");
	CreateEntityNew(self,ENT_BENCH,"models/bench.mdl",chunk_death);

	self.touch	= obj_push;
}


/*QUAKED obj_cart (0.3 0.1 0.6) (-36 -32 -10) (36 75 64)
A cart 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void obj_cart()
{
	precache_model("models/cart.mdl");
	CreateEntityNew(self,ENT_CART,"models/cart.mdl",chunk_death);
	self.hull=HULL_SCORPION;

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
}


/*QUAKED obj_chest1 (0.3 0.1 0.6) (-16 -16 0) (16 16 32)
A treasure chest
-------------------------FIELDS-------------------------
skin - 0 - generic texture (default)
       1 - roman texture
--------------------------------------------------------
*/
void obj_chest1()
{
	precache_model("models/chest1.mdl");
	CreateEntityNew(self,ENT_CHEST1,"models/chest1.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
}


/*QUAKED obj_chest2 (0.3 0.1 0.6) (-16 -16 0) (16 16 32)
A treasure chest on legs
-------------------------FIELDS-------------------------
skin - 0 - generic texture (default)
       1 - meso texture
       2 - egypt texture
--------------------------------------------------------
*/
void obj_chest2()
{
	precache_model3("models/chest2.mdl");
	CreateEntityNew(self,ENT_CHEST2,"models/chest2.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;

}

/*QUAKED obj_chest3 (0.3 0.1 0.6) (-16 -16 0) (16 16 32)
A treasure chest on legs
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void obj_chest3()
{
	precache_model2("models/chest3.mdl");
	CreateEntityNew(self,ENT_CHEST3,"models/chest3.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;

}
/*
void boulder_fall (void)
{
	if(!self.flags&FL_ONGROUND||self.frags<time)
	{
		self.movetype=MOVETYPE_BOUNCE;
		if(self.velocity_z>=0)
			self.velocity_z-=10;
	}

	self.think=boulder_fall;
	self.nextthink=time+0.1;
}

void boulder_push()
{
	if(self.velocity)
		self.avelocity=self.velocity*-1;
	obj_fly_hurt(other);
	if(other.movetype&&other.velocity!='0 0 0'&&other.solid!=SOLID_TRIGGER&&other.solid!=SOLID_PHASE&&other.solid)
	{
		if(!walkmove(other.angles_y,1,TRUE))
		{
		dprint("can't walk\n");
		self.velocity=(other.velocity+self.velocity)*0.5;
		self.frags=time+0.01;
		}
		if(self.flags&FL_ONGROUND)
		{
			self.movetype=MOVETYPE_BOUNCEMISSILE;//movetype_slide- no friction
			self.flags-=FL_ONGROUND;
		}
	}
}
*/
/*QUAK-ED obj_boulder (0.3 0.1 0.6) (-32 -32 -32) (32 32 32)
A big freaking rock
-------------------------FIELDS-------------------------
health = 75
mass = 25
--------------------------------------------------------
*/
/*
void obj_boulder (void)
{
	precache_model2("models/boulder.mdl");
	CreateEntityNew(self,ENT_BOULDER,"models/boulder.mdl",chunk_death);

	self.flags (+) FL_PUSH;
	self.touch	= obj_push;

//	self.think=boulder_fall;
//	self.nextthink=time+0.1;
}
*/

/*QUAKED obj_sword (0.3 0.1 0.6) (-16 -16 -8) (16 16 8)
A sword 
-------------------------FIELDS-------------------------
health = 50
--------------------------------------------------------
*/
void obj_sword (void)
{
	precache_model("models/sword.mdl");
	CreateEntityNew(self,ENT_SWORD,"models/sword.mdl",chunk_death);

	if(self.targetname)
		self.use=chunk_death;
}

/*
void BalBoltStick (void)
{
vector dir;
		self.velocity='0 0 0';
		self.movetype=MOVETYPE_NONE;
		self.solid=SOLID_BBOX;
		self.takedamage=DAMAGE_YES;
		if (!self.health)
			self.health=10;
		self.th_die=chunk_death;
		makevectors(self.angles);
		dir=normalize(v_forward);
		if(pointcontents(self.origin+dir*24)!=CONTENT_SOLID)
			remove(self);
}
*/
void BalBoltTouch (void)
{
	if(other.takedamage)
	{
//FIXME: Sound
	vector dir;
		if(other!=self.goalentity&&self.velocity!='0 0 0')
		{
			self.goalentity=other;
			dir=normalize(self.velocity);
			traceline(self.origin-dir*25,self.origin+dir*25,FALSE,self);
			if(other.thingtype==THINGTYPE_FLESH)
				MeatChunks (trace_endpos,self.velocity*0.5+'0 0 200', 3,trace_ent);
			SpawnPuff (trace_endpos, self.velocity*0.5+'0 0 200', self.dmg,trace_ent);
			T_Damage(other,self,self.owner.enemy.enemy,self.dmg);
		}
		self.think=chunk_death;
		thinktime self : 0.2;
		if(other.solid==SOLID_BSP||normalize(self.velocity)!=self.movedir)
			chunk_death();
	}
	else
		chunk_death();
/*
		self.touch=SUB_Null;
		self.think=BalBoltStick;
		self.nextthink=time+0.1;
	}
	else
	{
//FIXME: Sound
	vector dir;
		makevectors(self.angles);
		dir=normalize(v_forward);
		self.dmg/=1.2;
		if(self.movetype!=MOVETYPE_BOUNCE)
			if(pointcontents(self.origin+dir*52)==CONTENT_SOLID)
			{
				setorigin(self,self.origin+dir*8);
				self.avelocity=self.velocity='0 0 0';
				self.touch=SUB_Null;
				self.movetype=MOVETYPE_NONE;
				self.dmg=0;
				self.think=SUB_Remove;
				self.nextthink=time + 2;
			}
			else
			{
				self.movetype=MOVETYPE_BOUNCE;
				self.avelocity=RandomVector('360 360 360');
			}
	}
*/
}

void FireBalBolt (void)
{
vector org;

	newmis=spawn();
	newmis.owner=self;
	makevectors(self.angles);
	org=self.origin+self.proj_ofs+v_forward*10;
	if(self.spawnflags&SPAWNFLAG_BALLISTA_TRACK)
	{
		newmis.velocity=normalize((self.enemy.absmax+self.enemy.absmin)*0.5-org)*1000;
	}
	else
	{
		newmis.velocity=normalize(self.view_ofs-org)*1000;
		traceline(org,org+newmis.velocity,FALSE,self);
		if(trace_ent!=self.goalentity&&self.goalentity.health)
			newmis.velocity=normalize((self.goalentity.absmin+self.goalentity.absmax)*0.5-org)*1000;
/*		if(deathmatch)
		{
			newmis.think=Missile_Arc;
			newmis.nextthink=time+0.2;
		}
*/
	}
	newmis.movedir=normalize(newmis.velocity);
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_PHASE;
	newmis.thingtype=THINGTYPE_WOOD;
	newmis.touch=BalBoltTouch;
	newmis.angles=vectoangles(newmis.velocity);
	newmis.avelocity_z=500;
	newmis.dmg=self.dmg;
	newmis.goalentity=newmis;

	setmodel(newmis,"models/balbolt.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,org);
}

void()ballista_think;
void() ballista_fire = [++ 1 .. 30 ]
{
	if (self.frame == 2)
		sound (self, CHAN_WEAPON, "weapons/ballista.wav", 1, ATTN_NORM);
	else if (self.frame==4)
		FireBalBolt();
	else if (self.frame == 15)
		sound (self, CHAN_WEAPON, "weapons/ballwind.wav", 1, ATTN_NORM);

	if(cycle_wrapped)
	{
		self.frame=0;
		self.last_attack=time;
		if(self.spawnflags&SPAWNFLAG_BALLISTA_TRACK)
			self.think=ballista_think;
		else
			self.think=SUB_Null;
//			return;
//		else if(self.oldthink)
//		{
//			self.think=self.oldthink;
//			self.nextthink=time+1;
//		}
	}
};

void ballista_think()
{
	entity targ;
	float pitchmod,checklooped,bestdist,lastdist;
	vector my_pitch, ideal_pitch;

	if(!self.enemy||!visible(self.enemy)||!self.enemy.flags2&FL_ALIVE&&!self.enemy.artifact_active&ART_INVISIBILITY&&!self.enemy.artifact_active&ART_INVINCIBILITY)
	{
//		dprint("looking\n");
		self.enemy=targ = world;
		bestdist=9999;
		while(!checklooped)
		{	
			targ = find (targ, classname, "player");
			if(visible(targ)&&targ.flags2&FL_ALIVE&&!targ.artifact_active&ART_INVISIBILITY&&!targ.artifact_active&ART_INVINCIBILITY)
			{
				lastdist=vlen(targ.origin-self.origin);
				if(lastdist<bestdist)
				{
//					dprint("acquired one\n");
					bestdist=lastdist;
					self.enemy=targ;
				}
			}
			if(targ==world)
				checklooped=TRUE;
		}
	}

	if (self.enemy)
	{
//		dprint("tracking\n");
//Yaw
		enemy_yaw = vectoyaw(self.enemy.origin - self.origin);

		ai_attack_face();
//Pitch
		makevectors(self.angles);	
		my_pitch=normalize(v_forward);
		ideal_pitch=normalize(self.enemy.origin-self.origin);
		ideal_pitch=vectoangles(ideal_pitch);
		if(ideal_pitch_z>my_pitch_z)
		{
			if(ideal_pitch_z-my_pitch_z>self.count)
				pitchmod=self.count;
			else
				pitchmod=ideal_pitch_z-my_pitch_z;
			self.angles_z+=pitchmod;
		}
		else if(ideal_pitch_z<my_pitch_z)
		{	
			if(my_pitch_z-ideal_pitch_z>self.count)
				pitchmod=self.count;
			else
				pitchmod=my_pitch_z-ideal_pitch_z;
			self.angles_z-=pitchmod;
		}
		if(self.last_attack+self.speed<time)
			if(visible(self.enemy))
				if(infront(self.enemy))
				{
					if(random()<0.2)
						ballista_fire();
				}
		
	}
	self.nextthink = self.ltime + 0.1;
	self.think = ballista_think;
}
		
/*QUAKED obj_ballista (0.3 0.1 0.6) (-45 -45 0) (45 45 60) SPAWNFLAG_BALLISTA_TRACK
A ballista which is animated to shoot an arrow
-------------------------FIELDS-------------------------
health = default = 0 (indestructable)
cnt    = degrees of pitch off the starting point pos & neg (default = 30)
count  = degrees per movement (default = 5)
dmg	   = amount of damage projectile will do (default  = 50)
speed  = delay, in seconds, between firings (higher number means longer wait) (default = 5)
--------------------------------------------------------
*/
void obj_ballista (void)
{
	precache_model("models/ballista.mdl");
	precache_model("models/balbolt.mdl");
	precache_sound ("weapons/ballista.wav");	// firing
	precache_sound ("weapons/ballwind.wav");	// ballista winding back to fire

	if(!self.mass)
		self.mass=1000;

	CreateEntityNew(self,ENT_BALLISTA,"models/ballista.mdl",chunk_death);
	self.hull=HULL_SCORPION;

	if (!self.cnt) 
	  self.cnt = 30;

	if (!self.count)
	  self.count = 5;

	if(!self.health)
		self.takedamage=DAMAGE_NO;

	if(!self.dmg)
		self.dmg = 50;

	if(!self.speed)
		self.speed = 5;

	self.oldorigin = self.angles;

	if (self.spawnflags & SPAWNFLAG_BALLISTA_TRACK)
	{
		self.yaw_speed = self.count;
		self.th_missile = ballista_fire;
		self.think = ballista_think;
		self.nextthink = time + 0.5;
		self.last_attack=time+0.5;
	}

	self.th_weapon = ballista_fire;
	self.view_ofs=self.proj_ofs='0 0 48';
//	self.use = ballista_use;
}


// Knocks you back if in it's way 
void bell_attack(float anim)
{
	vector spot1,spot2,bell_angle;

	if (!anim)
		bell_angle = self.v_angle;
	else
	{	
		bell_angle = self.v_angle;
		bell_angle_y = self.v_angle_y - 180;
	}		

	makevectors (bell_angle);
	spot1 = self.origin + '0 0 -180';  // from the bottom of the bell	
	spot2 = spot1 + v_forward * 150;			

	tracearea (spot1,spot2,'-80 -80 0','80 80 64',FALSE,self);

	if (trace_fraction == 1.0)  
		return;
	
	if (trace_ent.takedamage)
	{
		sound (self, CHAN_WEAPON, "weapons/gauntht1.wav", 1, ATTN_NORM);

		if (!trace_ent.movetype == MOVETYPE_NONE)
		{
			if(trace_ent.flags&FL_ONGROUND)
				trace_ent.flags = trace_ent.flags - FL_ONGROUND;
			trace_ent.velocity =  trace_ent.origin - self.origin;
			trace_ent.velocity_z = 280;
		}
	}
}

void bell_smallring (void) [++ 31 .. 51]
{
	self.nextthink = time + HX_FRAME_TIME + HX_FRAME_TIME + HX_FRAME_TIME;

	if (cycle_wrapped)
	{
		self.frame = 0;
		self.nextthink = time - 1;
		self.think = SUB_Null;
	}
}

void bell_bigring (void) [++ 0 .. 30]
{
	self.nextthink = time + HX_FRAME_TIME + HX_FRAME_TIME + HX_FRAME_TIME;

	if (self.frame == 2) 
	{
		sound (self, CHAN_VOICE, "misc/bellring.wav", 1, ATTN_IDLE);
		bell_attack(0);
	}

	if (self.frame == 8)
	{
		sound (self, CHAN_VOICE, "misc/bellring.wav", 1, ATTN_IDLE);
		bell_attack(1);
	}

	if (cycle_wrapped)
	{
		self.frame = 0;
		self.nextthink = time - 1;
		self.think = SUB_Null;
	}
}

void bell_ring (void)
{

	if (self.frame != 0)
		return;

	if ((self.last_health - self.health) < 20)
		bell_smallring();
	else
		bell_bigring();

	self.last_health = self.health;
}

/*QUAKED obj_bell (0.3 0.1 0.6) (-100 -100 -210) (100 100 8)
A big bell that rings when hit. 
-------------------------FIELDS-------------------------
health = 250
--------------------------------------------------------
*/
void obj_bell (void)
{
	precache_sound("misc/bellring.wav");
	precache_model("models/bellring.mdl");

	CreateEntityNew(self,ENT_BELL,"models/bellring.mdl",chunk_death);

	self.last_health = self.health;
	self.th_pain = bell_ring;
	self.use = bell_bigring;
}

/*QUAKED brush_pushable (0.3 0.1 0.6) ?
A brush the player can push
-------------------------FIELDS-------------------------
mass - (default 5)
--------------------------------------------------------
*/
void brush_pushable()
{

	self.max_health = self.health;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_PUSHPULL;
	setorigin (self, self.origin);	
	setmodel (self, self.model); 
	self.classname="pushable brush";
	self.touch	= obj_push;
    self.hull = HULL_BIG;
	setsize(self,self.mins,self.maxs);
	if(!self.mass)
		self.mass = 5;
}

void statue_death (void)
{
	if (self.enemy)   // The head must die first
	{
		self.enemy.nextthink = time + .01;
		self.enemy.think = chunk_death;
		chunk_death();
	}
	else
		chunk_death();
}

/*QUAKED obj_statue_mummy_head (0.3 0.1 0.6) (-16 -16 -26) (16 16 160)
Statue of the nubis head
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------

*/
void() obj_statue_mummy_head =
{
	precache_model2("models/mhdstatu.mdl");
	CreateEntityNew(self,ENT_STATUE_MUMMYHEAD,"models/mhdstatu.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;

};

/*QUAKED obj_statue_mummy (0.3 0.1 0.6) (-16 -16 0) (16 16 160)
Statue of the mummy monster
-------------------------FIELDS-------------------------
health = 200
mass = 150
--------------------------------------------------------
*/
void() obj_statue_mummy =
{
	entity head;

	head=spawn();
	head.scale = self.scale;

	if (self.scale)
	{
		head.origin_z = 132 * self.scale;
		head.mass = 150 * self.scale;
	}
	else
	{
		head.origin_z = 132;
	}

	head.origin += self.origin;
	head.angles = self.angles;
	self.enemy = head;

	precache_model2 ("models/mumstatu.mdl");
	CreateEntityNew(self,ENT_STATUE_MUMMY_BODY,"models/mumstatu.mdl",statue_death);

	self.drawflags += SCALE_ORIGIN_BOTTOM;

	precache_model2 ("models/mhdstatu.mdl");
	CreateEntityNew(head,ENT_STATUE_MUMMY_HEAD,"models/mhdstatu.mdl",statue_death);

	head.health = self.health;	
	head.drawflags += SCALE_ORIGIN_BOTTOM;

};


/*QUAKED obj_pot1 (0.3 0.1 0.6) (-24 -24 0) (24 24 50)
A clay pot with handles
-------------------------FIELDS-------------------------
health = 10
mass = 100;
worldspawn worldtype determines which skin texture gets used
--------------------------------------------------------
*/
void obj_pot1 (void)
{

	precache_model("models/pot1.mdl");
	if (world.worldtype==WORLDTYPE_CASTLE)
		self.skin = 0;
	else if (world.worldtype==WORLDTYPE_EGYPT)
		self.skin = 1;
	else if (world.worldtype==WORLDTYPE_MESO)
		self.skin = 2;
	else if (world.worldtype==WORLDTYPE_ROMAN)
		self.skin = 3;

	CreateEntityNew(self,ENT_POT1,"models/pot1.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;

	if(self.targetname)
		self.use=chunk_death;
}


/*QUAKED obj_pot2 (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A pot with gently curved sides
-------------------------FIELDS-------------------------
health = 10
mass = 100;
worldspawn worldtype determines which skin texture gets used
--------------------------------------------------------
*/
void obj_pot2 (void)
{

	precache_model("models/pot2.mdl");
	if (world.worldtype==WORLDTYPE_CASTLE)
		self.skin = 0;
	else if (world.worldtype==WORLDTYPE_EGYPT)
		self.skin = 1;
	else if (world.worldtype==WORLDTYPE_MESO)
		self.skin = 2;
	else if (world.worldtype==WORLDTYPE_ROMAN)
		self.skin = 3;

	CreateEntityNew(self,ENT_POT2,"models/pot2.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;

	if(self.targetname)
		self.use=chunk_death;
}


/*QUAKED obj_pot3 (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A pot with a sharply curved sides
-------------------------FIELDS-------------------------
health = 10
mass = 100;
worldspawn worldtype determines which skin texture gets used
--------------------------------------------------------
*/
void obj_pot3 (void)
{
	precache_model("models/pot3.mdl");
	if (world.worldtype==WORLDTYPE_CASTLE)
		self.skin = 0;
	else if (world.worldtype==WORLDTYPE_EGYPT)
		self.skin = 1;
	else if (world.worldtype==WORLDTYPE_MESO)
		self.skin = 2;
	else if (world.worldtype==WORLDTYPE_ROMAN)
		self.skin = 3;

	CreateEntityNew(self,ENT_POT3,"models/pot3.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;

	if(self.targetname)
		self.use=chunk_death;
}


/*QUAKED obj_statue_tut (0.3 0.1 0.6) (-36 -36 0) (36 36 248)
An Egyptian statue of a guy with a flat head hat
-------------------------FIELDS-------------------------
health = 1000
mass = 2000
--------------------------------------------------------
*/
void obj_statue_tut (void)
{
	precache_model2("models/tutstatu.mdl");

	CreateEntityNew(self,ENT_STATUE_TUT,"models/tutstatu.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;
}

/*QUAKED obj_flag (0.3 0.1 0.6) (-16 -16 0) (16 16 160)
A flag that wave in the breeze
-------------------------FIELDS-------------------------
health = 50
--------------------------------------------------------
*/
void obj_flag (void)
{
	precache_model("models/flag.mdl");
	CreateEntityNew(self,ENT_FLAG,"models/flag.mdl",chunk_death);
//	self.frame = random(0,40);
}

/*QUAKED obj_statue_snake (0.3 0.1 0.6) (-16 -16 0) (16 16 80) INVINCIBLE
The front of a snake
INVINCIBLE = Won't take damage
-------------------------FIELDS-------------------------
defaults:
health = 100
mass = 200;
--------------------------------------------------------
*/
void obj_statue_snake (void)
{
	precache_model2("models/snkstatu.mdl");

	CreateEntityNew(self,ENT_STATUE_SNAKE,"models/snkstatu.mdl",chunk_death);

	if(self.spawnflags&1)
	{
		self.health=0;
		self.takedamage=DAMAGE_NO;
	}
}

/*QUAKED obj_hedge1 (0.3 0.1 0.6) (-16 -16 0) (16 16 80)
A hedge that looks like an X-mas tree
-------------------------FIELDS-------------------------
health = 20
mass = 200;
--------------------------------------------------------
*/
void obj_hedge1 (void)
{
	precache_model2("models/hedge1.mdl");
	CreateEntityNew(self,ENT_HEDGE1,"models/hedge1.mdl",chunk_death);
}


/*QUAKED obj_hedge2 (0.3 0.1 0.6) (-16 -16 0) (16 16 80)
A hedge that is square and of medium height
-------------------------FIELDS-------------------------
health = 20
mass = 200;
--------------------------------------------------------
*/
void obj_hedge2 (void)
{
	precache_model2("models/hedge2.mdl");
	CreateEntityNew(self,ENT_HEDGE2,"models/hedge2.mdl",chunk_death);
}

/*QUAKED obj_hedge3 (0.3 0.1 0.6) (-16 -16 0) (16 16 80)
A hedge that is tall and thin
-------------------------FIELDS-------------------------
health = 20
mass = 200;
--------------------------------------------------------
*/
void obj_hedge3 (void)
{
	precache_model2("models/hedge3.mdl");
	CreateEntityNew(self,ENT_HEDGE3,"models/hedge3.mdl",chunk_death);
}

/*QUAKED obj_fountain (0.3 0.1 0.6) (-24 -24 0) (24 24 80)
A water fountain
-------------------------FIELDS-------------------------
health = 20
mass = 200;
--------------------------------------------------------
*/
void obj_fountain (void)
{
	precache_model2("models/fountain.mdl");
	CreateEntityNew(self,ENT_FOUNTAIN,"models/fountain.mdl",chunk_death);
}

/*QUAKED obj_book_open (0.3 0.1 0.6) (-8 -8 0) (8 8 10)
A book that is open
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_book_open (void)
{
	precache_model("models/bookopen.mdl");
	CreateEntityNew(self,ENT_BOOKOPEN,"models/bookopen.mdl",chunk_death);
	if(self.targetname)
		self.use=chunk_death;
}

/*QUAKED obj_book_closed (0.3 0.1 0.6) (-8 -8 0) (8 8 10)
A book that is closed
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_book_closed (void)
{
	precache_model("models/bookclos.mdl");
	CreateEntityNew(self,ENT_BOOKCLOSED,"models/bookclos.mdl",chunk_death);
	if(self.targetname)
		self.use=chunk_death;
}


/*QUAKED obj_fence (0.3 0.1 0.6) (-26 -26 0) (26 26 70)
A section of fence
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_fence (void)
{
	precache_model3("models/fence.mdl");
	CreateEntityNew(self,ENT_FENCE,"models/fence.mdl",chunk_death);
}


/*QUAKED obj_bush1 (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A small round bush
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_bush1 (void)
{
	precache_model("models/bush1.mdl");
	CreateEntityNew(self,ENT_BUSH1,"models/bush1.mdl",chunk_death);
}


/*QUAKED obj_tombstone1 (0.3 0.1 0.6) (-24 -24 0) (24 24 60)
A tombstone in the shape of a cross
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_tombstone1 (void)
{
	precache_model("models/tombstn1.mdl");
	CreateEntityNew(self,ENT_TOMBSTONE1,"models/tombstn1.mdl",chunk_death);
}

/*QUAKED obj_tombstone2 (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A tombstone with a rounded top
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_tombstone2 (void)
{
	precache_model("models/tombstn2.mdl");
	CreateEntityNew(self,ENT_TOMBSTONE2,"models/tombstn2.mdl",chunk_death);
}

/*QUAKED obj_statue_angel (0.3 0.1 0.6) (-60 -60 0) (60 60 120)
A statue of Xena in all her glory, sorry - just wishful thinking. It's a statue of an angle praying.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_statue_angel (void)
{
	precache_model("models/anglstat.mdl");
	CreateEntityNew(self,ENT_STATUE_ANGEL,"models/anglstat.mdl",chunk_death);
}

void webs_think (void) [++ 1 .. 100]
{
	if(self.spawnflags&8)
	{
		self.touch=SUB_Null;
		if(cycle_wrapped)
		{
			self.touch=webs_think;
			self.nextthink=-1;
		}
	}
}

void webs_touch ()
{
//Make sticky?  Bouncy?
	if(!other.movetype||other.movetype==MOVETYPE_PUSHPULL||other.classname==self.classname)
		return;

	if(!other.flags&FL_ONGROUND)
		other.flags+=FL_ONGROUND;
}

void webs_death (void)
{
	if(!other.movetype||other.movetype==MOVETYPE_PUSHPULL||other.classname==self.classname)
		return;

	chunk_death();	
}

/*QUAKED obj_webs (0.3 0.1 0.6) (-25 -25 -25) (25 25 25) SOLID ANIMATE WEAK TOUCHMOVE FLAT NOTRANS
Big webby stuff.
Origin is in the center.
Note that the web is only 46 wide and 2 or 3 pixels thick, and 50 tall.

SOLID = Make it so you can walk on it and not pass though it.
ANIMATE = give it a slight constant animation so it looks like it's being moved by a breeze.  Looks best on cobwebs and corner web.
WEAK = This will make it break apart upon touch.
TOUCHMOVE = Will cycle through it's animation once only when touched, not meant to be used with any other spawnflag
FLAT = Meant to be used with solid, this will lay it out flat (all you have to do then is give it it's yaw angle) and adjust the bounding box size so it can be walked on.
NOTRANS = No translucency, totally solid coloring

FEILDS:
-----------------------------
abslight = Spiderwebs may need to be brighter or darker than their surroundings to look best.
health = default is 0, which means it won't take damage, otherwise, you can shoot it away or, if it has low health, you can break it by landing very hard on it
skin = default is 0 
	0 = many little spider webs
	1 = corner web, will appear in right side of the front of the box.
	2 = cobwebs, light and very nice looking
	3 = One giant web
	4 = One big-ass web, 15 times normal size.
scale = scale it up or down, note that this defaults to 1, and the maximum is 2.5

THESE ANGLES ARE RELATIVE TO IT'S FRONT (given by angle at bottom)
v_angle_x = how much pitch to tilt it front-back
v_angle_y = how much yaw to turn it (same as angle field at bottom)
v_angle_z = how much roll to tilt it sideways
These can also be entered in one field, such as:
v_angle = 0 0 0 (x, y, z)
-----------------------------
*/
void obj_webs (void)
{
	if(self.skin==4)
	{
		precache_model2("models/megaweb.mdl");
		CreateEntityNew(self,ENT_WEB,"models/megaweb.mdl",chunk_death);
		self.skin=0;
	}
	else
	{
		precache_model3("models/webs.mdl");
		CreateEntityNew(self,ENT_WEB,"models/webs.mdl",chunk_death);
	}

	if(!self.angles_y)
		self.angles=self.v_angle;
	else
	{
		self.angles_x=self.v_angle_x;
		self.angles_z=self.v_angle_z;
	}
	
	if(self.health)
	{
		self.takedamage=DAMAGE_YES;
		self.solid=SOLID_TRIGGER;
	}
	
	if(self.spawnflags&1)
	{
		self.solid=SOLID_BBOX;
		self.touch=webs_touch;
	}

	if(self.spawnflags&4)
	{
		if(!self.solid)
			self.solid=SOLID_TRIGGER;
		self.touch = webs_death;
	}

	if(self.spawnflags&8)
	{
		if(!self.solid)
			self.solid=SOLID_TRIGGER;
		self.touch = webs_think;
	}

	if(self.spawnflags&16)
	{
		self.angles_x=90;
		self.angles_z=0;
		setsize(self,'-25 -25 -2','25 25 2');
	}

	if(!self.spawnflags&32)
		self.drawflags=DRF_TRANSLUCENT;

//	self.use=chunk_death;
	setorigin(self,self.origin);

	if(self.spawnflags&2)
	{
		self.think=webs_think;
		self.nextthink=time;
	}
}

/*QUAKED obj_corpse1 (0.3 0.1 0.6) (-32 -32 0) (32 32 10)
A body laying face down
-------------------------FIELDS-------------------------
health = 20
mass = 200;

skin = determines the skin of the model
 0 - burnt, nude guy
 1 - normal, nude guy
 2 - yucky diseased, nude guy
 3 - wound in back, has on pants
--------------------------------------------------------
*/
void obj_corpse1 (void)
{
	precache_model ("models/corps1.mdl");
	CreateEntityNew(self,ENT_CORPSE1,"models/corps1.mdl",chunk_death);
	self.use = chunk_death;
}

/*QUAKED obj_corpse2 (0.3 0.1 0.6) (-32 -32 0) (32 32 10)
A body laying face up
-------------------------FIELDS-------------------------
health = 20
mass = 200;
skin = determines the skin of the model
 0 - shoulder and facial wounds
 1 - clawed chest
 2 - stomach wound
 3 - just dead
 4 - webbed 
--------------------------------------------------------
*/
void obj_corpse2 (void)
{
	precache_model("models/corps2.mdl");
	CreateEntityNew(self,ENT_CORPSE2,"models/corps2.mdl",chunk_death);
	self.use = chunk_death;
}

void cauldron_run (void)
{
	vector spot1;

	spot1= self.origin;
	spot1_z= self.origin_z + 35;
	CreateWhiteSmoke(spot1,'0 0 8',HX_FRAME_TIME * 2);
	self.think = cauldron_run;
	self.nextthink = time + random(0.5,1.5);
}

/*QUAKED obj_cauldron (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A cauldron
-------------------------FIELDS-------------------------
health = 50
mass = 15
--------------------------------------------------------
*/
void obj_cauldron (void)
{
	precache_model("models/cauldron.mdl");
	CreateEntityNew(self,ENT_CAULDRON,"models/cauldron.mdl",chunk_death);

	self.touch	= obj_push;
	self.flags	(+) FL_PUSH;
	self.drawflags += SCALE_ORIGIN_BOTTOM;
	
	self.think = cauldron_run;
	self.nextthink = time + random(0.5,1.5);

}

/*QUAKED obj_skullstick (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A skull on a stick - mmm,mmm, good
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_skullstick (void)
{
	precache_model2("models/skllstk1.mdl");
	CreateEntityNew(self,ENT_SKULLSTICK,"models/skllstk1.mdl",chunk_death);
}


/*QUAKED obj_skull_stick2 (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
Two skulls on a stick
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_skull_stick2 (void)
{
	precache_model2("models/skllstk2.mdl");
	CreateEntityNew(self,ENT_SKULLSTICK,"models/skllstk2.mdl",chunk_death);
}

/*QUAKED obj_ice (0 0 1) ? NOTRANS
Slippery, slidey ice
NOTRANS = No translucency
-------------------------FIELDS-------------------------
health = default = 20
friction = default is 0.2, 0 is none, 1 is normal, 10 is max
abslight = default is 0.5
--------------------------------------------------------
*/
void ice_touch (void)
{
	if(other.flags&FL_ONGROUND)
		if(random()>self.friction)
			other.flags-=FL_ONGROUND;
}

void ice_slab_melt (void)
{
	if(self.scale>0.05)
	{
		self.scale-=0.05;
		setsize(self,self.mins*0.9,self.maxs*0.9);
		thinktime self : 0.1;
	}
	else
		remove(self);
}

void obj_ice (void)
{
//thingtype_ice, need ice chunks
	if(self.flags2&FL_SUMMONED)
	{
//Make pushable, floating?!
		self.solid = SOLID_BBOX;
		self.movetype = MOVETYPE_NONE;
		self.drawflags(+)SCALE_TYPE_XYONLY;
		self.scale = 1;
		self.think = ice_slab_melt;
		thinktime self : 10;
	}
	else
	{
		self.solid = SOLID_BSP;
		self.movetype = MOVETYPE_PUSH;
	}
	self.takedamage=DAMAGE_YES;
	self.thingtype = THINGTYPE_ICE;
	setorigin (self, self.origin);	
	setmodel (self, self.model);

	self.frozen=TRUE;
	self.classname="ice";

	self.use = chunk_death;

	self.drawflags+=MLS_ABSLIGHT;

	if(!self.abslight)
		self.abslight = 0.75;

	if(!self.spawnflags&1)
		self.drawflags+=DRF_TRANSLUCENT;

	if(!self.health)
		self.health = 20;
	self.max_health = self.health;

	if(!self.friction)
		self.friction = 0.2;

//	self.touch = friction_change_touch;
	self.touch = ice_touch;
	self.th_die = chunk_death;
}

/*QUAKED obj_beefslab (0.3 0.1 0.6) (-16 -16 0) (16 16 40)
A slab of beef.
-------------------------FIELDS-------------------------
health = 50
--------------------------------------------------------
*/
void obj_beefslab (void)
{
	precache_model3("models/beefslab.mdl");
	CreateEntityNew(self,ENT_BEEFSLAB,"models/beefslab.mdl",chunk_death);
}

/*QUAKED obj_seaweed (0.3 0.1 0.6) (-8 -8 0) (8 8 32)
An animate seaweed that sways from side to side.
-------------------------FIELDS-------------------------
health = 10
--------------------------------------------------------
*/
void obj_seaweed (void)
{
	precache_model("models/seaweed.mdl");
	CreateEntityNew(self,ENT_SEAWEED,"models/seaweed.mdl",chunk_death);
}

/*QUAKED obj_statue_lion (0.3 0.1 0.6) (-56 -14 0) (56 14 60)
Statue of a lion.
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------

*/
void obj_statue_lion(void)
{
	precache_model2("models/lion.mdl");
	CreateEntityNew(self,ENT_STATUE_LION,"models/lion.mdl",chunk_death);

	self.drawflags += SCALE_ORIGIN_BOTTOM;

}

/*QUAKED obj_statue_athena(0.3 0.1 0.6) (-30 -30 0) (30 30 90)
Statue of a Athena
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------

*/
void obj_statue_athena (void)
{
	precache_model2("models/athena.mdl");
	CreateEntityNew(self,ENT_STATUE_ATHENA,"models/athena.mdl",chunk_death);

	self.drawflags += SCALE_ORIGIN_BOTTOM;

}


/*QUAKED obj_statue_neptune(0.3 0.1 0.6) (-30 -30 0) (30 30 100)
Statue of Neptune (I think)
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------

*/
void obj_statue_neptune (void)
{
	precache_model2("models/neptune.mdl");
	CreateEntityNew(self,ENT_STATUE_NEPTUNE,"models/neptune.mdl",chunk_death);

	self.drawflags += SCALE_ORIGIN_BOTTOM;

}

/*QUAKED obj_bonepile(0.3 0.1 0.6) (-10 -10 0) (10 10 10)
A pile of bones. Uses the model of the puzzle keep1
-------------------------FIELDS-------------------------
health = 
--------------------------------------------------------
*/
void obj_bonepile (void)
{
	precache_model3("models/bonepile.mdl");
	CreateEntityNew(self,ENT_BONEPILE,"models/bonepile.mdl",chunk_death);

	self.use = chunk_death;

	self.drawflags += SCALE_ORIGIN_BOTTOM;
}

/*QUAKED obj_statue_caesar(0.3 0.1 0.6) (-24 -24 0) (24 24 90)
Statue of a Caesar Romero
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------
*/
void obj_statue_caesar (void)
{
	precache_model2("models/caesar.mdl");
	CreateEntityNew(self,ENT_STATUE_CAESAR,"models/caesar.mdl",chunk_death);

	self.drawflags += SCALE_ORIGIN_BOTTOM;

}

/*QUAKED obj_statue_snake_coil (0.3 0.1 0.6) (-44 -44 0) (44 44 90)
Statue of a coiled snake (just like the one that comes to life) but this one doesn't come to life.
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------
*/
void obj_statue_snake_coil (void)
{
	precache_model2 ("models/snake.mdl");
	CreateEntityNew(self,ENT_STATUE_SNAKE_COIL,"models/snake.mdl",chunk_death);

	self.scale = .5;
	self.drawflags += SCALE_ORIGIN_BOTTOM;

}

/*QUAKED obj_skull (0.3 0.1 0.6) (-8 -8 0) (8 8 16)
A skull, suitable for over the fireplace or perhaps a colorful holiday display
-------------------------FIELDS-------------------------
health = 10
--------------------------------------------------------
*/
void obj_skull (void)
{
	precache_model("models/skull.mdl");
	CreateEntityNew(self,ENT_SKULL,"models/skull.mdl",chunk_death);
}

/*QUAKED obj_pew (0.3 0.1 0.6) (-16 -40 0) (16 40 50)
A church pew - like you might find in a church.
-------------------------FIELDS-------------------------
health = 50
--------------------------------------------------------
*/
void obj_pew (void)
{
	precache_model("models/pew.mdl");
	CreateEntityNew(self,ENT_PEW,"models/pew.mdl",chunk_death);
}

/*QUAKED obj_statue_olmec (0.3 0.1 0.6) (-40 -40 0) (40 40 130)
A olmec statue, of course. What the heck is an olmec?
-------------------------FIELDS-------------------------
health = 50
--------------------------------------------------------
*/
void obj_statue_olmec (void)
{
	precache_model2("models/olmec1.mdl");
	CreateEntityNew(self,ENT_STATUE_OLMEC,"models/olmec1.mdl",chunk_death);
}

/*QUAKED obj_statue_mars (0.3 0.1 0.6) (-30 -30 0) (30 30 80)
A statue of Mars.
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------
*/
void obj_statue_mars (void)
{
	precache_model2("models/mars.mdl");
	CreateEntityNew(self,ENT_STATUE_MARS,"models/mars.mdl",chunk_death);
}

/*QUAKED obj_playerhead_paladin (0.3 0.1 0.6) (-8 -8 0) (8 8 16)
The head of the paladin.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_playerhead_paladin (void)
{
	precache_model("models/h_pal.mdl");
	CreateEntityNew(self,ENT_PLAYERHEAD,"models/h_pal.mdl",chunk_death);
	self.use=chunk_death;
}

/*QUAKED obj_playerhead_assassin (0.3 0.1 0.6) (-8 -8 0) (8 8 16)
The head of the assassin.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_playerhead_assassin (void)
{
	precache_model("models/h_ass.mdl");
	CreateEntityNew(self,ENT_PLAYERHEAD,"models/h_ass.mdl",chunk_death);
	self.use=chunk_death;
}

/*QUAKED obj_playerhead_necromancer (0.3 0.1 0.6) (-8 -8 0) (8 8 16)
The head of the necromancer.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_playerhead_necromancer (void)
{
	precache_model ("models/h_nec.mdl");
	CreateEntityNew(self,ENT_PLAYERHEAD,"models/h_nec.mdl",chunk_death);
	self.use=chunk_death;
}

/*QUAKED obj_playerhead_crusader (0.3 0.1 0.6) (-8 -8 0) (8 8 16)
The head of the crusader.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_playerhead_crusader (void)
{
	precache_model ("models/h_cru.mdl");
	CreateEntityNew(self,ENT_PLAYERHEAD,"models/h_cru.mdl",chunk_death);
	self.use=chunk_death;
}

/*QUAKED obj_statue_king (0.3 0.1 0.6) (-30 -30 0) (30 30 120)
A statue of a king holding a sword in front of him.
-------------------------FIELDS-------------------------
health = 200
--------------------------------------------------------
*/
void obj_statue_king (void)
{
	precache_model3("models/king.mdl");
	CreateEntityNew(self,ENT_STATUE_KING,"models/king.mdl",chunk_death);
	self.mins -= '0 0 80';
	self.maxs -= '0 0 80';
	setsize(self, self.mins, self.maxs);
}

/*QUAKED obj_plant_generic (0.3 0.1 0.6) (-10 -10 0) (10 10 20)
A generic plant that should have some kind of pot placed below it.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_plant_generic (void)
{
	precache_model ("models/plantgen.mdl");
	CreateEntityNew(self,ENT_PLANT_GENERIC,"models/plantgen.mdl",chunk_death);
}

/*QUAKED obj_plant_meso (0.3 0.1 0.6) (-10 -10 0) (10 10 40)
A generic plant that should have some kind of pot placed below it.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_plant_meso (void)
{
	precache_model2("models/plantmez.mdl");
	CreateEntityNew(self,ENT_PLANT_MESO,"models/plantmez.mdl",chunk_death);
}

/*QUAKED obj_plant_rome (0.3 0.1 0.6) (-24 -24 0) (24 24 90)
A plant for the Rome area.
-------------------------FIELDS-------------------------
health = 20
--------------------------------------------------------
*/
void obj_plant_rome (void)
{
	precache_model2("models/plantrom.mdl");
	CreateEntityNew(self,ENT_PLANT_ROME,"models/plantrom.mdl",chunk_death);
}

