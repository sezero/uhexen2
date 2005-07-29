/*
 * $Header: /home/ozzie/Download/0000/uhexen2/00_unused/gamecode/portals/sprites.hc,v 1.1.1.1 2005-07-29 20:45:29 sezero Exp $
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

/*QUAK-ED sprite_talking_door (0.3 0.1 0.6) (-16 -16 -10) (16 16 10)
Talking door animation
-------------------------FIELDS-------------------------
netname: name of target to movechain to
--------------------------------------------------------
*/
void sprite_talking_door(void)
{
	setmodel (self, "gfx/bckgrnd2.spr");

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;

	self.th_die = SUB_Null;
	self.touch = SUB_Null;
	self.use= SUB_Null;
	setsize (self, '-32 -32 -16','32 32 16');
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/29 11:34:59  sezero
 * Initial import
 *
 * 
 * 2     1/23/98 11:55a Jweier
 * 
 * 12    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 10    8/23/97 7:15p Rlove
 * 
 * 9     6/18/97 10:46a Rjohnson
 * Code cleanu
 * 
 * 8     3/25/97 4:58p Rjohnson
 * Cleaned up pre-cache stuff
 * 
 * 7     3/21/97 10:19a Rlove
 * Changed obj_die calls to chunk_death
 * 
 * 6     3/10/97 8:53a Rlove
 * Added sprite moss
 * 
 * 5     3/04/97 9:11a Rlove
 * Added sprite background 1 & 2
 * 
 * 4     3/03/97 1:59p Rlove
 * Added sprite treetop
 * 
 * 3     2/21/97 2:23p Rlove
 * Added sprite trees and models chests (please no jokes)
 * 
 * 2     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */

