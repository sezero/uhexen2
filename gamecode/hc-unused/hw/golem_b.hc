
//**************************************************************************
//**
//** golem_b.hc
//**
//** $Header: /HexenWorld/HCode/golem_b.hc 1     2/04/98 1:59p Rjohnson $
//**
//** Bronze golem.
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

// Left hand punch attack
$frame gLpnch1      gLpnch2      gLpnch3      gLpnch4      gLpnch5
$frame gLpnch6      gLpnch7      gLpnch8      gLpnch9      gLpnch10
$frame gLpnch11     gLpnch12     gLpnch13     gLpnch14     gLpnch15
$frame gLpnch16     gLpnch17     gLpnch18     gLpnch19     gLpnch20
$frame gLpnch21     gLpnch22

// Right hand pound attack
$frame gRpnd1       gRpnd2       gRpnd3       gRpnd4       gRpnd5
$frame gRpnd6       gRpnd7       gRpnd8       gRpnd9       gRpnd10
$frame gRpnd11      gRpnd12      gRpnd13      gRpnd14      gRpnd15
$frame gRpnd16      gRpnd17      gRpnd18      gRpnd19      gRpnd20

// Stomp attack
$frame gstomp1      gstomp2      gstomp3      gstomp4      gstomp5
$frame gstomp6      gstomp7      gstomp8      gstomp9      gstomp10
$frame gstomp11     gstomp12     gstomp13     gstomp14     gstomp15
$frame gstomp16     gstomp17     gstomp18     gstomp19     gstomp20
$frame gstomp21     gstomp22     gstomp23     gstomp24     gstomp25
$frame gstomp26     gstomp27     gstomp28     gstomp29     gstomp30

void GolemBMeleeDecide(void);
void GolemBPunchLeft(void);
void GolemBPoundRight(void);

// CODE --------------------------------------------------------------------

void() golemb_gbirth1    = [ $gbirth1   , golemb_gbirth2    ] {  };
void() golemb_gbirth2    = [ $gbirth2   , golemb_gbirth3    ] {  };
void() golemb_gbirth3    = [ $gbirth3   , golemb_gbirth4    ] {  };
void() golemb_gbirth4    = [ $gbirth4   , golemb_gbirth5    ] {  };
void() golemb_gbirth5    = [ $gbirth5   , golemb_gbirth6    ] {  };
void() golemb_gbirth6    = [ $gbirth6   , golemb_gbirth7    ] {  };
void() golemb_gbirth7    = [ $gbirth7   , golemb_gbirth8    ] {  };
void() golemb_gbirth8    = [ $gbirth8   , golemb_gbirth9    ] {  };
void() golemb_gbirth9    = [ $gbirth9   , golemb_gbirth10   ] {  };
void() golemb_gbirth10   = [ $gbirth10  , golemb_gbirth11   ] {  };
void() golemb_gbirth11   = [ $gbirth11  , golemb_gbirth12   ] {  };
void() golemb_gbirth12   = [ $gbirth12  , golemb_gbirth1    ] {  };

/*
void() golemb_ggem1      = [ $ggem1     , golemb_ggem2      ] {  };
void() golemb_ggem2      = [ $ggem2     , golemb_ggem3      ] {  };
void() golemb_ggem3      = [ $ggem3     , golemb_ggem4      ] {  };
void() golemb_ggem4      = [ $ggem4     , golemb_ggem5      ] {  };
void() golemb_ggem5      = [ $ggem5     , golemb_ggem6      ] {  };
void() golemb_ggem6      = [ $ggem6     , golemb_ggem7      ] {  };
void() golemb_ggem7      = [ $ggem7     , golemb_ggem8      ] {  };
void() golemb_ggem8      = [ $ggem8     , golemb_ggem9      ] {  };
void() golemb_ggem9      = [ $ggem9     , golemb_ggem10     ] {  };
void() golemb_ggem10     = [ $ggem10    , golemb_ggem11     ] {  };
void() golemb_ggem11     = [ $ggem11    , golemb_ggem12     ] {  };
void() golemb_ggem12     = [ $ggem12    , golemb_ggem13     ] {  };
void() golemb_ggem13     = [ $ggem13    , golemb_ggem14     ] {  };
void() golemb_ggem14     = [ $ggem14    , golemb_ggem15     ] {  };
void() golemb_ggem15     = [ $ggem15    , golemb_ggem16     ] {  };
void() golemb_ggem16     = [ $ggem16    , golemb_ggem17     ] {  };
void() golemb_ggem17     = [ $ggem17    , golemb_ggem18     ] {  };
void() golemb_ggem18     = [ $ggem18    , golemb_ggem19     ] {  };
void() golemb_ggem19     = [ $ggem19    , golemb_ggem20     ] {  };
void() golemb_ggem20     = [ $ggem20    , golemb_ggem21     ] {  };
void() golemb_ggem21     = [ $ggem21    , golemb_ggem22     ] {  };
void() golemb_ggem22     = [ $ggem22    , golemb_ggem23     ] {  };
void() golemb_ggem23     = [ $ggem23    , golemb_ggem24     ] {  };
void() golemb_ggem24     = [ $ggem24    , golemb_ggem25     ] {  };
void() golemb_ggem25     = [ $ggem25    , golemb_ggem1      ] {  };
*/

