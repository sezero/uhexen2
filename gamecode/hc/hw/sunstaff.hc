/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/sunstaff.hc,v 1.2 2007-02-07 16:58:07 sezero Exp $
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
	vector	end1, end2, end3;
	//Draw a larger pulsating transparent yellow beam,
	//rotating, with a smaller solid white beam in the
	//center.  Player casts brightlight while using it.
	//each point reflection has a double-sphere similar
	//to the beam.  Each reflection does 3/4 less damage.
	//primary damage = 37?

	// make some global to keep track of the last time the light showed up...

	if(self.attack_finished>time)
		return;

	self.effects(+)EF_BRIGHTLIGHT;
	if(self.artifact_active&ART_TOMEOFPOWER)
		damg=35;
	else
		damg=14;

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

	end1 = trace_endpos;

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

		if(reflect_count == 1)
		{
			end2 = trace_endpos;
		}
		else
		{
			end3 = trace_endpos;
		}

	    LightningDamage (org1, trace_endpos+normalize(dir)*7, self, damg/2,"sunbeam");
	}

	// so each of these will cost either 17, 23, or 29 bytes instead of the 30, 60, or 90 of before
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_SUNSTAFF_CHEAP);
	WriteEntity (MSG_MULTICAST, self);
	WriteByte (MSG_MULTICAST, reflect_count);
	WriteCoord(MSG_MULTICAST, org2_x);
	WriteCoord(MSG_MULTICAST, org2_y);
	WriteCoord(MSG_MULTICAST, org2_z);
	WriteCoord(MSG_MULTICAST, end1_x);
	WriteCoord(MSG_MULTICAST, end1_y);
	WriteCoord(MSG_MULTICAST, end1_z);
	if(reflect_count > 0)
	{
		WriteCoord(MSG_MULTICAST, end2_x);
		WriteCoord(MSG_MULTICAST, end2_y);
		WriteCoord(MSG_MULTICAST, end2_z);
		if(reflect_count > 1)
		{
			WriteCoord(MSG_MULTICAST, end3_x);
			WriteCoord(MSG_MULTICAST, end3_y);
			WriteCoord(MSG_MULTICAST, end3_z);
		}
	}
	multicast(self.origin,MULTICAST_PHS);
}

void() FireSunstaffPower =
{
	vector  org1, curOrg;
	float	curFraction;
	entity	curIgnore, hurtGuy;

	if(self.attack_finished>time)
		return;

	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	org1 = self.origin + self.proj_ofs+ v_forward*7;
    traceline (org1, org1 + v_forward * 1000.0, TRUE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_SUNSTAFF_POWER);
	WriteEntity(MSG_MULTICAST, self);
	WriteCoord (MSG_MULTICAST, org1_x);
	WriteCoord (MSG_MULTICAST, org1_y);
	WriteCoord (MSG_MULTICAST, org1_z);
	WriteCoord (MSG_MULTICAST, trace_endpos_x);
	WriteCoord (MSG_MULTICAST, trace_endpos_y);
	WriteCoord (MSG_MULTICAST, trace_endpos_z);
	multicast(self.origin,MULTICAST_PHS_R);

//    LightningDamage (org1 - v_forward*7, trace_endpos, self, 80, "sunbeam");

	hurtGuy=findradius(trace_endpos,200);
	while(hurtGuy)
	{
		T_Damage(hurtGuy, self, self, 160 * (1.0 - (vlen(hurtGuy.origin - trace_endpos)/200)));
		hurtGuy=hurtGuy.chain;
	}

	curFraction = 1000.0;
	curOrg = org1;
	curIgnore = self;
	while((curFraction > 0)&&(curIgnore != world))
	{
		traceline(curOrg, org1 + v_forward * 1000.0, FALSE, curIgnore);
		if(trace_fraction < 1.0)
		{
			curFraction -= vlen(trace_endpos - curOrg);
			curOrg = trace_endpos;
			curIgnore = trace_ent;

			T_Damage (trace_ent, self, self, 60);
		}
		else
		{
			curFraction -= 1002.0;
		}
	}
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
	if(self.attack_finished<=time&&self.button0&&self.greenmana>=1&&self.bluemana>=1)
	{
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			if(self.greenmana >= 35 && self.bluemana >= 35)
			{
				FireSunstaffPower();
				self.greenmana-=20;
				self.bluemana-=20;
				self.attack_finished=time + 1.0;
			}
		}
		else
		{
			if(self.t_width!=SOUND_STARTED)
			{
				sound(self,CHAN_UPDATE+PHS_OVERRIDE_R,"crusader/sunhuml.wav",1,ATTN_LOOP);
				self.effects(+)EF_UPDATESOUND;
				self.t_width=SOUND_STARTED;
			}
			makevectors(self.v_angle);
			FireSunstaff(v_forward,0);
			self.greenmana-=0.7;
			self.bluemana-=0.7;
			self.attack_finished=time + 0.1;
		}
	}

	if(self.wfs==WF_CYCLE_WRAPPED&&(!self.button0||
		((self.greenmana<1||self.bluemana<1)&&(!(self.artifact_active&ART_TOMEOFPOWER)))||
		((self.greenmana<20||self.bluemana<20)&&(self.artifact_active&ART_TOMEOFPOWER))
		))
	{
		stopSound(self,CHAN_UPDATE);
		self.t_width=SOUND_STOPPED;
		self.effects(-)EF_BRIGHTLIGHT|EF_UPDATESOUND;
		sunstaff_fire_settle();
	}
}

void sunstaff_fire (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		sunstaff_fire_loop();
	}
	else
	{
		self.wfs = advanceweaponframe($fire1,$fire4);
		self.th_weapon=sunstaff_fire;
		if(self.wfs==WF_CYCLE_WRAPPED)
			sunstaff_fire_loop();
	}
}

void() Cru_Sun_Fire =
{
	if(self.weaponframe<$idle1 || self.weaponframe>$idle31)
		return;

	//sound (self, CHAN_AUTO, "crusader/sunstart.wav", 1, ATTN_NORM);
	weapon_sound(self, "crusader/sunstart.wav");
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

