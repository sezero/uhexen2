/*
 * $Header: /HexenWorld/Siege/javelin.hc 3     5/25/98 1:39p Mgummelt $
 */
/*
==============================================================================

GAUNTLET

==============================================================================
*/
$cd q:/art/models/weapons/javelin/final
$origin 0 5 10
//$rotatehtr 90 180 0 
$base base skin
$skin skin

$frame jav000  jav001  jav002  jav003  jav004
$frame jav005  jav006  jav007  jav008  jav009
$frame jav010  jav011  jav012  jav013  jav014 
$frame jav015  jav016  jav017  jav018
$frame jav019  jav020

void() W_FireJavelin;
void() T_MissileTouch;

//============================================================================
void()  throwjav0  =[ $jav000, throwjav1    ] {self.weaponframe=0;};
void()  throwjav1  =[ $jav001, throwjav2    ] {self.weaponframe=1;};
void()  throwjav2  =[ $jav002, throwjav3    ] {self.weaponframe=2;};
void()  throwjav3  =[ $jav003, throwjav4    ] {self.weaponframe=3;};
void()  throwjav4  =[ $jav004, throwjav5    ] {self.weaponframe=4;};
void()  throwjav5  =[ $jav005, throwjav6    ] {self.weaponframe=5;};
void()  throwjav6  =[ $jav006, throwjav7    ] {self.weaponframe=6;};
void()  throwjav7  =[ $jav007, throwjav8    ] {self.weaponframe=7;};
void()  throwjav8  =[ $jav008, throwjav9    ] {self.weaponframe=8;};
void()  throwjav9  =[ $jav009, throwjav10   ] {self.weaponframe=9;W_FireJavelin();};
void()  throwjav10 =[ $jav010, throwjav11   ] {self.weaponframe=10;};
void()  throwjav11 =[ $jav011, throwjav12   ] {self.weaponframe=11;};
void()  throwjav12 =[ $jav012, throwjav13   ] {self.weaponframe=12;};
void()  throwjav13 =[ $jav013, throwjav14   ] {self.weaponframe=13;};
void()  throwjav14 =[ $jav014, throwjav15   ] {self.weaponframe=14;};
void()  throwjav15 =[ $jav015, throwjav16   ] {self.weaponframe=15;};
void()  throwjav16 =[ $jav016, throwjav17   ] {self.weaponframe=16;};
void()  throwjav17 =[ $jav017, throwjav18   ] {self.weaponframe=17;};
void()  throwjav18 =[ $jav018, throwjav19   ] {self.weaponframe=18;};
void()  throwjav19 =[ $jav019, throwjav20   ] {self.weaponframe=19;};
void()  throwjav20 =[ $jav020, self.th_run   ] {self.weaponframe=20;};


//============================================================================
/*
================================
launch_javelin

================================
*/
void(vector org, vector dir) launch_javelin =
{
	local entity missile;

	missile = spawn ();
	missile.owner = self;
   missile.touch = T_MissileTouch;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;

	missile.angles = vectoangles(dir);

   missile.touch = T_MissileTouch;
   missile.classname = "javelin";

	org = org + '0 10 10';
	setmodel (missile, "models/javproj.mdl");
	setsize (missile, VEC_ORIGIN, VEC_ORIGIN);
	setorigin (missile, org);
	missile.velocity = dir * 800;

};

void() W_FireJavelin = 
{
	local vector    dir;
	local entity missile;

   sound (self, CHAN_WEAPON, "raven/javthrow.wav", 1, ATTN_NORM);
	self.punchangle_x = -2;

	self.attack_finished = time + 0.5;

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.classname = "javprojectile";
		
// set missile speed	
	makevectors (self.v_angle);
	missile.velocity = aim(self, 1000);
	missile.velocity = missile.velocity * 1000;
	missile.angles = vectoangles(missile.velocity);
	
	missile.touch = T_MissileTouch;
	
// set missile duration
	missile.nextthink = time + 5;
	missile.think = SUB_Remove;

	setmodel (missile, "models/javproj.mdl");
	setsize (missile, '0 0 0', '0 0 0');		
	setorigin (missile, self.origin + v_forward*-14 + v_right * 16 + v_up * 32);

};

/*QUAKED wp_javelin (1 0 0) (-16 -16 -24) (16 16 40) 
Javelin weapon for Paladin

-------------------------FIELDS-------------------------
none
--------------------------------------------------------

*/
void() wp_javelin =
{
	if (deathmatch)
	{
		remove(self);
		return;
   }

};

