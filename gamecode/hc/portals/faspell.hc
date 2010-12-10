/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/faspell.hc,v 1.2 2007-02-07 16:59:32 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\Fangel\spell\FAspell.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\Fangel\spell
$origin 0 0 0
$base BASE skin1
$skin skin1
$flags 0

//
$frame SPELL01      SPELL02      SPELL03      SPELL04      SPELL05      
$frame SPELL06      SPELL07      SPELL08      SPELL09      



void() faSpellTouch =
{
float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(12,22);
	
	if (other.health)
		T_Damage (other, self, self.owner, damg );

	T_RadiusDamage (self, self.owner, damg, other);

	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	self.origin = self.origin - 8*normalize(self.velocity);
	CreateRedSpark(self.origin);
	remove(self);
};

// Frame Code
/*
void() frame_SPELL01    = [ $SPELL01   , frame_SPELL02    ] {  };
void() frame_SPELL02    = [ $SPELL02   , frame_SPELL03    ] {  };
void() frame_SPELL03    = [ $SPELL03   , frame_SPELL04    ] {  };
void() frame_SPELL04    = [ $SPELL04   , frame_SPELL05    ] {  };
void() frame_SPELL05    = [ $SPELL05   , frame_SPELL06    ] {  };
void() frame_SPELL06    = [ $SPELL06   , frame_SPELL07    ] {  };
void() frame_SPELL07    = [ $SPELL07   , frame_SPELL08    ] {  };
void() frame_SPELL08    = [ $SPELL08   , frame_SPELL09    ] {  };
void() frame_SPELL09    = [ $SPELL09   , frame_SPELL01    ] {  };
*/


void() faspell_frames =
{
	float old_angle, old_count;
	vector new_posA, new_posB, posA, posB;

	self.think = faspell_frames;
	thinktime self : HX_FRAME_TIME;

	AdvanceFrame($SPELL01,$SPELL09);
	AdvanceFrame($SPELL01,$SPELL09);

	old_angle = self.spell_angle;
	old_count = self.count;
	self.spell_angle += random(32,42);
	if (self.spell_angle >= 360) self.spell_angle -= 360;
	if (self.count < 6)
	   self.count += 0.6;

	makevectors (self.angles);

	posA = v_right * sin(self.spell_angle) * self.count;
	posB = v_right * sin(old_angle) * old_count;
	new_posA = (posA-posB);
	posA = v_up * cos(self.spell_angle) * self.count;
	posB = v_up * cos(old_angle) * old_count;
	new_posB = (posA-posB);
	new_posA += new_posB;
	movestep(new_posA_x,new_posA_y,new_posA_z, FALSE);

	if(self.lifetime<time)
		remove(self);
};





void(vector offset, float set_speed) do_faSpell =
{
entity missile;
vector vec;

	self.last_attack=time;
	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.flags = FL_FLY;
	missile.health = 10;
	missile.drawflags=MLS_FIREFLICKER;

	setmodel (missile, "models/faspell.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

// set missile speed	

	makevectors (self.angles);
	setorigin (missile, self.origin + v_factor(offset));

    vec = self.enemy.origin - missile.origin + self.enemy.proj_ofs;
	vec = normalize(vec);

	missile.velocity = (vec+aim_adjust(self.enemy))*set_speed;
	missile.angles = vectoangles(missile.velocity);
	missile.spell_angle = random(360);
	missile.count = 1;
	
	missile.touch = faSpellTouch;

	missile.think = faspell_frames;

	missile.lifetime=time+3;
	thinktime missile : HX_FRAME_TIME;
};

