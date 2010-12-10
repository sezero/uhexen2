/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/raven.hc,v 1.1.1.1 2004-11-29 11:35:11 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\RAVEN\FINAL\raven.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RAVEN\FINAL
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame 1stpec1      1stpec2      1stpec3      1stpec4      1stpec5      
$frame 1stpec6      1stpec7      1stpec8      1stpec9      1stpec10     
$frame 1stpec11     1stpec12     1stpec13     1stpec14     1stpec15     
$frame 1stpec16     1stpec17     1stpec18     1stpec19     1stpec20     
$frame 1stpec21     1stpec22     1stpec23     1stpec24     

//
$frame 2ndpec1      2ndpec2      2ndpec3      2ndpec4      2ndpec5      
$frame 2ndpec6      2ndpec7      2ndpec8      2ndpec9      2ndpec10     
$frame 2ndpec11     2ndpec12     2ndpec13     

//
$frame bounce1      bounce2      bounce3      bounce4      bounce5      
$frame bounce6      bounce7      bounce8      

//
$frame death1       

//
$frame down1        

//
$frame fastfly1     fastfly2     fastfly3     fastfly4     fastfly5     

//
$frame flaunt1      flaunt2      flaunt3      flaunt4      flaunt5      
$frame flaunt6      flaunt7      flaunt8      

//
$frame fold1        fold2        fold3        fold4        fold5        
$frame fold6        fold7        fold8        fold9        fold10       
$frame fold11       fold12       fold13       fold14       fold15       

//
$frame grabing1     grabing2     grabing3     grabing4     grabing5     

//
$frame land1        land2        land3        land4        land5        

//
$frame lowlft1      

//
$frame lowrght1     

//
$frame POLY         

//
$frame root1        

//
$frame shit1        shit2        shit3        shit4        shit5        
$frame shit6        shit7        

//
$frame slowfly1     slowfly2     slowfly3     slowfly4     slowfly5     
$frame slowfly6     slowfly7     slowfly8     slowfly9     

//
$frame soarght1     soarlft1     soar1        

//
$frame takeoff1     takeoff2     takeoff3     takeoff4     takeoff5     

//
$frame upleft1      

//
$frame uprght1      

//NOTE: Need Raven chunks (feathers)
//	needs "Nevermore" sound, squawk sound, feather flapping sound

void()raven_flap_fast;
void()raven_glide;
void()raven_glide_right;
void()raven_glide_left;
void()raven_hop_wings_out;
void()raven_stand;
void()raven_slowdown;
void()raven_land;
void()raven_fold_wings;
void()raven_ruffle_end;
void()raven_peck_up1;
void()raven_peck_up_all;
void()raven_peck_up2;
void()raven_peck2;
void()raven_peck_down;
void()raven_peck_down1;
void()raven_peck_down2;

void drop_feathers()
{
vector ofs;
	newmis=spawn();
	newmis.movetype=MOVETYPE_FLY;
	newmis.avelocity_y=random(-200,200);
	newmis.angles_x=random(-90,90);
	newmis.velocity_z=random(-100);
	ofs=RandomVector('32 32 32');
	newmis.think=SUB_Remove;
	thinktime newmis : 3;
	setmodel(newmis,"models/fether.mdl");
	setorigin(newmis,self.origin+ofs);
}


/*
---------------
IN AIR
---------------
*/
void raven_takeoff (void) [++ $takeoff1 .. $takeoff5]
{
//after unfold, jumps in air, goes to fast flap
	ai_walk(3+self.frame-$takeoff1);
	if(cycle_wrapped)
	{
		self.flags(+)FL_FLY;
		self.movetype=MOVETYPE_FLY;
		self.think=raven_flap_fast;
		thinktime self : 0;
	}
}

void raven_flap_fast (void) [++ $fastfly1 .. $fastfly5]
{
//Taking off or flying fast
	ai_walk(self.speed+(self.frame-$fastfly1)*2);
	if(cycle_wrapped&&random()<0.5)
	{
		self.think=raven_glide;
		thinktime self : 0;
	}
}

/*
void raven_flap_slow (void) [++ $slowfly1 .. $slowfly9]
{
//used every now and then when gliding
	ai_walk(self.speed+self.frame-$slowfly1);
}
*/

void raven_glide_think ()
{
	ai_walk(self.speed);
	if(self.goalentity.origin_z<self.origin_z)
	{
		traceline(self.origin+v_forward*48,self.origin+v_forward*48-'0 0 100',FALSE,self);
		if(trace_fraction<1)
		{
			self.msg3="landing";
			self.think=raven_slowdown;
			thinktime self : 0;
			return;
		}
	}
	
	if(self.ideal_yaw>self.angles_y)
	{
		self.think=raven_glide_right;
		thinktime self : 0;
	}
	else if(self.ideal_yaw>self.angles_y)
	{
		self.think=raven_glide_left;
		thinktime self : 0;
	}
	else
	{
		self.think=raven_glide;
		thinktime self : 0;
	}
}

