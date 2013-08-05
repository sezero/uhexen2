/*
==================================================================
MG_AI.HC

Michael Gummelt Artificial Intelligence Routines!!!
US Patent# 2.56734376314532533 + E17
Hoo-hah!
==================================================================
*/

/*
==============================================

GENERAL

==============================================
*/
/*
=============
get_visibility

Checks for drf_translucent and abslight of an object,
and uses that and it's world lighting value to set it's
visibility value
>=1 = Totally visible (default)
.
.
.
<=0 = Totally invisible
This value should be used in monster aiming as well.

NOTE: This only works on players since light_level info
	is taken from player's weaponmodel lighting (0-255)
=============
*/
void get_visibility (entity targ , float range_mod)
{
//NOTE: incorporate distance?
float base, divider, attack_mod;

//FIXME: .light_level gives a value of 0 if MLS_POWERMODE is on...
//Temp fix for now...
	if(targ.classname!="player"||targ.drawflags&MLS_POWERMODE)
	{
		targ.visibility=1;
		return;
	}

	if(targ.effects&EF_NODRAW)
	{
		targ.visibility=0;
		return;
	}

	if(targ.drawflags&DRF_TRANSLUCENT&&targ.frozen<=0)
	{
		if(targ.model=="models/assassin.mdl")
			divider=3+targ.level;//Bonus for hiding in shadows
		else
			divider=3;	//Makes it 3 times harder to see
	}
	else
		divider=1;

	if(targ.drawflags&MLS_ABSLIGHT)//&&targ.frozen<=0)
		base=targ.abslight/2.5;
	else
		base=targ.light_level/75;//75 is semi-fullbright

	if(range_mod)
		range_mod=vlen(targ.origin-self.origin)/333;
	else
		range_mod = 1;

	if(targ.last_attack>time - 3)//Remember where they were when fired
		attack_mod=time - targ.last_attack;

	targ.visibility=base/divider/range_mod + attack_mod;
}

/*
=============
float visibility_good (entity targ,float chance_mod)
MG
Does a random check to see if self can see the target based
it's visibility (calls get_visibility for that targ first)
The higher the chance_mod, the lower the chance of
good visibility.
=============
*/
float visibility_good (entity targ,float chance_mod)
{
	if(!targ)
		return FALSE;

	if(targ.frozen>0)
		return TRUE;

	get_visibility(targ,TRUE);

	if(self.classname=="monster_mezzoman")
		if(targ.velocity=='0 0 0')
			chance_mod/=2;//Night vision!
		else
			chance_mod/=5;//Night vision and cats are beter at seeing slight movement

	if(random(chance_mod)<targ.visibility)
		return TRUE;

	return FALSE;
}

/*
=============
float FindMonsterTarget ()
MG

Called by FindTarget, checks for anything alive and
visible within range and sets it as enemy (as long as
it's not the monster's controller, or has the same
controller).  Returns TRUE if it finds something,
FALSE if not.
=============
*/
float FindMonsterTarget ()
{
entity found;
float okay;
	if(self.controller.enemy!=world&&self.controller.enemy.flags2&FL_ALIVE&&visible(self.controller.enemy))
	{
		self.enemy=self.controller.enemy;
		return TRUE;
	}

	okay=FALSE;
	found=findradius(self.origin,1000);
	while(found!=world)
	{
		if(found!=self)
			if(found.flags2&FL_ALIVE)
				if(visible(found))
					if(found!=self.controller)
						if(found.controller!=self.controller)
						{
							if(coop)
							{
								if(found.classname!="player")
									okay = TRUE;
							}
							else if(teamplay)
							{
								if(found.team!=self.controller.team)
									okay = TRUE;
							}
							else
								okay = TRUE;
							if(okay)
							{
								self.enemy=found;
								return TRUE;
							}
						}
		found=found.chain;
	}
	if(self.classname=="monster_imp_lord")
		self.enemy=self.controller;
	return FALSE;
}

