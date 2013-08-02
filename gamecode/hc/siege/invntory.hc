/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/invntory.hc,v 1.4 2007-02-07 17:01:18 sezero Exp $
 */

void teleport_touch (void);
void player_fly(void);
void player_stopfly(void);
//void XbowBoltTurn(entity bolt);
void PolyTurn(entity bolt);
//void DrillaTurn(entity bolt);

void Use_RingFlight()
{
	self.rings(+)RING_FLIGHT;
	self.ring_flight = 20;
	self.ring_flight_time = time + 1;
	player_fly();
	self.rings_low (-) RING_FLIGHT;
	self.cnt_flight -= 1;
}

void()monster_imp_lord;
void BecomeImp ()
{
float move_cnt;
	if(other.solid!=SOLID_BSP)
		return;

	self.solid=SOLID_NOT;
	setorigin(self,self.origin+'0 0 50');
	setsize(self,'-40 -40 -50','40 40 50');
	self.hull=HULL_GOLEM;
	newmis=spawn();
	setorigin(newmis,self.origin);
	tracearea(self.origin,self.origin+'0 0 1',self.mins,self.maxs,FALSE,newmis);
	while((trace_fraction<1||trace_allsolid)&&move_cnt<36)
	{
		setorigin(newmis,newmis.origin+'0 0 1');
		tracearea(newmis.origin,newmis.origin+'0 0 1',self.mins,self.maxs,FALSE,newmis);
		move_cnt+=1;
	}
	if(trace_fraction==1&&!trace_allsolid)
	{
		self.touch=SUB_Null;

		newmis.flags2(+)FL_SUMMONED;
		newmis.controller=self.owner;
		newmis.siege_team=self.owner.siege_team;
		newmis.skin=self.owner.skin;
		newmis.classname="monster_imp_lord";
		if(self.owner.enemy!=world&&self.owner.enemy.flags2&FL_ALIVE&&visible2ent(self.owner.enemy,self))
		{
			newmis.enemy=newmis.goalentity=self.owner.enemy;
			newmis.monster_awake=TRUE;
		}
		else
		{
			newmis.enemy=newmis.goalentity=self.owner;
			newmis.monster_awake=TRUE;
		}
		self.owner.imp_count+=1;
		newmis.imp_count=self.owner.imp_count;
		newmis.think=monster_imp_lord;
		thinktime newmis : 0;

		sound (newmis, CHAN_AUTO, "weapons/expsmall.wav", 1, ATTN_NORM);
		sound (newmis, CHAN_VOICE, "imp/upbig.wav", 1, ATTN_NORM);
		setorigin(self,self.origin-'0 0 50');
		BecomeExplosion(CE_FLOOR_EXPLOSION);
		newmis.hull = HULL_HYDRA;
		newmis.solid = SOLID_SLIDEBOX;
	}
	else
	{
		CreateWhiteFlash(self.origin);
		self.think=SUB_Remove;
		thinktime self : 0;
		self.owner.greenmana+=60;
	}
}

void Use_Summoner ()
{
	makevectors(self.v_angle);
//sound
entity missile;
    missile=spawn();
    missile.owner=self;
    missile.classname="summon";
    missile.movetype=MOVETYPE_BOUNCE;
    missile.solid=SOLID_BBOX;
	missile.touch=BecomeImp;
	missile.effects=EF_DIMLIGHT;
	missile.drawflags=MLS_POWERMODE;

	missile.movedir=normalize(v_forward);
	missile.velocity=normalize(v_forward)*300 +v_up*100;
	missile.angles=vectoangles(missile.velocity);
	missile.avelocity_x=60;

	setmodel (missile, "models/fireball.mdl");
	setsize(missile,'0 0 0','0 0 0');
	setorigin(missile,self.origin+self.proj_ofs+v_forward*16);
	missile.think=BecomeImp;
	thinktime missile : 1;
	self.cnt_summon-=1;
}

/*
	teleport_coin_run - The entity "teleportcoin" is created when teleport artifact is used 
*/
void teleport_coin_run (void)
{
	//added this for chaos device hangin-around
//	self.touch = teleport_touch;
//	self.think = SUB_Remove;
//	self.nextthink = time + 3.0;//!shorter time, por favor!

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_TELEPORT_LINGER);
	WriteCoord (MSG_BROADCAST, self.origin_x);
	WriteCoord (MSG_BROADCAST, self.origin_y);
	WriteCoord (MSG_BROADCAST, self.origin_z);
	WriteCoord (MSG_BROADCAST, 3.0);

	//this part was here b4
	other = self.enemy;
	teleport_touch();
}

/*
   Use_teleportCoin - generates an entity that acts like a teleporter trigger which places the
                      player in his startspot or in deathmatch at a random start spot
*/
void () Use_TeleportCoin =
{
entity teleport_ent;

	self.flags2(+)FL_TORNATO_SAFE;
	teleport_ent = spawn();

	teleport_ent.goalentity = SelectSpawnPoint ();

	teleport_ent.classname = "teleportcoin";
	teleport_ent.inactive = FALSE;
	teleport_ent.think = teleport_coin_run;
	teleport_ent.nextthink = time + .01;
	teleport_ent.spawnflags = 1;//player_only, that is.

	self.cnt_teleport -= 1;
	teleport_ent.enemy = self;

	//added this for chaos device hangin-around
	setorigin (teleport_ent, self.origin);
	teleport_ent.movetype = MOVETYPE_NONE;
	teleport_ent.solid = SOLID_TRIGGER;
	teleport_ent.takedamage = DAMAGE_NO;
    setsize(teleport_ent,'-16 -16 0','16 16 56');
//	setmodel (teleport_ent, "models/sheep.mdl");//uncomment if tempent not working to see where teleport is supposed to be


};

