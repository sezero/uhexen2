/*QUAKED obj_catapult2 (0 .5 .8) (-150 -150 0) (150 150 28) not_usable
not_usable - can't be used, stuck in the up position

"speed"		Throw speed (300 default)
"wait"		wait before resetting (3 default)
"health"	Just how tough is it (defaults to 1000)
"mass"		How hard is it to push (defaults to 1000)
"thingtype" Defaults to THINGTYPE_WOOD
"sounds"
0)	no sound
1)	stone
2)	base
3)	stone chain
4)	screechy metal

*/

void catapult_ready (void)
{
//FIXME: No linking in touch, do it all in here with a tracearea (is that working?) and findradius
	if(self.flags&FL_ONGROUND)
		self.movetype = MOVETYPE_NONE;

	if(self.origin!=self.oldorigin||self.angles!=self.o_angle)
	{
	entity found;
	vector dir,org;	
//		dprint("updating dependancies\n");
		makevectors(self.angles);
		dir=normalize(v_forward);
		org=self.origin+dir*-4*self.level;
		org_z=self.absmax_z;
		found=nextent(world);
		while(found)
		{
			if(vhlen(found.origin-org)<2*self.level&&found.catapulter==self)
			{
//				dprint(found.classname);
//				dprint(" updated\n");
				setorigin(found,org+found.pos_ofs);
				found.angles=self.angles+found.angle_ofs;
				found.velocity='0 0 0';
				found.flags(+)FL_ONGROUND;
			}
			found=nextent(found);
		}
	}
	self.o_angle=self.angles;
	self.oldorigin=self.origin;
	self.think=catapult_ready;
	thinktime self : 0;
}

void() catapult_reset =
{
	if(self.frame==22)
	{
		sound(self,CHAN_VOICE,"misc/catdrop.wav",1,ATTN_NORM);
		self.frame=0;
	}

	if(self.frame>=20)
	{
		sound(self,CHAN_VOICE,"misc/catreset.wav",1,ATTN_NORM);
		self.frame=20;
		self.think=catapult_ready;
		thinktime self : 0;
	}
	else
	{
		self.frame+=1;
		self.think=catapult_reset;
		thinktime self : 0.05;
	}
};

void catapult_wait (void)
{
	self.think=catapult_reset;
	thinktime self : self.wait;
}

