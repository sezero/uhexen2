/*

Wall Shambler

Pentacles [PEN-tuh-kleez]

World's First 3-D engine wall/ceiling/floor crawler/jumper!!!(?)

MG & JM
*/

/*
Tentacly thing (or not?) That can "crawl" along walls and ceilings
and floors to get to player as well as throw itself, roll up like
a ball, and bounce.  Does acid damage when splatters.  Some may
spit acid.  Some may work together, shooting beams radially inward
to a central spot, which coalesces into a beam that hits enemy
(or large missile?  tracking?)

STEP1:
Designer puts wallsham in map may point it at a surface.  The
Wallsham will begin on that surface.  If the wallsham is not
pointed at a surface, it will look for the closest surface.
This is expensive, so designers should always point the wallsham
at a surface.

STEP2:
Wallsham waits to find an enemy.  If it doesn't have one, it will
stick around and wait unless it has a path, which it will attempt
to follow.  It will also add this entity to a global list of
wallsham enemies until it goes away or dies.  Wallshams NEVER have
another wallsham as an enemy.  If a wallsham doesn't have an enemy
or it's enemy is dead, it consults the list of wallsham enemies and
tries to track/attack the closest if it's visible.

STEP3:
When wallsham finds an enemy, it decides if it can attack.  If
not, it gets closer.  It may be scripted to run away (to, for example,
lead the player into an ambush).  If it can attack, it decides
if it wants to.  If so, does attack (may be launching itself
at enemy like a bouncing ball and splattering on enemy).

STEP4:
Movement.  Wallsham attempts to move towards player.  If it bumps
into a blocking world brush, it redirects it's angles and attaches
to that bumped surface.  If it successfully moves, it checks it's
"stickdir" for a face and attaches itself to it if it finds one
that's 16 units or closer.  It also will change it's angles to
match the slope of this new face.  If it does not find one,
it will move back to where it was (real quick like), and either
prepare to jump, see if it can attack, choose some other direction,
or (very rarely) wait around.  When it's on the ground, it uses
MOVETYPE_WALK, when it's on the walls or cielings, it uses
MOVETYPE_FLY with the FL_FLY flag on, when it jumps, it uses
MOVETYPE_TOSS, and finally, when it throws itself, it uses
MOVETYPE_BOUNCE.

STEP5:
When it dies, if doesn't explode, it deflates and green ooze runs
out that can do damage if stepped in.  This eventually seeps into
the ground.  If it does explode, it explodes into a goopy blob of
guts, tentacles, and acidic slime, this does radius damage, but not
to other wallshams.

*/
$frame ball000      ball001      ball002      ball003      ball004      
$frame ball005      ball006      ball007      ball008      ball009      
$frame ball010      

//
$frame lunge000     lunge001     lunge002     lunge003     lunge004     
$frame lunge005     lunge006     lunge007     lunge008     lunge009     
$frame lunge010     

//
$frame ready000     ready001     ready002     ready003     ready004     
$frame ready005     ready006     ready007     ready008     ready009     
$frame ready010     ready011     ready012     ready013     ready014     
$frame ready015     

//
$frame spit000      spit001      spit002      spit003      spit004      
$frame spit005      spit006      spit007      spit008      spit009      
$frame spit010      

//
$frame walk000      walk001      walk002      walk003      walk004      
$frame walk005      walk006      walk007      walk008      walk009      
$frame walk010      walk011      walk012      walk013      walk014      
$frame walk015      

void(float do_move)face_movechain;
void(entity attacker, float damage)pent_pain;
void SUB_Return()
{
	return;
}

void setnewwalldir ()
{
float shortest, dirnum;
	makevectors (self.angles);
	dirnum=shortest=1;
	self.th_pain=SUB_Return;
//	dprint("Grabbing any wall\n");
	while(dirnum<7)
	{
		if(dirnum==1)
			traceline(self.origin,self.origin+v_forward*64,TRUE,self);
		else if(dirnum==2)
			traceline(self.origin,self.origin-v_forward*64,TRUE,self);
		else if(dirnum==3)
			traceline(self.origin,self.origin+v_right*64,TRUE,self);
		else if(dirnum==4)
			traceline(self.origin,self.origin-v_right*64,TRUE,self);
		else if(dirnum==5)
			traceline(self.origin,self.origin+v_up*64,TRUE,self);
		else
			traceline(self.origin,self.origin-v_up*64,TRUE,self);

		if(trace_fraction<shortest)
		{
			shortest=trace_fraction;
			self.walldir=normalize(trace_endpos - self.origin);
		}
		dirnum+=1;
	}
	if(self.walldir=='0 0 0')
		self.walldir='0 0 -1';//defaults to the floor
	self.th_pain=pent_pain;
	self.th_run();
}

