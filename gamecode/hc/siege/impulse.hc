/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/impulse.hc,v 1.3 2007-02-07 17:01:07 sezero Exp $
 */

void PlayerAdvanceLevel(float NewLevel);
void Polymorph (entity loser);
//void create_swarm (void);

void restore_weapon ()
{//FIXME: use idle, not select
	self.weaponframe = 0;
	if (self.playerclass==CLASS_PALADIN)
	{
		if (self.weapon == IT_WEAPON1)
			if(self.flags2&FL2_EXCALIBUR)
				self.weaponmodel = "models/v_excal.mdl";
			else
				self.weaponmodel = "models/vorpal.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/axe.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/xbow2.mdl";
	}
	else if (self.playerclass==CLASS_CRUSADER)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/warhamer.mdl";
		else
			self.weaponmodel = "models/icestaff.mdl";
	}
	else if (self.playerclass==CLASS_NECROMANCER)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/sickle.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/xbow2.mdl";
		else
			self.weaponmodel = "models/spllbook.mdl";  // FIXME: still need these models
	}
	else if (self.playerclass==CLASS_ASSASSIN)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/punchdgr.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/xbow2.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/crossbow.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/v_assgr.mdl";
	}
	else if (self.playerclass==CLASS_SUCCUBUS)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/sucwp1.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/vorpal.mdl";
		else if (self.weapon == IT_WEAPON3)
			self.weaponmodel = "models/xbow2.mdl";
		else if (self.weapon == IT_WEAPON4)
			self.weaponmodel = "models/v_assgr.mdl";
	}
	else if (self.playerclass==CLASS_DWARF)
	{
		if (self.weapon == IT_WEAPON1)
			self.weaponmodel = "models/warhamer.mdl";
		else if (self.weapon == IT_WEAPON2)
			self.weaponmodel = "models/axe.mdl";
	}
}

void see_coop_view ()
{
entity startent,found;
float gotone;
	if(self.viewentity!=self)
		centerprint(self,"Ally vision not available in chase camera mode\n");

	if(!coop&&!teamplay)
	{
		centerprint(self,"Ally vision not available\n");
		return;
	}

	if(self.cameramode==world)
		startent=self;
	else
		startent=self.cameramode;
	found=startent;
	while(!gotone)
	{
		found=find(found,classname,"player");
		if(found.flags&FL_CLIENT)
			if((deathmatch&&teamplay&&found.team==self.team)||coop)
				if(found.cameramode==world||found==self)
					gotone=TRUE;
		if(found==startent)
		{
			centerprint(self,"No allies available\n");
			return;
		}
	}

	if(found==self)
	{
		CameraReturn();
		return;
	}
	centerprint(self,found.netname);
	AllyVision(self,found);
	self.weaponmodel=self.cameramode.weaponmodel;
	self.weaponframe=self.cameramode.weaponframe;
}

void player_everything_cheat(float override)
{
	if((deathmatch||coop)&&!override)
		return;

	CheatCommand(override);		// Give them weapons and mana	

//	Artifact_Cheat();	// Give them all artifacts

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
			bprint(PRINT_MEDIUM, lastent.netname);
			bprint(PRINT_MEDIUM, " (L-");
			bprint(PRINT_MEDIUM, ftos(lastent.level));
			if(lastent.playerclass==CLASS_ASSASSIN)
				bprint(PRINT_MEDIUM, " Assassin) ");
			else if(lastent.playerclass==CLASS_PALADIN)
				bprint(PRINT_MEDIUM, " Paladin) ");
			else if(lastent.playerclass==CLASS_CRUSADER)
				bprint(PRINT_MEDIUM, " Crusader) ");
			else
				bprint(PRINT_MEDIUM, " Necromancer) ");
			bprint(PRINT_MEDIUM, " FRAGS: ");
			bprint(PRINT_MEDIUM, ftos(lastent.frags));
			bprint(PRINT_MEDIUM, " (LF: ");
			bprint(PRINT_MEDIUM, ftos(lastent.level_frags));
			bprint(PRINT_MEDIUM, ")\n");
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
	dprint(PRINT_MEDIUM, "Gravity: ");
	dprint(PRINT_MEDIUM, ftos(self.gravity));
	dprint(PRINT_MEDIUM, "\n");
};

