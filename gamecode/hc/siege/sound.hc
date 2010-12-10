/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/sound.hc,v 1.2 2007-02-07 17:01:24 sezero Exp $
 */

void sound_maker_run(void)
{
	sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
}

void sound_maker_wait(void)
{
	self.think = sound_maker_run;
	thinktime self : self.delay;
}

/*QUAKED sound_maker (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
A sound that can be triggered.
-------------------------FIELDS-------------------------
 "soundtype" values :

  1 - bell ringing
  2 - Organ Music (not the organ you're thinking about)
  3 - Tomb sound (hey, it's too late in the project to waste time typing comments!
--------------------------------------------------------
*/
void sound_maker (void)
{
	if (self.soundtype==1)
	{
		precache_sound ("misc/bellring.wav");
		self.noise1 = ("misc/bellring.wav");
	}
	else if (self.soundtype==2)
	{
		precache_sound2 ("misc/organ.wav");
		self.noise1 = ("misc/organ.wav");
	}
	else if (self.soundtype==3)
	{
		precache_sound ("misc/tomb.wav");
		self.noise1 = ("misc/tomb.wav");
	}
	
	if (self.delay) 
		self.use = sound_maker_wait;
	else 
		self.use = sound_maker_run;
}

void sound_again(void)
{
	float chance;

	if (self.soundtype == 13)
	{
		chance = random();
		if (chance < .33)
			sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
		else if (chance < .66)
			sound (self, CHAN_VOICE, self.noise2, 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, self.noise3, 1, ATTN_NORM);
	}
	else if ((self.soundtype == 11) || (self.soundtype == 12) || (self.soundtype == 14))
	{
		if (random() < .5)
			sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, self.noise2, 1, ATTN_NORM);
	}
	else
		sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);

	self.think = sound_again;
	self.nextthink = time + random(5,30);
}


/*QUAKED sound_ambient (0.3 0.1 0.6) (-10 -10 -8) (10 10 8)
Creates an ambient sound in the world.
-------------------------FIELDS-------------------------
 "soundtype" values :

   1 - windmill 
   2 - dripping, echoing sewer water sound
   3 - dripping water with no echo 
   4 - subtle sky/wind
   5 - crickets / night sounds
   6 - birds
   7 - raven caw
   8 - rocks falling
   9 - lava bubble
  10 - water gurgle
  11 - metal
  12 - pounding
  13 - random moans and screams
  14 - creaking
  15 - chain rattling
--------------------------------------------------------
*/
void sound_ambient (void)
{

	if (self.soundtype == 1)
	{
		precache_sound ("ambience/windmill.wav");
		self.noise1 = ("ambience/windmill.wav");
	}
	else if (self.soundtype == 2)
	{
		precache_sound ("ambience/drip1.wav");
		self.noise1 = ("ambience/drip1.wav");
		self.think = sound_again;
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 3)
	{
		precache_sound ("ambience/drip2.wav");
		self.noise1 = ("ambience/drip2.wav");
		self.think = sound_again;
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 4)
	{
		precache_sound ("ambience/wind.wav");
		self.noise1 = ("ambience/wind.wav");
	}
	else if (self.soundtype == 5)
	{
		precache_sound ("ambience/night.wav");
		self.noise1 = ("ambience/night.wav");
		self.think = sound_again;
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 6)
	{
		precache_sound ("ambience/birds.wav");
		self.noise1 = ("ambience/birds.wav");
		self.think = sound_again;
		thinktime self : random(15,60);
	}
	else if (self.soundtype == 7)
	{
		precache_sound ("ambience/raven.wav");
		self.noise1 = ("ambience/raven.wav");
		self.think = sound_again;
		thinktime self : random(15,60);
	}
	else if (self.soundtype == 8)
	{
		precache_sound ("ambience/rockfall.wav");
		self.noise1 = ("ambience/rockfall.wav");
		self.think = sound_again;
		thinktime self : random(15,60);
	}
	else if (self.soundtype == 9)
	{
		precache_sound ("ambience/lava.wav");
		self.noise1 = ("ambience/lava.wav");
	}
	else if (self.soundtype == 10)
	{
		precache_sound ("ambience/water.wav");
		self.noise1 = ("ambience/water.wav");
	}
	else if (self.soundtype == 11)
	{
		precache_sound ("ambience/metal.wav");
		self.noise1 = ("ambience/metal.wav");
		precache_sound ("ambience/metal2.wav");
		self.noise2 = ("ambience/metal2.wav");
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 12)
	{
		precache_sound ("ambience/pounding.wav");
		self.noise1 = ("ambience/pounding.wav");
		precache_sound ("ambience/poundin2.wav");
		self.noise2 = ("ambience/poundin2.wav");
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 13)
	{
		precache_sound ("ambience/moan1.wav");
		self.noise1 = ("ambience/moan1.wav");
		precache_sound ("ambience/moan2.wav");
		self.noise2 = ("ambience/moan2.wav");
		precache_sound ("ambience/moan3.wav");
		self.noise3 = ("ambience/moan3.wav");
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 14)
	{
		precache_sound ("ambience/creak.wav");
		self.noise1 = ("ambience/creak.wav");
		precache_sound ("ambience/creak2.wav");
		self.noise2 = ("ambience/creak2.wav");
		thinktime self : random(5,30);
	}
	else if (self.soundtype == 15)
	{
		precache_sound ("ambience/rattle.wav");
		self.noise1 = ("ambience/rattle.wav");
		thinktime self : random(5,30);
	}

	if (!self.think)
		ambientsound (self.origin, self.noise1, 1, ATTN_STATIC);
	else
		sound (self, CHAN_VOICE, self.noise1, 1, ATTN_NORM);

}

