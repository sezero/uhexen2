/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/crossbow.hc,v 1.1.1.1 2004-11-29 11:31:29 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\crossbow\final\crossbow.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\crossbow\final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     select13     select14     select15     

//
$frame shoot1       shoot2       shoot3       shoot4       shoot5       
$frame shoot6       shoot7       shoot8       shoot9       shoot10      
$frame shoot11      shoot12      shoot13      shoot14      shoot15      
$frame shoot16      shoot17      shoot18      shoot19      


void flashspin ()
{
	if(self.lifetime<time)
		remove(self);
	else
	{
		self.scale+=0.05;
		thinktime self : 0.075;
	}
}

void MakeFlash(vector org)
{
	newmis=spawn_temp();
	newmis.movetype=MOVETYPE_NOCLIP;
	newmis.angles=vectoangles(v_forward);
	newmis.avelocity_z=random(200,700);
	newmis.scale=0.1;
	newmis.drawflags(+)MLS_ABSLIGHT|SCALE_ORIGIN_CENTER;
	newmis.abslight=0.5;
	newmis.lifetime=time+0.3;
	newmis.think=flashspin;
	thinktime newmis : 0;
	setmodel(newmis,"models/arrowhit.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,org);
}

void() FallAndRemove =
{
//self.enemy.movetype?
	traceline(self.origin,self.origin-'0 0 24',TRUE,self);
	if(pointcontents(self.origin)==CONTENT_SOLID||trace_fraction<1)
		remove(self);
	self.movetype=MOVETYPE_BOUNCE;
	self.velocity_z=random(-60,-150);
	self.flags(-)FL_ONGROUND;
	self.avelocity=RandomVector('50 50 50');
	self.think=SUB_Remove;
	thinktime self : self.wait;
};

void() CB_BoltStick=
{
	if(self.wait<=time)
		if(self.classname=="bolt")
		{
			self.wait=2;
			self.think=FallAndRemove;
		}
	    else if(self.classname=="stickmine")
			self.think=MultiExplode;
		else
			self.think=DarkExplosion;
	else if(self.enemy.health<=0&&self.health)
	{
		self.health=0;
		if(self.classname=="bolt")
		{
			self.wait=random(1,3);
			self.think=FallAndRemove;
		}
	    else
		{
			self.movetype=MOVETYPE_BOUNCE;
			self.velocity_z=random(-100,100);
			self.avelocity=RandomVector('50 50 50');
		}
	}
    else if(self.movetype!=MOVETYPE_BOUNCE)
    {
		setorigin(self,self.enemy.origin+self.view_ofs);
	    self.angles=self.o_angle + self.enemy.angles;
        self.think=CB_BoltStick;
    }
    thinktime self : 0;
};

void() CB_BoltHit=
{
	if(other==self.owner||(other.owner==self.owner&&other.classname==self.classname))
	    return;

vector stickdir, stickspot,center;
float rad,stick;
		v_forward=normalize(self.velocity);
		stopSound(self,CHAN_BODY);
		//sound(self,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
		setsize(self,'0 0 0','0 0 0');
		self.takedamage=DAMAGE_NO;
        self.velocity='0 0 0';
        self.movetype=MOVETYPE_NOCLIP;
        self.solid=SOLID_NOT;
        self.touch=SUB_Null;
		self.health=other.health;

		if(other.thingtype==THINGTYPE_FLESH)
	        sound(self, CHAN_WEAPON, "assassin/arr2flsh.wav", 1, ATTN_NORM);
		else if(other.thingtype==THINGTYPE_WOOD)
	        sound(self, CHAN_WEAPON, "assassin/arr2wood.wav", 1, ATTN_NORM);
	    else
			sound(self, CHAN_WEAPON, "weapons/met2stn.wav", 1, ATTN_NORM);

		MakeFlash(self.origin-v_forward*8);

		if(other.takedamage)
		{
			if(self.classname=="bolt")
				T_Damage(other,self,self.owner,10);
			else
				T_Damage(other,self,self.owner,3);
			SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,other);
			if(other.solid!=SOLID_BSP)
			{
//Put it right below view of player
				if(other.classname=="player")
				{
					stickdir_z=other.origin_z+other.proj_ofs_z+ 1;
					stickdir=other.origin+normalize(self.origin-other.origin)*12;
					stick=TRUE;
					setorigin(self,stickdir);
				}
				else
				{
					rad=(other.maxs_x+other.maxs_z)*0.5;
					center=(other.absmax+other.absmin)*0.5;
					stickspot=self.origin+v_forward*other.maxs_x*2;
					if(vlen(center-stickspot)<rad*0.5)
					{
						stick=TRUE;
						setorigin(self,stickspot);
					}
					else
						stick=FALSE;
				}
				self.wait=time + random(0.1,2);
			}
		}
		else
		{
			CreateWhiteSmoke(self.origin-v_forward*8,'0 0 8',HX_FRAME_TIME);
			SpawnPuff(self.origin+v_forward*8,'0 0 0'-v_forward*24,10,world);
			if(self.classname=="bolt")
			{
				if(random()<0.7)
					chunk_death();
				else if(random()<0.5)
				{
					self.movetype=MOVETYPE_BOUNCE;
					self.velocity_z=-20;
					self.flags(-)FL_ONGROUND;
					self.avelocity_x=random(-360,360);
					self.avelocity_y=random(-360,360);
					self.avelocity_z=random(-360,360);
					self.touch=SUB_Null;
					self.think=SUB_Remove;
					thinktime self : random(0.5,1.5);
					return;
				}
			}
			else
				stick=TRUE;
			self.wait=time + random(1,3);
		}

//FIXME: only stick in if thingtype is wood or flesh,
//otherwise, no damage and bounce off!
        if(other.movetype||other.takedamage||stick||self.health)
        {
			if(stick)
			{
				self.enemy=other;
				self.view_ofs=(self.origin-other.origin);
				self.o_angle=(self.angles-self.enemy.angles);
				if(other.health)
					self.health=other.health;
				else
					self.health=FALSE;
				self.think=CB_BoltStick;
				thinktime self : 0;
			}
			else if(self.classname=="bolt")
				remove(self);
			else
				DarkExplosion();
        }
        else
        {
			self.movetype=MOVETYPE_NONE;
			if(self.classname=="bolt")
				self.think=SUB_Remove;
		    else
				self.think=DarkExplosion;
            thinktime self : 2;
        }
};

void ArrowFlyThink (void)
{
	if(self.lifetime<time&&self.mins=='0 0 0')
	{
		self.takedamage=DAMAGE_YES;
//        setsize(self,'-3 -3 -2','3 3 2');
	}
	if(self.model=="models/flaming.mdl")
	{
		self.frame+=1;
		if(self.frame>9)
			self.frame=0;
	}
	self.angles=vectoangles(self.velocity);
	self.think=ArrowFlyThink;
	thinktime self : 0.05;
}
void ArrowSound (void)
{
	//attn_static instead?
//	sound(self,CHAN_BODY,"assassin/arrowfly.wav",1,ATTN_NORM);
	self.think=ArrowFlyThink;
	thinktime self : 0;
}

void FlamingArrowThink (void)
{
float waterornot;
	waterornot=pointcontents(self.origin);
	if(waterornot==CONTENT_WATER||waterornot==CONTENT_SLIME)
	{
		sound (self, CHAN_WEAPON, "misc/fout.wav", 1, ATTN_NORM);
		DeathBubbles(1);
		setmodel(self,"models/NFarrow.mdl");
	}
	ArrowSound();
}

void ArrowThink (void)
{
vector dir;
	dir=normalize(self.velocity);
	traceline(self.origin,self.origin+dir*1000,FALSE,self);
	if(!trace_ent.takedamage)
		HomeThink();
    self.angles=vectoangles(self.velocity);
	if(self.classname=="bolt")
		self.think=ArrowSound;
	else
		self.think=FlamingArrowThink;
	thinktime self : 0;
}