void wedge_run(void)
{
    if ((self.owner.velocity_x == 0) && (self.owner.velocity_y == 0) && (self.owner.velocity_z == 0))
		self.effects(+)EF_NODRAW;	// All stop
    else if (self.effects & EF_NODRAW) 
		self.effects(-)EF_NODRAW;

	self.angles = vectoangles(self.owner.velocity);

	self.origin = self.owner.origin;
	self.think = wedge_run;
//self.nextthink = time + HX_FRAME_TIME;
	self.nextthink = time + .04;    // This faster time is because it would lag behind every once in a while

	if ((self.owner.health<=0) || !(self.owner.artifact_active & ART_HASTE))
		remove(self);

}

/*
void launch_hastewedge (void)
{
	local entity tail;

	tail = spawn ();
	tail.movetype = MOVETYPE_NOCLIP;
	tail.solid = SOLID_NOT;
	tail.classname = "haste_wedge";
	setmodel (tail, "models/wedge.mdl");
	setsize (tail, '0 0 0', '0 0 0');		
	tail.drawflags(+)DRF_TRANSLUCENT;

	tail.owner = self;
	tail.origin = tail.owner.origin;
	tail.velocity = tail.owner.velocity;
    tail.angles = tail.owner.angles;

	tail.think = wedge_run;
	tail.nextthink = time + HX_FRAME_TIME;

}
*/

void Use_TomeofPower (void)
{
	if(self.model=="models/sheep.mdl")
		self.sheep_time=0;
	else
	{
		if((tomeMode == 1)||(tomeMode == 2))
		{	// does nothing but sheep stuff...
			return;
		}
		self.artifact_active (+) ART_TOMEOFPOWER;
		self.tome_time = time + TOME_TIME;
	}
	self.cnt_tome -= 1;
}

void Use_Haste (entity targ_ent)
{
	targ_ent.artifact_active (+) ART_HASTE;
	targ_ent.haste_time = time + 20;

	targ_ent.effects(+)EF_DARKFIELD;
	PlayerSpeed_Calc(targ_ent);
}

/*
============
Use_ProximityMine
============
*/

void proximity_explode()
{
	T_RadiusDamage (self, self.owner, self.dmg, world);
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_TIME_BOMB);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	multicast(self.origin,MULTICAST_PHS);
	remove(self);
}

void proximity_think ()
{
float okay;
	thinktime self : 0.1;
	if(self.lifetime<time)
	{
		self.think=proximity_explode;
		thinktime self : 0;
		return;
	}
	if(!visible(self.enemy)&&!self.enemy.effects&EF_NODRAW)
		self.enemy=world;
	if(self.enemy)
	{
	float dist;
	vector org;
		org=(self.enemy.absmin+self.enemy.absmax)*0.5;
		dist=vlen(org-self.origin);
		if(dist<100)
		{
			sound(self,CHAN_AUTO,"misc/warning.wav",1,ATTN_NORM);
			self.think=proximity_explode;
			thinktime self : 0.3;
		}
		else
			self.velocity=normalize(org-self.origin)*150;
	}
	else
	{
		if(random()<0.5)
		{
		float bestdist,lastdist;
		entity found;
			bestdist=1001;
			found=findradius(self.origin,1000);
			while(found)
			{
				if(found.health&&found.flags2&FL_ALIVE&&!found.effects&EF_NODRAW&&found!=self.owner&&found!=self&&found.controller!=self.owner)
				{
					if(coop&&found.classname=="player")
						okay=FALSE;
					if(teamplay&&found.team==self.owner.team)
						okay=FALSE;
					else
						okay=TRUE;
					if(okay)
					{
						lastdist=vlen(found.origin-self.origin);
						if(lastdist<bestdist)
						{
							self.enemy=found;
								bestdist=lastdist;
						}
					}
				}
				found=found.chain;
			}
		}
	}
}

void Use_Proximity_Mine ()
{
	newmis=spawn();
	newmis.owner=self;
	newmis.classname="proximity";
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.dmg=50+self.level*10;
	if(newmis.dmg>120)
		newmis.dmg=120;
	newmis.health=10;
	newmis.takedamage=DAMAGE_YES;
	newmis.touch=newmis.th_die=proximity_explode;
	newmis.angles_x=90;
	newmis.avelocity_y=100;
	newmis.skin=1;
	newmis.drawflags(+)MLS_POWERMODE;
	setmodel (newmis, "models/glyphwir.mdl");
	setsize(newmis,'-3 -3 -3','3 3 3');
	newmis.hull=HULL_POINT;
	setorigin(newmis,self.origin+self.proj_ofs);
	newmis.lifetime=time+30;
	newmis.think=proximity_think;
	thinktime newmis : 0;
}

/*
============
UseTimebomb
============
*/

