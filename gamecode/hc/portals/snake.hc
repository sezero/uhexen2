/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/snake.hc,v 1.2 2007-02-07 16:59:36 sezero Exp $
 */
/*
==============================================================================

SNAKE

==============================================================================
*/

// FRAME  0
$frame rootpose     
//
// FRAME  1 - 13
$frame 1cycle1      1cycle2      1cycle3      1cycle4      1cycle5      
$frame 1cycle6      1cycle7      1cycle8      1cycle9      1cycle10     
$frame 1cycle11     1cycle12     1cycle13     

//
// FRAME  14 - 26
$frame 2cycle1      2cycle2      2cycle3      2cycle4      2cycle5      
$frame 2cycle6      2cycle7      2cycle8      2cycle9      2cycle10     
$frame 2cycle11     2cycle12     2cycle13     

//
// FRAME  27 - 36
$frame hiss1        hiss2        hiss3        hiss4        hiss5        
$frame hiss6        hiss7        hiss8        hiss9        hiss10       

//
// FRAME  37 - 44
$frame settle1      settle2      settle3      settle4      settle5      
$frame settle6      settle7      settle8      

//
// FRAME  45 - 57
$frame spitLF1      spitLF2      spitLF3      spitLF4      spitLF5      
$frame spitLF6      spitLF7      spitLF8      spitLF9      spitLF10     
$frame spitLF11     spitLF12     spitLF13     

//
// FRAME  58 - 70
$frame spitRT1      spitRT2      spitRT3      spitRT4      spitRT5      
$frame spitRT6      spitRT7      spitRT8      spitRT9      spitRT10     
$frame spitRT11     spitRT12     spitRT13     

//
// FRAME  71 - 81
$frame sway1        sway2        sway3        sway4        sway5        
$frame sway6        sway7        sway8        sway9        sway10       
$frame sway11       

//
// FRAME  72 - 94
$frame UNsway1      UNsway2      UNsway3      UNsway4      UNsway5      
$frame UNsway6      UNsway7      UNsway8      UNsway9      UNsway10     
$frame UNsway11     UNsway12     UNsway13     

//
// FRAME  95 - 113
$frame wake1        wake2        wake3        wake4        wake5        
$frame wake6        wake7        wake8        wake9        wake10       
$frame wake11       wake12       wake13       wake14       wake15       
$frame wake16       wake17       wake18       wake19       

void wake_effect (void);
void snake_think(void);
float snake_look(void);

/*
void snake_deflect (void)
{
	local entity item;
	local vector vec, realVec;
	local float diff;
	float DidDeflect;
	
	DidDeflect = 0;
	item = findradius(self.origin, 300);
	while (item)
	{

		if ((item.owner.classname == "player") && ((item.movetype == MOVETYPE_FLYMISSILE) ||
			(item.movetype == MOVETYPE_BOUNCEMISSILE)))
		{
			vec = self.origin - item.origin + self.view_ofs;
			vec = normalize(vec);
			realVec = normalize(item.velocity);
			diff = fabs(vec_x - realVec_x) + fabs(vec_y - realVec_y) + fabs(vec_z - realVec_z);
			if (diff <= 0.4) 
			{
				diff = vlen(item.velocity);
				diff = diff / random(1.5, 2.5);

				vec = '0 0 0' - item.velocity;
				vec = normalize(vec);

				vec += randomv('-0.5 -0.5 -0.5', '0.5 0.5 0.5');
				vec = normalize(vec);

				vec = vec * diff;

				item.velocity = vec;

				item.angles = vectoangles(item.velocity);
				item.owner = self;
				DidDeflect = 1;
			}
		}
		item = item.chain;
	}

	if (DidDeflect)
		sound (self, CHAN_WEAPON, "fangel/deflect.wav", 1, ATTN_NORM);
}

void snake_checkdeflect(void)
{
//	if (self.enemy.last_attack + 0.5 < time)
//		return;

	snake_deflect();
}
*/


void snake_sleep (void)
{
	self.nextthink = time - 1;
	self.think = SUB_Null;
}

