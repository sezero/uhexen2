/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/portals/sunstaff.hc,v 1.1.1.1 2004-11-29 11:36:04 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\sunstaff\final\newfinal\sunstaff.hc
MG
==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\sunstaff\final\newfinal
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame fircyc1      fircyc2      fircyc3      fircyc4      fircyc5      
$frame fircyc6      fircyc7      fircyc8      fircyc9      fircyc10     

//
$frame fire1        fire2        fire3        fire4        

//
$frame idle1        idle2        idle3        idle4        idle5        
$frame idle6        idle7        idle8        idle9        idle10       
$frame idle11       idle12       idle13       idle14       idle15       
$frame idle16       idle17       idle18       idle19       idle20       
$frame idle21       idle22       idle23       idle24       idle25       
$frame idle26       idle27       idle28       idle29       idle30       
$frame idle31       

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     select13     select14     

//
$frame settle1      settle2      settle3      settle4      settle5      



void FireSunstaff (vector dir, float ofs)
{
vector  org1,org2, vec, dir, endspot,endplane;
float remainder, reflect_count,damg;
//Draw a larger pulsating transparent yellow beam,
//rotating, with a smaller solid white beam in the
//center.  Player casts brightlight while using it.
//each point reflection has a double-sphere similar
//to the beam.  Each reflection does 3/4 less damage.
//primary damage = 37?

	if(self.attack_finished>time)
		return;

	if(self.t_width<time)
	{
		sound(self,CHAN_WEAPON,"crusader/sunhum.wav",1,ATTN_NORM);
		self.t_width=time+0.2;
	}

	self.effects(+)EF_BRIGHTLIGHT;
	if(self.artifact_active&ART_TOMEOFPOWER&&!ofs)
		damg=17;
	else
		damg=7;

//If powered up, start a quake
	if(!ofs)
		remainder=1000;
	else
		remainder=750;
	makevectors(self.v_angle);
	org1 = self.origin + self.proj_ofs+ v_forward*7;
	org2 = org1 + dir*20;
    vec = org2 + dir*remainder;
    traceline (org2, vec, TRUE, self);
	endspot=trace_endpos;
	endplane=trace_plane_normal;
	remainder-=remainder*trace_fraction;

	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_STREAM_SUNSTAFF1);
	WriteEntity (MSG_BROADCAST, self);
	WriteByte (MSG_BROADCAST, ofs+STREAM_ATTACHED);
	WriteByte (MSG_BROADCAST, 1);
	WriteCoord (MSG_BROADCAST, org2_x);
	WriteCoord (MSG_BROADCAST, org2_y);
	WriteCoord (MSG_BROADCAST, org2_z);
	WriteCoord (MSG_BROADCAST, trace_endpos_x);
	WriteCoord (MSG_BROADCAST, trace_endpos_y);
	WriteCoord (MSG_BROADCAST, trace_endpos_z);

    LightningDamage (org1 - v_forward*7, trace_endpos+normalize(dir)*7, self, damg,"sunbeam");

   while(remainder>0&&reflect_count<2)
   {
		org1 = endspot;
	    dir +=2*endplane;

		vec = org1 + normalize(dir)*remainder;
//FIXME: what about reflective triggers?
		traceline (org1,vec,TRUE,self);
		endspot=trace_endpos;
		endplane=trace_plane_normal;
		remainder-=remainder*trace_fraction;
		reflect_count+=1;

		WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
		WriteByte (MSG_BROADCAST, TE_STREAM_SUNSTAFF1);
		WriteEntity (MSG_BROADCAST, self);
		WriteByte (MSG_BROADCAST, ofs+reflect_count);
		WriteByte (MSG_BROADCAST, 2);
		WriteCoord (MSG_BROADCAST, org1_x);
		WriteCoord (MSG_BROADCAST, org1_y);
		WriteCoord (MSG_BROADCAST, org1_z);
		WriteCoord (MSG_BROADCAST, trace_endpos_x);
		WriteCoord (MSG_BROADCAST, trace_endpos_y);
		WriteCoord (MSG_BROADCAST, trace_endpos_z);

	    LightningDamage (org1, trace_endpos+normalize(dir)*7, self, damg/2,"sunbeam");
	}
}

void() FireSunstaffPower =
{
/*
Will fire a main thick beam that will reflect off walls
and cut through stuff.
Two beams on side will waver a bit and track at + & - 45 degrees.
If the main beam hits something, the two secondary beams will
pull forward and lock on as well.
*/
vector dir1,dir2;
	makevectors(self.v_angle);
//NOTE: Maybe extra beams cycle around- rotate around 1st beam.
	dir1=(v_forward+v_right)*0.5;
	dir2=(v_forward+(v_right*-1))*0.5;
	FireSunstaff(normalize(v_forward),0);
	FireSunstaff(dir1,4);
	FireSunstaff(dir2,8);
};

void()sunstaff_ready_loop;
void() Cru_Sun_Fire;

void sunstaff_fire_settle ()
{
	self.wfs = advanceweaponframe($settle1,$settle5);
	self.th_weapon=sunstaff_fire_settle;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.effects(-)EF_BRIGHTLIGHT;
		self.last_attack=time;
		sunstaff_ready_loop();
	}
}

