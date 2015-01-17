/*
 * $Header: /HexenWorld/HCode/window.hc 1     2/04/98 1:59p Rjohnson $
 */
/*
==============================================================================

WINDOWS

==============================================================================
*/

float () crandom;

//============================================================================


//============================================================================
vector() ChunkVelocity =
{
	local vector v;

	v_x = 300 * crandom();
	v_y = 300 * crandom();
	v_z = 300 + 100 * random();

	v = v * 0.7;

	return v;
};

void(vector space) CreateSpriteChunks =
{
	local entity sprite;

	sprite = spawn();

	space_x = space_x * random();
	space_y = space_y * random();
	space_z = space_z * random();

	setorigin (sprite, self.absmin + space);
   if (self.thingtype==THINGTYPE_GLASS)
	   setmodel (sprite, "gfx/glass.spr");
	else
	   setmodel (sprite, "gfx/stone.spr");

	setsize (sprite, '0 0 0', '0 0 0');
	sprite.movetype = MOVETYPE_BOUNCE;
	sprite.solid = SOLID_NOT;
	sprite.velocity = ChunkVelocity();
	sprite.think = SUB_Remove;
	sprite.ltime = time;
	sprite.nextthink = time + 1 + random()*1;

};

void(vector space) CreateModelChunks =
{
	local entity chunk;
	local float final;

	chunk = spawn();

	space_x = space_x * random();
	space_y = space_y * random();
	space_z = space_z * random();

	setorigin (chunk, self.absmin + space);

	if (self.thingtype==THINGTYPE_GLASS)
	{
		final = random();

		if (final<0.20)
			setmodel (chunk, "models/shard1.mdl");
		else if (final<0.40)
			setmodel (chunk, "models/shard2.mdl");
		else if (final<0.60)
			setmodel (chunk, "models/shard3.mdl");
		else if (final<0.80)
			setmodel (chunk, "models/shard4.mdl");
		else 
			setmodel (chunk, "models/shard5.mdl");
	}
	else if (self.thingtype==THINGTYPE_WOOD)
	{
		final = random();
		if (final < 0.25)
			setmodel (chunk, "models/splnter1.mdl");
		else if (final < 0.50)
			setmodel (chunk, "models/splnter2.mdl");
		else if (final < 0.75)
			setmodel (chunk, "models/splnter3.mdl");
		else 
			setmodel (chunk, "models/splnter4.mdl");
	}
	else
	   setmodel (chunk, "progs/gib1.mdl");

	setsize (chunk, '0 0 0', '0 0 0');
	chunk.movetype = MOVETYPE_BOUNCE;
	chunk.solid = SOLID_NOT;
	chunk.velocity = ChunkVelocity();
	chunk.think = SUB_Remove;
	chunk.avelocity_x = random()*1200;
	chunk.avelocity_y = random()*1200;
	chunk.avelocity_z = random()*1200;
	chunk.ltime = time;
	chunk.nextthink = time + 1 + random();

};

void () brush_die = 
{
   local vector space;
   local float holdcount,spritecount,chunkcount;

   space = self.absmax - self.absmin;

   holdcount = space_x + space_y + space_z;

	spritecount = holdcount/8;
	chunkcount = holdcount/16;

	if (self.thingtype==THINGTYPE_GLASS)
		sound (self, CHAN_VOICE, "raven/glassbrk.wav", 1, ATTN_NORM);
	else if (self.thingtype==THINGTYPE_WOOD)
		sound (self, CHAN_VOICE, "raven/woodbrk.wav", 1, ATTN_NORM);
	else
		sound (self, CHAN_VOICE, "raven/wallbrk.wav", 1, ATTN_NORM);

   while (spritecount>0)
	{
	//	CreateSpriteChunks(space);
		spritecount = spritecount - 1;
	}

   while (chunkcount>0)
	{
		CreateModelChunks(space);
		chunkcount = chunkcount - 1;
	}

	remove(self);
};

/*QUAKED breakable_brush (0 0 1) ?
Breakable window or wall
-------------------------FIELDS-------------------------
thingtype - type of chunks and sprites it will generate
    0 - glass
    1 - stone
    2 - wood    
health - amount of damage item can take.  Default is based on thingtype
   glass - 25
   stone - 75
   wood - 50

--------------------------------------------------------

*/
void() breakable_brush =
{
	self.max_health = self.health;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.th_die = brush_die;


	if (!self.health)
	{
		if (self.thingtype == THINGTYPE_GLASS)
			self.health = 25;
		else if (self.thingtype == THINGTYPE_STONE)
			self.health = 75;
		else if (self.thingtype == THINGTYPE_WOOD)
			self.health = 50;
		else
			self.health = 25;
	}

	self.use = brush_die;
	self.takedamage = DAMAGE_NO_GRENADE;

};

