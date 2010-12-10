/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/path.hc,v 1.2 2007-02-07 16:57:08 sezero Exp $
 */

/*
=============
pathcorner_touch

Something has bumped into a path_corner.  If it is a monster
change to the next target and continue.
==============
*/
void() pathcorner_touch =
{
local entity	temp;

	if (other.pathentity != self)	// This corner was not targeted by this monster
		return;
	
	if (other.enemy)
		return;		// fighting, not following a path

	if(self.wait==-2&&other.flags&FL_MONSTER)
		remove(other);

	temp = self;
	self = other;
	other = temp;

	self.goalentity = self.pathentity = find (world, targetname, other.target);
	self.ideal_yaw = vectoyaw(self.goalentity.origin - self.origin);
	if (!self.pathentity)
	{
		self.pausetime = time + 999999;
		self.th_stand ();
		return;
	}
};

/*
==============================================================================

TARGET CODE

The angle of the pathentity effects standing and bowing direction, but has no effect on movement, which always heads to the next target.

targetname
must be present.  The name of this pathentity.

target
the next spot to move to.  If not present, stop here for good.

pausetime
The number of seconds to spend standing or bowing for path_stand or path_bow

==============================================================================
*/
float SYNCH = 1;
/*QUAKED path_corner (0.5 0.3 0) (-8 -8 -8) (8 8 8) SYNCH
Monsters will continue walking towards the next target corner.

FOR TRAINS:
-------------------------FIELDS-------------------------
SYNCH - Will make the train automatically calculate a new anglespeed based on the distance it's going and will finish the turn at the same time the move is done.
"speed" - how fast the train should move to this path corner.  This will change the train's default speed to whatever you specify.  Nice for trains that should speed up and slow down.  No speed will let the train move at whatever speed it was last set to.
"angles" - how much to modify the train's angles by.  So if you set it at '0 90 0', and the train was at an angle of '30 60 90', the train would rotate unitl it's angles equalled '30 150 90'
"anglespeed" - how fast the train should rotate to the new angle.  Again, this will change the train's default anglespeed.
The defaults of all of these are 0.
--------------------------------------------------------

As usual, any rotating brush needs an origin brush.
*/
void path_corner ()
{
	if (!self.targetname)
		objerror ("path_corner has no targetname");
		
	if(!self.mangle)
		self.mangle=self.angles;
	self.solid = SOLID_TRIGGER;
	self.touch = pathcorner_touch;
	setsize (self, '-8 -8 -8', '8 8 8');
}

