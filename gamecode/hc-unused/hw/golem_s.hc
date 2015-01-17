
//**************************************************************************
//**
//** golem_s.hc
//**
//** $Header: /HexenWorld/HCode/golem_s.hc 1     2/04/98 1:59p Rjohnson $
//**
//** Stone golem.  Also contains generic golem code.
//**
//**************************************************************************

// FRAMES ------------------------------------------------------------------

// Transition from still to attack stance
$frame gbirth1 gbirth2 gbirth3 gbirth4 gbirth5
$frame gbirth6 gbirth7 gbirth8 gbirth9 gbirth10
$frame gbirth11 gbirth12

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

// Two hand pound attack
$frame gDpnd1       gDpnd2       gDpnd3       gDpnd4       gDpnd5
$frame gDpnd6       gDpnd7       gDpnd8       gDpnd9       gDpnd10
$frame gDpnd11      gDpnd12      gDpnd13      gDpnd14      gDpnd15
$frame gDpnd16      gDpnd17      gDpnd18      gDpnd19      gDpnd20

// Left hand punch attack
$frame gLpnch1      gLpnch2      gLpnch3      gLpnch4      gLpnch5
$frame gLpnch6      gLpnch7      gLpnch8      gLpnch9      gLpnch10
$frame gLpnch11     gLpnch12     gLpnch13     gLpnch14     gLpnch15
$frame gLpnch16     gLpnch17     gLpnch18     gLpnch19     gLpnch20
$frame gLpnch21     gLpnch22

// Right hand punch attack
$frame gRpnch1      gRpnch2      gRpnch3      gRpnch4      gRpnch5
$frame gRpnch6      gRpnch7      gRpnch8      gRpnch9      gRpnch10
$frame gRpnch11     gRpnch12     gRpnch13     gRpnch14     gRpnch15
$frame gRpnch16     gRpnch17     gRpnch18     gRpnch19     gRpnch20
$frame gRpnch21     gRpnch22

$framevalue 0

$frame tgrun1        tgrun2        tgrun3        tgrun4        tgrun5        
$frame tgrun6        tgrun7        tgrun8        tgrun9        tgrun10       
$frame tgrun11       tgrun12       tgrun13       tgrun14       tgrun15       
$frame tgrun16       tgrun17       tgrun18       tgrun19       tgrun20       
$frame tgrun21 tgrun22 tgrun23 tgrun24

$frame tgwalk1       tgwalk2       tgwalk3       tgwalk4       tgwalk5       
$frame tgwalk6       tgwalk7       tgwalk8       tgwalk9       tgwalk10      
$frame tgwalk11      tgwalk12      tgwalk13      tgwalk14      tgwalk15      
$frame tgwalk16      tgwalk17      tgwalk18      tgwalk19      tgwalk20      
$frame tgwalk21      tgwalk22      tgwalk23      tgwalk24      tgwalk25      
$frame tgwalk26      tgwalk27      tgwalk28      tgwalk29      tgwalk30      
$frame tgwalk31 tgwalk32 tgwalk33 tgwalk34

void GolemSPunchLeft(void);
void GolemSPunchRight(void);
void GolemSPound(void);
void golem_run_init(void);

// CODE --------------------------------------------------------------------

void() golems_gbirth1    = [ $gbirth1   , golems_gbirth2    ] {  };
void() golems_gbirth2    = [ $gbirth2   , golems_gbirth3    ] {  };
void() golems_gbirth3    = [ $gbirth3   , golems_gbirth4    ] {  };
void() golems_gbirth4    = [ $gbirth4   , golems_gbirth5    ] {  };
void() golems_gbirth5    = [ $gbirth5   , golems_gbirth6    ] {  };
void() golems_gbirth6    = [ $gbirth6   , golems_gbirth7    ] {  };
void() golems_gbirth7    = [ $gbirth7   , golems_gbirth8    ] {  };
void() golems_gbirth8    = [ $gbirth8   , golems_gbirth9    ] {  };
void() golems_gbirth9    = [ $gbirth9   , golems_gbirth10   ] {  };
void() golems_gbirth10   = [ $gbirth10  , golems_gbirth11   ] {  };
void() golems_gbirth11   = [ $gbirth11  , golems_gbirth12   ] {  };
void() golems_gbirth12   = [ $gbirth12  , golems_gbirth1    ] {  };

