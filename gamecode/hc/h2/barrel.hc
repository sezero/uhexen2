/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/h2/barrel.hc,v 1.1.1.1 2004-11-29 11:37:15 sezero Exp $
 */
/*
==============================================================================

BARRELS

==============================================================================
*/


$cd \art\models\objects\barrel\final
$base base 128 128
$skin skin
$frame resting


//void	MakeExplosion(string explodemodel);
void()barrel_check_float;

/*void obj_barrel_gravity()
{
	local vector slope;

	if(self.ltime <= time)
	{
		slope = getslope(self);
		if(slope != self.dest)
		{
			self.speed		= trace_plane_normal_z * 20;
			self.ideal_yaw	= 
			self.dest		= trace_plane_normal;
		}
		self.ltime = time + 0.4;
	}

	thinktime self : 0.05;
}
*/

void float(void)
{
float x_mod, y_mod, z_mod;
vector org;

	org=self.origin;
	if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA)
	{
		if(self.velocity_x)
			self.velocity_x/=1.1;
		if(self.velocity_y)
			self.velocity_y/=1.1;

		org_z+= self.maxs_z*0.77;//float only 23% above waterlevel
		if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA)
		{
//			self.flags(+)FL_SWIM;
			self.flags(-)FL_ONGROUND;
			if(self.velocity_z<77)
				self.velocity_z=80;
			else
				self.velocity_z+=random(0,0.01);
		}
		else
		{
			self.velocity_z-=random(0,0.01);
		}
	
		if(random()<0.3)
		{
			y_mod=random(-0.15,0.15);
			if(random()<0.5)
				self.angles_y+=y_mod;
			else
				self.angles_y+=y_mod;
		}
	
		if(random()<0.3)
		{
			x_mod=random(-0.15,0.15);
			if(fabs(self.angles_x+x_mod)>10)
				self.angles_x-=x_mod;
			else
				self.angles_x+=x_mod;
		}
	
		if(random()<0.3)
		{
			z_mod=random(-0.15,0.15);
			if(fabs(self.angles_z+z_mod)>10)
				self.angles_z-=z_mod;
			else
				self.angles_z+=z_mod;
		}
		thinktime self : 0.1;
	}
	else
	{
		self.angles_z=self.angles_z=0;
		self.classname="barrel";
		self.think=barrel_check_float;
		thinktime self : 0.5;
	}
}

void barrel_check_float (void)
{
vector org;
	org=self.origin;
	if(pointcontents(org)==CONTENT_WATER||pointcontents(org)==CONTENT_SLIME||pointcontents(org)==CONTENT_LAVA&&(!self.spawnflags&BARREL_SINK))
	{
		self.classname=="barrel_floating";
		self.think=float;
		thinktime self : 0;
	}
	else
	{
		self.classname="barrel";
		self.think=barrel_check_float;
		thinktime self : 0.5;
	}
}

/*
 * obj_barrel_explode() -- Blows the barrel up.
 */

void obj_barrel_explode()
{
entity attacker;

	self.th_die=SUB_Null;
	self.takedamage = DAMAGE_NO;

	if(self.enemy.flags2&FL_ALIVE)//give credit to person who blew it up
		attacker=self.enemy;
	else
		attacker=self;

	T_RadiusDamage(self, attacker, 100, self);

	sound(self, CHAN_VOICE, "weapons/explode.wav", 1, ATTN_NORM);

	particleexplosion(self.origin + '0 0 83',384,60,40);

	starteffect(CE_LG_EXPLOSION , (self.absmin+self.absmax)*0.5);

	chunk_death();
}


void()monster_rat;
void rat_spawn(float offset)
{
	newmis=spawn();
	newmis.angles_y=self.angles_y+offset*60;
	newmis.flags2(+)FL_SUMMONED;
	if(self.target)
		newmis.target=self.target;
	makevectors(newmis.angles);
	setorigin(newmis,self.origin+v_forward*16);
	newmis.think=monster_rat;
	thinktime newmis : 0;
}

void barrel_die ()
{
	self.solid=SOLID_NOT;
	if(random()<0.3||self.target!=""||self.classname=="monster_ratnest")
	{
	float r;
		r=rint(random(3,6));
		while(r>0)
		{
			rat_spawn(r);
			r-=1;
		}
	}
	if(self.classname!="monster_ratnest")
		chunk_death();
	else
		remove(self);
}

