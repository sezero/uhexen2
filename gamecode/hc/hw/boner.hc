/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/boner.hc,v 1.1.1.1 2004-11-29 11:25:09 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\spllbook\spllbook.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\spllbook
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame fire1        fire2        fire3        fire4        fire5        
$frame fire6        fire7        fire8        fire9        fire10       
$frame fire11       fire12       

//
$frame go2mag01     go2mag02     go2mag03     go2mag04     go2mag05     
$frame go2mag06     go2mag07     go2mag08     go2mag09     go2mag10     
$frame go2mag11     go2mag12     go2mag13

//
$frame go2shd01     go2shd02     
$frame go2shd03     go2shd04     go2shd05     go2shd06     go2shd07      
$frame go2shd08     go2shd09     go2shd10     go2shd11     go2shd12      
$frame go2shd13     go2shd14      

//
$frame idle1        idle2        idle3        idle4        idle5        
$frame idle6        idle7        idle8        idle9        idle10       
$frame idle11       idle12       idle13       idle14       idle15       
$frame idle16       idle17       idle18       idle19       idle20       
$frame idle21       idle22       

//
$frame mfire1       mfire2       mfire3       mfire4       mfire5       
$frame mfire6       mfire7       mfire8       

//
$frame midle01      midle02      midle03      midle04      midle05      
$frame midle06      midle07      midle08      midle09      midle10      
$frame midle11      midle12      midle13      midle14      midle15      
$frame midle16      midle17      midle18      midle19      midle20      
$frame midle21      midle22      

//
$frame mselect01    mselect02    mselect03    mselect04    mselect05    
$frame mselect06    mselect07    mselect08    mselect09    mselect10    
$frame mselect11    mselect12    mselect13    mselect14    mselect15    
$frame mselect16    mselect17    mselect18    mselect19    mselect20    

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      


/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

float RicochetCount;

void(vector org)smolder;
void(vector org, float damage) Ricochet =
{
//float r;

	RicochetCount+=1;
	if(RicochetCount > 7)
	{
		RicochetCount = 0;
		WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
		WriteByte (MSG_MULTICAST, TE_HWBONERIC);
		WriteCoord (MSG_MULTICAST, org_x);
		WriteCoord (MSG_MULTICAST, org_y);
		WriteCoord (MSG_MULTICAST, org_z);
		if(damage > 100)
			damage = 100;
		WriteByte (MSG_MULTICAST, damage * 2);
		multicast(self.origin,MULTICAST_PHS_R);

/*		particle4(org,3,random(368,384),PARTICLETYPE_GRAV,damage*2);
		r = random(100);
		if (r > 95)
			sound (targ,CHAN_AUTO,"weapons/ric1.wav",1,ATTN_NORM);
		else if (r > 91)
			sound (targ,CHAN_AUTO,"weapons/ric2.wav",1,ATTN_NORM);
		else if (r > 87)
			sound (targ,CHAN_AUTO,"weapons/ric3.wav",1,ATTN_NORM);
*/
	}


};

entity  multi_ent;
float   multi_damage;

void() ClearMultDamg =
{
	multi_ent = world;
	multi_damage = 0;
};

void() ApplyMultDamg =
{
float kicker, inertia;
	if (!multi_ent)
		return;

entity loser,winner;
	winner=self;
    loser=multi_ent;
    kicker = multi_damage * 7 - vlen(winner.origin - loser.origin);
	if(kicker>0)
	{	
        if(loser.flags&FL_ONGROUND)
		{	
			loser.flags(-)FL_ONGROUND;
			loser.velocity_z = loser.velocity_z + 150;
		}
        if (loser.mass<=10)
			inertia = 1;
                  else inertia = loser.mass/10;
            if(loser==self)
                    loser.velocity = loser.velocity - (normalize(loser.v_angle) * (kicker / inertia));
            else loser.velocity = loser.velocity + (normalize(winner.v_angle) * (kicker / inertia));
	   SpawnPuff (loser.origin, loser.velocity*0.1, multi_damage*0.25,loser);
       T_Damage (loser, winner, winner, multi_damage);
	}
};

