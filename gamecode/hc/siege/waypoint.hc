/*
====================================================================
Waypoint Monster Following System
MG
4/10/97
.mintel will determine how many waypoints a monster will follow
before it doesn't know which way the enemy went and it will go into
wander mode.  A value of 0 will mean literally out of sight,
out of mind, once you're not visible, they forget about you.
A value of 1 will make them go to the last spot they saw you,
and that's it, they can't determine where you went from
there.  Potentially, they could track you forever without
seeing you but once, if this is desired, just make a VERY
high .mintel for the monster.  Note that when a monster
sees the player again, all his waypoints are removed
and he will "reset" follow mode, meaning if you leave his
sight again, the count of how many waypoints he follows
starts from 0 again.
What it can't do:
       *Can't make monsters navigate obstacles better
        when player is in sight (like water, etc).
       *Can't help monster find player better if he
        hasn't seen him yet, but this shouldn't happen often.
	   *Monsters still use idiot code to track waypoints.
Note:  this requires modifications to ai.hc (all the
self.th_run's) and an .mintel value for the monster.
=================================================================
*/

void RemoveWaypoint()
{
	if(self.lockentity)
	{
		self.lockentity.goalentity=self.controller.enemy;
		if(self.lockentity.classname=="monster_hydra")
			self.lockentity.search_time=time+5;
	}
	remove(self);
}

void()ResetWaypoints=
{
//        dprint("Waypoint deleted\n");
	if (self.controller)
	{
		self.controller.goalentity=self.controller.enemy;
		if(self.controller.classname=="monster_hydra")
			self.controller.search_time=time+5;
	}
	remove(self);
};

void()TransferWaypoint=
{
//        dprint("Next Waypoint transferred\n");
		if(self.goalentity.classname=="waypoint")
	        self.controller.goalentity=self.goalentity;
		else
		{
			if(self.controller.classname=="monster_hydra")
				self.controller.search_time=time+5;
	        self.controller.goalentity=self.enemy;
		}
        remove(self);
};

void()WaypointTouch=
{
        if(other!=self.controller)
                return;

        if(self.controller.mintel>=self.point_seq)
                TransferWaypoint();
        else
                ResetWaypoints();
};

void() SetNextWaypoint =
{
        if(visible(self.enemy)&&self.classname=="waypoint")//keep the spot in mind, but don't spawn there yet
            self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
//Don't set more if self.point_seq>=self.controller.mintel
//That won't allow other monsters to use this waypoint as well, though
//(purpose of that would be to recycle waypoints and spawn less entities)
        else 
		if(self.lastwaypointspot!=self.wallspot&&
			(
				(self.classname!="waypoint"&&self.mintel>0)||
				(self.classname=="waypoint"&&self.point_seq<self.controller.mintel)
			)
		  )
        {
		entity waypoint;
                self.lastwaypointspot=self.wallspot;
                waypoint=spawn();
                waypoint.lockentity=self;
                waypoint.classname="waypoint";
                waypoint.point_seq=self.point_seq+1;
                waypoint.movetype=MOVETYPE_NONE;
//                waypoint.solid=SOLID_TRIGGER;
                waypoint.touch=TransferWaypoint;
				waypoint.view_ofs=self.view_ofs;
                setmodel(waypoint,"models/null.spr");
				waypoint.effects=EF_NODRAW;

//                setmodel(waypoint,"models/test.mdl");
//				waypoint.abslight=1;
//				waypoint.drawflags=MLS_ABSLIGHT;

                setsize(waypoint,'0 0 0','0 0 0');
                setorigin(waypoint,self.wallspot);
				if(self.search_time>time&&(self.model=="models/imp.mdl"||self.model=="models/hydra.mdl"))	//Lay points, but don't follow yet
					self.trigger_field=waypoint;
                else
					self.goalentity=waypoint;
                if(self.classname=="waypoint")
                {
	                waypoint.controller=self.controller;
					waypoint.speed=self.speed;
                    self.enemy=self.controller.enemy;
//Should I have this waypoint stop making more waypoints?  What if he backtracks?
                }
                else
				{
                    waypoint.controller=self;
					if(self.movetype==MOVETYPE_FLY)
						waypoint.speed=6;
					else
						waypoint.speed=self.size_x;
				}

                waypoint.enemy=self.enemy;
				waypoint.wallspot=(waypoint.enemy.absmin+waypoint.enemy.absmax)*0.5;
                waypoint.think=SetNextWaypoint;
                thinktime waypoint : 0;
				waypoint.lifetime=time+30;//stop thinking after 30 seconds?? remove?  make static?
        }
        if(self.classname=="waypoint")
        {
                if(self.controller.enemy!=self.enemy||(!self.enemy.flags2&FL_ALIVE)||(!self.controller.flags2&FL_ALIVE))
				{
//						dprint("Owner's enemy has changed or died or owner died\n");
                        ResetWaypoints();
				}
                if(visible2ent(self.controller.enemy,self.controller))//||self.controller.enemy==self.controller.goalentity)//Check also for self.controller.goalentity=self.enemy since it would only do this if they see him
				{
//						dprint("Owner sees his enemy or his goalentity is his enemy\n");
//						dprint(self.controller.enemy.classname);
//						dprint(self.controller.goalentity.classname);
//						dprint("\n");
                        ResetWaypoints();
				}
                if(self.lockentity.goalentity!=self&&self.lockentity.trigger_field!=self)//if previous waypoint can see him (backtracked), remove self
				{
//					dprint("Previous waypoint changed direction\n");
					remove(self);
				}
                else if(visible2ent(self.goalentity,self.controller))//&&self.enemy==self.controller.enemy)                
				{
//					dprint("Owner can see next waypoint\n");
					if(self.controller.mintel>=self.point_seq)
						TransferWaypoint();
                    else
                        ResetWaypoints();
                }
				else if(random()<0.2&&vhlen(self.origin-self.controller.origin)<self.speed&&visible2ent(self,self.controller))
				{
//					dprint("Owner close enough and can see me\n");
					TransferWaypoint();
				}

                self.think=SetNextWaypoint;
				if(self.lifetime<time)
				{
					self.think=RemoveWaypoint;
					thinktime self : 30;
				}
				else
	                thinktime self : 0.2;//maybe 1? don't keep too much of a track on him
        }
};