/*
==================================================================
float CheckJump()
MG
Checks to see if the enemy is not at the same level as monster
or something is blocking the path of the monster.  If there is 
a clear jump arc to the enemy and the monster will not land in
water or lava, the monster will attempt to jump the distance.
==================================================================
*/
float CheckJump ()
{
local vector spot1, spot2, jumpdir;
float jump_height, jumpup, ignore_height;

	makevectors(self.angles);
	jumpdir=normalize(self.goalentity.origin-self.origin);
	jumpdir_z=0;
	jump_height=jumpdir*v_forward;
	if(jump_height<0.3)
	{
//		dprint("jump direction more than 60 degrees off of forward\n");
		return FALSE;
	}

	spot1=self.origin;
	spot2=self.enemy.origin;

	spot1_z=0;
	spot2_z=0;
	if(self.model=="models/yakman.mdl")
	{
		if(vlen(spot2-spot1)>384)//Yakman can't jump too far
		{
//			dprint("too far for yakman to jump\n");
			return FALSE;
		}
		jump_height=12;
	}
	else
		jump_height=16;

	if(pointcontents(spot1+v_forward*24-'0 0 10')!=CONTENT_SOLID)
		ignore_height=TRUE;

	if(self.classname!="monster_mezzoman"&&!self.spiderType&&self.model!="models/yakman.mdl")
		if(vlen(spot1-spot2)>256)
			ignore_height=FALSE;

//also check to make sure you can't walkmove forward
	if(self.jump_flag>time)		//Don't jump too many times in a row
	{
//		dprint("just jumped\n");
		return FALSE;
	}
	if(pointcontents(self.goalentity.origin)!=CONTENT_EMPTY)
	{
//		dprint("goalentity in water or lava\n");
		return FALSE;
	}
	if(!visible(self.goalentity))
	{
//		dprint("can't see goalentity\n");
		return FALSE;
	}
	if(!ignore_height&&self.goalentity.absmin_z+36>=self.absmin_z&&self.think!=SpiderJumpBegin&&self.classname!="monster_mezzoman"&&self.model!="models/yakman.mdl")
	{
//		dprint("not above goalentity, and not spider\n");
		return FALSE;
	}
	if(!self.flags&FL_ONGROUND)
	{
//		dprint("not on ground\n");
		return FALSE;
	}
	if(!self.goalentity.flags&FL_ONGROUND&&self.goalentity.classname!="waypoint")
	{
//		dprint("goalentity in air\n");
		return FALSE;
	}
	if(!infront(self.goalentity))
	{
//		dprint("goalentity not in front\n");
		return FALSE;
	}
	if(vlen(spot1-spot2)>777&&!ignore_height)
	{
//		dprint("too far away\n");
		return FALSE;
	}
	if(vlen(spot1-spot2)<=100)//&&self.think!=SpiderMeleeBegin)
	{
//		dprint("too close & not spider\n");
		return FALSE;
	}

	if(self.think==SpiderJumpBegin)
		jump_height=vlen((self.goalentity.absmax+self.goalentity.absmin)*0.5-self.origin)/13;
	else if(self.classname=="monster_mezzoman"||self.model=="models/yakman.mdl")
		if(self.goalentity.absmin_z>=self.absmin_z+36)
		{
			jump_height=vlen((self.goalentity.absmax+self.goalentity.absmin)*0.5-self.origin)/13;
			jumpup=TRUE;
		}
		else if(self.goalentity.absmin_z>self.absmin_z - 36)
		{
			if(ignore_height)
				jump_height=vlen((self.goalentity.absmax+self.goalentity.absmin)*0.5-self.origin)/13;
			else
			{
//				dprint("Mez/Yak: Goal not above and not below\n");
				return FALSE;
			}
		}

	spot1=self.origin;
	spot1_z=self.absmax_z;
	spot2=spot1;
	spot2_z+=36;

	traceline(spot1, spot2,FALSE,self);

	if(trace_fraction<1)
	{
//		dprint("not enough room above\n");
		return FALSE;
	}

	if(!jumpup)
	{
//		spot1+=normalize(v_forward)*((self.maxs_x+self.maxs_y)*0.5);
		spot1+=jumpdir*((self.maxs_x+self.maxs_y)*0.5);

		traceline(self.origin, spot1 + '0 0 36',FALSE,self);

		if(trace_fraction<1)
		{
//			dprint("not enough room in front\n");
			return FALSE;
		}

//		traceline(spot1,spot1+jumpdir*64 - '0 0 500',FALSE,self);
		tracearea(spot1,spot1+jumpdir*64 - '0 0 500','-8 -8 0','8 8 4',FALSE,self);

		if(pointcontents(trace_endpos)==CONTENT_WATER||pointcontents(trace_endpos)==CONTENT_SLIME||pointcontents(trace_endpos)==CONTENT_LAVA)
		{
//			dprint("won't jump in water\n");
			return FALSE;
		}
	}

	ai_face();
//	self.ideal_yaw=jumpdir_y;
//	ChangeYaw();
	if(self.think!=SpiderJumpBegin)
	{
		self.jump_flag=time + 7;	//Only try to jump once every 7 seconds
		SightSound();
		if(!jumpup)
		{
			self.velocity=jumpdir*jump_height*18*self.scale;//was 17
			self.velocity_z = jump_height*14*self.scale;//was 12
		}
		else
		{
			self.velocity=jumpdir*jump_height*14*self.scale;//was 10
			self.velocity_z = jump_height*17*self.scale;//was 14
		}
		if(self.model!="models/yakman.mdl")
			self.flags(-)FL_ONGROUND;
		else
			self.level=TRUE;

		if(self.th_jump)
		{
			self.think=self.th_jump;
			thinktime self : 0;
		}
		else
			thinktime self : 0.3;
	}
	else
	{
		self.level=jump_height;
		return TRUE;
	}
}

