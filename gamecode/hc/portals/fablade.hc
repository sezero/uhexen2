/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/fablade.hc,v 1.1.1.1 2004-11-29 11:31:23 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\Fangel\wingblad\FAblade.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\Fangel\wingblad
$origin 0 0 0
$base BASE skin1
$skin skin1
$flags 0

//
$frame BLADE        


void() faBladeTouch =
{
float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(8,16);
	
	if (other.health)
		T_Damage (other, self, self.owner, damg );


	sound (self, CHAN_WEAPON, "weapons/expsmall.wav", 1, ATTN_NORM);
	self.origin = self.origin - 8*normalize(self.velocity);
	CreateGreenSmoke(self.origin, '0 0 8', HX_FRAME_TIME * 4);
	remove(self);
};


// Frame Code
//void() frame_BLADE      = [ $BLADE     , frame_BLADE      ] {  };




void(vector offset, float set_speed, vector dest_offset) do_faBlade =
{
entity missile;
vector vec;

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.flags = FL_FLY;
	missile.health = 10;
	missile.drawflags=MLS_ABSLIGHT;
	missile.abslight=0.5;

	setmodel (missile, "models/fablade.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

// set missile speed	

	makevectors (self.angles);
	setorigin (missile, self.origin + v_factor(offset));

   vec = self.enemy.origin - missile.origin + self.enemy.proj_ofs + dest_offset;
	vec = normalize(vec);

	missile.velocity = (vec+aim_adjust(self.enemy))*set_speed;
	missile.angles = vectoangles(missile.velocity);
	
	missile.touch = faBladeTouch;
};


/*
 * $Log: not supported by cvs2svn $
 * 
 * 2     2/12/98 5:55p Jmonroe
 * remove unreferenced funcs
 * 
 * 15    10/28/97 1:00p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 13    9/04/97 3:19p Mgummelt
 * 
 * 12    9/04/97 3:00p Mgummelt
 * 
 * 11    8/29/97 12:59a Mgummelt
 * 
 * 10    8/27/97 7:07p Mgummelt
 * 
 * 9     8/05/97 12:04p Rlove
 * 
 * 8     8/04/97 12:19p Rlove
 * 
 * 6     6/19/97 3:08p Rjohnson
 * Code space optimizations
 * 
 * 5     5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 4     5/06/97 1:29p Mgummelt
 * 
 * 3     4/24/97 2:21p Mgummelt
 * 
 * 2     3/12/97 10:57p Rjohnson
 * Doesn't spawn the explosion or do radius damage
 * 
 * 1     3/12/97 4:55p Rjohnson
 * Initial Version
 */