/*
 * obj_barrel_slide() -- Slides a barrel in the direction given in self.movedir.
 */
/*
void obj_barrel_slide()
{
	local entity victim;
	local float  direction;
//old if function, direction was null
//	if(walkmove(direction, self.count) == FALSE)
	if(!walkmove(self.cnt, self.count, FALSE))
		{
//Is this supposed to push something else it hits?
		victim = findradius(self.origin, self.count + 38);
		while(victim)
			{
			if(victim.movetype != MOVETYPE_NONE && victim != self)
				{
				direction = vectoyaw(victim.origin - self.origin);
				if(self.cnt <= 60 && direction >= 300)
					direction -= 360;
				if(direction > self.cnt - 60 &&
				   direction < self.cnt + 60)
					victim.velocity += (victim.origin - self.origin) * self.count;
				}
			victim = victim.chain;
			}
		}

//	if(!self.spawnflags & BARREL_DOWNHILL)
//		self.count -= 0.7;
//	if(self.count < 0.5)
//		self.think = obj_barrel_gravity;
//What is this supposed to do?
//	obj_barrel_gravity();
//	reduce self.count to simulate friction, right?
	self.count = self.count - 1;
	if(self.count<=0)
	{
		self.think=SUB_Null;
		self.nextthink = -1;
	}
	else
		thinktime self : 0.05;
}
*/

void obj_barrel_roll (void)
{
	self.v_angle_y=self.angles_y;
	self.v_angle_x=self.v_angle_z=0;
	makevectors(self.v_angle);
	self.velocity-=self.movedir*self.speed;
	self.movedir=normalize(v_forward);
	self.speed/=1.01;
	self.anglespeed=self.speed/7.7;
	if(self.speed<1&&self.speed>-1)
	{
		self.velocity = '0 0 0';
		self.think = SUB_Null;
		self.nextthink = -1;
	}
	else
	{
	vector dircheck;
//	float hitcheck;
//Rolling sound
		if(self.flags&FL_ONGROUND)
		{
			self.flags(-)FL_ONGROUND;
			self.last_onground=time;
			self.level=TRUE;
		}
		else if(self.level)
		{
			self.level=FALSE;
			self.last_onground = time - 1;
			self.speed/=2;//slow down if go off cliff, looks weird otherwise
		}
		

		self.velocity+=self.movedir*self.speed;
		self.angles_x-=self.anglespeed;
		self.think=obj_barrel_roll;
		thinktime self : 0.05;

		dircheck=normalize(self.velocity);
		makevectors(self.angles);
		traceline(self.origin,self.origin+dircheck*39,FALSE,self);
		if(trace_fraction==1)
		{
			traceline(self.origin+v_right*16,self.origin+v_right*16+dircheck*39,FALSE,self);
			if(trace_fraction==1)
				traceline(self.origin-v_right*16,self.origin-v_right*16+dircheck*39,FALSE,self);
		}
		if(trace_fraction<1)
		{
			sound(self,CHAN_AUTO,"fx/thngland.wav",1,ATTN_NORM);		// landing thud
			self.speed*=-0.25;
			self.last_onground=time - 1;
			obj_fly_hurt(trace_ent);
			self.velocity=dircheck*self.speed;
		}
	}			
}


/*
 * obj_barrel_shoot() -- Called when a barrel is shot.
 */
void obj_barrel_shoot()
{
	sound(self,CHAN_AUTO,"fx/thngland.wav",1,ATTN_NORM);		// landing thud

//	if(self.aflag)
//		return;
/*
	self.movedir = normalize(self.origin - self.enemy.origin);
	self.velocity=self.velocity + self.movedir*self.count;
	self.avelocity_y=random(100,300);
	self.velocity_z=self.velocity_z + 50;
	if(self.velocity_z>150)
		self.velocity_z=150;
	self.flags(-)FL_ONGROUND;
*/
}


/*
 * obj_barrel_use() -- Called when a barrel is triggered.
 */

void obj_barrel_use()
{
//	if(other.movetype == MOVETYPE_STEP)
//		obj_pull();
//	else
		obj_barrel_explode();
}


