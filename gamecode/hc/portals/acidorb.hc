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
$frame idlebn01     idlebn02     idlebn03     idlebn04     idlebn05     
$frame idlebn06     idlebn07     idlebn08     idlebn09     idlebn10     
$frame idlebn11     idlebn12     idlebn13     idlebn14     idlebn15     
$frame idlebn16     idlebn17     idlebn18     idlebn19     idlebn20     
$frame idlebn21     idlebn22     idlebn23     idlebn24     idlebn25     

//
$frame idlean26     idlean27     idlean28     idlean29     idlean30     
$frame idlean31     idlean32     idlean33     idlean34     idlean35     
$frame idlean36     idlean37     idlean38     idlean39     idlean40     
$frame idlean41     idlean42     idlean43     idlean44     idlean45     
$frame idlean46     idlean47     idlean48     idlean49     idlean50     

//
$frame normal51     normal52     normal53     normal54     normal55     
$frame normal56     normal57     normal58     normal59     normal60     
$frame normal61     normal62     normal63     normal64     

//
$frame topowr65     topowr66     topowr67     topowr68     topowr69     
$frame topowr70     topowr71     topowr72     topowr73     topowr74     
$frame topowr75     topowr76     topowr77     topowr78     topowr79     
$frame topowr80     topowr81     topowr82     topowr83     topowr84     

//
$frame pidleb085    pidleb086    pidleb087    pidleb088    pidleb089    
$frame pidleb090    pidleb091    pidleb092    pidleb093    pidleb094    
$frame pidleb095    pidleb096    pidleb097    pidleb098    pidleb099    
$frame pidleb100    pidleb101    pidleb102    pidleb103    pidleb104    
$frame pidleb105    pidleb106    pidleb107    pidleb108    pidleb109    

//
$frame pidlea110    pidlea111    pidlea112    pidlea113    pidlea114    
$frame pidlea115    pidlea116    pidlea117    pidlea118    pidlea119    
$frame pidlea120    pidlea121    pidlea122    pidlea123    pidlea124    
$frame pidlea125    pidlea126    pidlea127    pidlea128    pidlea129    
$frame pidlea130    pidlea131    pidlea132    pidlea133    pidlea134    

//
$frame powern135    powern136    powern137    powern138    powern139    
$frame powern140    powern141    powern142    powern143    powern144    
$frame powern145    powern146    powern147    powern148    

//
$frame tonrml149    tonrml150    tonrml151    tonrml152    tonrml153    
$frame tonrml154    tonrml155    tonrml156    tonrml157    tonrml158    
$frame tonrml159    tonrml160    tonrml161    tonrml162    tonrml163    
$frame tonrml164    tonrml165    tonrml166    tonrml167    tonrml168    
//
$frame ndesel213    ndesel214    ndesel215    ndesel216    ndesel217    
$frame ndesel218    ndesel219    ndesel220    ndesel221    ndesel222    
$frame ndesel223    

//
$frame pselon224    pselon225    pselon226    pselon227    pselon228    
$frame pselon229    pselon230    pselon231    pselon232    pselon233    
$frame pselon234    



void acidblobFizzle (void)
{
	CreateGreenSmoke (self.origin,'0 0 8',HX_FRAME_TIME * 2);
	remove(self);
}

void acidblobThink (void)
{
vector adjust_angles;
	if(self.lifetime<time)
		if(self.dmg==15)
			acidblobFizzle();
		else
			MultiExplode();

	if(self.dmg>15)
	{
		starteffect(CE_ACID_MUZZFL, self.origin,'0 0 .1',HX_FRAME_TIME);
//		CreateGreenSmoke (self.origin,'0 0 8',HX_FRAME_TIME * 2);
		thinktime self : 0.1;
	}
	else
		thinktime self : 0.3;

	adjust_angles=vectoangles(self.velocity);
	self.angles_x=adjust_angles_x;
	self.angles_y=adjust_angles_y;
	self.think=acidblobThink;
}

void acidblobTouch (void)
{
	if(other.controller==self.owner)
		return;

	if(self.dmg==15)
	{
		starteffect(CE_ACID_SPLAT, self.origin-self.movedir*6,'0 0 0', HX_FRAME_TIME);
		if(other==world)
		{
//			if(!self.pain_finished&&random()<0.3)
//			{
			//SOUND: wet splats
			//	sound(self.controller,CHAN_BODY,"misc/rubble.wav",1,ATTN_NORM);
//				self.pain_finished=TRUE;
//			}
			sound(self,CHAN_AUTO,"succubus/dropfizz.wav",1,ATTN_NORM);
			acidblobFizzle();
//			return;
		}
		else if(other.classname=="acidblob")
			return;
	}

	if(other.takedamage&&other.health)
	{
		T_Damage(other,self,self.owner,self.dmg);
		if(self.dmg>15)
		{
			if((other.flags&FL_CLIENT||other.flags&FL_MONSTER)&&other.mass<200)
			{
			vector hitdir;
				hitdir=self.o_angle*300;
				hitdir_z+=150;
				if(hitdir_z<0)
					hitdir_z=0;
				other.velocity=hitdir;
				other.flags(-)FL_ONGROUND;
			}
			self.dmg/=2;
		}
		else
			sound(self,CHAN_AUTO,"succubus/acidhit.wav",1,ATTN_NORM);
	}
	else if(self.dmg>15)
		self.dmg=100;

	if(self.dmg>15)
		MultiExplode();
	else
		acidblobFizzle();
}

