/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/crossbow.hc,v 1.1.1.1 2004-11-29 11:25:50 sezero Exp $
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
      
void AssBoltExplosion ()
{
	T_RadiusDamage (self, self.owner, self.dmg, world);

	remove (self);
}

void XbowBoltTurn(entity bolt)
{
	vector dir;

	bolt.xbo_teleported = 1;
	bolt.takedamage=DAMAGE_NO;
//	bolt.solid = SOLID_NOT;
	bolt.xbo_startpos = bolt.origin;
	dir = vectoangles(bolt.velocity);
	updateeffect(bolt.xbo_effect_id, CE_HWXBOWSHOOT, bolt.boltnum*16+128, dir_x, dir_y, bolt.origin);
}

void() CB_BoltStick=
{
	if(self.wait<=time)
		self.think=AssBoltExplosion;
    thinktime self : 0;
};

void CB_RemoveEffect (void)
{
	endeffect(MSG_ALL,self.xbo_effect_id);
	remove(self);
}

void CB_FinishBoltEffect (void)
{
	entity finisher;
	finisher = spawn();
	finisher.think = CB_RemoveEffect;
	finisher.xbo_effect_id = self.xbo_effect_id;
	thinktime finisher : 5.0;
}

void CB_RemoveBoltFromList (void)
{
	entity curbolt;

	if (self.xbo_effect_id == -1)
	{
		return;
	}

	if ((self == self.firstbolt)&&(self.nextbolt == world))//i'm the last guy in the list--stop effect
	{
		CB_FinishBoltEffect();
		self.xbo_effect_id = -1;
		return;
	}

	if (self == self.firstbolt)//i'm the first in the list--let everyone know that the the new head of the list is the one after me
	{
		curbolt = self.nextbolt;
		while (curbolt != world)
		{
			curbolt.firstbolt = self.nextbolt;
			curbolt = curbolt.nextbolt;
		}
	}
	else
	{
		curbolt = self.firstbolt;
		while (curbolt.nextbolt != self)
		{
			curbolt = curbolt.nextbolt;
		}
		curbolt.nextbolt = self.nextbolt;
	}
	self.xbo_effect_id = -1;
}

void CB_HitEffect (vector v_forward)
{
	vector stickspot;
	float ttype;

	stickspot = v_forward * 8;

	//build the impact code byte now:

	//lowest 4 bits of byte indicate thingtype--can't use THINGTYPE_ consts because there are too many
	ttype = GetImpactType(other);

	if (other.takedamage)//high bit of the byte indicates whether hit object takes damage
	{
		ttype += 128;
	}

	ttype += self.boltnum * 16;//2nd, 3rd, and 4th higshest bits in byte indicate bolt number

	ttype += 1;//lowest bit set means that this bolt has hit

	//done building impact code byte****

	//now figure out how far i've travelled

	stickspot = self.origin-self.xbo_startpos;

	updateeffect(self.xbo_effect_id, CE_HWXBOWSHOOT, ttype, vlen(stickspot));

	CB_RemoveBoltFromList();
}

void() CB_BoltHit=
{
	if(other==self.owner||(other.owner==self.owner&&other.classname==self.classname))
	    return;

	if (self.xbo_teleported)
		return;

		float stick;

		v_forward=normalize(self.velocity);
		setsize(self,'0 0 0','0 0 0');
		self.takedamage=DAMAGE_NO;
        self.velocity='0 0 0';
        self.movetype=MOVETYPE_NOCLIP;
        self.solid=SOLID_NOT;
        self.touch=SUB_Null;
//		self.health=other.health;

		CB_HitEffect (v_forward);

		if(other.takedamage)
		{
			if(self.classname=="bolt")
				T_Damage(other,self,self.owner,15);
			else
				T_Damage(other,self,self.owner,3);
		}
		else
		{
			if(self.classname!="bolt")
				stick=TRUE;
			self.wait=time + self.fusetime;//random(1,3);
		}

//FIXME: only stick in if thingtype is wood or flesh,
//otherwise, no damage and bounce off!
        if(other.movetype||other.takedamage||stick||other.health)
        {
			if(stick)
			{
				self.enemy=other;
				self.think=CB_BoltStick;
				thinktime self : 0;
			}
			else if(self.classname=="bolt")
				remove(self);
			else
				AssBoltExplosion();
        }
        else
        {
			self.movetype=MOVETYPE_NONE;
			if(self.classname=="bolt")
				self.think=SUB_Remove;
		    else
				self.think=AssBoltExplosion;
            thinktime self : 2;
        }
};