/*
====================================================================
void MonsterCheckContents ()
MG
Monsters check to see if they're in lava or under water and
do damage do themselves if appropriate.
void do_contents_dam ()
Just spawns a temporary ent to damage self, using T_Damage on
self does weird stuff- won't kill self, just become invincible
====================================================================
*/
void do_contents_dam ()
{
	T_Damage(self.enemy,world,world,self.dmg);
	if(self.dmg==5)
	{
		self.classname="contents damager";
		setorigin(self,self.enemy.origin+self.enemy.view_ofs);
		DeathBubbles(1);
	}
	remove(self);
}

void MonsterCheckContents ()
{
	if(random()>0.3)
		return;

	if(pointcontents(self.origin)==CONTENT_LAVA)
	{
		if(self.flags2&FL2_FIREHEAL)
		{
			if(self.health<self.max_health)
				self.health+=1;
		}
		else
		{
			newmis=spawn();
			newmis.think=do_contents_dam;
			newmis.enemy=self;
			newmis.dmg=30;
			thinktime newmis : 0;
		}
	}
	if(self.movetype==MOVETYPE_SWIM||self.model=="models/skullwiz.mdl"||self.netname=="golem"||self.classname=="monster_pirhana")
		return;
	if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
	{
		if(self.air_finished<time)	//Start drowning
		{
//			dprint("drowning!\n");
			newmis=spawn();
			newmis.think=do_contents_dam;
			newmis.enemy=self;
			newmis.dmg=5;
			thinktime newmis : 0;
		}
	}
	else
		self.air_finished=time+12;
}

/*
====================================================================
void pitch_roll_for_slope (vector slope, entity forwhom)
(My personal favorite!)
MG
This will adjust the pitch and roll of a monster to match
a given slope - if a non-'0 0 0' slope is passed, it will
use that value, otherwise it will use the ground underneath
the monster.  If it doesn't find a surface, it does nothinh\g
and returns.
====================================================================
*/
void pitch_roll_for_slope (vector slope, entity forwhom)
{
//vector new_angles,new_angles2,old_forward,old_right;
//float dot,mod;

	if(slope=='0 0 0')
	{
		traceline(forwhom.origin,forwhom.origin-'0 0 300',TRUE,forwhom);
		if(trace_fraction>0.05&&forwhom.movetype==MOVETYPE_STEP)
			forwhom.flags(-)FL_ONGROUND;
		if(trace_fraction==1)
			return;
		slope=trace_plane_normal;
	}

	matchAngleToSlope(slope, forwhom);//Done in C
/*
	makevectors(forwhom.angles);
	old_forward=v_forward;
	old_right=v_right;

	new_angles=vectoangles(slope);
	new_angles_x=(90-new_angles_x)*-1;//Gets actual slope
	new_angles2='0 0 0';
	new_angles2_y=new_angles_y;

	makevectors(new_angles2);

	mod=v_forward*old_right;
	if(mod<0)
		mod=1;
	else
		mod=-1;

	dot=v_forward*old_forward;
	forwhom.angles_x=dot*new_angles_x;
	forwhom.angles_z=(1-fabs(dot))*new_angles_x*mod;
*/
}