void FireAcidBlob (string type)
{
vector org;
entity acidblob;
	acidblob=spawn();
	setmodel(acidblob,"models/sucwp2p.mdl");
	acidblob.thingtype=THINGTYPE_ACID;
	if(type=="aciddrop")
	{
		acidblob.classname=type;
		acidblob.velocity=RandomVector('150 150 0');
		acidblob.velocity_z=random(300,500);
		acidblob.lifetime=time + 1.5;
		acidblob.dmg=15;
//		acidblob.scale=random(0.5,1);
		acidblob.movetype=MOVETYPE_BOUNCE;
		org=self.origin;
		setsize(acidblob,'0 0 0', '0 0 0');
	}
	else
	{
		acidblob.th_die=MultiExplode;
		self.bluemana-=8;
		acidblob.classname="acidblob";
		self.punchangle_x = -4;
		//SOUND: Acid fire
		sound(self,CHAN_AUTO,"succubus/acidpfir.wav",1,ATTN_NORM);
		self.attack_finished=time + 0.7;
		self.effects(+)EF_MUZZLEFLASH;
		makevectors(self.v_angle);
		acidblob.speed=1000;
		acidblob.o_angle=normalize(v_forward);		
		acidblob.velocity=acidblob.o_angle*acidblob.speed;
		acidblob.veer=30;
		acidblob.lifetime=time + 5;
		acidblob.dmg=75;
		acidblob.scale=2.5;
		acidblob.movetype=MOVETYPE_FLYMISSILE;
		org=self.origin+self.proj_ofs+v_forward*8-v_right*12 - '0 0 3';
		setsize(acidblob,'0 0 0', '0 0 0');
	}
//	acidblob.abslight = 0.5;
//	acidblob.drawflags(+)MLS_FIREFLICKER;//|MLS_ABSLIGHT;

	acidblob.angles=vectoangles(acidblob.velocity);
	if(random()<0.5)
		acidblob.avelocity_z=random()*-360 - 100;
	else
		acidblob.avelocity_z=random()*360+100;

	if(self.classname=="acidblob")
		acidblob.owner=self.owner;
	else
		acidblob.owner=self;
//	acidblob.controller=self;

	acidblob.movedir=normalize(acidblob.velocity);
	acidblob.solid=SOLID_BBOX;
	acidblob.touch=acidblobTouch;

	acidblob.think=acidblobThink;
	thinktime acidblob : 0.1;

	setorigin(acidblob,org);
}

void AcidMissileTouch (void)
{
	if(pointcontents(self.origin)==CONTENT_SKY)
	{
		remove(self);
		return;
	}

	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	starteffect(CE_ACID_HIT, self.origin-self.movedir*6,'0 0 0', HX_FRAME_TIME);
//	CreateGreenSmoke (self.origin,'0 0 8',HX_FRAME_TIME * 2);
	if(!other.takedamage)
	{
		sound(self,CHAN_AUTO,"succubus/acidhit.wav",1,ATTN_NORM);
//		sound (self, CHAN_WEAPON, "succubus/brnbounce.wav", 1, ATTN_NORM);
		remove(self);
		return;
	}

	T_Damage(other,self,self.owner,self.dmg);

	sound(self,CHAN_AUTO,"succubus/acidhit.wav",1,ATTN_NORM);
	remove(self);
}

void FireAcidMissile (float offset)
{//fixme: add trail
vector spread;

	makevectors(self.v_angle);
//	starteffect(CE_ACID_MUZZFL, self.origin+self.proj_ofs+v_forward*10-v_right*8,normalize(self.velocity)+v_forward*8,HX_FRAME_TIME);

	self.effects(+)EF_MUZZLEFLASH;
	self.bluemana-=3;
	newmis=spawn();
//	newmis.thingtype=THINGTYPE_ACID;
//	newmis.classname="acid missile";
	newmis.owner=self;
//	newmis.drawflags(+)SCALE_ORIGIN_CENTER|MLS_FIREFLICKER;
//	newmis.scale=1.3;
//	newmis.abslight=1;
	newmis.frags=FALSE;
	newmis.movetype=MOVETYPE_FLYMISSILE;

	newmis.solid=SOLID_BBOX;
	newmis.touch=AcidMissileTouch;

	newmis.dmg=random(27,33);

//	newmis.speed=800;//Hardcoded so less info is sent over network
	spread=normalize(v_right)*(offset*25);
	spread=normalize(v_up)*(offset*8);
	newmis.velocity=normalize(v_forward)*850 + spread;
	newmis.avelocity=randomv('0 0 -400','0 0 400');
	newmis.movedir=normalize(newmis.velocity);
	newmis.angles=vectoangles(newmis.velocity);

	setmodel(newmis,"models/sucwp2p.mdl");
	setsize(newmis,'0 0 0','0 0 0');

	setorigin(newmis,self.origin+self.proj_ofs+v_forward*8-v_right*12 + v_right*(offset*3) + '0 0 2');
	sound(newmis,CHAN_AUTO,"succubus/acidfire.wav",1,ATTN_NORM);

	newmis.think=SUB_Remove;//AcidMissileFade;
	thinktime newmis : 2;
}