void(float offset, float powered_up) FireCB_Bolt =
{
local entity missile;
        makevectors(self.v_angle);
        missile=spawn();
        missile.owner=self;
        missile.solid=SOLID_BBOX;
		missile.hull=HULL_POINT;
		missile.health=3;
		if(deathmatch)
			offset*=.333;
		if(powered_up)
		{
			missile.frags=TRUE;
			missile.thingtype=THINGTYPE_METAL;
	        missile.movetype=MOVETYPE_FLYMISSILE;
	        missile.classname="flaming arrow";
	        setmodel(missile,"models/flaming.mdl");
			missile.dmg=40;
			missile.drawflags(+)MLS_FIREFLICKER;
			missile.th_die=MultiExplode;
		}
		else
		{
			missile.thingtype=THINGTYPE_WOOD;
	        missile.movetype=MOVETYPE_FLYMISSILE;
	        missile.classname="bolt";
	        setmodel(missile,"models/arrow.mdl");
			missile.th_die=chunk_death;
        }
		missile.touch=CB_BoltHit;
		missile.speed=random(700,1200);
		missile.o_angle=missile.velocity=normalize(v_forward)*missile.speed+v_right*offset;
        missile.angles=vectoangles(missile.velocity);
		
		missile.ideal_yaw=TRUE;
		missile.turn_time = 0;
		missile.veer=0;

		missile.think= ArrowThink;
		thinktime missile : 0;
		missile.lifetime=time+0.2;

        setsize(missile,'0 0 0','0 0 0');
        setorigin(missile,self.origin+self.proj_ofs+v_forward*8);
};


void()crossbow_fire;
void crossbow_idle(void)
{
	self.th_weapon=crossbow_idle;
	self.weaponframe=$shoot19;
}

void crossbow_fire (void)
{
	self.wfs = advanceweaponframe($shoot1,$shoot18);
	self.th_weapon=crossbow_fire;
	if (self.weaponframe >= $shoot2 && self.weaponframe <= $shoot4)
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			if(self.weaponframe==$shoot2)
			{
				sound(self,CHAN_WEAPON,"assassin/firefblt.wav",1,ATTN_NORM);
				self.bluemana-=10;
				FireCB_Bolt(0,TRUE);
			}
			else if(self.weaponframe==$shoot3)
			{
				FireCB_Bolt(-100,TRUE);
				FireCB_Bolt(100,TRUE);
			}
			else if(self.weaponframe==$shoot4)
			{
				FireCB_Bolt(-200,TRUE);
				FireCB_Bolt(200,TRUE);
				self.attack_finished=time+0.3;
			}
		}
		else
		{
			if(self.weaponframe==$shoot2)
			{
				sound(self,CHAN_WEAPON,"assassin/firebolt.wav",1,ATTN_NORM);
				self.bluemana-=3;
				FireCB_Bolt(0,FALSE);
			}
			else if(self.weaponframe==$shoot3)
				FireCB_Bolt(-100,FALSE);
			else if(self.weaponframe==$shoot4)
			{
				FireCB_Bolt(100,FALSE);
				self.attack_finished=time+0.5;
			}
		}
	else if (self.wfs==WF_CYCLE_WRAPPED)
		crossbow_idle();
}

void crossbow_select (void)
{
//selection sound?
	self.wfs = advanceweaponframe($select15,$select1);
	self.weaponmodel = "models/crossbow.mdl";
	self.th_weapon=crossbow_select;
	if (self.weaponframe==$select1)
	{
		self.attack_finished = time - 1;
		crossbow_idle();
	}
}