/*
==============================================

IMP

==============================================
*/

/*
================================================================
checkenemy()
Checks to see if enemy is of the same monstertype and old enemy
is alive and visible.  If so, changes back to it's last enemy.
================================================================
*/
void checkenemy (void)
{
entity oldtarget;
/*	if(self.enemy==world)
	{
		if(!LocateTarget())
		{
			if(self.controller.classname=="player")
				self.enemy=self.controller;
			else
			{
				self.enemy=world;
				self.think=self.th_stand;
			}
		}
		self.goalentity=self.enemy;
		return;
	}
*/
	if(self.enemy.classname=="player"&&self.enemy.flags2&FL_ALIVE&&self.enemy!=self.controller)
		return;

	if (!self.enemy.flags2&FL_ALIVE||self.enemy==self.controller)
	{
		if(self.controller.classname=="player")
		{
			self.enemy = self.controller;
			self.goalentity=self.enemy;
		}
		else
			self.enemy = world;

		if (self.oldenemy.flags2&FL_ALIVE)
		{
			self.enemy = self.oldenemy;
			self.goalentity = self.enemy;
			self.think = self.th_run;
		}
		else if(LocateTarget())
		{
			self.goalentity = self.enemy;
			self.think = self.th_run;
		}
		else
		{
			if(self.controller.classname=="player")
				self.goalentity=self.enemy=self.controller;
			else
				self.goalentity=self.enemy=world;

			if (self.pathentity)
				self.think=self.th_walk;
			else
				self.think=self.th_stand;
		}
		thinktime self : 0;
		return;
	}

	if(self.classname=="monster_imp_lord")
		return;

	if(self.oldenemy.classname=="player"&&(self.oldenemy.flags2&FL_ALIVE)&&visible(self.oldenemy))
	{
		if((self.model=="models/spider.mdl"||self.model=="models/scorpion.mdl")&&self.enemy.model==self.model)
			self.enemy=self.oldenemy;
		else
		{
			oldtarget=self.enemy;
			self.enemy=self.oldenemy;
			self.oldenemy=oldtarget;
		}
		self.goalentity=self.enemy;
	}
}

/*
================================================================
float fov(entity targ,entity from,float scope)

Field-Of-View

Returns TRUE if vector from entity "from" to entity "targ" is
within "scope" degrees of entity "from"'s forward angle.
================================================================
*/
float fov(entity targ,entity from,float scope)
{
vector spot1,spot2;
float dot;
	spot1=from.origin+from.proj_ofs;

	spot2=(targ.absmin+targ.absmax)*0.5;

	if(from.classname=="player")
		makevectors(from.v_angle);
	else
		makevectors(from.angles);

//	scope=1 - (scope/180);//converts angles into %
	dot=normalize(spot2-spot1)*v_forward;
	dot=180 - (dot*180);
//	dprintf("FOV value : %s\n",dot);
	if(dot<=scope)
		return TRUE;

	return FALSE;
}

/*
================================================================
check_pos_enemy()
MG
Checks to see if enemy is visible, if so, remember the spot for
waypoints, else set your waypoint at the last spot you saw him.
Also resets search_time timer if you see him.
================================================================
*/
void check_pos_enemy ()
{
	if(!self.mintel)
		return;

	if(!visible(self.enemy))
	{
		self.attack_state = AS_STRAIGHT;
		SetNextWaypoint();
		if(self.model=="models/imp.mdl")	//Imps keep looking in general area for a while
			if(self.search_time<time&&self.goalentity==self.enemy&&self.trigger_field.classname=="waypoint")
				self.goalentity=self.trigger_field;
	}
	else
	{
		if(self.model=="models/imp.mdl")
			self.search_time=time+5;	//If lose sight, keep searching for 5 secs
		self.goalentity=self.enemy;
		self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
	}
}

