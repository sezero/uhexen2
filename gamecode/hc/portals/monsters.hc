/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/monsters.hc,v 1.2 2007-02-07 16:59:34 sezero Exp $
 */
/* ALL MONSTERS SHOULD BE 1 0 0 IN COLOR */

// name =[framenum,	nexttime, nextthink] {code}
// expands to:
// name ()
// {
//		self.frame=framenum;
//		self.nextthink = time + nexttime;
//		self.think = nextthink
//		<code>
// };

void monster_unfreeze ()
{
	if(random()<0.2)
		sound(self,CHAN_AUTO,"misc/drip.wav",1,ATTN_NORM);

	if(self.skin==GLOBAL_SKIN_ICE&&self.think!=monster_unfreeze)
	{
		sound(self,CHAN_BODY,"crusader/frozen.wav",1,ATTN_NORM);
		self.think=monster_unfreeze;
		thinktime self : 0.1;
	}
	else if(self.colormap<149)
	{
		if(self.skin==GLOBAL_SKIN_ICE)
		{
			self.skin=self.oldskin;
			self.colormap=144;
			self.abslight=0.5;
		}
		else
		{
			self.colormap+=1;
			self.abslight+=0.05;
		}
		self.think=monster_unfreeze;
		thinktime self : 0.1;
	}
	else
	{
		self.abslight=0;
		self.colormap=0;
		self.frozen=0;
		self.drawflags(-)DRF_TRANSLUCENT|MLS_ABSLIGHT;
		self.thingtype=THINGTYPE_FLESH;//old thingtype?
		self.touch=self.oldtouch;//oldtouch?
		self.movetype=self.oldmovetype;
		thinktime self : 0.1;
		self.think = FoundTarget;
	}
}

void monster_start_frozen ()
{
	self.frozen=50;
	if(self.skin<10)
		self.oldskin=self.skin;
	else
		self.skin=0;
	self.colormap=0;
	self.abslight=0.5;
	self.skin=GLOBAL_SKIN_ICE;
	self.thingtype=THINGTYPE_ICE;
	self.oldmovetype=self.movetype;
	self.movetype=MOVETYPE_TOSS;
//    loser.flags(-)FL_FLY;
//    loser.flags(-)FL_SWIM;
	if(self.flags&FL_ONGROUND)
		self.last_onground=time;
    self.flags(-)FL_ONGROUND;
	self.oldtouch=self.touch;
	self.touch=obj_push;
	self.drawflags(+)DRF_TRANSLUCENT|MLS_ABSLIGHT;
	self.think=SUB_Null;
	self.nextthink=-1;
}

/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/
void() monster_use =
{
	if (self.enemy)
		return;
	if (self.health <= 0)
		return;
	if (!self.flags2&FL_ALIVE)
		return;
	if (activator.items & IT_INVISIBILITY)
		return;
	if (activator.flags & FL_NOTARGET)
		return;
	if (activator.classname != "player")
		return;
	
	if(self.frozen&&!self.monster_awake)
	{
		self.enemy=activator;
		monster_unfreeze();
		return;
	}

	if(self.classname=="monster_mezzoman"&&!visible(activator)&&!self.monster_awake)
	{
		self.enemy=activator;
		mezzo_choose_roll(activator);
		return;
	}
// delay reaction so if the monster is teleported, its sound is still
// heard
	else
	{
		self.enemy = activator;
		thinktime self : 0.1;
		self.think = FoundTarget;
	}
};

/*
================
monster_death_use

When a mosnter dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_respawn_go ()
{
	self.frags=FALSE;
	sound (self, CHAN_AUTO, "weapons/expsmall.wav", 1, ATTN_NORM);
	starteffect(CE_FLOOR_EXPLOSION , self.origin+'0 0 64');
	self.think=self.th_init;
	thinktime self : 0.1;
}

void monster_respawn_init ()
{
	self.frags=TRUE;
	spawn_tdeath(self.origin,self);
	self.think=monster_respawn_go;
	thinktime self : 0.5;
}

void(float force_respawn) monster_death_use =
{
// fall to ground
	if((!deathmatch&&skill>=4)||force_respawn)
	{
		if(self.th_init!=SUB_Null||force_respawn)
		{
			if((self.monsterclass<CLASS_BOSS&&!self.flags2&FL_SUMMONED)||force_respawn)
			{
				entity newmonster;
				newmonster=spawn();
				if(self.classname=="monster_mezzoman")
				{
					if(self.model=="models/snowleopard.mdl")
					{
						if(self.strength>=3)
							newmonster.classname="monster_weretiger";
						else
							newmonster.classname="monster_weresnowleopard";
					}
					else if(self.strength)
						newmonster.classname="monster_werepanther";
					else
						newmonster.classname="monster_werejaguar";
				}
				else if(self.netname=="monster_archer_ice")
					newmonster.classname=self.netname;
				else
					newmonster.classname=self.classname;
				
				if(self.classname=="monster_pentacles")
					newmonster.target=self.target;
				else if(self.model=="models/sheep.mdl"&&world.target=="sheep")
				{
					newmonster.target=self.target;
					newmonster.targetname=self.targetname;
				}
				newmonster.spawnflags=self.spawnflags;
				setorigin(newmonster,self.init_org);
				newmonster.init_org=self.init_org;
				newmonster.th_init=self.th_init;
				newmonster.flags2(+)FL2_RESPAWN;
				if(self.skin>=100)
					newmonster.skin=self.oldskin;
				else
					newmonster.skin=self.skin;
				setsize(newmonster,self.mins,self.maxs);
				newmonster.think=monster_respawn_init;
				thinktime newmonster : 5+random(10);
			}
		}
	}
	self.flags(-)FL_FLY;
	self.flags(-)FL_SWIM;

	if (!self.target)
		return;

	activator = self.enemy;
	SUB_UseTargets ();
};


//============================================================================

void() walkmonster_start_go =
{
	if(!self.touch)
		self.touch=obj_push;

	if(!self.spawnflags&NO_DROP)
	{
		self.origin_z = self.origin_z + 1;	// raise off floor a bit
		droptofloor();
		if (!walkmove(0,0, FALSE))
		{
			if(self.flags2&FL_SUMMONED)
			{
				remove(self);
				return; /* THOMAS: return  was missing here */
			}
			else
			{
				dprint ("walkmonster in wall at: ");
				dprint (vtos(self.origin));
				dprint ("\n");
			}
		}
		if(self.model=="model/spider.mdl"||self.model=="model/scorpion.mdl")
			pitch_roll_for_slope('0 0 0',self);
	}

	if(!self.ideal_yaw)
	{
//		dprint("no preset ideal yaw\n");
		self.ideal_yaw = self.angles * '0 1 0';
	}
	
	if (!self.yaw_speed)
		self.yaw_speed = 20;

	if(self.view_ofs=='0 0 0')
		self.view_ofs = '0 0 25';

	if(self.proj_ofs=='0 0 0')
		self.proj_ofs = '0 0 25';

	if(!self.use)
		self.use = monster_use;

	if(!self.flags&FL_MONSTER)
		self.flags(+)FL_MONSTER;
	
	if(self.flags&FL_MONSTER&&self.classname=="player_sheep")
		self.flags(-)FL_MONSTER;

	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
		self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
		if (!self.pathentity)
		{
			dprint ("Monster can't find target at ");
			dprint (vtos(self.origin));
			dprint ("\n");
		}
