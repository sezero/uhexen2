/*QUAKED obj_catapult2 (0 .5 .8) (-150 -150 0) (150 150 28) ?

"speed"		Throw speed (300 default)
"wait"		wait before resetting (3 default)
"health"	Just how tough is it (defaults to 1000)
"mass"		How hard is it to push (defaults to 1000)
"thingtype" Defaults to THINGTYPE_WOOD
"aflag" - Max distance it can be pushed from start pos (default 256)
			"-1" disables this
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
//	if(self.flags&FL_ONGROUND&&world.model!="maps/siege3.bsp")
//		self.movetype = MOVETYPE_NONE;

	if(self.origin!=self.oldorigin||self.angles!=self.o_angle)
	{
	entity found;
	vector dir,org,fmins,fmaxs;	
//		dprint("updating dependancies\n");
		makevectors(self.angles);
		dir=normalize(v_forward);
		org=self.origin+dir*-4*self.level;
		org_z=self.absmax_z;
		found=nextent(world);
		while(found)
		{
			//Find ground entity
			fmins=found.mins;
			fmaxs=found.maxs;
			fmins_z=0;
			fmaxs_z=8;
			tracearea(found.origin+'0 0 1',found.origin-'0 0 8',fmins,fmaxs,FALSE,found);
			if(trace_ent!=self)
				found.catapulter=world;
			if(vhlen(found.origin-org)<2*self.level&&found.catapulter==self)
			{
			float repos_dist,repos_dist_cnt,repos_dir_angle;
			vector repos_dir;
			entity save_ent;
//				dprint(found.classname);
//				dprint(" updated\n");
				repos_dir=org+found.pos_ofs - found.origin;
				repos_dist=vlen(repos_dir);
				repos_dir=normalize(repos_dir);
				repos_dir_angle=vectoyaw(repos_dir);
				save_ent=self;
				self=found;
				while(repos_dist_cnt<repos_dist)
				{
					repos_dist_cnt+=1;
					if(!walkmove(repos_dir_angle,1,FALSE))
						repos_dist_cnt=repos_dist;
				}
				self=save_ent;
//				setorigin(found,org+found.pos_ofs);
				found.angles=self.angles+found.angle_ofs;
				found.velocity='0 0 0';
				found.flags(+)FL_ONGROUND;
			}
			found=nextent(found);
		}
	}
	if(self.angles_y>self.o_angle_y)
		AdvanceFrame(33,42);
	else if(self.angles_y<self.o_angle_y)
		AdvanceFrame(42,33);
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
entity firstcat, catdude;
	firstcat=catdude=find(world,classname,other.catapulter.classname);
	while(catdude)
	{
		if(other.catapulter==self)
			other.catapulter=world;
		catdude=find(catdude,classname,other.catapulter.classname);
		if(catdude==firstcat)
			catdude=world;
	}

	self.think=catapult_reset;
	thinktime self : self.wait;
}

void catapult_fire (void)
{
	if(self.frame==20||self.frame>22)
	{
	sound(self,CHAN_VOICE,"misc/catlnch.wav",1,ATTN_NORM);
	entity found;
	vector dir,org,addvel;
	float distance, force,centrifugal,throwback;	
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

				traceline(found.origin,found.origin-'0 0 33',FALSE,found);
				if(trace_ent!=self)
				{//off back edge, throw backwards
					throwback=TRUE;
					centrifugal=vhlen(found.origin-org);
					force=self.speed + random(-100,100) + centrifugal*4;//Ignore mass, Not exact physics, but feels better
					addvel=dir*-1*force+v_right*random(-50,50);//Give some left-right innacuracy to it
					force=self.speed + random(-100,100)+ centrifugal*4;
					addvel_z=force;
				}
				else
				{
					throwback=FALSE;
					centrifugal=vhlen(found.origin-self.origin);
					force=self.speed + random(-100,100) + centrifugal*4;//4 Ignore mass, Not exact physics, but feels better
					addvel=dir*force+v_right*random(-50,50);//Give some left-right innacuracy to it
					force=self.speed + random(-100,100)+ centrifugal*4;
					addvel_z=force;//FIXME: CAP at ???
				}
//				if(found.playerclass==CLASS_DWARF)
//					addvel*=1.05;
				found.velocity+=addvel;
				if(!found.touch)
					found.touch=obj_push;
				found.flags(-)FL_ONGROUND;
				if(found.classname=="player")//&&found.playerclass!=CLASS_SUCCUBUS)
				{
					found.touch=PlayerTouch;//experimental- only for impacts now
					found.teleport_time=time+3;//No mario
					if(found.velocity_z>850)
						found.velocity_z=850;//cap?
				}
				if(!found.flags2&FL_ALIVE)
				{
					found.velocity=found.velocity*1.3;
					if(throwback)
						found.avelocity=found.velocity*(48-centrifugal)*random(-1,1);
					else
						found.avelocity=found.velocity*random(-1,1);
					found.movetype=MOVETYPE_BOUNCE;
				}

				if(found.model=="models/sheep.mdl")
				{
					if(found.classname!="player")
					{
						found.velocity=found.velocity*1.3;
						found.teleport_time+=666;//impact damage forever
						found.touch=found.th_die;
						found.enemy = self.enemy;
						if(found.trigger_field)
							remove(found.trigger_field);
					}
					found.avelocity=found.velocity*random(-1,1);
					found.movetype=MOVETYPE_BOUNCE;
					sound(found,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
					found.pain_finished=time+1;
				}
				if(found.model=="models/barrel.mdl")
				{
					found.touch = found.th_die;//Maybe make them always explode on touch
					if(found.netname=="obj_barrel_gfire")
						sound(self,CHAN_BODY,"misc/gflaunch.wav",1,ATTN_NORM);

					if(found.trigger_field)
						remove(found.trigger_field);
					found.enemy = self.enemy;
					found.think = found.th_die;
					thinktime found : 5;
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
	if(!self.enemy.flags2&FL_ALIVE&&self.enemy.classname!="blood missile")
		return;

	if(self.frame==20||self.frame>22)
		catapult_fire();
}

void catapult2_touch(void)
{
	if(other.solid==SOLID_BSP||other==self.movechain||other==world)
		return;

/*	dprint(other.classname);
	dprint("\n");
	dprint(ftos(other.absmin_z));
	dprint(" > ");
	dprint(ftos(self.absmax_z));
	dprint("?\n");*/

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

