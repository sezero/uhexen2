
//**************************************************************************
//**
//** golem_i.hc
//**
//** $Header: /HexenWorld/HCode/golem_i.hc 1     2/04/98 1:59p Rjohnson $
//**
//** Iron golem.
//**
//**************************************************************************

// FRAMES ------------------------------------------------------------------

// Transition from still to attack stance
$frame gbirth1      gbirth2      gbirth3      gbirth4      gbirth5
$frame gbirth6      gbirth7      gbirth8      gbirth9      gbirth10
$frame gbirth11     gbirth12

// Death
$frame gdeath1      gdeath2      gdeath3      gdeath4      gdeath5
$frame gdeath6      gdeath7      gdeath8      gdeath9      gdeath10
$frame gdeath11     gdeath12     gdeath13     gdeath14     gdeath15
$frame gdeath16     gdeath17     gdeath18     gdeath19     gdeath20
$frame gdeath21     gdeath22     gdeath23     gdeath24     gdeath25
$frame gdeath26     gdeath27     gdeath28     gdeath29     gdeath30
$frame gdeath31     gdeath32     gdeath33     gdeath34     gdeath35
$frame gdeath36     gdeath37     gdeath38     gdeath39     gdeath40
$frame gdeath41     gdeath42     gdeath43     gdeath44     gdeath45
$frame gdeath46     gdeath47     gdeath48     gdeath49     gdeath50
$frame gdeath51

// Walking
$frame gwalk1       gwalk2       gwalk3       gwalk4       gwalk5
$frame gwalk6       gwalk7       gwalk8       gwalk9       gwalk10
$frame gwalk11      gwalk12      gwalk13      gwalk14      gwalk15
$frame gwalk16      gwalk17      gwalk18      gwalk19      gwalk20
$frame gwalk21      gwalk22      gwalk23      gwalk24      gwalk25
$frame gwalk26      gwalk27      gwalk28      gwalk29      gwalk30
$frame gwalk31      gwalk32      gwalk33      gwalk34      gwalk35
$frame gwalk36      gwalk37      gwalk38      gwalk39      gwalk40
$frame gwalk41      gwalk42      gwalk43      gwalk44      gwalk45
$frame gwalk46      gwalk47      gwalk48      gwalk49      gwalk50
$frame gwalk51      gwalk52      gwalk53      gwalk54      gwalk55
$frame gwalk56      gwalk57      gwalk58      gwalk59      gwalk60

// Transition from attack stance to walking
$frame gtran1       gtran2       gtran3       gtran4       gtran5
$frame gtran6       gtran7       gtran8       gtran9       gtran10
$frame gtran11      gtran12      gtran13      gtran14      gtran15
$frame gtran16      gtran17      gtran18      gtran19      gtran20
$frame gtran21      gtran22      gtran23      gtran24      gtran25
$frame gtran26

// Gem attack
$frame ggem1        ggem2        ggem3        ggem4        ggem5
$frame ggem6        ggem7        ggem8        ggem9        ggem10
$frame ggem11       ggem12       ggem13       ggem14       ggem15
$frame ggem16       ggem17       ggem18       ggem19       ggem20
$frame ggem21       ggem22       ggem23       ggem24       ggem25

// Left hand pound attack
$frame gLpnd1       gLpnd2       gLpnd3       gLpnd4       gLpnd5
$frame gLpnd6       gLpnd7       gLpnd8       gLpnd9       gLpnd10
$frame gLpnd11      gLpnd12      gLpnd13      gLpnd14      gLpnd15
$frame gLpnd16      gLpnd17      gLpnd18      gLpnd19      gLpnd20

// Right hand punch attack
$frame gRpnch1      gRpnch2      gRpnch3      gRpnch4      gRpnch5
$frame gRpnch6      gRpnch7      gRpnch8      gRpnch9      gRpnch10
$frame gRpnch11     gRpnch12     gRpnch13     gRpnch14     gRpnch15
$frame gRpnch16     gRpnch17     gRpnch18     gRpnch19     gRpnch20
$frame gRpnch21     gRpnch22

void GolemIMeleeDecide(void);
void GolemIPunchRight(void);
void GolemIPoundLeft(void);

// CODE --------------------------------------------------------------------