/*
void() golemb_gLpnch1    = [ $gLpnch1   , golemb_gLpnch2    ] {  };
void() golemb_gLpnch2    = [ $gLpnch2   , golemb_gLpnch3    ] {  };
void() golemb_gLpnch3    = [ $gLpnch3   , golemb_gLpnch4    ] {  };
void() golemb_gLpnch4    = [ $gLpnch4   , golemb_gLpnch5    ] {  };
void() golemb_gLpnch5    = [ $gLpnch5   , golemb_gLpnch6    ] {  };
void() golemb_gLpnch6    = [ $gLpnch6   , golemb_gLpnch7    ] {  };
void() golemb_gLpnch7    = [ $gLpnch7   , golemb_gLpnch8    ] {  };
void() golemb_gLpnch8    = [ $gLpnch8   , golemb_gLpnch9    ] {  };
void() golemb_gLpnch9    = [ $gLpnch9   , golemb_gLpnch10   ] {  };
void() golemb_gLpnch10   = [ $gLpnch10  , golemb_gLpnch11   ] {  };
void() golemb_gLpnch11   = [ $gLpnch11  , golemb_gLpnch12   ] {  };
void() golemb_gLpnch12   = [ $gLpnch12  , golemb_gLpnch13   ] {  };
void() golemb_gLpnch13   = [ $gLpnch13  , golemb_gLpnch14   ] {  };
void() golemb_gLpnch14   = [ $gLpnch14  , golemb_gLpnch15   ] {  };
void() golemb_gLpnch15   = [ $gLpnch15  , golemb_gLpnch16   ] {  };
void() golemb_gLpnch16   = [ $gLpnch16  , golemb_gLpnch17   ] {  };
void() golemb_gLpnch17   = [ $gLpnch17  , golemb_gLpnch18   ] {  };
void() golemb_gLpnch18   = [ $gLpnch18  , golemb_gLpnch19   ] {  };
void() golemb_gLpnch19   = [ $gLpnch19  , golemb_gLpnch20   ] {  };
void() golemb_gLpnch20   = [ $gLpnch20  , golemb_gLpnch21   ] {  };
void() golemb_gLpnch21   = [ $gLpnch21  , golemb_gLpnch22   ] {  };
void() golemb_gLpnch22   = [ $gLpnch22  , golem_run_init    ] {  };
*/

/*
void() golemb_gRpnd1     = [ $gRpnd1    , golemb_gRpnd2     ] {  };
void() golemb_gRpnd2     = [ $gRpnd2    , golemb_gRpnd3     ] {  };
void() golemb_gRpnd3     = [ $gRpnd3    , golemb_gRpnd4     ] {  };
void() golemb_gRpnd4     = [ $gRpnd4    , golemb_gRpnd5     ] {  };
void() golemb_gRpnd5     = [ $gRpnd5    , golemb_gRpnd6     ] {  };
void() golemb_gRpnd6     = [ $gRpnd6    , golemb_gRpnd7     ] {  };
void() golemb_gRpnd7     = [ $gRpnd7    , golemb_gRpnd8     ] {  };
void() golemb_gRpnd8     = [ $gRpnd8    , golemb_gRpnd9     ] {  };
void() golemb_gRpnd9     = [ $gRpnd9    , golemb_gRpnd10    ] {  };
void() golemb_gRpnd10    = [ $gRpnd10   , golemb_gRpnd11    ] {  };
void() golemb_gRpnd11    = [ $gRpnd11   , golemb_gRpnd12    ] {  };
void() golemb_gRpnd12    = [ $gRpnd12   , golemb_gRpnd13    ] {  };
void() golemb_gRpnd13    = [ $gRpnd13   , golemb_gRpnd14    ] {  };
void() golemb_gRpnd14    = [ $gRpnd14   , golemb_gRpnd15    ] {  };
void() golemb_gRpnd15    = [ $gRpnd15   , golemb_gRpnd16    ] {  };
void() golemb_gRpnd16    = [ $gRpnd16   , golemb_gRpnd17    ] {  };
void() golemb_gRpnd17    = [ $gRpnd17   , golemb_gRpnd18    ] {  };
void() golemb_gRpnd18    = [ $gRpnd18   , golemb_gRpnd19    ] {  };
void() golemb_gRpnd19    = [ $gRpnd19   , golemb_gRpnd20    ] {  };
void() golemb_gRpnd20    = [ $gRpnd20   , golem_run_init    ] {  };
*/

