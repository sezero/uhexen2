/*
==============================================================================

HURTER (jweier)

==============================================================================
*/

void () hurter_touch =
{
	local float damage;

	damage = HRT_BASE_DAMAGE * random();

	if (other.classname == "player") 
	{
		T_Damage(other, self, self, damage);
	}
};

void () hurter = 
{
	local entity hurt;

	hurt = spawn ();
	hurt.owner = self;
    hurt.touch = hurter_touch;
	hurt.movetype = MOVETYPE_NONE;
	hurt.solid = SOLID_NOT;

    //TODO: Make proper size (currently size of Fish)
	setsize (self, '-16 -16 -24', '16 16 24');

	//TODO: Add sound effect if any
	//self.noise = "raven/in_hurt.wav";

	hurt.classname = "hurter";

	setorigin (hurt, hurt.origin);
};