/*
================================================================
float clear_path (entity targ,float whole_body)
MG
returns TRUE if there is a clear shot or path between self
and "targ".  "whole_body" TRUE will check for a path.
================================================================
*/
float clear_path (entity targ,float whole_body)
{
vector destiny,org;
	if(targ==world)
		return FALSE;

	destiny=targ.origin+targ.proj_ofs;

	if(self.attack_state!=AS_FERRY)
		self.attack_state = AS_STRAIGHT;

	if(whole_body)
	{
		org=(self.absmin+self.absmax)*0.5;
//		tracearea (org, destiny, '-16 -16 0','16 16 28',FALSE,self);
		tracearea (org, destiny, self.mins,self.maxs,FALSE,self);
	}
	else
	{
		org=self.origin+self.proj_ofs;
		traceline (org, destiny,FALSE,self);
	}

	if(!whole_body&&trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, destiny, FALSE, trace_ent);

	if (trace_ent == targ)
		return TRUE;

	if(whole_body)
	{
		if(self.attack_state!=AS_FERRY)
			self.attack_state = AS_SLIDING;
		return FALSE;
	}

	if(trace_ent.classname=="player"&&targ.classname=="player")
		return TRUE;

	if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
	{//Don't have a clear shot, and don't want to shoot obstruction
		self.attack_state = AS_SLIDING;
		return FALSE;
	}

	return TRUE;
}

/*
================================================================
check_view(entity targ,vector org,vector dir,float dist,float interval)
MG
Will see if it can see the targ entity along the dir
given to it- used to determine which direction a monster
should move in to get a clear line of sight to the targ.
Returns the distance it took to see targ, FALSE if couldn't.
================================================================
*/
float check_view(entity targ,vector org,vector dir,float dist,float interval)
{
float dist_counter;
	newmis=spawn();
	dir=normalize(dir);
	while(dist_counter<dist)
	{
		dist_counter+=interval;
		setorigin(newmis,org+dir*dist_counter);
		if(visible2ent(targ,newmis))
		{
			traceline (newmis.origin,(targ.absmin+targ.absmax)*0.5,FALSE,self);
			if (trace_ent == targ)
			{
				remove(newmis);
				return dist_counter;
			}
		}
	}
	remove(newmis);
	return FALSE;
}

/*
================================================================
vector check_axis_move (vector checkdir,float minspeed,float maxspeed)
MG
Calls check_view for enemy along given vector, and if it
fails, checks again along opposite direction on that vector.
If check_view is successful, returns the vector*the distance
along that vector at which it found the goalentity, this
"speed" maxes out at "maxpeed" and is at least "minspeed".
Not used for waypoint or path navigation, only for going
after enemies.
================================================================
*/
vector check_axis_move (vector checkdir,float minspeed,float maxspeed)
{
float go_dist;
	checkdir=normalize(checkdir);
	if(random()<0.5)
		checkdir=checkdir*-1;
	go_dist=check_view(self.enemy,self.origin+self.view_ofs,checkdir,500,30);
	if(!go_dist&&random()<0.5)
	{
		checkdir*=-1;
		go_dist=check_view(self.enemy,self.origin+self.view_ofs,checkdir,500,30);
	}
	if(go_dist)
	{
		if(go_dist>maxspeed)
			go_dist=maxspeed;
		else if(go_dist<minspeed)
			go_dist=minspeed;
		checkdir=checkdir*go_dist;
		return checkdir;
	}
	return '0 0 0';
}

/*
================================================================
float check_z_move(float maxdist)
MG
Intended for flying monsters, will try to move up if enemy
or goalentity is above monster, down if it is below.  Uses
movestep, not velocity.
Will move a maximum step of "maxdist".
Returns FALSE if blocked, TRUE if movement completed.
================================================================
*/
float check_z_move(float maxdist)
{
float goaldist;
entity targ;
	if(self.enemy!=world&&visible(self.enemy))
		targ=self.enemy;
	else if(self.goalentity!=world)
		targ=self.goalentity;
	else
		return FALSE;

/*What does this do?
	if(fabs(targ.origin_z-self.origin_z)<48&&!visible(targ))
		return FALSE;	//FIXME: Find an up or down
*/

	if(targ.origin_z!=self.absmin_z)
	{
		goaldist=(targ.absmin_z+targ.absmax_z)*0.5-(self.absmax_z+self.absmin_z)*0.5;
		maxdist=fabs(maxdist);
		if(fabs(goaldist)>maxdist)
		if(goaldist>0)
			goaldist=maxdist;
		else
			goaldist=maxdist*-1;
		if(!movestep(0,0,goaldist, FALSE))
			return FALSE;
	}
	return TRUE;
}

