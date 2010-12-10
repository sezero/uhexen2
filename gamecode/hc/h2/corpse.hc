/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/corpse.hc,v 1.2 2007-02-07 16:56:59 sezero Exp $
 */


void corpseblink (void)
{
	self.think = corpseblink;
	thinktime self : 0.1;
	self.scale -= 0.10;

	if (self.scale < 0.10)
		remove(self);
}

void init_corpseblink (void)
{
	CreateYRFlash(self.origin);

	self.drawflags (+) DRF_TRANSLUCENT | SCALE_TYPE_ZONLY | SCALE_ORIGIN_BOTTOM;

	corpseblink();
}

void() Spurt =
{
float bloodleak;

	makevectors(self.angles);
    bloodleak=rint(random(3,8));
    SpawnPuff (self.origin+v_forward*24+'0 0 -22', '0 0 -5'+ v_forward*random(20,40), bloodleak,self);
    sound (self, CHAN_AUTO, "misc/decomp.wav", 0.3, ATTN_NORM);
    if (self.lifetime < time||self.watertype==CONTENT_LAVA)
	    T_Damage(self,world,world,self.health);
	else
	{
	    self.think=Spurt;
		thinktime self : random(0.5,6.5);
	}
};

void () CorpseThink =
{
	self.think = CorpseThink;
	thinktime self : 3;

	if (self.watertype==CONTENT_LAVA)	// Corpse fell in lava
		T_Damage(self,self,self,self.health);
	else if (self.lifetime < time)			// Time is up, begone with you
		init_corpseblink();
};

/*
 * This uses entity.netname to hold the head file (for CorpseDie())
 * hack so that we don't have to set anything outside this function.
 */
void()MakeSolidCorpse =
{
vector newmaxs;
// Make a gibbable corpse, change the size so we can jump on it

//Won't be necc to pass headmdl once everything has it's .headmodel
//value set in spawn
    self.th_die = chunk_death;
	self.touch = obj_push;
    self.health = random(10,25);
	self.takedamage = DAMAGE_YES;
	self.solid = SOLID_PHASE;
	self.experience_value = 0;
	if(self.classname!="monster_hydra")
		self.movetype = MOVETYPE_STEP;//Don't get in the way	
	if(!self.mass)
		self.mass=1;

//To fix "player stuck" probem
	newmaxs=self.maxs;
	if(newmaxs_z>5)
		newmaxs_z=5;
	setsize (self, self.mins,newmaxs);

	if(self.flags&FL_ONGROUND)
		self.velocity='0 0 0';
    self.flags(-)FL_MONSTER;
    self.controller = self;
	self.onfire = FALSE;

	pitch_roll_for_slope('0 0 0');

    if ((self.decap)  && (self.classname == "player"))
    {	
		if (deathmatch||teamplay)
			self.lifetime = time + random(20,40); // decompose after 40 seconds
		else 
			self.lifetime = time + random(10,20); // decompose after 20 seconds

        self.owner=self;
        self.think=Spurt;
        thinktime self : random(1,4);
    }
    else 
	{
		self.lifetime = time + random(10,20); // disappear after 20 seconds
		self.think=CorpseThink;
		thinktime self : 0;
	}
};

