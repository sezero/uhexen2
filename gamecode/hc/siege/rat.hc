/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/rat.hc,v 1.1 2005-01-26 17:26:12 sezero Exp $
 */

// Scuttle
//$frame swalk1 swalk2 swalk3 swalk4 swalk5
//$frame swalk6 swalk7 swalk8 swalk9 swalk10
//$frame swalk11 swalk12 swalk13 swalk14 swalk15
//$frame swalk16

void() rat_wait;
void() rat_noise;
void rat_die_wait()
{
	if(self.health<-80)
	{
		chunk_death();
		return;
	}
	if(!self.velocity)
	{
		if(random()<0.5)
			self.angles_x=0;
		else
			self.angles_x=180;
		self.avelocity='0 0 0';
		MakeSolidCorpse();
		return;
	}
	else
	{
		self.think=rat_die_wait;
		thinktime self : 0.5;
	}
}

void rat_death (void)
{
	if(self.health<-80)
	{
		chunk_death();
		return;
	}

vector	bounce_vel;
	bounce_vel=normalize(self.origin-damage_attacker.origin);
	bounce_vel*=random(self.health*-30);
	self.velocity=bounce_vel;
	self.velocity_z=random(150,450);
	self.flags(-)FL_ONGROUND;
	self.movetype=MOVETYPE_BOUNCE;
	self.avelocity_x=random(self.health*10) - self.health;	
	self.avelocity_y=random(self.health*10) - self.health;	
	self.avelocity_z=random(self.health*10) - self.health;	
	self.think=rat_die_wait;
	thinktime self : 0;
}

void ratrun(void)
{
entity player;
float player_see_me;
	self.frame += 1;

	thinktime self : HX_FRAME_TIME;
	self.think = ratrun;

	AdvanceFrame(0,17);
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
//	dprint(ftos(self.ideal_yaw));
//	dprint("\n");
	ChangeYaw();

	if(pointcontents(self.origin+'0 0 7')!=CONTENT_EMPTY)
	{
		if(pointcontents(self.origin)==CONTENT_LAVA)
		{
//			dprint("lava death\n");
			chunk_death();
		}
		else
		{
			self.velocity_z=1;
			self.flags(+)FL_SWIM;
		}
	}
	else
		self.velocity_z=-1;

	if((random()<0.1||self.origin==self.oldorigin)&&self.pain_finished<time&&!self.flags&FL_SWIM)
	{
	vector ahead_org;
	float drop_content;
		makevectors(self.angles);
		ahead_org=self.origin+'0 0 7'+v_forward*10;
		traceline(ahead_org,ahead_org-'0 0 1000',FALSE,self);
		drop_content=pointcontents(trace_endpos);
		if(drop_content!=CONTENT_EMPTY&&drop_content!=CONTENT_SOLID)
		{
			self.pain_finished=time+3;
			thinktime self : 0.5;
			self.velocity=v_forward*200+'0 0 150';
			self.flags(-)FL_ONGROUND;
		}
	}

	if(random()<0.1&&random()<0.1)
		rat_noise();

	if(self.goalentity.classname=="player")
	{
		if(vlen(self.goalentity.origin-self.origin)<36)
			self.think=rat_wait;
	}
	else
	{
		player=find(world,classname,"player");
		while(player)
		{
			if(random()<0.1&&random()<0.1&&random()<0.1&&random()<0.1&&random()<0.1&&!self.target)
			{
				self.goalentity=player;
				player_see_me=TRUE;
			}
			else if(visible2ent(self,player))
				player_see_me=TRUE;
			player=find(player,classname,"player");
		}
		if(!player_see_me&&time>self.lifetime)
		{
			if(self.goalentity.classname=="tempgoal")
				remove(self.goalentity);
			remove(self);
		}
		if(self.frags>=10)
		{
			self.frags=0;
			makevectors(self.angles);
			newmis=spawn();
			setorigin(newmis,v_forward*-1000+v_right*(random()*100 - 50));
			remove(self.goalentity);
			self.goalentity=newmis;
		}
	}
	if(!walkmove(self.angles_y,random(3,7),FALSE))
	{
//TEMP
		remove(self);
//		self.frags+=1;
	}
}

void rat_wait ()
{
	if(vlen(self.goalentity.origin-self.origin)>56)
		self.think=ratrun;
	else
		self.think=rat_wait;
	thinktime self : 0.05;
}

void rat_noise (void)
{
	sound(self,CHAN_VOICE,"misc/squeak.wav",1,ATTN_NORM);
}

void rat_touch (void)
{
	return;
}

//==========================================================================
//
// monster_ratnest
//
//==========================================================================

