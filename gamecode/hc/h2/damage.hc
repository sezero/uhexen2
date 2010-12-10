/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/damage.hc,v 1.3 2007-02-07 16:57:00 sezero Exp $
 */

void() T_MissileTouch;
void() info_player_start;
void necromancer_sphere(entity ent);
void crusader_sphere(entity ent);

void() monster_death_use;
void()player_pain;
void()PlayerDie;
void MonsterDropStuff(void);
void Use_TeleportCoin(void);
void UseInvincibility(void);
void Use_TomeofPower(void);
void use_super_healthboost();


float ClassArmorProtection[16] =
{
	// Paladin Armor Protection
	.05,	// AMULET
	.10,	// BRACERS
	.25,	// BREASTPLATE
	.15,	// HELMET

	// Crusader Armor Protection
	.15,	// AMULET
	.05,	// BRACER
	.10,	// BREASTPLATE
	.25,	// HELMET

	// Necromancer Armor Protection
	.25,	// AMULET
	.15,	// BRACER
	.05,	// BREASTPLATE
	.10,	// HELMET

	// Assassin Armor Protection
	.10,	// AMULET
	.15,	// BRACER
	.25,	// BREASTPLATE
	.05		// HELMET

};




//============================================================================
/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
float(entity targ, entity inflictor) CanDamage =
{
// bmodels need special checking because their origin is 0,0,0
vector inflictor_org,targ_org,ofs;
float targ_rad,loop_cnt;

	if(inflictor.flags2&FL_ALIVE)
		inflictor_org = inflictor.origin+inflictor.proj_ofs;
	else
		inflictor_org = (inflictor.absmin+inflictor.absmax)*0.5;

	targ_org=(targ.absmin+targ.absmax)*0.5;
//	targ_rad=targ.maxs_x;
	targ_rad=15;

	if (targ.movetype == MOVETYPE_PUSH)
	{
		traceline(inflictor_org, targ_org, TRUE, self);
		if (trace_fraction == 1)
			return TRUE;
		if (trace_ent == targ)
			return TRUE;
		return FALSE;
	}
	
	ofs='0 0 0';
	loop_cnt=5;
	while(loop_cnt)
	{
		if(loop_cnt!=5)
		{
			if(loop_cnt<3)
				ofs_x=targ_rad*-1;
			else
				ofs_x=targ_rad;
			if(loop_cnt==3||loop_cnt==2)
				ofs_y=targ_rad*-1;
			else
				ofs_y=targ_rad;
		}
		traceline(inflictor_org, targ_org + ofs, TRUE, self);
		if (trace_fraction == 1)
			return TRUE;
		loop_cnt-=1;
	}
//	dprintv("Can't damage from %s",inflictor_org);
//	dprintv(" to %s\n",targ_org);
	return FALSE;
};

entity FindExpLeader()
{
entity lastent, leader;
float top_exp;
	lastent=nextent(world);
	num_players=0;
	while(lastent)
	{
		if(lastent.classname=="player")
		{
			num_players+=1;
			if(lastent.experience>top_exp)
			{
				leader=lastent;
				top_exp=leader.experience;
			}
		}
		lastent=find(lastent,classname,"player");
	}
	return leader;
}

float Pal_DivineIntervention(void)
{
	float chance;

	if (self.level < 6)
		return(FALSE);

	chance = self.level * .02;
	if (chance > .20)
		chance = .20;

	if (chance < random())
		return(FALSE);

	centerprint (self,"Your God has saved your mortal body!");
	self.health = self.max_health;
	self.cnt_teleport += 1;
	Use_TeleportCoin();

	self.cnt_invincibility += 1;
	UseInvincibility ();
	self.invincible_time = time + 5;

	self.cnt_tome += 1;
	Use_TomeofPower ();

	self.artifact_active(+)ARTFLAG_DIVINE_INTERVENTION;
	self.divine_time = time + HX_FRAME_TIME;
	sound (self, CHAN_BODY, "paladin/devine.wav", 1, ATTN_NORM);

	return(TRUE);
}