void bolt_death (void)
{
	vector stickspot;
	float ttype;

	stickspot = v_forward * 8;

	//build the impact code byte now:

	//lowest 4 bits of byte indicate thingtype--can't use THINGTYPE_ consts because there are too many

	//high bit of the byte indicates whether hit object takes damage

	ttype = self.boltnum * 16;//2nd, 3rd, and 4th higshest bits in byte indicate bolt number

	ttype += 1;//lowest bit set means that this bolt has hit

	//done building impact code byte****

	//now figure out how far i've travelled

	stickspot = self.origin-self.xbo_startpos;

	updateeffect(self.xbo_effect_id, CE_HWXBOWSHOOT, ttype, vlen(stickspot));

	CB_RemoveBoltFromList();
	remove(self);
}

void fbolt_death (void)
{
	vector stickspot;
	float ttype;

	self.takedamage=DAMAGE_NO;
	self.th_die = SUB_Null;

	stickspot = v_forward * 8;

	//build the impact code byte now:

	//lowest 4 bits of byte indicate thingtype--can't use THINGTYPE_ consts because there are too many

	//high bit of the byte indicates whether hit object takes damage

	ttype = self.boltnum * 16;//2nd, 3rd, and 4th higshest bits in byte indicate bolt number

	ttype += 1;//lowest bit set means that this bolt has hit

	//done building impact code byte****

	//now figure out how far i've travelled

	stickspot = self.origin-self.xbo_startpos;

	updateeffect(self.xbo_effect_id, CE_HWXBOWSHOOT, ttype, vlen(stickspot));

	CB_RemoveBoltFromList();
	AssBoltExplosion();
}

void ArrowFlyThink (void)
{
	if (self.xbo_teleported >0)
	{
		self.xbo_teleported = self.xbo_teleported - 1;
//		self.solid = SOLID_BBOX;
//		self.takedamage=DAMAGE_YES;
	}
	else
	{
		self.xbo_teleported = FALSE;
	}
	self.velocity = normalize(self.velocity)*self.speed;
	if(self.lifetime<time&&self.mins=='0 0 0')
	{
		3;//or 4?...
//		self.takedamage=DAMAGE_YES;
//        setsize(self,'-3 -3 -2','3 3 2');
	}
	if(self.model=="models/flaming.mdl")
	{
		self.frame+=1;
		if(self.frame>9)
			self.frame=0;
	}
//	self.angles=vectoangles(self.velocity);
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
	ArrowSound();
}

void ArrowThinkEnt (entity who)//call me right away now
{
	vector dir, oldvel;
	oldvel = who.velocity;
	dir=normalize(who.velocity);
	traceline(who.origin,who.origin+dir*1000,FALSE,who);
	if(!trace_ent.takedamage)
		HomeThinkEnt(who);

    who.angles=vectoangles(who.velocity);

	if(who.classname=="bolt")
	{
		//only send new course if it's changed
		if (oldvel_x != who.velocity_x || oldvel_y != who.velocity_y || oldvel_z != who.velocity_z)
		{
			dir = vectoangles(who.velocity);
			updateeffect(who.xbo_effect_id, CE_HWXBOWSHOOT, who.boltnum*16, dir_x, dir_y);
		}
		who.think=ArrowSound;
	}
	else
	{
		//only send new course if it's changed
		if (oldvel_x != who.velocity_x || oldvel_y != who.velocity_y || oldvel_z != who.velocity_z)
		{
			dir = vectoangles(who.velocity);
			updateeffect(who.xbo_effect_id, CE_HWXBOWSHOOT, who.boltnum*16, dir_x, dir_y);
		}
		who.think=FlamingArrowThink;
	}
}