void TimeBombExplode()
{
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_TIME_BOMB);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	multicast(self.origin,MULTICAST_PHS);

	T_RadiusDamage(self, self.owner, 140.0, self.owner);

	remove(self);
}

void TimeBombTouch()
{
	if((other == self.owner)||(other == world)||(!(other.takedamage)))
	{
		return;
	}

	TimeBombExplode();
}

void TimeBombThink()
{
	vector	destination;

/*	float dist;
	float vel;
	// move around and stay close to the owner.
	dist = vlen(self.owner.origin - self.origin);

	if(dist > 160)
	{	// teleport into place
		self.origin = self.owner.origin + normalize(self.origin - self.owner.origin)*120 + '0 0 20';
	}
	else if(dist > 80)
	{
		vel = vlen(self.owner.velocity);
		self.velocity = normalize(self.owner.origin + '0 0 20' - self.origin)*vel;
	}
	else
	{
		self.velocity_x = 0;
		self.velocity_y = 0;
		self.velocity_z = 0;
	}

*/
	// self.health is a constant value that adds a good randomness for multiple bombs...
	destination_x = self.owner.origin_x + cos(time*200 + self.health*100) * 90;
	destination_y = self.owner.origin_y + sin(time*200 + self.health*100) * 90;
	destination_z = self.owner.origin_z + cos(time*300 + self.health*100) * 20 + 36;

	self.origin = destination;

	//self.velocity_x = (destination_x - self.origin_x)/.05;
	//self.velocity_y = (destination_y - self.origin_y)/.05;
	//self.velocity_z = (destination_z - self.origin_z)/.05;

	// run out if too long
	if(time > self.health + 10.0)
	{
		TimeBombExplode();
	}
	thinktime self : 0.05;
}


