/*
 * $Header: /HexenWorld/HCode/horse.hc 1     2/04/98 1:59p Rjohnson $
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