//WAS pushable...
	else if(other.classname=="player"&&(vlen(other.velocity)>100||coop||deathmatch)&&other.flags&FL_ONGROUND&&(other.impulse==13||self.last_use_time + 2>time))
	{//only players can directly push it
//	Push or spin
		vector dir1, dir2,move_vel,dest_spot;
		float magnitude,dot_forward,inertia;//,dot_right;
		if(other.impulse == 13)
			self.last_use_time = time;
		self.angles_x=self.angles_z=0;
		makevectors(self.angles);
		magnitude=400;//vlen(other.velocity);
		inertia=1/(self.mass/10);

		dir1=normalize(self.origin-other.origin);
		dir2=normalize(v_forward);
		dir1_z=dir2_z=0;

		dot_forward= dir1*dir2;

	    if(dot_forward >0.8)
		{
			move_vel=dir2*magnitude*inertia*5;
			dest_spot=self.origin+(move_vel*0.01);
			if(self.aflag!=-1)
				if(vhlen(dest_spot-self.wallspot)>=self.aflag)
				{
					self.movetype=MOVETYPE_NONE;//pushed too far
					return;
				}
//			walkmove(self.angles_y,1,FALSE);
			//			dprint("Move forward\n");
			self.velocity=move_vel;//FIXME: adds up if do += (for multiple pushers)
//			dprint(vtos(self.velocity));
			self.flags(-)FL_ONGROUND;
			AdvanceFrame(23,32);
		}
		else if(dot_forward<-0.8)
		{
			move_vel=dir2*-1*magnitude*inertia*5;
			dest_spot=self.origin+(move_vel*0.01);
			if(self.aflag!=-1)
				if(vhlen(dest_spot-self.wallspot)>=self.aflag)
				{
					self.movetype=MOVETYPE_NONE;//pushed too far
					return;
				}
//			dprint("Move backwards\n");
//			walkmove(self.angles_y,-1,FALSE);
			self.velocity=move_vel;//FIXME: adds up if do += (for multiple pushers)
			self.flags(-)FL_ONGROUND;
			AdvanceFrame(32,23);
		}
//		if(self.movechain)
//			setorigin(self.movechain,self.origin+'0 0 26');
/*		else
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
		}*/
	}

}

void obj_catapult2 (void)
{
	precache_model("models/cattest.mdl");
	precache_sound ("misc/catlnch.wav");
	precache_sound ("misc/catreset.wav");
	precache_sound ("misc/catdrop.wav");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_PUSHPULL;
	self.touch=catapult2_touch;
//	setmodel (self, "models/catapult.mdl");
	setmodel (self, "models/cattest.mdl");
	setsize(self,'-145 -145 0','145 145 26');
	self.hull=HULL_SCORPION;
	setorigin (self, self.origin);	
	self.wallspot=self.origin;
	if(self.aflag==0)
		self.aflag=256;//Max dist to move from origin- not working?  Set to movetyp NONE if gets too far?  then can;t turn...
	self.classname="catapult";
	self.level = 30;
	self.frame=20;
	
	self.th_pain=catapult_pain;
	self.th_weapon=catapult_fire;

	if (!self.speed)
		self.speed = 300;//Too strong?
	if (self.wait==0)
		self.wait = 3;
	self.th_die = chunk_death;
	self.takedamage = DAMAGE_YES;
	self.use=catapult_fire;

	if(!self.thingtype)
		self.thingtype = THINGTYPE_WOOD;
	
	if(!self.mass)
		self.mass = 1000;

	if (!self.health)
		self.health=1000;
	
	self.max_health = self.health = self.health*2;

	self.think=catapult_ready;
	thinktime self : 0;

	spawn_push_trigger(10);
}

