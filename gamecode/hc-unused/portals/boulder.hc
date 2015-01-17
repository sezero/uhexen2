/*
 * $Header: /H2 Mission Pack/HCode/boulder.hc 2     2/10/98 3:53p Jmonroe $
 */

$cd \art\models\objects\boulder\final
$base base 128 128
$skin skin
$frame resting

void() boulder_find_path;

//============================================================================

void(entity targ) boulder_moveto_target =
{
	SUB_CalcMove(targ.origin, 50, boulder_find_path);
	if (!targ.target) remove(self);
	 else self.target = targ.target;
};

void() boulder_next =
{
	local entity targ;

	dprint("finding path\n");

	targ = find(world, targetname, self.target);

	if (!targ) remove(self);

	boulder_moveto_target(targ);
};

void() boulder_find_path =
{
	local entity targ;

	dprint("finding path\n");

	targ = find(world, targetname, self.target);

	if (!targ) remove(self);

	self.target = targ.target;
	
	self.nextthink = self.ltime + 0.1;
	self.think = boulder_next;
};

//============================================================================
vector() BoulderChunkVelocity =
{
	local vector v;

	v = randomv('-140 -140 70', '140 140 210');

	return v;
};
/*
void(vector space) BoulderCreateSpriteChunks =
{
	local entity sprite;

	sprite = spawn();

	space = randomv(space);

	setorigin (sprite, self.absmin + space);
	setmodel (sprite, "gfx/stone.spr");

	setsize (sprite, '0 0 0', '0 0 0');
	sprite.movetype = MOVETYPE_BOUNCE;
	sprite.solid = SOLID_NOT;
	sprite.velocity = BoulderChunkVelocity();
	sprite.think = SUB_Remove;
	sprite.ltime = time;
	sprite.nextthink = time + 1 + random()*1;
};
*/
void(vector space) BoulderCreateModelChunks =
{
	local entity chunk;

	chunk = spawn();

	space_x = space_x * random();
	space_y = space_y * random();
	space_z = space_z * random();

	setorigin (chunk, self.absmin + space);
	setmodel (chunk, "models/shard.mdl");
	chunk.skin=1;

	setsize (chunk, '0 0 0', '0 0 0');
	chunk.movetype = MOVETYPE_BOUNCE;
	chunk.solid = SOLID_NOT;
	chunk.velocity = BoulderChunkVelocity();
	chunk.think = SUB_Remove;
	chunk.avelocity_x = random()*1200;
	chunk.avelocity_y = random()*1200;
	chunk.avelocity_z = random()*1200;
	chunk.ltime = time;
	chunk.nextthink = time + 1 + random();
};

void() boulder_die =
{
   local vector space;
   local float holdcount,spritecount,chunkcount;

   space = self.absmax - self.absmin;

   holdcount = space_x + space_y + space_z;

	spritecount = holdcount/8;
	chunkcount = holdcount/16;

	sound (self, CHAN_VOICE, "raven/wallbrk.wav", 1, ATTN_NORM);
	
	while (spritecount>0)
	{
	//	CreateSpriteChunks(space);
		spritecount = spritecount - 1;
	}

	while (chunkcount>0)
	{
		BoulderCreateModelChunks(space);
		chunkcount = chunkcount - 1;
	}

	remove(self);
};

void() boulder_crush =
{
  dprint("Crusha!\n");
  T_Damage (other, self, self, 50);
};

void() boulder_use =
{
  self.velocity_x = 100 * random();
  self.velocity_y = 100 * random();
  self.velocity_z = 400;

  self.avelocity_x = random()*1200;
  self.avelocity_y = random()*1200;
  self.avelocity_z = random()*1200;

  self.movetype = MOVETYPE_BOUNCE;
};

/*QUAKED trap_boulder (0.3 0.1 0.6) (-13 -13 -14) (13 13 22)
A boulder
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void() trap_boulder =
{
	local entity boulder;

	boulder = spawn();
	
	precache_model2("models/boulder.mdl");
	setmodel(self, "models/boulder.mdl");

	self.health = 75;
	self.max_health = self.health;
	
	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_BOUNCE;

	self.th_die = boulder_die;
	//self.touch = boulder_use;
	self.blocked = boulder_crush;
	//self.use= boulder_use;

	self.takedamage = DAMAGE_YES;

	self.think = boulder_find_path;
	self.nextthink = self.ltime + 0.1;
};