void() flymonster_start_go =
{
entity latch;
	// spread think times so they don't all happen at same time
	if(self.target!=string_null)
		latch=find(world,targetname,self.target);
	else
		latch=world;

	if(!latch)
		setnewwalldir();//objerror("Pentacles: Target Me!!!\n");
	else
	{
		self.walldir=normalize(latch.origin-self.origin);
		if(self.walldir=='0 0 0')
			self.walldir='0 0 -1';//defaults to the floor
		latch.think=SUB_Remove;
		thinktime latch : 3;//Or leave for others?
	}

//	if (!walkmove(0,0, FALSE))
//	{
//		dprint ("flymonster in wall at: ");
//		dprint (vtos(self.origin));
//		dprint ("\n");
//	}

/*	if (self.target)
	{
		self.goalentity = self.pathentity = find(world, targetname, self.target);
//		if (!self.pathentity)
//		{
//			dprint ("Monster can't find target at ");
//			dprint (vtos(self.origin));
//			dprint ("\n");
//		}
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
*/
		self.pausetime = 99999999;
		self.th_stand ();
//	}
};

void pent_pain_anim () [++ $ready000 .. $ready015]//FIXME: Pain frames or some other effect?
{
	self.movechain.frame=self.frame;
	if(cycle_wrapped)
		if(self.walldir=='0 0 0')
			flymonster_start_go();
		else
			self.th_run();
}

void pent_pain (entity attacker, float damage)
{
	sound (self, CHAN_VOICE, "pent/pain.wav", 1, ATTN_NORM);
	if (random()*70 > damage)
		return;		// didn't flinch

	pent_pain_anim ();
}

void pent_unball () [-- $ball010 .. $ball000]
{
	self.movechain.frame=self.frame;
	if(self.frame==$ball000)
	{
		self.think=self.th_run;
		self.nextthink=0.05;
	}
}

void pent_explode ()
{
	if(self.movechain!=world)
		if(self.movechain.model=="models/pent.mdl")
			remove(self.movechain);
	sound(self,CHAN_BODY,"weapons/expsmall.wav",1,ATTN_NORM);
	MultiExplode();
}

void pent_hit ()
{
vector hitdir;
//float grab;
	if(other.takedamage&&other.classname!="monster_pentacles")
		pent_explode();
	else if(other.solid==SOLID_BSP&&self.safe_time<time)
	{
/*		if(self.movedir=='0 0 0')
		{
			if(self.velocity=='0 0 0')
				objerror("Pentacles: no velocity on hit!!!\n");
		}*/
		hitdir=normalize(self.velocity);
		self.velocity='0 0 0';
		tracearea(self.origin,self.origin+hitdir*64,self.mins*0.5,self.maxs*0.5,TRUE,self);
		if(trace_plane_normal=='0 0 0');
		{
//			dprint("trace_area failed, using traceline\n");
			traceline(self.origin,self.origin+hitdir*64,TRUE,self);
		}

/*		if(trace_fraction==1)
			dprint("ERROR: Hit wall, but it's not there!");
		else if(trace_plane_normal=='0 0 0')
			dprint("ERROR: Hit wall with no normal!");
		else if('0 0 1' * trace_plane_normal > 0.3)
		{
			if(random()<0.2)//least chance- land on floor
				grab = TRUE;
			else
				grab = FALSE;
		}
		else if('0 0 -1' * trace_plane_normal > 0.3)//best chance to land on ceiling
			grab = TRUE;
		else// if(random()<0.8)//50/50 chance to grab walls
			grab = TRUE;
		else
			grab = FALSE;
		if(grab)
		{
*/
			sound (self, CHAN_BODY, "pent/latch.wav", 1, ATTN_NORM);
			self.frags=0;
//			dprint("Grabbed new wall\n");
			self.walldir='0 0 0' - trace_plane_normal;
			if(self.walldir=='0 0 0')
				setnewwalldir();
			if(self.walldir=='0 0 0')
			{
//				dprint("WHAT THE FUCK!@!!!!!\n");
				self.walldir='0 0 -1';
			}
			self.touch=SUB_Null;
			self.movechain.avelocity='0 0 0';
			self.movetype=MOVETYPE_STEP;
			self.flags(+)FL_FLY;
			self.pos1=self.pos2=self.origin;
			face_movechain(FALSE);
			self.think=pent_unball;
			thinktime self : 0.05;

/*		}
		else
		{
			if(self.velocity=='0 0 0')
			{
				self.frags=0;
				dprint("Grabbed new wall\n");
				self.touch=SUB_Return;
				self.movechain.avelocity='0 0 0';
				self.movetype=MOVETYPE_STEP;
				self.flags(+)FL_FLY;
				self.think=setnewwalldir;
				thinktime self : 0.05;
			}
			else
			{
				self.flags(-)FL_ONGROUND;
				self.think=setnewwalldir;
				thinktime self : 1.5;
			}
		}
*/
	}
}

