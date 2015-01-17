/*
 * $Header: /HexenWorld/Siege/Bridge.hc 3     5/25/98 1:38p Mgummelt $
 */
/*
==============================================================================

ROPE BRIDGES

==============================================================================
*/




/*
 * Rope Bridge entity fields:
 * ----------------------------
 *  .owner		- First link in this bridge.
 *  .count		- Number of the last link in the bridge.
 *  .cnt		- Number of this link.
 *  .movechain	- Next link in the chain.  Not used for last link.
 *  .dmg		- Amount of stress being put on this link.
 *  .enemy		- Last entity which was putting weight on this link.
 *  .speed		- Distance this link will sway in the wind.
 *  .yaw_speed  - Distance this link is from its origin cos of wind.
 */




/*
 * ropebridge_touch() -- Affects the bridge when it's walked on.
 */

void ropebridge_touch()
{
	local entity link;
	local float  difference;

	if(other.movetype != MOVETYPE_STEP && other.movetype != MOVETYPE_WALK)
		return;
	if(other == self.enemy)
		return;

	link		= self.owner.movechain;
	self.enemy	= other;

	while(link != self)					// Links from start to self
		{
		link.dmg = link.dmg - link.cnt / self.cnt;
		link	 = link.movechain;
		}

	self.dmg	= self.dmg + self.cnt;
	difference	= 1 / (self.count - self.cnt);
	link		= self.movechain;

	while(link.movechain)				// Links from self to end
		{
		link.dmg = link.dmg - difference * (self.count - link.cnt);
		link	 = link.movechain;
		}

	link = self.owner.movechain;
	while(link.movechain)
		{
		link.oldorigin_z = link.dmg;
		setorigin(link, link.origin + link.oldorigin);
		link = link.movechain;
		}
}



/*
 * ropebridge_relieveweight() -- Relieves weight from touching monsters/players.
 */

void ropebridge_relieveweight(entity focal)
{
	local entity link;

	link = self.owner.movechain;


}



/*
 * ropebridge_think() -- Sways the entire bridge a little.
 */

void ropebridge_think()
{
	local entity link;
	link = self.movechain;

	while(link.movechain)
		{
		if(link.enemy != world)
			if(!EntitiesTouching(link, link.enemy))
				{
				ropebridge_relieveweight(link);
				link.enemy = world;
				}
		if(link.t_length)
			setorigin(link, link.origin + (link.movedir * link.yaw_speed));
		else
			setorigin(link, link.origin - (link.movedir * link.yaw_speed));
		if(link.t_width)
			{
			link.yaw_speed += 0.7;
			if(link.yaw_speed >= link.speed)
				{
				link.t_width	= 0;
				link.yaw_speed	= link.speed;
				}
			}
		else {
			link.yaw_speed -= 0.7;
			if(link.yaw_speed < 0)
				{
				link.t_length	= 0;
				link.t_width	= 1;
				link.yaw_speed	= 0;
				}
			}
		link = link.movechain;
		}

	self.nextthink = time + 0.05;
}


/*
 * ropebridge_init() -- Sets up each link of the bridge.
 *                      Only called by the first link.
 */

void ropebridge_init()
{
	local entity	link;
	local float		midpoint, thispoint;

	link = self;

	while(link.target)
		{
		if(link.classname != "ropebridge")
			{
			bprint("Error: Rope bridge link ");
			bprint(ftos(self.count));
			bprint(" is targeted wrong\n");
			return;
			}
		link.owner		= self;
		link.movechain	= find(world, targetname, link.target);
		link.cnt		= self.count;
		link			= link.movechain;
		self.count		= self.count + 1;
		}

	link				= self;
	midpoint			= self.count * 0.5;
	thispoint			= 1;

	while(thispoint <= self.count)
		{
		if(thispoint < midpoint)
			link.speed	= sqrt(thispoint - 1);
		else
			link.speed	= sqrt(self.count - thispoint);
		link.yaw_speed	= link.speed;
		link.count		= self.count;
		link			= link.movechain;
		thispoint		= thispoint + 1;
		}

	self.nextthink		= time + 0.05;
	self.think			= ropebridge_think;
}



/*QUAKED ropebridge_link (0.3 0.1 0.6) ? START
A link of a rope bridge which sways in the wind and drops a little when it's walked on.
--------------------------------------
.target - The next link in the bridge.
          Don't set if it's the last
		  link.
START   - Check this if this is the
          first link in the bridge.
--------------------------------------
*/

void ropebridge_link()
{
	self.solid			= SOLID_BSP;
	self.movetype		= MOVETYPE_PUSH;
	self.classname		= "ropebridge";
	self.movedir		= '0 1 -0.4';
	setmodel (self, self.model);
	setorigin(self, self.origin);

	if(self.spawnflags)
		{
		self.count		= 1;
		self.nextthink	= time + 0.05;
		self.think		= ropebridge_init;
		}
	else
		self.touch		= ropebridge_touch;
}