/*
void() golems_gDpnd1     = [ $gDpnd1    , golems_gDpnd2     ] {  };
void() golems_gDpnd2     = [ $gDpnd2    , golems_gDpnd3     ] {  };
void() golems_gDpnd3     = [ $gDpnd3    , golems_gDpnd4     ] {  };
void() golems_gDpnd4     = [ $gDpnd4    , golems_gDpnd5     ] {  };
void() golems_gDpnd5     = [ $gDpnd5    , golems_gDpnd6     ] {  };
void() golems_gDpnd6     = [ $gDpnd6    , golems_gDpnd7     ] {  };
void() golems_gDpnd7     = [ $gDpnd7    , golems_gDpnd8     ] {  };
void() golems_gDpnd8     = [ $gDpnd8    , golems_gDpnd9     ] {  };
void() golems_gDpnd9     = [ $gDpnd9    , golems_gDpnd10    ] {  };
void() golems_gDpnd10    = [ $gDpnd10   , golems_gDpnd11    ] {  };
void() golems_gDpnd11    = [ $gDpnd11   , golems_gDpnd12    ] {  };
void() golems_gDpnd12    = [ $gDpnd12   , golems_gDpnd13    ] {  };
void() golems_gDpnd13    = [ $gDpnd13   , golems_gDpnd14    ] {  };
void() golems_gDpnd14    = [ $gDpnd14   , golems_gDpnd15    ] {  };
void() golems_gDpnd15    = [ $gDpnd15   , golems_gDpnd16    ] {  };
void() golems_gDpnd16    = [ $gDpnd16   , golems_gDpnd17    ] {  };
void() golems_gDpnd17    = [ $gDpnd17   , golems_gDpnd18    ] {  };
void() golems_gDpnd18    = [ $gDpnd18   , golems_gDpnd19    ] {  };
void() golems_gDpnd19    = [ $gDpnd19   , golems_gDpnd20    ] {  };
void() golems_gDpnd20    = [ $gDpnd20   , golem_run_init    ] {  };

void() golems_gLpnch1    = [ $gLpnch1   , golems_gLpnch2    ] {  };
void() golems_gLpnch2    = [ $gLpnch2   , golems_gLpnch3    ] {  };
void() golems_gLpnch3    = [ $gLpnch3   , golems_gLpnch4    ] {  };
void() golems_gLpnch4    = [ $gLpnch4   , golems_gLpnch5    ] {  };
void() golems_gLpnch5    = [ $gLpnch5   , golems_gLpnch6    ] {  };
void() golems_gLpnch6    = [ $gLpnch6   , golems_gLpnch7    ] {  };
void() golems_gLpnch7    = [ $gLpnch7   , golems_gLpnch8    ] {  };
void() golems_gLpnch8    = [ $gLpnch8   , golems_gLpnch9    ] {  };
void() golems_gLpnch9    = [ $gLpnch9   , golems_gLpnch10   ] {  };
void() golems_gLpnch10   = [ $gLpnch10  , golems_gLpnch11   ] {  };
void() golems_gLpnch11   = [ $gLpnch11  , golems_gLpnch12   ] {  };
void() golems_gLpnch12   = [ $gLpnch12  , golems_gLpnch13   ] {  };
void() golems_gLpnch13   = [ $gLpnch13  , golems_gLpnch14   ] {  };
void() golems_gLpnch14   = [ $gLpnch14  , golems_gLpnch15   ] {  };
void() golems_gLpnch15   = [ $gLpnch15  , golems_gLpnch16   ] {  };
void() golems_gLpnch16   = [ $gLpnch16  , golems_gLpnch17   ] {  };
void() golems_gLpnch17   = [ $gLpnch17  , golems_gLpnch18   ] {  };
void() golems_gLpnch18   = [ $gLpnch18  , golems_gLpnch19   ] {  };
void() golems_gLpnch19   = [ $gLpnch19  , golems_gLpnch20   ] {  };
void() golems_gLpnch20   = [ $gLpnch20  , golems_gLpnch21   ] {  };
void() golems_gLpnch21   = [ $gLpnch21  , golems_gLpnch22   ] {  };
void() golems_gLpnch22   = [ $gLpnch22  , golem_run_init    ] {  };

void()
	golems_gRpnch1    = [$gRpnch1    golems_gRpnch2    ],
	golems_gRpnch2    = [$gRpnch2    golems_gRpnch3    ],
	golems_gRpnch3    = [$gRpnch3    golems_gRpnch4    ],
	golems_gRpnch4    = [$gRpnch4    golems_gRpnch5    ],
	golems_gRpnch5    = [$gRpnch5    golems_gRpnch6    ],
	golems_gRpnch6    = [$gRpnch6    golems_gRpnch7    ],
	golems_gRpnch7    = [$gRpnch7    golems_gRpnch8    ],
	golems_gRpnch8    = [$gRpnch8    golems_gRpnch9    ],
	golems_gRpnch9    = [$gRpnch9    golems_gRpnch10   ],
	golems_gRpnch10   = [$gRpnch10   golems_gRpnch11   ],
	golems_gRpnch11   = [$gRpnch11   golems_gRpnch12   ],
	golems_gRpnch12   = [$gRpnch12   golems_gRpnch13   ],
	golems_gRpnch13   = [$gRpnch13   golems_gRpnch14   ],
	golems_gRpnch14   = [$gRpnch14   golems_gRpnch15   ],
	golems_gRpnch15   = [$gRpnch15   golems_gRpnch16   ],
	golems_gRpnch16   = [$gRpnch16   golems_gRpnch17   ],
	golems_gRpnch17   = [$gRpnch17   golems_gRpnch18   ],
	golems_gRpnch18   = [$gRpnch18   golems_gRpnch19   ],
	golems_gRpnch19   = [$gRpnch19   golems_gRpnch20   ],
	golems_gRpnch20   = [$gRpnch20   golems_gRpnch21   ],
	golems_gRpnch21   = [$gRpnch21   golems_gRpnch22   ],
	golems_gRpnch22   = [$gRpnch22   golem_run_init    ];
*/