/*
void() golemb_gstomp1    = [ $gstomp1   , golemb_gstomp2    ] {  };
void() golemb_gstomp2    = [ $gstomp2   , golemb_gstomp3    ] {  };
void() golemb_gstomp3    = [ $gstomp3   , golemb_gstomp4    ] {  };
void() golemb_gstomp4    = [ $gstomp4   , golemb_gstomp5    ] {  };
void() golemb_gstomp5    = [ $gstomp5   , golemb_gstomp6    ] {  };
void() golemb_gstomp6    = [ $gstomp6   , golemb_gstomp7    ] {  };
void() golemb_gstomp7    = [ $gstomp7   , golemb_gstomp8    ] {  };
void() golemb_gstomp8    = [ $gstomp8   , golemb_gstomp9    ] {  };
void() golemb_gstomp9    = [ $gstomp9   , golemb_gstomp10   ] {  };
void() golemb_gstomp10   = [ $gstomp10  , golemb_gstomp11   ] {  };
void() golemb_gstomp11   = [ $gstomp11  , golemb_gstomp12   ] {  };
void() golemb_gstomp12   = [ $gstomp12  , golemb_gstomp13   ] {  };
void() golemb_gstomp13   = [ $gstomp13  , golemb_gstomp14   ] {  };
void() golemb_gstomp14   = [ $gstomp14  , golemb_gstomp15   ] {  };
void() golemb_gstomp15   = [ $gstomp15  , golemb_gstomp16   ] {  };
void() golemb_gstomp16   = [ $gstomp16  , golemb_gstomp17   ] {  };
void() golemb_gstomp17   = [ $gstomp17  , golemb_gstomp18   ] {  };
void() golemb_gstomp18   = [ $gstomp18  , golemb_gstomp19   ] {  };
void() golemb_gstomp19   = [ $gstomp19  , golemb_gstomp20   ] {  };
void() golemb_gstomp20   = [ $gstomp20  , golemb_gstomp21   ] {  };
void() golemb_gstomp21   = [ $gstomp21  , golemb_gstomp22   ] {  };
void() golemb_gstomp22   = [ $gstomp22  , golemb_gstomp23   ] {  };
void() golemb_gstomp23   = [ $gstomp23  , golemb_gstomp24   ] {  };
void() golemb_gstomp24   = [ $gstomp24  , golemb_gstomp25   ] {  };
void() golemb_gstomp25   = [ $gstomp25  , golemb_gstomp26   ] {  };
void() golemb_gstomp26   = [ $gstomp26  , golemb_gstomp27   ] {  };
void() golemb_gstomp27   = [ $gstomp27  , golemb_gstomp28   ] {  };
void() golemb_gstomp28   = [ $gstomp28  , golemb_gstomp29   ] {  };
void() golemb_gstomp29   = [ $gstomp29  , golemb_gstomp30   ] {  };
void() golemb_gstomp30   = [ $gstomp30  , golemb_gstomp1    ] {  };
*/

