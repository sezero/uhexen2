/*
====================================================================
PROJBHVR.HC
Projectile Behaviours
By Michael Gummelt

Various routines for projectile behaviours.
===================================================================
*/
void(float num_bubbles) DeathBubbles;

/*
====================================================
void Skip()
MG
Returns True if the projectile hit a surface at
a slight enough angle to deflect.  False if not
(meaning the missile should be destroyed in
the touch function this was called from).
The missile	must be a MOVETYPE_BOUNCEMISSILE for 
this to work, and it's o_angle must match it's 
last known velocity.  For now the angle of 
deflection is anything below the 0.2 (using the dot
product of the negative value of the o_angle
and the trace_plane_normal of the surface it
hit.)  But this could be easily customized to
use a parameter or entity field.
Values Used: .o_angle (.movetype must be 
	MOVETYPE_BOUNCEMISSILE)
=====================================================
*/
float Skip (void)
{
	vector dir1,dir2;
	float dot;
	dir1 = normalize(self.velocity);
	traceline(self.origin-dir1*4,self.origin+dir1*16,TRUE,self);
	dir2=trace_plane_normal;
	dir1*=-1;
	dot=dir1*dir2;

	if(dot<=0.15&&trace_fraction<1)
		return TRUE;
	else
		return FALSE;
}

/*
====================================================
void Veer(float amount)
MG
This function will make a projectile
wander from it's course in a random
manner.  It does not actually directly
use the .veer value, you must send the
veer amount value to the function as
a parameter.  But this allows it to
be used in other ways (call it once,
etc.)  So you can call it by using
Veer(self.veer) or Veer(random()*300)
or Veer([any number]), etc.
=====================================================
*/
void Veer(float amount)
{
//useful code for making projectiles wander randomly to a specified degree
vector veerdir;
		veerdir_x=veerdir_y=veerdir_z=amount;
		self.velocity+=RandomVector(veerdir);
		self.angles=vectoangles(self.velocity);
}

void VeerThink ()
{
	Veer(self.veer);
	if(self.think==Veer)
		thinktime self : 0.1;
}

/*
=========================================================
float ahead (entity loser, entity from)
MG

Checks to see if "loser" is within the forward cone
(based on facing, NOT velocity!) of "from".
Cone size defaults to 0.2 unless crossbow arrows (they
do a one-time narrow-cone aquisition of 0.8)

NOTE: "accept" can be a value between -1 and 1, the
	higher the value, the smaller the cone.

Returns TRUE if so, FALSE if not.
=========================================================
*/
float ahead (entity loser, entity from)
{
vector proj_dir, spot1, spot2, vec;
float accept, dot;

	proj_dir=normalize(from.velocity);

	spot1 = from.origin;
	spot2 = (loser.absmin+loser.absmax)*0.5;

	if(from.classname=="flaming arrow"||from.classname=="bolt")
		accept=0.875;
	else
		accept=0.2;
	
	vec = normalize (spot2 - spot1);
	dot = vec * proj_dir;
	
	if ( dot > accept)
		return TRUE;

	return FALSE;
}

/*
=========================================================
float heading (entity loser, entity from, float accept)
MG

Checks to see if "loser" is within the forward cone
(based on velocity, NOT facing!) of "from".

"accept" is the size of the cone (see "ahead"), which,
if none is sent, defaults to 0.8 (rather narrow).

Returns TRUE if so, FALSE if not.
=========================================================
*/
float heading (entity loser, entity from, float accept)
{
vector proj_dir, spot1, spot2, vec;
float dot;

	proj_dir=normalize(from.velocity);

	spot1 = from.origin;
	spot2 = (loser.absmin+loser.absmax)*0.5;

	if(!accept)
		accept=0.8;
	
	vec = normalize (spot2 - spot1);
	dot = vec * proj_dir;
	
	if ( dot > accept)
		return TRUE;

	return FALSE;
}

void()HomeThink;
/*
======================================
entity HomeFindTarget()
MG
Simply looks for and returns a target
to go after.
======================================
*/
entity HomeFindTarget()
{
entity loser;
float dist, bestdist;	
	
	if(self.think!=HomeThink)//one-time only acquisition
		bestdist=5000;
	else
		bestdist=1000;
	loser=findradius(self.origin,bestdist);
	bestdist+=1;
	while (loser)
	{
		if(loser.health&&loser.takedamage&&(loser.flags2&FL_ALIVE)&&visible(loser)&&loser!=self&&loser!=world&&loser!=self.owner&&!other.effects&EF_NODRAW)//&&!(loser.artifact_active&ARTFLAG_STONED) Why Not?
			if((!self.aflag||self.ideal_yaw)&&!ahead(loser,self))			//looks for someone in front first time
				dprint("");//not infront\n");
			else if(teamplay&&loser.classname=="player"&&((loser.team==self.owner.team&&self.owner.classname=="player")||(loser.team==self.controller.team&&self.owner.classname=="player")))
				dprint("");//targeting teammate\n");
			else if(coop&&loser.classname=="player"&&(self.owner.classname=="player"||self.controller.classname=="player"))
				dprint("");//target coop player\n");
			else if((self.classname=="flame arrow"||self.classname=="bolt")&&deathmatch&&vlen(loser.velocity)>300)
				dprint("");//DM: player moving too fast\n");
			else
			{
			//make it wait for closest (by vlen) or just go for first found?
				dist=vlen(self.origin-loser.origin);
				if(dist<bestdist)
				{
					bestdist=dist;
					self.enemy=loser;
				}
				if(bestdist<100)//don't look for anything close, that's good enough
				{
					self.aflag=TRUE;
					return self.enemy;
				}
			}
		loser=loser.chain;
	}
	self.aflag=TRUE;
	if(self.enemy)
		return self.enemy;
	else
		return world;
}