void pent_keep_vel ()
{
	self.think=pent_keep_vel;
	if(self.velocity!='0 0 0')
		self.movedir=normalize(self.velocity);
	else if(self.movedir)
		self.velocity=self.movedir*500;
//	else
//		dprint("pent in air with no vel!\n");
	thinktime self : 0.05;
}

void pent_ball () [++ $ball000 .. $ball010]
{
	self.movechain.frame=self.frame;
	if(self.velocity!='0 0 0')
		self.movedir=normalize(self.velocity);
	else if(self.movedir)
		self.velocity=self.movedir*500;
//	else//remdp
//		dprint("pent in air with no vel!\n");
	if(self.frame==$ball010)
	{
		self.think=pent_keep_vel;
		thinktime self : 0;
//		self.think=self.th_run;
//		self.nextthink=-1;
	}
}

void pent_throw () [++ $lunge000 .. $lunge010]
{//FIXME: Prepare- go into a ball
	self.movechain.frame=self.frame;
	if(self.movetype!=MOVETYPE_STEP)
		return;
//	dprint("Throwing self\n");
	if(self.frame==$lunge010)
	{
		self.safe_time=time+0.05;
		sound (self, CHAN_VOICE, "pent/jump.wav", 1, ATTN_NORM);
		self.movetype=MOVETYPE_BOUNCE;
		self.flags(-)FL_FLY;
		if(self.frags>1)
		{
			self.velocity_z=800*(self.frags/200);
			self.movechain.avelocity=randomv('-300 -300 -300','300 300 300');
			self.think=pent_ball;
			thinktime self : 0.05;
		}
		else if(self.frags==1&&!visible(self.enemy))
		{
			self.weaponframe_cnt=0;
			self.velocity=('0 0 0' - self.walldir)*500+'0 0 150';
			self.movechain.avelocity=randomv('-300 -300 -300','300 300 300');
			self.think=pent_ball;
			thinktime self : 0.05;
		}
		else
		{
			self.weaponframe_cnt=0;
			self.velocity=normalize(self.enemy.origin+self.enemy.view_ofs-self.origin)*500+'0 0 150';
			self.movechain.angles=vectoangles(self.velocity);
			self.movechain.angles_y=0;
			self.movechain.angles_z-=90;
			self.movechain.avelocity_z=random()*600 - 300;
			self.think=pent_keep_vel;
			thinktime self : 0;
//		self.think=self.th_run;
//		self.nextthink=-1;
		}
		self.pain_finished = time + 7;
		self.flags(-)FL_ONGROUND;
		self.touch=pent_hit;
	}
}