void(entity hit, float damage) AddMultDamg =
{
	if (!hit)
		return;
	
	if (hit != multi_ent)
	{
		ApplyMultDamg ();
		multi_damage = damage;
		multi_ent = hit;
	}
	else
		multi_damage = multi_damage + damage;
};

void(float damage, vector dir) TraceHit =
{
	local   vector  vel, org;
	
	vel = (normalize(dir + v_factorrange('-1 -1 0','1 1 0')) + 2 * trace_plane_normal) * 200;
	org = trace_endpos - dir*4;

	if (trace_ent.takedamage)
	{
		AddMultDamg (trace_ent, damage);
	}
	else
		Ricochet(org,damage);
};

void(float shotcount, vector dir, vector spread) InstantDamage =
{
vector direction;
vector  src;
	
	makevectors(self.v_angle);

	src = self.origin + self.proj_ofs+'0 0 6'+v_forward*10;
	ClearMultDamg ();
	while (shotcount > 0)
	{
		direction = dir + random(-1,1)*spread_x*v_right;
		direction += random(-1,1)*spread_y*v_up;

		traceline (src, src + direction*2048, FALSE, self);
		if (trace_fraction != 1.0)
			TraceHit (4, direction);
		shotcount = shotcount - 1;
	}
	ApplyMultDamg ();
};

void bone_shard_touch ()
{
	if(other==self.owner)
		return;
//string hitsound;

	if(other.takedamage)
	{
//		hitsound="necro/bonenhit.wav";
		T_Damage(other, self,self.owner,self.dmg);
	}
	else
	{
//		hitsound="necro/bonenwal.wav";
		T_RadiusDamage(self,self.owner,self.dmg*2,self.owner);
	}
//	starteffect(CE_WHITE_SMOKE, self.origin,'0 0 0', HX_FRAME_TIME);
//	sound(self,CHAN_WEAPON,hitsound,1,ATTN_NORM);
//	particle4(self.origin,3,random(368,384),PARTICLETYPE_GRAV,self.dmg/2);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_HWBONEPOWER2);
	if(other.takedamage)
	{
		WriteByte(MSG_MULTICAST, 1);
	}
	else
	{
		WriteByte(MSG_MULTICAST, 0);
	}
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	multicast(self.origin,MULTICAST_PHS_R);

	
	remove(self);	
}

void bone_shard_touch2 (void)
{
	endeffect(MSG_ALL,self.wrq_effect_id);
	bone_shard_touch();
}

void bone_removeshrapnel (void)
{
	remove(self);	
}

// shrapnel is too much net hit, will be faked by client only shards
// although the server will have invisible shards for damage determination
void fire_bone_shrapnel ()
{
vector shard_vel;
	newmis=spawn();
	newmis.classname = "bone_fake_shrapnel"; //so reflection won't send turneffect
	newmis.owner=self.owner;
	newmis.movetype=MOVETYPE_BOUNCE;
	newmis.solid=SOLID_PHASE;
	newmis.effects (+) EF_NODRAW;
	newmis.touch=bone_shard_touch;
	newmis.dmg=15;
	newmis.think=bone_removeshrapnel;
	thinktime newmis : 3;

	newmis.speed=777;
	trace_fraction=0;
	trace_ent=world;
	while(trace_fraction!=1&&!trace_ent.takedamage)
	{
		shard_vel=randomv('1 1 1','-1 -1 -1');
		traceline(self.origin,self.origin+shard_vel*36,TRUE,self);
	}
	newmis.velocity=shard_vel*newmis.speed;
	newmis.avelocity=randomv('777 777 777','-777 -777 -777');

//	setmodel(newmis,"models/boneshrd.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+shard_vel*8);

//	newmis.wrq_effect_id = starteffect(CE_BONESHRAPNEL, newmis.origin, newmis.velocity,
//		newmis.angles,newmis.avelocity);

}

void bone_shatter ()
{
float shard_count;
	endeffect(MSG_ALL,self.wrq_effect_id);

	shard_count=20;
	while(shard_count)
	{
		fire_bone_shrapnel();
		shard_count-=1;
	}
}