/*
====================================================================

MEDUSA

====================================================================
*/

/*
============================================================
float lineofsight(entity targ, entity from)
MG
Traces a line along "from"'s view_ofs along v_forward and returns
VRAI if it hits targ, FAUX if not, mon ami.
============================================================
*/
float lineofsight(entity targ, entity from)
{
//FIXME: account for monster's lack of pitch if z diff
vector org,dir;
	if(from.classname=="player")
		makevectors(from.v_angle);
	else if(from.classname=="monster_medusa")
		makevectors(from.angles+from.angle_ofs);
	else
		makevectors(from.angles);

	org=from.origin+from.view_ofs;

	dir=normalize(v_forward);

	traceline(org, org+dir*1000,FALSE,from);

	if(trace_ent!=targ)
		return FALSE;
	else
	{
//		dprint("Line of sight from ");
//		dprint(from.classname);
//		dprint(" to ");
//		dprint(targ.classname);
//		dprint(" confirmed\n");
		return TRUE;
	}
}

/*
====================================================================

EIDOLON

====================================================================
*/

/*
=====================================================
entity riderpath_findbest(entity subject_path)
MG
Returns closest rider path corner that "subject_path"
leads to.  Used for Rider Bosses.
=====================================================
*/
entity riderpath_findbest(entity subject_path)
{
entity search,found,best_path;
float next,num_points,position,bestdist,lastdist;

	num_points = 0;
	if (subject_path.next_path_1) 
		num_points += 1;
	if (subject_path.next_path_2) 
		num_points += 1;
	if (subject_path.next_path_3) 
		num_points += 1;
	if (subject_path.next_path_4) 
		num_points += 1;
	if (subject_path.next_path_5) 
		num_points += 1;
	if (subject_path.next_path_6) 
		num_points += 1;

	if (!num_points)
	{
		dprintf("rider path %s has no next points\n",subject_path.path_id);
		remove(self);
		return world;
	}

	bestdist=vlen(self.goalentity.origin-self.origin);
	lastdist=bestdist;
	position=0;
	best_path=world;
	while(position<num_points)
	{
		position+=1;
		if (position==1) 
			next = subject_path.next_path_1;
		else if (position==2) 
			next = subject_path.next_path_2;
		else if (position==3) 
			next = subject_path.next_path_3;
		else if (position==4) 
			next = subject_path.next_path_4;
		else if (position==5) 
			next = subject_path.next_path_5;
		else if (position==6) 
			next = subject_path.next_path_6;
		found = world;
		search = find(world, classname, "rider_path");
		while(search != world && found == world)
		{
			if (search.path_id == next) 
				found = search;
			else
				search = find(search, classname, "rider_path");
		}

		if (!found)
		{
			dprintf("Could not find rider path %s\n",next);
			remove(self);
			return world;
		}
		else
		{
			lastdist=vlen(self.goalentity.origin-found.origin);
			if(lastdist<bestdist)
			{
				best_path=found;
				bestdist=lastdist;
			}
		}
	}
	if (!best_path)
		return world;
	return best_path;
}

/*
=====================================================================

MEZZOMAN
(Personal Favorite!)

=====================================================================
*/

/*
=========================================================
entity look_projectiles ()
MG
WARNING!  Expensive, should not be called often at all!
Searches a radius 1000 around self to find projectiles
(anything with movetype bounce, flymissile, bouncemissile)
then checks to see if it's heading at self (see "heading"
function in PROJBHVR.HC).  It finds the closest of these
and returns that entity.  If it finds nothing, returns
"world".
=========================================================
*/
entity look_projectiles ()
{
entity found, enemy_proj;
float dist, bestdist;

	found=findradius(self.origin,1000);
	bestdist=1001;
	while(found)
	{
		if(found.movetype==MOVETYPE_FLYMISSILE||found.movetype==MOVETYPE_BOUNCE||found.movetype==MOVETYPE_BOUNCEMISSILE)
		if(visible(found))
		{
			dist=vlen(found.origin-self.origin);
			if(dist<bestdist)
			{
				if(heading(self,found,0.9))//Try this small range for heading, but problem is, they won't split up & surround you as much...
				{
					bestdist=dist;
					enemy_proj=found;
				}
			}
		}
		found=found.chain;
	}
	if(enemy_proj)
	{
		self.level=bestdist/vlen(enemy_proj.velocity);
		return enemy_proj;
	}
	else
		return world;
}