/*
============
Killed
============
*/
void(entity targ, entity attacker, entity inflictor) Killed =
{
entity oself;
float exp_bonus;
	oself = self;
	self = targ;

	if(!self.flags2&FL_ALIVE)
		if (self.movetype == MOVETYPE_PUSH || self.movetype == MOVETYPE_NONE)
		{	// doors, triggers, etc
			if(self.th_die)
				self.th_die();
			self=oself;
			return;
		}

	self.flags2(-)FL_ALIVE;
	self.touch = self.th_pain = SUB_Null;

	if (attacker.classname == "player")
	{
		if ((attacker.playerclass==CLASS_NECROMANCER) && (attacker.level >= 3))
		{
			if ((targ.flags & FL_MONSTER) || (targ.flags & FL_CLIENT))
				necromancer_sphere (attacker);
		}
	//	else if ((attacker.playerclass==CLASS_CRUSADER) && (attacker.level >= 3))
	// Pa3PyX: this is supposed to be a level 6 ability,
	//	   according to the manual, not level 3.
		else if ((attacker.playerclass == CLASS_CRUSADER) && (attacker.level >= 6))
		{
			if ((targ.flags & FL_MONSTER) || (targ.flags & FL_CLIENT))
				crusader_sphere (attacker);
		}
	}

//Check for decapitation death
	self.movedir='0 0 0';
	if(self.model!="models/sheep.mdl"&&self.deathtype!="teledeath"&&self.deathtype!="teledeath2"&&self.deathtype!="teledeath3"&&self.deathtype != "teledeath4")
		if
		(inflictor.classname=="ax_blade"||
			(inflictor.classname=="player"&&
				(
					(attacker.playerclass==CLASS_ASSASSIN&&attacker.weapon==IT_WEAPON1)||
					(attacker.playerclass==CLASS_PALADIN&&attacker.weapon!=IT_WEAPON4)||
					(attacker.playerclass==CLASS_NECROMANCER&&attacker.weapon==IT_WEAPON1)
				)
			)
		)
			if(random()<0.3||self.classname=="monster_medusa")
				self.decap=2;
			else
				self.decap=TRUE;
		else if(inflictor.classname!="player"&&vlen(inflictor.origin-self.origin+self.view_ofs)<=17&&self.health>=-40&&self.health<-10)
			if(random()<0.4)
			{
				self.movedir=normalize(self.origin+self.view_ofs-inflictor.origin);
				self.decap=2;
			}

	if(self.skin==GLOBAL_SKIN_STONE||self.frozen>0)
	{	//Frozen or stoned
		if(self.classname!="player")
			self.th_die=shatter;			
		thinktime self : 0;
		self.attack_finished=time;
		self.pausetime=time;
		self.teleport_time=time;
		if(self.frozen>0)
			self.deathtype="ice shatter";
		else if(self.skin==GLOBAL_SKIN_STONE)
			self.deathtype="stone crumble";
	}

	if (self.classname == "player")
		ClientObituary(self, attacker, inflictor);

	if(attacker.deadflag<DEAD_DYING)
	{
		if(attacker.model=="models/sheep.mdl"&&attacker.flags&FL_CLIENT)
		{//3000 exp bonus for killing as sheep.
			sound (attacker, CHAN_BODY, "misc/comm.wav", 1, ATTN_NORM);
			centerprint(attacker,"Sheep kill BONUS!!!\n");
			exp_bonus=3000;
		}

		if(deathmatch)
		{
		entity lastleader;
			if(self.classname=="player")
				self.experience_value=(self.level*800 - 500)*exp_mult;//Exp gained is (level*800 - 500) * exp_mult
			if(attacker.flags&FL_CLIENT)
			{
				attacker.level_frags+=self.level;//Level frags
				lastleader=FindExpLeader();//Find King of the Hill
				if(lastleader==targ&&attacker!=targ)//Killed King		
				{
					sound (world, CHAN_BODY, "misc/comm.wav", 1, ATTN_NONE);
					bprint(attacker.netname);
					bprint(" took out the King of the Hill (");
					bprint(targ.netname);
					bprint(")!!!\n");
					if(num_players>2)//Only give bonus if more than 2 players
						targ.experience_value+=500*num_players - 2;	//Give an extra 500* num players,you beat others to the kill
				}
				if((self.classname=="player"&&attacker.classname=="player"&&teamplay&&attacker.team==self.team)||attacker==targ)
					drop_level(attacker,1);//Killed someone on your team, or killed self, lose a level, get no exp
				else
				{
					if(attacker.level<targ.level)
						drop_level(targ,1); //If killed by a lower level player, lose 1 level

					if(attacker!=self.controller)//No credit for killing your imp!
						AwardExperience(attacker,self,self.experience_value+exp_bonus);
				}
				if(FindExpLeader()!=lastleader)
				{//Tell everyone if the king of the hill has changed
					sound (world, CHAN_BODY, "misc/comm.wav", 1, ATTN_NONE);
					bprint(attacker.netname);
					bprint(" is the NEW King of the Hill!\n");
				}
			}
		}
		else if(self.classname=="player"&&attacker.classname=="player"&&(coop||teamplay&&attacker.team==self.team))
			drop_level(attacker,1);	//Killed friend in coop, lose a level

		else if(attacker.flags&FL_CLIENT&&attacker!=self.controller&&(self.monsterclass<CLASS_BOSS||self.classname=="obj_chaos_orb"))//Bosses award Exp themselves, to all players in coop
			AwardExperience(attacker,self,self.experience_value+exp_bonus);
	}

	self.enemy = attacker;

// bump the monster counter
	if (self.flags & FL_MONSTER)
	{
		MonsterDropStuff();
		killed_monsters = killed_monsters + 1;
		WriteByte (MSG_ALL, SVC_KILLEDMONSTER);
		monster_death_use();
		pitch_roll_for_slope('0 0 0');
	}
	else if(self.target)
		SUB_UseTargets();

	self.th_stand=self.th_walk=self.th_run=self.th_pain=self.oldthink=self.think=self.th_melee=self.th_missile=SUB_Null;
	
	if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
		DeathBubbles(20);

	if(attacker.classname=="rider_death")
		spawn_ghost(attacker);

	if(oself!=targ)
	{
		if(self.classname=="player")
			PlayerDie();	
		else if (self.th_die)
			self.th_die ();

		self = oself;
	}
	else
	{
		if(self.classname=="player")
			self.think=PlayerDie;
		else if (self.th_die)
			self.think=self.th_die;
		thinktime self : 0;
	}
	if (self.health < -99)
		self.health = -99;		// don't let sbar look bad if a player
};


