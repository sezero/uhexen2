/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/breakabl.hc,v 1.1.1.1 2004-11-29 11:25:09 sezero Exp $
 */

/*
==============================================================================

BREAKABLE BRUSHES

==============================================================================
*/

float BREAK_KILLALL		= 1;
float BREAK_HIERARCH	= 2;
float BREAK_NOLINK		= 4;
float BREAK_CHECKNAME	= 8;
float BREAK_ORDERED		= 16;
float BREAK_TRANSLUCENT = 32;
float BREAK_INVINCIBLE  = 64;
float BREAK_INVISIBLE  = 128;

float (entity e1, entity e2) EntitiesTouching;

float breakhealth[21] = 
{
	0,
	75,		// THINGTYPE_GREYSTONE
	50,		// THINGTYPE_WOOD
	100,	// THINGTYPE_METAL
	30,		// THINGTYPE_FLESH
	999,	// THINGTYPE_FIRE
	25,		// THINGTYPE_CLAY
	35,		// THINGTYPE_LEAVES
	35,		// THINGTYPE_HAY
	75,		// THINGTYPE_BROWNSTONE
	35,		// THINGTYPE_CLOTH
	35,		// THINGTYPE_WOOD_LEAF
	75,		// THINGTYPE_WOOD_METAL
	65,		// THINGTYPE_WOOD_STONE
	90,		// THINGTYPE_METAL_STONE
	60,		// THINGTYPE_METAL_CLOTH
	10,		// THINGTYPE_WEB
	10,		// THINGTYPE_GLASS
	50,		// THINGTYPE_ICE
	10,		// THINGTYPE_CLEARCLASS
	10		// THINGTYPE_CLEARCLASS
};

//============================================================================

void linkBreakables()
{
  local entity	t, starte;
  local vector	cmins, cmaxs;

	if (self.enemy) return;		// already linked by another breakable
	
	if (self.spawnflags & BREAK_NOLINK)
	{
		self.owner = self.enemy = self;
		return;		// don't want to link this door
	}

	cmins = self.mins;
	cmaxs = self.maxs;
	
	starte = self;
	t = self;
	
	do
	{
		self.owner = starte;			// master breakable

		if (self.health) starte.health = self.health;
		if (self.targetname) starte.targetname = self.targetname;

		t = find (t, classname, self.classname);	
		if (!t)
		{
			self.enemy = starte;		// make the chain a loop

			self = self.owner;

			if (self.health) return;
			if (self.targetname)	return;
			if (self.items) return;

			return;
		}

		if (EntitiesTouching(self,t) && 
			(((self.spawnflags & BREAK_CHECKNAME) && (self.netname == t.netname)) ||
			(!self.spawnflags & BREAK_CHECKNAME)))
		{	
	
			if (t.enemy) 
			{
				//dprint("cross connected brushes!!\n");
				return;
			}
			
			self.enemy = t;
			self = t;

			if (t.mins_x < cmins_x)
				cmins_x = t.mins_x;
			if (t.mins_y < cmins_y)
				cmins_y = t.mins_y;
			if (t.mins_z < cmins_z)
				cmins_z = t.mins_z;
			if (t.maxs_x > cmaxs_x)
				cmaxs_x = t.maxs_x;
			if (t.maxs_y > cmaxs_y)
				cmaxs_y = t.maxs_y;
			if (t.maxs_z > cmaxs_z)
				cmaxs_z = t.maxs_z;
		}
	} while (1);
}


//============================================================================
void brush_use_hierarchy()
{
   local entity starte, oself, other;
	local float headNum;

	oself = starte = self;
	headNum = self.frags;

	do
	{
	  if (self.frags >= headNum) 
		self.th_die();

	  self = self.enemy;
	} while ( (self != starte) && (self != world) && (self.frags != self.cnt));
}


void brush_use_ordered()
{
   local entity starte, oself;

	starte = oself = self;

	self.health = self.max_health;
		
	if (self.frags == self.cnt) 
		self.th_die();
	else do
	{
		self = self.enemy;
	} while ( (self != oself) && (self != world) && (self.frags != self.cnt));
	
	if (self.frags == self.cnt && self != world && self != oself) self.th_die();

	do
	{
		oself.cnt += 1;
		oself = oself.enemy;
	} while ( (oself != starte) && (oself != world) );
}