void pent_fly () [++ $lunge000 .. $lunge010]
{
	self.movechain.frame=self.frame;
	if(self.movetype!=MOVETYPE_STEP)
		return;
	if(self.frame==$lunge010)
	{
		self.weaponframe_cnt=0;
		self.safe_time=time+0.05;
		sound (self, CHAN_VOICE, "pent/jump.wav", 1, ATTN_NORM);
		self.movetype=MOVETYPE_BOUNCEMISSILE;//FLY;
		self.flags(-)FL_FLY;
		if(self.hull!=HULL_HYDRA)
		{
//			dprint("Hull fucked\n");
			self.hull=HULL_HYDRA;
		}
/*		dprint("flying\n");
		dprintv("origin: %s\n",self.origin);
		dprintv("pos_ofs: %s\n",self.pos_ofs);
*/
		self.velocity=normalize(self.pos_ofs-self.origin)*800;
//		dprintv("velocity: %s\n",self.velocity);
		self.movechain.angles=vectoangles(self.velocity);
		self.movechain.angles_y=0;
		self.movechain.angles_z-=90;
		self.movechain.avelocity_z=random()*600 - 300;
		self.think=pent_keep_vel;
		thinktime self : 0;
//		self.think=self.th_run;
//		self.nextthink=-1;

		self.pain_finished = time + 7;
		self.flags(-)FL_ONGROUND;
		self.touch=pent_hit;
	}
}

void pent_find_near_wall ()
{
float try_cnt,foundwall,enemy_dist;
vector dest_spot;
	enemy_dist=vlen(self.enemy.origin-self.origin);
	if(enemy_dist>512)
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}
	while(!foundwall&&try_cnt<10)
	{
		dest_spot=normalize(self.enemy.origin+self.enemy.view_ofs-self.origin);
		dest_spot=vectoangles(dest_spot);
		makevectors(dest_spot);
		dest_spot=v_forward+v_right*(random() - 0.5)+v_up*(random() - 0.5);
		dest_spot=self.origin+dest_spot*(enemy_dist*1.2);
		tracearea(self.origin,dest_spot,self.mins,self.maxs,FALSE,self);
		if(trace_ent.solid==SOLID_BSP&&trace_fraction<1)
			foundwall=TRUE;
		else
			try_cnt+=1;
	}
	if(try_cnt>=10||trace_endpos==self.origin)
	{
		self.think=self.th_run;
		thinktime self : 0;
		return;
	}
	self.pos_ofs=trace_endpos;
	pent_fly();
}

void() pent_spitball =
{
entity missile;
	self.effects(+)EF_MUZZLEFLASH;
	missile=spawn();
	missile.classname="acid missile";
	missile.owner=self;
	missile.drawflags(+)SCALE_ORIGIN_CENTER|MLS_FIREFLICKER;
	missile.scale=.7;
	missile.abslight=1;
	missile.frags=FALSE;
	missile.movetype=MOVETYPE_FLYMISSILE;

	missile.solid=SOLID_BBOX;
	missile.touch=BloodMissileTouch;

	missile.dmg=random(2,7)*skill+1;

	missile.speed=800;
	missile.velocity=normalize(self.enemy.origin+self.enemy.proj_ofs - self.origin)*missile.speed+aim_adjust(self.enemy);
	missile.avelocity=randomv('0 0 -400','0 0 400');
	missile.movedir=normalize(missile.velocity);
	missile.angles=vectoangles(missile.velocity);

	setmodel(missile,"models/sucwp1p.mdl");
//	setmodel(missile,"models/sucwp2p.mdl");
	setsize(missile,'0 0 0','0 0 0');

	setorigin(missile,self.origin/*+v_forward*8*/);
	sound(missile,CHAN_AUTO,"pent/fire.wav",1,ATTN_NORM);

	missile.think=SUB_Remove;//AcidMissileFade;
	thinktime missile : 2;
};

void pent_spit () [++ $spit000 .. $spit010]
{
	self.movechain.frame=self.frame;
	if(self.frame==$spit010)
	{
		pent_spitball();
		self.th_run();
	}
}

void pent_deflate () [++ $ball000 .. $ball010]//fixme: death frames?
{
	if(self.frame==$ball010)
	{
		self.movetype=MOVETYPE_BOUNCE;//was _STEP
		self.flags(-)FL_FLY;
		MakeSolidCorpse();
	}
}

void pent_die ()
{
	if(self.health<-20||self.health>0)
		pent_explode();
	else
	{
		setmodel (self, "models/pent.mdl");
		if(self.movechain!=world)
			if(self.movechain.model=="models/pent.mdl")
			{
				self.frame=self.movechain.frame;
				self.angles=self.movechain.angles;
			}
		self.scale=1.7;
		self.effects(-)EF_NODRAW;
		if(self.movechain!=world)
			if(self.movechain.model=="models/pent.mdl")
				remove(self.movechain);
		sound (self, CHAN_VOICE, "pent/die.wav", 1, ATTN_NORM);
		pent_deflate();
	}
}

