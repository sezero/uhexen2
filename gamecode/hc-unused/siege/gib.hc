/*
 * $Header: /HexenWorld/Siege/gib.hc 5     5/25/98 1:38p Mgummelt $
 */

vector() ChunkVelocity =
{
	local vector v;

	v = randomv('-210 -210 -210', '210 210 280');

	return v;
};

void(vector space) CreateSpriteChunks =
{
	local entity sprite;

	sprite = spawn();

	space = randomv(space);

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
	else if (self.thingtype==THINGTYPE_METAL)
	{
		final = random();
		if (final < 0.25)
			setmodel (chunk, "models/metlchk1.mdl");
		else if (final < 0.50)
			setmodel (chunk, "models/metlchk2.mdl");
		else if (final < 0.75)
			setmodel (chunk, "models/metlchk3.mdl");
		else 
			setmodel (chunk, "models/metlchk4.mdl");
	}
	else if (self.thingtype==THINGTYPE_FLESH)
	{
		final = random();
		if (final < 0.33)
			setmodel (chunk, "models/flesh1.mdl");
		else if (final < 0.66)
			setmodel (chunk, "models/flesh2.mdl");
		else
			setmodel (chunk, "models/flesh3.mdl");
	}
	else setmodel (chunk, "models/flesh1.mdl");

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

void () chunk_death = 
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
	else if ((self.thingtype==THINGTYPE_GREYSTONE) || (self.thingtype==THINGTYPE_BROWNSTONE)||self.thingtype==THINGTYPE_DIRT)
		sound (self, CHAN_VOICE, "raven/wallbrk.wav", 1, ATTN_NORM);
	else if (self.thingtype==THINGTYPE_METAL)
		sound (self, CHAN_VOICE, "raven/metalbrk.wav", 1, ATTN_NORM);
	else if (self.thingtype==THINGTYPE_FLESH)
		sound (self, CHAN_VOICE, "raven/fleshbrk.wav", 1, ATTN_NORM);
	else if (self.thingtype==THINGTYPE_CLOTH)
		sound (self, CHAN_VOICE, "raven/clothbrk.wav", 1, ATTN_NORM);
	else
		sound (self, CHAN_VOICE, "raven/wallbrk.wav", 1, ATTN_NORM);

	// FIXME: are we adding sprites to the deaths also???
//	while (spritecount>0)
//	{
//		CreateSpriteChunks(space);
//		spritecount = spritecount - 1;
//	}

	while (chunkcount>0)
	{
		CreateModelChunks(space);
		chunkcount = chunkcount - 1;
	}

	SUB_UseTargets();

	remove(self);
};