void spawn_barrel(float barrel_type)
{
	precache_model("models/barrel.mdl");
	if(barrel_type==BARREL_NORMAL)
		precache_model("models/rat.mdl");

	CreateEntityNew(self,ENT_BARREL,"models/barrel.mdl",chunk_death);

	if(self.spawnflags&ON_SIDE)
	{
		self.aflag=1;
		self.frame=1;
		self.v_angle=self.angles;
		setsize(self, '-18 -13 -13','18 13 13');
		self.hull=HULL_CROUCH;
	}
	else
		setsize(self, '-13 -13 0','13 13 36');

	if (self.scale)
		self.mass *=self.scale;

	self.classname = "barrel";

	self.flags(+)FL_PUSH;
	self.touch		= obj_push;
	self.th_pain	= obj_barrel_shoot;

	if (barrel_type==BARREL_NORMAL)
	{
		self.th_die	= barrel_die;
		self.skin=0;
	}
	else if (barrel_type==BARREL_INDESTRUCTIBLE)
	{
		self.health	= 999999;
		self.th_die	= SUB_Null;
		self.skin=1;
	}
	else if (barrel_type==BARREL_EXPLODING)
	{
		self.th_die	= obj_barrel_explode;
		self.skin=2;
	}

	if(!self.spawnflags&BARREL_SINK)
	{
		self.think=barrel_check_float;
		thinktime self : 0;
	}

	if(pointcontents(self.origin)!=CONTENT_EMPTY&&(!self.spawnflags&BARREL_SINK))
	{
		self.classname="barrel_floating";
		self.think=float;
		thinktime self : 0;
	}
	else if(!self.flags2&FL_SUMMONED&&!self.spawnflags&BARREL_NO_DROP)
		droptofloor();

	if(self.targetname)
		self.use=self.th_die;
}


/*QUAKED obj_barrel (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK

A barrel, just a plain old barrel
-------------------------FIELDS-------------------------
.health  - How hard it is to smash
           Default: 25
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Floats in water
--------------------------------------------------------
*/
void obj_barrel(void)
{
	precache_sound("misc/squeak.wav");
	spawn_barrel(BARREL_NORMAL);
	self.mass = 75;
}


/*QUAKED obj_barrel_indestructible (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK

A barrel you just can't break
-------------------------FIELDS-------------------------
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Floats in water
--------------------------------------------------------
*/
void obj_barrel_indestructible(void)
{
	spawn_barrel(BARREL_INDESTRUCTIBLE);
	self.mass = 95;
}


/*QUAKED obj_barrel_exploding (0.3 0.1 0.6) (-13 -13 0) (13 13 36) DOWNHILL NO_DROP ON_SIDE SINK
An exploding barrel with red XXX on the side
WARNING!:  Putting too many exploding barrels next to each other will cause a crash, there is no way around this, so if it happens, it's to be considered a Designer error!  
Putting them in lines and chains seems to be ok, as long as you don't stack them or group them too closely, more than 4 in a tight group is probably pushing it.
-------------------------FIELDS-------------------------
.health  - How hard it is to blow up
           Default: 25
DOWNHILL - This barrel will slide downhill with gravity.
NO_DROP - Will not drop to floor before spawning
ON_SIDE - Will make the barrel appear to be on it's side, the top will point right (90 degrees)
			Note- barrels on their side must be placed at least 13 units above the floor.
SINK - Sinks in water
 --------------------------------------------------------
*/
void obj_barrel_exploding(void)
{
	spawn_barrel(BARREL_EXPLODING);
	self.mass = 85;
}



