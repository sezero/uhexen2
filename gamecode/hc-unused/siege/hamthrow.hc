/*
 * $Header: /HexenWorld/Siege/hamthrow.hc 3     5/25/98 1:38p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\weapons\warhammer\prjctile\hamthrow.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\warhammer\prjctile
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame HAMMER1      HAMMER2      HAMMER3      HAMMER4      HAMMER5      
$frame HAMMER6      




// Frame Code
void() frame_HAMMER1    = [ $HAMMER1   , frame_HAMMER2    ] {  };
void() frame_HAMMER2    = [ $HAMMER2   , frame_HAMMER3    ] {  };
void() frame_HAMMER3    = [ $HAMMER3   , frame_HAMMER4    ] {  };
void() frame_HAMMER4    = [ $HAMMER4   , frame_HAMMER5    ] {  };
void() frame_HAMMER5    = [ $HAMMER5   , frame_HAMMER6    ] {  };
void() frame_HAMMER6    = [ $HAMMER6   , frame_HAMMER1    ] {  };




/*QUAKED hamthrow (1 0 0) (0 0 0) (50 50 50)
New item for QuakeEd

-------------------------FIELDS-------------------------
--------------------------------------------------------

*/
/*
void() hamthrow =
{
   if (deathmatch)
   {
      remove(self);
      return;
   }

   precache_model2 ("models/hamthrow.mdl");
   self.solid = SOLID_SLIDEBOX;
   self.movetype = MOVETYPE_NONE;

   setmodel (self, "models/hamthrow.mdl");
   self.skin = 0;

   setsize (self, '0 0 0', '50 50 50');
   self.health = 100;
};

*/
// Throwing Hammer
void() ThrowHammerThink =
{

if (self.aflag == 1)
{
        if(visible(self.controller))
        {
        if (self.effects == 0)
                self.effects = 8;
        else if (self.effects == 8)
                {
                self.effects = 4;
                sound(self, CHAN_WEAPON, "items/protect3.wav", 0.3, ATTN_NORM);
                sprint(self.controller,"Mjolnir Beckons....\n");
                self.wait = self.wait + 1;
                }
        else if (self.effects == 4)
                self.effects = 8;
        if (self.wait > 10)
                {
                        sound(self.controller, CHAN_ITEM, "idweapons/pkup.wav",1, ATTN_NORM);
                        self.controller.amthammer = self.controller.amthammer + 1;
                        sprint(self.controller,"Mjolnir has magically returned\n");
//                        MagicEffect(self.controller);
                        remove(self);
                }
        }
        else self.effects = 0;
        self.nextthink = time + 0.5;
}
else
{
        if (self.aflag == -1)
        {
        local vector vtemp,dir;
          vtemp = self.controller.origin + '0 0 10';
          dir = normalize(vtemp - self.origin);
        if (self.watertype < -2)
             self.velocity = dir * 150;
        else self.velocity = dir * 375;
          self.angles = vectoangles(self.velocity);
        if (self.flags & FL_ONGROUND)// && self.follow == 0)
                {
                   self.avelocity = '500 0 0';
                   self.flags = self.flags - FL_ONGROUND;
//                   self.follow = 1;
                }
        }
        sound(self, CHAN_WEAPON, "weapons/whoosh.wav", 0.3, ATTN_NORM);
//        SpawnFlame(self.origin);
        self.nextthink = time + 0.2;
}
        if(self.controller.health<=0)
                {
                GrenadeExplode2();
                remove(self);
                }
self.think = ThrowHammerThink;
};

void() HammerTouch =
{
local float inertia;
  if (other == self.controller)
        {
                if (self.aflag!=0||self.bloodloss<time)
                {
                        sound(other, CHAN_ITEM, "idweapons/pkup.wav",1, ATTN_NORM);
                        other.amthammer = other.amthammer + 1;
                        remove(self);
                        return;
                }
                else return;
        }

  else if (other.takedamage)
  {
      if(self.velocity != VEC_ORIGIN && other != self.controller)
        if (self.aflag < 1)  
        {
//              spawn_touchblood(40);
//              SpawnChunk(self.origin, self.velocity);
              other.punchangle_x = -20;
              self.enemy = other;  
              if(other.health) 
              {
			  if (other.mass<=0)
				inertia=1;
			  else 
				inertia = other.mass;
			  CastLightning();
              other.velocity_x = other.velocity_x + self.velocity_x / inertia;
              other.velocity_y = other.velocity_y + self.velocity_y / inertia;
              other.velocity_z = other.velocity_z + 100;
			  if(other.flags&FL_ONGROUND)
	              other.flags = other.flags - FL_ONGROUND;
			  CastLightning();
			  CastLightning();
              }
              T_Damage(other, self, self.controller, 10);
        }
/*
        else if (other.classname == "player"&&other.amthammer==0)
        {
                sprint(self.controller,"Your Hammer was stolen by ");
                sprint(self.controller,other.netname);
                sprint(self.controller,"!\n");
                sprint(other,"You got ");
                sprint(other,self.controller.netname);
                sprint(other,"'s Hammer!\n");
                sound(other, CHAN_ITEM, "idweapons/pkup.wav",other.volume + 1, ATTN_NORM);
                other.amthammer = other.amthammer + 1;
                other.follow = 1;
                other.impulse = 1;
                remove(self);
        }  
*/
  }
  else
  {
    if (random()<0.5) 
      sound(self, CHAN_WEAPON, "player/axhit2.wav", 1, ATTN_NORM);
    else
      sound(self, CHAN_WEAPON, "idweapons/tink1.wav", 1, ATTN_NORM);
  }

  if (self.aflag < 1)
       if (visible(self.controller))
               self.aflag = -1;
       else
       {
       if (self.aflag != 1)
        sprint(self.controller,"You lost your Hammer!\n");
       self.aflag = 1;
       self.effects = 0;
       self.avelocity = '300 300 300';
       setsize(self, '-3 -3 -5', '3 3 3');
       self.movetype = MOVETYPE_BOUNCE;
       }
};


void() ThrowHammer =
{
local entity missile;
  sound(self, CHAN_WEAPON, "weapons/whoosh.wav", 1, ATTN_NORM);
  missile = spawn();
  missile.owner = missile;
  missile.controller = self;
  missile.classname = "mjolnir";
  missile.movetype = MOVETYPE_FLYMISSILE;
  missile.solid = SOLID_BBOX;
  makevectors(self.v_angle);
  missile.velocity = aim(self, 10000);
  missile.angles = vectoangles(missile.velocity);
  if (self.waterlevel > 2)
        missile.velocity = missile.velocity * 300;
  else
        missile.velocity = missile.velocity * 750;
  missile.touch = HammerTouch;
  missile.health = 100000;
  missile.takedamage = DAMAGE_YES;
  missile.th_die = T_MissileTouch;
  missile.nextthink = time;
  missile.bloodloss = time+2;
  missile.think = ThrowHammerThink;
  setmodel(missile, "models/hamthrow.mdl");
  setsize(missile,'-1 -2 -4','1 2 4');
//  setsize(missile, VEC_ORIGIN, VEC_ORIGIN);
  setorigin(missile, self.origin + v_forward * FL_SWIM + '0 0 16');
  missile.avelocity = '-500 0 0';
  missile.aflag = 0;
//  missile.effects = 4;
  missile.mass=200;
  self.amthammer = self.amthammer - 1;
  self.attack_finished=time + 0.5;
};

