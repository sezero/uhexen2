//In progs.src, must come AFTER all player.hc's


void() SetModelAndThinks =
{//MG
//Note: you'll also want to set a head model
//for each.

//FIXME
//	self.touch=PlayerTouch;

	self.th_die=PlayerDie;
	self.th_goredeath=player_frames_behead;
	self.th_pain=player_pain;
//FIXME: used a fixed level?
//	self.experience=350000;
	self.level=6;
	self.flags2(-)FL2_FIRERESIST;//WHY?
	if(self.playerclass==CLASS_ASSASSIN)
	{
		self.items(+)IT_WEAPON2|IT_WEAPON3|IT_WEAPON4;
		self.mass=6;//should be 15

		Ass_Change_Weapon();	//sets other th_*'s based on weapon in hand

		setmodel (self, "models/assassin.mdl");
		self.headmodel="models/h_ass.mdl";
		if(self.weapon==IT_WEAPON4)
			self.th_weapon=grenade_select;
		else if(self.weapon==IT_WEAPON2||self.weapon==IT_WEAPON3)
			self.th_weapon=crossbow_select;
		else
			self.th_weapon=punchdagger_select;
		self.armor_breastplate = 20;
	}
	else if(self.playerclass==CLASS_SUCCUBUS)
	{
		self.mass=7;

		self.items(+)IT_WEAPON2|IT_WEAPON3|IT_WEAPON4|IT_WEAPON5|IT_WEAPON6|IT_WEAPON7|IT_WEAPON8;
		self.flags2(+)FL2_FIRERESIST;
		
		Suc_Change_Weapon();	//sets other th_*'s based on weapon in hand

		setmodel (self, "models/succubus.mdl");
		self.headmodel="models/h_suc.mdl";
		if(self.weapon==IT_WEAPON4)
			self.th_weapon=grenade_select;
		else if(self.weapon==IT_WEAPON2)
			self.th_weapon=vorpal_select;
		else if(self.weapon==IT_WEAPON3)
			self.th_weapon=crossbow_select;
		else if(self.weapon==IT_WEAPON1)
			self.th_weapon=bloodrain_select;
		else
			self.th_weapon=flameorb_select;
		self.armor_amulet = self.armor_bracer = 20;
	}
	else if(self.playerclass==CLASS_CRUSADER)
	{
		self.mass=7;//should be 10
		setmodel (self, "models/crusader.mdl");
		self.headmodel="models/h_cru.mdl";

		Cru_Change_Weapon();
		if(self.weapon==IT_WEAPON1)
			self.th_weapon=warhammer_select;
		else
			self.th_weapon=icestaff_select;
		self.items(+)IT_WEAPON2|IT_WEAPON3|IT_WEAPON4|IT_WEAPON5|IT_WEAPON6|IT_WEAPON7|IT_WEAPON8;
		self.armor_bracer = self.armor_breastplate = self.armor_helmet = 20;
	}
	else if(self.playerclass==CLASS_PALADIN)
	{
		self.items(+)IT_WEAPON2|IT_WEAPON3;
		self.mass=8;//should be 15
		Pal_Change_Weapon();	//sets other th_*'s based on weapon in hand

		setmodel (self, "models/paladin.mdl");
		self.headmodel="models/h_pal.mdl";

		if(self.weapon==IT_WEAPON3)
			self.th_weapon=crossbow_select;
		else if(self.weapon==IT_WEAPON2)
			self.th_weapon=axe_select;
		else
			self.th_weapon=vorpal_select;
		self.armor_amulet = self.armor_bracer = self.armor_breastplate = self.armor_helmet = 20;
	}
	else if(self.playerclass==CLASS_NECROMANCER)
	{
		self.mass=7;//should be 10
		setmodel (self, "models/necro.mdl");
		self.headmodel="models/h_nec.mdl";

		Nec_Change_Weapon();	//sets other th_*'s based on weapon in hand

		if(self.weapon==IT_WEAPON1)
			self.th_weapon=sickle_select;
		else if(self.weapon==IT_WEAPON2)
			self.th_weapon=crossbow_select;
		else
			self.th_weapon=magicmis_select;
		self.items(+)IT_WEAPON2|IT_WEAPON3|IT_WEAPON4|IT_WEAPON5|IT_WEAPON6|IT_WEAPON7|IT_WEAPON8;
	}
	if(self.playerclass==CLASS_DWARF)
	{
		self.items(+)IT_WEAPON2;
		self.mass=4;//should be 15
		Dwf_Change_Weapon();	//sets other th_*'s based on weapon in hand

		setmodel (self, "models/hank.mdl");
		self.headmodel="models/h_hank.mdl";

		if(self.weapon==IT_WEAPON2)
			self.th_weapon=axe_select;
		else
			self.th_weapon=warhammer_select;
		self.armor_amulet = self.armor_bracer = self.armor_breastplate = self.armor_helmet = 20;
		setsize(self,'-16 -16 0','16 16 28');
		self.hull=HULL_CROUCH;
	}
	else
		setsize(self,'-16 -16 0','16 16 56');
	self.init_model=self.model;
	self.flags(+)FL_SPECIAL_ABILITY1;
	self.flags(+)FL_SPECIAL_ABILITY2;
};