void() golemi_gbirth1    = [ $gbirth1   , golemi_gbirth2    ] {  };
void() golemi_gbirth2    = [ $gbirth2   , golemi_gbirth3    ] {  };
void() golemi_gbirth3    = [ $gbirth3   , golemi_gbirth4    ] {  };
void() golemi_gbirth4    = [ $gbirth4   , golemi_gbirth5    ] {  };
void() golemi_gbirth5    = [ $gbirth5   , golemi_gbirth6    ] {  };
void() golemi_gbirth6    = [ $gbirth6   , golemi_gbirth7    ] {  };
void() golemi_gbirth7    = [ $gbirth7   , golemi_gbirth8    ] {  };
void() golemi_gbirth8    = [ $gbirth8   , golemi_gbirth9    ] {  };
void() golemi_gbirth9    = [ $gbirth9   , golemi_gbirth10   ] {  };
void() golemi_gbirth10   = [ $gbirth10  , golemi_gbirth11   ] {  };
void() golemi_gbirth11   = [ $gbirth11  , golemi_gbirth12   ] {  };
void() golemi_gbirth12   = [ $gbirth12  , golemi_gbirth1    ] {  };

/*
void() golemi_gdeath1    = [ $gdeath1   , golemi_gdeath2    ] {  };
void() golemi_gdeath2    = [ $gdeath2   , golemi_gdeath3    ] {  };
void() golemi_gdeath3    = [ $gdeath3   , golemi_gdeath4    ] {  };
void() golemi_gdeath4    = [ $gdeath4   , golemi_gdeath5    ] {  };
void() golemi_gdeath5    = [ $gdeath5   , golemi_gdeath6    ] {  };
void() golemi_gdeath6    = [ $gdeath6   , golemi_gdeath7    ] {  };
void() golemi_gdeath7    = [ $gdeath7   , golemi_gdeath8    ] {  };
void() golemi_gdeath8    = [ $gdeath8   , golemi_gdeath9    ] {  };
void() golemi_gdeath9    = [ $gdeath9   , golemi_gdeath10   ] {  };
void() golemi_gdeath10   = [ $gdeath10  , golemi_gdeath11   ] {  };
void() golemi_gdeath11   = [ $gdeath11  , golemi_gdeath12   ] {  };
void() golemi_gdeath12   = [ $gdeath12  , golemi_gdeath13   ] {  };
void() golemi_gdeath13   = [ $gdeath13  , golemi_gdeath14   ] {  };
void() golemi_gdeath14   = [ $gdeath14  , golemi_gdeath15   ] {  };
void() golemi_gdeath15   = [ $gdeath15  , golemi_gdeath16   ] {  };
void() golemi_gdeath16   = [ $gdeath16  , golemi_gdeath17   ] {  };
void() golemi_gdeath17   = [ $gdeath17  , golemi_gdeath18   ] {  };
void() golemi_gdeath18   = [ $gdeath18  , golemi_gdeath19   ] {  };
void() golemi_gdeath19   = [ $gdeath19  , golemi_gdeath20   ] {  };
void() golemi_gdeath20   = [ $gdeath20  , golemi_gdeath21   ] {  };
void() golemi_gdeath21   = [ $gdeath21  , golemi_gdeath22   ] {  };
void() golemi_gdeath22   = [ $gdeath22  , golemi_gdeath23   ] {  };
void() golemi_gdeath23   = [ $gdeath23  , golemi_gdeath24   ] {  };
void() golemi_gdeath24   = [ $gdeath24  , golemi_gdeath25   ] {  };
void() golemi_gdeath25   = [ $gdeath25  , golemi_gdeath26   ] {  };
void() golemi_gdeath26   = [ $gdeath26  , golemi_gdeath27   ] {  };
void() golemi_gdeath27   = [ $gdeath27  , golemi_gdeath28   ] {  };
void() golemi_gdeath28   = [ $gdeath28  , golemi_gdeath29   ] {  };
void() golemi_gdeath29   = [ $gdeath29  , golemi_gdeath30   ] {  };
void() golemi_gdeath30   = [ $gdeath30  , golemi_gdeath31   ] {  };
void() golemi_gdeath31   = [ $gdeath31  , golemi_gdeath32   ] {  };
void() golemi_gdeath32   = [ $gdeath32  , golemi_gdeath33   ] {  };
void() golemi_gdeath33   = [ $gdeath33  , golemi_gdeath34   ] {  };
void() golemi_gdeath34   = [ $gdeath34  , golemi_gdeath35   ] {  };
void() golemi_gdeath35   = [ $gdeath35  , golemi_gdeath36   ] {  };
void() golemi_gdeath36   = [ $gdeath36  , golemi_gdeath37   ]
{
	self.solid = SOLID_NOT;
};
void() golemi_gdeath37   = [ $gdeath37  , golemi_gdeath38   ] {  };
void() golemi_gdeath38   = [ $gdeath38  , golemi_gdeath39   ] {  };
void() golemi_gdeath39   = [ $gdeath39  , golemi_gdeath40   ] {  };
void() golemi_gdeath40   = [ $gdeath40  , golemi_gdeath41   ] {  };
void() golemi_gdeath41   = [ $gdeath41  , golemi_gdeath42   ] {  };
void() golemi_gdeath42   = [ $gdeath42  , golemi_gdeath43   ] {  };
void() golemi_gdeath43   = [ $gdeath43  , golemi_gdeath44   ] {  };
void() golemi_gdeath44   = [ $gdeath44  , golemi_gdeath45   ] {  };
void() golemi_gdeath45   = [ $gdeath45  , golemi_gdeath46   ] {  };
void() golemi_gdeath46   = [ $gdeath46  , golemi_gdeath47   ] {  };
void() golemi_gdeath47   = [ $gdeath47  , golemi_gdeath48   ] {  };
void() golemi_gdeath48   = [ $gdeath48  , golemi_gdeath49   ] {  };
void() golemi_gdeath49   = [ $gdeath49  , golemi_gdeath50   ] {  };
void() golemi_gdeath50   = [ $gdeath50  , golemi_gdeath51   ] {  };
void() golemi_gdeath51   = [ $gdeath51  , golemi_gdeath51   ] {  };
*/

