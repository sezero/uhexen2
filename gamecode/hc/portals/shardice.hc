/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/shardice.hc,v 1.2 2007-02-07 16:59:36 sezero Exp $
 */
/*
==============================================================================

ice shard (IMP)

==============================================================================
*/

// For building the model
$cd q:/art/models/monsters/imp/final
$base shrdbase 64 128 
$skin shrdskin

$frame shard


void() shardTouch =
{
float	damg;

	if (other == self.owner)
		return;		// don't explode on owner

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if(self.owner.classname=="monster_imp_lord")
		damg = random(33,77);
	else
		damg = random(5,10);
	
	if(other.health<=damg&&other.thingtype==THINGTYPE_FLESH&&random()<0.1)
		SnowJob(other,self);
	else if (other.health)
	{
		T_Damage (other, self, self.owner, damg );
	    sound (self, CHAN_BODY, "crusader/icehit.wav", 1, ATTN_NORM);
		if(other.classname=="player")
		{
			other.artifact_active(+)ARTFLAG_FROZEN;
			newmis=spawn();
			newmis.enemy=other;
			newmis.artifact_active=ARTFLAG_FROZEN;
			newmis.think=remove_artflag;
			thinktime newmis : 0.1;
		}
	}
	remove(self);
};





//============================================================================


void()  shard_1  =[ $shard ,  shard_1   ] { };


//============================================================================


void(vector offset, float set_speed, vector dest_offset) do_shard =
{
entity missile;
vector vec;

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;
	missile.health = 10;

	if(self.classname=="monster_imp_lord")
	{
		set_speed*=2;
		missile.scale=2;
	}

	setmodel (missile, "models/shardice.mdl");
	setsize (missile, '0 0 0', '0 0 0');		

// set missile speed	

	makevectors (self.angles);
	vec = self.origin + self.view_ofs + v_factor(offset);
	setorigin (missile, vec);

	vec = self.enemy.origin - missile.origin + self.enemy.proj_ofs + dest_offset;
	vec = normalize(vec);

	missile.velocity = (vec+aim_adjust(self.enemy))*set_speed;
	missile.angles = vectoangles(missile.velocity);
	
	missile.touch = shardTouch;

	missile.think = shard_1;
	missile.nextthink = time + HX_FRAME_TIME;
};

