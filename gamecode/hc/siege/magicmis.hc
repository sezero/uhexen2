/*
 * $Header: /home/ozzie/Download/0000/uhexen2/gamecode/hc/siege/magicmis.hc,v 1.1 2005-01-26 17:26:11 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\spllbook\spllbook.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\spllbook
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame fire1        fire2        fire3        fire4        fire5        
$frame fire6        fire7        fire8        fire9        fire10       
$frame fire11       fire12       

//
$frame go2mag01     go2mag02     go2mag03     go2mag04     go2mag05     
$frame go2mag06     go2mag07     go2mag08     go2mag09     go2mag10     
$frame go2mag11     go2mag12     go2mag13     
$frame go2shd1      go2shd2      
$frame go2shd3      go2shd4      go2shd5      go2shd6      go2shd7      
$frame go2shd8      go2shd9      
$frame go2shd10		go2shd11     go2shd12     go2shd13     go2shd14

//
$frame idle1        idle2        idle3        idle4        idle5        
$frame idle6        idle7        idle8        idle9        idle10       
$frame idle11       idle12       idle13       idle14       idle15       
$frame idle16       idle17       idle18       idle19       idle20       
$frame idle21       idle22       

//
$frame mfire1       mfire2       mfire3       mfire4       mfire5       
$frame mfire6       mfire7       mfire8       

//
$frame midle01      midle02      midle03      midle04      midle05      
$frame midle06      midle07      midle08      midle09      midle10      
$frame midle11      midle12      midle13      midle14      midle15      
$frame midle16      midle17      midle18      midle19      midle20      
$frame midle21      midle22      

//
$frame mselect01    mselect02    mselect03    mselect04    mselect05    
$frame mselect06    mselect07    mselect08    mselect09    mselect10    
$frame mselect11    mselect12    mselect13    mselect14    mselect15    
$frame mselect16    mselect17    mselect18    mselect19    mselect20    

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      



void MagicMissileTouch (void)
{
	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	self.level=FALSE;
	if(other.takedamage)
	{
		if(!other.flags2&FL_ALIVE)
			self.dmg/=3;//less damage to non-living things
		T_Damage(other,self,self.owner,self.dmg);
	}

	T_RadiusDamage(self,self.owner,self.dmg,other);
//	sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
	starteffect(CE_MAGIC_MISSILE_EXPLOSION,self.origin-self.movedir*8,0.05);
	remove(self);
}

void MagicMissileThink()
{
	HomeThink();
	if(self.lifetime<time)
		self.th_die();
	else
		thinktime self : self.homerate;

}

void FireMagicMissile (float offset)
{
//entity star1,star2;
vector spread;

	if(self.classname=="monster_eidolon")
		v_forward=self.v_angle;
	else
		makevectors(self.v_angle);

	self.effects(+)EF_MUZZLEFLASH;
	newmis=spawn();
	newmis.classname="magic missile";
	newmis.owner=self;
	newmis.drawflags(+)SCALE_ORIGIN_CENTER;//|DRF_TRANSLUCENT;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.drawflags(+)MLS_FIREFLICKER;

	newmis.touch=MagicMissileTouch;
	newmis.dmg=random(7,17);

	newmis.speed=1000;
	spread=normalize(v_right)*(offset*25);
	newmis.velocity=normalize(v_forward)*newmis.speed + spread;
	newmis.movedir=normalize(newmis.velocity);
	newmis.avelocity_z=random(300,600);
	newmis.level=TRUE;

	setsize(newmis,'0 0 0','0 0 0');

	if(self.classname=="monster_eidolon")
	{
		newmis.scale=0.75;
		setorigin(newmis,self.origin+self.proj_ofs+v_forward*48+v_right*20);
		weapon_sound(self, "eidolon/spell.wav");
//		sound(self,CHAN_AUTO,"eidolon/spell.wav",1,ATTN_NORM);
	}
	else
	{
		newmis.scale=0.5;
		setorigin(newmis,self.origin+self.proj_ofs+v_forward*8+v_right*7+'0 0 5');
		weapon_sound(self, "necro/mmfire.wav");
//		sound(newmis,CHAN_AUTO,"necro/mmfire.wav",1,ATTN_NORM);
	}

	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		if(self.classname=="monster_eidolon")
		{
			newmis.enemy=self.enemy;
			newmis.classname = "eidolon spell";
			newmis.turn_time=3;
			newmis.dmg=random(30,40);
		}
		else
		{
			newmis.turn_time=2;
			newmis.dmg=random(30,35);
		}
		newmis.frags=TRUE;
		newmis.veer=100;
		newmis.homerate=0.2;
		newmis.lifetime=time+5;
		newmis.think=MagicMissileThink;
		newmis.th_die = SUB_Remove;
		newmis.hoverz=TRUE;
		thinktime newmis : 0.1;
		setmodel(newmis,"models/ball.mdl");
	}
	else
	{
		newmis.lifetime=time+5;
		newmis.veer=100;
		newmis.think=VeerThink;
		thinktime newmis : 0.2;
		setmodel(newmis,"models/ball.mdl");
	}
	// note to coders:
	// the ball model is NOT necessarily used, the networking code
	// will turn the ball into a tightly packed list of locations
	// those locations will have the newmmis model displayed
	// unless scale is set to .1 (for powered ice mace)
	// eidolon's attack will also appear as a magic missile now
	return;
}

void FireFlash ()
{
	vector org;
	
	makevectors(self.v_angle);
	org = self.origin+self.proj_ofs+'0 0 5'+v_right*2+v_forward*6;
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_HWMISSILEFLASH);
	WriteCoord(MSG_MULTICAST, org_x);
	WriteCoord(MSG_MULTICAST, org_y);
	WriteCoord(MSG_MULTICAST, org_z);
	WriteCoord(MSG_MULTICAST, self.v_angle_x*-1);
	WriteCoord(MSG_MULTICAST, self.v_angle_y);
	multicast(org,MULTICAST_PHS_R);

}

void  mmis_power()
{
	if(self.attack_finished>time)
		return;

	FireFlash();
	FireMagicMissile(-3);
	FireMagicMissile(0);
	FireMagicMissile(3);
	self.greenmana-=10;
	self.attack_finished=time+0.7;
}

void  mmis_normal()
{
	if(self.attack_finished>time)
		return;

	FireFlash();
	FireMagicMissile(0);
	self.greenmana-=2;
	self.attack_finished=time+0.3;
}

/*======================
ACTION
select
deselect
ready loop
relax loop
fire once
fire loop
ready to relax(after short delay)
relax to ready(Fire delay?  or automatic if see someone?)
=======================*/


