/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/impulse.hc,v 1.4 2007-02-07 16:57:06 sezero Exp $
 */

void PlayerAdvanceLevel(float NewLevel);
void player_level_cheat(void);
void player_experience_cheat(void);
void Polymorph (entity loser);
//void create_swarm (void);

void restore_weapon ()
{//FIXME: use idle, not select
	self.weaponframe = 0;
	if (self.playerclass==CLASS_PALADIN)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/gauntlet.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/vorpal.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/axe.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/purifier.mdl";
	}
	else if (self.playerclass==CLASS_CRUSADER)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/warhamer.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/icestaff.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/meteor.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/sunstaff.mdl";
	}
	else if (self.playerclass==CLASS_NECROMANCER)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/sickle.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/sickle.mdl";  // FIXME: still need these models
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/sickle.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/ravenstf.mdl";
	}
	else if (self.playerclass==CLASS_ASSASSIN)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/punchdgr.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/crossbow.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/v_assgr.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/scarabst.mdl";
	}
}

void see_coop_view ()
{
entity startent,found;
float gotone;
	if(!coop&&!teamplay)
	{
		centerprint(self,"Ally vision not available\n");
		return;
	}

	startent=self.viewentity;
	found=startent;
	while(!gotone)
	{
		found=find(found,classname,"player");
		if(found.flags2&FL_ALIVE)
			if((deathmatch&&found.team==self.team)||coop)
				gotone=TRUE;
		if(found==startent)
		{
			centerprint(self,"No allies available\n");
			return;
		}
	}

	sprint(self,found.netname);
	sprint(self," found!\n");
	self.viewentity=found;
	CameraViewPort(self,found);
	CameraViewAngles(self,found);
	if(self.viewentity==self)
	{
		self.oldweapon=self.weapon;//for deselection animation
		restore_weapon();
	}
	else
	{
		self.weaponmodel=self.viewentity.weaponmodel;
		self.weaponframe=self.viewentity.weaponframe;
	}
}

void player_everything_cheat(void)
{
	if(deathmatch||coop)
		return;

	CheatCommand();		// Give them weapons and mana	

	Artifact_Cheat();	// Give them all artifacts

	self.puzzles_cheat = 1;		// Get them past puzzles

	// Then they leave home and never call you. The ingrates.
}

void PrintFrags()
{
entity lastent;
	lastent=nextent(world);
	while(lastent)
	{
		if(lastent.classname=="player")
		{
			bprint(lastent.netname);
			bprint(" (L-");
			bprint(ftos(lastent.level));
			if(lastent.playerclass==CLASS_ASSASSIN)
				bprint(" Assassin) ");
			else if(lastent.playerclass==CLASS_PALADIN)
				bprint(" Paladin) ");
			else if(lastent.playerclass==CLASS_CRUSADER)
				bprint(" Crusader) ");
			else
				bprint(" Necromancer) ");
			bprint(" FRAGS: ");
			bprint(ftos(lastent.frags));
			bprint(" (LF: ");
			bprint(ftos(lastent.level_frags));
			bprint(")\n");
		}
		lastent=find(lastent,classname,"player");
	}
}

/*
void()gravityup =
{
	self.gravity+=0.01;
	if(self.gravity==10)
		self.gravity=0;
	dprint("Gravity: ");
	dprint(ftos(self.gravity));
	dprint("\n");
};

void()gravitydown =
{
	self.gravity-=0.01;
	if(self.gravity==-10)
		self.gravity=0;
	dprint("Gravity: ");
	dprint(ftos(self.gravity));
	dprint("\n");
};
*/

void player_stopfly(void)
{
	self.movetype = MOVETYPE_WALK;
	self.idealpitch = cvar("sv_walkpitch");
	self.idealroll = 0;
}

void player_fly(void)
{
	self.movetype = MOVETYPE_FLY;
	self.velocity_z = 100;   // A little push up
	self.hoverz = .4;  
}