void Use_TimeBomb()
{
	newmis=spawn();
	newmis.owner=self;
	newmis.enemy=world;
	newmis.classname="timebomb";
	newmis.solid=SOLID_BBOX;
	newmis.dmg=50;
	newmis.touch=TimeBombTouch;
	newmis.angles_x=90;
	newmis.avelocity_y=100;
	newmis.skin=1;
	newmis.drawflags(+)MLS_ABSLIGHT;
	newmis.abslight=0.5;
	setmodel (newmis, "models/glyphwir.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs);
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.think=TimeBombThink;
	thinktime newmis : 0.05;
	newmis.health = time;
}

/*
============
	Anything which can change a velocity of a client effect missile
	must call this function
============
*/

void UpdateMissileVelocity(entity missile)
{
	
	if((missile.flags & EF_NODRAW)||(missile.model == ""))
	{	//if nodraw, it is probably a client effect and needs updating
		if((missile.classname == "magic missile")||
		(missile.classname == "bone_powered")||
		(missile.classname == "bone_shrapnel")||
		(missile.classname == "bone_normal"))
		{
			turneffect(missile.wrq_effect_id, missile.origin, missile.velocity);
		}
		else if ((missile.classname == "bird_missile")||
		(missile.classname == "set_missile"))
		{
			turneffect(missile.raven_effect_id, missile.origin, missile.velocity);
		}
/*		else if ((missile.classname == "flaming arrow")||
		(missile.classname == "bolt"))
		{
			XbowBoltTurn(missile);
		}*/
		else if (missile.classname == "polyblob")
		{
			PolyTurn(missile);
		}
//		else if (missile.classname == "pincer")
//		{
//			DrillaTurn(missile);
//		}
	}
}

/*
============
UseBlast
============
*/
void UseBlast (void)
{
	vector  dir,holdpos;
	entity  victim;
	float v_length,push,percent,points,inertia;
	
	self.cnt_blast -= 1;//moved this up here--doing damage to other things can
	//trigger an explosion and kill me, so if i decriment count after that, i might end up with -1 discs.

	victim = findradius( self.origin, BLAST_RADIUS*2);
//	self.safe_time=time+7;

	while(victim)
	{
		if(victim.classname=="cube_of_force"&&victim.controller!=self&&random()<0.2)
		{
			if(victim.artifact_flags&AFL_CUBE_RIGHT)
				victim.controller.artifact_flags(-)AFL_CUBE_RIGHT;
			if(victim.artifact_flags&AFL_CUBE_LEFT)
				victim.controller.artifact_flags(-)AFL_CUBE_LEFT;
			victim.frags=2;
			victim.movetype=MOVETYPE_BOUNCE;
			stopSound(victim,0);
			victim.owner = victim.controller = self;
			victim.velocity = normalize(victim.origin - (self.absmin+self.absmax)*0.5)*600;
			victim.avelocity=randomv('-300 -300 -300','300 300 300');
			if(victim.movedir!='0 0 0')
				victim.movedir=normalize(victim.velocity);
			victim.dmg=75;
			victim.touch = GrenadeTouch2;
			victim.think = MultiExplode;
			thinktime victim : 3;

			holdpos = victim.origin;
			holdpos_z += (victim.maxs_z - victim.mins_z)/2;
			traceline(self.origin,holdpos,FALSE,self);
			CreateBlueFlash(trace_endpos);
		}
		else if (victim.classname!="hook"&&victim.owner.classname != "circfire" && victim.classname != "cube_of_force"&&victim.monsterclass<CLASS_BOSS)
		{
//			dprint(victim.classname);
//			dprint(" blasted\n");
			if (
				(
				(victim.health&&victim!=self) ||
				(victim.movetype == MOVETYPE_FLYMISSILE) ||
				(victim.movetype == MOVETYPE_BOUNCEMISSILE)
				) &&victim.owner != self&&victim.movetype!=MOVETYPE_PUSH)
			{
				traceline(self.origin,victim.origin,TRUE,self);

				if (trace_fraction == 1)  // No walls in the way
				{
					sound (self, CHAN_WEAPON, "raven/blast.wav", 1, ATTN_NORM);

					if (((victim.movetype != MOVETYPE_FLYMISSILE) && (victim.movetype != MOVETYPE_BOUNCEMISSILE)) || (victim.classname =="chain_head" ))
					{	
						dir =  victim.origin - self.origin;
						v_length = vlen (dir);

						// For missile entities can only be moved within blast radius
						if  (v_length < BLAST_RADIUS)
						{
							// The further away, the less the push
							percent = BLAST_RADIUS / v_length;
								
							if (percent > 3)
								percent = 3;

							if (victim.mass>20)
								inertia = victim.mass/20;
							else
								inertia = 1;

							push = (percent + 1)/inertia;
							victim.velocity = dir * push;
							victim.flags(-)FL_ONGROUND;

							push = ((percent * 100) + 100)/inertia;
							victim.velocity_z = push;
							UpdateMissileVelocity(victim);
						}
					}
					else
					{
						victim.frags=2;
						//if(victim.classname=="pincer")
						//	victim.enemy=victim.owner;
						victim.enemy=victim.owner;
						victim.owner = self;
						if (victim.classname!="tornato")
						{
							victim.velocity = victim.velocity * -1;
							victim.angles = vectoangles(victim.velocity);
							UpdateMissileVelocity(victim);
						}
					}

					holdpos = victim.origin;
					holdpos_z += (victim.maxs_z - victim.mins_z)/2;
					traceline(self.origin,holdpos,FALSE,self);
					CreateBlueFlash(trace_endpos);

					points = percent * BLASTDAMAGE;  // Minimum blast damage
					if (points > 10)
						points = 10;

//Bad idea- if someone uses a blast radius on a player and that player falls in
//the lava 10 minutes later without being hurt by another player in the
//meantime, original player gets credit- not good.
//					if(victim.classname=="player")
//						if(!victim.artifact_active&ARTFLAG_FROZEN)
//							victim.credit_enemy=self;

					T_Damage (victim, self, self, points);
				}
			}
		}

		if (victim.classname=="tornato" && victim.enemy.flags2&FL_ALIVE)
			victim.enemy.flags2(+)FL_TORNATO_SAFE;

//		if(victim.classname=="swarm")
//		{
//			victim.think=hive_die;
//			thinktime victim : 0;
//		}

		victim = victim.chain;
	}

	if(self.v_angle_x > 30)
	{
		if(self.artifact_active & ART_TOMEOFPOWER)
		{
			self.velocity_z += 500 * sin(self.v_angle_x);
		}
		else
		{
			self.velocity_z += 300 * sin(self.v_angle_x);
		}
	}
}

void jail_touch ()
{
entity found,oself;
float found_cnt;
	particleexplosion(self.origin,random(144,159),self.absmax_z-self.absmin_z,10);
	if(other.classname!="player"||(!other.flags&FL_CLIENT)||other.model=="models/yakman.mdl")
	{
		//remove(self);
		// KS change:
		self.nextthink = time + 0.1;
		self.think = SUB_Remove;

		return;
	}

	found=find(world,targetname,"prison");
	found_cnt=0;
	while(found)
	{
		found_cnt+=1;
		if(random(8)<=1||found_cnt==8)
		{
			self.goalentity=found;
			found=world;
		}
		else
			found=find(found,targetname,"prison");
	}
	if(!self.goalentity)
		return;

	if(other.siege_team!=self.owner.siege_team)
		self.owner.experience += 50;//jailed enemy
	else //jailed self or pal
		self.owner.experience -= 25;

	other.artifact_active(+)ARTFLAG_DIVINE_INTERVENTION;
	other.divine_time = time + HX_FRAME_TIME;
	other.flags2(+)FL_TORNATO_SAFE;
	other.jail_time=time+180;
	sprintname(other,PRINT_HIGH,self.owner);
	sprint(other,PRINT_HIGH," imprisoned you!\n");
	other.climbing = FALSE;
	oself=self;
	self=other;
	DropBackpack();
	self=oself;
	self.classname = "teleportcoin";
	self.inactive = FALSE;
	self.think = teleport_coin_run;
	self.nextthink = time + .01;
	self.spawnflags =0;

	self.enemy = other;
}

void UseInvincibility (void)
{
entity jailer;
entity found;
float found_cnt;
	jailer=spawn();
	jailer.owner=self;

	if(self.siege_team==ST_ATTACKER)
	{
		jailer.classname = "teleportcoin";
		jailer.inactive = FALSE;
		jailer.spawnflags =0;
		jailer.enemy=self;
		particleexplosion(self.origin+'0 0 0.5'*self.maxs_z,random(144,159),self.absmax_z-self.absmin_z,10);
		if(self.beast_time)
			UnBeast(self);

		found=find(world,targetname,"prison");
		found_cnt=0;
		while(found)
		{
			found_cnt+=1;
			if(random(8)<=1||found_cnt==8)
			{
				jailer.goalentity=found;
				found=world;
			}
			else
				found=find(found,targetname,"prison");
		}
		if(!jailer.goalentity)
		{
			remove(jailer);
			return;
		}
		jailer.think = teleport_coin_run;
		jailer.nextthink = time + .01;
	}
	else
	{
		makevectors(self.v_angle);
		jailer.solid=SOLID_BBOX;
		jailer.touch=jail_touch;
		jailer.movetype=MOVETYPE_FLYMISSILE;
		jailer.speed=1000;
		jailer.velocity=v_forward*jailer.speed;
		jailer.drawflags=MLS_POWERMODE;
		jailer.effects=EF_DIMLIGHT;

		setmodel(jailer,"models/polymrph.spr");
		setsize(jailer,'0 0 0','0 0 0');
		setorigin(jailer,self.origin+self.proj_ofs+v_forward*10);
	}
	self.cnt_invincibility -= 1;
/*
	self.artifact_active (+) ART_INVINCIBILITY;
	if(deathmatch)
		self.invincible_time = time + TOME_TIME;
	else
		self.invincible_time = time + 10;
	self.artifact_low (-) ART_INVINCIBILITY;

//Temp invincibility effects
	if(self.playerclass==CLASS_CRUSADER)
		self.skin = GLOBAL_SKIN_STONE;
	else if(self.playerclass==CLASS_PALADIN)
		self.effects(+)EF_BRIGHTLIGHT; 
	else if(self.playerclass==CLASS_ASSASSIN)
		self.colormap=140;
	else if(self.playerclass==CLASS_SUCCUBUS)
		self.colormap=0;
	else if(self.playerclass==CLASS_NECROMANCER)
		self.effects(+)EF_DARKLIGHT;

*/
}

void UseInvisibility (entity targ_ent)
{
	targ_ent.artifact_active (+) ART_INVISIBILITY;
	targ_ent.invisible_time = time + 20;
	targ_ent.artifact_low (-) ART_INVISIBILITY;

	msg_entity=targ_ent;
	WriteByte(MSG_ONE, SVC_SET_VIEW_FLAGS);
	WriteByte(MSG_ONE,DRF_TRANSLUCENT);
	targ_ent.effects(+)EF_NODRAW|EF_LIGHT; 
	targ_ent.oldskin=targ_ent.skin;
	targ_ent.drawflags(+)DRF_TRANSLUCENT;
	targ_ent.skin=101;
	if(targ_ent.cnt_invisibility>=1)
		targ_ent.cnt_invisibility -= 1;
}

void()Use_Polymorph;
void()Use_Tripwire;
void()Use_Fireball;

void poisong_die()
{
	//dprint("Poisong_die\n");
	stopSound(self,0);
	SmallExplosion();
}

void SpewPoison ()
{
//vector updir;
	if(self.solid!=SOLID_PHASE)				
	{
		setsize(self,'-3 -3 -3','3 3 3');
		self.solid=SOLID_PHASE;
	}
	if(self.owner.tripwire_cnt>self.tripwire_cnt+4)
		self.lifetime=0;

	if(self.lifetime==-1)
		self.lifetime=time + 30;
	else if(self.lifetime<time)
	{

		sound(self,CHAN_UPDATE+PHS_OVERRIDE_R,"succubus/endhisss.wav",1,ATTN_NORM);
		self.owner.tripwire_cnt-=1;
		remove(self);
		return;
	}
	if(!self.aflag)
	{
		self.aflag=TRUE;
		starteffect(CE_ACID_EXPL,self.origin+'0 0 8','0 0 .2',HX_FRAME_TIME*2);
		weapon_sound(self, "succubus/gasss.wav");		
//		sound(self,CHAN_BODY,"succubus/gasss.wav",1,ATTN_NORM);
		self.t_width=time+2;
	}
	else if(self.t_width<time&&self.aflag==TRUE)//(self.velocity=='0 0 0'||self.flags&FL_ONGROUND)&&
	{
		self.aflag=2;
		sound(self,CHAN_UPDATE+PHS_OVERRIDE_R,"succubus/hisss.wav",1,ATTN_LOOP);
		self.effects(+)EF_UPDATESOUND;
	}
//	updir=randomv('-10 -10 10','10 10 30');
//Hissing sound
	//CreateGreenSmoke(self.origin,updir,HX_FRAME_TIME*3);
	self.effects (+) EF_POISON_GAS;
//Set up a trigger instead of T_Dam.  Might be faster.
	T_RadiusDamage(self,self.owner,3,self.owner);//was 1
	thinktime self : 0.1;
}

void Use_PoisonGas() 
{
        makevectors(self.v_angle);
		weapon_sound(self, "misc/whoosh.wav");
//		sound(self,CHAN_WEAPON,"misc/whoosh.wav",1,ATTN_NORM);
entity missile;
        missile=spawn();
        missile.owner=self;
        missile.classname="poison grenade";
        missile.movetype=MOVETYPE_BOUNCE;
		missile.takedamage=TRUE;
		missile.health=10;
		missile.th_die=poisong_die;
        missile.solid=SOLID_BBOX;
        missile.touch=GrenadeTouch2;
		missile.lifetime=-1;

		self.tripwire_cnt+=1;
		missile.tripwire_cnt=self.tripwire_cnt;

		missile.o_angle = self.origin+self.proj_ofs+v_forward*8;

		missile.speed=500;

		if(self.v_angle_x)
			missile.velocity = v_forward*missile.speed + v_up * 200 + crandom()*v_right*10 + crandom()*v_up*10;
		else
		{
			missile.velocity = aim(self, missile.o_angle,1000);
	        missile.velocity = missile.velocity * missile.speed;
            missile.velocity_z = 200;
		}

		missile.angles = vectoangles(missile.velocity);
		missile.avelocity=randomv('-300 -300 -300','300 300 300');

        setmodel(missile,"models/glyphwir.mdl");
        setsize(missile,'0 0 0','0 0 0');
        setorigin(missile,missile.o_angle);
		missile.think=SpewPoison;
		thinktime missile : 0.5;
}

void BreakChainsForEnt(entity ent)
{
	if(!ent.flags2&FL_CHAINED)
		return;

	ent.flags2(-)FL_CHAINED;
	if (ent.classname == "player")
	{
		if(!(ent.rings&RING_FLIGHT))
			ent.movetype=MOVETYPE_WALK;
	}
	else
	{
		ent.movetype = ent.oldmovetype;
	}
}

void BreakChains()
{
	if(!self.flags2&FL_CHAINED)
		return;

	self.flags2(-)FL_CHAINED;
	if(!self.rings&RING_FLIGHT)
		self.movetype=MOVETYPE_WALK;
}

void UseInventoryItem (void)
{

	if (self.health <= 0)
		return;

//Have to allow panic button and QuickInventory impulses to work as well as impulse 23
	if(self.flags2&FL_CHAINED)//&&self.inventory!=INV_TELEPORT&&self.inventory!=INV_BLAST)
		return;

	// Is it in the inventory
	if ((self.inventory == INV_TORCH) && (self.cnt_torch))
	{
		if (self.torchtime < (time + 5))
		{
			UseTorch ();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_TELEPORT) && (self.cnt_teleport))
	{
		Use_TeleportCoin ();
		BreakChains();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_HP_BOOST) && (self.cnt_h_boost))
	{
		use_healthboost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_SUPER_HP_BOOST) && (self.cnt_sh_boost))
	{
		use_super_healthboost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_MANA_BOOST) && (self.cnt_mana_boost))
	{
		UseManaBoost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_GLYPH) && (self.cnt_glyph))
	{
		if(self.playerclass==CLASS_ASSASSIN)
		{
			self.cnt_glyph-=1;
			Use_Tripwire();
			self.flags (+) FL_ARTIFACTUSED;
		}
		else
			centerprint(self,"You can't use that item!\n");
/*		else if(self.playerclass==CLASS_CRUSADER)
			Use_TimeBomb ();
		else if(self.playerclass==CLASS_PALADIN)
			Use_Fireball();
		else if(self.playerclass==CLASS_NECROMANCER)
			Use_Proximity_Mine();
		else if(self.playerclass==CLASS_SUCCUBUS)
			Use_PoisonGas();
		self.flags (+) FL_ARTIFACTUSED;*/
	}
	else if ((self.inventory == INV_HASTE) && (self.cnt_haste))
	{
		if (self.haste_time < (time + 5))
		{
			Use_Haste(self);		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_BLAST) && (self.cnt_blast))
	{
		UseBlast();		
		BreakChains();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_CUBEOFFORCE) && (self.cnt_cubeofforce))
	{
		if ((!self.artifact_flags & AFL_CUBE_LEFT) ||
			(!self.artifact_flags & AFL_CUBE_RIGHT))
		{
			UseKeyFinder();
			//UseCubeOfForce();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_INVINCIBILITY) && (self.cnt_invincibility))
	{
		if (self.invincible_time < (time + 5))
		{
			UseInvincibility();		
			BreakChains();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_INVISIBILITY) && (self.cnt_invisibility))
	{
		if (self.invisible_time < (time + 5))
		{
			UseInvisibility(self);		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_TOME) && (self.cnt_tome))
	{
		if (self.tome_time < (time + 5))
		{
			Use_TomeofPower();		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	else if ((self.inventory == INV_POLYMORPH) && (self.cnt_polymorph))
	{
		Use_Polymorph();		
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_SUMMON) && (self.cnt_summon))
	{
		Use_Summoner();		
		self.flags (+) FL_ARTIFACTUSED;
	}
	else if ((self.inventory == INV_FLIGHT) && (self.cnt_flight))
	{
		Use_RingFlight();
		self.flags (+) FL_ARTIFACTUSED;
	}
	else
	{	// Can't even try to use flight in non-deathmatch
		if ((!deathmatch) && (self.inventory == INV_FLIGHT))
			time = time;
		else
		{
			sprinti(self, PRINT_MEDIUM, STR_DONOTPOSSESS);
		}
	}

}

void PanicButton ()
{

	if (self.health <= 0)
		return;

//	if(self.flags2&FL_CHAINED)
//		return;

	if (self.cnt_torch)
	{
		if (self.torchtime < (time + 5))
		{
			UseTorch ();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_teleport)
	{
		Use_TeleportCoin ();
		BreakChains();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_h_boost)
	{
		use_healthboost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_sh_boost)
	{
		use_super_healthboost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_mana_boost)
	{
		UseManaBoost ();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_glyph)
	{
		self.cnt_glyph=self.cnt_glyph - 1;
		if(self.playerclass==CLASS_ASSASSIN)
			Use_Tripwire();
		else if(self.playerclass==CLASS_CRUSADER)
			Use_TimeBomb ();
		else if(self.playerclass==CLASS_PALADIN)
			Use_Fireball();
		else if(self.playerclass==CLASS_NECROMANCER)
			Use_Proximity_Mine();
		else if(self.playerclass==CLASS_SUCCUBUS)
			Use_PoisonGas();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_haste)
	{
		if (self.haste_time < (time + 5))
		{
			Use_Haste(self);		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_blast)
	{
		UseBlast();		
		BreakChains();
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_cubeofforce)
	{
		if ((!self.artifact_flags & AFL_CUBE_LEFT) ||
			(!self.artifact_flags & AFL_CUBE_RIGHT))
		{
			UseKeyFinder();
			//UseCubeOfForce();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_invincibility)
	{
		if (self.invincible_time < (time + 5))
		{
			UseInvincibility();		
			BreakChains();
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_invisibility)
	{
		if (self.invisible_time < (time + 5))
		{
			UseInvisibility(self);		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_tome)
	{
		if (self.tome_time < (time + 5))
		{
			Use_TomeofPower();		
			self.flags (+) FL_ARTIFACTUSED;
		}
	}
	if (self.cnt_polymorph)
	{
		Use_Polymorph();		
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_summon)
	{
		Use_Summoner();		
		self.flags (+) FL_ARTIFACTUSED;
	}
	if (self.cnt_flight)
	{
		Use_RingFlight();
		self.flags (+) FL_ARTIFACTUSED;
	}
}

void burn_it ()
{
	if(other==self.owner||other.classname=="flarrow")
		return;

	if(other.flags2&FL_ALIVE&&self.flags&FL_ONGROUND&&random()<0.99)
		return;//can't leave 'em sitting 1% chance to set fire per touch if not in air

	if(!other.flags2&FL2_ONFIRE);
	{//torch 'em
		if(flammable(other))
		{
			spawn_burner(other,FALSE);
			if(other.classname!="bolt")
			{
				self.touch=SUB_Null;
				self.think=SUB_Remove;
				thinktime self : 0;
			}
			else
			{//now a flaming arrow
				other.classname="flarrow";
				other.frags=TRUE;
			}
		}
	}
}

void thrown_torch_think ()
{//FIXME: If you pick it back up, it should still be lit and timing out
	if (self.torchtime < time)
		self.torchthink ();
	if(self.velocity=='0 0 0')
		self.touch=SUB_Null;
	self.think=thrown_torch_think;
	thinktime self : 0.5;
}

void throw_torch (entity throwtorch)
{
	makevectors(self.v_angle);
	if(self.torchtime>time+10)//less burntime if thrown, 10 sec max
		throwtorch.torchtime = time + 10;
	else
		throwtorch.torchtime = self.torchtime;
	if(self.effects&EF_DIMLIGHT)
		throwtorch.effects(+)EF_DIMLIGHT;
	if(self.effects&EF_TORCHLIGHT)
		throwtorch.effects(+)EF_TORCHLIGHT;
	throwtorch.torchthink=self.torchthink;
	throwtorch.touch = burn_it;

	throwtorch.think=thrown_torch_think;
	thinktime throwtorch : 0;

	self.effects(-)EF_DIMLIGHT;   // Turn off lights
	self.artifact_flags(-)AFL_TORCH;  // Turn off torch flag
	self.effects(-)EF_TORCHLIGHT;
	self.torchtime = 0;
}

void  DropInventoryItem (void)
{
	entity item,holdent,oself;
	float throwflag,torch_thrown;

	makevectors(self.v_angle);
	traceline(self.origin + self.proj_ofs,self.origin + self.proj_ofs + v_forward * 60,FALSE,self);

	if (trace_fraction < 1)
	{
		if(trace_ent.classname!="player")
		{
			if(trace_ent.model=="models/gfire.mdl"&&trace_ent.frame==0)
			{
				if ((self.inventory == INV_TORCH) && (self.cnt_torch))
				{
					if(self.artifact_flags&AFL_TORCH)
					{
						self.effects(-)EF_DIMLIGHT;   // Turn off lights
						self.artifact_flags(-)AFL_TORCH;  // Turn off torch flag
						self.effects(-)EF_TORCHLIGHT;
						self.torchtime = 0;
					}
					oself=self;
					self=trace_ent;
					self.use();
					self=oself;
					self.cnt_torch -=1;
					if (self.cnt_torch < 0)
						self.cnt_torch = 0;
					return;
				}
			}
			centerprint(self,"Not enough room to throw");
			return;
		}
		else if ((self.inventory == INV_TORCH) && (self.cnt_torch))
			if(flammable(trace_ent)&&!(trace_ent.flags2&FL2_ONFIRE)&&self.artifact_flags&AFL_TORCH)
			{//set them on fire
				spawn_burner(trace_ent,FALSE);
				self.effects(-)EF_DIMLIGHT;   // Turn off lights
				self.artifact_flags(-)AFL_TORCH;  // Turn off torch flag
				self.effects(-)EF_TORCHLIGHT;
				self.torchtime = 0;
				self.cnt_torch -=1;
				if (self.cnt_torch < 0)
					self.cnt_torch = 0;
				return;
			}
	}

	item = spawn();

	item.flags(+)FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	item.owner = self;
	item.artifact_ignore_owner_time = time + 2;
	item.artifact_ignore_time = time + 0.1;

	setsize (item, '-8 -8 -38', '8 8 24');
	holdent=self;
	self = item;
	throwflag = 0;

	// Is it in the inventory
	if ((holdent.inventory == INV_TORCH) && (holdent.cnt_torch))
	{
		if(holdent.artifact_flags&AFL_TORCH)
			torch_thrown=TRUE;
		spawn_artifact(ARTIFACT_TORCH,NO_RESPAWN);
		holdent.cnt_torch -=1;
		if (holdent.cnt_torch < 0)
			holdent.cnt_torch = 0;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_HP_BOOST) && (holdent.cnt_h_boost))
	{
		spawn_artifact(ARTIFACT_HP_BOOST,NO_RESPAWN);
		holdent.cnt_h_boost -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_SUPER_HP_BOOST) && (holdent.cnt_sh_boost))
	{
		spawn_artifact(ARTIFACT_SUPER_HP_BOOST,NO_RESPAWN);
		holdent.cnt_sh_boost -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_MANA_BOOST) && (holdent.cnt_mana_boost))
	{
		spawn_artifact(ARTIFACT_MANA_BOOST,NO_RESPAWN);
		holdent.cnt_mana_boost -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_TELEPORT) && (holdent.cnt_teleport))
	{
		spawn_artifact(ARTIFACT_TELEPORT,NO_RESPAWN);
		holdent.cnt_teleport -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_TOME) && (holdent.cnt_tome))
	{
		spawn_artifact(ARTIFACT_TOME,NO_RESPAWN);
		holdent.cnt_tome -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_SUMMON) && (holdent.cnt_summon))
	{
		spawn_artifact(ARTIFACT_SUMMON,NO_RESPAWN);
		holdent.cnt_summon -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_INVISIBILITY) && (holdent.cnt_invisibility))
	{
		spawn_artifact(ARTIFACT_INVISIBILITY,NO_RESPAWN);
		holdent.cnt_invisibility -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_GLYPH) && ((holdent.cnt_glyph&&holdent.playerclass!=CLASS_CRUSADER)||holdent.cnt_glyph>=5))
	{
		spawn_artifact(ARTIFACT_GLYPH,NO_RESPAWN);
		if(holdent.playerclass==CLASS_CRUSADER)
			holdent.cnt_glyph -=5;
		else
			holdent.cnt_glyph -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_HASTE) && (holdent.cnt_haste))
	{
		spawn_artifact(ARTIFACT_HASTE,NO_RESPAWN);
		holdent.cnt_haste -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_BLAST) && (holdent.cnt_blast))
	{
		spawn_artifact(ARTIFACT_BLAST,NO_RESPAWN);
		holdent.cnt_blast -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_POLYMORPH) && (holdent.cnt_polymorph))
	{
		spawn_artifact(ARTIFACT_POLYMORPH,NO_RESPAWN);
		holdent.cnt_polymorph -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_FLIGHT) && 
					((holdent.cnt_flight > 0 && !(holdent.rings&RING_FLIGHT))||//if we have just 1 left, and it's in use, don't drop it
					(holdent.cnt_flight > 1)))
	{
		spawn_artifact(ARTIFACT_FLIGHT,NO_RESPAWN);
		holdent.cnt_flight -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_CUBEOFFORCE) && (holdent.cnt_cubeofforce))
	{
		spawn_artifact(ARTIFACT_CUBEOFFORCE,NO_RESPAWN);
		holdent.cnt_cubeofforce -=1;
		throwflag = 1;
	}
	else if ((holdent.inventory == INV_INVINCIBILITY) && (holdent.cnt_invincibility))
	{
		spawn_artifact(ARTIFACT_INVINCIBILITY,NO_RESPAWN);
		holdent.cnt_invincibility -=1;
		throwflag = 1;
	}

	self = holdent;

	if (throwflag)  // Something could be thrown
	{
		// Throw it
		item.velocity = normalize (v_forward);
		item.velocity = item.velocity * 200;
		item.velocity_x += random(-20,20);	// So they don't land on top if each other if player is 
		item.velocity_y += random(-20,20);	// standing in one place and throwing multiple items
		item.velocity_z = 200;
		makevectors(self.v_angle);
		setorigin(item,self.origin + self.proj_ofs + v_up * 10 + v_forward * 40 + v_right * 8);
		
		sound(self,CHAN_BODY,"misc/whoosh.wav",1,ATTN_NORM);
		if(torch_thrown)
			throw_torch(item);
	}
	else
		remove(item);

}

void Inventory_Quick(float which)
{
	float old_inv;

	old_inv = self.inventory;
	self.inventory = which;
	UseInventoryItem();
	self.inventory = old_inv;
}

