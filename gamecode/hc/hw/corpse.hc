/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/corpse.hc,v 1.1.1.1 2004-11-29 11:23:36 sezero Exp $
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:01  theoddone33
 * Inital import
 *
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 35    9/11/97 6:56p Mgummelt
 * 
 * 34    9/11/97 3:41p Mgummelt
 * 
 * 33    9/11/97 12:02p Mgummelt
 * 
 * 32    8/29/97 11:14p Mgummelt
 * 
 * 31    8/26/97 2:40p Mgummelt
 * 
 * 30    8/14/97 10:27p Bgokey
 * 
 * 29    8/14/97 1:27p Mgummelt
 * 
 * 28    8/14/97 1:17p Mgummelt
 * 
 * 27    8/13/97 5:34p Mgummelt
 * 
 * 26    8/13/97 1:28a Mgummelt
 * 
 * 25    8/12/97 6:10p Mgummelt
 * 
 * 24    8/06/97 10:18p Mgummelt
 * 
 * 23    8/06/97 11:10a Rlove
 * 
 * 22    7/22/97 4:13p Rlove
 * Putting final touches on skull wizard.
 * 
 * 21    7/01/97 7:09a Rlove
 * Corpses give no experience points.
 * 
 * 20    6/30/97 5:38p Mgummelt
 * 
 * 19    6/25/97 9:23p Mgummelt
 * 
 * 18    6/19/97 5:15p Mgummelt
 * 
 * 17    6/18/97 6:09p Mgummelt
 * 
 * 16    6/18/97 4:00p Mgummelt
 * 
 * 15    5/30/97 10:04p Mgummelt
 * 
 * 14    5/28/97 10:45a Rlove
 * Moved sprite effects to client side - smoke, explosions, and flashes.
 * 
 * 13    5/27/97 10:57a Rlove
 * Took out old Id sound files
 * 
 * 12    5/15/97 6:34p Rjohnson
 * Code cleanup
 * 
 * 11    5/13/97 2:26p Rlove
 * 
 */