/*
======================================================
float solid_under(vector startpos , vector endpos)
MG
Will check in increments of 5 pixels along a given
path from startpos to endpos if there is solid gound
at least 18 pixels below (18 is the monster step-height).
If so, returns TRUE, if there is a gap anywhere along
that, returns FALSE.
Used mainly by Mezzomen to see if they should jump
across gaps at enemy.
======================================================
*/
float solid_under(vector startpos , vector endpos)
{
float diff_count;
vector dir;
	dir=normalize(endpos-startpos);
	diff_count=vlen(endpos-startpos)/5;
	while(diff_count>0)
	{
		traceline(startpos,startpos-'0 0 18',TRUE,self);
		if(trace_fraction==1)
			return FALSE;
		startpos+=dir*5;
		diff_count-=1;
	}
	return TRUE;
}

/*
======================================================
float check_heading_left_or_right (entity object)
MG
Will check to see if the given object will be to
the left or the right of self once it gets to
self.  Uses it's current position and extrapolates
based on it's heading (velocity).
Will return:
1  = left
-1 = right
0  = neither.
Special case: If called by a monster that's not
	awake, will return opposite of these assuming
	that the monster wants to cut off player-
	only used by the Rolling Ambush Mezzomen.
======================================================
*/
float check_heading_left_or_right (entity object)
{
vector	spot1, spot2, vec;
float dot, rng, reverse;

	makevectors (self.angles);
	spot1 = self.origin + self.view_ofs;
	spot2 = object.origin;
//To get the eventual location of the projectile when it gets to him...
	rng=vlen(spot1-spot2);
	spot2+=normalize(object.velocity)*(rng+15);//Add a bit for good measure

	vec = normalize (spot2 - spot1);
//FIXME? What about behind me?

	if(object.classname=="player"&&!self.monster_awake)
	{
		self.monster_awake=TRUE;
		sound(self,CHAN_VOICE,self.noise,1,ATTN_NORM);
		reverse=-1;
	}
	else
		reverse=1;

	dot = vec * v_right;
	if ( dot > 0)
		return -1*reverse;

	dot = vec * (v_right*-1);
	if ( dot > 0)
		return 1*reverse;

	return 0;
}

/*
======================================================
float navigate (float walkspeed)
MG
Checks to see which side of the entity is blocked
and will move in the opposite direction using
walkmove (for left-right) or movestep (for up-down)
if it can.  Will move the specified distance.
If it can't move that way or it doesn't find a blocked
side, it returns false.

Meant for use with flying and swimming monsters
because movetogoal doesn't make them navigate!
======================================================
*/
float navigate (float walkspeed)
{
vector checkdir,org,new_angle;
float vert_size,horz_size;
	makevectors(self.angles);
	checkdir=v_right;
	org=self.origin+checkdir*self.size_x;
	vert_size=self.size_z/2;
	horz_size=(self.size_x+self.size_y)/4;
	traceline(org,org+v_forward*horz_size,FALSE,self);
	if(trace_fraction==1&&!trace_allsolid)
	{
		checkdir=v_right*-1;
		org=self.origin+checkdir*horz_size;
		traceline(org,org+v_forward*horz_size,FALSE,self);
	}
	if(self.flags&FL_FLY||self.flags&FL_SWIM)
	{
		if(trace_fraction==1&&!trace_allsolid)
		{
			checkdir=v_up;
			org=self.origin+checkdir*vert_size;
			traceline(org,org+v_forward*horz_size,FALSE,self);
		}
		if(trace_fraction==1&&!trace_allsolid)
		{
			checkdir=v_up*-1;
			org=self.origin+checkdir*vert_size;
			traceline(org,org+v_forward*horz_size,FALSE,self);
		}
	}
	if(trace_fraction<1||trace_allsolid)
	{
		if(checkdir==v_right||checkdir==v_right*-1)
		{
			new_angle=vectoangles(checkdir*-1);
			if(!walkmove(new_angle_y,walkspeed,FALSE))
			{
//				dprint("Couldn't Side-step\n");
				return FALSE;
			}
//			dprint("Side-stepped\n");
			return TRUE;
		}
		if(checkdir==v_up)
			walkspeed*=-1;
		if(!movestep(0,0,walkspeed,FALSE))
		{
//			dprint("couldn't move up/down\n");
			return FALSE;
		}
//		dprint("up-down move\n");
		return TRUE;
	}
//	dprint("Not blocked\n");
	return FALSE;//FOUND NO BLOCKING!!!
}