// this used to be an objerror
		if (self.pathentity.classname == "path_corner")
			if(self.spawnflags&SF_FROZEN)
				monster_start_frozen();
			else
				self.th_walk ();
		else
		{
			if(self.goalentity.health>0&&self.goalentity.flags2&FL_ALIVE)
			{
				self.enemy=self.goalentity;
				self.th_run();
			}
			else
			{
				self.pausetime = 99999999;
				if(self.spawnflags&SF_FROZEN)
					monster_start_frozen();
				else
					self.th_stand ();
			}
		}
	}
	else
	{
			self.pausetime = 99999999;
			if(self.spawnflags&SF_FROZEN)
				monster_start_frozen();
			else
				self.th_stand ();
	}

// spread think times so they don't all happen at same time
	self.nextthink+=random(0.5);
};

void walkmonster_start ()
{
// delay drop to floor to make sure all doors have been spawned
// spread think times so they don't all happen at same time
	if(self.puzzle_id)
		MonsterPrecachePuzzlePiece();
	
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;

	if(self.scale<=0)
		self.scale=1;

	self.nextthink+=random(0.5);
	self.think = walkmonster_start_go;
	total_monsters = total_monsters + 1;
}



/*
void() flymonster_start_go =
{
	self.takedamage = DAMAGE_YES;

	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 10;

	if(self.view_ofs=='0 0 0');
		self.view_ofs = '0 0 24';
	if(self.proj_ofs=='0 0 0');
		self.proj_ofs = '0 0 24';

	self.use = monster_use;

	self.flags(+)FL_FLY;
	self.flags(+)FL_MONSTER;

	if(!self.touch)
		self.touch=obj_push;

	if (!walkmove(0,0, FALSE))
	{
		dprint ("flymonster in wall at: ");
		dprint (vtos(self.origin));
		dprint ("\n");
	}

	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
		if (!self.pathentity)
		{
			dprint ("Monster can't find target at ");
			dprint (vtos(self.origin));
			dprint ("\n");
		}
// this used to be an objerror

		if (self.pathentity.classname == "path_corner")
			self.th_walk ();
		else
		{
			self.pausetime = 99999999;
			self.th_stand ();
		}
	}
	else
	{

		self.pausetime = 99999999;
		self.th_stand ();
	}
};

void() flymonster_start =
{
// spread think times so they don't all happen at same time
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;
	self.nextthink+=random(0.5);
	self.think = flymonster_start_go;
	total_monsters = total_monsters + 1;
};

void() swimmonster_start_go =
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.touch)
		self.touch=obj_push;

	self.takedamage = DAMAGE_YES;
	total_monsters = total_monsters + 1;

	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 10;

	if(self.view_ofs=='0 0 0');
		self.view_ofs = '0 0 10';
	if(self.proj_ofs=='0 0 0');
		self.proj_ofs = '0 0 10';

	self.use = monster_use;
	
	self.flags(+)FL_SWIM;
	self.flags(+)FL_MONSTER;

	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
		if (!self.pathentity)
		{
			dprint ("Monster can't find target at ");
			dprint (vtos(self.origin));
			dprint ("\n");
		}
// this used to be an objerror
		self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
		self.th_walk ();
	}
	else
	{
		self.pausetime = 99999999;
		self.th_stand ();
	}

// spread think times so they don't all happen at same time
	self.nextthink+=random(0.5);
};

void() swimmonster_start =
{
// spread think times so they don't all happen at same time
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;
	self.nextthink+=random(0.5);
	self.think = swimmonster_start_go;
	total_monsters = total_monsters + 1;
};
*/