void brush_use()
{
	local entity starte, other;
	
	starte = self;
	
	activator = other;

	if (starte.spawnflags & BREAK_KILLALL)
	{
		do
		{
			other = self.enemy;
			self.th_die();	
			self = other;
		} while ( (self != starte) && (self != world) );
	}
	else 
	{
//		dprint("Chunkalicious baby!\n");
		self.th_die();
	}
}

void brush_no_link_use (void)
{
//entity found, starte;

	SUB_UseTargets();
/*	if(self.target)
	{
		found=find(found,targetname,self.target);
		if(found!=world)
		{
			starte=found;
			found.think=found.use;
			thinktime found : 0;
			found=find(found,targetname,self.target);
			while(found!=starte&&found!=world)
			{
				found.think=found.use;
				thinktime found : 0;
				found=find(found,targetname,self.target);
			}
		}
	}
*/
	self.th_die();
}

/*QUAKED breakable_brush (0 0 1) ? KILLALL HIERARCH NOLINK CHECKNAME ORDERED TRANSLUCENT INVINCIBLE INVISIBLE
Breakable window or wall

You can manually control the heirarchy of breaking by targeting all the brushes you want this brush to break.
If you target a light with this object and turn on the "breaklight" spawnflag, it will turn off that light when it's broken.  The light will default to 300 if no lightvalue1 is given.

AUTOMATIC LINKING OPTIONS:
killall - when killed, the brush will kill all connected brushes
hierarch - link all brushes in a hierarchy.  The hierarchy priority is set in the 
			  frags field of each brush.  Lower numbers will kill higher numbers.  If
			  brushes share the same priority, they will die at the same time.
nolink - don't automatically link this brush with other brushes (use only manual targeting to link)
checkname - link brushes, but also check the name you place in the netname field.
				Brushes must then not only touch, but also have the same netname to link
ordered - like hierarch, except that no matter which brush you kill, the brushes 
			 will always break in a certain order.  The order is set in the frags field.
			 The brush with a frags set to 1 will break first, brush with frags set to
			 2 will break second, etc.

OTHER FIELDS:
translucent - you can see through it
invincible - can't be shot and broken, but will break by linking
-------------------------FIELDS-------------------------
flag - order number
thingtype - type of chunks and sprites it will generate
    0 - glass (default)
    1 - grey stone
    2 - wood
    3 - metal
    4 - flesh 
    5 - fire
    6 - clay
    7 - leaves
    8 - hay
    9 - brown stone
   10 - cloth
   11 - wood & leaf
   12 - wood & metal
   13 - wood stone
   14 - metal stone
   15 - metal cloth
   16 - spider web
   19 - clear glass
   20 - red glass

health - amount of damage item can take.  Default is based on thingtype
   glass       -  25
   grey stone  -  75
   wood        -  50
   metal       - 100
   flesh       -  30
   fire        - 999
   clay        -  25
   leaves      -  35
   hay         -  35
   brown stone -  75
   cloth       -  35
   wood&leaf   -  35
   wood&metal  -  75
   wood&stone  -  65
   metal&stone -  90
   metal&cloth -  60
   others      -  25

abslight - to set the absolute light level

--------------------------------------------------------

*/
void breakable_brush()
{
	self.max_health = self.health;
	self.solid = SOLID_BSP;
	self.movetype = MOVETYPE_PUSH;
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	if (self.spawnflags & BREAK_ORDERED) 
	{
	  self.th_die = brush_use_ordered;
	  self.cnt = 1;
	}
	else if (self.spawnflags & BREAK_HIERARCH) 
		self.th_die = brush_use_hierarchy;
	else 
		self.th_die = brush_use;

	if (self.spawnflags & BREAK_TRANSLUCENT) 
		self.drawflags(+)DRF_TRANSLUCENT;
	
	if(!self.thingtype)
		self.thingtype=THINGTYPE_GLASS;

	if (!self.health)
		self.health=breakhealth[self.thingtype];

	self.max_health = self.health;

	if (self.flags)
		self.frags = self.flags;

	if (self.abslight)
		self.drawflags(+)MLS_ABSLIGHT;

	if (self.spawnflags&BREAK_INVINCIBLE)
		self.takedamage = DAMAGE_NO;
	else
		self.takedamage = DAMAGE_YES;
//		self.takedamage = DAMAGE_NO_GRENADE;

	if (self.spawnflags&BREAK_INVISIBLE)
	{	
		self.effects (+) EF_NODRAW;
		self.th_die = SUB_Remove;
	}
	else
		self.th_die = chunk_death;

	if(self.spawnflags&BREAK_NOLINK)
		self.use=self.th_die;//brush_no_link_use;
	else
	{
		self.use = brush_use;
		self.think=linkBreakables;
		self.nextthink=self.ltime+0.1;
	}

	self.ltime = time;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:00  theoddone33
 * Inital import
 *
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 46    9/04/97 3:00p Mgummelt
 * 
 * 45    8/24/97 8:32p Mgummelt
 * 
 * 44    8/23/97 5:16p Rlove
 * 
 * 43    8/18/97 1:45p Mgummelt
 * 
 * 42    8/14/97 7:12p Mgummelt
 * 
 * 41    7/21/97 3:03p Rlove
 * 
 * 40    7/14/97 2:11p Mgummelt
 * 
 * 39    7/11/97 7:06a Rlove
 * 
 * 38    7/09/97 7:35a Rlove
 * New thingtype of CLEARGLASS
 * 
 * 37    6/30/97 9:31a Rlove
 * Added invisible breakable brushes
 * 
 * 36    6/25/97 9:23p Mgummelt
 * 
 * 35    6/18/97 4:00p Mgummelt
 * 
 * 34    6/17/97 2:30p Rlove
 * tighten up health code a bit
 * 
 * 33    6/15/97 5:10p Mgummelt
 * 
 * 32    6/14/97 2:59p Mgummelt
 * 
 * 31    6/06/97 10:58a Rjohnson
 * Fix for lights
 * 
 * 30    6/05/97 8:51p Mgummelt
 * 
 * 28    6/03/97 10:48p Mgummelt
 * 
 * 27    5/28/97 5:10p Rlove
 * 
 * 26    5/27/97 8:22p Mgummelt
 * 
 * 25    5/27/97 7:58a Rlove
 * New thingtypes of GreyStone,BrownStone, and Cloth.
 * 
 * 24    5/21/97 3:34p Rlove
 * New chunks
 * 
 * 23    5/17/97 1:23p Jweier
 * 
 * 22    5/12/97 11:11p Mgummelt
 * 
 * 21    5/11/97 9:11p Mgummelt
 * 
 * 20    5/06/97 4:27p Rjohnson
 * Added absolute light level
 * 
 * 19    4/26/97 3:52p Mgummelt
 * 
 * 18    4/24/97 2:53p Rjohnson
 * Added backpack functionality and spawning of objects
 * 
 * 17    4/21/97 12:30p Rjohnson
 * Added translucent flag
 * 
 * 16    4/09/97 4:56p Jweier
 * If a brush is targetted, it will not take damage
 * 
 * 15    3/28/97 10:58a Jweier
 * 
 * 14    3/28/97 10:40a Jweier
 * 
 * 13    3/28/97 10:07a Jweier
 * 
 * 12    3/27/97 12:29p Jweier
 * Added QuakeEd comments for clarification
 * 
 * 11    3/26/97 2:46p Aleggett
 * Allowed breakable brushes to "use" an entity when they "die".
 * 
 * 10    3/21/97 9:38a Rlove
 * Created CHUNK.HC and MATH.HC, moved brush_die to chunk_death so others
 * can use it.
 * 
 * 9     3/20/97 6:15p Jweier
 * 
 * 8     3/19/97 6:30p Jweier
 * reversed priority numbering (easier on designer)
 * 
 * 7     3/19/97 5:18p Jweier
 * added more hierarchial functionality
 * 
 * 6     3/15/97 5:48p Jweier
 * Ordered breaking!
 * 
 * 5     3/15/97 5:28p Jweier
 * Added in more features (ordered breaks, name checks, etc)
 * 
 * 4     3/15/97 4:16p Jweier
 * Basic hierarchial breakable object code added!
 * 
 * 3     3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 2     3/06/97 8:37a Rlove
 * Added thingtype of metal to Breakable entity
 * 
 * 1     3/04/97 4:24p Rlove
 */
