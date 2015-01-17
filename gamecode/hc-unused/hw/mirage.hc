/*
 * $Header: /HexenWorld/HCode/Mirage.hc 1     2/04/98 1:59p Rjohnson $
 */

/*
==============================================================================

MIRAGE

==============================================================================
*/
$cd id1/models/player_4
$origin 0 -6 24
$base base
$skin skin

$frame axrun1 axrun2 axrun3 axrun4 axrun5 axrun6






/*
==============================================================================
MIRAGE FRAMES
==============================================================================
*/

void ai_mirage(float dist);

void()  mirage_run1       =[      $axrun1,      mirage_run2       ]
{
	if(time > self.ltime)
		{
		self.owner.holo_engaged = 0;
		remove(self);
		return;
		}
	if(time + 3 > self.ltime)
		self.model = string_null;
	ai_mirage(11);
};
void()  mirage_run2       =[      $axrun2,      mirage_run3       ] {ai_mirage(8);};
void()  mirage_run3       =[      $axrun3,      mirage_run4       ]
{
	if(time + 3 > self.ltime)
		self.model = "models/player.mdl";
	ai_mirage(10);
};
void()  mirage_run4       =[      $axrun4,      mirage_run5       ] {ai_mirage(10);};
void()  mirage_run5       =[      $axrun5,      mirage_run6       ] {ai_mirage(8);};
void()  mirage_run6       =[      $axrun6,      mirage_run1       ] {ai_mirage(15);};





/*
 * ai_mirage() -- The so-called "intelligence" of the Mirage.
 */

void ai_mirage(float dist)
{
	local	vector	delta;
	local	float	axis;
	local	float	direct, ang_rint, ang_floor, ang_ceil;

	movedist = dist;

	if(self.enemy.health <= 0)
		{
		self.enemy = world;
		if(self.oldenemy.health > 0)
			{
			self.enemy = self.oldenemy;
			HuntTarget();
			}
		else {
			self.th_run();
			return;
			}
		}

	self.show_hostile = time + 1;		// wake up monsters

	enemy_vis = visible(self.enemy);
	if(enemy_vis)
		self.search_time = time + 5;

	if(coop && self.search_time < time)
		if(FindTarget())
			return;

	enemy_infront	= infront(self.enemy);
	enemy_range		= range(self.enemy);
	enemy_yaw		= vectoyaw(self.enemy.origin - self.origin);
 
//	if(CheckAnyAttack())
//		return;						// beginning a fake attack

	if(self.attack_state == AS_SLIDING)
		{
		ai_run_slide();
		return;
		}

	movetogoal(dist);
}




/*
 * remove_mirage() -- Removes the Mirage belonging to self from the world.
 */

void remove_mirage()
{
        local entity mirage;
		mirage = world;

		while(mirage.owner != self)
			{
			mirage = find(mirage, classname, "mirage");
			if(mirage == world)
				{
				bprint("Error: Mirage not found\n");
				return;
				}
			}
        remove(mirage);
        bprint("Mirage disengaged\n");
		if(time + 0.5 > mirage.ltime)
			self.holo_engaged = 0;
		else
			self.holo_engaged = mirage.ltime - time;
}




/*
 * init_mirage() -- Sets the Mirage entity fields after spawning.
 */

void init_mirage()
{
	self.solid		= SOLID_NOT;
	self.movetype	= MOVETYPE_STEP;

	setmodel(self, self.owner.model);
	setsize(self, '-16 -16 -24', '16 16 40');
	self.health		= 666;
	self.weapon		= IT_SHOTGUN;
	self.ltime		= time + self.owner.holo_engaged;

	self.th_stand	= mirage_run1;
	self.th_walk	= mirage_run1;
	self.th_run		= mirage_run1;
	self.classname	= "mirage";

	self.takedamage = DAMAGE_NO;
	self.angles     = self.owner.angles;
	self.yaw_speed  = 20;
	self.proj_ofs=self.view_ofs   = '0 0 25';

	setorigin(self, self.owner.origin);

	FindTarget();
	self.pathentity = self.goalentity;

	self.owner.holo_engaged += 100;
	bprint("Mirage engaged\n");
	mirage_run1();
}




/*
 * Mirage() -- Handles requests to use the Mirage.
 */

void Mirage()
{
	local entity mirage;

	if(self.classname != "player")
		return;

	if(self.holo_engaged == 0)				/* Mirage ran out */
		bprint("Mirage not available\n");
	else if(self.holo_engaged >= 100)		/* Mirage is engaged */
		remove_mirage();
	else {									/* Mirage isn't engaged */
		mirage			 = spawn();
		mirage.owner	 = self;
		mirage.nextthink = time + 0.05;
		mirage.think	 = init_mirage;
		}
 }




/*QUAK-ED item_mirage (0 0 0) (-8 -8 -8) (8 8 8) FLOATING
Gives a player ability to use a Mirage, similar to the "Holoduke" sprite.
Each item is worth 15 seconds.
-------------------------FIELDS-------------------------

--------------------------------------------------------
*/
/*
void item_mirage_touch()
{
	local entity mirage;

	if(other.classname != "player")
		return;
	if(other.health <= 0)
		return;

	remove(self);
	sound(other, CHAN_VOICE, "items/artpkup.wav", 1, ATTN_NORM);
	stuffcmd(other, "bf");
	bprint("Got Mirage");

	other.holo_engaged += 15;
	if(other.holo_engaged >= 115)
		{
		while(mirage.owner != other)
			{
			mirage = find(mirage, classname, "mirage");
			if(mirage == world)
				return;
			}
		mirage.model = "models/player.mdl";
		mirage.ltime += 15;
		}
	else if(other.holo_engaged > 100)
		other.holo_engaged = 100;
}


void item_mirage()
{
	precache_model("models/mirage.mdl");
	setmodel(self, "models/mirage.mdl");
	self.touch = item_mirage_touch;
	StartItem();
}