float MAX_PENTJUMP = 256;
float pent_check_attack()
{
vector	spot1, spot2;
entity	targ;
float	chance,targ_range;

	targ = self.enemy;
	
// see if any entities are in the way of the shot
	spot1 = self.origin;
	spot2 = (targ.absmin+targ.absmax)*0.5;

	traceline (spot1, spot2, FALSE, self);

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != targ)
		if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
			return FALSE;//Don't have a clear shot, and don't want to shoot obstruction
			
//FIXME: check for translucent water?
//	if (trace_inopen && trace_inwater)
//		return FALSE;			// sight line crossed contents

	targ_range = vlen(self.enemy.origin+self.enemy.view_ofs - self.origin);
	
	if(self.movetype==MOVETYPE_STEP)
	{
		chance=random()*MAX_PENTJUMP;
		if(chance > targ_range&&random()>0.8)//checks 20 times a second, so may jump once every sec, max
		{	// melee attack
			self.th_melee ();
			return TRUE;
		}
	}
//FIXME: check for darkness, maybe won't fire, maybe aim will be off
	
// missile attack
	if (time < self.attack_finished)
		return FALSE;
		
	if (enemy_range == RANGE_MELEE)
	{
		chance = 0.9;
		self.attack_finished = 0;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.3;
	}
	else if (enemy_range == RANGE_MID)
		chance = 0.15;
	else
		chance = 0.05;

	if(skill>0)
		chance*=skill;

	if (random () < chance)
	{
		self.th_missile ();
		SUB_AttackFinished (random(0,3 - skill));
		return TRUE;
	}
	else if(targ_range<=random(384,512)&&random()<0.5+(skill/5))
	{
		pent_find_near_wall();
		SUB_AttackFinished (random(0,2 - (skill/2)));
		return TRUE;
	}

	return FALSE;
}

float MAX_PENTDIST = 32;
void pent_check_wall_change ()
{
	self.pos2=self.origin;
	if(self.pos2!=self.pos1)
	{
//		dprint("Checking pos change\n");
		self.movedir=normalize(self.pos2-self.pos1)*32;
		tracearea(self.origin,self.origin+self.movedir,self.mins*0.5,self.maxs*0.5,FALSE,self);
		if(trace_fraction<1&&trace_plane_normal!='0 0 0'&&trace_ent.solid==SOLID_BSP)
		{	
//			dprintv("New wall, normal : %s\n",trace_plane_normal);
			self.walldir='0 0 0'-trace_plane_normal;
			setorigin(self,trace_endpos-self.walldir*vlen(self.size)*0.5);
		}
/*		else if(trace_allsolid)
			dprint("trace entirely in wall\n");
		else if(trace_startsolid)
			dprint("trace started in wall\n");
		else if(trace_fraction>=1)
			dprint("Trace is 1\n");
		else if(trace_plane_normal=='0 0 0')
			dprint("No normal for plane\n");
		else
		{
			dprint(trace_ent.classname);
			dprint(" not solid_bsp\n");
		}
*/	}
//	else
//		dprint("Didn't move!\n");
	self.pos1=self.origin;
}

void offset_movechain ()
{
vector offset;
float ofs_dist,dot;
	if(self.movechain!=world)
		if(self.movechain.model=="models/pent.mdl")
			makevectors(self.movechain.angles);
	dot=v_up*'0 0 -1';
	ofs_dist=8+8*dot;
	offset=v_up*ofs_dist;
		if(self.movechain!=world)
			if(self.movechain.model=="models/pent.mdl")
				setorigin(self.movechain,self.origin+offset);
}