void crossbow_deselect (void)
{
	self.wfs = advanceweaponframe($select1,$select15);
	self.th_weapon=crossbow_deselect;
	if (self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}


/*
 * $Log: not supported by cvs2svn $
 * 
 * 3     3/16/98 2:19a Mgummelt
 * 
 * 2     3/13/98 3:27a Mgummelt
 * Replaced all sounds that played a null.wav with stopSound commands
 * 
 * 68    10/28/97 1:00p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 66    9/11/97 12:02p Mgummelt
 * 
 * 65    9/10/97 11:39p Mgummelt
 * 
 * 64    8/26/97 7:38a Mgummelt
 * 
 * 63    8/21/97 5:04p Mgummelt
 * 
 * 62    8/18/97 12:20p Mgummelt
 * 
 * 61    8/15/97 2:55a Mgummelt
 * 
 * 60    8/08/97 6:21p Mgummelt
 * 
 * 59    8/07/97 10:30p Mgummelt
 * 
 * 58    8/06/97 10:19p Mgummelt
 * 
 * 57    8/04/97 8:03p Mgummelt
 * 
 * 56    7/30/97 3:32p Mgummelt
 * 
 * 55    7/28/97 8:27p Mgummelt
 * 
 * 54    7/26/97 8:38a Mgummelt
 * 
 * 53    7/25/97 11:45a Mgummelt
 * 
 * 52    7/24/97 4:06p Rlove
 * 
 * 51    7/24/97 3:53p Rlove
 * 
 * 50    7/24/97 3:26a Mgummelt
 * 
 * 49    7/21/97 4:03p Mgummelt
 * 
 * 48    7/21/97 4:02p Mgummelt
 * 
 * 47    7/17/97 6:53p Mgummelt
 * 
 * 46    7/17/97 11:44a Mgummelt
 * 
 * 45    7/16/97 8:12p Mgummelt
 * 
 * 44    7/16/97 6:06p Mgummelt
 * 
 * 43    7/15/97 8:30p Mgummelt
 * 
 * 42    7/14/97 9:30p Mgummelt
 * 
 * 41    7/10/97 7:21p Mgummelt
 * 
 * 40    7/09/97 6:31p Mgummelt
 * 
 * 39    7/02/97 8:46p Mgummelt
 * 
 * 38    7/01/97 3:30p Mgummelt
 * 
 * 37    7/01/97 2:21p Mgummelt
 * 
 * 36    6/30/97 5:38p Mgummelt
 * 
 * 35    6/18/97 6:20p Mgummelt
 * 
 * 34    6/18/97 4:00p Mgummelt
 * 
 * 33    6/16/97 4:00p Mgummelt
 * 
 * 32    6/05/97 8:16p Mgummelt
 * 
 * 31    6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 * 30    6/03/97 10:48p Mgummelt
 * 
 * 29    5/29/97 12:25p Mgummelt
 * 
 * 28    5/28/97 8:13p Mgummelt
 * 
 * 27    5/23/97 11:51p Mgummelt
 * 
 * 26    5/23/97 2:54p Mgummelt
 * 
 * 25    5/22/97 2:50a Mgummelt
 * 
 * 24    5/20/97 9:32p Mgummelt
 * 
 * 23    5/15/97 12:30a Mgummelt
 * 
 * 22    5/06/97 1:29p Mgummelt
 * 
 * 21    5/05/97 10:09p Mgummelt
 * 
 * 20    5/05/97 4:48p Mgummelt
 * 
 * 19    4/30/97 5:03p Mgummelt
 * 
 * 18    4/25/97 8:32p Mgummelt
 * 
 * 17    4/24/97 3:57p Mgummelt
 * 
 * 16    4/24/97 2:15p Mgummelt
 * 
 * 15    4/22/97 5:55p Mgummelt
 * 
 * 14    4/21/97 8:47p Mgummelt
 * 
 * 13    4/21/97 12:31p Mgummelt
 * 
 * 12    4/18/97 11:44a Rlove
 * changed advanceweaponframe to return frame state
 * 
 * 11    4/17/97 1:28p Rlove
 * added new built advanceweaponframe
 * 
 * 10    4/16/96 11:51p Mgummelt
 * 
 * 9     4/16/97 4:34p Mgummelt
 * 
 * 8     4/16/97 4:22p Mgummelt
 * 
 * 7     4/11/96 1:51p Mgummelt
 * 
 * 6     4/10/96 2:49p Mgummelt
 * 
 * 5     4/09/97 1:38p Mgummelt
 * 
 * 4     4/09/97 1:21p Mgummelt
 * 
 * 3     4/09/97 1:01p Mgummelt
 * 
 * 2     4/09/97 12:23p Mgummelt
 * 
 * 1     4/09/97 12:22p Mgummelt
 * 
 * 2     2/28/97 5:00p Rlove
 */