void sunstaff_fire_loop ()
{
	self.wfs = advanceweaponframe($fircyc1,$fircyc10);
	self.th_weapon=sunstaff_fire_loop;
	if(self.attack_finished<=time&&self.button0&&self.greenmana>=2&&self.bluemana>=2)
	{
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			FireSunstaffPower();
			self.greenmana-=2;
			self.bluemana-=2;
		}
		else
		{
			makevectors(self.v_angle);
			FireSunstaff(v_forward,0);
			self.greenmana-=0.35;
			self.bluemana-=0.35;
		}
		self.attack_finished=time + 0.05;
	}

	if(self.wfs==WF_CYCLE_WRAPPED&&(!self.button0||self.greenmana<2||self.bluemana<2))
	{
		self.effects(-)EF_BRIGHTLIGHT;
		sunstaff_fire_settle();
	}
}

void sunstaff_fire (void)
{
	self.wfs = advanceweaponframe($fire1,$fire4);
	self.th_weapon=sunstaff_fire;
	if(self.wfs==WF_CYCLE_WRAPPED)
		sunstaff_fire_loop();
}

void() Cru_Sun_Fire =
{
	if(self.weaponframe<$idle1 || self.weaponframe>$idle31)
		return;

	sound (self, CHAN_AUTO, "crusader/sunstart.wav", 1, ATTN_NORM);
	self.th_weapon=sunstaff_fire;
	thinktime self : 0;
};

void sunstaff_ready_loop (void)
{
	self.wfs = advanceweaponframe($idle1,$idle31);
	self.th_weapon=sunstaff_ready_loop;
}

void sunstaff_select (void)
{
//go to ready loop, not relaxed?
	self.wfs = advanceweaponframe($select1,$select14);
	self.weaponmodel = "models/sunstaff.mdl";
	self.th_weapon=sunstaff_select;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		sunstaff_ready_loop();
	}
}

void sunstaff_deselect (void)
{
//go to ready loop, not relaxed?
	self.wfs = advanceweaponframe($select14,$select1);
	self.th_weapon=sunstaff_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}


/*
 * $Log: not supported by cvs2svn $
 * 
 * 56    10/28/97 1:01p Mgummelt
 * Massive replacement, rewrote entire code... just kidding.  Added
 * support for 5th class.
 * 
 * 54    9/07/97 9:42a Mgummelt
 * 
 * 53    9/02/97 7:54p Mgummelt
 * 
 * 52    9/01/97 8:18p Mgummelt
 * 
 * 51    8/31/97 3:57p Mgummelt
 * 
 * 50    8/30/97 6:58p Mgummelt
 * 
 * 49    8/29/97 4:17p Mgummelt
 * Long night
 * 
 * 48    8/26/97 6:01p Mgummelt
 * 
 * 47    8/26/97 7:38a Mgummelt
 * 
 * 46    8/26/97 2:26a Mgummelt
 * 
 * 45    8/13/97 5:49p Mgummelt
 * 
 * 44    8/08/97 6:21p Mgummelt
 * 
 * 43    8/07/97 10:31p Mgummelt
 * 
 * 42    8/06/97 10:19p Mgummelt
 * 
 * 41    7/25/97 4:19p Mgummelt
 * 
 * 40    7/24/97 8:47p Mgummelt
 * 
 * 39    7/24/97 12:33p Mgummelt
 * 
 * 38    7/24/97 3:27a Mgummelt
 * 
 * 37    7/21/97 4:04p Mgummelt
 * 
 * 36    7/21/97 4:02p Mgummelt
 * 
 * 35    7/21/97 11:45a Mgummelt
 * 
 * 34    7/19/97 9:53p Mgummelt
 * 
 * 33    7/18/97 3:55p Mgummelt
 * 
 * 32    7/18/97 11:06a Mgummelt
 * 
 * 31    7/15/97 8:31p Mgummelt
 * 
 * 30    7/14/97 9:30p Mgummelt
 * 
 * 29    7/10/97 7:21p Mgummelt
 * 
 * 28    7/09/97 6:31p Mgummelt
 * 
 * 27    7/03/97 2:52p Mgummelt
 * 
 * 26    7/01/97 3:30p Mgummelt
 * 
 * 25    7/01/97 2:21p Mgummelt
 * 
 * 24    7/01/97 12:15p Mgummelt
 * 
 * 23    6/30/97 5:39p Mgummelt
 * 
 * 22    6/18/97 4:22p Mgummelt
 * 
 * 21    6/18/97 4:00p Mgummelt
 * 
 * 20    6/05/97 9:30a Rlove
 * Weapons now have deselect animations
 * 
 * 19    6/04/97 8:16p Mgummelt
 * 
 * 18    5/23/97 11:51p Mgummelt
 * 
 * 17    5/19/97 11:36p Mgummelt
 * 
 * 16    5/15/97 11:40a Mgummelt
 * 
 * 15    5/06/97 8:30p Mgummelt
 * 
 * 14    5/06/97 1:29p Mgummelt
 * 
 * 13    5/05/97 10:29a Mgummelt
 * 
 * 12    5/02/97 8:06p Mgummelt
 * 
 * 11    4/30/97 7:38p Mgummelt
 * 
 * 10    4/30/97 5:02p Mgummelt
 * 
 * 9     4/28/97 6:54p Mgummelt
 * 
 * 8     4/25/97 8:32p Mgummelt
 * 
 * 7     4/24/97 2:22p Mgummelt
 * 
 * 6     4/21/97 12:30p Mgummelt
 * 
 * 5     4/16/96 11:52p Mgummelt
 * 
 * 4     4/14/96 3:47p Mgummelt
 * 
 * 3     4/13/96 3:31p Mgummelt
 * 
 * 2     4/12/96 8:56p Mgummelt
 * 
 * 1     4/12/96 8:04p Mgummelt
 */
