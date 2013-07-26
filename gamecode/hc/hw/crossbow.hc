/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/crossbow.hc,v 1.2 2007-02-07 16:57:52 sezero Exp $
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
//	self.health=other.health;

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
//		setsize(self,'-3 -3 -2','3 3 2');
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
//	bprint(PRINT_MEDIUM,ftos(missile.xbo_effect_id));
//	bprint(PRINT_MEDIUM," effect has new bolt\n");

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

//	if(deathmatch)//i'm not finding a global like this available on client, so always decrease offset
		offset*=.333;
	if(powered_up)
	{
		missile.frags=TRUE;
		missile.thingtype=THINGTYPE_METAL;
		missile.movetype=MOVETYPE_FLYMISSILE;
		missile.classname="flaming arrow";
//		setmodel(missile,"models/flaming.mdl");
		missile.dmg=40;
		missile.drawflags(+)MLS_FIREFLICKER;
//		missile.th_die=fbolt_death;
	}
	else
	{
		missile.thingtype=THINGTYPE_WOOD;
		missile.movetype=MOVETYPE_FLYMISSILE;
//bounce testing
//		missile.movetype=MOVETYPE_BOUNCEMISSILE;
		missile.classname="bolt";
//		setmodel(missile,"models/arrow.mdl");
//		missile.th_die=bolt_death;
	}
	missile.touch=CB_BoltHit;
//	missile.speed=random(700,1200);
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
	{
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

