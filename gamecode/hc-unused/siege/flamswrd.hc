/*
 * $Header: /HexenWorld/Siege/flamswrd.hc 3     5/25/98 1:38p Mgummelt $
 */

/*
==============================================================================

FLAME SWORD

==============================================================================
*/

// For building the model
$cd q:\art\models\weapons\flamswrd\final
$origin 0 0 54
$base base
$skin skin

//============================================================================

$frame swg000   swg001   swg002   swg003   swg004
$frame swg005   swg006   swg007   swg008   swg009
$frame swg010   swg011   swg012   swg013   swg014
$frame swg015   swg016  


//frame bswg000  bswg001   bswg002   bswg003   bswg004
//$frame bswg005  bswg006   bswg007   bswg008   bswg009
//$frame bswg010  bswg011   bswg012   bswg013   bswg014
//$frame bswg015  


//============================================================================

// Flame Sword
void()	player_swing1 =	[$swg000, player_swing2	] {self.weaponframe=1;};
void()	player_swing2 =	[$swg001, player_swing3	] {self.weaponframe=2;};
void()	player_swing3 =	[$swg002, player_swing4	] {self.weaponframe=3;};
void()	player_swing4 =	[$swg003, player_swing5	] {self.weaponframe=4;};
void()	player_swing5 =	[$swg004, player_swing6	] {self.weaponframe=5;};
void()	player_swing6 =	[$swg005, player_swing7	] {self.weaponframe=6;};
void()	player_swing7 =	[$swg006, player_swing8	] {self.weaponframe=7;};
void()	player_swing8 =	[$swg007, player_swing9	] {self.weaponframe=8;};
void()	player_swing9 =	[$swg008, player_swing10  ] {self.weaponframe=9;};
void()	player_swing10 =	[$swg009, player_swing11	] {self.weaponframe=10;};
void()	player_swing11 =	[$swg010, player_swing12	] {self.weaponframe=11;};
void()	player_swing12 =	[$swg011, player_swing13	] {self.weaponframe=12;};
void()	player_swing13 =	[$swg012, player_swing14	] {self.weaponframe=13;};
void()	player_swing14 =	[$swg013, player_swing15	] {self.weaponframe=14;};
void()	player_swing15 =	[$swg014, player_swing16	] {self.weaponframe=15;};
void()	player_swing16 =	[$swg015, player_swing1	] {self.weaponframe=16;};

