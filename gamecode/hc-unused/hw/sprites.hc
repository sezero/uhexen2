/*
 * $Header: /HexenWorld/HCode/SPRITES.hc 1     2/04/98 1:59p Rjohnson $
 */

// these are the only sprites still in the game...

//$spritename s_explod
//$type vp_parallel
//$load id1/gfx/sprites/explod03.lbm
//$frame	24	24	56	56
//$frame	120	24	56	56
//$frame	216	24	56	56
//$frame	24	88	56	56
//$frame	120	88	56	56
//$frame	216	88	56	56


//$spritename s_bubble
//$type vp_parallel
//$load id1/gfx/sprites/bubble.lbm
//$frame	16	16	16	16
//$frame	40	16	16	16


//$spritename s_light
//$type vp_parallel
//$load id1/gfx/sprites/light.lbm
//$frame	104	32	32	32

/*QUAK-ED sprite_tree1 (0.3 0.1 0.6) (-32 -32 -95) (32 32 90)
A tree 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_tree1 =
{

	precache_model ("gfx/tree1.spr");
	setmodel (self, "gfx/tree1.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = chunk_death;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -95','32 32 90');

};
*/

/*QUAK-ED sprite_tree2 (0.3 0.1 0.6) (-32 -32 -95) (32 32 90)
A tree 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_tree2 =
{

	precache_model ("gfx/tree2.spr");
	setmodel (self, "gfx/tree2.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = chunk_death;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -95','32 32 90');

};
*/

/*QUAK-ED sprite_treetop (0.3 0.1 0.6) (-32 -32 -10) (32 32 10)
A tree top
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_treetop =
{

	precache_model ("gfx/treetop.spr");
	setmodel (self, "gfx/treetop.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.angles_x = -90;

	self.th_die = SUB_Null;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -95','32 32 90');

};
*/
/*QUAK-ED sprite_backgrnd_1 (0.3 0.1 0.6) (-48 -48 -16) (48 48 16)
A sprite background
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_backgrnd_1 =
{

	//rj precache_model ("gfx/bckgrnd1.spr");
	setmodel (self, "gfx/bckgrnd1.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = SUB_Null;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -16','32 32 16');

};
*/

/*QUAK-ED sprite_backgrnd_2 (0.3 0.1 0.6) (-48 -48 -16) (48 48 16)
Another sprite background
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_backgrnd_2 =
{

	//rj precache_model ("gfx/bckgrnd2.spr");
	setmodel (self, "gfx/bckgrnd2.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = SUB_Null;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -16','32 32 16');

};
*/

/*QUAK-ED sprite_moss1 (0.3 0.1 0.6) (-16 -16 -10) (16 16 10)
Moss
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
/*
void() sprite_moss1 =
{

	precache_model ("gfx/moss1.spr");
	setmodel (self, "gfx/moss1.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = SUB_Null;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-16 -16 -10','16 16 10');

};
*/

