/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/damage.hc,v 1.1 2005-01-26 17:26:10 sezero Exp $
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

float hitpoint_table[30] =
{
	70,		85,				// Paladin
	0,		0,      0,

	65,		75,				// Crusader
	0,		0,      0,

	65,		75,				// Necromancer
	0,		0,      0,

	65,		75,				// Assassin
	0,		0,      0,

	65,		75,				// Succubus
	0,		0,      0,

	90,		100,			// Dwarf
	0,		0,      0
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

float flammable (entity toast)
{
	if(toast.health>0)
		if(toast.thingtype==THINGTYPE_WOOD||
			toast.thingtype==THINGTYPE_FLESH||
			toast.thingtype==THINGTYPE_LEAVES||
			toast.thingtype==THINGTYPE_HAY||
			toast.thingtype==THINGTYPE_CLOTH||
			toast.thingtype==THINGTYPE_WOOD_LEAF||
			toast.thingtype==THINGTYPE_WOOD_METAL||
			toast.thingtype==THINGTYPE_WOOD_STONE||
			toast.thingtype==THINGTYPE_METAL_CLOTH||
			toast.thingtype==THINGTYPE_WEBS)
				return TRUE;
	return FALSE;
}

void burner_think ()
{
	if(self.lifetime<time||self.enemy.health<0||self.enemy.fire_damage<=0)
	{
		stopSound(self.enemy,CHAN_BODY);
		//sound(self.enemy,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		self.enemy.effects (-) EF_ONFIRE;
		remove(self);
		return;
	}
	else if(pointcontents(self.enemy.origin)==CONTENT_WATER&&!self.frags)
	{
		sound (self.enemy, CHAN_BODY, "misc/fout.wav", 1, ATTN_NORM);
		smolder((self.enemy.absmin+self.enemy.absmax)*0.5);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		self.enemy.effects (-) EF_ONFIRE;
		remove(self);
		return;
	}
	else
	{
		//org=(self.enemy.absmin+self.enemy.absmax)*0.5+randomv(self.enemy.size*-0.25,self.enemy.size*0.25);
		//vel=randomv('-3 -3 0','3 3 7'); 

		//starteffect(CE_ONFIRE, org,vel, 0);
		//sound(self.enemy,CHAN_BODY,"raven/fire1.wav",1,ATTN_NORM);

		thinktime self : random(0.5);
		if(self.enemy.playerclass==CLASS_SUCCUBUS)
		{//healed by fire!
			self.enemy.health+=(self.enemy.fire_damage+ random(1))/4;
			if(self.enemy.health>self.enemy.max_health)
				self.enemy.health=self.enemy.max_health;
		}
		else
		{
			self.enemy.deathtype="burn";
			T_Damage(self.enemy,self,self.owner,self.enemy.fire_damage + random(1));
			self.enemy.deathtype="";
		}
	}
}

void spawn_burner (entity loser,float gfire)
{
	if (loser.flags2 & FL2_ONFIRE)
	{
		loser.fire_damage += 2;
		return;
	}

	if (coop && loser.classname == "player" && loser.team == self.owner.team && teamplay)
		return;

	if(loser.netname=="obj_barrel_gfire")
	{
		if(loser.frame==0)
		{//start up a greek fire barrel
		entity oself;
			oself=self;
			self=loser;
			loser.use();
			self=oself;
		}
		return;
	}

	loser.fire_damage = 2;

	entity burner;
	burner=spawn();
	burner.owner=self.owner;
	burner.enemy=loser;
	burner.think=burner_think;
	burner.effects (+) EF_NODRAW;
	burner.enemy.effects (+) EF_DIMLIGHT;
	thinktime burner : 0;
	burner.enemy.flags2 (+) FL2_ONFIRE;
	burner.enemy.effects (+) EF_ONFIRE;
	if(gfire)
	{
		burner.frags=TRUE;//Water has no effect!  Yer screwed, pal!
		burner.lifetime=time+random(10)+20;
	}
	else if(self.classname=="flarrow")
		burner.lifetime=time+random(5)+5;
	else
		burner.lifetime=time+random(3)+3;
//	if(burner.enemy.thingtype!=THINGTYPE_FLESH)
//		burner.enemy.fire_damage/=2;
	sound (burner.enemy, CHAN_AUTO, "misc/combust.wav", 1, ATTN_NORM);
}

void poison_think ()
{
	self.enemy.deathtype="poison";
	T_Damage (self.enemy, self, self.owner, 1 );
	self.enemy.deathtype="";
//was crashing, maybe another ent had equivalent flag or client got removed...
//	if(self.enemy.flags&FL_CLIENT)
//		stuffcmd(self.enemy,"bf\n");
	if(self.lifetime<time||self.enemy.health<=0||(!self.enemy.flags2&FL2_POISONED))
	{
		self.enemy.flags2(-)FL2_POISONED;
		PlayerSpeed_Calc (self.enemy);
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

float ClassArmorProtection[20] =
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
	.05,	// HELMET

	// Dwarf Armor Protection
	.00,	// AMULET
	.10,	// BRACER
	.50,	// BREASTPLATE
	.25		// HELMET
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
	if(self.flags2&FL2_EXCALIBUR)
	{
		WriteTeam (SVC_NODOC,self);
		WriteByte (MSG_ALL, SVC_MIDI_NAME);
		WriteString (MSG_ALL, "");//stops midi
		if(!gameover)
			centerprint_all_clients("The Defender of the Crown has been killed!\n",world);
		dprint(attacker.netname);
		dprint(" killed the defender of the crown\n");
	}

	self.flags2(-)FL_ALIVE|FL2_EXCALIBUR|FL2_HARDFALL;
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

	if(self.classname=="barrel")
		if(inflictor.classname=="balbolt")
			self.frags = TRUE;

	//Check for decapitation death
	self.movedir='0 0 0';
	if(attacker.beast_time>time)
		self.decap=FALSE;
	else if((attacker.flags2&FL2_EXCALIBUR&&self.classname=="player")||(attacker.classname=="monster_mezzoman"&&random()<0.3))
	{
		self.decap=TRUE;
		if(self.health<-20)
			self.health=-20;
	}
	else if(self.model!="models/sheep.mdl"&&self.deathtype!="teledeath"&&self.deathtype!="teledeath2"&&self.deathtype!="teledeath3"&&self.deathtype != "teledeath4")
		if
		(inflictor.classname=="ax_blade"||
			(inflictor.classname=="player"&&
				(
//					(attacker.playerclass==CLASS_ASSASSIN&&attacker.weapon==IT_WEAPON1)||
					(attacker.playerclass==CLASS_PALADIN&&attacker.weapon!=IT_WEAPON3)||
					(attacker.playerclass==CLASS_NECROMANCER&&attacker.weapon==IT_WEAPON1)||
					(attacker.playerclass==CLASS_SUCCUBUS&&attacker.weapon==IT_WEAPON2)||
					(attacker.playerclass==CLASS_DWARF&&attacker.view_ofs_z>=targ.proj_ofs_z)
				)
			)
		)
			if(attacker.playerclass==CLASS_DWARF&&attacker.weapon==IT_WEAPON3)
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
	else if(self.th_die==SUB_Null)
		if(self.target)
			SUB_UseTargets();

	if(self.classname=="player"&&self.puzzle_inv1!="")
	{
		self.puzzle_id=self.puzzle_inv1;
		DropPuzzlePiece(FALSE);
		self.puzzle_inv1=self.puzzle_id="";
	}

	self.th_stand=self.th_walk=self.th_run=self.th_pain=self.oldthink=self.think=self.th_melee=self.th_missile=SUB_Null;
	
	if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
		DeathBubbles(20);

//	if(attacker.classname=="rider_death")
//		spawn_ghost(attacker);

	self.beast_time=0;
	if(self.classname=="player_sheep")
		self.enemy=attacker;

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
	if(attacker.siege_team==self.siege_team)
		return;

	if(self.controller.classname=="player")
	{//Summoned/controlled monsters
		if(coop)
			if(attacker.classname=="player")
				return;

		if(deathmatch&&teamplay)
			if((attacker.team==self.controller.team)||(attacker.siege_team==self.controller.siege_team))
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

void MeleeSpark (entity ent1,entity ent2)
{
vector p1,p2;
float r;
	p1=ent1.origin+ent2.proj_ofs;
	p2=ent2.origin+ent2.proj_ofs;
	p1_x=(p1_x+p2_x)*0.5;
	p1_y=(p1_y+p2_y)*0.5;
	p1_z=(p1_z+p2_z)*0.5;

	r=random();
	if(r<0.33)
	{
		ent1.effects(+)EF_MUZZLEFLASH;
		sound (ent1, CHAN_AUTO, "weapons/met2met.wav", 1, ATTN_NORM);
	}
	else if(r<0.66)
	{
		ent2.effects(+)EF_MUZZLEFLASH;
		sound(ent2,CHAN_AUTO,"weapons/hitwall.wav",0.5,ATTN_NORM);
	}
	else
	{
		ent1.effects(+)EF_MUZZLEFLASH;
		sound(ent1,CHAN_AUTO,"paladin/axric1.wav",0.75,ATTN_NORM);
	}

	r=random();
	if(r<0.33)
		starteffect(CE_BLUESPARK,p1);
	else if(r<0.66)
		starteffect(CE_REDSPARK,p1);
	else
		starteffect(CE_YELLOWSPARK,p1);
}

float using_melee (entity who)
{
	switch(who.playerclass)
	{
	case CLASS_DWARF:
		return TRUE;
		break;
	case CLASS_PALADIN:
		if(who.weapon!=IT_WEAPON3)
			return TRUE;
		return FALSE;
		break;
	case CLASS_CRUSADER:
	case CLASS_NECROMANCER:
	case CLASS_ASSASSIN:
		if(who.weapon==IT_WEAPON1)
			return TRUE;
		return FALSE;
		break;
	case CLASS_SUCCUBUS:
		if(who.weapon==IT_WEAPON2)
			return TRUE;
		return FALSE;
		break;
	default:
		return FALSE;
		break;
	}
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
//float	save;
float	total_damage,do_mod;//,hurt_exp_award;
float armor_damage;//,alt_award;
entity holdent;//,lastleader,newking;
float wrath;

	if (!targ.takedamage)
		return;

	if(attacker.flags2&FL2_PARRIED)
	{
		attacker.flags2(-)FL2_PARRIED;
		return;
	}

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
					if((attacker.team==targ.team&&dmMode!=DM_SIEGE)||(attacker.siege_team==targ.siege_team&&dmMode==DM_SIEGE))
						return;//can't hurt pals
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


	if(attacker==inflictor&&attacker!=targ)//melee
		if(targ.last_attack+0.75>=time)
			if(using_melee(attacker))
				if(using_melee(targ))
					if(random()<0.3)
						if(fov(targ,attacker,20))
							if(fov(attacker,targ,20))
							{
								MeleeSpark(attacker,targ);
								targ.flags2(+)FL2_PARRIED;
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

	if(targ.flags2&FL2_EXCALIBUR)
		damage*=random(.2)+ 0.2;//20% - 40% damage to excalibur holder

/*	if(attacker.classname=="player"&&deathmatch)
	{	// adjust damage based on the level of the player
		damage *= (1.0 + (attacker.level - 1)*0.14);
	}*/

	damage_attacker = attacker;

	if(attacker.flags&FL_CLIENT&&attacker==inflictor&&attacker!=targ)
	{//Damage mod for strength using melee weaps
		if(
			(attacker.weapon==IT_WEAPON1&&attacker.playerclass!=CLASS_SUCCUBUS)||
			(attacker.weapon==IT_WEAPON2&&(attacker.playerclass==CLASS_PALADIN||attacker.playerclass==CLASS_DWARF))
		  )
		{
			if(attacker.playerclass==CLASS_CRUSADER)
			{
				if(!attacker.artifact_active&ART_TOMEOFPOWER)
					do_mod=TRUE;
			}
			else
				do_mod=TRUE;
		}
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
	if (targ.classname == "player"&&!targ.flags2&FL2_HARDFALL)
	{	// How much armor does he have
		if(inflictor.classname=="balbolt"&&targ.playerclass!=CLASS_DWARF)
			armor_damage = 0;
		else
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
//		targ.dmg_take = targ.dmg_take + total_damage;
//		targ.dmg_save = targ.dmg_save + save;
		targ.dmg_inflictor = inflictor;
	}

// figure momentum add
	if ( (inflictor != world) && (targ.movetype == MOVETYPE_WALK) )
	{
		dir = (targ.absmin + targ.absmax)*0.5 - (inflictor.absmin + inflictor.absmax) * 0.5;
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

	if(self.deathtype!="smitten")//nothing stops the REAL GOD (server)
	{
		// Check to see if divine intervention took place			
		if(attacker.playerclass==CLASS_CRUSADER&&attacker.weapon==IT_WEAPON8)//divine intervention
			wrath=TRUE;
		else
			wrath=FALSE;

		if(!wrath||(wrath&&random()<0.3&&targ==attacker))//30 % chance to allow possibility of saving caster from wrath, no others may be spared
			if ((targ.health <= 0) && (targ.classname == "player") && (targ.playerclass == CLASS_CRUSADER))
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
	}


	if(targ.classname=="monster_imp_lord")
		if(targ.controller)
			T_Damage(targ.controller,attacker,attacker,total_damage/3.33);

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
		if(attacker.flags&FL_CLIENT&&attacker.classname=="player")
			CheckExpAward(attacker,inflictor,targ,total_damage,TRUE);
		Killed (targ, attacker,inflictor, total_damage, targ.origin - attacker.origin);
		return;
	}
	if(attacker.flags&FL_CLIENT&&attacker.classname=="player")
		CheckExpAward(attacker,inflictor,targ,total_damage,FALSE);
	targ.deathtype="";

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
	{
		self.enemy = inflictor;
		if(attacker.siege_team==ST_ATTACKER)
			self.health+=total_damage;//can't take damage from attacker hits
	}
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

	if((self.classname=="door"||self.classname=="door_rotating")&&self.takedamage)
	{
//		if(!self.pain_target)
//			self.pain_target="rippler";
		breakable_hurt_use(attacker,total_damage);
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
					if(attacker.classname=="monster_eidolon")//Necromancer takes no radius damage from his own magic
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
						else if(inflictor.netname=="obj_barrel_gfire")
						{
							//if(points>10)
							if(flammable(head))
							{
								points/=2;//not so much radius damage
								if(!head.flags2&FL2_ONFIRE)
									spawn_burner(head,TRUE);
								else
									head.fire_damage += 2;//burn more!
							}
							else
								points=3;
						}
						else if(inflictor.classname=="fireballblast")
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
/*
 * $Log: not supported by cvs2svn $
 * 
 * 41    6/01/98 2:49a Mgummelt
 * 
 * 40    5/31/98 2:59p Mgummelt
 * 
 * 39    5/27/98 6:08p Mgummelt
 * 
 * 38    5/25/98 10:56p Mgummelt
 * Last version before send-off, v0.15
 * 
 * 37    5/25/98 1:38p Mgummelt
 * 
 * 36    5/25/98 1:30p Mgummelt
 * 
 * 35    5/07/98 11:19p Mgummelt
 * Almost ready for release- fix some dwarf-related compatibility problems
 * 
 * 34    5/06/98 11:10p Mgummelt
 * Last version before 1st beta
 * 
 * 33    5/05/98 8:33p Mgummelt
 * Added 6th playerclass for Siege only- Dwarf
 * 
 * 32    4/30/98 6:01p Mgummelt
 * 
 * 31    4/30/98 10:21a Mgummelt
 * Siege version 0.1 4/30/98
 * 
 * 30    4/27/98 4:34p Mgummelt
 * Siege beta version 0.13
 * 
 * 29    4/24/98 1:31a Mgummelt
 * Siege version 0.02 4/24/98 1:31 AM
 * 
 * 28    4/23/98 5:18p Mgummelt
 * Siege version 0.01 4/23/98
 * 
 * 27    4/17/98 9:00p Mgummelt
 * 1st version of Siege- loads but totally fucked
 * 
 * 25    4/13/98 1:13p Mgummelt
 * fixing push fields so they don't caus eplayer impact damage
 * 
 * 24    4/10/98 7:34p Mgummelt
 * Attempting to fix crazy super experience gain, capped level at 20, no
 * matter what
 * 
 * 23    4/10/98 12:04p Ssengele
 * limit radius for untomed scarab staff.
 * 
 * 22    4/10/98 11:11a Mgummelt
 * Resetting credit_enemy when killed
 * 
 * 21    4/09/98 1:57p Mgummelt
 * Some experience changes
 * 
 * 20    4/09/98 1:39p Ssengele
 * put in extra check to make sure i'm not awarding experience to world,
 * award exper. to right fella in t_damage for lava, frozen damage.
 * 
 * 19    4/08/98 7:10p Mgummelt
 * Added in script-side support for new spartanPrint option- only sends
 * chat and relevant death messages to clients
 * 
 * 18    4/08/98 2:27p Mgummelt
 * giving frag credit to the player who froze someone, not the one who
 * finishes them off
 * 
 * 17    4/07/98 9:59p Mgummelt
 * took out a couple sprints and made it so can't die from falling damage
 * 
 * 16    4/06/98 12:48p Ssengele
 * put in safety net to prevent endless loops in tripmine damage, ditched
 * a couple of unreferenced vars
 * 
 * 15    4/02/98 2:08p Mgummelt
 * Testing damage modification on grenades
 * 
 * 14    4/01/98 3:03p Ssengele
 * faster xbolts, initial optimizing of forcecube.
 * 
 * 13    3/31/98 12:49p Ssengele
 * took out unnoticeable shine to underwater xbolts that made for a good
 * chunk of traffic, re-optimized king-o-hill messages, commented damage
 * prints.
 * 
 * 12    3/30/98 2:22p Ssengele
 * assassin takes more damage from her own grenades, added meleeDamScale
 * server var to adjust amount of damage players take when they have their
 * 1st weapon selected.
 * 
 * 11    3/30/98 3:18a Rmidthun
 * Tweaking exp awarding for damage/kills
 * 
 * 10    3/29/98 6:47p Rmidthun
 * 
 * 9     3/26/98 4:54p Mgummelt
 * Fixed King of the Hill logic, added pain-exp, added poison damage to
 * t_rad
 * 
 * 8     3/23/98 3:12p Ssengele
 * netnice name printing in
 * 
 * 7     3/20/98 4:17p Nalbury
 * Added capture the icon ... made it so sheep could still pick up
 * everything...
 * 
 * 6     3/17/98 1:09p Ssengele
 * fixed free experience for sheep, imp heads
 * 
 * 5     3/11/98 2:41a Nalbury
 * Added fixedlevel server option.
 * 
 * 4     3/09/98 2:39p Nalbury
 * Took out level based damage modifier
 * 
 * 3     3/07/98 2:49a Nalbury
 * worked on player gib stuff
 * 
 * 2     3/04/98 10:45p Nalbury
 * Added damagescale, start with tome of power, and start with 2nd weapon
 * options.
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 153   10/07/97 12:59p Mgummelt
 * 
 * 152   9/25/97 11:11a Mgummelt
 * 
 * 151   9/11/97 4:33p Mgummelt
 * 
 * 150   9/11/97 12:02p Mgummelt
 * 
 * 149   9/10/97 7:50p Mgummelt
 * 
 * 148   9/10/97 7:34p Mgummelt
 * 
 * 147   9/09/97 3:59p Mgummelt
 * 
 * 146   9/03/97 7:50p Mgummelt
 * 
 * 145   9/03/97 7:47p Mgummelt
 * 
 * 144   9/02/97 9:06p Mgummelt
 * 
 * 143   9/02/97 6:05p Mgummelt
 * 
 * 142   9/02/97 3:33p Mgummelt
 * 
 * 141   9/02/97 2:55a Mgummelt
 * 
 * 140   9/01/97 11:12p Rlove
 * 
 * 139   9/01/97 3:27p Mgummelt
 * 
 * 138   9/01/97 3:08a Mgummelt
 * 
 * 137   9/01/97 1:35a Mgummelt
 * 
 * 136   8/31/97 10:48p Mgummelt
 * 
 * 135   8/31/97 2:36p Mgummelt
 * 
 * 134   8/31/97 11:38a Mgummelt
 * To which I say- shove where the sun don't shine- sideways!  Yeah!
 * How's THAT for paper cut!!!!
 * 
 * 133   8/31/97 8:52a Mgummelt
 * 
 * 132   8/29/97 11:42p Mgummelt
 * 
 * 131   8/29/97 8:26p Mgummelt
 * 
 * 130   8/29/97 4:17p Mgummelt
 * Long night
 * 
 * 129   8/29/97 12:33a Mgummelt
 * 
 * 128   8/29/97 12:33a Mgummelt
 * 
 * 127   8/28/97 8:56p Mgummelt
 * 
 * 126   8/28/97 2:42p Mgummelt
 * 
 * 125   8/28/97 12:44a Mgummelt
 * 
 * 124   8/27/97 11:44p Mgummelt
 * 
 * 123   8/27/97 11:30p Mgummelt
 * 
 * 122   8/27/97 10:52p Mgummelt
 * 
 * 121   8/26/97 10:28p Mgummelt
 * 
 * 120   8/26/97 6:00p Mgummelt
 * 
 * 119   8/26/97 10:17a Mgummelt
 * 
 * 118   8/26/97 9:30a Mgummelt
 * 
 * 117   8/26/97 7:38a Mgummelt
 * 
 * 116   8/26/97 2:26a Mgummelt
 * 
 * 115   8/24/97 4:03p Rlove
 * 
 * 114   8/21/97 1:53p Mgummelt
 * 
 * 113   8/21/97 4:23a Mgummelt
 * 
 * 112   8/21/97 3:33a Mgummelt
 * 
 * 111   8/20/97 3:44p Mgummelt
 * 
 * 110   8/20/97 1:16p Rlove
 * 
 * 109   8/19/97 1:09p Mgummelt
 * 
 * 108   8/19/97 12:57p Mgummelt
 * 
 * 107   8/18/97 4:47p Rlove
 * 
 * 106   8/17/97 3:45p Mgummelt
 * 
 * 105   8/17/97 3:06p Mgummelt
 * 
 * 104   8/16/97 6:25p Mgummelt
 * 
 * 103   8/15/97 11:27p Mgummelt
 * 
 * 102   8/15/97 4:59p Mgummelt
 * 
 * 101   8/15/97 2:40p Rlove
 * 
 * 100   8/14/97 10:27p Bgokey
 * 
 * 99    8/14/97 8:31p Mgummelt
 * 
 * 98    8/14/97 4:47p Mgummelt
 * 
 * 97    8/14/97 5:55a Rlove
 * Now it really works when you die and Super Health brings you back to
 * life.
 * 
 * 96    8/13/97 5:33p Rlove
 * 
 * 95    8/13/97 5:32p Mgummelt
 * 
 * 94    8/13/97 12:11p Mgummelt
 * 
 * 93    8/13/97 8:15a Rlove
 * 
 * 92    8/11/97 3:27p Rlove
 * Work on Divine Intervention
 * 
 * 91    8/11/97 2:54p Rlove
 * 
 * 90    8/09/97 6:28a Mgummelt
 * 
 * 89    8/09/97 4:14a Mgummelt
 * 
 * 88    8/09/97 2:04a Mgummelt
 * 
 * 87    8/09/97 1:49a Mgummelt
 * 
 * 86    8/06/97 10:18p Mgummelt
 * 
 * 85    8/05/97 8:32p Mgummelt
 * 
 * 84    8/05/97 6:47p Mgummelt
 * 
 * 83    8/05/97 11:24a Rlove
 * Only melee hurts the snake
 * 
 * 82    8/02/97 10:17a Rlove
 * Monster don't attack monsters any more.
 * 
 * 81    7/30/97 11:49p Mgummelt
 * 
 * 80    7/30/97 11:22p Mgummelt
 * 
 * 79    7/30/97 3:32p Mgummelt
 * 
 * 78    7/29/97 9:15p Mgummelt
 * 
 * 77    7/28/97 7:50p Mgummelt
 * 
 * 76    7/28/97 2:10p Mgummelt
 * 
 * 75    7/28/97 1:51p Mgummelt
 * 
 * 74    7/26/97 8:38a Mgummelt
 * 
 * 73    7/25/97 11:26p Mgummelt
 * 
 * 72    7/25/97 3:50p Mgummelt
 * 
 * 71    7/25/97 11:20a Mgummelt
 * 
 * 70    7/24/97 12:33p Mgummelt
 * 
 * 69    7/24/97 12:30p Mgummelt
 * 
 * 68    7/24/97 3:26a Mgummelt
 * 
 * 67    7/21/97 3:03p Rlove
 * 
 * 66    7/21/97 12:35p Mgummelt
 * 
 * 65    7/21/97 11:45a Mgummelt
 * 
 * 64    7/19/97 9:53p Mgummelt
 * 
 * 63    7/18/97 11:06a Mgummelt
 * 
 * 62    7/17/97 4:12p Mgummelt
 * 
 * 61    7/17/97 2:17p Mgummelt
 * 
 * 60    7/15/97 8:41p Mgummelt
 * 
 * 59    7/15/97 8:30p Mgummelt
 * 
 * 58    7/15/97 8:03p Mgummelt
 * 
 * 57    7/01/97 3:30p Mgummelt
 * 
 * 56    7/01/97 2:21p Mgummelt
 * 
 * 55    7/01/97 9:46a Rlove
 * Crusader soul sphere is in. It does double damage.
 * 
 * 54    6/30/97 7:30p Rlove
 * 
 * 53    6/30/97 3:23p Mgummelt
 * 
 * 52    6/29/97 10:56a Rlove
 * 
 * 51    6/27/97 10:18a Rlove
 * 
 * 50    6/18/97 4:21p Mgummelt
 * 
 * 49    6/18/97 10:18a Rjohnson
 * Removed excess entity fields
 * 
 * 48    6/17/97 7:14a Rlove
 * 
 * 47    6/16/97 7:35p Mgummelt
 * 
 * 46    6/16/97 4:00p Mgummelt
 * 
 * 45    6/16/97 2:08p Rlove
 * Temp fix for armor calc loop
 * 
 * 43    6/16/97 12:03p Rjohnson
 * Removed imp stuff
 * 
 * 42    6/10/97 3:43p Rlove
 * New armor calc
 * 
 * 41    6/06/97 9:17p Mgummelt
 * 
 * 40    5/31/97 4:00p Mgummelt
 * 
 * 39    5/31/97 3:59p Mgummelt
 * 
 * 38    5/31/97 12:18a Mgummelt
 * 
 * 37    5/29/97 4:33p Mgummelt
 * 
 * 36    5/27/97 9:40a Rlove
 * Took out super_damage and radsuit fields
 * 
 * 35    5/23/97 2:54p Mgummelt
 * 
 * 34    5/22/97 7:29p Mgummelt
 * 
 * 33    5/22/97 6:30p Mgummelt
 * 
 * 32    5/22/97 2:50a Mgummelt
 * 
 * 31    5/19/97 11:36p Mgummelt
 * 
 * 30    5/17/97 8:45p Mgummelt
 * 
 * 29    5/15/97 5:05a Mgummelt
 * 
 * 28    5/15/97 12:30a Mgummelt
 * 
 * 26    5/07/97 4:23p Mgummelt
 * 
 * 25    5/05/97 10:09p Mgummelt
 * 
 * 24    5/05/97 4:48p Mgummelt
 * 
 * 23    5/03/97 8:49a Rlove
 * 
 * 22    5/01/97 8:52p Mgummelt
 * 
 * 21    4/30/97 5:03p Mgummelt
 * 
 * 20    4/28/97 11:15a Rlove
 * Owner no longer gets hurt by radius explosion
 * 
 * 19    4/25/97 8:32p Mgummelt
 * 
 * 18    4/24/97 8:48p Mgummelt
 * 
 * 17    4/17/97 4:10p Mgummelt
 * 
 * 16    4/17/97 2:50p Mgummelt
 * 
 * 15    4/17/97 2:03p Mgummelt
 * 
 * 14    4/16/97 4:22p Mgummelt
 * 
 * 13    4/13/96 3:30p Mgummelt
 * 
 * 12    4/11/97 12:32a Mgummelt
 * 
 * 11    4/10/96 2:50p Mgummelt
 * 
 * 10    4/10/97 11:36a Mgummelt
 * 
 * 9     4/09/96 8:28p Mgummelt
 * 
 * 8     4/09/96 7:54p Mgummelt
 * 
 * 7     4/09/96 7:31p Mgummelt
 * 
 * 6     4/09/96 4:49p Mgummelt
 * 
 * 5     4/09/96 4:43p Mgummelt
 * 
 * 4     4/07/97 4:13p Mgummelt
 * 
 * 3     4/07/97 3:07p Mgummelt
 * 
 * 2     3/31/97 6:39a Rlove
 * New stuff
 * 
 * 1     3/31/97 6:39a Rlove
 * 
 * 15    3/21/97 4:22p Aleggett
 * Added sliding information to T_Damage for barrels
 * 
 * 13    3/21/97 9:38a Rlove
 * Created CHUNK.HC and MATH.HC, moved brush_die to chunk_death so others
 * can use it.
 * 
 * 12    3/14/97 9:21a Rlove
 * Plaques are done 
 * 
 * 11    3/12/97 4:23p Rlove
 * New Monster AI
 * 
 * 10    2/10/97 4:27p Rjohnson
 * Made it so that T_Damage will not change an enemy if the attacker has
 * no target set
 * 
 * 9     2/07/97 1:37p Rlove
 * Artifact of Invincibility
 * 
 * 8     2/03/97 4:42p Rlove
 * Newest soul sphere
 * 
 * 7     2/03/97 3:12p Rlove
 * Added soul spheres
 * 
 * 6     1/28/97 10:28a Rjohnson
 * Added experience fields and awarding experience function
 * 
 * 5     12/31/96 8:41a Rlove
 * Glyph of the Ancients is working
 * 
 * 4     12/13/96 10:50a Rjohnson
 * When damage it being done, if the monster is awake, it will just change
 * the target, otherwise, it will try to wake the monster (which changes
 * it think function)
 * 
 * 3     12/06/96 2:02p Rjohnson
 * Revised T_Damage and T_RadiusDamage
 * 
 * 2     11/11/96 1:12p Rlove
 * Added Source Safe stuff
 */
