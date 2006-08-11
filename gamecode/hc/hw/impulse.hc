/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/impulse.hc,v 1.3 2006-08-11 09:21:27 sezero Exp $
 */

void PlayerAdvanceLevel(float NewLevel);
void player_level_cheat(void);
void player_experience_cheat(void);
void Polymorph (entity loser);

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


/*void()gravityup =
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

void makeplayer ()
{
	dprint("make player \n");
	newmis=spawn();
	setmodel(newmis,self.model);
	setorigin(newmis,self.origin);
	newmis.frame = self.frame;
	newmis.angles=self.angles;
	newmis.think=SUB_Remove;
	thinktime newmis : 10;
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

//Have to allow panic button and QuickInventory impulses to work as well as impulse 23
//	if(self.flags2&FL_CHAINED&&self.impulse!=23)
//		return;

	if (self.impulse == 9&&skill<3)
		CheatCommand ();
	else if (self.impulse == 99)
		ClientKill();
	else if (self.impulse ==149)
		dprintf("Serverflags are now: %s\n",serverflags);
   else if (self.impulse == 23 )  // To use inventory item
		UseInventoryItem ();
//	else if(self.impulse==33)
//		see_coop_view();
	else if(self.impulse==32)
		PanicButton();
//	else if (self.impulse == 27)//Uncomment this for a good time!
//		ToggleChaseCam(self);
	else if (self.impulse == 28)//&&(!coop)&&(!deathmatch))
		makeplayer();
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
	else if (self.impulse >= 100 && self.impulse <= 115)
	{
		Inventory_Quick(self.impulse - 99);
	}
	else if (self.impulse == 254)
	{
		sprint(self,PRINT_MEDIUM, "King of the Hill is ");
		search=FindExpLeader();
		sprintname(self,PRINT_MEDIUM, search);
		sprint(self,PRINT_MEDIUM, " (EXP = ");
		sprint(self,PRINT_MEDIUM, ftos(search.experience));
		sprint(self,PRINT_MEDIUM, ") \n");
	}
	else if (self.impulse == 255)
		PrintFrags();
	else if (self.impulse>170&&self.impulse<176&&cvar("registered"))
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
		self.effects=self.drawflags=FALSE;
		remove_invincibility(self);
		self.playerclass=self.newclass;//So it drops exp the right amount
		drop_level(self,2);

		newmis=spawn();
		newmis.classname="classchangespot";
		newmis.angles=self.angles;
		setorigin(newmis,self.origin);
//what's this  - it's code for single play which we don't need anymore
//		if(!deathmatch&&!coop)
//			parm7=self.newclass;//Just to tell respawn() not to use restart
//		else
		{
			self.model=self.init_model;
			GibPlayer('0 0 1');
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
	else if (self.impulse == 14)
	{
		if (self.last_use_time < time - 10)//can only sheepify self after 10 secs of inactivity
		{
			Polymorph(self);
			self.last_use_time = time;
		}
	}
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/02/15 10:57:27  sezero
 * updated the boss files according to H2MP. although coop is
 * broken as of now, maybe someone messes with these one day...
 *
 * Revision 1.1.1.1  2004/11/29 11:27:37  sezero
 * Initial import
 *
 * Revision 1.1.1.1  2001/11/09 17:05:06  theoddone33
 * Inital import
 *
 * 
 * 11    4/06/98 12:07p Jmonroe
 * added make player to verify correct models
 * 
 * 10    3/30/98 3:26a Rmidthun
 * allowing use of panic button and quick inventory impulses when chained
 * 
 * 9     3/29/98 4:43p Rmidthun
 * cleaned up some, commented out coop view and chase camera since they
 * crash the server
 * 
 * 8     3/26/98 6:40p Mgummelt
 * Put succubus into on the fly class change
 * 
 * 7     3/26/98 4:55p Mgummelt
 * re-implemented chasecam, fixed coop_view (works for DM teams)
 * 
 * 6     3/24/98 6:06p Ssengele
 * exploding grenades, xbolts, tripmines can't go into infinite loops
 * anymore; sheep impulse has 10 sec delay; scarab staff using union in
 * non-dangerous way (tho that frankly dint seem to cause any probs)
 * 
 * 5     3/23/98 3:12p Ssengele
 * netnice name printing in
 * 
 * 4     3/17/98 11:48a Ssengele
 * sheep/suicide abuse limited--can only sheepify every 3 secs, suicide
 * every 5 secs (re-enabled impulse 14, hope that's ok)
 * 
 * 3     3/09/98 5:42p Rjohnson
 * Removed cheat
 * 
 * 2     3/07/98 6:22p Rjohnson
 * Player class updates
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 123   10/23/97 11:38a Mgummelt
 * 
 * 122   10/16/97 5:06p Mgummelt
 * 
 * 121   10/07/97 11:59a Mgummelt
 * 
 * 120   9/25/97 3:43p Mgummelt
 * 
 * 119   9/23/97 5:18p Mgummelt
 * 
 * 118   9/23/97 4:48p Mgummelt
 * 
 * 117   9/11/97 4:21p Mgummelt
 * 
 * 116   9/11/97 3:38p Mgummelt
 * 
 * 115   9/10/97 11:39p Mgummelt
 * 
 * 114   9/10/97 10:48p Mgummelt
 * 
 * 113   9/10/97 7:50p Mgummelt
 * 
 * 112   9/10/97 6:44p Mgummelt
 * 
 * 111   9/09/97 3:45p Mgummelt
 * 
 * 110   9/09/97 2:32p Rjohnson
 * Removed cheats
 * 
 * 109   9/01/97 9:32p Rlove
 * 
 * 108   9/01/97 6:27p Rlove
 * 
 * 107   9/01/97 3:38p Rlove
 * 
 * 106   8/31/97 7:07p Rlove
 * 
 * 105   8/31/97 2:36p Mgummelt
 * 
 * 104   8/31/97 11:38a Mgummelt
 * To which I say- shove where the sun don't shine- sideways!  Yeah!
 * How's THAT for paper cut!!!!
 * 
 * 103   8/31/97 8:52a Mgummelt
 * 
 * 102   8/27/97 9:22p Mgummelt
 * 
 * 101   8/27/97 8:14p Mgummelt
 * 
 * 100   8/25/97 6:08p Mgummelt
 * 
 * 99    8/25/97 6:07p Mgummelt
 * 
 * 98    8/25/97 4:06p Mgummelt
 * 
 * 97    8/24/97 8:32p Mgummelt
 * 
 * 96    8/24/97 4:35p Rlove
 * Working on flight
 * 
 * 95    8/23/97 8:24p Mgummelt
 * 
 * 94    8/21/97 4:04p Rlove
 * 
 * 93    8/21/97 10:55a Mgummelt
 * 
 * 92    8/21/97 4:55a Mgummelt
 * 
 * 91    8/21/97 4:23a Mgummelt
 * 
 * 90    8/20/97 5:00p Mgummelt
 * 
 * 89    8/20/97 5:48a Rlove
 * 
 * 88    8/19/97 8:14p Mgummelt
 * 
 * 87    8/19/97 2:12p Rjohnson
 * Fix for inventory
 * 
 * 86    8/19/97 12:57p Mgummelt
 * 
 * 85    8/19/97 10:04a Rjohnson
 * Removed camera stuff
 * 
 * 84    8/18/97 1:46p Mgummelt
 * 
 * 83    8/18/97 1:45p Mgummelt
 * 
 * 82    8/18/97 11:25a Mgummelt
 * 
 * 81    8/17/97 3:13a Mgummelt
 * 
 * 80    8/15/97 4:39p Mgummelt
 * 
 * 79    8/15/97 4:08p Rlove
 * 
 * 78    8/15/97 3:23p Rlove
 * 
 * 77    8/09/97 1:58a Mgummelt
 * 
 * 76    8/09/97 1:56a Mgummelt
 * 
 * 75    8/08/97 5:50p Rjohnson
 * Quick use for inventory items
 * 
 * 74    8/06/97 12:09a Bgokey
 * 
 * 73    8/05/97 3:17p Rlove
 * 
 * 72    8/01/97 4:40p Rlove
 * 
 * 71    8/01/97 1:43a Mgummelt
 * 
 * 70    7/31/97 6:36p Mgummelt
 * 
 * 69    7/30/97 10:43p Mgummelt
 * 
 * 68    7/28/97 7:50p Mgummelt
 * 
 * 67    7/28/97 1:51p Mgummelt
 * 
 * 66    7/26/97 8:38a Mgummelt
 * 
 * 65    7/25/97 10:35p Mgummelt
 * 
 * 64    7/25/97 9:50a Rlove
 * 
 * 63    7/21/97 3:03p Rlove
 * 
 * 62    7/18/97 4:25p Mgummelt
 * 
 * 61    7/17/97 4:54p Rlove
 * 
 * 60    7/17/97 1:53p Rlove
 * 
 * 59    7/17/97 11:43a Mgummelt
 * 
 * 58    7/16/97 8:12p Mgummelt
 * 
 * 57    7/15/97 8:41p Mgummelt
 * 
 * 56    7/15/97 2:31p Mgummelt
 * 
 * 55    7/14/97 2:12p Mgummelt
 * 
 * 54    7/08/97 3:09p Rlove
 * 
 * 53    7/07/97 6:34p Rjohnson
 * Added a test impulse
 * 
 * 52    7/07/97 2:24p Mgummelt
 * 
 * 51    7/03/97 4:43p Rlove
 * 
 * 50    7/03/97 1:59p Rlove
 * 
 * 49    7/03/97 11:40a Rjohnson
 * Added an impulse to display coords
 * 
 * 48    6/30/97 3:23p Mgummelt
 * 
 * 47    6/30/97 9:41a Rlove
 * 
 * 46    6/27/97 5:35p Mgummelt
 * 
 * 45    6/23/97 4:50p Mgummelt
 * 
 * 44    6/18/97 4:00p Mgummelt
 * 
 * 43    6/18/97 2:44p Mgummelt
 * 
 * 42    6/06/97 2:52p Rlove
 * Artifact of Super Health now functions properly
 * 
 * 41    6/05/97 4:44p Rlove
 * Fly mode is network friendly now.
 * 
 * 40    6/02/97 7:58p Mgummelt
 * 
 * 39    5/31/97 9:29p Mgummelt
 * 
 * 38    5/28/97 1:43p Rlove
 * Plaques now activate when you bump into them.
 * 
 * 37    5/27/97 8:22p Mgummelt
 * 
 * 36    5/27/97 9:23a Rlove
 * 
 * 35    5/27/97 8:57a Rlove
 * Plaques work again. proj_ofs needed to be added to the origin so you
 * could 'see' the plaque
 * 
 * 34    5/23/97 11:51p Mgummelt
 * 
 * 33    5/23/97 1:29p Rlove
 * 
 * 32    5/19/97 11:36p Mgummelt
 * 
 * 31    5/15/97 11:43a Rjohnson
 * Stats updates
 * 
 * 30    5/14/97 3:36p Rjohnson
 * Inital stats implementation
 * 
 * 29    5/13/97 2:18p Rjohnson
 * Added some monster removing impulses
 * 
 * 28    5/12/97 11:12p Mgummelt
 * 
 * 27    5/08/97 5:47p Mgummelt
 * 
 * 26    5/06/97 1:29p Mgummelt
 * 
 * 25    5/05/97 10:09p Mgummelt
 * 
 * 24    5/02/97 8:06p Mgummelt
 * 
 * 23    4/26/97 3:52p Mgummelt
 * 
 * 22    4/25/97 8:32p Mgummelt
 * 
 * 21    4/22/97 5:55p Mgummelt
 * 
 * 20    4/19/97 1:01p Rjohnson
 * Added a puzzle inventory impulse
 * 
 * 19    4/17/97 1:45p Mgummelt
 * 
 * 18    4/16/96 11:51p Mgummelt
 * 
 * 17    4/13/96 3:30p Mgummelt
 * 
 * 16    4/09/96 8:29p Mgummelt
 * 
 * 15    4/09/96 4:43p Mgummelt
 * 
 * 14    4/09/97 11:07a Mgummelt
 * 
 * 13    4/07/97 6:30p Mgummelt
 * 
 * 12    4/07/97 3:09p Mgummelt
 * 
 * 11    4/07/97 2:50p Mgummelt
 * 
 * 10    4/07/97 1:46p Mgummelt
 * 
 * 9     4/04/97 6:37p Mgummelt
 * 
 * 8     4/04/97 5:40p Rlove
 * 
 * 7     3/18/97 7:37a Rlove
 * Added tome of power
 * 
 * 6     2/12/97 10:17a Rlove
 * 
 * 5     1/07/97 3:23p Rlove
 * Fixed plaque problem
 * 
 * 4     12/30/96 8:31a Rlove
 * Changing the use command
 * 
 * 3     12/26/96 1:32p Rlove
 * Took out old impulses
 * 
 * 2     11/12/96 2:39p Rlove
 * Updates for the inventory system. Torch, HP boost and Super HP Boost.
 */