void monster_pissed (entity attacker)
{
entity found;
	if(self.controller.classname=="player")
	{//Summoned/controlled monsters
		if(coop)
			if(found.classname=="player")
				return;

		if(deathmatch&&teamplay)
			if(found.team==self.controller.team)
				return;
	}

	if (self.enemy.classname == "player")
		self.oldenemy = self.enemy;
	self.enemy = attacker;
	self.goalentity = self.enemy;

	if (self.th_walk)
		FoundTarget ();
}

float armor_inv(entity victim)
{
	float armor_cnt;

	armor_cnt =0;

	if (victim.armor_amulet)
		armor_cnt += 1;

	if (victim.armor_bracer)
		armor_cnt += 1;

	if (victim.armor_breastplate)
		armor_cnt += 1;

	if (victim.armor_helmet)
		armor_cnt += 1;

	return(armor_cnt);
}

float armor_calc(entity targ,float damage)
{
	float total_armor_protection;
	float armor_cnt;
	float armor_damage;
	float perpiece;
	float curr_damage,armor_damage;

	total_armor_protection = 0;

	if (targ.armor_amulet)
		total_armor_protection += ClassArmorProtection[targ.playerclass - 1]; 

	if (targ.armor_bracer)
		total_armor_protection += ClassArmorProtection[targ.playerclass - 1 + 1];

	if (targ.armor_breastplate)
		total_armor_protection += ClassArmorProtection[targ.playerclass - 1 + 2];

	if (targ.armor_helmet)
		total_armor_protection += ClassArmorProtection[targ.playerclass - 1 + 3];

	total_armor_protection += targ.level * .001;

	armor_cnt = armor_inv(targ);

	if (armor_cnt) // There is armor
	{
		armor_damage = total_armor_protection * damage;

		// Damage is greater than all the armor
		if (armor_damage > (targ.armor_amulet + targ.armor_bracer + 
				targ.armor_breastplate + targ.armor_helmet))
		{
			targ.armor_amulet		= 0;
			targ.armor_bracer		= 0;
			targ.armor_breastplate	= 0;
			targ.armor_helmet		= 0;
		}	
		else			// Damage the armor
		{
			curr_damage = armor_damage;
			// FIXME: Commented out the loop for E3 because of a runaway loop message
		//	while (curr_damage>0)
		//	{
				armor_cnt = armor_inv(targ);

				perpiece = curr_damage / armor_cnt;

				if ((targ.armor_amulet) && (curr_damage))
				{
					targ.armor_amulet -= perpiece;	
					curr_damage -= perpiece;
					if (targ.armor_amulet < 0)
					{
						curr_damage -= targ.armor_amulet;
						targ.armor_amulet = 0;
					}	

					if (targ.armor_amulet < 1)
						targ.armor_amulet = 0;
				}				

				if ((targ.armor_bracer) && (curr_damage))
				{
					targ.armor_bracer -= perpiece;	
					curr_damage -= perpiece;
					if (targ.armor_bracer < 0)
					{
						curr_damage -= targ.armor_bracer;
						targ.armor_bracer = 0;
					}	
					
					if (targ.armor_bracer < 1)
						targ.armor_bracer = 0;
				}				

				if ((targ.armor_breastplate) && (curr_damage))
				{
					targ.armor_breastplate -= perpiece;	
					curr_damage -= perpiece;
					if (targ.armor_breastplate < 0)
					{
						curr_damage -= targ.armor_breastplate;
						targ.armor_breastplate = 0;
					}	
					
					if (targ.armor_breastplate < 1)
						targ.armor_breastplate = 0;
				}				

				if ((targ.armor_helmet) && (curr_damage))
				{
					targ.armor_helmet -= perpiece;	
					curr_damage -= perpiece;
					if (targ.armor_helmet < 0)
					{
						curr_damage -= targ.armor_helmet;
						targ.armor_helmet = 0;
					}	

					if (targ.armor_helmet < 1)
						targ.armor_helmet = 0;
				}	

		//	}
		}
	}
	else
		armor_damage =0;

	return(armor_damage);
}