/*
===================================================================
void HomeThink()
MG
Makes a projectile find a visible enemy
and head towards it.  If you set the projectile's
.ideal_yaw to TRUE, it will only look in front
of itself for enemies (makes for poor tracking,
but this is desirable sometimes).  If the value is
FALSE, It will look in front until it locks onto
something, once it does that, it will look
for anything in any direction.  This way when you
fire it in a certain direction, it has
a better chance of locking onto something
in front of you, not the closest thing (which
could likely be behind you.  You can go to
this function after a delay so a projectile 
won't start homing until it's been in the
air for a little.  You can set how often the 
projectile will correct it's velocity with 
the .homerate value, which is the .nextthink of
this function, so the lower the number, 
the more active the homing.  Also, HomeThink has
a built in Veer() function call, to give it some
randomness.  If you don't want it to veer,
just set the .veer to FALSE.
The rate of turning is controlled by the
.turn_time value.  A value of 0 will make
it turn instantly, a value of 1 will take the
average of the current and desired velocities.
Higher values will result in a LOWER turn
rate.  Here's the calculation, simplified a bit:
new_v=(old_v*self.turn_time + ideal_v)/(self.turn_time+1)
The speed of the projectile is controlled by
self.speed.
If .hoverz is true, the projectile will slow down
the tighter the turn is- makes for better tracking.
If the Homethink is the think of the projectile,
it will check for .lifetime, if the lifetime has
expired, it will execute it's th_die function,
which you must declare!
Note that you can give a projectile an enemy and
it will start tracking that one.  If you give the
projectile a lockentity, it will never try to
acquire a different entity.
Values Used: .speed, .homerate, .veer,
	.turn_time, .ideal_yaw, .lifetime, .th_die,
	.hoverz, .enemy, .lockentity
============================================================
*/
void HomeThink()
{
	local vector huntdir;

	if(self.thingtype==THINGTYPE_FIRE)
	{
		local float waterornot;
		waterornot=pointcontents(self.origin);
		if(waterornot==CONTENT_WATER||waterornot==CONTENT_SLIME)
			DeathBubbles(1);
	}

	if(self.enemy!=world&&!self.lockentity)
		if(!visible(self.enemy)||!self.enemy.health||!self.enemy.flags2&FL_ALIVE)
		{
		//if you can't see him, don't track (and look for someone else?)
			self.oldenemy=self.enemy;//remember him
			self.enemy=world;
		}
	if(self.enemy==world)
	{
		if(random()<0.3||self.think!=HomeThink)//findradius was too damn costly!!!
			HomeFindTarget();
		if(self.enemy==world&&self.oldenemy!=world&&visible(self.oldenemy)&&self.oldenemy.health&&(self.oldenemy.flags2&FL_ALIVE))
			self.enemy=self.oldenemy;
	}

	if(self.enemy!=world&&visible(self.enemy))
	{
	vector olddir, newdir;
	float oldvelmult , newveldiv, speed_mod;
		olddir=normalize(self.velocity);
		if(self.enemy.classname=="player"&&self.enemy.view_ofs!='0 0 0')
			huntdir=self.enemy.origin+self.enemy.view_ofs;
		else
			huntdir=(self.enemy.absmin+self.enemy.absmax)*0.5;
		huntdir = normalize(huntdir-self.origin);
		oldvelmult = self.turn_time;
		newveldiv = 1/(self.turn_time + 1);
		newdir=(olddir*oldvelmult + huntdir)*newveldiv;
		if(self.hoverz)//Slow down on turns
			speed_mod=olddir*newdir;
		else
			speed_mod=1;
		if(speed_mod<0.05)
			speed_mod=0.05;
		if(self.velocity!=huntdir*self.speed)
			self.velocity=(olddir*oldvelmult + huntdir)*newveldiv*self.speed*speed_mod;
	}
	//give slight waver
	if(self.veer)
		Veer(self.veer);
	self.movedir=normalize(self.velocity);
	if(self.think==HomeThink)
	{
		if(self.lifetime<time)
			self.th_die();
		else
			thinktime self : self.homerate;
	}
}

/*
=====================================================
void SpiralThink()
MG

Makes a spinning projectile move in a spiral
pattern based on how fast it's spinning.
NOTE: Has to be rolling (have avelocity_z) to work
Values used:
.movedir is forward,
.speed is speed;
.anglespeed is spiral width
.cnt is how much to increase anglespeed each think
====================================================
*/
void SpiralThink()
{
vector newangles;
	makevectors(self.angles);
	self.velocity=self.movedir*self.speed;
	newangles=vectoangles(self.velocity);
	self.angles_y=newangles_y;
	self.angles_x=newangles_x;
	if(self.cnt!=0)
		self.anglespeed+=self.cnt;
	self.velocity+=v_right*self.anglespeed;
	if(self.think==SpiralThink)
		thinktime self : 0.1;
}

/*
=====================================================
void Missile_Arc (void)
MG

Simulates pull of gravity on missiles without
independant means of propulsion, without having
to use MOVETYPE_BOUNCE where things fall too fast.
Also adjusts the pitch to the new dir, which is
nice for arrows and ballista shots.
A good idea is to call this after a delay time
so the missile won't start falling right away.
====================================================
*/
void Missile_Arc (void)
{
vector newpitch;

	self.velocity_z-=60;
	newpitch=vectoangles(self.velocity);
	self.angles_x=newpitch_x;
	self.think=Missile_Arc;
	thinktime self : 0.1;
}