/*
=============================================================
vector extrapolate_pos_for_speed (vector p1,float pspeed,entity targ,float accept) 
MG
Estimates where the "targ" will be by the time a projectile
travelling at "pspeed" leaving "org" arrives at "targ"'s origin.
It then calculates a new spot to shoot at so that the
projectile will arrive at such spot at the same time as
"targ".  Will return '0 0 0' (FALSE) if there is not a clear
line of fire to the spot or if the new vector is out of the
acceptable range (based on dot product of original vec and
the new vec).
=============================================================
*/
vector extrapolate_pos_for_speed (vector p1,float pspeed,entity targ,float accept) 
{
float dist1,dist2,tspeed,dot,eta1,eta2,eta_delta,failed;
vector p2,p3,targ_dir,vec1,vec2;

//	dprint("Extrapolating\n");
	p2=targ.origin+targ.view_ofs;		//current target viewport
	vec1=p2 - p1;						//vector to p2
	dist1=vlen(vec1);					//distance to p2
	vec1=normalize(vec1);				//direction to p2
	targ_dir=targ.velocity;				//target velocity
	tspeed=vlen(targ_dir);				//target speed
	targ_dir=normalize(targ_dir);		//target direction

	eta1=dist1/pspeed;					//Estimated time of arrival of projectile to p2

	p3=p2 + targ_dir * tspeed * eta1;	//Extrapolated postion of targ at time + eta1
	dist2=vlen(p3-p1);					//new distance to p3

	eta2=dist2/pspeed;					//ETA of projectile to p3
	eta_delta=eta2-eta1;				//change in ETA's

	p3+=targ_dir*tspeed*eta_delta*random();//Add any diff in ETA to p3's location,random a little in case they slow down

	traceline(p1,p3,FALSE,self);
	if(trace_fraction<1)
	{
		if(trace_ent.thingtype>=THINGTYPE_WEBS)
			traceline (trace_endpos, p3, FALSE, trace_ent);
		if(trace_fraction<1)
			if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
			{//Don't have a clear shot, and don't want to shoot obstruction
	//			dprint("No clear shot\n");
				self.attack_state = AS_SLIDING;
				failed=TRUE;
			}
	}

	vec2=normalize(p3-p1);				//New vector to p3
	dot=vec1*vec2;
	if(dot<accept)						//Change in dir too great
	{
//		dprint("Out of range\n");
		failed=TRUE;
	}

	if(failed)
		p3='0 0 0';
//	else
//		dprint("Successful extrapolation\n");

/*Was using this to show blue for failed extrap, red for succ.
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_STREAM_COLORBEAM);	//beam type
	WriteEntity (MSG_BROADCAST, self);				//owner
	WriteByte (MSG_BROADCAST, 0);					//tag + flags
	WriteByte (MSG_BROADCAST, 20);					//time
	WriteByte (MSG_BROADCAST, failed);			//color

	WriteCoord (MSG_BROADCAST, p1_x);
	WriteCoord (MSG_BROADCAST, p1_y);
	WriteCoord (MSG_BROADCAST, p1_z);

	WriteCoord (MSG_BROADCAST, p3_x);
	WriteCoord (MSG_BROADCAST, p3_y);
	WriteCoord (MSG_BROADCAST, p3_z);
*/
	return p3;
}

/*
=============================================================
vector aim_adjust (entity targ)
MG
Will return a normalized offset vector based on the targ's
light level, used for monster aiming at shadow-hiding players.
=============================================================
*/
vector aim_adjust (entity targ)
{
float ofs;
vector vofs;
	if(!targ)
		return '0 0 0';

	makevectors(self.angles);
	get_visibility(targ,TRUE);
	ofs=(1 - targ.visibility - skill/10)*0.1;
	if(skill<3&&ofs>0)
	{
		vofs=v_up*0.5*random(0-ofs,ofs)+v_right*1.5*random(0-ofs,ofs);
		return vofs;
	}
	return '0 0 0';
}