void face_movechain (float do_move)
{
	if(self.movetype!=MOVETYPE_STEP)
		return;

	if(self.movechain==world)
		remove(self);
	if(self.movechain.model!="models/pent.mdl")
		remove(self);

float dist;
//vector new_angles,new_angles2,forward1;
//	traceline(self.origin,self.origin+self.walldir*64,TRUE,self);
	if(self.walldir=='0 0 0')
	{
		setnewwalldir();
		//		dprint("ERROR: Pent wall dir is zero!!!\n");
		//self.nextthink = -1;
		//self.think=SUB_Return;
		//self.effects(+)EF_DIMLIGHT;
	}
	else if(pointcontents(self.origin)==CONTENT_SOLID)
	{
//		dprint("ERROR: Pent in wall!!!\n");
		setorigin(self,self.oldorigin);
//		self.nextthink = -1;
//		self.think=SUB_Return;
//		self.effects(+)EF_DIMLIGHT;
	}
	else
	{
		if(do_move)
			tracearea(self.origin,self.origin+self.walldir*MAX_PENTDIST,self.mins,self.maxs,FALSE,self);
		else
			traceline(self.origin,self.origin+self.walldir*MAX_PENTDIST,TRUE,self);
		//FIXME - look back a little... oldorigin?
	//	if(trace_fraction==1)
	//			self.movedir = normalize(self.oldorigin - self.origin) + self.walldir;
	//		tracearea(self.origin,self.origin+self.movedir*64,self.mins,self.maxs,FALSE,self);
		dist=trace_fraction*MAX_PENTDIST;
		if(trace_allsolid||trace_startsolid)//||trace_plane_normal=='0 0 0')
			dist=0;

		if(trace_plane_normal=='0 0 0')
			dist=MAX_PENTDIST;

		if(trace_ent.solid==SOLID_BSP&&dist>self.speed&&dist<MAX_PENTDIST)
		{//Fixme: do a movestep here instead...
//			dprint("Moving along wall\n");
			if(do_move)
			{
				setorigin(self,trace_endpos);//self.origin+self.walldir*dist);
				pent_check_wall_change();
			}
			self.movechain.angles='0 0 0';
			matchAngleToSlope(trace_plane_normal,self.movechain);
			//offset_movechain();
//			new_angles = vectoangles(trace_plane_normal);
//			makevectors(self.movechain.angles);//was new_angles
//			forward1 = self.movedir - v_forward;
//			new_angles2 = vectoangles(forward1);
//			new_angles2 *=-1;
//			self.movechain.angles += new_angles2;
		//	self.movechain.angles=new_angles;
		//	self.movechain.angles=vectoangles(self.movedir);
		}
		else if(dist>=MAX_PENTDIST)
		{
//			dprint("Ran out of wall!\n");
			self.pos1=self.pos2='0 0 0';
			self.frags=TRUE;//Grab next wall
			self.th_melee();
		}
		else if(dist<=self.speed)
		{
			if(do_move)
				pent_check_wall_change();
			self.movechain.angles='0 0 0';
			matchAngleToSlope(trace_plane_normal,self.movechain);
			//offset_movechain();
//			new_angles = vectoangles(trace_plane_normal);
//			makevectors(self.movechain.angles);//was new_angles
//			forward1 = self.movedir - v_forward;
//			new_angles2 = vectoangles(forward1);
//			new_angles2_y *=-1;
//			self.movechain.angles += new_angles2;
		//	self.movechain.angles=new_angles;
		//	self.movechain.angles=vectoangles(self.movedir);
		}
		if(self.level==666)
		{
			self.movechain.avelocity='0 100 0';
			self.think=SUB_Return;
			thinktime self: -1;
		}
	}
}