void bone_power_touch ()
{
//vector randomvec;

//	sound(self,CHAN_WEAPON,"necro/bonephit.wav",1,ATTN_NORM);
	if(other.takedamage)
	{
		T_Damage(other, self,self.owner,self.dmg*2);
	}

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_HWBONEPOWER);
	if(other.takedamage)
	{
		WriteByte (MSG_MULTICAST, 4);  //number of ghosts
	}
	else
	{
		WriteByte(MSG_MULTICAST, 0);	//no ghosts
	}

	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteCoord (MSG_MULTICAST, self.movedir_x);
	WriteCoord (MSG_MULTICAST, self.movedir_y);
	WriteCoord (MSG_MULTICAST, self.movedir_z);
	multicast(self.origin,MULTICAST_PHS_R);


	self.solid=SOLID_NOT;
	self.flags2(+)FL2_ADJUST_MON_DAM;
	T_RadiusDamage(self,self.owner,self.dmg,other);

	bone_shatter();
/*	dprint("Doing final effect\n");
	starteffect(CE_BONE_EXPLOSION, self.origin-self.movedir*6,'0 0 0', HX_FRAME_TIME);
	particle4(self.origin,50,random(368,384),PARTICLETYPE_GRAV,10);
	dprint("removing\n");
*/	remove(self);	
}

void bone_fire(float powered_up, vector ofs)
{
//SOUND
vector org;

	makevectors(self.v_angle);
	newmis=spawn();
	newmis.owner=self;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.speed=1000;

	org=self.origin+self.proj_ofs+v_forward*8+v_right*(ofs_y+12)+v_up*ofs_z;
	setorigin(newmis,org);

	if(powered_up)
	{
		newmis.velocity=v_forward*newmis.speed;
		self.punchangle_x=-2;
//		sound(self,CHAN_WEAPON,"necro/bonefpow.wav",1,ATTN_NORM);
		self.attack_finished=time + 1.3;
		newmis.classname = "bone_powered";
		newmis.dmg=80;//was 200
		newmis.frags=TRUE;
//		newmis.takedamage=DAMAGE_YES;
//		newmis.health=3;
//		newmis.th_die=bone_shatter;
		newmis.touch=bone_power_touch;
//		newmis.avelocity=randomv('777 777 777','-777 -777 -777');
//		setmodel(newmis,"models/bonelump.mdl");
		setsize(newmis,'0 0 0','0 0 0');
		self.greenmana-=20;
		newmis.effects (+) EF_NODRAW;
		org = randomv('777 777 777','-777 -777 -777');
		newmis.wrq_effect_id = starteffect(CE_HWBONEBALL, newmis.origin, newmis.velocity,
			newmis.angles,org);
	}
	else
	{
		newmis.classname = "bone_normal";
		newmis.speed+=random(500);
		newmis.velocity=v_forward*newmis.speed;
		newmis.dmg=10;
		newmis.touch=bone_shard_touch2;
		newmis.effects (+) EF_NODRAW;
//		setmodel(newmis,"models/boneshot.mdl");
		setsize(newmis,'0 0 0','0 0 0');
//		newmis.velocity+=v_right*ofs_y*10+v_up*ofs_z*10;

//		newmis.angles=vectoangles(newmis.velocity);
//		newmis.avelocity_z=random(777,-777);

		newmis.wrq_effect_id = starteffect(CE_BONESHARD, newmis.origin, newmis.velocity);
	}
}

void  bone_normal()
{
vector dir;
//sound
//	sound(self,CHAN_WEAPON,"necro/bonefnrm.wav",1,ATTN_NORM);
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	dir=normalize(v_forward);
	InstantDamage(4,dir,'0.1 0.1 0.1');
//	InstantDamage(12,dir,'0.1 0.1 0.1');
	self.greenmana-=0.5;
	self.attack_finished=time+0.3;
}