/*
============
T_Damage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
============
*/
void(entity targ, entity inflictor, entity attacker, float damage) T_Damage=
{
vector	dir;
entity	oldself;
float	save;
float	total_damage,do_mod;
float armor_damage;
entity holdent;

	if (!targ.takedamage)
		return;

	if(targ.invincible_time>time)
	{
		sound(targ,CHAN_ITEM,"misc/pulse.wav",1,ATTN_NORM);
		return;
	}

	if(targ!=attacker)
		if (targ.deathtype != "teledeath"&&targ.deathtype != "teledeath2"&&targ.deathtype != "teledeath3"&&targ.deathtype != "teledeath4")
		{
			if(coop&&teamplay&&attacker.classname=="player"&&targ.classname=="player")
			return;

			if(teamplay)
				if(attacker.classname=="player"&&targ.classname=="player")
					if(attacker.team==targ.team)
						return;
		}

	if (targ.flags & FL_GODMODE)
		return;

	if(targ.classname=="monster_mezzoman")
	{
		if(inflictor.flags2&FL_NODAMAGE)
		{
			inflictor.flags2(-)FL_NODAMAGE;
			if(random()<0.3)
				CreateSpark (inflictor.origin);
			return;
		}

		if(targ.movechain.classname=="mezzo reflect")
			if(infront_of_ent(inflictor,targ))
			{
				sound(targ,CHAN_AUTO,"mezzo/slam.wav",1,ATTN_NORM);
				makevectors(targ.angles);
				if(random()<0.1)
					CreateSpark(targ.origin+targ.view_ofs+v_forward*12);
				else if(random()<0.7)
					particle4(targ.origin+targ.view_ofs+v_forward*12,random(5,15),256 + (8 * 15),PARTICLETYPE_FASTGRAV,2 * damage);
				return;
			}
	}

	// Nothing but melee weapons hurt the snake
//	if ((targ.classname == "monster_snake") && 
//		((!inflictor.classname == "player") || (!attacker.classname == "player")))
//		return;

	if(targ.health<=0)
	{
		targ.health=targ.health-damage;//Keep taking damage while dying, if enough, may gib in mid-death
		return;
	}

//Damage modifiers
// used by buttons and triggers to set activator for target firing
	damage_attacker = attacker;

	if(attacker.flags&FL_CLIENT&&attacker==inflictor)
	{//Damage mod for strength using melee weaps
		if(attacker.weapon==IT_WEAPON1)
		{
			if(attacker.playerclass==CLASS_CRUSADER)
			{
				if(!attacker.artifact_active&ART_TOMEOFPOWER)
					do_mod=TRUE;
			}
			else
				do_mod=TRUE;
		}
		else if(attacker.playerclass==CLASS_PALADIN)
			if(attacker.weapon==IT_WEAPON2&&!attacker.artifact_active&ART_TOMEOFPOWER)
				do_mod=TRUE;
		if(do_mod)
		{
			do_mod = attacker.strength - 11;
			damage+=damage*do_mod/30;//from .84 - 1.23
		}
	}

	if(targ.flags&FL_MONSTER&&inflictor.flags2&FL2_ADJUST_MON_DAM)
		damage*=2;//Special- more damage against monsters

	if (attacker.super_damage)
		damage += attacker.super_damage * damage;

	// Calculating Damage to a player
	if (targ.classname == "player")
	{	// How much armor does he have
		armor_damage = armor_calc(targ,damage);

		// What hits player
		total_damage = damage - armor_damage;
	}
	else
		total_damage = damage;

// add to the damage total for clients, which will be sent as a single
// message at the end of the frame
// FIXME: remove after combining shotgun blasts?
	if (targ.flags & FL_CLIENT)
	{
		targ.dmg_take = targ.dmg_take + total_damage;
		targ.dmg_save = targ.dmg_save + save;
		targ.dmg_inflictor = inflictor;
	}

// figure momentum add
	if ( (inflictor != world) && (targ.movetype == MOVETYPE_WALK) )
	{
		dir = targ.origin - (inflictor.absmin + inflictor.absmax) * 0.5;
		dir = normalize(dir);
		targ.velocity = targ.velocity + dir*damage*8;
	}

// check for godmode or invincibility
// do the damage
	targ.health = targ.health - total_damage;

	if(targ.health>=0&&targ.health<1.0000)//No more Zombie mode!!! (Sorry!)
		targ.health=-0.1;

	if (targ.health <=0 && targ.classname == "player" && targ.cnt_sh_boost)
	{
		if (deathmatch || skill == 0)	// Only in deatmatch or easy mode
		{
			holdent = self;
			self = targ;
			use_super_healthboost();
			centerprint(self,"Saved by the Mystic Urn!\n");
			stuffcmd(self,"bf\n");
			sound (self, CHAN_AUTO, "misc/comm.wav", 1, ATTN_NORM);
			self.deathtype="";
			self = holdent;
			return;
		}
	}

	// Check to see if divine intervention took place			
	if ((targ.health <= 0) && (targ.classname == "player") && (targ.playerclass == CLASS_PALADIN))
	{
		holdent = self;
		self = targ;	
		if (Pal_DivineIntervention())
		{
			self.deathtype="";
			self = holdent;
			return;
		}
		self = holdent;
	}


	if (targ.health <= 0)
	{
		if(attacker.controller.classname=="player")
		{//Proper frag credit to controller of summoned stuff
			inflictor=attacker;
			attacker=attacker.controller;
		}
		targ.th_pain=SUB_Null;	//Should prevents interruption of death sequence
		Killed (targ, attacker,inflictor);
		return;
	}

// react to the damage
	oldself = self;
	self = targ;

// barrels need sliding information
	if (self.classname == "barrel")
	{
		self.enemy = inflictor;
		self.count = damage;
	}
	else if (self.classname == "catapult")
		self.enemy = inflictor;
	else if(self.classname=="player")
		self.enemy = attacker;

	if ( (self.flags & FL_MONSTER) && attacker != world && !(attacker.flags & FL_NOTARGET)&&attacker!=self.controller&&(attacker.controller!=self.controller||attacker.controller==world))
	{	// Monster's shouldn't attack each other (kin don't shoot kin)
		if (self != attacker && attacker != self.enemy&&(self.enemy.classname!="player"||attacker.classname=="player"||attacker.controller.classname=="player"))// && attacker.flags & FL_CLIENT)
		{
			if (self.classname != attacker.classname||random(100)<=5) //5% chance they'll turn on selves
			{
				if((self.model=="models/spider.mdl"||self.model=="models/scorpion.mdl")&&attacker.model==self.model)
				{
					if(random()<0.3)
						monster_pissed(attacker);
				}
				else
					monster_pissed(attacker);
			}
		}
	}

	if (self.th_pain)
	{
		if(self.classname=="player"&&self.model!="models/sheep.mdl")
			player_pain();
		else 
			self.th_pain (attacker, total_damage);
	// nightmare mode monsters don't go into pain frames often
		if (skill == 3)
			self.pain_finished = time + 5;		
	}

	self = oldself;
};