void pent_up_down (float movespeed)
{
float goalheight,dot,goaldist;
	makevectors(self.angles);
	dot='0 0 1'*self.walldir;
	if(dot<-0.7&&random()<0.3&&self.weaponframe_cnt+3<time)//on a almost flat surface, try to jump to cieling every three seconds
	{//Make them do this if can't see enemy?
		traceline(self.origin,self.origin+'0 0 512',FALSE,self);
		if(trace_fraction<1&&trace_ent.solid==SOLID_BSP)//Roof within 132 above
		{//Jump to ceiling
			self.weaponframe_cnt=time;
			self.frags=trace_fraction*512;
			self.th_melee();
		}
	}
	else if(dot<0.5&&dot>-0.5)//If on a 45 degree slope or more
		if(visible(self.enemy))//if(visible(self.goalentity))
		{
			goalheight=self.enemy.origin_z+self.enemy.view_ofs_z;
			if(goalheight!=self.origin_z)
			{
				goaldist=goalheight-self.origin_z;
				if(fabs(goaldist)>movespeed)
					if(goaldist>0)
						goaldist=movespeed;
					else
						goaldist=0 - movespeed;
				movestep(0,0,goaldist, FALSE);
			}
		}
		else if(self.t_width>time)//Last dir change
		{//This is getting canceled out in C- C FL_FLY monsters do a Z move towards goal!
//			dprintf("Moving %s\n",self.level);
			self.flags(-)FL_NOZ;
			if(!movestep(0,0,self.level, FALSE))
				self.t_width=0;
			else
				self.flags(+)FL_NOZ;
		}
		else
		{
			self.flags(-)FL_NOZ;
			if(random()<0.5)
			{
				if(random()<0.8)
				{
					if(!movestep(0,0,movespeed, FALSE))
					{
						if(movestep(0,0,0 - movespeed, FALSE)&&random()<0.5)
						{
							self.level=0 - movespeed;
							self.t_width=time+7;
							self.flags(+)FL_NOZ;
						}
					}
					else
					{
						self.level=movespeed;
						self.t_width=time+7;
						self.flags(+)FL_NOZ;
					}
				}
				else if(!movestep(0,0,0 - movespeed, FALSE))
				{
					if(movestep(0,0,movespeed, FALSE))
					{
						self.level=movespeed;
						self.t_width=time+7;
						self.flags(+)FL_NOZ;
					}
				}
				else
				{
					self.level=0 - movespeed;
					self.t_width=time+7;
					self.flags(+)FL_NOZ;
				}
			}
			else if(visible(self.goalentity))//if(visible(self.goalentity))
			{
				goalheight=self.goalentity.origin_z+self.goalentity.view_ofs_z;
				if(goalheight!=self.origin_z)
				{
					goaldist=goalheight-self.origin_z;
					if(fabs(goaldist)>movespeed)
						if(goaldist>0)
							goaldist=movespeed;
						else
							goaldist=0 - movespeed;
					movestep(0,0,goaldist, FALSE);
				}
			}
		}
}

void pent_run () [++ $walk000 .. $walk015]
{
	if(self.movetype==MOVETYPE_STEP)
	{
		self.movechain.frame=self.frame;
		self.velocity='0 0 0';
//		self.flags(-)FL_ONGROUND;
		if(self.frame>$walk008 &&self.frame<=$walk015)
		{
//			self.pos1=self.origin;
			ai_run(self.speed);
			if(self.flags&FL_ONGROUND&&'0 0 -1'*self.walldir>0.75)
			{
				traceline(self.origin,self.origin - '0 0 64',TRUE,self);
				if(trace_plane_normal!='0 0 0')
					self.walldir = '0 0 0' - trace_plane_normal;
//				dprintv("On ground, New walldir = %s\n",self.walldir);
			}
			if(random()<0.5)
				pent_up_down(self.speed);
			face_movechain(TRUE);
		}
	}
	else if(self.movetype==MOVETYPE_BOUNCE&&(self.velocity=='0 0 0'||self.pain_finished<time))
	{
//		dprint("Fixing bounce\n");
		self.touch=SUB_Return;
		self.velocity='0 0 0';
		self.movechain.avelocity='0 0 0';
		self.flags(-)FL_ONGROUND;
		self.movetype=MOVETYPE_STEP;
		self.flags(+)FL_FLY;
		setnewwalldir();
	}
}

void pent_walk () [++ $walk000 .. $walk015]
{
	if(self.movetype==MOVETYPE_STEP)
	{
		self.movechain.frame=self.frame;
		self.velocity='0 0 0';
//		self.flags(-)FL_ONGROUND;
		if(self.frame>$walk008 &&self.frame<=$walk015)
		{
//			self.pos1=self.origin;
			ai_walk(self.speed/2);
			if(self.flags&FL_ONGROUND&&'0 0 -1'*self.walldir>0.75)
			{
				traceline(self.origin,self.origin - '0 0 64',TRUE,self);
				if(trace_plane_normal!='0 0 0')
					self.walldir = '0 0 0' - trace_plane_normal;
//				dprintv("On ground, New walldir = %s\n",self.walldir);
			}
			if(random()<0.5)
				pent_up_down(self.speed/2);
			face_movechain(TRUE);
		}
	}
}

