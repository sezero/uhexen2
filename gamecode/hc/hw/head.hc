void(string gibname, float dm) ThrowGib;
vector(float dm) VelocityForDamage;
void CorpseThink (void);


void () HeadThink =
{
	if ((self.lifetime<time&&(!deathmatch))||self.watertype == CONTENT_LAVA)
        self.th_die();
	else
	{
		self.think = HeadThink;
		thinktime self : 0.5;
	}
};


/*
 * This uses entity.netname to hold the head file (for CorpseDie())
 * hack so that we don't have to set anything outside this function.
 */
void ThrowSolidHead (float dm)
{
	setmodel (self, self.headmodel);
	if(self.headmodel==""||self.headmodel=="models/flesh1.mdl"
		||self.headmodel=="models/flesh2.mdl"
		||self.headmodel=="models/flesh3.mdl")
	{
		remove(self);
			return;
	}
	self.headmodel="";
    self.frame = 0;

	self.movetype = MOVETYPE_STEP;
	self.takedamage = DAMAGE_YES;
	self.solid = SOLID_PHASE;
	setsize (self, '-3 -3 -3', '3 3 3');//Allows step-over, but can't walk on or jump off them
	if(!self.mass)
		self.mass=1;
	self.hull=HULL_POINT;
	if (dm != -666)
	{
		if (dm < 40)  // Give it a little push
		   dm = 40;

		self.velocity = VelocityForDamage (dm);
	}

	setorigin(self,self.origin + '0 0 20');
	self.flags(-)FL_ONGROUND;
    self.health = 5;
    self.th_die = chunk_death;
    self.flags(-)FL_MONSTER;
	self.angles=RandomVector('300 300 300');
    self.classname = "head"; 
    self.controller = self;
    self.onfire = FALSE;
	self.lifetime = time + random(10,20); // decompose after 20 seconds

    if(!deathmatch && !coop && !teamplay)
	{
		self.think=CorpseThink;
		thinktime self : 0.5;
    }
    else
    {
		self.think = SUB_Null;
		self.nextthink = -1;
    }
}