void()gravitydown =
{
	self.gravity-=0.01;
	if(self.gravity==-10)
		self.gravity=0;
	dprint(PRINT_MEDIUM, "Gravity: ");
	dprint(PRINT_MEDIUM, ftos(self.gravity));
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
	if(!self.flags&FL_ONGROUND)
		return;

	makevectors(self.v_angle);
	dir=normalize(v_forward);

	traceline(self.origin+self.proj_ofs,self.origin+self.proj_ofs+dir*48,FALSE,self);

	if (!trace_ent.flags&FL_ONGROUND)
		return;

/*oops, now using a catapult will push it
	if(trace_ent.classname=="catapult")
	{
		if(trace_ent.frame==20||trace_ent.frame>22)
		{
			trace_ent.think=catapult_fire;
			thinktime trace_ent : 0;
		}
		return;
	}
*/
	if(trace_ent!=world&&trace_ent.movetype&&trace_ent.solid&&trace_ent.solid!=SOLID_BSP)//&&trace_ent.flags&FL_ONGROUND
	{
		if(!trace_ent.mass)
			inertia = 1;
		else if(trace_ent.mass<=50)
			inertia=trace_ent.mass/10;
		else
			inertia=trace_ent.mass/100;
		lift=(self.strength/30+0.5)*300/inertia;//was /40
		if(lift>300)
			lift=300;//don't throw it over your head
		if(trace_ent.velocity_z<lift)
		{
			trace_ent.velocity_z+=lift;
			if(trace_ent.velocity_z>lift)
				trace_ent.velocity_z=lift;
		}
		else
			return;

		trace_ent.flags(-)FL_ONGROUND;

		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
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

float CanClimb()
{
vector org;
	makevectors(self.v_angle);
	org = self.origin+self.proj_ofs;
	traceline(org,org+v_forward*48,FALSE,self);
	if(trace_fraction<1&&trace_ent.solid==SOLID_BSP)
		return TRUE;
	return FALSE;
}

void makeplayer ()
{
	newmis=spawn();
	setmodel(newmis,self.model);
	setorigin(newmis,self.origin);
	newmis.frame = self.frame;
	newmis.angles=self.angles;
	newmis.think=SUB_Remove;
	newmis.skin=self.skin;
	thinktime newmis : 10;
}

void delete_ents()
{
entity found;
	return;
	found=nextent(world);
	while(found)
	{
		if(found.classname!="player")
			remove(found);
		found=nextent(found);
	}
}

void CountSides ()
{
entity found;
float num_att,num_def;
string printnum;
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
	sprint(self,PRINT_HIGH,"Attackers: ");
	printnum=ftos(num_att);
	sprint(self,PRINT_HIGH,printnum);
	sprint(self,PRINT_HIGH,"\n");
	sprint(self,PRINT_HIGH,"Defenders: ");
	printnum=ftos(num_def);
	sprint(self,PRINT_HIGH,printnum);
	sprint(self,PRINT_HIGH,"\n");
}

/*
============
ImpulseCommands

============
*/
void() ImpulseCommands =
{
//	entity search;
//	float total;
//	string s2;

	if(!self.impulse)
		return;

/*
	if(self.netname=="CHEATER")
		switch(self.cnt)
		{
		case 0:
			if(self.impulse == 3)
				self.cnt+=1;
			else
				self.cnt=0;
			break;
		case 1:
			if(self.impulse == 7)
				self.cnt+=1;
			else
				self.cnt=0;
			break;
		case 2:
			if(self.impulse == 1)
				self.cnt+=1;
			else
				self.cnt=0;
			break;
		case 3:
			if(self.impulse == 7)
				self.cnt=0;
			player_everything_cheat(TRUE);
			break;
		}*/

//Have to allow panic button and QuickInventory impulses to work as well as impulse 23
//	if(self.flags2&FL_CHAINED&&self.impulse!=23)
//		return;

/*	else if (self.impulse == 56)
	{//time left
	string printnum;
	float p1,p2,p3;
		p1=cvar("timelimit")*60 - time;//find # of seconds left
		p3=floor(p1/60);//find # of minutes
		p2=(p1 - p3*60)/100;//leftover seconds, shift to right of decimal
		p3+=p2;
		printnum=ftos(p3);
		sprint(self,PRINT_HIGH,printnum);
		sprint(self,PRINT_HIGH,"\n");
//		centerprint(self,printnum);
	}*/
//	else if (self.impulse == 99)
//		ClientKill();
//	else if (self.impulse == 98)
//		makeplayer();
//	else if (self.impulse ==149)
//		dprintf("Serverflags are now: %s\n",serverflags);
	if (self.impulse == 23&&!self.climbing)  // To use inventory item
		UseInventoryItem ();
//	else if(self.impulse==33)
//		see_coop_view();
	else if(self.impulse==69)
		delete_ents();
	else if (self.impulse==34)
	{
		if(self.puzzle_inv1!="")
		{
			self.puzzle_id=self.puzzle_inv1;
			DropPuzzlePiece(TRUE);
			self.puzzle_inv1=self.puzzle_id="";
			centerprint(self,"Key Dropped...\n");
		}
		else
			centerprint(self,"You don't have the key!\n");
	}
	else if(self.impulse==35)
		CountSides();
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
	else if (self.impulse == 42)
	{
		dprintv("Coordinates: %s\n", self.origin);
		dprintv("Angles: %s\n",self.angles);
		dprint("Map is ");
		dprint(mapname);
		dprint("\n");
	}
	else if(self.impulse==44)
		DropInventoryItem();
	else if (self.impulse >= 100 && self.impulse <= 115)
	{
		Inventory_Quick(self.impulse - 99);
	}
	else if(self.impulse == 201)
	{
		if(self.siege_team)
		{
			if(self.siege_team==ST_DEFENDER)
				centerprint(self,"You are already a defender!\n");
			else
				centerprint(self,"Can't - you are an attacker!\n");
		}
		else
		{
			self.siege_team=ST_DEFENDER;
			setsiegeteam(self,ST_DEFENDER);//update C and clients
		}
	}
	else if (self.impulse == 202)
	{
		if(self.siege_team)
		{
			if(self.siege_team==ST_DEFENDER)
				centerprint(self,"Can't - you are a defender!\n");
			else
				centerprint(self,"You are already an attacker!\n");
		}
		else
		{
			self.siege_team=ST_ATTACKER;
			setsiegeteam(self,ST_ATTACKER);//update C and clients
		}
	}
	else if (self.impulse == 203)
	{
		if(self.siege_team)
		{
			if(self.siege_team==ST_DEFENDER)
				centerprint(self,"You are a defender!\n");
			else
				centerprint(self,"You are an attacker!\n");
		}
		else
			centerprint(self,"No team!!!\n");
	}
//	else if (self.impulse == 255)
//		PrintFrags();

	if(self.model=="models/sheep.mdl")
	{
		self.impulse=0;
		return;
	}
	else if (self.impulse >= 1 && self.impulse <= 8)
		W_ChangeWeapon ();
//	else if ((self.impulse == 10) && (wp_deselect == 0))
//		CycleWeaponCommand ();
//	else if (self.impulse == 11)
//		ServerflagsCommand ();
//	else if (self.impulse == 12)
//		CycleWeaponReverseCommand ();
	else if(self.impulse == 13)
	{
		if(self.playerclass==CLASS_ASSASSIN&&self.flags2&FL2_WALLCLIMB)
		{
			if(self.climbing==FALSE)
			{
				if(CanClimb())
				{
					if(self.last_climb+1<time)
						Climb();
					self.impulse = 0;
					return;
				}
			}
			else
			{
				ClimbDrop();
				self.impulse = 0;
				return;
			}
		}
		HeaveHo();
	}
/*	else if (self.impulse == 14)
	{
		if (self.last_use_time < time - 10)//can only sheepify self after 10 secs of inactivity
		{
			Polymorph(self);
			self.last_use_time = time;
		}
	}*/
	else if (self.impulse == 22 &&!self.flags2 & FL2_CROUCHED)  // To crouch
	{
		if(self.flags2 & FL2_CROUCH_TOGGLE)
			self.flags2(-)FL2_CROUCH_TOGGLE;
		else
			self.flags2(+)FL2_CROUCH_TOGGLE;
//		PlayerCrouch();
	}
/*	else if (self.impulse>170&&self.impulse<177&&cvar("registered")&&self.last_use_time < time - 10)
	{
		if(randomclass)
		{
			centerprint(self,"Cannot switch classes with randomclass active...\n");
			self.impulse=0;
			return;
		}

		if(self.level<3)
		{
			centerprint(self,"You must have achieved level 3 or higher to change class!\n");
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
		else if(self.impulse==175)
			if(self.playerclass==CLASS_SUCCUBUS)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_SUCCUBUS;
		if(self.impulse==176)
			if(self.playerclass==CLASS_DWARF)
			{
				self.impulse=0;
				return;
			}
			else
				self.newclass=CLASS_DWARF;


		self.last_use_time = time;

		self.effects=self.drawflags=FALSE;
		remove_invincibility(self);
		self.playerclass=self.newclass;//So it drops exp the right amount
//		drop_level(self,2);

		newmis=spawn();
		newmis.classname="classchangespot";
		newmis.angles=self.angles;
		setorigin(newmis,self.origin);
//what's this  - it's code for single play which we don't need anymore
//		if(!deathmatch&&!coop)
//			parm7=self.newclass;//Just to tell respawn() not to use restart
//		else
//		{
			self.model=self.init_model;
			GibPlayer('0 0 1');
			self.frags -= 2;	// extra penalty
//		}
		respawn ();
	}
*/
	self.impulse = 0;
};

