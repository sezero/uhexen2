/*
==========================================================

Siege.hc

Totally new entities specifically for Siege

Mike Gummelt

==========================================================
*/

//New effects for players
void turn_undead () {return;}

float num_for_weap (float it_weap)
{
	if(it_weap==IT_WEAPON1)
		return 1;
	else if(it_weap==IT_WEAPON2)
		return 2;
	else if(it_weap==IT_WEAPON3)
		return 3;
	else if(it_weap==IT_WEAPON4)
		return 4;
	else if(it_weap==IT_WEAPON5)
		return 5;
	else if(it_weap==IT_WEAPON6)
		return 6;
	else if(it_weap==IT_WEAPON7)
		return 7;
	else if(it_weap==IT_WEAPON8)
		return 8;
	else
		return 0;
}

float melee_dmg_mod_for_strength (float str_value)
{
float mod_return;
	mod_return=(str_value - 10)/4;
	if(mod_return<0.3)
		mod_return=0.3;
	return mod_return;
}

/*
class 1 - 5
weapons 1 - 3
rate of fire - seconds between shots
accuracy - 0 to 1 * random vector
*/

float rate_and_acc_for_weap [36] =
{
	0.3,0,	0.2,0,	2,.33,	//paladin
	0.2,0,	0,0,	0,0,	//crusader
	0.3,0,	2.5,.2, 0,0,	//necro
	0.1,0,	1,0,	3,0,	//assassin
	0.1,0,	0.3,0,	1,0.1,	//succubus
	0.2,0,	0.1,0,	0,0		//dwarf
};

/*
class 1 - 6
*/

float player_swim_mod [6] =
{
	1,		//paladin- has free action - already ok
	.9,		//crusader
	.82,	//necro
	1.2,	//assassin
	.75,	//succubus
	1		//dwarf - fucked in C code already
};
/*
class 1 - 6
*/

float player_jump_mod [6] =
{
	1.2,//paladin
	0.9,//crusader
	0.8,//necro
	1.5,//assassin
	2,//succubus
	0.65//dwarf 
};

void WriteTeam (float svmsg,entity holder)
{
	entity found;
	found=find(world,classname,"player");
	while(found)
	{
		if(found.flags&FL_CLIENT)
		{
			msg_entity = found;
			switch(svmsg)
			{
			case SVC_HASKEY://send to all- in case need to clear out previous owner on any team
				WriteByte(MSG_ONE,svmsg);
				if(found.siege_team==other.siege_team)
					WriteEntity(MSG_ONE,holder);//tell team
				else
					WriteEntity(MSG_ONE,world);//clear opposite team
				break;
			case SVC_NONEHASKEY://just send to team of last holder
				if(found.siege_team==holder.siege_team)
					WriteByte(MSG_ONE,svmsg);
				break;
			case SVC_ISDOC://only send to def.
				if(found.siege_team==ST_DEFENDER)
				{
					WriteByte(MSG_ONE,svmsg);
					WriteEntity(MSG_ONE,holder);
				}
				break;
			case SVC_NODOC://only to def.
				if(found.siege_team==ST_DEFENDER)
					WriteByte(MSG_ONE,svmsg);
				break;
			}
		}
		found=find(found,classname,"player");
	}
}
 
//Teleport Triggers call this
void become_defender (entity defent)
{
	if(defent.classname!="player"||defent.siege_team==ST_DEFENDER)
		return;
	defent.siege_team=ST_DEFENDER;
	defent.skin=0;
	defent.last_time=time;
	defent.health=defent.max_health;
	setsiegeteam(defent,ST_DEFENDER);//update C and clients
	bprintname(PRINT_HIGH,defent);
	bprint(PRINT_HIGH," becomes a Defender!\n");
	self.target="defender";
}

void become_attacker (entity attent)
{
	if(attent.classname!="player"||attent.siege_team==ST_ATTACKER)
		return;
	attent.siege_team=ST_ATTACKER;
	attent.skin=1;
	attent.last_time=time;
	attent.health=attent.max_health;
	setsiegeteam(attent,ST_ATTACKER);//update C and clients
	bprintname(PRINT_HIGH,attent);
	bprint(PRINT_HIGH," becomes an Attacker!\n");
	self.target="attacker";
}

void become_either (entity eitherent)
{
entity found;
float num_att,num_def;
	num_att=num_def=0;
	found=find(world,classname,"player");
	while(found)
	{
		if(found.siege_team==ST_ATTACKER)
			num_att+=1;
		else if(found.siege_team==ST_DEFENDER)
			num_def+=1;
		found=find(found,classname,"player");
	}
	dprintf("Attackers: %s, Defenders:",num_att);
	dprintf("%s\n",num_def);
	if(num_att>num_def)
		become_defender(eitherent);
	else if(num_def>num_att)
		become_attacker(eitherent);
	else if(random()<0.5)
		become_defender(eitherent);
	else
		become_attacker(eitherent);
}

