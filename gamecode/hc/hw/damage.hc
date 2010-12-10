/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/damage.hc,v 1.2 2007-02-07 16:57:52 sezero Exp $
 */

void() T_MissileTouch;
void() info_player_start;
void necromancer_sphere(entity ent);
void crusader_sphere(entity ent);

void() monster_death_use;
void(float damage, vector dir)PlayerDie;
void MonsterDropStuff(void);
void Use_TeleportCoin(void);
void UseInvincibility(void);
void Use_TomeofPower(void);
void use_super_healthboost();

float hitpoint_table[25] =
{
	70,		85,				// Paladin
	8,		13,      4,

	65,		75,				// Crusader
	5,		10,      3,

	65,		75,				// Necromancer
	5,		10,      3,

	65,		75,				// Assassin
	5,		10,      3,

	65,		75,				// Succubus
	5,		10,      3
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

float CheckExpAward (entity attacker,entity targ,float fatality,float damage)
{
float exp_bonus,health_mod,exp_base,init_exp_mult,damage_mod;
entity lastleader,newking;
//string printnum;
	if(attacker.classname!="player")
	{
//		dprint("Attacker not a player!\n");
		return FALSE;
	}

	if(attacker.deadflag>=DEAD_DYING)
	{
//		dprint("Attacker dead!\n");
		return FALSE;
	}

	if(!attacker.max_health)
	{
		dprint("ERROR- attacker without max health!\n");
		attacker.max_health = (hitpoint_table[(attacker.playerclass - 1) * 5] + hitpoint_table[(attacker.playerclass - 1) * 5])*0.5;
	}
	
	if(!targ.max_health)
	{
		if(targ.classname=="player")
		{
			dprint("ERROR- target without max health!\n");
			targ.max_health = (hitpoint_table[(targ.playerclass - 1) * 5] + hitpoint_table[(targ.playerclass - 1) * 5])*0.5;
		}
		else//something's FUBAR
			return FALSE;
	}

	//NOTE: exp_mult is for DM only
	init_exp_mult=exp_mult;
	health_mod=1;
	if(deathmatch)
	{
		if(attacker.artifact_active&ART_INVINCIBILITY)
			health_mod=0.25;//CHEAP!!!
		else if(attacker.health>attacker.max_health)
		{
			health_mod=attacker.max_health/attacker.health;
			if(health_mod<0.5)
				health_mod=0.5;
		}
		else if(attacker.health<attacker.max_health*0.5)
		{
			health_mod=((attacker.max_health*0.5)/attacker.health)*0.1;
			if(health_mod<1)
				health_mod=1;
			else if(health_mod>3.3)
				health_mod=3.3;
		}
	}
	exp_mult*=health_mod;

	if(targ.classname=="player")
	{
		if(targ.artifact_active&ARTFLAG_FROZEN)//if targ frozen, set exp_mult and health_mod to 1?
			exp_mult=health_mod=1;

/*
		bprint(PRINT_HIGH,"damage = ");
		printnum=ftos(damage);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n health = ");
		printnum=ftos(targ.health);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n max_health = ");
		printnum=ftos(targ.max_health);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n exp_mult = ");
		printnum=ftos(init_exp_mult);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n health_mod = ");
		printnum=ftos(health_mod);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n base_exp = ");
		printnum=ftos(targ.level*800 - 500);
		bprint(PRINT_HIGH,printnum);
		*/

		if(damage>targ.max_health)
			damage=targ.max_health;
		targ.experience_value=(targ.level*700 - 300)*exp_mult;
		if(fatality)
		{
			damage_mod = damage/targ.max_health*2;//2 times remainder of health for a kill
			if(damage_mod>2)
			{
				dprintf("WHOA!  Way too much exp: %s!!!\n",damage_mod);
				damage_mod=2;
			}
		}
		else
		{
			damage_mod = damage/targ.max_health*0.5;//1/2 ratio of damage to max health for injury
			if(damage_mod>1)
			{
				dprintf("WHOA!  Way too much exp: %s!!!\n",damage_mod);
				damage_mod=1;
			}
		}
		targ.experience_value*=damage_mod;
		exp_base=targ.experience_value;
		/*
		bprint(PRINT_HIGH,"\n exp_val = ");
		printnum=ftos(exp_base);
		bprint(PRINT_HIGH,printnum);
		bprint(PRINT_HIGH,"\n");
*/
	}
	else
	{
		if(fatality)
			exp_base=targ.experience_value;//remainder of health
		else
			exp_base=targ.init_exp_val*(damage/targ.max_health*0.5);//give appropriate exp for % of damage to max_health, divided by 2
	}

	if(exp_base<=0)
	{
//		dprint("target has no exp_val\n");
		return FALSE;
	}

	if(attacker.model=="models/sheep.mdl"&&attacker.flags&FL_CLIENT&&attacker!=self)
	{//3000 exp bonus for killing as sheep.
		if(fatality)
		{
			sound (attacker, CHAN_BODY, "misc/comm.wav", 1, ATTN_NORM);
			centerprint(attacker,"Sheep kill BONUS!!!\n");
			exp_bonus=3000;
		}
	}

	if(deathmatch)
	{
		lastleader=FindExpLeader();//Find King of the Hill
		if(targ.classname=="player")//Exp gained is (level*800 - 500) * exp_mult
		{
			if(!fatality)
			{
				if((targ.classname=="player"&&teamplay&&attacker.team==self.team)||attacker==targ)//hit your own guy
				{
//					dprint("Attacker hit own teammate in DM!\n");
					return FALSE;
				}
				else
				{
//					dprint("Attacker valid player in DM!\n");
					return exp_base;//exp_bonus only for fatalities
				}
			}
			else
			{
				attacker.level_frags+=targ.level;//Level frags
				if(lastleader==targ&&attacker!=targ)//Killed King		
				{
					sound (attacker, CHAN_ITEM, "misc/comm.wav", 1, ATTN_STATIC);
					centerprint(attacker,"You took out the King of the Hill!\n");
					if(num_players>2)//Only give bonus if more than 2 players
						exp_bonus+=500*(num_players - 2);	//Give an extra 500* num players,you beat others to the kill
				}
			}
		}
		
		if((targ.classname=="player"&&teamplay&&attacker.team==targ.team)||attacker==targ)
		{
			if(!fatality)
			{//this is checked above too, but what the hay
//				dprint("Attacker hit own teammate in DM!\n");
				return FALSE;
			}
			else if(attacker==targ)//killed self- no extra penalty
				return FALSE;
			else
			{
//				sprint(attacker,PRINT_HIGH,"You killed a teammate!\n");
				drop_level(attacker,1);//Killed someone on your team, lose a level, get no exp
			}
		}
		else
		{
			if(targ.classname=="player"&&attacker.level<targ.level)//was attacker.level+2
			{
				if(fatality)
				{
//					sprint(targ,PRINT_HIGH,"You were killed by a lower level enemy!\n");
					drop_level(targ,1); //If player killed by a lower level player, lose 1 level (diff in levels must be 3 or more)
				}
			}
			if(attacker!=targ.controller)//No credit for killing your imp, or various dead things (eg. yer imp's head)!
			{
				if(!fatality)
				{
//					dprint("Attacker hit valid player in DM!\n");
					return exp_base;//exp_bonus is only for fatalities
				}
				else
					AwardExperience(attacker,targ,exp_base+exp_bonus);
			}
			else// if(!fatality)
			{
//				dprint("Attacker hit own ent in DM!\n");
				return FALSE;//Hit or killed your own ent in DM, no exp
			}
		}

		newking=FindExpLeader();
		if(newking!=lastleader)
		{//Tell everyone if the king of the hill has changed
			sound (world, CHAN_BODY, "misc/comm.wav", 1, ATTN_NONE);
			bprintname(PRINT_MEDIUM,newking);
			bprinti(PRINT_MEDIUM,STR_NEWKINGOHILL);
//RICK: ADD TO C
//			WriteByte(MSG_ALL, SVC_UPDATE_KINGOFHILL);
//			WriteEntity (MSG_ALL, newking);
		}
	}
	else if(targ.classname=="player"&&coop&&teamplay)//&&attacker.team==targ.team)
	{
		if(!fatality)
		{
//			dprint("Attacker hit own teammate in coop\n");
			return FALSE;
		}
		else
		{
//			sprint(attacker,PRINT_HIGH,"You killed a coop buddy!\n");
			drop_level(attacker,1);	//Killed friend in coop, lose a level
		}
	}
	else if(attacker!=targ.controller&&(targ.monsterclass<CLASS_BOSS||targ.classname=="obj_chaos_orb"))//Bosses award Exp themselves, to all players in coop
	{
		if(!fatality)
		{
//			dprint("Attacker hit valid exp targ\n");
			return exp_base;//exp_bonus only for fatalities
		}
		else
			AwardExperience(attacker,targ,exp_base+exp_bonus);
	}
	return TRUE;
}

void poison_think ()
{
	self.enemy.deathtype="poison";
	T_Damage (self.enemy, self, self.owner, 1 );
	if(self.enemy.flags&FL_CLIENT)
		stuffcmd(self.enemy,"bf\n");
	if(self.lifetime<time||self.enemy.health<=0||(!self.enemy.flags2&FL2_POISONED))
	{
		self.enemy.flags2(-)FL2_POISONED;
		self.think=SUB_Remove;
	}
	thinktime self : 1;
}

void spawn_poison(entity loser,entity killer,float poison_length)
{
entity poison_ent;
	loser.flags2(+)FL2_POISONED;
	poison_ent=spawn();
	poison_ent.think=poison_think;
	poison_ent.enemy=loser;
	poison_ent.owner=killer;

	thinktime poison_ent : 0.05;
	poison_ent.lifetime=time+poison_length;
}

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
void(entity targ, entity attacker, entity inflictor, float damage, vector dir) Killed =
{
entity oself;
float was_alive;
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

	was_alive = self.flags2&FL_ALIVE;
	self.flags2(-)FL_ALIVE;
	self.touch = self.th_pain = SUB_Null;

	if (attacker.classname == "player")
	{
		if ((attacker.playerclass==CLASS_NECROMANCER) && (attacker.level >= 3))
		{
			if ((targ.flags & FL_MONSTER) || (targ.flags & FL_CLIENT))
				necromancer_sphere (attacker);
		}
		else if ((attacker.playerclass==CLASS_CRUSADER) && (attacker.level >= 3))
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
	{
		print_ent1=attacker;
		print_ent2=self;
		ClientObituary(self, attacker, inflictor);
		print_ent1=print_ent2=world;
	}

	if(!fixedLevel&&was_alive)
		if(((pointcontents(self.origin)==CONTENT_LAVA&&attacker==world)||self.artifact_active&ARTFLAG_FROZEN)&&self.credit_enemy!=world)
			CheckExpAward(self.credit_enemy,self,TRUE,damage);
		else
			CheckExpAward(attacker,self,TRUE,damage);
	self.credit_enemy=world;
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
			PlayerDie(damage, dir);	
		else if (self.th_die)
			self.th_die ();

		self = oself;
	}
	else
	{
		if(self.classname=="player")
		{
			PlayerDie(damage, dir);
			//self.think=PlayerDie;
		}
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
float	total_damage,do_mod,hurt_exp_award;
float armor_damage,alt_award;
entity holdent,lastleader,newking;

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

//Credit for certain types of kills, like freezing or pushing off ledges into lava, etc.
/*
	if(targ.classname=="player")
		if(targ.credit_enemy!=world&&targ.credit_enemy!=attacker)
			if((attacker!=world||pointcontents(targ.origin)!=CONTENT_LAVA)&&(!targ.artifact_active&ARTFLAG_FROZEN))
				targ.credit_enemy=world;//was hit by someone else or not taking lava damage, and not frozen, so clear credit_enemy
*/
//Damage modifiers
// used by buttons and triggers to set activator for target firing


	if(damageScale > 0)
	{
		damage *= damageScale;
	}

	if (meleeDamScale > 0)
	{
		if (targ.weapon==IT_WEAPON1)
		{
			damage *= meleeDamScale;
		}
	}
	
	if((dmMode == DM_CAPTURE_THE_TOKEN)&&(attacker.gameFlags & GF_HAS_TOKEN))
	{
		damage *= 2.0;
	}
	else if((dmMode == DM_CAPTURE_THE_TOKEN)&&(targ.gameFlags & GF_HAS_TOKEN))
	{
		damage *= .5;
	}

/*	if(attacker.classname=="player"&&deathmatch)
	{	// adjust damage based on the level of the player
		damage *= (1.0 + (attacker.level - 1)*0.14);
	}*/

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
		if(inflictor.classname=="minigrenade"&&targ.health>-40)
			targ.health=-40+random(-1,-20);//Ensure gib deathtype

		if(attacker.controller.classname=="player")
		{//Proper frag credit to controller of summoned stuff
			inflictor=attacker;
			attacker=attacker.controller;
		}
		targ.th_pain=SUB_Null;	//Should prevents interruption of death sequence
		Killed (targ, attacker,inflictor, total_damage, targ.origin - attacker.origin);
		return;
	}

// react to the damage
	oldself = self;
	self = targ;

// barrels need sliding information
	if(!fixedLevel&&self.flags2&FL_ALIVE)
	{
		lastleader=FindExpLeader();
		if(((pointcontents(self.origin)==CONTENT_LAVA&&attacker==world)||self.artifact_active&ARTFLAG_FROZEN)&&self.credit_enemy!=world)
			alt_award=TRUE;
		if(alt_award)
			hurt_exp_award=CheckExpAward(self.credit_enemy,self,FALSE,total_damage);
		else
			hurt_exp_award=CheckExpAward(attacker,self,FALSE,total_damage);
		if(hurt_exp_award>0)
		{
			if(alt_award)
				AwardExperience(self.credit_enemy,self,hurt_exp_award);
			else
				AwardExperience(attacker,self,hurt_exp_award);
			if(deathmatch)
			{
				newking=FindExpLeader();
				if(newking!=lastleader)
				{//Tell everyone if the king of the hill has changed
					sound (world, CHAN_BODY, "misc/comm.wav", 1, ATTN_NONE);
					bprintname(PRINT_MEDIUM,newking);
					bprinti(PRINT_MEDIUM,STR_NEWKINGOHILL);
//RICK: ADD TO C
//					WriteByte(MSG_ALL, SVC_UPDATE_KINGOFHILL);
//					WriteEntity (MSG_ALL, newking);
				}
			}
			if(self.classname!="player")
				self.experience_value-=hurt_exp_award;
		}
	}

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
			player_pain(attacker,total_damage);
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

	if(inflictor.flags&FL_CLIENT)//more damage up top
		inflictor_org = inflictor.origin+inflictor.view_ofs;
	else
		inflictor_org = (inflictor.absmin+inflictor.absmax)*0.5;
	if(inflictor.classname=="circfire")
		radius=150;
	else if(inflictor.classname=="poison grenade")
		radius=200;//FIXME- greater distance above...
	else
		radius=damage+40;

	if (inflictor.classname == "pincer" && radius > 200)
	{
		radius = 200;
	}

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
				if(inflictor.classname=="poison grenade")
				{
					if(head.flags2&FL_ALIVE&&head.thingtype==THINGTYPE_FLESH)
						points=0;
					else
						points=damage;
				}
				else
					points = 0.5*vlen (inflictor_org - org);
				if (points < 0)
					points = 0;
				points = damage - points;
				if (head == attacker)
					if(attacker.classname=="monster_eidolon"||attacker.playerclass==CLASS_NECROMANCER)//Necromancer takes no radius damage from his own magic
						points = 0;
					else if(inflictor.model=="models/assgren.mdl")//Some more resistance to the Assassin's own Big One
						points*=0.5;
					else
						points*=0.5;

		//following stops multiple grenades from blowing each other up
				if(head.owner==inflictor.owner&&
					head.classname==inflictor.classname&&
					(head.classname=="stickmine"||head.classname=="tripwire"||head.classname=="proximity"))
					points=0;
				if((inflictor.classname=="snowball"||inflictor.classname=="blizzard")&&head.frozen>0)
					points=0;
				if (points > 0)
				{
					if (CanDamage (head, inflictor)||inflictor.classname=="fireballblast")
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
							if(head.velocity_z<30&&head.flags&FL_ONGROUND)
								head.velocity_z=30;
			                head.flags(-)FL_ONGROUND;
						}

						if(inflictor.classname=="poison grenade")
						{
							if(!head.flags2&FL2_POISONED)
							{//Poison them
								if(head.flags&FL_CLIENT&&(coop||teamplay==1))
								{
									if(head.team!=attacker.team&&!coop)
									{
										centerprint(head,"You have been poisoned!\n");
										spawn_poison(head,attacker,random(10,20));
									}
								}
								else
									spawn_poison(head,attacker,random(10,20));
							}
						}


						if(inflictor.classname=="fireballblast")
						{
							if(points>10||points<5)
								points=random(5,10);

							if(head.health<=points)
								points=1000;
							T_Damage (head, inflictor, attacker, points);
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
float   points;
entity  head;
vector	org;

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