entity (float offset, float powered_up, entity prevbolt, float boltnumber, float effectnum) FireCB_Bolt =
{
		local entity missile;
        makevectors(self.v_angle);
        missile=spawn();
		missile.xbo_teleported = FALSE;
		missile.xbo_effect_id = effectnum;
		missile.takedamage=DAMAGE_NO;
//		bprint(PRINT_MEDIUM,ftos(missile.xbo_effect_id));
//		bprint(PRINT_MEDIUM," effect has new bolt\n");

        missile.owner=self;
        missile.solid=SOLID_BBOX;
		missile.hull=HULL_POINT;
		missile.health=9999;//geesh, are we still getting stack overflows?!?!?! bolts shouldn't be taking damage, but if they still are for whatever reason, give them lotsa health.

		 // make sll of bolts in this effect
		if (prevbolt == world)
		{
			missile.firstbolt = missile;
		}
		else
		{
			prevbolt.nextbolt = missile;
			missile.firstbolt = prevbolt.firstbolt;
		}
		missile.nextbolt = world;

		missile.boltnum = boltnumber;


//		if(deathmatch)//i'm not finding a global like this available on client, so always decrease offset
			offset*=.333;
		if(powered_up)
		{
			missile.frags=TRUE;
			missile.thingtype=THINGTYPE_METAL;
	        missile.movetype=MOVETYPE_FLYMISSILE;
	        missile.classname="flaming arrow";
//	        setmodel(missile,"models/flaming.mdl");
			missile.dmg=40;
			missile.drawflags(+)MLS_FIREFLICKER;
//			missile.th_die=fbolt_death;
		}
		else
		{
			missile.thingtype=THINGTYPE_WOOD;
	        missile.movetype=MOVETYPE_FLYMISSILE;
//bounce testing
//	        missile.movetype=MOVETYPE_BOUNCEMISSILE;
	        missile.classname="bolt";
//	        setmodel(missile,"models/arrow.mdl");
//			missile.th_die=bolt_death;
        }
		missile.touch=CB_BoltHit;

//		missile.speed=random(700,1200);
		missile.speed = 800.0 + seedrand()*500.0;
		missile.fusetime = 1.0 + seedrand()*2.0;

		missile.o_angle=missile.velocity=normalize(v_forward)*missile.speed+v_right*offset;
        missile.angles=vectoangles(missile.velocity);
		
		missile.ideal_yaw=TRUE;
		missile.turn_time = 0;
		missile.veer=0;

		missile.lifetime=time+0.2;

        setsize(missile,'0 0 0','0 0 0');
        setorigin(missile,self.origin+self.proj_ofs+v_forward*8+v_right*offset*0.05);

		missile.xbo_startpos = missile.origin;//save start pos so i can send the total distance i travelled when i finish

		ArrowThinkEnt(missile);
		thinktime missile : 0;

		return (missile);
};


void()crossbow_fire;
void crossbow_idle(void)
{
	self.th_weapon=crossbow_idle;
	self.weaponframe=$shoot19;
}