/*
void() golemi_ggem1      = [ $ggem1     , golemi_ggem2      ] {ai_face();  };
void() golemi_ggem2      = [ $ggem2     , golemi_ggem3      ] {ai_face();  };
void() golemi_ggem3      = [ $ggem3     , golemi_ggem4      ] {ai_face();  };
void() golemi_ggem4      = [ $ggem4     , golemi_ggem5      ] {ai_face();  };
void() golemi_ggem5      = [ $ggem5     , golemi_ggem6      ] {ai_face();  };
void() golemi_ggem6      = [ $ggem6     , golemi_ggem7      ] {ai_face();  };
void() golemi_ggem7      = [ $ggem7     , golemi_ggem8      ] {ai_face();  };
void() golemi_ggem8      = [ $ggem8     , golemi_ggem9      ] {ai_face();  };
void() golemi_ggem9      = [ $ggem9     , golemi_ggem10     ] {ai_face();  };
void() golemi_ggem10     = [ $ggem10    , golemi_ggem11     ] {ai_face();  };
void() golemi_ggem11     = [ $ggem11    , golemi_ggem12     ] {ai_face();  };
void() golemi_ggem12     = [ $ggem12    , golemi_ggem13     ] {ai_face();  };
void() golemi_ggem13     = [ $ggem13    , golemi_ggem14     ] {ai_face();};
void() golemi_ggem14     = [ $ggem14    , golemi_ggem15     ] {ai_face();  };
void() golemi_ggem15     = [ $ggem15    , golemi_ggem16     ] {ai_face();  };
void() golemi_ggem16     = [ $ggem16    , golemi_ggem17     ] {ai_face();  };
void() golemi_ggem17     = [ $ggem17    , golemi_ggem18     ] {ai_face();};
void() golemi_ggem18     = [ $ggem18    , golemi_ggem19     ] {ai_face();  };
void() golemi_ggem19     = [ $ggem19    , golemi_ggem20     ] {ai_face();  };
void() golemi_ggem20     = [ $ggem20    , golemi_ggem21     ] {ai_face();  };
void() golemi_ggem21     = [ $ggem21    , golemi_ggem22     ] {ai_face();};
void() golemi_ggem22     = [ $ggem22    , golemi_ggem23     ] {ai_face();  };
void() golemi_ggem23     = [ $ggem23    , golemi_ggem24     ] {ai_face();  };
void() golemi_ggem24     = [ $ggem24    , golemi_ggem25     ] {ai_face();  };
void() golemi_ggem25     = [ $ggem25    , golem_run_init    ] {ai_face();  };
*/

