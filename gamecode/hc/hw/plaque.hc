/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/hw/plaque.hc,v 1.1.1.1 2004-11-29 11:28:33 sezero Exp $
 */

float PLAQUE_INVISIBLE = 1;
float PLAQUE_ACTIVATE  = 2;

/*
================
plague_use

Activate a plaque
================
*/
void plaque_use (void)
{
	if (self.spawnflags & PLAQUE_ACTIVATE)
		self.inactive = 0;
}

void plaque_touch (void)
{
	vector	spot1, spot2;	

	if (self.inactive)
		return;

	if ((other.classname == "player") && (!other.plaqueflg))
	{
		makevectors (other.v_angle);
		spot1 = other.origin + other.view_ofs;
		spot2 = spot1 + (v_forward*25); // Look just a little ahead
		traceline (spot1, spot2 , FALSE, other);

		if ((trace_fraction == 1.0) || (trace_ent.classname!="plaque"))
		{
			traceline (spot1, spot2 - (v_up * 30), FALSE, other);  // 30 down
		
			if ((trace_fraction == 1.0) || (trace_ent.classname!="plaque"))
			{
				traceline (spot1, spot2 + v_up * 30, FALSE, other);  // 30 up
			
				if ((trace_fraction == 1.0) || (trace_ent.classname!="plaque"))
					return;
			}
		}

		other.plaqueflg = 1;
		other.plaqueangle = other.v_angle;
		msg_entity = other;
	 	plaque_draw(MSG_ONE,self.message);

  		if (other.noise1 != "")
  			sound (other, CHAN_VOICE, self.noise1, 1, ATTN_NORM);
		else
			sound (other, CHAN_ITEM, self.noise, 1, ATTN_NORM);
	}
}

/*QUAKED plaque (.5 .5 .5) ? INVISIBLE deactivated

A plaque on the wall a player can read
-------------------------FIELDS-------------------------

"message" the index of the string in the text file

"noise1" the wav file activated when plaque is used

deactivated - if this is on, the plaque will not be readable until a trigger has activated it.
--------------------------------------------------------
*/
void() plaque =
{
	setsize (self, self.mins, self.maxs);
	setorigin (self, self.origin);	
	setmodel (self, self.model);
	self.solid = SOLID_SLIDEBOX;

	if (deathmatch)  // I don't do a remove because they might be a part of the architecture
		return;

	self.use = plaque_use;

	precache_sound("raven/use_plaq.wav");
	self.noise = "raven/use_plaq.wav";

	self.touch = plaque_touch;

	if (self.spawnflags & PLAQUE_INVISIBLE)
		self.effects (+) EF_NODRAW;

	if (self.spawnflags & PLAQUE_ACTIVATE)
		self.inactive = 1;
	else
		self.inactive = 0;
};
/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:05:09  theoddone33
 * Inital import
 *
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 19    8/16/97 9:16a Rlove
 * 
 * 18    7/23/97 7:04p Mgummelt
 * 
 * 17    7/17/97 3:12p Rlove
 * 
 * 16    7/17/97 7:27a Rlove
 * 
 * 15    7/16/97 3:54p Rjohnson
 * Fix for plaques
 * 
 * 14    7/11/97 7:25a Rlove
 * 
 * 13    7/08/97 3:23p Rjohnson
 * Switched messages to using a string index
 * 
 * 12    7/07/97 6:13p Rlove
 * Now there are invisible plaques
 * 
 * 11    6/25/97 8:35p Rjohnson
 * Made the plaque network friendly
 * 
 * 10    6/25/97 12:49p Rjohnson
 * Added a global text file
 * 
 * 9     5/28/97 1:43p Rlove
 * Plaques now activate when you bump into them.
 * 
 * 8     3/24/97 8:56p Rjohnson
 * 
 * 7     3/14/97 9:21a Rlove
 * Plaques are done 
 * 
 * 6     2/19/97 10:06a Rlove
 * New Pull Object and Plaque Code
 * 
 * 5     2/13/97 4:22p Rlove
 * 
 * 4     11/11/96 1:19p Rlove
 * Added Source Safe stuff
 */