/*
void() golems_gtran1     = [ $gtran1    , golems_gtran2     ] {  };
void() golems_gtran2     = [ $gtran2    , golems_gtran3     ] {  };
void() golems_gtran3     = [ $gtran3    , golems_gtran4     ] {  };
void() golems_gtran4     = [ $gtran4    , golems_gtran5     ] {  };
void() golems_gtran5     = [ $gtran5    , golems_gtran6     ] {  };
void() golems_gtran6     = [ $gtran6    , golems_gtran7     ] {  };
void() golems_gtran7     = [ $gtran7    , golems_gtran8     ] {  };
void() golems_gtran8     = [ $gtran8    , golems_gtran9     ] {  };
void() golems_gtran9     = [ $gtran9    , golems_gtran10    ] {  };
void() golems_gtran10    = [ $gtran10   , golems_gtran11    ] {  };
void() golems_gtran11    = [ $gtran11   , golems_gtran12    ] {  };
void() golems_gtran12    = [ $gtran12   , golems_gtran13    ] {  };
void() golems_gtran13    = [ $gtran13   , golems_gtran14    ] {  };
void() golems_gtran14    = [ $gtran14   , golems_gtran15    ] {  };
void() golems_gtran15    = [ $gtran15   , golems_gtran16    ] {  };
void() golems_gtran16    = [ $gtran16   , golems_gtran17    ] {  };
void() golems_gtran17    = [ $gtran17   , golems_gtran18    ] {  };
void() golems_gtran18    = [ $gtran18   , golems_gtran19    ] {  };
void() golems_gtran19    = [ $gtran19   , golems_gtran20    ] {  };
void() golems_gtran20    = [ $gtran20   , golems_gtran21    ] {  };
void() golems_gtran21    = [ $gtran21   , golems_gtran22    ] {  };
void() golems_gtran22    = [ $gtran22   , golems_gtran23    ] {  };
void() golems_gtran23    = [ $gtran23   , golems_gtran24    ] {  };
void() golems_gtran24    = [ $gtran24   , golems_gtran25    ] {  };
void() golems_gtran25    = [ $gtran25   , golems_gtran26    ] {  };
void() golems_gtran26    = [ $gtran26   , golems_gtran1     ] {  };
*/

void() golems_gstand1 = [ $gbirth1, golems_gstand1 ]
{
	ai_stand();
};

//==========================================================================
//
// GolemSMeleeDecide
//
//==========================================================================

void GolemSMeleeDecide(void)
{
	float rnd;

	rnd = random();
	if(rnd > 0.66)
	{
		GolemSPunchLeft();
	}
	else if(rnd > 0.33)
	{
		GolemSPunchRight();
	}
	else
	{
		GolemSPound();
	}
}

//==========================================================================
//
// GolemSPunchLeft
//
//==========================================================================

void GolemSPunchLeft(void) [++ $glpnch1..$glpnch22]
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
// GolemSPunchRight
//
//==========================================================================

void GolemSPunchRight(void) [++ $grpnch1..$grpnch22]
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
// GolemSPound
//
//==========================================================================

void GolemSPound(void) [++ $gdpnd1..$gdpnd20]
{
	if(cycle_wrapped)
	{
		golem_run_init();
		return;
	}
	if(self.frame == $gdpnd8)
	{
		sound(self, CHAN_BODY, "golem/swing.wav", 1, ATTN_NORM);
	}
	if(self.frame > $gdpnd9 && self.frame < $gdpnd18)
	{
		ai_charge(2);
		ai_melee();
	}
	ai_face();
}

//==========================================================================
//
// golem_run
//
//==========================================================================

void golem_run(void)
{
	vector tvec;

	if(self.count == 20)
	{
		if(self.frame == $tgrun24)
		{
			self.frame = $tgrun1;
		}
		else
		{
			self.frame += 1;
		}
		ai_run(6);
	}
	else
	{
		if(self.frame == $tgwalk34)
		{
			self.frame = $tgwalk1;
		}
		else
		{
			self.frame += 1;
		}
		ai_run(3);
	}


//	if(self.frame == $gwalk27 || self.frame == $gwalk58)
//	{
//		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
//	}

	thinktime self : HX_FRAME_TIME;
}