void raven_glide (void)
{
	self.frame=$soar1;
	if(self.angles_z>0)
		self.angles_z-=1;
	else if(self.angles_z<0)
		self.angles_z+=1;
	raven_glide_think();
}

void raven_glide_right (void)
{
//Add some roll
	self.frame=$soarght1;
	if(self.angles_z>-15)
		self.angles_z-=1;
	raven_glide_think();
}

void raven_glide_left (void)
{
//Add some roll
	self.frame=$soarlft1;
	if(self.angles_z<15)
		self.angles_z+=1;
	raven_glide_think();
}

void raven_slowdown (void) [++ $grabing1 .. $grabing5]
{
//landing or pulling off a piece or slowing down to grab something
	ai_walk(self.speed);
	if(self.msg3=="landing")
		if(self.speed>1)
			self.speed-=0.1;
	if(self.flags&FL_ONGROUND)
	{
		self.angles_z=0;
		self.msg3="";
		self.think=raven_land;
		thinktime self : 0;
	}
	if(self.angles_z>0)
		self.angles_z-=1;
	else if(self.angles_z<0)
		self.angles_z+=1;
}

void raven_land (void) [++ $land1 .. $land5]
{
//hitting the ground, comes from slowdown to flaunt
	ai_walk(1);
	if(cycle_wrapped)
	{
		self.flags(-)FL_FLY;
		self.movetype=MOVETYPE_STEP;
		self.think=raven_hop_wings_out;
		thinktime self : 0;
	}
}



/*
---------------
ON GROUND
---------------
*/
void raven_hop_wings_out (void) [++ $flaunt1 .. $flaunt8]
{
//flaunting, also after landing to folding
	ai_walk(5);
	if(cycle_wrapped)
	{
		if(self.msg3!="threatening"||random()<0.3)
		{
			self.think=raven_fold_wings;
			thinktime self : 0;
		}
	}
}

void raven_fold_wings (void) [++ $fold1 .. $fold15]
{
//fold wings away after flaunting or landing
	if(cycle_wrapped)
	{
		self.think=raven_stand;
		thinktime self : 0;
	}
}

void raven_open_wings (void) [-- $fold15 .. $fold1]
{
//open wings to flaunt or take off, or just randomly to ruffle
	if(cycle_wrapped)
	{
		if(self.msg3=="taking off")
			self.think=raven_flap_fast;
		else if(self.msg3=="threatening")
			self.think=raven_hop_wings_out;
		thinktime self : 0;
	}
	else if(random()<0.3)
	{
		self.think=raven_fold_wings;
		thinktime self : 0;
	}
}

void raven_hop (void) [++ $bounce1 .. $bounce8]
{
//hopping around, no flying
	ai_walk(2);
	if(cycle_wrapped)
	{
		self.think=raven_stand;
		thinktime self : 0;
	}
}

void raven_shit (void) [++ $shit1 .. $shit7]
{
//hold and drop shit on $shit6
	if(self.frame==$shit6)
		thinktime self : 0.77;
	else if(cycle_wrapped)
	{
		self.think=raven_stand;
		thinktime self : 0;
	}
}

void raven_ruffle_start (void) [++ $shit1 .. $shit3]
{
//frames 1-3 ping-pong for feather ruffle
	if(cycle_wrapped)
	{
		self.think=raven_ruffle_end;
		thinktime self : 0;
	}
}

void raven_ruffle_end (void) [-- $shit3 .. $shit1]
{
//frames 1-3 ping-pong for feather ruffle
	if(cycle_wrapped)
	{
		self.think=raven_stand;
		thinktime self : 0;
	}
}

void raven_choose_look (entity targ)
{
vector vec;
//FIXME: If !targ choose random dir
	vec=normalize((targ.absmin+targ.absmax)*0.5-self.origin+self.view_ofs);
	makevectors(self.angles);
}

void raven_stand_think ()
{
float r;
	if(!self.goalentity)
		self.goalentity=find(world,classname,"obj_corpse1");
			
	if(random()<0.3)
	{
		r=rint(random(1,10));
		if(r==1)
			self.think=raven_shit;
		else if(r==2)
			self.think=raven_ruffle_start;
		else if(r==3)
		{
			self.msg3="threatening";
			self.think=raven_open_wings;
		}
		else if(r==4)
			self.think=raven_ruffle_start;
		else if(r==5)
		{
			self.msg3="taking off";
			self.think=raven_open_wings;
		}
		else if(r==6)
			self.think=raven_peck_down;
		else
			self.think=raven_hop;
		thinktime self : 0;
	}
	else if(self.msg3=="looking")
		raven_choose_look(world);
	else
		thinktime self : 0.05;
}

void raven_stand (void)
{
	self.frame=$root1;
	self.think=raven_stand_think;
	thinktime self : 0;
}