/*
 * $Log: not supported by cvs2svn $
 * 
 * 76    8/19/97 12:09p Mgummelt
 * 
 * 75    8/18/97 12:52p Mgummelt
 * 
 * 74    8/16/97 5:46p Mgummelt
 * 
 * 73    8/14/97 1:28p Mgummelt
 * 
 * 72    8/13/97 5:46p Mgummelt
 * 
 * 71    8/07/97 4:15p Mgummelt
 * 
 * 70    7/21/97 6:42p Rlove
 * 
 * 69    7/21/97 4:03p Mgummelt
 * 
 * 68    7/21/97 4:02p Mgummelt
 * 
 * 67    7/15/97 8:03p Mgummelt
 * 
 * 66    7/10/97 1:51p Mgummelt
 * 
 * 65    7/10/97 1:46p Mgummelt
 * 
 * 64    6/21/97 9:11a Rlove
 * 
 * 63    6/18/97 5:59p Mgummelt
 * 
 * 62    6/18/97 5:30p Mgummelt
 * 
 * 61    6/18/97 4:00p Mgummelt
 * 
 * 60    6/12/97 8:54p Mgummelt
 * 
 * 59    6/12/97 12:16p Mgummelt
 * 
 * 58    6/11/97 9:36p Mgummelt
 * 
 * 57    6/03/97 10:48p Mgummelt
 * 
 * 56    6/02/97 7:58p Mgummelt
 * 
 * 55    6/01/97 7:32a Mgummelt
 * 
 * 54    5/28/97 5:10p Rlove
 * 
 * 53    5/24/97 2:42p Mgummelt
 * 
 * 52    5/23/97 2:54p Mgummelt
 * 
 * 51    5/22/97 6:30p Mgummelt
 * 
 * 50    5/22/97 2:50a Mgummelt
 * 
 * 49    5/19/97 11:36p Mgummelt
 * 
 * 48    5/17/97 8:45p Mgummelt
 * 
 * 47    5/15/97 3:09p Mgummelt
 * 
 * 46    5/15/97 3:45a Mgummelt
 * 
 * 44    5/12/97 11:11p Mgummelt
 * 
 * 43    5/11/97 10:01p Mgummelt
 * 
 * 42    5/11/97 7:30a Mgummelt
 * 
 * 41    5/08/97 10:25a Rlove
 * Lowered damage done by lethal exploding barrels
 * 
 * 40    5/08/97 9:45a Rlove
 * Condensed all the different barrels into three basic types.
 * 
 * 39    5/07/97 11:12a Rjohnson
 * Added a new field to walkmove and movestep to allow for setting the
 * traceline info
 * 
 * 38    5/02/97 8:06p Mgummelt
 * 
 * 37    4/24/97 2:15p Mgummelt
 * 
 * 36    4/21/96 1:35p Mgummelt
 * 
 * 35    4/21/96 1:32p Mgummelt
 * 
 * 34    4/18/97 5:24p Mgummelt
 * 
 * 33    4/15/96 6:14p Mgummelt
 * 
 * 32    4/10/96 2:49p Mgummelt
 * 
 * 31    4/09/96 7:54p Mgummelt
 * 
 * 30    4/09/96 4:43p Mgummelt
 * 
 * 29    4/07/97 6:29p Mgummelt
 * 
 * 28    4/07/97 5:03p Mgummelt
 * 
 * 27    4/07/97 4:49p Mgummelt
 * 
 * 26    4/07/97 4:48p Mgummelt
 * 
 * 25    4/07/97 2:50p Mgummelt
 * 
 * 24    4/05/97 6:18p Mgummelt
 * 
 * 23    4/05/97 6:08p Mgummelt
 * 
 * 22    4/05/97 5:56p Mgummelt
 * 
 * 21    4/05/97 9:49a Mgummelt
 * 
 * 20    4/04/97 4:31p Mgummelt
 * 
 * 19    3/31/97 9:48a Aleggett
 * 
 * 18    3/22/97 2:17p Aleggett
 * Sliding barrels can push players a little
 * 
 * 17    3/21/97 6:59p Aleggett
 * Fixed radius damage for exploding barrels
 * 
 * 16    3/21/97 5:17p Aleggett
 * Further barrel enhancements
 * 
 * 15    3/21/97 1:14p Rlove
 * Adding the MOVETYPE_PUSHPULL
 * 
 * 14    3/21/97 10:19a Rlove
 * Changed obj_die calls to chunk_death
 * 
 * 13    3/20/97 5:27p Aleggett
 * Exploding barrels!  What game does this remind you of?
 * 
 * 11    3/20/97 3:12p Aleggett
 * Added UNBREAKABLE flag to barrels, and variable health.
 * 
 * 10    3/18/97 5:24p Jweier
 * Added FL_PUSH to barrel
 * 
 * 9     3/18/97 11:31a Rlove
 * Added sword and boulder
 * 
 * 8     3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 7     2/20/97 9:22a Rlove
 * New blocking for barrel
 */
