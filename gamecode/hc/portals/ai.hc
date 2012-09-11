/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/ai.hc,v 1.4 2007-02-07 16:59:29 sezero Exp $
 */
void(entity etemp, entity stemp, entity stemp, float dmg) T_Damage;
/*

.enemy
Will be world if not currently angry at anyone.

.pathcorner
The next path spot to walk toward.  If .enemy, ignore .pathcorner
When an enemy is killed, the monster will try to return to it's path.

.hunt_time
Set to time + something when the player is in sight, but movement straight for
him is blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.ideal_yaw
A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active,
this will be the exact line towards the enemy.

.pausetime
A monster will leave it's stand state and head towards it's .pathcorner when
time > .pausetime.

walkmove(angle, speed) primitive is all or nothing
*/

//float ArcherCheckAttack (void);
float MedusaCheckAttack (void);
void()SetNextWaypoint;
void()SpiderMeleeBegin;
void()spider_onwall_wait;
float(entity targ , entity from)infront_of_ent;
void(entity proj)mezzo_choose_roll;
void()multiplayer_health;
void()riderpath_init;
void(float move_speed)riderpath_move;
float(float move_speed)eidolon_riderpath_move;
void() eidolon_guarding;
void()hive_die;
float()eidolon_check_attack;

//void()check_climb;

//
// globals
//
float	current_yaw;

//
// when a monster becomes angry at a player, that monster will be used
// as the sight target the next frame so that monsters near that one
// will wake up even if they wouldn't have noticed the player
//
float	sight_entity_time;
float(float v) anglemod =
{
	while (v >= 360)
		v = v - 360;
	while (v < 0)
		v = v + 360;
	return v;
};

//============================================================================

/*
=============
range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
float(entity targ) range =
{
vector	spot1, spot2;
float		r,melee;	

	if((self.solid==SOLID_BSP||self.solid==SOLID_TRIGGER)&&self.origin=='0 0 0')
		spot1=(self.absmax+self.absmin)*0.5;
	else
		spot1 = self.origin + self.view_ofs;

	if((targ.solid==SOLID_BSP||targ.solid==SOLID_TRIGGER)&&targ.origin=='0 0 0')
		spot2=(targ.absmax+targ.absmin)*0.5;
	else
		spot2 = targ.origin + targ.view_ofs;
	
	r = vlen (spot1 - spot2);

	if (self.classname=="monster_mummy")
		melee = 50;
	else if (self.classname=="monster_yakman"||self.netname=="golem")//longer reach
		melee = 150;
	else
		melee = 100;

	if (r < melee)
		return RANGE_MELEE;
	if (r < 500)
		return RANGE_NEAR;
	if (r < 1000)
		return RANGE_MID;
	return RANGE_FAR;
};

/*
=============
visible2ent

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
void spawntestmarker(vector org, float life, float skincolor)
{
	newmis=spawn_temp();
	newmis.drawflags=MLS_ABSLIGHT;
	newmis.abslight=1;
	newmis.frame=1;
	newmis.skin=skincolor;
	setmodel(newmis,"models/test.mdl");
	setorigin(newmis,org);
	newmis.think=SUB_Remove;
	if(life==-1)
		self.nextthink=-1;
	else
		thinktime newmis : life;
}

float visible2ent (entity targ, entity forent)
{
vector	spot1, spot2;
entity oself;
	if((forent.solid==SOLID_BSP||forent.solid==SOLID_TRIGGER)&&forent.origin=='0 0 0')
		spot1=(forent.absmax+forent.absmin)*0.5;
	else
		spot1 = forent.origin + forent.view_ofs;
		
	if((targ.solid==SOLID_BSP||targ.solid==SOLID_TRIGGER)&&targ.origin=='0 0 0')
		spot2=(targ.absmax+targ.absmin)*0.5;
	else
		spot2 = targ.origin + targ.view_ofs;

    traceline (spot1, spot2, TRUE, forent);   // see through other monsters

/*
	if(forent.classname=="monster_skull_wizard"&&trace_fraction==1&&self.think==self.th_stand)
	{
		dprint("Skullwizard awakened by: ");
		dprint(targ.classname);
		dprint("\n");
		forent.nextthink=-1;
		forent.think=SUB_Null;
		forent.th_run=self.th_stand;
		forent.effects=EF_BRIGHTLIGHT;
		if(targ.classname!="player")
		{
			targ.nextthink=-1;
			targ.think=SUB_Null;
			targ.th_run=targ.th_stand;
			targ.effects=EF_BRIGHTLIGHT;
		}
	}
*/

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, TRUE, trace_ent);
//	else if (trace_inopen && trace_inwater)//FIXME?  Translucent water?
//		return FALSE;			// sight line crossed contents

//	if (trace_allsolid)
//		dprint("trace all solid\n");

//	dprint(targ.classname);

	if (trace_fraction == 1)
	{
		if(forent.flags&FL_MONSTER)
		{
			oself=self;
			self=forent;
			if(visibility_good(targ,0.15 - skill/20))
			{
//				dprint("a monster, with good visible\n");
				self=oself;
				return TRUE;
			}
			self=oself;
		}
		else
		{
/*			spawntestmarker(spot1, -1, 0);
			dprintv("spot1%s\n", spot1);
			spawntestmarker(spot2, -1, 0);
			dprintv("spot2%s\n", spot2);
			dprint("not a monster, but visible\n");*/
			return TRUE;
		}
	}

	return FALSE;
}