void crossbow_fire (void)
{
	entity curmissile;
	float bolteffect,randseed;
    
	makevectors(self.v_angle);
	self.wfs = advanceweaponframe($shoot1,$shoot18);
	self.th_weapon=crossbow_fire;
	if (self.weaponframe == $shoot2)
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			randseed = random(255);
			setseed(randseed);
			bolteffect = starteffect(CE_HWXBOWSHOOT, self.origin+self.proj_ofs+v_forward*8, self.v_angle, 5, randseed);
			curmissile=FireCB_Bolt(-200.0,TRUE,world,0,bolteffect);
			curmissile=FireCB_Bolt(-100.0,TRUE,curmissile,1,bolteffect);
			curmissile=FireCB_Bolt(0,TRUE,curmissile,2,bolteffect);
			curmissile=FireCB_Bolt(100.0,TRUE,curmissile,3,bolteffect);
			curmissile=FireCB_Bolt(200.0,TRUE,curmissile,4,bolteffect);
			self.attack_finished=time+0.3;
			self.bluemana-=10;
		}
		else
		{
			randseed = random(255);
			setseed(randseed);
			bolteffect = starteffect(CE_HWXBOWSHOOT, self.origin+self.proj_ofs+v_forward*8, self.v_angle, 3, randseed);
			curmissile=FireCB_Bolt(-100.0,FALSE,world,0,bolteffect);
			curmissile=FireCB_Bolt(0,FALSE,curmissile,1,bolteffect);
			curmissile=FireCB_Bolt(100.0,FALSE,curmissile,2,bolteffect);
			self.attack_finished=time+0.5;
			self.bluemana-=3;
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
 * Revision 1.1.1.1  2001/11/09 17:05:01  theoddone33
 * Inital import
 *
 * 
 * 27    4/06/98 12:48p Ssengele
 * put in safety net to prevent endless loops in tripmine damage, ditched
 * a couple of unreferenced vars
 * 
 * 26    4/02/98 2:28p Ssengele
 * one more thing to make it more impossible to overflow the damn stack
 * 
 * 25    4/02/98 1:56p Ssengele
 * made xbow explosion chain stack overflow crash even More impossible.
 * 
 * 24    4/01/98 3:03p Ssengele
 * faster xbolts, initial optimizing of forcecube.
 * 
 * 23    3/31/98 12:49p Ssengele
 * took out unnoticeable shine to underwater xbolts that made for a good
 * chunk of traffic, re-optimized king-o-hill messages, commented damage
 * prints.
 * 
 * 22    3/25/98 3:32p Ssengele
 * nudged xbow damage up a bit
 * 
 * 21    3/24/98 6:52p Ssengele
 * can't damage xbolts no more, to fix stack overflow.
 * 
 * 20    3/24/98 6:06p Ssengele
 * exploding grenades, xbolts, tripmines can't go into infinite loops
 * anymore; sheep impulse has 10 sec delay; scarab staff using union in
 * non-dangerous way (tho that frankly dint seem to cause any probs)
 * 
 * 19    3/11/98 1:13p Ssengele
 * teleporting xbow bolts works well now.
 * 
 * 18    3/05/98 12:19p Ssengele
 * sheepinator spead corrected
 * 
 * 17    3/05/98 12:02a Ssengele
 * xbow bolt death that won't screw the linked list of bolts
 * 
 * 16    3/03/98 8:09p Ssengele
 * reflection code for xbow bolts and sheepinator; also, a little variance
 * for where bolts start out, so autoaim doesn't make them clump up quite
 * as much (variance is consistent between sv & cl)
 * 
 * 15    3/02/98 8:05p Ssengele
 * moved thingtype check into a subroutine, since it turned out to be
 * something i needed in a few different places.
 * 
 * 14    2/27/98 7:12p Ssengele
 * grrrrrrrr...xbow more optimized than ever (plus chunks, or did i add
 * those yesterday?), still spiking not-so-pretty (nudges first red line)
 * occasionally in the worst case scenario.
 * 
 * 13    2/26/98 6:55p Ssengele
 * arrows fall into bits
 * 
 * 12    2/26/98 4:55p Ssengele
 * cut out fat that xbow wasn't using--made bolt course correction instant
 * 
 * 11    2/26/98 2:03p Ssengele
 * further optimizing of xbow--at impact, i send distance it travelled
 * rather than end pos; pack alot of little pieces of info into a byte
 * 
 * 10    2/24/98 2:26p Ssengele
 * xbow impact in correct position. !whew!
 * 
 * 9     2/24/98 1:37p Ssengele
 * made xbow impact reliable
 * 
 * 8     2/24/98 11:07a Ssengele
 * debugging
 * 
 * 7     2/23/98 8:04p Ssengele
 * erg--checking in so's not to hog these files TOO much.  cleffect that
 * handles multiple xbow bolts is IN, but impact is neither consistant nor
 * in the right spot.
 * 
 * 6     2/19/98 12:45p Ssengele
 * put in client effect for powered-up xbow; spikes even bigger than
 * normal xbow,tho, so i'd like to chunk all of the arrows into one big
 * client effect, rather than give each of them their own.  this i'll do
 * later.
 * 
 * 5     2/18/98 8:25p Ssengele
 * xbow client effect (for normal mode) lookin good--but it trades off
 * significantly less traffic when arrows are in air for a slightly bigger
 * spike when they're fired.  will figure out how to minimize that spike
 * yet.
 * 
 * 4     2/18/98 5:37p Ssengele
 * start at xbow bolt client effect
 * 
 * 3     2/18/98 3:47p Ssengele
 * extra info for cl_netinfo value 2: will show what ent delta info is
 * sent
 * 
 * 2     2/17/98 9:05p Ssengele
 * first pass at speeding up xbow--consistently under red line now, but
 * there's some kind of overhead while the bolts are in the air that i'd
 * like to track down.
 * also, white puff not back in yet, and particles need to be added
 * 
 * 1     2/04/98 1:59p Rjohnson
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
