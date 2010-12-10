/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/barrel.hc,v 1.2 2007-02-07 17:00:33 sezero Exp $
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

//Checks to see if the bounding boxes of the 2 ents overlap at any point.
//Not intended for BSP models- bounding box models only
float overlapped (entity ent1, entity ent2)
{
vector mins1,maxs1,mins2,maxs2;
float overlapped_axes;

	mins1=ent1.absmin;
	maxs1=ent1.absmax;
	mins2=ent2.absmin;
	maxs2=ent2.absmax;
	
	if(mins1_x>maxs2_x||maxs1_x<mins2_x)
		return FALSE;
	else
	{
//		dprint("X axes overlap\n");
		overlapped_axes+=1;
	}

	if(mins1_y>maxs2_y||maxs1_y<mins2_y)
		return FALSE;
	else
	{
//		dprint("Y axes overlap\n");
		overlapped_axes+=1;
	}
	
	if(mins1_z>maxs2_z||maxs1_z<mins2_z)
		return FALSE;
	else
	{
//		dprint("Z axes overlap\n");
		overlapped_axes+=1;
	}

	if(overlapped_axes==3)
	{
//		dprint("All 3 axes overlap!\n");
		return TRUE;
	}

	return FALSE;
}

void obj_barrel_fake_chunk_death(void)
{
	vector space;
	float spacecube,model_cnt,scalemod;
	string deathsound;

	space = self.absmax - self.absmin;

	spacecube = space_x * space_y * space_z;

	model_cnt = spacecube / 8192;   // (16 * 16 * 16)

	deathsound="fx/woodbrk.wav";

	sound (self, CHAN_VOICE, deathsound, 1, ATTN_NORM);

	if (spacecube < 5000)
	{
		scalemod = .20;
		model_cnt = model_cnt * 3;	// Because so few pieces come out of a small object
	}
	else if (spacecube < 50000)
	{
		scalemod = .45;
		model_cnt = model_cnt * 3;	// Because so few pieces come out of a small object
	}
	else if (spacecube < 500000)
	{
		scalemod = .50;
	}
	else if (spacecube < 1000000)
	{
		scalemod = .75;
	}
	else
	{
		scalemod = 1;
	}

	if(model_cnt>CHUNK_MAX)
		model_cnt=CHUNK_MAX;

	while (model_cnt>0)
	{
		if (chunk_cnt < CHUNK_MAX*2)
		{
			CreateModelChunks(space,scalemod);
			chunk_cnt+=1;
		}

		model_cnt-=1;
	}
	
	make_chunk_reset();

	self.solid = SOLID_NOT;
//	self.effects = EF_NODRAW;
	
	if(self.trigger_field)
		remove(self.trigger_field);
}

void barrel_go ();
void rep_barrel_wait ()
{
entity item;
//float dist,s1,s2;
	
//	dprint("Barrel checking to respawn\n");
	item  = findradius(self.origin, 256);
	while (item)
	{
//		dprint("Barrel found ents\n");
		if (item.solid&&item.solid!=SOLID_TRIGGER&&item.solid!=SOLID_BSP&&item!=self)
		{
//			dprint("Barrel found solid\n");
//			dist = vhlen(item.origin-self.origin);
//			s1 = item.maxs_x*1.5;
//			s2 = 24;//barrel size, at most- 22.6 if 16 by 16
//			if(s1+s2>=dist)
			if(overlapped(self,item))
			{
//				dprint("Barrel waiting- too close to ");
//				dprint(item.classname);
//				dprint("\n");
				self.think = rep_barrel_wait;
				thinktime self : 2;
				return;
			}
		}

		item = item.chain;
	}
	barrel_go();
}

void spawn_rep_barrel (void)
{
	obj_barrel_fake_chunk_death();
	setorigin(self,self.wallspot);
	self.effects=self.flags=self.flags2=self.frame=self.fire_damage=0;
	self.enemy=world;
	self.classname=self.netname;
	self.flags2(+)FL_SUMMONED;
	self.angles=self.velocity=self.avelocity='0 0 0';
	self.health=self.max_health;
	self.think=rep_barrel_wait;
	if(self.netname=="obj_barrel_gfire")
		thinktime self : 3;
	else
		thinktime self : 0.5;
	setmodel(self,"models/null.spr");
//	dprintv("Barrel respawning with %s angles\n",self.angles);
/*
	entity replacement;
	replacement=spawn();
	replacement.spawnflags=self.spawnflags;
	setmodel(replacement,"models/null.spr");
	setorigin(replacement,self.wallspot);
	replacement.frags=self.frags;
	replacement.classname=self.netname;
	replacement.flags2(+)FL_SUMMONED;
	replacement.targetname=self.targetname;
	replacement.think=rep_barrel_wait;
	replacement.nextthink = time + 1;
*/
}



//BIG FIRE========================================================

void burn_out ()
{
	if(self.scale>0.1)
	{
		self.scale-=0.1;
		setsize(self.trigger_field,'-24 -24 0'*self.scale,'24 24 48'*self.scale);
		thinktime self : 0.01;
		self.dmg=self.trigger_field.dmg=self.scale*3;
	}
	else
	{
		remove(self.trigger_field);
		stopSound(self,0);
		remove(self);
	}
}

void spawn_big_fire(vector org)
{
entity bigfire,oself;
	bigfire=spawn();
	
	oself=self;
	self = bigfire;

	self.dmg=2;
	self.classname="big greek fire";
	self.drawflags(+)SCALE_ORIGIN_BOTTOM|DRF_TRANSLUCENT|MLS_FIREFLICKER;
	self.scale = 1;
	setmodel(self,"models/newfire.mdl");
	setorigin(self,org);
	spawn_burnfield(org);
	setsize(self.trigger_field,'-48 -48 0','48 48 64');
	sound (self,CHAN_UPDATE+PHS_OVERRIDE_R, "misc/flamloop.wav", 0.5, ATTN_LOOP);
	self.think = burn_out;
	thinktime self : 20;//burn for 20 seconds

	self=oself;
}