void  aorb_power()
{
	if(self.attack_finished>time)
		return;

	FireAcidBlob("acidblob");
	self.attack_finished=time+0.7;
}

void  aorb_normal()
{
	if(self.attack_finished>time)
		return;

	FireAcidMissile(0);
	self.attack_finished=time+0.4;
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

void()acidorb_ready_power;
void()acidorb_ready_normal;
void() Suc_Aorb_Fire;

void acidorb_fire (void)
{
	if(self.button0&&self.weaponframe==$normal60 &&!self.artifact_active&ART_TOMEOFPOWER)
		self.weaponframe=$normal58;
	else
		if(self.artifact_active&ART_TOMEOFPOWER)
		{
			self.wfs = advanceweaponframe($powern135,$powern148);
			if(self.weaponframe<$powern141)
				self.weaponframe+=1;
		}
		else
		{
			self.wfs = advanceweaponframe($normal51,$normal64);
			if(self.weaponframe<$normal58)
				self.weaponframe+=1;
		}
	self.th_weapon=acidorb_fire;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED||self.bluemana<3||(self.bluemana<8&&self.artifact_active&ART_TOMEOFPOWER))
		if(self.artifact_active&ART_TOMEOFPOWER)
			acidorb_ready_power();
		else
			acidorb_ready_normal();
	else if(self.weaponframe==$normal58)// &&self.attack_finished<=time)
		aorb_normal();
	else if(self.weaponframe==$powern141)
		aorb_power();//Fixme: hold this frame for a few
}

void Suc_Aorb_Fire() 
{	
	acidorb_fire();

	thinktime self : 0;
}

void acidorb_jellyfingers_normal ()
{
	self.wfs = advanceweaponframe($idlebn01,$idlebn25);
	self.th_weapon=acidorb_jellyfingers_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			acidorb_ready_power();
		else
			acidorb_ready_normal();
}

void acidorb_jellyfingers_power ()
{
	self.wfs = advanceweaponframe($pidleb085,$pidleb109);
	self.th_weapon=acidorb_jellyfingers_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			acidorb_ready_power();
		else
			acidorb_ready_normal();
}

void acidorb_to_power (void)
{
	self.wfs = advanceweaponframe($topowr65,$topowr84);
	self.th_weapon=acidorb_to_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		acidorb_ready_power();
}

void acidorb_to_normal (void)
{
	self.wfs = advanceweaponframe($tonrml149,$tonrml168);
	self.th_weapon=acidorb_to_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		acidorb_ready_normal();
}

void acidorb_ready_normal (void)
{
	self.wfs = advanceweaponframe($idlean26,$idlean50);
	if(random()<0.1&&self.weaponframe==$idlean50)
		self.th_weapon=acidorb_jellyfingers_normal;
	else
		self.th_weapon=acidorb_ready_normal;
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		self.weaponframe=$topowr65;
		acidorb_to_power();
	}
}

void acidorb_ready_power (void)
{
	self.wfs = advanceweaponframe($pidlea110,$pidlea134);
	if(random()<0.1&&self.weaponframe==$pidlea134)
		self.th_weapon=acidorb_jellyfingers_power;
	else
		self.th_weapon=acidorb_ready_power;
	if(!self.artifact_active&ART_TOMEOFPOWER)
	{
		self.weaponframe=$tonrml149;
		acidorb_to_normal();
	}
}

void acidorb_select_normal (void)
{
	self.wfs = advanceweaponframe($ndesel223,$ndesel213);
	self.weaponmodel = "models/sucwp2.mdl";
	self.th_weapon=acidorb_select_normal;
	self.t_width=-1;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		acidorb_ready_normal();
	}
}

void acidorb_select_power (void)
{
	self.wfs = advanceweaponframe($pselon224,$pselon234);
	self.weaponmodel = "models/sucwp2.mdl";
	self.th_weapon=acidorb_select_power;
	self.t_width=-1;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		acidorb_ready_power();
	}
}

void acidorb_select (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		acidorb_select_power();
	else
		acidorb_select_normal();
}

void acidorb_deselect_normal (void)
{
	self.wfs = advanceweaponframe($ndesel213,$ndesel223);
	self.th_weapon=acidorb_deselect_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void acidorb_deselect_power (void)
{
	self.wfs = advanceweaponframe($pselon234,$pselon224);
	self.th_weapon=acidorb_deselect_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void acidorb_deselect (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		acidorb_deselect_power();
	else
		acidorb_deselect_normal();
}