void raven_die (void)
{
	drop_feathers();
	if(self.health<-20)
		chunk_death();
	else
	{
		self.frame=$death1;
		self.flags(-)FL_FLY;
		if(self.movetype!=MOVETYPE_BOUNCE)
			self.movetype=MOVETYPE_BOUNCE;
		self.angles_z=0;
		self.avelocity_y=random(200,400);
		MakeSolidCorpse();
	}
}



/*
---------------
LOOKING
---------------
*/
/*
void raven_look_left_low (void)
{
	self.frame=$lowlft1;
	self.th_stand();
}

void raven_look_right_low (void)
{
	self.frame=$lowrght1;
	self.th_stand();
}

void raven_look_left_high (void)
{
	self.frame=$upleft1;
	self.th_stand();
}

void raven_look_right_high (void)
{
	self.frame=$uprght1;
	self.th_stand();
}

void raven_look_down (void)
{
	self.frame=$down1;
	self.th_stand();
}
*/

/*
---------------
EATING
---------------
*/
void peck_effect ()
{
	makevectors(self.angles);
	traceline(self.origin,self.origin+v_forward*16,FALSE,self);
	SpawnPuff(trace_endpos,'0 0 20' , random(3,10),trace_ent);
}

void raven_peck1 (void) [++ $1stpec8 .. $1stpec20]
{
	if(random()<0.2)
		peck_effect();
	if(cycle_wrapped)
	{
		if(random()<0.2)
		{
			self.think=raven_peck_up1;
			thinktime self : 0;
		}
		else if(random()<0.5)
		{
			self.think=raven_peck_up_all;
			thinktime self : 0;
		}
	}
	else if(random()<0.2)
	{
		self.think=raven_peck2;
		thinktime self : 0;
	}
}

void raven_peck2 (void) [++ $2ndpec1 .. $2ndpec10]
{
	if(random()<0.2)
		peck_effect();
	if(cycle_wrapped)
	{
		if(random()<0.2)
		{
			self.think=raven_peck_up2;
			thinktime self : 0;
		}
		else if(random()<0.5)
		{
			self.think=raven_peck_up_all;
			thinktime self : 0;
		}
	}
	else if(random()<0.2)
	{
		self.think=raven_peck1;
		thinktime self : 0;
	}
}

void raven_peck_down (void) [++ $1stpec1 .. $1stpec7]
{
	if(cycle_wrapped)
	{
		self.think=raven_peck1;
		thinktime self : 0;
	}
}

void raven_peck_up_all (void) [-- $1stpec7 .. $1stpec1]
{
	if(cycle_wrapped)
	{
		if(random()<0.7)
			self.think=raven_stand;
		else
			self.think=raven_peck_down;
		thinktime self : 0;
	}
}

void raven_peck_up1 (void) [++ $1stpec21 .. $1stpec24]
{
	if(self.frame==$1stpec24)
	{
		self.think=raven_peck_down1;
		thinktime self : random(0.3,1.3);
	}
}

void raven_peck_up2 (void) [++ $2ndpec11 .. $2ndpec13]
{
	if(self.frame==$2ndpec13)
	{
		self.think=raven_peck_down2;
		thinktime self : random(0.3,1.3);
	}
}

void raven_peck_down1 (void) [-- $1stpec24 .. $1stpec21]
{
	if(cycle_wrapped)
	{
		if(random()<0.5)
			self.think=raven_peck1;
		else
			self.think=raven_peck2;
		thinktime self : 0;
	}
}

void raven_peck_down2 (void) [-- $2ndpec13 .. $2ndpec11]
{
	if(cycle_wrapped)
	{
		if(random()<0.5)
			self.think=raven_peck1;
		else
			self.think=raven_peck2;
		thinktime self : 0;
	}
}

/*QUAKED monster_raven (1 0 0) (-16 -16 0) (16 16 56) 
Scavenger black bird of ill portent.
"Nevermore!"
*/
void() monster_raven =
{
	if (!self.flags2&FL_SUMMONED)
	{
		precache_model4 ("models/raven.mdl");//converted for MP
		precache_model2 ("models/fether.mdl");
		precache_sound2 ("raven/squawk.wav");
//		precache_sound2 ("raven/nevermor.wav");
	}
	self.solid = SOLID_SLIDEBOX;
	self.takedamage=DAMAGE_YES;
	self.thingtype=THINGTYPE_FLESH;
	self.movetype = MOVETYPE_STEP;
	self.view_ofs = '0 0 17';
	self.speed=10;
	self.yaw_speed = 10;
	self.health = 10;
	self.mass = 1;
	self.mintel=5;

	self.th_stand=raven_stand;
	self.th_walk=raven_hop;
	self.th_run=raven_glide;
//	self.th_pain=raven_pain;
	self.th_melee=raven_peck_down;
	self.th_missile=raven_shit;
	self.th_jump=raven_takeoff;
	self.th_die=raven_die;

	setmodel (self, "models/raven.mdl");

	setsize (self, '-7 -7 0', '7 7 20');

	walkmonster_start();
};