void HeaveHo (void)
{
vector dir;
float inertia, lift;
	makevectors(self.v_angle);
	dir=normalize(v_forward);

	traceline(self.origin+self.proj_ofs,self.origin+self.proj_ofs+dir*48,FALSE,self);
	if(trace_ent.movetype&&trace_ent.solid&&trace_ent!=world&&trace_ent.flags&FL_ONGROUND&&trace_ent.solid!=SOLID_BSP)
	{
		if(!trace_ent.mass)
			inertia = 1;
		else if(trace_ent.mass<=50)
			inertia=trace_ent.mass/10;
		else
			inertia=trace_ent.mass/100;
		lift=(self.strength/40+0.5)*300/inertia;
		if(lift>300)
			lift=300;
		trace_ent.velocity_z+=lift;

		if (trace_ent.flags&FL_ONGROUND)
			trace_ent.flags-=FL_ONGROUND;

		if(self.playerclass==CLASS_ASSASSIN)
			sound (self, CHAN_BODY,"player/assjmp.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_BODY,"player/paljmp.wav", 1, ATTN_NORM);
		self.attack_finished=time+1;
	}
}

void AddServerFlag(float addflag)
{
	addflag=byte_me(addflag+8);
	dprintf("Serverflags were: %s\n",serverflags);
	dprintf("Added flag %s\n",addflag);
	serverflags(+)addflag;
	dprintf("Serverflags are now: %s\n",serverflags);
}

/*
============
ImpulseCommands

============
*/
void() ImpulseCommands =
{
	entity search;
	float total;
//	string s2;

	if(self.flags2&FL_CHAINED&&self.impulse!=23)
		return;

	if (self.impulse == 9&&skill<3)
		CheatCommand ();
	else if (self.impulse == 14)
		Polymorph(self);
	else if (self.impulse == 99)
		ClientKill();
	else if (self.impulse ==149)
		dprintf("Serverflags are now: %s\n",serverflags);
//	else if (self.impulse >149 && self.impulse <157)
//		AddServerFlag(self.impulse - 149);
//	else if (self.impulse == 21 )  // To activate torch
//		UseTorch ();
	else if (self.impulse == 23 )  // To use inventory item
		UseInventoryItem ();
	else if(self.impulse==33)
		see_coop_view();
	else if(self.impulse==32)
		PanicButton();
/*	else if (self.impulse == 27)//Uncomment this for a good time!
		MakeCamera();*/
	else if (self.impulse == 34)
	{	// S.A listing puzzle inventory is a good idea
		sprint(self,"Puzzle Inventory: ");
		if (self.puzzle_inv1)
		{
			sprint(self,self.puzzle_inv1);
			sprint(self," ");
		}
		if (self.puzzle_inv2)
		{
			sprint(self,self.puzzle_inv2);
			sprint(self," ");
		}
		if (self.puzzle_inv3)
		{
			sprint(self,self.puzzle_inv3);
			sprint(self," ");
		}
		if (self.puzzle_inv4)
		{
			sprint(self,self.puzzle_inv4);
			sprint(self," ");
		}
		if (self.puzzle_inv5)
		{
			sprint(self,self.puzzle_inv5);
			sprint(self," ");
		}
		if (self.puzzle_inv6)
		{
			sprint(self,self.puzzle_inv6);
			sprint(self," ");
		}
		if (self.puzzle_inv7)
		{
			sprint(self,self.puzzle_inv7);
			sprint(self," ");
		}
		if (self.puzzle_inv8)
		{
			sprint(self,self.puzzle_inv8);
			sprint(self," ");
		}
		sprint(self,"\n");
	}
	else if (self.impulse==35&&skill<3)
	{
		search = nextent(world);
		total = 0;

		while(search != world)
		{
			if (search.flags & FL_MONSTER)
			{
				total += 1;
				remove(search);
			}
			search = nextent(search);
		}
		dprintf("Removed %s monsters\n",total);
	}
	else if (self.impulse==36&&skill<3)
	{
		search = nextent(world);
		total = 0;

		while(search != world)
		{
			if (search.flags & FL_MONSTER)
			{
				total += 1;
				thinktime search : 99999;
			}
			search = nextent(search);
		}
		dprintf("Froze %s monsters\n",total);
	}
	else if (self.impulse==37&&skill<3)
	{
		search = nextent(world);
		total = 0;

		while(search != world)
		{
			if (search.flags & FL_MONSTER)
			{
				total += 1;
				thinktime search : HX_FRAME_TIME;
			}
			search = nextent(search);
		}
		dprintf("UnFroze %s monsters\n",total);
	}
/*	else if (self.impulse==38)
	{
		sprint(self,"Class: ");
		s2 = ftos(self.playerclass);
		sprint(self,s2);
		sprint(self,"\n");

		sprint(self,"   Hit Points: ");
		s2 = ftos(self.health);
		sprint(self,s2);
		s2 = ftos(self.max_health);
		sprint(self,"/");
		sprint(self,s2);
		sprint(self,"\n");

		sprint(self,"   Strength: ");
		s2 = ftos(self.strength);
		sprint(self,s2);
		sprint(self,"\n");

		sprint(self,"   Intelligence: ");
		s2 = ftos(self.intelligence);
		sprint(self,s2);
		sprint(self,"\n");

		sprint(self,"   Wisdom: ");
		s2 = ftos(self.wisdom);
		sprint(self,s2);
		sprint(self,"\n");

		sprint(self,"   Dexterity: ");
		s2 = ftos(self.dexterity);
		sprint(self,s2);
		sprint(self,"\n");
	}*/
	else if(self.impulse==25)
	{
		if(deathmatch||coop)
		{
			self.impulse=0;
			return;
		}
		else
		{
			self.cnt_tome += 1;
			Use_TomeofPower();
		}
	}
	else if(self.impulse==39&&skill<3)
	{
		if(deathmatch||coop)
		{
			self.impulse=0;
			return;
		}
		else	// Toggle flight
		{
			if (self.movetype != MOVETYPE_FLY)
				player_fly();
			else
				player_stopfly();
		}
	}
	else if(self.impulse==40&&skill<3)
	{
		if(deathmatch||coop)
		{
			self.impulse=0;
			return;
		}
		else
			player_level_cheat();
	}
	else if(self.impulse==41&&skill<3)
	{
		if(deathmatch||coop)
		{
			self.impulse=0;
			return;
		}
		else
			player_experience_cheat();
	}
	else if (self.impulse == 42)
	{
		dprintv("Coordinates: %s\n", self.origin);
		dprintv("Angles: %s\n",self.angles);
		dprint("Map is ");
		dprint(mapname);
		dprint("\n");
	}
	else if(self.impulse==43&&skill<3)
		player_everything_cheat();
	else if(self.impulse==44)
		DropInventoryItem();
/*	else if (self.impulse == 99)
	{	// RJ's test impulse
		search = nextent(world);
		total = 0;

		while(search != world)
		{
			if (search.classname == "monster_fish")
			{
				total += 1;
				dprintf("%s. ",total);
				dprintv("%s\n",search.origin);
			}
			search = nextent(search);
		}
	}*/
	else if (self.impulse >= 100 && self.impulse <= 115)
	{
		Inventory_Quick(self.impulse - 99);
	}
	else if (self.impulse == 254)
	{
		sprint(self,"King of the Hill is ");
		search=FindExpLeader();
		sprint(self,search.netname);
		sprint(self," (EXP = ");
		sprint(self,ftos(search.experience));
		sprint(self,") \n");
	}
	else if (self.impulse == 255)
		PrintFrags();
	else if (self.impulse>170&&self.impulse<175&&cvar("registered"))
	{
		if(self.level<3)
		{
			sprint(self,"You must have achieved level 3 or higher to change class!\n");
			self.impulse=0;
			return;
		}

		if(self.impulse==171)//Quick Class-change hot-keys
			if(self.playerclass==CLASS_PALADIN)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_PALADIN;
		else if(self.impulse==172)
			if(self.playerclass==CLASS_CRUSADER)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_CRUSADER;
		else if(self.impulse==173)
			if(self.playerclass==CLASS_NECROMANCER)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_NECROMANCER;
		else if(self.impulse==174)
			if(self.playerclass==CLASS_ASSASSIN)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_ASSASSIN;
		self.effects=self.drawflags=FALSE;
		self.playerclass=self.newclass;//So it drops exp the right amount
		drop_level(self,2);

		newmis=spawn();
		newmis.classname="classchangespot";
		newmis.angles=self.angles;
		setorigin(newmis,self.origin);

		if(!deathmatch&&!coop)
			parm7=self.newclass;//Just to tell respawn() not to use restart
		else
		{
			self.model=self.init_model;
			GibPlayer();
			self.frags -= 2;	// extra penalty
		}
		respawn ();
	}

	if(self.model=="models/sheep.mdl")
	{
		self.impulse=0;
		return;
	}
	else if (self.impulse >= 1 && self.impulse <= 4)
		W_ChangeWeapon ();
	else if ((self.impulse == 10) && (wp_deselect == 0))
		CycleWeaponCommand ();
//	else if (self.impulse == 11)
//		ServerflagsCommand ();
	else if (self.impulse == 12)
		CycleWeaponReverseCommand ();
	else if(self.impulse == 13)
		HeaveHo();
	else if (self.impulse == 22 &&!self.flags2 & FL2_CROUCHED)  // To crouch
	{
		if(self.flags2 & FL2_CROUCH_TOGGLE)
			self.flags2(-)FL2_CROUCH_TOGGLE;
		else
			self.flags2(+)FL2_CROUCH_TOGGLE;
//		PlayerCrouch();
	}
	self.impulse = 0;
};