void bone_fire_once()
{
vector ofs;
	ofs_z=random(-5,5);
	ofs_x=random(-5,5);
	ofs_y=random(-5,5);
	bone_fire(FALSE,ofs);
}


/*======================
ACTION
select
deselect
ready loop
relax loop
fire once
fire loop
ready to relax(after short delay)
relax to ready(Fire delay?  or automatic if see someone?)
=======================*/


void()boneshard_ready;
void() Nec_Bon_Attack;


void boneshard_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire12);
	if(self.button0&&self.weaponframe>$fire3 &&!self.artifact_active&ART_TOMEOFPOWER)
		self.weaponframe=$fire3;
	self.th_weapon=boneshard_fire;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED||self.greenmana<1||(self.greenmana<10&&self.artifact_active&ART_TOMEOFPOWER))
		boneshard_ready();
	else if(self.weaponframe==$fire3)
		if(self.artifact_active&ART_TOMEOFPOWER)
			bone_fire(TRUE,'0 0 0');
		else
			bone_normal();

	if(random()<0.5&&!self.artifact_active&ART_TOMEOFPOWER&&self.weaponframe<=$fire6)
		bone_fire_once();
}

void() Nec_Bon_Attack =
{
	boneshard_fire();

	thinktime self : 0;
};

void boneshard_jellyfingers ()
{
	self.wfs = advanceweaponframe($idle1,$idle22);
	self.th_weapon=boneshard_jellyfingers;
	if(self.wfs==WF_CYCLE_WRAPPED)
		boneshard_ready();
}

void boneshard_ready (void)
{
	self.weaponframe=$idle1;
	if(random()<0.1&&random()<0.3&&random()<0.5)
		self.th_weapon=boneshard_jellyfingers;
	else
		self.th_weapon=boneshard_ready;
}

void boneshard_select (void)
{
	self.wfs = advanceweaponframe($select7,$select1);
	self.weaponmodel = "models/spllbook.mdl";
	self.th_weapon=boneshard_select;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		boneshard_ready();
	}
}

void boneshard_deselect (void)
{
	self.wfs = advanceweaponframe($select1,$select7);
	self.th_weapon=boneshard_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}