/*QUAKED monster_rat (1 0.3 0) (-3 -3 0) (3 3 7)
If the rat is targeted to a path, the rats will follow that path
------- key / value ----------------------------------
------- spawnflags -----------------------------------
AMBUSH
*/
void rat_make_goal (void)
{
vector goaldir;
	goaldir=self.angles;
	goaldir_y=self.ideal_yaw;
	makevectors(goaldir);
	newmis=spawn();
	newmis.effects=EF_NODRAW;
	setmodel(newmis,"models/null.spr");
	setorigin(newmis,self.origin+v_forward*1000);
	self.goalentity=newmis;
	newmis.classname="tempgoal";
	thinktime newmis : 30;
	newmis.think=SUB_Remove;
}

void rat_stupor (void)
{
	self.angles_y+=random(-2,2);
	self.frame=random(17);
	self.think=rat_stupor;
	thinktime self : random(0.3,1.3);
}

void rat_go (void)
{
	self.th_stand = ratrun;
	self.th_walk = ratrun;
	self.think=ratrun;
	thinktime self : 0;
}

void monster_rat (void)
{
//FIXME: DAMN RAT JUMPS UP IN AIR AND GET STUCK IN EACH OTHER!!!!!!!!!!!
	if(!self.flags2&FL_SUMMONED)
	{
		precache_model("models/rat.mdl");
		precache_sound("misc/squeak.wav");
		self.th_stand = rat_stupor;
		self.th_walk = rat_stupor;
		self.use=rat_go;
	}
	else
	{
		self.th_stand = ratrun;
		self.th_walk = ratrun;
	}
	
	setmodel(self, "models/rat.mdl");

	self.movetype = MOVETYPE_STEP;
	self.solid = SOLID_SLIDEBOX;
	if(pointcontents(self.origin)!=CONTENT_EMPTY)
	{
		self.flags(+)FL_SWIM;
		self.angles_x=10;
	}

	self.touch=rat_touch;
	self.classname="monster_rat";

	setsize(self, '-3 -3 0', '3 3 7');
	self.health = 3;

	self.thingtype=THINGTYPE_FLESH;
	self.th_run = ratrun;
	self.th_melee = ratrun;
	self.th_missile = ratrun;
	self.th_pain = rat_noise;
	self.th_die = rat_death;

	self.flags(+)FL_MONSTER;
	self.yaw_speed = 10;
	self.lifetime = time+10;

	if(!self.target)
	{
		self.ideal_yaw=random(360);
//		self.movetype=MOVETYPE_NOCLIP;
//		self.solid=SOLID_NOT;
		rat_make_goal();
	}

	walkmonster_start();
}

/*QUAKED monster_ratnest (1 0.3 0) (-20 -20 0) (20 20 10)
A group of 3 to 6 rats that flee when triggered
If the nest is targeted to a path, the rats will follow that path
------- key / value ----------------------------------
------- spawnflags -----------------------------------
AMBUSH
*/
void monster_ratnest(void)
{
	precache_model("models/rat.mdl");
	precache_sound("misc/squeak.wav");
	self.use=barrel_die;
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 3     5/25/98 1:39p Mgummelt
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 25    8/25/97 4:15p Mgummelt
 * 
 * 24    7/21/97 4:04p Mgummelt
 * 
 * 23    7/21/97 4:02p Mgummelt
 * 
 * 22    6/19/97 5:15p Mgummelt
 * 
 * 21    6/18/97 7:27p Mgummelt
 * 
 * 20    6/18/97 4:00p Mgummelt
 * 
 * 19    6/14/97 10:07p Mgummelt
 * 
 * 18    6/14/97 2:22p Mgummelt
 * 
 * 17    6/13/97 9:41p Mgummelt
 * 
 * 16    6/13/97 5:02p Mgummelt
 * 
 * 15    6/13/97 4:44p Mgummelt
 * 
 * 13    6/13/97 4:39p Mgummelt
 * 
 * 12    6/13/97 3:57p Mgummelt
 * 
 * 11    6/13/97 3:52p Mgummelt
 * 
 * 10    6/13/97 3:48p Mgummelt
 * 
 * 9     6/13/97 3:36p Mgummelt
 * 
 * 8     6/13/97 3:28p Mgummelt
 * 
 * 7     6/13/97 10:44a Mgummelt
 * 
 * 6     6/12/97 8:54p Mgummelt
 * 
 * 5     6/12/97 12:44p Mgummelt
 * 
 * 4     6/11/97 9:36p Mgummelt
 * 
 * 3     6/11/97 8:02p Rlove
 * New new new
 * 
 */