void golem_run_init(void)
{
	if(self.count == 20)
	{
		if(self.frame < $tgrun1 || self.frame > $tgrun24)
		{
			self.frame = $tgrun1;
		}
		ai_run(2);
	}
	else
	{
		if(self.frame < $tgwalk1 || self.frame > $tgwalk34)
		{
			self.frame = $tgwalk1;
		}
		ai_run(1);
	}

	thinktime self : HX_FRAME_TIME;
	self.think = golem_run;
}

//==========================================================================
//
// golem_walk
//
//==========================================================================

void golem_walk(void) [++ $gwalk1..$gwalk60]
{
	if(self.frame == $gwalk27 || self.frame == $gwalk58)
	{
		sound(self, CHAN_BODY, "golem/step.wav", 1, ATTN_NORM);
	}
	ai_walk(1);
}

//==========================================================================
//
// golem_die
//
//==========================================================================

void golem_die(void)
{
	if(self.frame == $gdeath51)
	{
		self.nextthink = time - 1;
/*		if(self.classname == "monster_golem_stone")
		{
		}
		else if(self.classname == "monster_golem_iron")
		{
		}
		else
		{ // Assumed bronze
		}
*/
		MakeSolidCorpse();
		return;
	}
	if(self.frame == $gdeath36)
	{
		self.solid = SOLID_NOT;
	}
	if(self.health<-50)
		self.think=chunk_death;
	if(self.flags&FL_ONGROUND)
		self.frame += 1;
	self.nextthink = time + HX_FRAME_TIME;
}

void golem_die_init(void) [ $gdeath1, golem_die ]
{
}

//==========================================================================
//
// golem_flinch
//
//==========================================================================

float golem_flinch(float firstFrame, float lastFrame)
{
	if(self.frame < firstFrame || self.frame > lastFrame)
	{
		return 0;
	}
	self.nextthink = self.nextthink+0.1+random()*0.2;
	self.frame = self.frame - 8 - rint(random() * 12);
	self.pain_finished = time + 1;
	if(self.frame < firstFrame)
	{ // Wrap
		self.frame = lastFrame + 1 - (firstFrame - self.frame);
	}
	return 1;
}

//==========================================================================
//
// golems_pain
//
//==========================================================================

void golems_pain(void)
{

/*	float i;

	i = (self.drawflags&MLS_MASKIN)+1;
	if(i > 7)
	{
		i = 0;
	}
	self.drawflags = (self.drawflags&MLS_MASKOUT)|i;
	dprint("mls=");
	dprint(ftos(i));
	dprint("\n");
*/
	if(self.count == 20)
	{
		self.count = 0;
	}
	else
	{
		self.count = 20;
	}


	if(self.pain_finished > time)
	{
		return;
	}
	if(golem_flinch($gwalk1, $gwalk60)) return;
	if(golem_flinch($gLpnch1, $gLpnch22)) return;
	if(golem_flinch($gRpnch1, $gRpnch22)) return;
	golem_flinch($gDpnd1, $gDpnd20);
}

//==========================================================================
//
// monster_golem_stone
//
//==========================================================================
/*QUAKED monster_golem_stone (1 0 0) (-32 -32 -24) (32 32 64) AMBUSH
Stone Golem.
------- key / value ----------------------------
health = 200
experience_value = 125
------- spawnflags -----------------------------
AMBUSH
*/

void monster_golem_stone(void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	//precache_model("models/golem_s.mdl");
	precache_model("models/goltest.mdl");
//	precache_model("models/h_golem.mdl"); // empty for now

	precache_sound2("golem/awaken.wav");
	precache_sound2("golem/step.wav");
	precache_sound2("golem/swing.wav");
	precache_sound2("golem/stnpain.wav");

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.thingtype = THINGTYPE_GREYSTONE;
	self.yaw_speed = 10;
	self.mass = 75;
	self.mintel = 2;

	//setmodel(self, "models/golem_s.mdl");
	setmodel(self, "models/goltest.mdl");

	//setsize(self, '-20 -20 0', '20 20 80');
	setsize(self, '-20 -20 -20', '20 20 60');

	self.scale = 1.5;

	self.health = 200;
	self.experience_value = 125;

	self.th_stand = golems_gstand1;
	self.th_walk = golem_walk;
	self.th_run = golem_run_init;
	self.th_die = golem_die_init;
	self.th_melee = GolemSMeleeDecide;
	//self.th_missile = golems_run_init;
	self.th_pain = golems_pain;
	self.view_ofs = '0 0 64';

	walkmonster_start();
}