void boneshard_select_from_mmis (void)
{
	self.wfs = advanceweaponframe($go2shd01,$go2shd14);
	self.weaponmodel = "models/spllbook.mdl";
	self.th_weapon=boneshard_select_from_mmis;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		boneshard_ready();
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:00  theoddone33
 * Inital import
 *
 * 
 * 17    4/08/98 4:43p Rmidthun
 * don't use turn effect for bone shrapnel, since it is not a client
 * effect
 * 
 * 16    4/02/98 11:40a Rmidthun
 * cleaned up the code, removed some old dead code
 * 
 * 15    4/01/98 1:27p Rmidthun
 * reduced damage of these weapons to bring them more in line with the
 * other classes
 * 
 * 14    3/27/98 3:09p Rmidthun
 * reduced bone shard net traffic a little by using temp ent instead of
 * particle
 * 
 * 13    3/26/98 10:57a Rmidthun
 * slowed rate of fire for unpowered, since much of the damage is instant,
 * this won't reduce the effectiveness very much.  The weapon was a little
 * overpowered anyway.
 * 
 * 12    3/25/98 1:02p Rmidthun
 * remove the bone shard client effect
 * 
 * 11    3/05/98 10:14a Rmidthun
 * made unpowered bone shards more powerful, they were weaker than the
 * missiles before.  Also made then use less mana, since it burns through
 * so quickly.
 * 
 * 10    3/03/98 3:01p Rmidthun
 * client effect missiles needed an update function to be called when
 * other things change their velocity, this was put into invntory.hc.
 * 
 * 9     2/23/98 2:23p Rmidthun
 * changed broadcasts to multicast, this should reduce net traffic for
 * larger games
 * 
 * 8     2/23/98 11:29a Rmidthun
 * reduced net traffic for unpowered bone shard by reducing the number of
 * particle and smoke effects produced by the InstantDamage.  Now, smoke
 * is only spawned at damage time (instead of each piece of damage) and
 * particles are sent one out of four times, since 4 traces are spawned
 * for each bone.  The number of particles was multiplied by 4 to
 * compensate.
 * 
 * 7     2/20/98 4:45p Rmidthun
 * moved rest of sounds to client side
 * 
 * 6     2/20/98 1:33p Rmidthun
 * don't send self to bonepower2 or ravendie tempents
 * 
 * 5     2/20/98 10:08a Rmidthun
 * bone shards don't need to send angle or angular vel
 * 
 * 4     2/19/98 3:57p Rmidthun
 * moved "boneball" to client effects, this allows the particle stream to
 * be added back.  Also fixed a bug where damage was called in the middle
 * of a tempent message.  Bad idea...
 * 
 * 3     2/18/98 2:14p Rmidthun
 * moved sound into client effect
 * 
 * 2     2/17/98 10:04a Rmidthun
 * moved powered up bone shards to client effects
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 51    10/17/97 3:53p Mgummelt
 * 
 * 50    10/16/97 12:02p Mgummelt
 * 
 * 49    10/13/97 10:28a Rlove
 * Made the bone shard powerup more network friendly.
 * 
 * 48    10/10/97 9:13a Rlove
 * Moved bone shard shrapnel to client side effect
 * 
 * 47    10/08/97 11:14a Rlove
 * 
 * 46    9/17/97 1:27p Rlove
 * 
 * 45    9/11/97 12:02p Mgummelt
 * 
 * 44    9/11/97 9:05a Mgummelt
 * 
 * 43    9/07/97 9:42a Mgummelt
 * 
 * 42    8/31/97 8:52a Mgummelt
 * 
 * 41    8/30/97 6:58p Mgummelt
 * 
 * 40    8/29/97 4:17p Mgummelt
 * Long night
 * 
 * 39    8/26/97 7:38a Mgummelt
 * 
 * 38    8/24/97 4:34p Mgummelt
 * 
 * 37    8/19/97 12:57p Mgummelt
 * 
 * 36    8/13/97 5:46p Mgummelt
 * 
 * 35    8/08/97 6:21p Mgummelt
 * 
 * 34    8/07/97 10:30p Mgummelt
 * 
 * 33    8/06/97 10:19p Mgummelt
 * 
 * 32    8/04/97 8:03p Mgummelt
 * 
 * 31    8/01/97 9:51p Mgummelt
 * 
 * 30    8/01/97 9:49p Mgummelt
 * 
 * 29    7/31/97 10:55p Mgummelt
 * 
 * 28    7/30/97 8:26p Mgummelt
 * 
 * 27    7/30/97 3:32p Mgummelt
 * 
 * 26    7/29/97 3:46p Mgummelt
 * 
 * 25    7/28/97 8:41p Mgummelt
 * 
 * 24    7/28/97 8:27p Mgummelt
 * 
 * 23    7/28/97 7:50p Mgummelt
 * 
 * 22    7/24/97 11:38a Mgummelt
 * 
 * 21    7/21/97 4:03p Mgummelt
 * 
 * 20    7/21/97 4:02p Mgummelt
 * 
 * 19    7/15/97 8:30p Mgummelt
 * 
 * 18    7/14/97 9:30p Mgummelt
 * 
 * 17    7/10/97 7:21p Mgummelt
 * 
 * 16    7/09/97 6:31p Mgummelt
 * 
 * 15    7/01/97 3:30p Mgummelt
 * 
 * 14    7/01/97 2:21p Mgummelt
 * 
 * 13    6/30/97 5:38p Mgummelt
 * 
 * 12    6/23/97 4:50p Mgummelt
 * 
 * 11    6/19/97 3:33p Rjohnson
 * removed crandom()
 * 
 * 10    6/18/97 5:59p Mgummelt
 * 
 * 9     6/18/97 4:19p Mgummelt
 * 
 * 8     6/18/97 4:00p Mgummelt
 * 
 * 7     6/12/97 8:54p Mgummelt
 * 
 * 6     6/09/97 10:21p Mgummelt
 * 
 * 5     6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 */