/*
============
T_RadiusDamage
============
*/
//void(entity loser)SpawnFlameOn;
void(entity inflictor, entity attacker, float damage, entity ignore) T_RadiusDamage =
{
float 	points,inertia,radius;
entity	head;
vector	inflictor_org, org;

//FIXME:  If too many radius damage effects go off at the same time, it crashes in a loop
//			This usually occurs when object whose death is radius damage destoy
//			other objects with a radius damage death (namely: exploding barrels)

	inflictor_org = (inflictor.absmin+inflictor.absmax)*0.5;
	if(inflictor.classname=="circfire")
		radius=150;
	else
		radius=damage+40;
	head = findradius(inflictor_org, radius);

	if(inflictor.classname=="fireballblast")
		damage+=attacker.level*33;
	
	while (head)
	{
		if (head != ignore&&head!=inflictor)// && head!=inflictor.owner)
		{
			if (head.takedamage)
			{
				org = (head.absmax + head.absmin)*0.5;
				points = 0.5*vlen (inflictor_org - org);
				if (points < 0)
					points = 0;
				points = damage - points;
				if (head == attacker)
					if(attacker.classname=="monster_eidolon"||attacker.playerclass==CLASS_NECROMANCER)//Necromancer takes no radius damage from his own magic
						points = 0;
					else if(inflictor.model=="models/assgren.mdl")//Some more resistance to the Assassin's own Big One
						points*=0.25;
					else
						points*=0.5;

		//following stops multiple grenades from blowing each other up
				if(head.owner==inflictor.owner&&
					head.classname==inflictor.classname&&
					(head.classname=="stickmine"||head.classname=="tripwire"))
					points=0;
				if((inflictor.classname=="snowball"||inflictor.classname=="blizzard")&&head.frozen>0)
					points=0;
				if (points > 0)
				{
					if (CanDamage (head, inflictor))
					{
						if(other.movetype!=MOVETYPE_PUSH)
						{
							if (head.mass<=10)
								inertia = 1;
							else if(head.mass<=100)
								inertia = head.mass/10;
							else 
								inertia = head.mass;
		                    head.velocity=head.velocity+normalize(org-inflictor_org)*(points*10/inertia);
			                head.flags(-)FL_ONGROUND;
						}
						if(inflictor.classname=="fireballblast")
						{
							if(points>10||points<5)
								points=random(5,10);

							if(head.flags&FL_FIREHEAL)
							{
								if(head.health+points<=head.max_health)
									head.health=head.health+points;
								else
									head.health=head.max_health;
							}
							else if(!head.flags&FL_FIRERESIST)
							{
								if(head.health<=points)
									points=1000;
								T_Damage (head, inflictor, attacker, points);
							}
						}
						else
							T_Damage (head, inflictor, attacker, points);
					}
				}
			}
		}
		head = head.chain;
	}
};

