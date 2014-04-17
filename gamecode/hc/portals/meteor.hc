/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/meteor.hc,v 1.3 2007-02-07 16:59:34 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\meteor\final\meteor.hc

==============================================================================
*/
// For building the model
$cd Q:\art\models\weapons\meteor\final
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame idle

//
$frame Select1      Select2      Select3      Select4      Select5      
$frame Select6      Select7      Select8      Select9      Select10     
$frame Select11     Select12     Select13     Select14     Select15     
$frame Select16     Select17     Select18     

//
$frame fire1     fire2     fire3     fire4     fire5     
$frame fire6     fire7     fire8     fire9     


void MeteoriteFizzle (void)
{
	CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
	remove(self);
}

void MeteorThink (void)
{
	if(self.lifetime<time)
		if(self.dmg==3)
			MeteoriteFizzle();
		else
			MultiExplode();

	if(self.dmg>3)
		CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);

	self.think=MeteorThink;
	thinktime self : 0.3;
}

void MeteorTouch (void)
{
	if(other.controller==self.owner)
		return;

	if(self.dmg==3)
	{
		if(other==world)
		{
			if(!self.pain_finished&&random()<0.3)
			{
				sound(self.controller,CHAN_BODY,"misc/rubble.wav",1,ATTN_NORM);
				self.pain_finished=TRUE;
			}
			return;
		}
		else if(other.classname=="meteor")
			return;
	}

	if(other.takedamage&&other.health)
	{
		T_Damage(other,self,self.owner,self.dmg);
		if(self.dmg>3)
		{
			if((other.flags&FL_CLIENT||other.flags&FL_MONSTER)&&other.mass<200)
			{
			vector hitdir;
				hitdir=self.o_angle*300;
				hitdir_z+=150;
				if(hitdir_z<0)
					hitdir_z=0;
				other.velocity=hitdir;
				other.flags(-)FL_ONGROUND;
			}
			self.dmg/=2;
		}
	}
	else if(self.dmg>3)
		self.dmg=100;

	if(self.dmg>3)
		MultiExplode();
	else
		MeteoriteFizzle();
}

void FireMeteor (string type)
{
vector org;
entity meteor;
	meteor=spawn();
	setmodel(meteor,"models/tempmetr.mdl");
	if(type=="minimeteor")
	{
		meteor.classname="minimeteor";
		meteor.velocity=RandomVector('200 200 0');
		meteor.velocity_z=random(200,400);
		meteor.lifetime=time + 1.5;
		meteor.dmg=3;
		meteor.scale=random(0.15,0.45);
		meteor.movetype=MOVETYPE_BOUNCE;
		org=self.origin;
		setsize(meteor,'0 0 0', '0 0 0');
	}
	else
	{
		meteor.th_die=MultiExplode;
		if(self.classname=="player")
		{
			self.greenmana-=8;
			self.velocity+=normalize(v_forward)*-300;//include mass
			self.flags(-)FL_ONGROUND;
		}
		meteor.classname="meteor";
		self.punchangle_x = -6;
		sound(self,CHAN_AUTO,"crusader/metfire.wav",1,ATTN_NORM);
		self.attack_finished=time + 0.7;
		self.effects(+)EF_MUZZLEFLASH;
		makevectors(self.v_angle);
		meteor.speed=1000;
		meteor.o_angle=normalize(v_forward);		
		meteor.velocity=meteor.o_angle*meteor.speed;
		meteor.veer=30;
		meteor.lifetime=time + 5;
		meteor.dmg=75;
		meteor.movetype=MOVETYPE_FLYMISSILE;
		org=self.origin+self.proj_ofs+v_forward*12;
		setsize(meteor,'0 0 0', '0 0 0');
	}
//	meteor.abslight = 0.5;
	// Pa3PyX
//	meteor.drawflags(+)MLS_FIREFLICKER;//|MLS_ABSLIGHT;
	meteor.abslight = 1.0;
	meteor.drawflags (+) (MLS_FIREFLICKER | MLS_ABSLIGHT);

	meteor.avelocity=RandomVector('360 360 360');

	if(self.classname=="tornato")
		meteor.owner=self.controller;
	else if(self.classname=="meteor")
		meteor.owner=self.owner;
	else
		meteor.owner=self;
	meteor.controller=self;

	meteor.solid=SOLID_BBOX;
	meteor.touch=MeteorTouch;

	meteor.think=MeteorThink;
	thinktime meteor : 0.1;

	setorigin(meteor,org);
}