void catapult_fire (void)
{
	if(self.frame==20)
	{
	sound(self,CHAN_VOICE,"misc/catlnch.wav",1,ATTN_NORM);
	entity found;
	vector dir,org,addvel;
	float distance, force,centrifugal;	
		found=nextent(world);
		makevectors(self.angles);
		dir=normalize(v_forward);
		org=self.origin+dir*-4*self.level;
		org_z=self.absmax_z;

		while(found)
		{
			distance=vhlen(found.origin-org);

//			if(found.catapulter==self)

			if(distance<2*self.level&&found.origin_z>self.origin_z+self.maxs_z*0.75)//&&found.catapulter==self)
			{
//				inertia?
				found.catapult_time=time+3;
				found.catapulter=world;
				centrifugal=vhlen(found.origin-self.origin);
				force=self.speed + random(-100,100) + centrifugal*4;//Ignore mass, Not exact physics, but feels better
				addvel=dir*force+v_right*random(-50,50);//Give some left-right innacuracy to it
				force=self.speed + random(-100,100)+ centrifugal*4;
				addvel_z=force;
				found.velocity+=addvel;
				if(!found.touch)
					found.touch=obj_push;
				found.flags(-)FL_ONGROUND;
				if(!found.flags2&FL_ALIVE)
				{
					found.avelocity=found.velocity*random(-1,1);
					found.movetype=MOVETYPE_BOUNCE;
				}
				if(found.model=="models/sheep.mdl")
				{
					found.avelocity=found.velocity*random(-1,1);
					found.movetype=MOVETYPE_BOUNCE;
					sound(found,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
					found.pain_finished=time+1;
				}
			}
		
			found=nextent(found);
		}
	}
	if(self.frame>=22)
	{
		self.frame=22;
		self.think=catapult_wait;
		thinktime self : 0;
	}
	else
	{
		self.frame+=1;
		self.think=catapult_fire;
		thinktime self : 0.05;
	}
}

void catapult_pain (void)
{
	if(!self.enemy.flags2&FL_ALIVE)
		return;

	if(self.frame==20)
		catapult_fire();
}

void catapult2_touch(void)
{
	if(other.solid==SOLID_BSP||other==world)
		return;

//	dprint(ftos(other.absmin_z));
//	dprint(" > ");
//	dprint(ftos(self.absmax_z));
//	dprint("?\n");

	if(other.origin_z-(other.mins_z*0.75)>=self.origin_z+(self.maxs_z*0.75))
	{
		if(other.solid!=SOLID_TRIGGER&&other.movetype&&other.catapulter!=self&&other.catapult_time<time+0.1)
		{
		vector dir,org;	
		float distance;
			makevectors(self.angles);
			dir=normalize(v_forward);
			org=self.origin+dir*-4*self.level;
			org_z=self.absmax_z;
			distance=vhlen(other.origin-org);
			if(distance<=2*self.level)
			{
//				dprint(other.classname);
//				dprint(" locked and loaded!\n");
				other.catapult_time=time+0.1;
				other.catapulter=self;
				other.velocity=self.velocity;
				other.pos_ofs=other.origin-org;
				other.angle_ofs=self.angles-other.angles;
			}
			other.flags(+)FL_ONGROUND;
		}

	}
/*WAS pushable...
	else if(vlen(other.velocity)>100&&other.flags&FL_ONGROUND)
	{
//	Push or spin
		vector dir1, dir2;
		float magnitude,dot_forward,inertia;//dot_right,
		self.angles_x=self.angles_z=0;
		makevectors(self.angles);
		magnitude=vlen(other.velocity);
		inertia=1/(self.mass/10);

		dir1=normalize(self.origin-other.origin);
		dir2=normalize(v_forward);
		dir1_z=dir2_z=0;

		dot_forward= dir1*dir2;

	    if(dot_forward >0.8)
		{
//			dprint("Move forward\n");
			self.velocity+=dir2*magnitude*inertia;
//			dprint(vtos(self.velocity));
			self.flags(-)FL_ONGROUND;
		}
		else if(dot_forward<-0.8)
		{
//			dprint("Move backwards\n");
			self.velocity+=dir2*magnitude*-1*inertia;
			self.flags(-)FL_ONGROUND;
		}
		else
		{
			dir1=normalize(other.velocity);
			dir2=normalize(v_right);
			dot_right= dir1*dir2;
		    if(dot_right >0.2)
			{
			    if(dot_forward >0.2)
				{
					self.angles_y-=1*magnitude/100;
				}
				else if(dot_forward<-0.2)
				{
					self.angles_y+=1*magnitude/100;
				}
			}
			else if(dot_right<-0.2)
			{
			    if(dot_forward >0.2)
				{
					self.angles_y+=1*magnitude/100;
				}
				else if(dot_forward<-0.2)
				{
					self.angles_y-=1*magnitude/100;
				}
			}
		}
	}
*/
}

void obj_catapult2 (void)
{
	precache_model("models/cattest.mdl");
	precache_sound ("misc/catlnch.wav");
	precache_sound ("misc/catreset.wav");
	precache_sound ("misc/catdrop.wav");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_PUSHPULL;
//	setmodel (self, "models/catapult.mdl");
	setmodel (self, "models/cattest.mdl");
	setsize(self,'-145 -145 0','145 145 26');
	self.hull=HULL_SCORPION;//HYDRA;
	setorigin (self, self.origin);	
	self.classname="catapult";
	self.level=30;
	self.frame=20;
	

//	if (!self.speed)
		self.speed = 300;//Too strong?
	if (self.wait==0)
		self.wait = 3;

	if(!self.thingtype)
		self.thingtype = THINGTYPE_WOOD;
	
	if(!self.mass)
		self.mass = 1000;

	if (!self.health)
		self.health=1000;
	self.max_health = self.health;

	if(!self.spawnflags&1)
	{
		self.touch=catapult2_touch;
		self.th_die = chunk_death;
		self.takedamage = DAMAGE_YES;
		self.use=catapult_fire;
		self.th_pain=catapult_pain;
		self.th_weapon=catapult_fire;
		self.think=catapult_ready;
		thinktime self : 0;
	}
	else
	{
	entity solidbox;
		self.frame=22;
		solidbox=spawn();
		setorigin(solidbox,self.origin+'0 0 24');
		solidbox.solid=SOLID_BBOX;
		setsize(solidbox,'-24 -24 0','24 24 100');
	}
}