void pent_stand () [++ $ready000 .. $ready015]
{
	if(self.movetype==MOVETYPE_STEP)
	{
		self.movechain.frame=self.frame;
		self.velocity='0 0 0';
		self.flags(-)FL_ONGROUND;
//		self.pos1=self.origin;
		ai_stand();
		face_movechain(TRUE);
	}
}

/*QUAKED monster_pentacles (1 0 0) (-8 -8 -8) (8 8 8) STATIONARY ONESURF NOFLOOR NOJUMP NOSPIT
All sides must be at least 8 away from the walls
'speed' (default = 8)
'experience_value' default = 100
skin
	0 (default) = Brown Rocky
	1	=	White Snowy

The following are not implemented- if you want them implemented, let me know, otherwise, they will not be.
STATIONARY - Will not move from it's spot
ONESURF - Will not move from it's current wall or surface to a new one
NOFLOOR - Will not move around on the floor
NOJUMP - Will not jump off the walls
NOSPIT - Will not shoot
*/
void() monster_pentacles =
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	if(!self.th_init)
	{
		self.th_init=monster_pentacles;
		self.init_org=self.origin;
	}

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model4 ("models/pent.mdl");
		precache_model4 ("models/sucwp1p.mdl");
	//	precache_model4 ("models/sucwp2p.mdl");
		precache_model4 ("models/tempmetr.mdl");//temp- meteor projectile
		precache_sound4 ("succubus/blobexpl.wav");
		precache_sound4 ("succubus/dropfizz.wav");
		precache_sound4 ("succubus/brnwall.wav");
		precache_sound4 ("succubus/brnhit.wav");
		precache_sound4 ("succubus/brnfire.wav");
		precache_sound4 ("pent/fire.wav");
		precache_sound4 ("pent/jump.wav");
		precache_sound4 ("pent/die.wav");
		precache_sound4 ("pent/pain.wav");
		precache_sound4 ("pent/latch.wav");
	}

    self.solid = SOLID_PHASE;
	self.movetype = MOVETYPE_STEP;
	if(self.skin==1)
		self.thingtype = THINGTYPE_ICE;
	else
		self.thingtype = THINGTYPE_BROWNSTONE;
	if(!self.experience_value)
		self.experience_value = 100;
	setmodel (self, "models/null.spr");
	self.effects=EF_NODRAW;

	self.movechain=spawn();
	setmodel (self.movechain, "models/pent.mdl");
	self.movechain.solid=SOLID_NOT;
	self.movechain.movetype=MOVETYPE_NOCLIP;
	setsize(self.movechain,'0 0 0','0 0 0');
	self.movechain.hull=HULL_POINT;
	setorigin(self.movechain,self.origin);
	self.movechain.angles=self.angles;
	self.movechain.owner=self;
	self.movechain.scale=1.7;
	self.movechain.skin=self.skin;
	
	setsize (self, '-8 -8 -8', '8 8 8');
	self.hull=HULL_HYDRA;

	if(!self.health)
		self.health = 40 + skill * 10;
	if(!self.max_health)
		self.max_health=self.health;

	self.mass = 25;
	self.dmg = 50 + skill*20;
	if(coop)
		self.dmg+=20;//chunks don't do damage

	self.th_stand = pent_stand;
	self.th_walk = pent_walk;
	self.th_run = pent_run;
	self.th_missile = pent_spit;
	self.th_melee = pent_throw;
	self.th_pain = pent_pain;
	self.th_die = pent_die;

	self.takedamage=DAMAGE_YES;
	self.flags2(+)FL_ALIVE;
	total_monsters = total_monsters + 1;

	self.ideal_yaw = self.angles * '0 1 0';
	self.yaw_speed = 10;
	if(!self.speed)
		self.speed = 8;

	self.use = monster_use;
	self.pos1=self.pos2=self.origin;

	self.init_exp_val = self.experience_value;
	self.flags(+)FL_FLY;
	self.flags(+)FL_MONSTER;
	self.flags(+)FL_HUNTFACE|FL_SET_TRACE;
	self.nextthink+=random(0.5);
	self.think = flymonster_start_go;
};

