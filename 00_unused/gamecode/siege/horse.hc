/*
 * $Header: /home/ozzie/Download/0000/uhexen2/00_unused/gamecode/siege/horse.hc,v 1.1.1.1 2005-07-29 20:43:10 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\RdrWar\tsthorse\horse.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RdrWar\tsthorse
$origin 0 0 0
$base base skin
$skin skin
$flags 0

//
$frame horse        




// Frame Code
void() frame_horse      = [ $horse     , frame_horse      ] {  };




void() rick_test =
{
   entity rider;

   if (deathmatch)
   {
      remove(self);
      return;
   }

   rider = spawn_rider();

   precache_model2 ("models/horse.mdl");
   self.solid = SOLID_SLIDEBOX;
   self.movetype = MOVETYPE_FLY;

   setmodel (self, "models/horse.mdl");
   self.skin = 0;

   setsize (self, '0 0 0', '50 50 50');
   self.health = 100;

   self.movechain = rider;
   rider.origin = self.origin;
   rider.flags = rider.flags | FL_MOVECHAIN_ANGLE;

   self.avelocity = '100 100 100';
};


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/01/26 17:26:11  sezero
 * Raven's original Siege hcode.
 *
 * 
 * 3     5/25/98 1:38p Mgummelt
 * 
 * 1     2/04/98 1:59p Rjohnson
 */