/*
void() golemb_gtran1     = [ $gtran1    , golemb_gtran2     ] {  };
void() golemb_gtran2     = [ $gtran2    , golemb_gtran3     ] {  };
void() golemb_gtran3     = [ $gtran3    , golemb_gtran4     ] {  };
void() golemb_gtran4     = [ $gtran4    , golemb_gtran5     ] {  };
void() golemb_gtran5     = [ $gtran5    , golemb_gtran6     ] {  };
void() golemb_gtran6     = [ $gtran6    , golemb_gtran7     ] {  };
void() golemb_gtran7     = [ $gtran7    , golemb_gtran8     ] {  };
void() golemb_gtran8     = [ $gtran8    , golemb_gtran9     ] {  };
void() golemb_gtran9     = [ $gtran9    , golemb_gtran10    ] {  };
void() golemb_gtran10    = [ $gtran10   , golemb_gtran11    ] {  };
void() golemb_gtran11    = [ $gtran11   , golemb_gtran12    ] {  };
void() golemb_gtran12    = [ $gtran12   , golemb_gtran13    ] {  };
void() golemb_gtran13    = [ $gtran13   , golemb_gtran14    ] {  };
void() golemb_gtran14    = [ $gtran14   , golemb_gtran15    ] {  };
void() golemb_gtran15    = [ $gtran15   , golemb_gtran16    ] {  };
void() golemb_gtran16    = [ $gtran16   , golemb_gtran17    ] {  };
void() golemb_gtran17    = [ $gtran17   , golemb_gtran18    ] {  };
void() golemb_gtran18    = [ $gtran18   , golemb_gtran19    ] {  };
void() golemb_gtran19    = [ $gtran19   , golemb_gtran20    ] {  };
void() golemb_gtran20    = [ $gtran20   , golemb_gtran21    ] {  };
void() golemb_gtran21    = [ $gtran21   , golemb_gtran22    ] {  };
void() golemb_gtran22    = [ $gtran22   , golemb_gtran23    ] {  };
void() golemb_gtran23    = [ $gtran23   , golemb_gtran24    ] {  };
void() golemb_gtran24    = [ $gtran24   , golemb_gtran25    ] {  };
void() golemb_gtran25    = [ $gtran25   , golemb_gtran26    ] {  };
void() golemb_gtran26    = [ $gtran26   , golemb_gtran1     ] {  };
*/

void() golemb_gstand1 = [ $gbirth1, golemb_gstand1 ] { ai_stand(); };

//==========================================================================
//
// GolemBMeleeDecide
//
//==========================================================================

void GolemBMeleeDecide(void)
{
	if(random() > 0.5)
	{
		GolemBPunchLeft();
	}
	else
	{
		GolemBPoundRight();
	}
}

//==========================================================================
//
// GolemBPunchLeft
//
//==========================================================================

void GolemBPunchLeft(void) [++ $glpnch1..$glpnch22]
{
	if(cycle_wrapped)
	{
		golem_run_init();
		return;
	}
	if(self.frame == $glpnch8)
	{
		sound(self, CHAN_BODY, "golem/swing.wav", 1, ATTN_NORM);
	}
	if(self.frame > $glpnch9 && self.frame < $glpnch18)
	{
		ai_charge(2);
		ai_melee();
	}
	ai_face();
}

//==========================================================================
//
// GolemBPoundRight
//
//==========================================================================

void GolemBPoundRight(void) [++ $grpnd1..$grpnd20]
{
	if(cycle_wrapped)
	{
		golem_run_init();
		return;
	}
	if(self.frame == $grpnd8)
	{
		sound(self, CHAN_BODY, "golem/swing.wav", 1, ATTN_NORM);
	}
	if(self.frame > $grpnd11 && self.frame < $grpnd17)
	{
		ai_charge(2);
		ai_melee();
	}
	ai_face();
}

//==========================================================================
//
// golemb_pain
//
//==========================================================================

void golemb_pain(void)
{
	if(self.pain_finished > time)
	{
		return;
	}
	if(golem_flinch($gwalk1, $gwalk60)) return;
	if(golem_flinch($ggem1, $ggem25)) return;
	if(golem_flinch($gLpnch1, $gLpnch22)) return;
	if(golem_flinch($gRpnd1, $gRpnd20)) return;
	golem_flinch($gstomp1, $gstomp30);
}

//==========================================================================
//
// monster_golem_bronze
//
//==========================================================================
/*QUAKED monster_golem_bronze (1 0 0) (-32 -32 -24) (32 32 64) AMBUSH
Bronze Golem.
------- key / value ----------------------------
health = 500
experience_value = 275
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_bronze(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2("models/golem_b.mdl");
//	precache_model2("models/h_golem.mdl"); // empty for now

	precache_sound2("golem/step.wav");
	precache_sound2("golem/swing.wav");
	precache_sound2("golem/mtlfall.wav");
	precache_sound2("golem/stomp.wav");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.thingtype = THINGTYPE_METAL;
	self.yaw_speed = 10;
	self.mass = 50;
	self.mintel = 2;

	setmodel(self, "models/golem_b.mdl");

	setsize(self, '-20 -20 0', '20 20 80');

	self.health = 500;
	self.experience_value = 275;

	self.th_stand = golemb_gstand1;
	self.th_walk = golem_walk;
	self.th_run = golem_run_init;
	self.th_die = golem_die_init;
	self.th_melee = GolemBMeleeDecide;
	self.th_pain = golemb_pain;
	self.view_ofs = '0 0 64';

	walkmonster_start();
}