//GREEK FIRE========================================================
void obj_barrel_gfire_explode (void)
{
	T_RadiusDamage (self, self, random(150,200), self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_PURIFY2_EXPLODE);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 8);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 8);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 8);
	multicast(self.origin,MULTICAST_PHS_R);

	spawn_big_fire(self.origin);

	if(self.spawnflags&BARREL_RESPAWN)
	{
		spawn_rep_barrel();
//		obj_barrel_fake_chunk_death();
		return;
	}

	remove(self);
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

	T_RadiusDamage(self, attacker, 200,self);//00, self);

//	sound(self, CHAN_VOICE, "weapons/explode.wav", 1, ATTN_NORM);

	particleexplosion(self.origin + '0 0 83',384,60,40);

	starteffect(CE_LG_EXPLOSION , (self.absmin+self.absmax)*0.5);

	if(self.spawnflags&BARREL_RESPAWN)
	{
		spawn_rep_barrel();
//		obj_barrel_fake_chunk_death();
		return;
	}

	chunk_death();
}

void obj_barrel_explode ()
{
	//delay so too many don't cause crash
	self.th_die=SUB_Null;
	self.takedamage = DAMAGE_NO;

	self.think=obj_barrel_explode_go;
	if(dmMode==DM_SIEGE)
		thinktime self : 0;
	else
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
	if(self.model!="models/gfire.mdl")
	{
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
	}
	else
	{
		//particleexplosion(self.origin + '0 0 24',264,60,40);
		sound(self,CHAN_AUTO,"raven/outwater.wav",1,ATTN_NORM);
	}

	
	if(self.spawnflags&BARREL_RESPAWN)
	{
		spawn_rep_barrel();
//		obj_barrel_fake_chunk_death();
		return;
	}
	
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

void obj_barrel_gfire_light ()
{
	self.use=SUB_Null;
	self.th_die	= obj_barrel_gfire_explode;
	sound (self, CHAN_AUTO, "weapons/expsmall.wav", 1, ATTN_NORM);
	starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 110');
	self.frame=1;
	sound(self,CHAN_UPDATE+PHS_OVERRIDE_R,"misc/flamloop.wav",0.5,ATTN_LOOP);
	self.effects(+)EF_UPDATESOUND|EF_BRIGHTLIGHT;
}

void spawn_barrel(float barrel_type)
{
	if(dmMode==DM_SIEGE)
		self.spawnflags(+)BARREL_RESPAWN;

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
	{
		setsize(self, '-13 -13 0','13 13 36');
		self.hull=HULL_PLAYER;
	}

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
	else if (barrel_type==BARREL_GFIRE)
	{
		if(!self.flags2&FL_SUMMONED)
			setorigin(self,self.origin+'-64 -12 0');//for now, get it out of the way
		self.th_die	= barrel_die;
		setmodel(self,"models/gfire.mdl");
		setsize(self,'-16 -16 0','16 16 56');
		self.hull=HULL_PLAYER;
		self.use=obj_barrel_gfire_light;
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

	spawn_push_trigger(3);
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
	if(self.health)
		self.max_health=self.health;
	spawn_barrel(BARREL_NORMAL);
	if(!self.max_health)
		self.max_health=self.health;
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
	if(self.health)
		self.max_health=self.health;
	spawn_barrel(BARREL_INDESTRUCTIBLE);
	if(!self.max_health)
		self.max_health=self.health;
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
	if(self.health)
		self.max_health=self.health;
	spawn_barrel(BARREL_EXPLODING);
	if(!self.max_health)
		self.max_health=self.health;
	self.mass = 85;
}

/*QUAKED obj_barrel_gfire (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK DROP_USE RESPAWN
Greek fire barrel
Use a torch to light them
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
void obj_barrel_gfire(void)
{
	precache_model("models/newfire.mdl");
	precache_model("models/gfire.mdl");
	precache_sound("misc/flamloop.wav");
	precache_sound("misc/gflaunch.wav");
	if(self.spawnflags&DROP_USE)
		self.spawnflags(+)BARREL_NO_DROP;
	if(self.health)
		self.max_health=self.health;
	spawn_barrel(BARREL_GFIRE);
	self.mass = 100;
	if(!self.max_health)
		self.max_health=self.health;
}

void barrel_go(void)
{
//	dprint("Barrel_go: ready to respawn\n");
	if(self.classname=="obj_barrel_normal")
	{
//		dprint("Respawning normal barrel\n");
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_NORMAL);
		self.mass = 75;
	}
	else if(self.classname=="obj_barrel_indestructible")
	{
//		dprint("Respawning invincible barrel\n");
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_INDESTRUCTIBLE);
		self.mass = 95;
	}
	else if(self.classname=="obj_barrel_exploding")
	{
//		dprint("Respawning explode barrel\n");
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_EXPLODING);
		self.mass = 100;//85
	}
	else if(self.classname=="obj_barrel_gfire")
	{
//		dprint("Respawning gfire barrel\n");
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_GFIRE);
		self.mass = 100;//85
	}
	else
	{
//		dprint("NO BARREL TYPE!!!\n");
		if(self.spawnflags&DROP_USE)
			self.spawnflags(+)BARREL_NO_DROP;
		spawn_barrel(BARREL_EXPLODING);
		self.mass = 100;//85
	}
}