float expvalforpclass [6] =
{//how much base exp for killing this class
	12,//paladin
	9,//crusader
	7,//necro
	8,//assassin
	10,//succ
	11//dwarf
};

float expmodforpclass [6] =
{//exp multiplier for killing AS this class
	0.6,//paladin
	1.5,//crusader
	1.4,//necro
	1,//assassin
	0.8,//succ
	1.2//dwarf
};

void CheckExpAward (entity attacker,entity inflictor,entity targ,float damage,float killed)
{
float t_expval;//base xp for this targ
float t_expbon;//bonus xp for this targ
float a_expbon;//attacker bonus
float a_expmod;//attacker multiplier
float total_exp;//total
float final_mod;//damage as % of max_health
float killbonus;//whether or not a kill is a bonus

	if(attacker==targ)
	{//killed self
		if(killed)
			attacker.experience-=2;//whoops!
		return;
	}

	if(targ.siege_team == attacker.siege_team)
	{//killed teammate
		if(killed)
			attacker.experience-=10;//bad aim!
		return;
	}

	t_expval = 0;
	t_expbon = 0;
	a_expbon = 0;
	a_expmod = 1;
	if(targ.classname=="player")
	{
		killbonus = TRUE;//Killed a player, kill is more valuable than hurt
		t_expval = expvalforpclass[targ.playerclass - 1];
//		if(killed&&targ.siege_team==ST_DEFENDER)
//			t_expval *= 2;//harder to kill a defender.

		if(targ.sheep_time>time)
			t_expbon -= 5;
		
		if(targ.flags2&FL2_EXCALIBUR)
			t_expbon += 188;//200 points for killing DoC

		if(targ.invisible_time>time||targ.haste_time>time)
			t_expbon += 30;//30 points for killing someone very fast or invis

		if(targ.playerclass == CLASS_ASSASSIN && targ.drawflags&DRF_TRANSLUCENT)
			t_expbon += 10;//10 points for killing a Assassin hiding in shadows

		if(targ.puzzle_inv1==g_keyname)
			t_expbon += 250;//250 bonus for killing key dude

		if(inflictor.classname=="barrel")
		{
			if(inflictor.frags)
				a_expbon += 50;//100 points for killing someone by shooting a barrel
		}
		else if(inflictor.classname=="balbolt")
		{
			if(targ.teleport_time>time)
				a_expbon += 200;//200 points for killing someone with ballista shot in mid air!
			else
				a_expbon += 75;//100 points for killing someone with ballits
		}

		if(attacker.sheep_time>time&&killed)
			a_expmod = 3;//3*point bonus for sheep kill
		else if(attacker.flags2&FL2_EXCALIBUR&&killed)
			a_expmod = 2;
		else
			a_expmod = expmodforpclass[attacker.playerclass - 1];
	}
	else if(targ.classname=="catapult")
	{
		if(attacker.siege_team==ST_DEFENDER)
			t_expval = 300;
		else
			t_expval -= 125;//broke own cat!
	}
	else if(targ.classname=="obj_ballista")
	{
		if(attacker.siege_team==ST_ATTACKER)
		{
			t_expval = 250;
			if(inflictor.classname=="barrel")
				a_expbon += 50;//nice aim!
			else if(inflictor.classname=="balbolt")
				a_expbon += 20;//sneaky!
		}
		else
			t_expval -= 100;//broke own ballista!
	}
	else if(targ.classname=="breakable_brush")
	{
		//fixme - set names on crowncase, other important breakables, etc.
		//penalize defenders? But not if tunneling or breaking tunnel supports
		
		//fixme - check target field first
		if(targ.target=="rockscnt")
		{//collapse tunnel
			switch(attacker.siege_team)
			{
				case ST_DEFENDER:
					t_expval = 150;//150 points for collapsing tunnel
					break;
				case ST_ATTACKER:
					if(killed)
						t_expval = -100;//-100 point for collapsing tunnel
					break;
			}
		}
		else if(targ.thingtype == THINGTYPE_METAL)
		{
			if(attacker.siege_team==ST_ATTACKER)
				t_expval = 250;//250 points for breaking chains, door, etc
		}
		else if(targ.thingtype == THINGTYPE_GREYSTONE)
		{//200 points for breaking rampart, opening back wall
			if(attacker.siege_team==ST_ATTACKER)
			{
				t_expval = 200;
				if(inflictor.classname=="barrel")
					t_expbon += 50;//extra for barrel users
			}
		}
		else//25 points for breaking a generic brush
			t_expval = 25;
	}
	total_exp = (t_expval + t_expbon + a_expbon)*a_expmod;
	if(!killbonus||!killed)
	{//only a % of total value, otherwise, killed a player, get it all
		if(!targ.max_health||damage>targ.max_health)
			final_mod = 1;
		else
			final_mod = damage/targ.max_health;
		total_exp *= final_mod;
	}
	attacker.experience += total_exp;
}