/*
=============
infront_of_ent

returns 1 if the targ is in front (in sight) of from
=============
*/
float infront_of_ent (entity targ , entity from)
{
	vector	vec,spot1,spot2;
	float	accept,dot;

	if(from.classname=="player")
	    makevectors (from.v_angle);
	else if(from.classname=="monster_medusa")
		makevectors (from.angles+from.angle_ofs);
	else
	    makevectors (from.angles);

	if((from.solid==SOLID_BSP||from.solid==SOLID_TRIGGER)&&from.origin=='0 0 0')
		spot1=(from.absmax+from.absmin)*0.5;
	else
		spot1 = from.origin + from.view_ofs;

	spot2=(targ.absmax+targ.absmin)*0.5;

    vec = normalize (spot2 - spot1);
	dot = vec * v_forward;

    accept = 0.3;
	
    if ( dot > accept)
		return TRUE;
	return FALSE;
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
float visible (entity targ)
{
	return visible2ent(targ,self);
}

/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
float infront (entity targ)
{
	return infront_of_ent(targ,self);
}


//============================================================================

/*
===========
ChangeYaw

Turns towards self.ideal_yaw at self.yaw_speed
Sets the global variable current_yaw
Called every 0.1 sec by monsters
============
*/
/*

void ChangeYaw ()
{
float		ideal, move;

//current_yaw = self.ideal_yaw;
// mod down the current angle
	current_yaw = anglemod( self.angles_y );
	ideal = self.ideal_yaw;


	if (current_yaw == ideal)
		return;
	
	move = ideal - current_yaw;
	if (ideal > current_yaw)
	{
		if (move > 180)
			move = move - 360;
	}
	else
	{
		if (move < -180)
			move = move + 360;
	}
		
	if (move > 0)
	{
		if (move > self.yaw_speed)
			move = self.yaw_speed;
	}
	else
	{
		if (move < 0-self.yaw_speed )
			move = 0-self.yaw_speed;
	}

	current_yaw = anglemod (current_yaw + move);

	self.angles_y = current_yaw;
}

*/


//============================================================================

void() HuntTarget =
{
	self.goalentity = self.enemy;
	self.think = self.th_run;
//	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
	thinktime self : 0.1;
//	SUB_AttackFinished (1);	// wait a while before first attack
};

void SightSound (void)
{
	if (self.classname == "monster_archer")
		sound (self, CHAN_VOICE, "archer/sight.wav", 1, ATTN_NORM);
	else if (self.classname == "monster_archer_lord")
		sound (self, CHAN_VOICE, "archer/sight2.wav", 1, ATTN_NORM);
	else if (self.classname == "monster_mummy")
		sound (self, CHAN_WEAPON, "mummy/sight.wav", 1, ATTN_NORM);
	else if (self.classname == "monster_mummy_lord")
		sound (self, CHAN_WEAPON, "mummy/sight2.wav", 1, ATTN_NORM);

}

void() FoundTarget =
{
	if (self.enemy.classname == "player")
	{	// let other monsters see this monster for a while
		sight_entity = self;
		sight_entity_time = time + 1;
	}
	
	self.show_hostile = time + 1;		// wake up other monsters

	SightSound ();

	HuntTarget ();
};

/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client (or fakeclient) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
float(float dont_hunt) FindTarget =
{
entity	client;
float		r;

// if the first spawnflag bit is set, the monster will only wake up on
// really seeing the player, not another monster getting angry

// spawnflags & 3 is a big hack, because zombie crucified used the first
// spawn flag prior to the ambush flag, and I forgot about it, so the second
// spawn flag works as well
	if(!deathmatch&&(self.classname=="monster_imp_lord"||self.classname=="cube_of_force"))
		return FindMonsterTarget();

	if (sight_entity_time >= time&&sight_entity!=world)
	{
		client = sight_entity;
		if (client.enemy == self.enemy)
			return TRUE;
	}
	else
	{
		client = checkclient ();
		if (!client)
			return FALSE;	// current check entity isn't in PVS
	}

	if(self.classname=="monster_imp_lord"&&client==self.controller)
		return FALSE;

	if (client == self.enemy)
		return FALSE;

	if (client.flags & FL_NOTARGET)
		return FALSE;

	r = range (client);
	if (r == RANGE_FAR)
		return FALSE;

	if(!visibility_good(client,5))
	{
//		dprint("Monster has low visibility on ");
//		dprint(client.netname);
//		dprintf("(%s)\n",client.visibility);
		return FALSE;
	}

	if(self.think!=spider_onwall_wait)
		if (r == RANGE_NEAR)
		{
			if (client.show_hostile < time && !infront (client))
				return FALSE;
		}
		else if (r == RANGE_MID)
		{
			if (!infront (client))
				return FALSE;
		}
	
	if (!visible (client))
		return FALSE;

//
// got one
//
	self.enemy = client;

	if (self.enemy.classname != "player")
	{//If this check fails, do not let this entity set self as
	 //sight_ent- avoids daisy-chaining of enemy sight
		self.enemy = self.enemy.enemy;
		if (self.enemy.classname != "player")
		{
			self.enemy = world;
			return FALSE;
		}
		SightSound ();
		HuntTarget ();
		return TRUE;
	}

	if(!dont_hunt)
		FoundTarget ();
	return TRUE;
};

void()SpiderJumpBegin;
//=============================================================================
/*
void(float dist) ai_forward =
{
	walkmove (self.angles_y, dist, FALSE);
};
*/
void(float dist) ai_back =
{
	walkmove ( (self.angles_y+180), dist, FALSE);
};


/*
=============
ai_pain

stagger back a bit
=============
*/
void(float dist) ai_pain =
{
//	ai_back (dist);
float	away;
	
	away = vectoyaw (self.origin - self.enemy.origin)+90*random(0.5,-0.5);
	
	walkmove (away, dist,FALSE);
};

/*
=============
ai_painforward

stagger back a bit
=============
*/
/*
void(float dist) ai_painforward =
{
	walkmove (self.ideal_yaw, dist, FALSE);
};
*/

/*
=============
ai_walk

The monster is walking it's beat
=============
*/
//THE PIT!

float find_enemy_target ()
{
entity found;
	if(self.target=="")
		return FALSE;

	found=find(world, targetname, self.target);
	if(!found)
	{
		found=find(world, targetname, self.targetname);
		if(found)
			if(found.enemy.flags2&FL_ALIVE)
				found=found.enemy;
	}
	if(found==world||!found.flags2&FL_ALIVE)
	{
		if(!found.targetname)
			self.target="";
		return FALSE;
	}
	self.goalentity = self.pathentity = found;
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
	self.enemy=self.goalentity;
	self.th_run();
	return TRUE;
}

void(float dist) ai_walk =
{
	MonsterCheckContents();

	movedist = dist;

	// check for noticing a player
//THE PIT!
	if(world.model=="maps/monsters.bsp")
		if(find_enemy_target())
			return;

	if (FindTarget (FALSE))
		return;

	if(!movetogoal(dist))
	{
		if(trace_ent.solid==SOLID_BSP&&trace_fraction<1)
		{
			if(trace_plane_normal!='0 0 0')
				self.walldir='0 0 0' - trace_plane_normal;
		}
	}
};


/*
=============
ai_stand

The monster is staying in one place for a while, with slight angle turns
=============
*/
void() ai_stand =
{
	MonsterCheckContents();
	
//THE PIT!
	if(world.model=="maps/monsters.bsp")
		if(find_enemy_target())
			return;

	if (FindTarget (FALSE))
		return;
	
	if (time > self.pausetime)
	{
		self.th_walk ();
		return;
	}

// change angle slightly
};

/*
=============
ai_turn

don't move, but turn towards ideal_yaw
=============
*/
/*
void() ai_turn =
{
	if (FindTarget (FALSE))
		return;
	
	ChangeYaw ();
};
*/

//=============================================================================

/*
=============
ChooseTurn
=============
*/
/*
void(vector dest3) ChooseTurn =
{
	local vector	dir, newdir;
	
	dir = self.origin - dest3;

	newdir_x = trace_plane_normal_y;
	newdir_y = 0 - trace_plane_normal_x;
	newdir_z = 0;
	
	if (dir * newdir > 0)
	{
		dir_x = 0 - trace_plane_normal_y;
		dir_y = trace_plane_normal_x;
	}
	else
	{
		dir_x = trace_plane_normal_y;
		dir_y = 0 - trace_plane_normal_x;
	}

	dir_z = 0;
	self.ideal_yaw = vectoyaw(dir);	
};
*/

/*
============
FacingIdeal

Within angle to launch attack?
============
*/
float() FacingIdeal =
{
	local	float	delta;
	
	delta = anglemod(self.angles_y - self.ideal_yaw);
	if (delta > 45 && delta < 315)
		return FALSE;
	return TRUE;
};


//=============================================================================

float()pent_check_attack;
float() CheckAnyAttack =
{
	if (!enemy_vis)
		return FALSE;

	if(self.classname=="monster_eidolon")
		if(self.goalentity==self.controller)
			return FALSE;
		else
			return eidolon_check_attack();

	if(self.classname=="monster_pentacles")
		return pent_check_attack();

	if(self.classname=="monster_medusa")
	{
//		dprint("medusa checking\n");
		return MedusaCheckAttack();
	}

	return CheckAttack ();
};


/*
=============
ai_attack_face

Turn in place until within an angle to launch an attack
=============
*/
void() ai_attack_face =
{
	self.ideal_yaw = enemy_yaw;
	ChangeYaw ();
	if (FacingIdeal())  // Ready to go get em
	{
		if (self.attack_state == AS_MISSILE)
			self.th_missile ();
		else if (self.attack_state == AS_MELEE)
			self.th_melee ();
		self.attack_state = AS_STRAIGHT;
	}
};


/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide ()
{
float	ofs;
	
	self.ideal_yaw = enemy_yaw;
	ChangeYaw ();
	if (self.lefty)
		ofs = 90;
	else
		ofs = -90;
	
	if (walkmove (self.ideal_yaw + ofs, movedist, FALSE))
		return;
		
	self.lefty = 1 - self.lefty;
	
	walkmove (self.ideal_yaw - ofs, movedist, FALSE);
}


/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/

void(float dist) ai_run =
{
	
	MonsterCheckContents();
	
	movedist = dist;
// see if the enemy is dead
	if (!self.enemy.flags2&FL_ALIVE||(self.enemy.artifact_active&ARTFLAG_STONED&&self.classname!="monster_medusa"))
	{
//THE PIT!
	if(world.model=="maps/monsters.bsp")
		if(find_enemy_target())
			return;

		self.enemy = world;
	// FIXME: look all around for other targets
		if (self.oldenemy.health > 0)
		{
			self.enemy = self.oldenemy;
			HuntTarget ();
		}
		else if(coop)
		{
			if(!FindTarget(TRUE))	//Look for other enemies in the area
			{
				if (self.pathentity)
					self.th_walk ();
				else
					self.th_stand ();
				return;
			}
		}
		else
		{
			if (self.pathentity)
				self.th_walk ();
			else
				self.th_stand ();
			return;
		}
	}

	self.show_hostile = time + 1;		// wake up other monsters

// check knowledge of enemy
	enemy_vis = visible(self.enemy);
	if (enemy_vis)
	{
		self.search_time = time + 5;
		if(self.mintel)
		{
			self.goalentity=self.enemy;
		    self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
		}
	}
	else
	{
		if(coop)
		{
			if(!FindTarget(TRUE))
				if(self.model=="models/spider.mdl")
				{
					if(random()<0.5)
						SetNextWaypoint();
				}
				else 
					SetNextWaypoint();
		}
		if(self.mintel)
			if(self.model=="models/spider.mdl")
			{
				if(random()<0.5)
					SetNextWaypoint();
			}
			else 
				SetNextWaypoint();
	}

	if(random()<0.5&&(!self.flags&FL_SWIM)&&(!self.flags&FL_FLY)&&(self.spawnflags&JUMP))
		CheckJump();

// look for other coop players
	if (coop && self.search_time < time)
	{
		if (FindTarget (FALSE))
			return;
	}

	enemy_infront = infront(self.enemy);
	enemy_range = range(self.enemy);
	if(self.classname!="monster_eidolon")
		enemy_yaw = vectoyaw(self.goalentity.origin - self.origin);
	
	if ((self.attack_state == AS_MISSILE) || (self.attack_state == AS_MELEE))  // turning to attack
	{
		if(self.classname!="monster_eidolon")
			ai_attack_face ();
		return;
	}

	if (CheckAnyAttack ())
		return;					// beginning an attack
		
	if (self.attack_state == AS_SLIDING)
	{
		ai_run_slide ();
		return;
	}
		
// head straight in
//	if(self.netname=="spider")
//		check_climb();
	if(self.classname=="monster_eidolon")
	{
		if(!self.path_current)
			riderpath_init();
		if(!eidolon_riderpath_move(dist))
		{
			if(self.think==self.th_run)
				eidolon_guarding();
		}
		else if(self.think==eidolon_guarding)
			self.th_run();
	}
	else if(!movetogoal(dist))
	{
		if(trace_ent.solid==SOLID_BSP&&trace_fraction<1)
		{
		vector movdir;
/*			dprint("RUNNING\n");
			dprintv("Pent hit wall - normal = %s\n",trace_plane_normal);
			dprintv("Origin = %s\n",self.origin);
			dprintv("End_pos = %s\n",trace_endpos);*/
			movdir=normalize(trace_endpos - self.origin);
//			dprintv("Move dir = %s\n",movdir);
			if(trace_plane_normal=='0 0 0')
			{
				traceline(self.origin,self.origin+movdir*64,TRUE,self);
//				dprintv("New normal = %s\n",trace_plane_normal);
			}
			if(trace_plane_normal!='0 0 0')
			{
				self.walldir='0 0 0' - trace_plane_normal;
//				dprintv("New walldir = %s\n",self.walldir);
			}
/*			else
			{
				dprintf("Trace fraction	= %s\n", trace_fraction);
				if(trace_startsolid)
					dprint("Trace started in wall\n");
				else if(trace_allsolid)
					dprint("Trace completely in wall\n");
				else if(trace_ent.solid==SOLID_BSP)
					dprint("Wall in my way\n");
				else
				{
					dprint(trace_ent.classname);
					dprint(" in my way\n");
				}
			}
*/		}
	}
};


void() monster_imp_fire;
void() monster_imp_ice;
void() monster_archer;
void() monster_archer_ice;
void() monster_weresnowleopard;
void() monster_weretiger;
void() monster_yakman;

void() monster_skull_wizard;
void() monster_scorpion_black;
void() monster_scorpion_yellow;
void() monster_spider_red_large;
void() monster_spider_red_small;
void() monster_spider_yellow_large;
void() monster_spider_yellow_small;
void(vector org, entity death_owner) spawn_tdeath;

void spawnspot_activate (void)
{
	self.deadflag=FALSE;//it's that simple!
}

float monster_spawn_precache_mp ()
{
float have_monsters;
	if(self.spawnflags&ICE_ARCHER)
	{
		precache_model4("models/archer2.mdl");
		precache_model4("models/akarrow2.mdl");
		precache_sound4 ("archer/arrowg2.wav");

		precache_sound ("archer/arrowg.wav");
		precache_sound ("archer/arrowr.wav");

		precache_model("models/archerhd.mdl");

		precache_model("models/gspark.spr");

		precache_sound ("archer/growl.wav");
		precache_sound ("archer/pain.wav");
		precache_sound ("archer/sight.wav");
		precache_sound ("archer/death.wav");
		precache_sound ("archer/draw.wav");
		have_monsters=TRUE;
	}
	if (self.spawnflags & ICE_IMP)
	{
		precache_model4 ("models/imp.mdl");//converted for MP
		precache_model ("models/h_imp.mdl");//empty for now
		precache_sound ("imp/up.wav");
		precache_sound ("imp/die.wav");
		precache_sound ("imp/swoop.wav");
		precache_sound ("imp/fly.wav");
		precache_model ("models/shardice.mdl");
		precache_sound ("imp/swoophit.wav");
		precache_sound ("imp/fireball.wav");
		precache_sound ("imp/shard.wav");
		precache_sound("hydra/turn-s.wav");
		have_monsters=TRUE;
	}
	if (self.spawnflags & SNOWLEOPARD)
	{
		precache_model4 ("models/snowleopard.mdl");
		precache_model2 ("models/mezzoref.spr");
		precache_model2 ("models/h_mez.mdl");
		precache_sound2 ("mezzo/skid.wav");
		precache_sound2 ("mezzo/roar.wav");
		precache_sound2 ("mezzo/reflect.wav");
		precache_sound2 ("mezzo/slam.wav");
		precache_sound2 ("mezzo/pain.wav");
		precache_sound2 ("mezzo/die.wav");
		precache_sound2 ("mezzo/growl.wav");
		precache_sound2 ("mezzo/snort.wav");
		precache_sound2 ("mezzo/attack.wav");
		have_monsters=TRUE;
	}
	if (self.spawnflags & WERETIGER)
	{
		precache_model4 ("models/snowleopard.mdl");
		precache_model2 ("models/mezzoref.spr");
		precache_model2 ("models/h_mez.mdl");
		precache_sound2 ("mezzo/skid.wav");
		precache_sound2 ("mezzo/roar.wav");
		precache_sound2 ("mezzo/reflect.wav");
		precache_sound2 ("mezzo/slam.wav");
		precache_sound2 ("mezzo/pain.wav");
		precache_sound2 ("mezzo/die.wav");
		precache_sound2 ("mezzo/growl.wav");
		precache_sound2 ("mezzo/snort.wav");
		precache_sound2 ("mezzo/attack.wav");
		have_monsters=TRUE;
	}
	if(self.spawnflags & YAKMAN)
	{
		precache_model4 ("models/yakman.mdl");
		precache_model4 ("models/yakball.mdl");
		precache_model2 ("models/iceshot2.mdl");
		precache_model4 ("models/powersh.mdl");
		precache_model4 ("models/sheepmis.mdl");
		precache_sound4 ("yakman/slam.wav");
		precache_sound4 ("yakman/pain.wav");
		precache_sound4 ("yakman/hoof.wav");
		precache_sound4 ("yakman/grunt.wav");
		precache_sound4 ("yakman/snort1.wav");
		precache_sound4 ("yakman/snort2.wav");
		precache_sound4 ("yakman/roar.wav");
		precache_sound4 ("yakman/die.wav");
		precache_sound ("crusader/icewall.wav");	
		precache_sound4 ("yakman/icespell.wav");	
		precache_sound ("crusader/icefire.wav");	
		precache_sound ("misc/tink.wav");				//Ice shots bounce
		precache_sound3 ("mezzo/skid.wav");
		have_monsters=TRUE;
	}
	return have_monsters;
}

float monster_spawn_precache (void)
{
float have_monsters;
	if (self.spawnflags & IMP)
	{
		precache_model4 ("models/imp.mdl");//converted for MP
		precache_model ("models/h_imp.mdl");//empty for now
		precache_sound ("imp/up.wav");
		precache_sound ("imp/die.wav");
		precache_sound ("imp/swoop.wav");
		precache_sound ("imp/fly.wav");
		precache_model ("models/shardice.mdl");
		precache_model ("models/fireball.mdl");
		precache_sound ("imp/swoophit.wav");
		precache_sound ("imp/fireball.wav");
		precache_sound ("imp/shard.wav");
		precache_sound("hydra/turn-s.wav");
		have_monsters=TRUE;
	}
	if (self.spawnflags & ARCHER)
	{
		precache_archer();
		have_monsters=TRUE;
	}
	if (self.spawnflags & WIZARD)
	{
		precache_model4("models/skullwiz.mdl");//converted for MP
		precache_model("models/skulbook.mdl");
		precache_model("models/skulhead.mdl");
		precache_model("models/skulshot.mdl");
		precache_model("models/spider.mdl");
		precache_sound("skullwiz/death.wav");
		precache_sound("skullwiz/blinkspk.wav");
		precache_sound("skullwiz/growl.wav");
		precache_sound("skullwiz/scream.wav");
		precache_sound("skullwiz/pain.wav");
		precache_sound("skullwiz/gate.wav");
		precache_sound("skullwiz/blinkin.wav");
		precache_sound("skullwiz/blinkout.wav");
		precache_sound("skullwiz/push.wav");
		precache_sound("skullwiz/firemisl.wav");
		precache_spider();
		have_monsters=TRUE;
	}
	if (self.spawnflags & SCORPION)
	{
		precache_model2("models/scorpion.mdl");
		precache_sound2("scorpion/awaken.wav");
		precache_sound2("scorpion/walk.wav");
		precache_sound2("scorpion/clawsnap.wav");
		precache_sound2("scorpion/tailwhip.wav");
		precache_sound2("scorpion/pain.wav");
		precache_sound2("scorpion/death.wav");
		have_monsters=TRUE;
	}
	if(self.spawnflags & SPIDER)
	{
		precache_spider();
		have_monsters=TRUE;
	}
	return have_monsters;
}

float byte_for_num[10] =
{
	1,2,4,8,16,32,128,256,512,1024
};

float check_monsterspawn_ok (void)
{
vector org;

	if(self.spawnername)
	{
	entity findspot;
	float foundspot,founddead;

//FIXME: have it chain them at spawning and do a self.controller=self.controller.chain;
		findspot=find(self.controller,netname,self.netname);
		while(!foundspot)
		{
			//Warning!  If you forget to put spawnspots and you give the spawner
			//a spawnername string, it will go into an infinite loop and the Universe
			//will cease to exist!
			if(findspot.spawnername==self.spawnername)
			{
			if(findspot.aflag==self.level+1)
				if(findspot.deadflag)
				{
					founddead=TRUE;
					self.level=findspot.aflag;
				}
				else
					foundspot=TRUE;
//			Oops!  doesn't automatically loop
			if(findspot==self.controller)
//			if(findspot==world)
				if(self.level==0)
					if(founddead)//found some, but they're not active yet
						return FALSE;
					else
					{
						remove(self);
						return FALSE;
					}
				else
					self.level=0;
			}
			if(!foundspot)
				findspot=find(findspot,netname,self.netname);
		}
		self.level=findspot.aflag;
		self.controller=findspot;
		org=findspot.origin;
	}
	else
		org=self.origin;

	if(self.controller.spawnflags&ONDEATH&&(self.controller.goalentity.flags2&FL_ALIVE))
		return FALSE;

	tracearea(org,org,self.mins,self.maxs,FALSE,self);
	newmis = spawn();
	if(trace_fraction<1)
		if(trace_ent.flags2&FL_ALIVE&&!self.frags)
		{
			remove(newmis);
			return FALSE;
		}
		else
			spawn_tdeath(trace_ent.origin,newmis);

	newmis.angles = self.angles;
	newmis.flags2+=FL_SUMMONED;
//	newmis.spawnflags=NO_DROP;

	float foundthink,rnd;

	while(!foundthink)
	{
		rnd=rint(random(1,5));
		rnd=byte_for_num[rnd - 1];//byte_me(rnd);
		if(self.controller.spawnflags&rnd)
			foundthink=TRUE;
	}

	if(self.classname=="func_monsterspawner_mp")
	{
		if (rnd==ICE_ARCHER)
		{
			newmis.classname = "monster_archer_ice";
			newmis.think = monster_archer_ice;
		}
		else if (rnd==ICE_IMP)
		{
			newmis.classname = "monster_imp_ice";
			newmis.think = monster_imp_ice;
		}
		else if (rnd==SNOWLEOPARD)
		{
			newmis.classname = "monster_weresnowleopard";
			newmis.think = monster_weresnowleopard;
		}
		else if (rnd==WERETIGER)
		{
			newmis.classname = "monster_weretiger";
			newmis.think = monster_weretiger;
		}
		else if (rnd==YAKMAN)//it must be a yakman, baby!
		{
			newmis.classname = "monster_yakman";
			if(random()<0.5)
				newmis.skin=1;
			newmis.think = monster_yakman;
		}
		else
			dprintf("Invalid monster spawner flag: %s\n",rnd);
	}
	else
	{
		if (rnd==IMP)
		{
			if(random()<0.5)
			{
				newmis.classname = "monster_imp_ice";
				newmis.think = monster_imp_ice;
			}
			else
			{
				newmis.classname = "monster_imp_fire";
				newmis.think = monster_imp_fire;
			}
		}
		else if (rnd==ARCHER)
		{
			newmis.classname = "monster_archer";
			newmis.think = monster_archer;
		}
		else if (rnd==WIZARD)
		{
			newmis.classname = "monster_skull_wizard";
			newmis.think = monster_skull_wizard;
		}
		else if (rnd==SCORPION)
		{
			if(random()<0.5)
			{
				newmis.classname = "monster_scorpion_black";
				newmis.think = monster_scorpion_black;
			}
			else
			{
				newmis.classname = "monster_scorpion_yellow";
				newmis.think = monster_scorpion_yellow;
			}
		}
		else//it must be a spider, baby!
		{
			rnd=rint(random(1,4));
			if(rnd==4)
			{
				newmis.classname = "monster_spider_yellow_large";
				newmis.think = monster_spider_yellow_large;
			}
			else if(rnd==3)
			{
				newmis.classname = "monster_spider_yellow_small";
				newmis.think = monster_spider_yellow_small;
			}
			else if(rnd==2)
			{
				newmis.classname = "monster_spider_red_large";
				newmis.think = monster_spider_red_large;
			}
			else
			{
				newmis.classname = "monster_spider_red_small";
				newmis.think = monster_spider_red_small;
			}
		}
	}

	self.controller.goalentity=newmis;
	setorigin(newmis,org);
	if(!self.controller.spawnflags&QUIET)
		spawn_tfog(org);
	newmis.nextthink = time;
	return TRUE;
}

void monsterspawn_active (void)
{//fixme: option to make spawned monster angry at activator?
	self.think=monsterspawn_active;
	if(check_monsterspawn_ok())
	{
		self.controller.frags+=1;
		if(self.controller!=self)
			self.frags+=1;
		if(self.controller!=self)
		{
			self.controller.deadflag=TRUE;
			self.controller.think=self.controller.use;
			self.controller.nextthink=time+self.controller.wait;
		}
		if(self.controller.frags>=self.controller.cnt)
		{
			if(self.controller.goalentity!=world)
				self.controller.goalentity.target=self.controller.target;
			remove(self.controller);
		}
		if(self.frags>=self.cnt)
		{
			if(self.goalentity!=world)
				self.goalentity.target=self.target;
			remove(self);
		}
		if(self.spawnflags&TRIGGERONLY)
			self.nextthink=-1;
		else
			self.nextthink=time+self.wait;
	}
	else if(self.spawnflags&TRIGGERONLY)//Don't keep trying
		self.nextthink=-1;
	else if(self.dflags)
		self.nextthink=time+self.wait;
	else
		self.nextthink=time+0.1;
}

/*QUAKED func_monsterspawner (1 .8 0) (-16 -16 0) (16 16 56) IMP ARCHER WIZARD SCORPION SPIDER ONDEATH QUIET TRIGGERONLY
If something is blocking the spawnspot, this will telefrag it as long as it's not a living entity (flags2&FL_ALIVE)

You can set up as many spots as you want for it to spawn at and it will cycle
between these.  Make them classname func_monsterspawn_spot and
their spawnername has to match this entity's spawnername.
You can control the order in which they are used by setting thier
aflag (1, 2, 3, etc- there is NO ZERO, that's for you designers!)
You should give the spawner an aflag too if you want it's origin
included in the cycle of spawning, but if there are no spawn spots (just a spawner), no aflag is needed anywhere.

WARNING!!!  If you forget to put spawnspots and you give the spawner a spawnername, it will go into an infinite loop and the Universe will cease to exist!

If you only want monsters to spawn at the monster spawner origin, then don't worry about aflags or the spawnername, it will know... It's that cool.

The Monsters will be spawned at the origin of the spawner (and/or spawnspots), so if you want them not to stick in the ground, put this above the ground some- maybe 24?  Make sure there's enough room around it for the monsters.

ONDEATH = only spawn the new monster after the last has died, defaults to FALSE (doesn't wait)
TRIGGERONLY = Will only spawn a monster when it's been used by a trigger.  The default is continous spawning.
wait = time to wait after spawning a monster until the next monster is spawned, defaults to 0.5 seconds. If there are multiple spawn spots, this will be the time between cycles (default 0.5)- this is a minimum wait, after a faield spawn, it will wait 0.1 seconds to try again
cnt = number of monsters, max to spawn, defaults to 17 (no reason, just like that number!)	If there are multiple spots, this should be the total off ALL the spots, including the spawner itself.
aflag = order in the spawning cycle
spawnername = spawnspots to look for- be sure to make spawnspots!
targetname = not needed unless you plan to activate this with a trigger
dflags = If dflags is set to "1", the spawner will wait it's "wait" value every time it fails to spawn a monster.

There will be a test on this on Thursday.  Interns are NOT exempt.
*/
void func_monsterspawner (void)
{
	if(!self.cnt)
		self.cnt=17;

	if(!self.wait)
		self.wait=0.5;

	self.netname="monsterspawn_spot";
	if(self.spawnername)
		self.controller=world;//misleading name, this is the last spawnspot
	else
		self.controller=self;

	self.level=0;//Spawn cycle counter
	//setmodel(self,self.model);
	setsize(self,'-16 -16 0','16 16 56');
	setorigin(self,self.origin);

	if(self.classname=="func_monsterspawner_mp")
	{
		if(!monster_spawn_precache_mp()&&!self.spawnername)
		{
	//		dprint("You don't have any monsters assigned to me, and I have no spawnername!\n");
			remove(self);
		}
	}
	else
	{
		if(!monster_spawn_precache()&&!self.spawnername)
		{
	//		dprint("You don't have any monsters assigned to me, and I have no spawnername!\n");
			remove(self);
		}
	}

	if(self.targetname)
		self.use=monsterspawn_active;
	else
	{
		self.think=monsterspawn_active;
		self.nextthink=time+3;//wait while map starts
	}
}

/*QUAKED func_monsterspawn_spot (1 .3 0) (-16 -16 0) (16 16 56) IMP ARCHER WIZARD SCORPION SPIDER ONDEATH QUIET
All this does is mark where to spawn monsters for a spawn spot.

You can set any monster type for each spawnspot.

Just make the spawnername of this entity equal to the spawnername of the spawner and the spawner will cycle through all it's spawnspots in the world.

If you don't set the aflag, the spawn spot will NOT be used.

Note that if you set a trigger to activate this spawnspot, you can have it turn on and be included in the spawner's cycle, but you still always have to set the aflag.

aflag = order in the spawning cycle, you MUST set this, or it's useless.
cnt = number of monsters this spot will spawn, defaults to 17.
spawnername = this HAS to match the spawner's spawnername!
wait = how long the minimum interval should be between monster spawns for THIS spot.
targetname = used if you want this to wait to be activated by a seperate trigger before being included in the spawning cycle.
*/
void func_monsterspawn_spot (void)
{

	if(!self.aflag)
	{
//		dprint("Ooo!  You didn't include me in the spawn cycle!  FIXME!\n");
		remove(self);
	}
	if(!self.cnt)
		self.cnt=17;
	self.netname="monsterspawn_spot";

	if(!monster_spawn_precache())
	{
//		dprint("You didn't give me any monsters to spawn!!!\n");
		remove(self);
	}

	//setmodel(self,self.model);
	setsize(self,'-16 -16 0','16 16 56');
	setorigin(self,self.origin);
	if(self.targetname)
		self.deadflag=TRUE;
	else
		self.use=spawnspot_activate;
}

/*QUAKED func_monsterspawn_spot_mp (1 .3 0) (-16 -16 0) (16 16 56) ICE_ARCHER ICE_IMP SNOWLEOPARD WERETIGER YAK ONDEATH QUIET
All this does is mark where to spawn monsters for a spawn spot.

You can set any monster type for each spawnspot.

Just make the spawnername of this entity equal to the spawnername of the spawner and the spawner will cycle through all it's spawnspots in the world.

If you don't set the aflag, the spawn spot will NOT be used.

Note that if you set a trigger to activate this spawnspot, you can have it turn on and be included in the spawner's cycle, but you still always have to set the aflag.

aflag = order in the spawning cycle, you MUST set this, or it's useless.
cnt = number of monsters this spot will spawn, defaults to 17.
spawnername = this HAS to match the spawner's spawnername!
wait = how long the minimum interval should be between monster spawns for THIS spot.
targetname = used if you want this to wait to be activated by a seperate trigger before being included in the spawning cycle.
*/
void func_monsterspawner_mp (void)
{
	func_monsterspawner ();
}

/*QUAKED func_monsterspawner_mp (1 .8 0) (-16 -16 0) (16 16 56) ICE_ARCHER ICE_IMP SNOWLEOPARD WERETIGER YAK ONDEATH QUIET TRIGGERONLY 
If something is blocking the spawnspot, this will telefrag it as long as it's not a living entity (flags2&FL_ALIVE)

You can set up as many spots as you want for it to spawn at and it will cycle
between these.  Make them classname func_monsterspawn_spot and
their spawnername has to match this entity's spawnername.
You can control the order in which they are used by setting thier
aflag (1, 2, 3, etc- there is NO ZERO, that's for you designers!)
You should give the spawner an aflag too if you want it's origin
included in the cycle of spawning, but if there are no spawn spots (just a spawner), no aflag is needed anywhere.

WARNING!!!  If you forget to put spawnspots and you give the spawner a spawnername, it will go into an infinite loop and the Universe will cease to exist!

If you only want monsters to spawn at the monster spawner origin, then don't worry about aflags or the spawnername, it will know... It's that cool.

The Monsters will be spawned at the origin of the spawner (and/or spawnspots), so if you want them not to stick in the ground, put this above the ground some- maybe 24?  Make sure there's enough room around it for the monsters.

ONDEATH = only spawn the new monster after the last has died, defaults to FALSE (doesn't wait)
TRIGGERONLY = Will only spawn a monster when it's been used by a trigger.  The default is continous spawning.
wait = time to wait after spawning a monster until the next monster is spawned, defaults to 0.5 seconds. If there are multiple spawn spots, this will be the time between cycles (default 0.5)
cnt = number of monsters, max to spawn, defaults to 17 (no reason, just like that number!)	If there are multiple spots, this should be the total off ALL the spots, including the spawner itself.
aflag = order in the spawning cycle
spawnername = spawnspots to look for- be sure to make spawnspots!
targetname = not needed unless you plan to activate this with a trigger

There will be a test on this on Thursday.  Interns are NOT exempt.
*/
void func_monsterspawn_spot_mp (void)
{
	func_monsterspawn_spot();
}

/*
//Uncomment these to remove all riders and eidolon from code.
void hive_die(){}
void spawn_ghost (entity attacker){}
void multiplayer_health(){}
void riderpath_init(){}
void riderpath_move(float move_speed){}
float eidolon_riderpath_move(float move_speed){return FALSE;}
void eidolon_guarding(){}
float eidolon_check_attack(){return FALSE;}
*/