/*
============
T_RadiusDamageWater
============
*/

void(entity inflictor, entity attacker, float dam, entity ignore) T_RadiusDamageWater =
{
        local   float   points;
        local   entity  head;
	local	vector	org;

    head = findradius(inflictor.origin, dam);
	
	while (head)
	{
        if (head != ignore)
		{
			if (head.takedamage)
			{
				if (pointcontents(head.origin) == CONTENT_WATER || pointcontents(head.origin) == CONTENT_SLIME) //  visible(inflictor)?
				{
					if (head.classname == "player" && head != attacker)
						head.enemy = attacker;
					org = head.origin + (head.mins + head.maxs)*0.5;
					points = 0.25 * vlen (inflictor.origin - org);
					if (points <= 64)
						points = 1;
					points = dam/points;
					if (points < 1||(self.classname=="mjolnir"&&head==self.controller)||head.classname=="monster_hydra")
						points = 0;
					if (points > 0)
					{
						head.deathtype="zap";
						spawnshockball((head.absmax+head.absmin)*0.5);
						T_Damage (head, inflictor, attacker, points);
//Bubbles if dead?
                    }
				}
			}
		}
		head = head.chain;
	}
};

/*
============
T_BeamDamage
============
*/
/*
void(entity attacker, float damage) T_BeamDamage =
{
	local	float 	points;
	local	entity	head;
	
	head = findradius(attacker.origin, damage+40);
	
	while (head)
	{
		if (head.takedamage)
		{
			points = 0.5*vlen (attacker.origin - head.origin);
			if (points < 0)
				points = 0;
			points = damage - points;
			if (head == attacker)
				points = points * 0.5;
			if (points > 0)
			{
				if (CanDamage (head, attacker))
					T_Damage (head, attacker, attacker, points);
			}
		}
		head = head.chain;
	}
};
*/
/*
============
T_RadiusManaDamage
============
*/
/*
void(entity inflictor, entity attacker, float manadamage, entity ignore) T_RadiusManaDamage =
{
	local	float 	points;
	local	entity	head;
	local	vector	org;

	head = findradius(inflictor.origin, manadamage);
	
	while (head)
	{
		if (head != ignore)
		{
			if ((head.takedamage) && (head.classname=="player"))
			{
				org = head.origin + (head.mins + head.maxs)*0.5;
				points = 0.5 * vlen (inflictor.origin - org);
				if (points < 0)
					points = 0;
				points = manadamage - points;
				if (head == attacker)
					points = points * 0.5;
				if (points > 0)
				{
					if (CanDamage (head, inflictor))
					{	
					   head.bluemana = head.bluemana - points;
						if (head.bluemana<0)
							head.bluemana=0;
					   head.greenmana = head.greenmana - points;
						if (head.greenmana<0)
							head.greenmana=0;
					}
				}
			}
		}
		head = head.chain;
	}
};
*/