void() tornato_die = [++24 .. 47]
{
	if(cycle_wrapped)
	{
		if(self.enemy)
		{
			self.enemy.avelocity='0 500 0';
			if(self.enemy.flags2&FL_ALIVE)
				self.enemy.movetype=self.enemy.oldmovetype;
		}
		if(self.movechain!=world)
			remove(self.movechain);
		remove(self);
	}
	self.movechain.frame+=1;
	if(self.movechain.frame>24)
		self.movechain.frame=0;
	if(self.movechain.scale>0.04)
		self.movechain.scale-=0.04;
	if(self.movechain.avelocity_y>0)
		self.movechain.avelocity_y-=20;
};

void() tornato_spin = [++0 .. 23]
{
float distance,content;
float pick_up;

	if(time>self.lifetime||self.torncount<self.owner.torncount - 1)
	{
		self.movechain.drawflags(+)MLS_ABSLIGHT|SCALE_ORIGIN_BOTTOM|SCALE_TYPE_XYONLY;
		self.think=tornato_die;
		thinktime self : 0;
	}
	self.movechain.frame+=1;
	if(self.movechain.frame>24)
		self.movechain.frame=0;

//FIXME:  add tracking to movement and firing.
	if(random()<0.2)
	{
		self.velocity_x+=random(-100*self.scale,100*self.scale);
		if(fabs(self.velocity_x)>1000)
			self.velocity_x/=2;
	}
	
	if(random()<0.2)
	{
		self.velocity_y+=random(-100*self.scale,100*self.scale);
		if(fabs(self.velocity_y)>1000)
			self.velocity_y/=2;
	}

	content=pointcontents(self.origin);
	if(content==CONTENT_WATER||content==CONTENT_LAVA)
	{
		self.velocity_z+=random(33,200);
		particle4(self.origin,random(20),264*15,PARTICLETYPE_GRAV,random()*10);
		particle4(self.origin,random(20),random(406,414),PARTICLETYPE_GRAV,random(10));
	}
	else if(random()<0.2)
	{
		distance=random(-30,15);//tries to stay on ground
		if(self.goalentity!=world&&self.enemy!=self.goalentity)
			if(self.goalentity.origin_z>self.origin_z)//unless goal is above it
				distance=random(-30,30);
		self.velocity_z+=distance;
		if(fabs(self.velocity_z)>333)
			self.velocity_z/=3;
	}

	if(self.enemy!=world)
	{
	vector org, dir;
	float let_go;
		self.velocity=self.velocity*0.5;
		org=self.origin;
		if(self.enemy.size_z>=self.size_z)
			org=self.origin;
		else
			org_z+=random(10)*self.scale+4*self.scale;

		if(vlen(self.enemy.origin-self.origin))
			let_go=TRUE;
		
		if(!self.enemy.flags2&FL_TORNATO_SAFE)
		{
			self.enemy.velocity='0 0 0';
			setorigin(self.enemy,org);
		}
		else
		{
			self.enemy.flags2(-)FL_TORNATO_SAFE;
			let_go=TRUE;
		}
//FIXME:  throw the Sheep
		if(!let_go&&self.enemy!=world&&!self.enemy.flags2&FL_ALIVE)//Don't let go of it if it's not a creature
			if(random()>=0.4||self.goalentity==world||(!visible(self.goalentity))||self.goalentity.health<=0)
				self.pain_finished=time+1;
			else
			{
				self.pain_finished=-1;
				if(self.goalentity.solid==SOLID_BSP&&self.goalentity.origin=='0 0 0')
					dir=normalize((self.goalentity.absmax+self.goalentity.absmin)*0.5-self.enemy.origin);
				else
					dir=normalize(self.goalentity.origin-self.enemy.origin);
			}
		if(!let_go&&self.enemy.takedamage&&self.enemy.health>0&&self.pain_finished>time)
		{
			if(random()<0.3)
				T_Damage(self.enemy,self,self.owner,self.scale);//was 3*is this needed with meteors flying out?
		}
		else
		{
			if(!let_go)
				if(self.pain_finished==-1)		//Throw it at my goal!
					self.enemy.velocity=dir*350*self.scale;
				else
				{
					self.enemy.velocity_z=random(200*self.scale);
					self.enemy.velocity_x=random(200*self.scale,-200*self.scale);
					self.enemy.velocity_y=random(200*self.scale,-200*self.scale);
				}
			self.pain_finished=time;
			self.enemy.safe_time=time+3+let_go*7;//let them get thrown away from the tornado for a full 3 seconds
			if(self.enemy.flags2&FL_ALIVE)
			{
				self.enemy.movetype=self.enemy.oldmovetype;
				if(self.enemy.classname=="player_sheep")
				{
					sound(self.enemy,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
					self.enemy.pain_finished=time+1;
				}
			}
			if(!let_go)
				self.enemy.avelocity_y=random(200*self.scale);
			self.enemy=self.movechain.movechain=world;
		}
		if(self.enemy.classname=="player")
		{
			self.enemy.punchangle_y=random(3,12);//FIXME: Do WRITEBYTE on angles?
			self.enemy.punchangle_x=random(-3,3);//FIXME: Do WRITEBYTE on angles?
			self.enemy.punchangle_z=random(-3,3);//FIXME: Do WRITEBYTE on angles?
		}
		if(self.enemy!=world&&self.goalentity==self.enemy)
			self.goalentity=world;//Hunt a new target, if it can
	}
	if(random()<0.3)
	{
	entity sucker;
	float seekspeed;
		sucker=findradius(self.origin,500);
		while(sucker)
		{
			pick_up=TRUE;
			if(sucker.monsterclass>=CLASS_BOSS)
				if(sucker.flags&FL_MONSTER||sucker.classname=="buddha_shield"||sucker.classname=="buddha_firewalker")
					pick_up=FALSE;
			if(!sucker.solid||sucker.solid==SOLID_BSP)
				pick_up=FALSE;
			if(teamplay&&sucker.classname=="player"&&(coop||self.controller.team==sucker.team))
				pick_up=FALSE;
			if(pick_up&&sucker.takedamage&&sucker.health&&sucker!=self.enemy&&sucker.mass<500*self.scale&&visible(sucker)&&sucker!=self.owner&&sucker.owner!=self.owner)
				if(sucker.movetype&&sucker.movetype!=MOVETYPE_PUSH)
				{
					seekspeed=(500 - vlen(sucker.origin-self.origin));
					sucker.velocity=normalize(self.origin-sucker.origin)*seekspeed;
					if(sucker.velocity_z<30)
						sucker.velocity_z=30;
					sucker.flags(-)FL_ONGROUND;
					if(sucker.classname=="player")
						sucker.adjust_velocity=sucker.velocity;
				}
			sucker=sucker.chain;
		}
		if(self.goalentity!=world&&visible(self.goalentity)&&self.goalentity.health>0)
		{
				seekspeed = random(150,333);
				if(self.goalentity.solid==SOLID_BSP&&self.goalentity.origin=='0 0 0')
					distance=vlen((self.goalentity.absmax+self.goalentity.absmin)*0.5-self.origin);
				else
					distance=vlen(self.goalentity.origin-self.origin);//Swoop in when close!
				if(distance<256)
					seekspeed+=(256-distance);
				if(self.goalentity.velocity)
					seekspeed+=vlen(self.goalentity.velocity);
				self.velocity=(self.velocity*3+normalize(self.goalentity.origin-self.origin)*seekspeed*self.scale)*0.25;//too fast?
		}
		else
		{
		float bestdist;
			self.goalentity=world;//out of sight, out of mind
			bestdist=1001;
			sucker=findradius(self.origin,1000);
			while(sucker)
			{
				pick_up=TRUE;
				if(sucker.classname=="buddha_shield"||sucker.classname=="buddha_firewalker")
					pick_up=FALSE;
				if(!sucker.solid)
					pick_up=FALSE;
				if(teamplay&&sucker.classname=="player"&&(coop||self.controller.team==sucker.team))
					pick_up=FALSE;
				if(sucker.takedamage&&sucker.health&&sucker!=self.enemy&&sucker.mass<500*self.scale&&visible(sucker)&&sucker!=self.owner&&!sucker.effects&EF_NODRAW&&pick_up&&sucker.owner!=self.owner)
				{
					if(sucker.solid==SOLID_BSP&&sucker.origin=='0 0 0')
						distance=vlen((sucker.absmax+sucker.absmin)*0.5-self.origin);
					else
						distance=vlen(sucker.origin-self.origin);

					if(self.goalentity.velocity=='0 0 0')
					{
						if(sucker.velocity!='0 0 0'&&(sucker.flags2&FL_ALIVE))
						{
							bestdist=distance;
							self.goalentity=sucker;
						}
						else if(!self.goalentity.flags2&FL_ALIVE)
						{
							if(sucker.flags2&FL_ALIVE)
							{
								bestdist=distance;
								self.goalentity=sucker;
							}
							else if(distance<bestdist)
							{
								bestdist=distance;
								self.goalentity=sucker;
							}
						}
						else if(sucker.flags2&FL_ALIVE&&distance<bestdist)
						{
							bestdist=distance;
							self.goalentity=sucker;
						}
					}
					else if(distance<bestdist&&sucker.velocity!='0 0 0'&&(sucker.flags2&FL_ALIVE))
					{
						bestdist=distance;
						self.goalentity=sucker;
					}
				}
				sucker=sucker.chain;
			}
		}
 	}

	if(random()<0.1)
	{
		if(random()<0.1)
		{
			self.proj_ofs_z=random(6,54);
			self.v_angle_x=random(-30,30);
			self.v_angle_y=random(-360,360);
			FireMeteor("meteor");
		}
		else
			FireMeteor("minimeteor");
	}
	if(self.flags&FL_ONGROUND)
	{
	vector dir;
		self.velocity_z*=-0.333;//Maybe a little more bounce?
		self.flags(-)FL_ONGROUND;
		dir_z=random(20,70);
		distance=random(10,30);
		SpawnPuff (self.origin, dir,distance,self);
		CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
	}
	if(self.t_width<time)
	{
		sound(self,CHAN_VOICE,"crusader/tornado.wav",1,ATTN_NORM);
		self.t_width=time+1;
	}
};

void()funnal_touch;
void tornato_merge (void)
{
//FIXME:  Don't scale up rocks- just add more rocks?
	self.scale+=0.025;
	self.owner.scale+=0.025;
	self.goalentity.scale-=0.024;
	self.goalentity.owner.scale-=0.024;
	if(self.scale>=self.target_scale)
	{
		self.touch=funnal_touch;
		self.scale=self.owner.scale=self.target_scale;
		self.think=SUB_Null;
		self.nextthink=-1;
		remove(self.goalentity.owner);
		remove(self.goalentity);
	}
	else
	{
		self.think=tornato_merge;
		thinktime self : 0.01;
	}
}

void funnal_touch (void)
{
//FIXME:  Ignore the controlling player's projectiles, leaving it in to test
	if(other.monsterclass>=CLASS_BOSS&&(other.flags&FL_MONSTER||other.classname=="buddha_shield"||other.classname=="buddha_firewalker"))
	{
		T_Damage(other,self,self.owner,7);
		traceline((self.absmin+self.absmax)*0.5,(other.absmin+other.absmax)*0.5,FALSE,self);
		SpawnPuff(trace_endpos,randomv('-1 -1 -1','1 1 1'),5,other);
		return;
	}

	if(other==self.controller||other.controller==self.owner||other==world||other==self.owner||other==self.owner||other.classname=="tornato"||(other.classname=="funnal"&&other.aflag)||other.movetype==MOVETYPE_PUSH)
		return;

	if(teamplay&&other.classname=="player"&&(coop||self.controller.team==other.team))
		return;

	if(self.aflag)
	{
		self.owner.think=SUB_Remove;
		self.think=SUB_Remove;
		return;
	}

	if(other.classname=="funnal"&&other.scale>=1&&self.scale>=1&&other.scale+self.scale<2.5)
	{
//Add random to stall the merging
		tracearea(self.origin,self.origin,self.mins+other.mins,self.maxs+other.maxs,TRUE,self);
		if(trace_fraction<1)
			return;
		self.goalentity=other;
		self.touch=other.touch=SUB_Null;
		if(other.controller!=self.controller)
			self.owner.owner=self.owner.controller=self.controller=self.owner;
//make scaling gradual
		self.drawflags=MLS_ABSLIGHT|SCALE_ORIGIN_BOTTOM;
		self.owner.drawflags=SCALE_ORIGIN_BOTTOM;
		other.drawflags=MLS_ABSLIGHT+SCALE_ORIGIN_BOTTOM+SCALE_TYPE_XYONLY;
		self.target_scale=self.scale+other.scale;
		if(self.target_scale>2.5)
			self.target_scale=2.5;
		setsize(self,self.mins+other.mins,self.maxs+other.maxs);
		setsize(self.owner,self.owner.mins+other.owner.mins,self.owner.maxs+other.owner.maxs);
		tornato_merge();
	}
	else if(other!=self.movechain&&other.movetype&&other.mass<500*self.scale&&other.classname!="funnal")//Can't pick up or move extremely heavy objects, bounce off them?
	{
		if(other.health&&other.takedamage&&other.solid!=SOLID_BSP)//Ignore health>1000?
		{
			if(!other.touch)
				other.touch=obj_push;//Experimental
			if(self.movechain==world&&other.safe_time<time)//&&self.scale>=1)
			{		
				self.movechain=other;
				other.flags(+)FL_MOVECHAIN_ANGLE;
				setorigin(other,self.origin+'0 0 4');//maybe need to take on bounding box of captured enemy too?
				other.velocity='0 0 0';
				if(other.flags2&FL_ALIVE)
					other.avelocity='0 0 0';
				else
				{
					other.avelocity_x=random(360);
					other.avelocity_z=random(360);
				}
				other.oldmovetype=other.movetype;
				other.movetype=MOVETYPE_NONE;
				self.owner.enemy=other;
				self.owner.pain_finished=time+random(3,10);//How long to hold them before throwing them away
				if(other.classname=="player_sheep"&&other.flags2&FL_ALIVE)
				{
					sound(other,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
					other.pain_finished=time+1;
				}
				return;
			}
		}
		vector dir;
		dir=normalize(self.angles);
		dir*=random(200,700)*self.scale;
		other.velocity+=dir;
		other.velocity_z=random(100,250)*self.scale;
		other.flags(-)FL_ONGROUND;
		if(other.takedamage)
			T_Damage(other,self.owner,self.owner.controller,5*self.scale);
		if(other.classname=="player_sheep"&&other.flags2&FL_ALIVE)
		{
			sound(other,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
			other.pain_finished=time+1;
		}
	}
}

void() tornato_grow = [++48 .. 72]
{
	if(cycle_wrapped)
	{
		self.movechain.scale=1;
		self.think=tornato_spin;
		thinktime self : 0;
	}
	self.movechain.frame+=1;
	if(self.movechain.frame>24)
		self.movechain.frame=0;
	self.movechain.scale+=0.04;
};
 
void FireMeteorTornado (void)
{
/*
FIX:
1:	BUG:If pull someone out of water & they die, stay in swim mode.(Fly mode does this with water too)
2:	More particles & splash sound when hit water
3:  Deflect projectiles
4:  Limit 2, if 3rd made, erase 1st
5:  Shorten life?
6:  Can't hurt owner
9:	Player's view should actually be changed with WRITEBYTE's?
10: Screw up aim of people inside tornado
11: gradual suck in, then stick to center?
12: incorporate mass?  At least check to see if it can be picked up, maybe give a little resistance
13: Meteors are going through walls
14:	Change bounding box to match what it picked up?
15: Scale up to match something big it picked up?
16: Check it there's room in front to make it, and at what height, use v_forward if possible.  If not enough room:?
17: Auntie Em, Auntie Em!
18:	Don't even consider movetype_push's?
19:	Scale randomly?
20:	Origins MUST be at bottom
21: Particle and Puff Sprites at origin when onground
22: If pick up something not alive, throw it at goalentity?  Random chance?
23: Bounding box should be a little bigger
24: If it hits water while holding a player, it should go down and drown them.
*/
entity tornato,funnal;
vector org;

	self.greenmana-=20;
	sound(self,CHAN_WEAPON,"crusader/torngo.wav",1,ATTN_NORM);
	makevectors(self.v_angle);
	org=self.origin+normalize(v_forward)*16;
	org_z=self.origin_z+1;
		
	tornato=spawn();
	self.torncount+=1;
	tornato.torncount=self.torncount;
	tornato.solid=SOLID_NOT;
	tornato.movetype=MOVETYPE_FLY;
	tornato.owner=tornato.controller=self;
	tornato.classname="tornato";
	tornato.enemy=world;
	setmodel(tornato,"models/tornato.mdl");
	setsize(tornato,'-18 -18 -3','18 18 64');
	tornato.hull=HULL_PLAYER;
	setorigin(tornato,org);
	tornato.velocity=normalize(v_forward)*250+'0 0 20';
	tornato.velocity_z=0;
	tornato.scale=1;
	if(visible(self.enemy)&&self.enemy.flags2&FL_ALIVE)//Infront too?
		tornato.goalentity=self.enemy;
	tornato.lifetime=time + 20;
	tornato.think=tornato_grow;
	thinktime tornato : 0;

	funnal=spawn();
	funnal.owner=tornato;
	funnal.solid=SOLID_TRIGGER;
	funnal.classname="funnal";
	funnal.movetype=MOVETYPE_FLYMISSILE;
	funnal.drawflags(+)MLS_ABSLIGHT|SCALE_ORIGIN_BOTTOM|SCALE_TYPE_ZONLY;
	funnal.abslight=0.2;
	funnal.scale=0.01;
	tornato.movechain=funnal;
	funnal.avelocity='0 100 0';
	funnal.controller=self;
	funnal.touch=funnal_touch;
	funnal.lifetime=time+1.7;
	setmodel(funnal,"models/funnal.mdl");
	setsize(funnal,'-18 -18 -3','18 18 64');
	funnal.hull=HULL_PLAYER;
	setorigin(funnal,org);
}

void()meteor_ready_loop;
void() Cru_Met_Attack;

void meteor_power_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire9);
	self.th_weapon=meteor_power_fire;
	// Pa3PyX
//	if(self.weaponframe==$fire2 && self.attack_finished<=time)
	if(self.weaponframe==$fire1 && self.attack_finished<=time) {
			self.attack_finished = time + 0.5;// Pa3PyX
			FireMeteorTornado();
	}

	if(self.wfs==WF_CYCLE_WRAPPED)
	{
			self.last_attack=time;
			meteor_ready_loop();
	}
}

void meteor_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire9);
	self.th_weapon=meteor_fire;

	if((!self.button0||self.attack_finished>time)&&self.wfs==WF_CYCLE_WRAPPED)
	{
		self.last_attack=time;
		meteor_ready_loop();
	}
	else if(self.weaponframe==$fire1 &&self.attack_finished<=time)
			FireMeteor("meteor");
}

void() Cru_Met_Attack =
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		self.th_weapon=meteor_power_fire;
	else
		self.th_weapon=meteor_fire;
	thinktime self : 0;
};

void meteor_ready_loop (void)
{
	self.weaponframe = $idle;
	self.th_weapon=meteor_ready_loop;
}

void meteor_select (void)
{
//go to ready loop, not relaxed?
	self.wfs = advanceweaponframe($Select1,$Select16);
	self.weaponmodel = "models/meteor.mdl";
	self.th_weapon=meteor_select;
	self.last_attack=time;
	// Pa3PyX
//	if(self.wfs==WF_CYCLE_WRAPPED)
	if(self.weaponframe==$Select16)
	{
		self.attack_finished = time - 1;
		meteor_ready_loop();
	}
}

void meteor_deselect (void)
{
	self.wfs = advanceweaponframe($Select16,$Select1);
	self.th_weapon=meteor_deselect;

	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