/*
void() golemi_gLpnd1     = [ $gLpnd1    , golemi_gLpnd2     ] {  };
void() golemi_gLpnd2     = [ $gLpnd2    , golemi_gLpnd3     ] {  };
void() golemi_gLpnd3     = [ $gLpnd3    , golemi_gLpnd4     ] {  };
void() golemi_gLpnd4     = [ $gLpnd4    , golemi_gLpnd5     ] {  };
void() golemi_gLpnd5     = [ $gLpnd5    , golemi_gLpnd6     ] {  };
void() golemi_gLpnd6     = [ $gLpnd6    , golemi_gLpnd7     ] {  };
void() golemi_gLpnd7     = [ $gLpnd7    , golemi_gLpnd8     ] {  };
void() golemi_gLpnd8     = [ $gLpnd8    , golemi_gLpnd9     ] {  };
void() golemi_gLpnd9     = [ $gLpnd9    , golemi_gLpnd10    ] {  };
void() golemi_gLpnd10    = [ $gLpnd10   , golemi_gLpnd11    ] {  };
void() golemi_gLpnd11    = [ $gLpnd11   , golemi_gLpnd12    ] {  };
void() golemi_gLpnd12    = [ $gLpnd12   , golemi_gLpnd13    ] {  };
void() golemi_gLpnd13    = [ $gLpnd13   , golemi_gLpnd14    ] {  };
void() golemi_gLpnd14    = [ $gLpnd14   , golemi_gLpnd15    ] {  };
void() golemi_gLpnd15    = [ $gLpnd15   , golemi_gLpnd16    ] {  };
void() golemi_gLpnd16    = [ $gLpnd16   , golemi_gLpnd17    ] {  };
void() golemi_gLpnd17    = [ $gLpnd17   , golemi_gLpnd18    ] {  };
void() golemi_gLpnd18    = [ $gLpnd18   , golemi_gLpnd19    ] {  };
void() golemi_gLpnd19    = [ $gLpnd19   , golemi_gLpnd20    ] {  };
void() golemi_gLpnd20    = [ $gLpnd20   , golem_run_init    ] {  };
*/

/*
void() golemi_gRpnch1    = [ $gRpnch1   , golemi_gRpnch2    ] {  };
void() golemi_gRpnch2    = [ $gRpnch2   , golemi_gRpnch3    ] {  };
void() golemi_gRpnch3    = [ $gRpnch3   , golemi_gRpnch4    ] {  };
void() golemi_gRpnch4    = [ $gRpnch4   , golemi_gRpnch5    ] {  };
void() golemi_gRpnch5    = [ $gRpnch5   , golemi_gRpnch6    ] {  };
void() golemi_gRpnch6    = [ $gRpnch6   , golemi_gRpnch7    ] {  };
void() golemi_gRpnch7    = [ $gRpnch7   , golemi_gRpnch8    ] {  };
void() golemi_gRpnch8    = [ $gRpnch8   , golemi_gRpnch9    ] {  };
void() golemi_gRpnch9    = [ $gRpnch9   , golemi_gRpnch10   ] {  };
void() golemi_gRpnch10   = [ $gRpnch10  , golemi_gRpnch11   ] {  };
void() golemi_gRpnch11   = [ $gRpnch11  , golemi_gRpnch12   ] {  };
void() golemi_gRpnch12   = [ $gRpnch12  , golemi_gRpnch13   ] {  };
void() golemi_gRpnch13   = [ $gRpnch13  , golemi_gRpnch14   ] {  };
void() golemi_gRpnch14   = [ $gRpnch14  , golemi_gRpnch15   ] {  };
void() golemi_gRpnch15   = [ $gRpnch15  , golemi_gRpnch16   ] {  };
void() golemi_gRpnch16   = [ $gRpnch16  , golemi_gRpnch17   ] {  };
void() golemi_gRpnch17   = [ $gRpnch17  , golemi_gRpnch18   ] {  };
void() golemi_gRpnch18   = [ $gRpnch18  , golemi_gRpnch19   ] {  };
void() golemi_gRpnch19   = [ $gRpnch19  , golemi_gRpnch20   ] {  };
void() golemi_gRpnch20   = [ $gRpnch20  , golemi_gRpnch21   ] {  };
void() golemi_gRpnch21   = [ $gRpnch21  , golemi_gRpnch22   ] {  };
void() golemi_gRpnch22   = [ $gRpnch22  , golem_run_init    ] {  };
*/

