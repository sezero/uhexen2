//====================================================================================================
//
//   Model names
//
//   Paladin: 
//			2   -  w_l2_c1   -  vorpal sword
//			3   -  w_l3_c1   -  axe
//			4.1 -  w_l41_c1  -  purifier head 
//			4.2 -  w_l42_c1  -  purifier staff
//
//   Crusader:
//			2   -  w_l2_c2   -  ice staff
//			3   -  w_l3_c2   -  meteor wand
//			4.1 -  w_l41_c2  -  sun staff head 
//			4.2 -  w_l42_c2  -  sun staff staff
//
//   Necromancer:
//			2   -  w_l2_c3   -  magic missile
//			3   -  w_l3_c3   -  bone shard
//			4.1 -  w_l41_c3  -  set staff head
//			4.2 -  w_l42_c3  -  set staff staff
//
//   Assassin:
//			2   -  w_l2_c4   -  crossbow
//			3   -  w_l3_c4   -  grenade
//			4.1 -  w_l41_c4  -  vindictus head
//			4.2 -  w_l42_c4  -  vindctus staff
//
//
//====================================================================================================



//====================================================================================================
//
//   GENERIC WEAPON ARTIFACTS - DETERMINED BY PLAYER CLASS
//
//====================================================================================================

/*QUAKED wp_weapon2 (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
In world weapon 2 pickup for single player mode
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void wp_weapon2 (void)
{
	precache_model("models/w_l2_c1.mdl");
	precache_model("models/w_l2_c2.mdl");
	precache_model("models/w_l2_c3.mdl");
	precache_model("models/w_l2_c4.mdl");

	CreateEntityNew(self,ENT_WEAPON2_ART,"models/w_l2_c1.mdl",SUB_Null);
	setsize(self,'0 0 0','0 0 0');
	self.hull=HULL_POINT;

	self.flags(+)FL_CLASS_DEPENDENT;
	self.touch = weapon_touch;	
	self.items=IT_WEAPON2;
	StartItem ();

}


/*QUAKED wp_weapon3 (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
In world weapon 3 pickup for single player mode
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void wp_weapon3 (void)
{
	precache_model("models/w_l3_c1.mdl");
	precache_model("models/w_l3_c2.mdl");
	precache_model("models/w_l3_c3.mdl");
	precache_model("models/w_l3_c4.mdl");

	CreateEntityNew(self,ENT_WEAPON41_ART,"models/w_l3_c1.mdl",SUB_Null);
	setsize(self,'0 0 0','0 0 0');
	self.hull=HULL_POINT;

	self.flags(+)FL_CLASS_DEPENDENT;
	self.touch = weapon_touch;	
	self.items=IT_WEAPON3;
	StartItem ();

}

/*QUAKED wp_weapon4_head (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
In world weapon 4 pickup (staff head) for single player mode
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void wp_weapon4_head (void)
{
	precache_model("models/w_l41_c1.mdl");
	precache_model("models/w_l41_c2.mdl");
	precache_model("models/w_l41_c3.mdl");
	precache_model("models/w_l41_c4.mdl");

	CreateEntityNew(self,ENT_WEAPON41_ART,"models/w_l41_c1.mdl",SUB_Null);
	setsize(self,'0 0 0','0 0 0');
	self.hull=HULL_POINT;

	self.flags(+)FL_CLASS_DEPENDENT;
	self.touch = weapon_touch;	
	self.items=IT_WEAPON4_1;
	StartItem ();

}

/*QUAKED wp_weapon4_staff (0 0 0) (-8 -8 -44) (8 8 20) FLOATING
In world weapon 4 pickup (staff) for single player mode
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void wp_weapon4_staff (void)
{
	precache_model("models/w_l42_c1.mdl");
	precache_model("models/w_l42_c2.mdl");
	precache_model("models/w_l42_c3.mdl");
	precache_model("models/w_l42_c4.mdl");

	CreateEntityNew(self,ENT_WEAPON42_ART,"models/w_l42_c1.mdl",SUB_Null);
	setsize(self,'0 0 0','0 0 0');
	self.hull=HULL_POINT;

	self.flags(+)FL_CLASS_DEPENDENT;
	self.touch = weapon_touch;	
	self.items=IT_WEAPON4_2;
	StartItem ();

}

