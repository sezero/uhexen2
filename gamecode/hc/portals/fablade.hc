/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/fablade.hc,v 1.2 2007-02-07 16:59:31 sezero Exp $
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
		T_Damage (other, self, self.owner, damg);

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