/*
void() golemi_gtran1     = [ $gtran1    , golemi_gtran2     ] {  };
void() golemi_gtran2     = [ $gtran2    , golemi_gtran3     ] {  };
void() golemi_gtran3     = [ $gtran3    , golemi_gtran4     ] {  };
void() golemi_gtran4     = [ $gtran4    , golemi_gtran5     ] {  };
void() golemi_gtran5     = [ $gtran5    , golemi_gtran6     ] {  };
void() golemi_gtran6     = [ $gtran6    , golemi_gtran7     ] {  };
void() golemi_gtran7     = [ $gtran7    , golemi_gtran8     ] {  };
void() golemi_gtran8     = [ $gtran8    , golemi_gtran9     ] {  };
void() golemi_gtran9     = [ $gtran9    , golemi_gtran10    ] {  };
void() golemi_gtran10    = [ $gtran10   , golemi_gtran11    ] {  };
void() golemi_gtran11    = [ $gtran11   , golemi_gtran12    ] {  };
void() golemi_gtran12    = [ $gtran12   , golemi_gtran13    ] {  };
void() golemi_gtran13    = [ $gtran13   , golemi_gtran14    ] {  };
void() golemi_gtran14    = [ $gtran14   , golemi_gtran15    ] {  };
void() golemi_gtran15    = [ $gtran15   , golemi_gtran16    ] {  };
void() golemi_gtran16    = [ $gtran16   , golemi_gtran17    ] {  };
void() golemi_gtran17    = [ $gtran17   , golemi_gtran18    ] {  };
void() golemi_gtran18    = [ $gtran18   , golemi_gtran19    ] {  };
void() golemi_gtran19    = [ $gtran19   , golemi_gtran20    ] {  };
void() golemi_gtran20    = [ $gtran20   , golemi_gtran21    ] {  };
void() golemi_gtran21    = [ $gtran21   , golemi_gtran22    ] {  };
void() golemi_gtran22    = [ $gtran22   , golemi_gtran23    ] {  };
void() golemi_gtran23    = [ $gtran23   , golemi_gtran24    ] {  };
void() golemi_gtran24    = [ $gtran24   , golemi_gtran25    ] {  };
void() golemi_gtran25    = [ $gtran25   , golemi_gtran26    ] {  };
void() golemi_gtran26    = [ $gtran26   , golemi_gtran1     ] {  };
*/

void() golemi_gstand1 = [ $gbirth1, golemi_gstand1 ] { ai_stand(); };

//==========================================================================
//
// GolemIMeleeDecide
//
//==========================================================================

void GolemIMeleeDecide(void)
{
	if(random() > 0.5)
	{
		GolemIPunchRight();
	}
	else
	{
		GolemIPoundLeft();
	}
}

//==========================================================================
//
// GolemIPunchRight
//
//==========================================================================

void GolemIPunchRight(void) [++ $grpnch1..$grpnch22]
{
	if(cycle_wrapped)
	{
		golem_run_init();
		return;
	}
	if(self.frame == $grpnch8)
	{
		sound(self, CHAN_BODY, "golem/swing.wav", 1, ATTN_NORM);
	}
	if(self.frame > $grpnch9 && self.frame < $grpnch18)
	{
		ai_charge(2);
		ai_melee();
	}
	ai_face();
}

//==========================================================================
//
// GolemIPoundLeft
//
//==========================================================================

void GolemIPoundLeft(void) [++ $glpnd1..$glpnd20]
{
	if(cycle_wrapped)
	{
		golem_run_init();
		return;
	}
	if(self.frame == $glpnd8)
	{
		sound(self, CHAN_BODY, "golem/swing.wav", 1, ATTN_NORM);
	}
	if(self.frame > $glpnd11 && self.frame < $glpnd17)
	{
		ai_charge(2);
		ai_melee();
	}
	ai_face();
}

//==========================================================================
//
// golemi_pain
//
//==========================================================================

void golemi_pain(void)
{
	if(self.pain_finished > time)
	{
		return;
	}
	if(golem_flinch($gwalk1, $gwalk60)) return;
	if(golem_flinch($ggem1, $ggem25)) return;
	if(golem_flinch($gRpnch1, $gRpnch22)) return;
	golem_flinch($gLpnd1, $gLpnd20);
}

//==========================================================================
//
// monster_golem_iron
//
//==========================================================================
/*QUAKED monster_golem_iron (1 0 0) (-32 -32 -24) (32 32 64) AMBUSH
Iron Golem.
------- key / value ----------------------------
health = 400
experience_value = 200
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_iron(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2("models/golem_i.mdl");
//	precache_model2("models/h_golem.mdl"); // empty for now

	precache_sound2("golem/step.wav");
	precache_sound2("golem/swing.wav");
	precache_sound2("golem/mtlfall.wav");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.thingtype = THINGTYPE_METAL;
	self.yaw_speed = 10;
	self.mass = 100;
	self.mintel = 2;

	setmodel(self, "models/golem_i.mdl");

	setsize(self, '-20 -20 0', '20 20 80');

	self.health = 400;
	self.experience_value = 200;

	self.th_stand = golemi_gstand1;
	self.th_walk = golem_walk;
	self.th_run = golem_run_init;
	self.th_die = golem_die_init;
	self.th_melee = GolemIMeleeDecide;
	//self.th_missile = golemi_ggem1;
	self.th_pain = golemi_pain;
	self.view_ofs = '0 0 64';

	walkmonster_start();
}