void snake_settle (void) [++ $settle1 .. $settle8]
{
	if (self.frame == 38)
		sound (self, CHAN_BODY, "snake/hiss.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		wake_effect();
}

void snake_unsway (void) [++ $UNsway1 .. $UNsway13]
{
	if (self.frame == 86)
		sound (self, CHAN_BODY, "snake/hiss.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		snake_settle();
}


void spit_touch(void)
{
	float damg;
	vector delta;

	damg = random(10,14);

	T_Damage (other, self, self.owner, damg );

	self.origin = self.origin - 8 * normalize(self.velocity) - '0 0 40';
	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	if (other)
	{
		delta = other.origin - self.origin;
		other.velocity = delta * 40;
		other.flags (-) FL_ONGROUND;
		other.velocity_z = 275;
	}

	CreateRedSpark (self.origin); 

	remove(self);
}

void snake_missile_think (void)
{
	if (self.lifetime < time)
	{
		spit_touch();
	}
	else
	{
		HomeThink();
		self.angles = vectoangles(self.velocity);
		self.think=snake_missile_think;
		thinktime self : HX_FRAME_TIME;
	}
}

void snake_missile_shoot(vector spot1)
{
	entity newmis;
	vector diff;

	newmis = spawn ();
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;

	setmodel (newmis, "models/goop.mdl");
	setsize (newmis, '0 0 0', '0 0 0');
	setorigin (newmis, spot1);

	newmis.angles = self.angles;

	diff = (self.enemy.origin + self.enemy.view_ofs) - spot1;
	newmis.velocity = normalize(diff);
	newmis.angles = vectoangles(newmis.velocity);

	makevectors (newmis.angles);
	newmis.velocity = newmis.velocity * 1000;
	newmis.speed=1000;	//Speed
	newmis.classname = "faminemissile";
	newmis.angles = vectoangles(newmis.velocity);

	newmis.veer=TRUE;		//No random wandering
	newmis.turn_time=3;		//Lower the number, tighter the turn
	newmis.ideal_yaw=TRUE;//Only track things in front
	newmis.lockentity = self.enemy;
	newmis.lifetime = time + 1.5;
	newmis.think=snake_missile_think;
	thinktime newmis : HX_FRAME_TIME;

	newmis.touch = spit_touch;
}

void snake_missile (void)
{
	makevectors(self.angles);
	snake_missile_shoot(self.origin + v_forward*4 + v_right * 38 + v_up * 195);

	makevectors(self.angles);
	snake_missile_shoot(self.origin + v_forward*4 + v_right * 57 + v_up * 195);
}

void snake_attackleft (void) [++ $spitLF1 .. $spitLF13]
{

	if (self.frame == $spitLF5)
		snake_missile();

	if (self.frame == 45)
		sound (self, CHAN_WEAPON, "snake/attack.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		snake_think();
}

void snake_attackright (void) [++ $spitRT1 .. $spitRT13]
{

	if (self.frame == $spitRT5)
		snake_missile();

	if (self.frame == 61)
		sound (self, CHAN_WEAPON, "snake/attack.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		snake_think();
}

void snake_attackfront (void) [++ $wake5 .. $wake19]
{

	if (self.frame == $wake10)
		snake_missile();

	if (self.frame == 100)
		sound (self, CHAN_WEAPON, "snake/attack.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		snake_think();
}

void snake_attack (void)
{
	self.attack_cnt += 1;

	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin) - self.angles_y;

	if (((self.ideal_yaw >= 0) && (self.ideal_yaw < 25)) ||
	    ((self.ideal_yaw >= -25) && (self.ideal_yaw < -1)))
		snake_attackfront();
	else if ((self.ideal_yaw >= 25) && (self.ideal_yaw < 180))
		snake_attackleft();
	else
		snake_attackright();
}

void snake_dancetongue(void) [++ $2cycle1 .. $2cycle13]
{
//hinktime self : HX_FRAME_TIME * 2;
	thinktime self : HX_FRAME_TIME *1.5;

	if (self.frame == 16)
		sound (self, CHAN_BODY, "snake/hiss.wav", 1, ATTN_NORM);

	if (cycle_wrapped)
		snake_think();
}

void snake_dancetail(void) [++ $1cycle1 .. $1cycle13]
{
//thinktime self : HX_FRAME_TIME * 2;
	thinktime self : HX_FRAME_TIME *1.5;

	if (cycle_wrapped)
		snake_think();
}

void snake_dance (void)
{
	self.attack_cnt = 0;

	if (random() < .5)
		snake_dancetail();
	else
		snake_dancetongue();
}

void snake_startsway(void) [++ $sway1 .. $sway11]
{
//hinktime self : HX_FRAME_TIME * 2;
	thinktime self : HX_FRAME_TIME;

	if (cycle_wrapped)
		snake_dance();
}

void snake_wake(void) [++ $wake1 .. $wake19]
{
//thinktime self : HX_FRAME_TIME * 2;
	thinktime self : HX_FRAME_TIME;

	if (self.frame < $wake15)
	{
		self.abslight = .25 + (.05 * (self.frame - $wake1));
	}
	else
		self.abslight -= .05;
		
	if (self.frame == $wake19)
		self.drawflags (-) MLS_ABSLIGHT;

	if (self.frame == 100)
		sound (self, CHAN_ITEM, "snake/wake.wav", 1, ATTN_NORM);

	self.skin = 0;
	if (cycle_wrapped)
		snake_startsway();
}


float snake_look(void)
{
	entity client;

	client = checkclient ();
	if (!client)
	{
		return FALSE;	// current check entity isn't in PVS
	}

	if ((client.items & IT_INVISIBILITY) || (client.flags & FL_NOTARGET))
	{
		return FALSE;
	}

	if (!visible (client))
		return FALSE;

	if (client.classname == "player")
	{
		self.enemy = client;
		return TRUE;
	}
	return FALSE;
}

void snake_think (void)
{
	float chance;

	if (snake_look())
	{
		enemy_range = vlen (self.origin - self.enemy.origin);
		enemy_range -= 80;

		if (enemy_range < 200)
			chance = 0.90;
		else if (enemy_range < 400)
			chance = 0.80;
		else if (enemy_range < 800)
			chance = 0.70;
		else if (enemy_range < 1200)
			chance = 0.60;
		else if (enemy_range < 2000)
			chance = 0.50;
		else  // Too far away
		{
			self.enemy = world;
			chance = 0;
		}

		chance -= ((self.attack_cnt/2) * .1);

		if (random() < chance)
		{
			self.last_attack = time;
			snake_attack();
		}
		else
			snake_dance();
	}
	else
	{
		self.enemy = world;
		snake_dance();
	}

	if (self.last_attack < time - 30)
		snake_unsway();
}

void wake_effect (void)
{
	entity newent;

	newent = spawn();

	setorigin(newent,self.origin);
	sound (self, CHAN_BODY, "snake/life.wav", 1, ATTN_NORM);

	if (self.colormap != 0)
	{
		CreateGreySmoke(self.origin + '0 0 60','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '16 16 55','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '32 32 80','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '64 64 50','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '-16 -16 65','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '-32 -32 45','0 0 12',HX_FRAME_TIME * 20);
		CreateGreySmoke(self.origin + '-64 -64 70','0 0 12',HX_FRAME_TIME * 20);

		self.use = SUB_Null;
		self.colormap = 0;
		self.takedamage = DAMAGE_YES;
		self.drawflags (+) MLS_ABSLIGHT;
		self.flags2 (+) FL_ALIVE;
		snake_wake();
	}
	else
	{
		self.colormap = 3;
		self.use = wake_effect;	
		self.takedamage = DAMAGE_NO;
		snake_sleep();
	}
}

/*QUAKED monster_snake (1 0.3 0) (-80 -80 0) (80 80 200) AMBUSH 
Monster snake that comes to life and attacks
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
void() monster_snake =
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NONE;
	self.colormap = 3;

	precache_model2 ("models/goop.mdl");
	precache_model2 ("models/snake.mdl");
	setmodel (self, "models/snake.mdl");

	precache_sound2 ("snake/attack.wav");
	precache_sound2 ("snake/hiss.wav");
	precache_sound2 ("snake/wake.wav");
	precache_sound2 ("snake/life.wav");
	precache_sound2 ("fangel/deflect.wav");

	setsize (self, '-80 -80 0', '80 80 200' );
	self.health = 1200;

	total_monsters += 1;
	self.view_ofs = '0 0 40';
	self.flags (+) FL_MONSTER;
	self.use = wake_effect;	
	self.th_die = chunk_death;
	self.thingtype = THINGTYPE_BROWNSTONE;
	self.monsterclass = CLASS_BOSS;

	self.counter = 0;
};