void()magicmis_ready;
void() Nec_Mis_Attack;

void magicmis_fire (void)
{
	if(self.button0&&self.weaponframe==$mfire5 &&!self.artifact_active&ART_TOMEOFPOWER)
		self.weaponframe=$mfire5;
	else
		self.wfs = advanceweaponframe($mfire1,$mfire8);
	self.th_weapon=magicmis_fire;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED||self.greenmana<2)//||(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana<10))
		magicmis_ready();
	else if(self.weaponframe==$mfire5)// &&self.attack_finished<=time)
//		if(self.artifact_active&ART_TOMEOFPOWER)
//			mmis_power();
//		else
			mmis_normal();
}

void() Nec_Mis_Attack =
{
	magicmis_fire();

	thinktime self : 0;
};

void magicmis_jellyfingers ()
{
	self.wfs = advanceweaponframe($midle01,$midle22);
	self.th_weapon=magicmis_jellyfingers;
	if(self.wfs==WF_CYCLE_WRAPPED)
		magicmis_ready();
}

void magicmis_ready (void)
{
	self.weaponframe=$midle01;
	if(random()<0.1&&random()<0.3&&random()<0.5)
		self.th_weapon=magicmis_jellyfingers;
	else
		self.th_weapon=magicmis_ready;
}


void magicmis_select (void)
{
	self.wfs = advanceweaponframe($mselect01,$mselect20);
	self.weaponmodel = "models/spllbook.mdl";
	self.th_weapon=magicmis_select;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		magicmis_ready();
	}
}

void magicmis_deselect (void)
{
	self.wfs = advanceweaponframe($mselect20,$mselect01);
	self.th_weapon=magicmis_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void magicmis_select_from_bone (void)
{
	self.wfs = advanceweaponframe($go2mag01,$go2mag13);
	self.weaponmodel = "models/spllbook.mdl";
	self.th_weapon=magicmis_select_from_bone;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		magicmis_ready();
	}
}

/*
 * $Log: not supported by cvs2svn $
 * 
 * 21    5/25/98 1:39p Mgummelt
 * 
 * 20    4/16/98 4:34p Mgummelt
 * 
 * 18    4/07/98 4:07p Rmidthun
 * reduced damage for powered missiles
 * 
 * 17    4/03/98 1:10p Rmidthun
 * fixed bug caused by removing the th_die code
 * 
 * 16    4/02/98 11:55a Rmidthun
 * cleaning up the code, removing the old client effect stuff
 * 
 * 15    4/01/98 1:27p Rmidthun
 * reduced damage of these weapons to bring them more in line with the
 * other classes
 * 
 * 14    3/12/98 1:19p Rmidthun
 * use weapon sound for firing sounds
 * 
 * 13    3/10/98 1:54p Rmidthun
 * send missiles as one packet (like the nails) - removes client effect
 * issues, comparable net traffic.  Firing tomed missiles causes major
 * slowdown on server, my test machine doesn't handle this well at all so
 * I'm not sure how it looks in reality.
 * 
 * 12    3/05/98 4:21p Rmidthun
 * made regular fire a little slower and more powerful
 * 
 * 11    3/04/98 10:13a Rmidthun
 * reduced homing rate to cut on net messages, also staggered homing
 * thinks so they won't all hit at the same time
 * 
 * 10    2/26/98 4:19p Rmidthun
 * added pos to turneffect so it can correct if need be, still working on
 * the actual correction
 * 
 * 9     2/24/98 11:44a Rmidthun
 * magic missiles are client effects, the homing function sends updates
 * 
 * 8     2/23/98 2:23p Rmidthun
 * changed broadcasts to multicast, this should reduce net traffic for
 * larger games
 * 
 * 7     2/20/98 4:45p Rmidthun
 * moved rest of sounds to client side
 * 
 * 6     2/20/98 11:17a Rmidthun
 * magic missile flash to tempent
 * 
 * 5     2/18/98 11:50a Rmidthun
 * eidolon missile effects (in case of coop)
 * 
 * 4     2/18/98 11:01a Rmidthun
 * fixed bug when the stars outlasted the missile, oops.
 * 
 * 3     2/18/98 10:29a Rmidthun
 * moved missile stars to client effect (finished)
 * 
 * 2     2/17/98 6:53p Rmidthun
 * moved missile stars to client, about half done, checked in so others
 * can get to constant.hc
 * 
 * 1     2/04/98 1:59p Rjohnson
 * 
 * 50    10/07/97 2:22p Mgummelt
 * 
 * 49    9/19/97 8:47a Rlove
 * 
 * 48    9/17/97 11:22a Rlove
 * 
 * 47    9/17/97 11:11a Rlove
 * 
 * 46    9/07/97 9:42a Mgummelt
 * 
 * 45    9/03/97 6:01a Mgummelt
 * 
 * 44    9/03/97 12:25a Mgummelt
 * 
 * 43    9/01/97 7:41p Mgummelt
 * 
 * 42    8/31/97 8:52a Mgummelt
 * 
 * 41    8/26/97 7:38a Mgummelt
 * 
 * 40    8/25/97 1:09a Mgummelt
 * 
 * 39    8/17/97 3:06p Mgummelt
 * 
 * 38    8/13/97 2:56p Mgummelt
 * 
 * 37    8/11/97 5:31p Mgummelt
 * 
 * 36    8/08/97 6:21p Mgummelt
 * 
 * 35    8/07/97 10:30p Mgummelt
 * 
 * 34    8/06/97 10:19p Mgummelt
 * 
 * 33    8/04/97 8:03p Mgummelt
 * 
 * 32    8/01/97 9:52p Mgummelt
 * 
 * 31    7/30/97 10:43p Mgummelt
 * 
 * 30    7/30/97 8:26p Mgummelt
 * 
 * 29    7/30/97 3:33p Mgummelt
 * 
 * 28    7/29/97 6:54p Mgummelt
 * 
 * 27    7/29/97 5:45p Mgummelt
 * 
 * 26    7/29/97 3:46p Mgummelt
 * 
 * 25    7/28/97 8:27p Mgummelt
 * 
 * 24    7/28/97 7:50p Mgummelt
 * 
 * 23    7/24/97 12:32p Mgummelt
 * 
 * 22    7/15/97 2:20p Mgummelt
 * 
 * 21    7/14/97 9:30p Mgummelt
 * 
 * 20    7/10/97 7:21p Mgummelt
 * 
 * 19    7/09/97 6:31p Mgummelt
 * 
 * 18    7/01/97 3:30p Mgummelt
 * 
 * 17    7/01/97 2:21p Mgummelt
 * 
 * 16    6/30/97 5:38p Mgummelt
 * 
 * 15    6/23/97 6:57p Mgummelt
 * 
 * 14    6/23/97 4:54p Mgummelt
 * 
 * 13    6/23/97 4:50p Mgummelt
 * 
 * 12    6/18/97 6:58p Mgummelt
 * 
 * 11    6/18/97 4:19p Mgummelt
 * 
 * 10    6/18/97 4:00p Mgummelt
 * 
 * 9     6/10/97 12:09a Mgummelt
 * 
 * 8     6/09/97 10:22p Mgummelt
 * 
 * 7     6/07/97 8:59p Mgummelt
 * 
 * 6     6/06/97 4:08p Mgummelt
 * 
 * 5     6/05/97 9:29a Rlove
 * Weapons now have deselect animations
 * 
 */
