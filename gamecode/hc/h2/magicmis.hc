/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/magicmis.hc,v 1.2 2007-02-07 16:57:07 sezero Exp $
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


void chain_remove ()
{
	if(self.movechain.movechain!=world)
		remove(self.movechain.movechain);
	if(self.movechain!=world)
		remove(self.movechain);
	remove(self);
}

void MagicMissileTouch (void)
{
	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	if(self.movechain.movechain!=world)
		remove(self.movechain.movechain);
	if(self.movechain!=world)
		remove(self.movechain);
	self.level=FALSE;
	if(other.takedamage)
		T_Damage(other,self,self.owner,self.dmg);

	T_RadiusDamage(self,self.owner,self.dmg,other);
	sound(self,CHAN_AUTO,"weapons/explode.wav",1,ATTN_NORM);
	starteffect(CE_MAGIC_MISSILE_EXPLOSION,self.origin-self.movedir*8,0.05);
	remove(self);
}
/*
void TorpedoTrail (void)
{
	particle4(self.origin,7,random(148,159),PARTICLETYPE_GRAV,random(10,20));
}
*/
void StarTwinkle (void)
{
	if(!self.owner.level)
		remove(self);

	if(self.owner.owner.classname!="monster_eidolon")
	if(!self.aflag)
	{
		self.scale+=0.05;
		if(self.scale>=1)
			self.aflag=TRUE;
	}
	else
	{
		self.scale-=0.05;
		if(self.scale<=0.01)
			self.aflag=FALSE;
	}
//	if(random()<0.3)
//		TorpedoTrail();
	self.think=StarTwinkle;
	thinktime self : 0.05;
}

void FireMagicMissile (float offset)
{
entity star1,star2;
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

	newmis.touch=MagicMissileTouch;
	newmis.dmg=random(20,25);

	newmis.speed=1000;
	spread=normalize(v_right)*(offset*25);
	newmis.velocity=normalize(v_forward)*newmis.speed + spread;
	newmis.movedir=normalize(newmis.velocity);
	newmis.avelocity_z=random(300,600);
	newmis.level=TRUE;

	setmodel(newmis,"models/ball.mdl");
	setsize(newmis,'0 0 0','0 0 0');

	if(self.classname=="monster_eidolon")
	{
		newmis.scale=0.75;
		setorigin(newmis,self.origin+self.proj_ofs+v_forward*48+v_right*20);
		sound(self,CHAN_AUTO,"eidolon/spell.wav",1,ATTN_NORM);
	}
	else
	{
		newmis.scale=0.5;
		setorigin(newmis,self.origin+self.proj_ofs+v_forward*8+v_right*7+'0 0 5');
		sound(newmis,CHAN_AUTO,"necro/mmfire.wav",1,ATTN_NORM);
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
			newmis.dmg=random(45,55);
		}
		newmis.effects=EF_DIMLIGHT;
		newmis.frags=TRUE;
//		newmis.dmg=random(30,40);
		newmis.veer=100;
		newmis.homerate=0.1;
//		newmis.turn_time=3;
		newmis.lifetime=time+5;
		newmis.th_die=chain_remove;
		newmis.think=HomeThink;
		newmis.hoverz=TRUE;
		thinktime newmis : 0.2;
	}
	else
	{
		newmis.think=chain_remove;
		thinktime newmis : 3;
	}

	star1=spawn();
	newmis.movechain = star1;
	star1.drawflags(+)MLS_ABSLIGHT;
	star1.abslight=0.5;
	star1.avelocity_z=400;
	star1.avelocity_y=300;
	star1.angles_y=90;
	if(self.classname=="monster_eidolon")
		setmodel(star1,"models/glowball.mdl");
	else
	{
		setmodel(star1,"models/star.mdl");
		star1.scale=0.3;
	}
	setorigin(star1,newmis.origin);
	star2=spawn();
	if(self.classname!="monster_eidolon")
	{
		star1.movechain = star2;
		star2.drawflags(+)MLS_ABSLIGHT;
		star2.abslight=0.5;
		star2.avelocity_z=-400;
		star2.avelocity_y=-300;
		star2.scale=0.3;
		setmodel(star2,"models/star.mdl");
		setorigin(star2,newmis.origin);
	}
	star1.movetype=star2.movetype=MOVETYPE_NOCLIP;
	star1.owner=star2.owner=newmis;
	star1.think=star2.think=StarTwinkle;
	thinktime star1 : 0;
	thinktime star2 : 0;
}

void flash_think ()
{
	makevectors(self.owner.v_angle);
	self.angles_x=self.owner.v_angle_x*-1;
	self.angles_y=self.owner.v_angle_y;
	setorigin(self,self.owner.origin+self.owner.proj_ofs+'0 0 5'+v_right*2+v_forward*6);
	thinktime self : 0.01;
	self.abslight-=0.05;
	self.scale+=0.05;
	if(self.lifetime<time||self.abslight<=0.05)
		remove(self);
}

void FireFlash ()
{
	makevectors(self.v_angle);
	newmis=spawn();
	newmis.movetype=MOVETYPE_NOCLIP;
	newmis.owner=self;
	newmis.abslight=0.5;
	newmis.scale=random(0.8,1.2);
	newmis.drawflags(+)MLS_ABSLIGHT|DRF_TRANSLUCENT;

	setmodel(newmis,"models/handfx.mdl");
	setorigin(newmis,self.origin+self.proj_ofs+'0 0 5'+v_right*2+v_forward*6);

	newmis.angles=self.v_angle;
	newmis.angles_x=self.v_angle_x*-1;
	newmis.angles_z=random(360);
	newmis.avelocity_z=random(360,720);
//	newmis.velocity=random(30)*v_forward;

	newmis.lifetime=time+0.075;
	newmis.think=flash_think;
	thinktime newmis : 0;
}

void  mmis_power()
{
	if(self.attack_finished>time)
		return;

	FireFlash();
	FireMagicMissile(-3);
	FireMagicMissile(0);
	FireMagicMissile(3);
	self.bluemana-=10;
	self.attack_finished=time+0.7;
}

void  mmis_normal()
{
	if(self.attack_finished>time)
		return;

	FireFlash();
	FireMagicMissile(0);
	self.bluemana-=2;
	self.attack_finished=time+0.2;
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
	if(self.wfs==WF_CYCLE_WRAPPED||self.bluemana<2||(self.artifact_active&ART_TOMEOFPOWER&&self.bluemana<10))
		magicmis_ready();
	else if(self.weaponframe==$mfire5)// &&self.attack_finished<=time)
		if(self.artifact_active&ART_TOMEOFPOWER)
			mmis_power();
		else
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

