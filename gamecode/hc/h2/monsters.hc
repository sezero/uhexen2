/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/h2/monsters.hc,v 1.1.1.1 2004-11-29 11:41:22 sezero Exp $
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
	if (activator.items & IT_INVISIBILITY)
		return;
	if (activator.flags & FL_NOTARGET)
		return;
	if (activator.classname != "player")
		return;
	
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
void() monster_death_use =
{
// fall to ground
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
				remove(self);
			else
			{
				dprint ("walkmonster in wall at: ");
				dprint (vtos(self.origin));
				dprint ("\n");
			}
		}
		if(self.model=="model/spider.mdl"||self.model=="model/scorpion.mdl")
			pitch_roll_for_slope('0 0 0');
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
/*		if(self.spawnflags&PLAY_DEAD&&self.th_possum!=SUB_Null)
		{
			self.think=self.th_possum;
			thinktime self : 0;
		}
		else
*/
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
/*		if(self.spawnflags&PLAY_DEAD&&self.th_possum!=SUB_Null)
		{
			self.think=self.th_possum;
			thinktime self : 0;
		}
		else 
		{
*/
			self.pausetime = 99999999;
			self.th_stand ();
//		}
	}

// spread think times so they don't all happen at same time
	self.nextthink+=random(0.5);
};

void() walkmonster_start =
{
// delay drop to floor to make sure all doors have been spawned
// spread think times so they don't all happen at same time
	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;

	if(self.scale<=0)
		self.scale=1;

	self.nextthink+=random(0.5);
	self.think = walkmonster_start_go;
	total_monsters = total_monsters + 1;
};



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
//		if(self.spawnflags&PLAY_DEAD&&self.th_possum!=SUB_Null)
//		{
//			self.think=self.th_possum;
//			thinktime self : 0;
//		}
//		else

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
//		if(self.spawnflags&PLAY_DEAD&&self.th_possum!=SUB_Null)
//		{
//			self.think=self.th_possum;
//			thinktime self : 0;
//		}
//		else 
//		{

			self.pausetime = 99999999;
			self.th_stand ();
//		}
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

/*
 * $Log: not supported by cvs2svn $
 * 
 * 32    8/29/97 11:14p Mgummelt
 * 
 * 31    8/26/97 9:00a Mgummelt
 * 
 * 30    8/20/97 1:21p Mgummelt
 * 
 * 29    8/19/97 12:57p Mgummelt
 * 
 * 28    8/14/97 7:12p Mgummelt
 * 
 * 27    8/13/97 11:54p Mgummelt
 * 
 * 26    7/21/97 3:03p Rlove
 * 
 * 25    7/14/97 4:09p Mgummelt
 * 
 * 24    6/28/97 6:32p Mgummelt
 * 
 * 23    6/18/97 8:00p Mgummelt
 * 
 * 22    6/18/97 7:10p Mgummelt
 * 
 * 21    6/18/97 5:30p Mgummelt
 * 
 * 20    6/18/97 4:00p Mgummelt
 * 
 * 19    6/16/97 11:23a Mgummelt
 * 
 * 18    6/14/97 10:07p Mgummelt
 * 
 * 17    6/13/97 3:57p Mgummelt
 * 
 * 16    6/12/97 8:54p Mgummelt
 * 
 * 15    6/09/97 3:08p Mgummelt
 * 
 * 14    6/05/97 8:16p Mgummelt
 * 
 * 13    5/23/97 3:43p Mgummelt
 * 
 * 12    5/22/97 7:28p Mgummelt
 * 
 * 11    5/22/97 6:30p Mgummelt
 * 
 * 10    5/22/97 5:20p Mgummelt
 * 
 * 9     5/10/97 12:07p Mgummelt
 * 
 * 8     5/07/97 3:40p Mgummelt
 * 
 * 7     5/07/97 11:12a Rjohnson
 * Added a new field to walkmove and movestep to allow for setting the
 * traceline info
 * 
 * 6     5/06/97 1:29p Mgummelt
 * 
 * 5     4/18/97 5:24p Mgummelt
 * 
 * 4     3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 3     3/10/97 8:29a Rlove
 * Halfway through rewriting Monster AI
 * 
 * 2     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */

