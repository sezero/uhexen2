/*
==============================================================================

 $Header: /cvsroot/uhexen2/gamecode/hc/portals/lightwp.hc,v 1.1.1.1 2004-11-29 11:33:16 sezero Exp $

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\spllbook
$origin 0 0 0
$base BASE skin
$skin skin
$flags 0

//
$frame fidle1       fidle2       fidle3       fidle4       fidle5       
$frame fidle6       fidle7       fidle8       fidle9       fidle10      
$frame fidle11      fidle12      fidle13      fidle14      fidle15      
$frame fidle16      

//
$frame nidle1       nidle2       nidle3       nidle4       nidle5       
$frame nidle6       nidle7       nidle8       nidle9       nidle10      
$frame nidle11      nidle12      nidle13      nidle14      nidle15      
$frame nidle16      

//
$frame normal1      normal2      normal3      normal4      normal5      
$frame normal6      normal7      normal8      normal9      normal10     
$frame normal11     normal12     normal13     normal14     normal15     
$frame normal16     

//
$frame pidle1       pidle2       pidle3       pidle4       pidle5       
$frame pidle6       pidle7       pidle8       pidle9       pidle10      
$frame pidle11      pidle12      pidle13      pidle14      pidle15      
$frame pidle16      

//
$frame power1       power2       power3       power4       power5       
$frame power6       power7       power8       power9       power10      
$frame power11      power12      power13      power14      power15      
$frame power16      

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     


/*
======================
Lightning Bolts Test

Unpowered: Ball Lightning

Powered: Chain lightning- arcs between monsters
======================
*/

void lball_remove ()
{
	stopSound(self,0);
	remove(self);
}

void LightningBallTouch ()
{
float zap_cnt,zap_other;
vector org,tospot;
	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	self.level=FALSE;
	if(other.takedamage)
	{
		T_Damage(other,self,self.owner,self.dmg);
		if(other.flags&FL_MONSTER)
			zap_other=TRUE;
	}
	T_RadiusDamage(self,self.owner,self.dmg,other);
	stopSound(self,0);
	sound(self,CHAN_AUTO,"crusader/lghtn2.wav",1,ATTN_NORM);
	starteffect(CE_LBALL_EXPL,self.origin-self.movedir*8,0.05);
	if(zap_other)
	{
		org=self.origin;
		tospot=normalize((other.absmin+other.absmax)*0.5-org);
		tospot=org+tospot*(random(75)+75);
		do_lightning (self.owner,zap_cnt,0,4,org,tospot,100,TE_STREAM_LIGHTNING);
	}

	while(zap_cnt<3)//8)
	{
		self.angles=randomv('0 0 0','360 360 360');
		makevectors(self.angles);
		org=self.origin;
		tospot=org+v_forward*(random(75)+75);
		do_lightning (self.owner,zap_cnt,0,4,org,tospot,50,TE_STREAM_LIGHTNING);
		zap_cnt+=1;
	}
	remove(self);
}

void FireLightningBall ()
{
entity lball;//,star1,star2;

	makevectors(self.v_angle);
	self.effects(+)EF_MUZZLEFLASH;
	stuffcmd (self, "bf\n");
	lball=spawn();
	lball.classname="lightning ball";
	lball.owner=self;
	lball.drawflags(+)SCALE_ORIGIN_CENTER;//|DRF_TRANSLUCENT;
	lball.movetype=MOVETYPE_FLYMISSILE;
	lball.solid=SOLID_BBOX;
	lball.level=TRUE;

	lball.touch=LightningBallTouch;
	lball.dmg=random(50,75);

	lball.speed=1000;
	lball.velocity=normalize(v_forward)*lball.speed;
	lball.angles=randomv('-600 -600 -600','600 600 600');
//	lball.avelocity=randomv('-600 -600 -600','600 600 600');

	setmodel(lball,"models/lball.mdl");
	setsize(lball,'0 0 0','0 0 0');

	lball.scale=0.75;
	setorigin(lball,self.origin+self.proj_ofs+v_forward*10);
	sound(self,CHAN_AUTO,"succubus/firelbal.wav",1,ATTN_NORM);

	lball.turn_time=2;
	lball.dmg=random(45,55);
	lball.effects=EF_DIMLIGHT;
	lball.frags=TRUE;
	lball.veer=100;
	lball.homerate=0.1;
	lball.lifetime=time+5;
	lball.th_die=lball_remove;
	lball.think=HomeThink;
	lball.hoverz=TRUE;
	thinktime lball : 0.2;
	lball.t_width=time+random(0.02,0.5);
	sound(lball,CHAN_WEAPON,"succubus/buzz2.wav",1,ATTN_LOOP);
/*
	star1=spawn();
	lball.movechain = star1;
	star1.drawflags(+)MLS_ABSLIGHT;
	star1.abslight=0.5;
	star1.avelocity_z=400;
	star1.avelocity_y=300;
	star1.angles_y=90;
	setmodel(star1,"models/star.mdl");
	star1.scale=0.3;
	setorigin(star1,lball.origin);
	star2=spawn();
	star1.movechain = star2;
	star2.drawflags(+)MLS_ABSLIGHT;
	star2.abslight=0.5;
	star2.avelocity_z=-400;
	star2.avelocity_y=-300;
	star2.scale=0.3;
	setmodel(star2,"models/star.mdl");
	setorigin(star2,lball.origin);
	star1.movetype=star2.movetype=MOVETYPE_NOCLIP;
	star1.owner=star2.owner=lball;
	star1.think=star2.think=StarTwinkle;
	thinktime star1 : 0;
	thinktime star2 : 0;
*/
}

void branch_fire (vector org)
{
vector tospot, lightn_dir;
float num_branches;

	tospot=org+v_forward*1000;
	traceline(org,tospot,TRUE,self);
	tospot=trace_endpos;

	self.t_width=time+1;
	sound(self,CHAN_WEAPON,"succubus/firelght.wav",1,ATTN_NORM);
	num_branches = 3;
//	num_branches = rint(random(3,7));
	self.count=0;
	while(num_branches)
	{
		self.count+=1;
		if(self.count>=8)
			self.count=0;

		do_lightning (self,self.count,STREAM_ATTACHED,4,org,tospot,30,TE_STREAM_LIGHTNING_SMALL);

		lightn_dir=normalize(tospot-org);
		org=org + lightn_dir*random(num_branches+20,num_branches+45);//Include trace_fraction?
		tospot=org+v_forward*1000;
		traceline(trace_endpos,tospot,TRUE,self);
		tospot=trace_endpos;
		if(random()<0.5)
			tospot+=v_right*random(150,400);
		else
			tospot-=v_right*random(150,400);
		if(random()<0.5)
			tospot+=v_up*random(150,400);
		else
			tospot-=v_up*random(150,400);
		
		num_branches-=1;
	}		
}

/*
void glowball_fade ()
{
	self.abslight-=0.1;
	self.scale-=0.01;
	if(self.lifetime<time)
		remove(self);
	else
	{
		thinktime self : 0.05;
		self.think=glowball_fade;
	}
}
*/

/*
void make_glowball (vector org)
{
entity glowball;
	glowball=spawn();
	glowball.solid=SOLID_NOT;
	glowball.movetype = MOVETYPE_NOCLIP;
	setmodel(glowball,"models/glowball.mdl");
	setorigin(glowball,org-v_forward*26+'0 0 3'-v_right*1);
	glowball.owner=self;
	glowball.angles=randomv('-300 -300 -300','300 300 300');
	glowball.avelocity=randomv('-600 -600 -600','600 600 600');
	glowball.scale=0.1;
	glowball.drawflags=MLS_POWERMODE;//ABSLIGHT;
//	glowball.abslight=1;
//	glowball.lifetime=time+0.2;
	thinktime glowball : 0.1;
	glowball.think=SUB_Remove;//glowball_fade;
}
*/

void shebitch_chain_lightning_strike () 
{
vector org, tospot;//,fire_vec;
float damg,damg_thresh, zap_count,fov_check;
entity loser, lastloser,firstloser;

	if(self.attack_finished>time)
		return;
	self.greenmana-=2;
	self.bluemana-=2;
	self.attack_finished=time+0.2;
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	org=self.origin+self.proj_ofs+v_forward*36;

/*	fire_vec=aim(self,org,1000);
	fire_vec=normalize(fire_vec);
	tracearea(org,org+fire_vec*3000,'-3 -3 -3', '3 3 3',FALSE,self);
	if(!trace_ent.takedamage)
	{
		branch_fire(org);
		return;
	}
*/
	loser=findradius(org,1000);
	firstloser=lastloser=loser;//trace_ent;
	while(loser!=world)
	{
		if(loser.health&&loser.flags2&FL_ALIVE&&loser!=self)
		{
			tospot=(loser.absmin+loser.absmax)*0.5;
			traceline(org,tospot,TRUE,self);
			if(infront(loser))
			{
				if(lastloser==firstloser)
				{
					fov_check=vlen(loser.origin-self.origin);
					fov_check=(50/fov_check);
					if(fov_check>1)
						fov_check=1;
					fov_check*=10;//at further distances, cone is smaller
					if(!fov(loser,self,fov_check))
						trace_fraction=0;
				}
			}
			else
				trace_fraction=0;

			if(trace_fraction==1)
			{
				if(loser.flags&FL_MONSTER)
					damg_thresh=40;
				else
					damg_thresh=random(10,20);
				if(loser.health>damg_thresh)
					damg=damg_thresh;
				else
					damg=1000;
				self.count+=1;
				if(self.count>=8)
					self.count=0;
				tospot=(loser.absmin+loser.absmax)*0.5;
				zap_count+=1;
//FIXME:  Make it so these are staggered so only 1 lightning per frame, max
				do_lightning (self,self.count,STREAM_ATTACHED,4,org,tospot,damg,TE_STREAM_LIGHTNING_SMALL);
				org=tospot;
				if(lastloser==self)
					firstloser=loser;
				lastloser=loser;
			}
		}			
		loser=loser.chain;
	}

	if(!zap_count)
		branch_fire(org);
}

/*
void shebitch_lightning_strike (void)
{
vector org;
	if(self.attack_finished>time)
		return;

	self.attack_finished=time+0.2;
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	org=self.origin+self.proj_ofs+v_forward*36;

//	make_glowball(org);
	branch_fire(org);
}
*/

void()lightning_ready_power;
void()lightning_ready_normal;
//void()lightning_to_normal;
//void()lightning_to_power;
void lightning_fire_normal (void)
{
	if(self.weaponframe_cnt)
		self.wfs = advanceweaponframe($fidle1,$fidle16);
	else
	{
		self.wfs = advanceweaponframe($normal1,$normal16);
		if(self.weaponframe==$normal2)
		{//FIXME:  WHY THE FUCK CAN'T I SET DRAWFLAGS?!!!!
			if(self.effects&EF_DIMLIGHT)
				self.lefty=TRUE;
			else
				self.effects(+)EF_DIMLIGHT;
//			self.drawflags(+)MLS_ABSLIGHT;
//			self.abslight=(self.weaponframe - $normal1)/15;
		}
		else if(self.weaponframe==$normal16)
		{
			if(!self.lefty)
				self.effects(-)EF_DIMLIGHT;
			else
				self.lefty=FALSE;
//			self.drawflags(-)MLS_ABSLIGHT;
//			self.abslight=0;
		}
	}
	self.th_weapon=lightning_fire_normal;
	self.last_attack=time;
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		if(self.effects&EF_DIMLIGHT)
		{
			if(!self.lefty)
				self.effects(-)EF_DIMLIGHT;
			else
				self.lefty=FALSE;
		}
		lightning_ready_power();
	}
	else if(self.greenmana<6||
			self.bluemana<6||
			(!self.button0&&self.weaponframe==$normal16)
			)
	{
		if(self.effects&EF_DIMLIGHT)
		{
			if(!self.lefty)
				self.effects(-)EF_DIMLIGHT;
			else
				self.lefty=FALSE;
		}
		lightning_ready_normal();
	}
	else if(self.weaponframe==$normal12 ||(self.weaponframe>=$fidle1 &&self.weaponframe<=$fidle16))
	{
		if(self.attack_finished<time)
		{
			self.weaponframe_cnt=FALSE;
			FireLightningBall();
			sound(self,CHAN_WEAPON,"succubus/firelbal.wav",1,ATTN_NORM);
			self.bluemana-=6;
			self.greenmana-=6;
			self.attack_finished=time+1;
			self.weaponframe=$normal13;
		}
		else
			self.weaponframe_cnt=TRUE;
	}
}

void lightning_fire_power (void)
{
	self.wfs = advanceweaponframe($power1,$power16);
	self.th_weapon=lightning_fire_power;
	self.last_attack=time;
	if(!self.artifact_active&ART_TOMEOFPOWER)
		lightning_ready_normal();
	else if(self.greenmana<2||self.bluemana<2||!self.button0)
		lightning_ready_power();
	else
	{
		if(!self.weaponframe_cnt)
			shebitch_chain_lightning_strike();
		self.weaponframe_cnt+=1;
		if(self.weaponframe_cnt==4)
			self.weaponframe_cnt=0;
	}
}

void() Suc_Litn_Fire =
{
	self.weaponframe_cnt=FALSE;
	if(self.artifact_active&ART_TOMEOFPOWER)
		lightning_fire_power();
	else
		lightning_fire_normal();

	thinktime self : 0;
};


void lightning_ready_power (void)
{
	if(random()<=0.07)
	{
		self.weaponframe=$pidle1 + rint(random(15));
		sound(self,CHAN_WEAPON,"succubus/buzz.wav",1,ATTN_NORM);
		self.effects(+)EF_MUZZLEFLASH;
	}
	else
		self.weaponframe=$pidle1;
	//	self.wfs = advanceweaponframe($pidle1,$pidle16);
	self.th_weapon=lightning_ready_power;
	if(!self.artifact_active&ART_TOMEOFPOWER)
		lightning_ready_normal();
}

void lightning_ready_flip (void)
{
	self.wfs = advanceweaponframe($nidle1,$nidle16);
	self.th_weapon=lightning_ready_flip;
	if(self.wfs==WF_CYCLE_WRAPPED)
		lightning_ready_normal();
}

void lightning_ready_normal (void)
{
	self.weaponframe=$normal1;
	self.th_weapon=lightning_ready_normal;
	if(self.artifact_active&ART_TOMEOFPOWER)
		lightning_ready_power();
	else if(random(1000)<1)
	{
		sound (self, CHAN_WEAPON, "weapons/vorpswng.wav", 1, ATTN_NORM);
		lightning_ready_flip();
	}
}

void lightning_ready (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		lightning_ready_power();
	else
		lightning_ready_normal();
}

void lightning_select (void)
{
	self.wfs = advanceweaponframe($select1,$select12);
	self.weaponmodel = "models/sucwp4.mdl";
	self.th_weapon=lightning_select;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		lightning_ready_flip();
//		lightning_ready();
	}
}

void lightning_deselect (void)
{
	self.wfs = advanceweaponframe($select12,$select1);
	self.th_weapon=lightning_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}


/*
void lightning_deselect (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		lightning_power_deselect();
	else
		lightning_normal_deselect();
}
void lightning_to_power (void)
{
	self.wfs = advanceweaponframe(83,102);
	self.th_weapon=lightning_to_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		lightning_ready_power();
}

void lightning_to_normal (void)
{
	self.wfs = advanceweaponframe(191,210);
	self.th_weapon=lightning_to_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		lightning_ready_normal();
}


void lightning_power_deselect (void)
{
	self.wfs = advanceweaponframe($select12,$select1);
	self.th_weapon=lightning_power_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void lightning_select (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		lightning_power_select();
	else
		lightning_normal_select();
}

void lightning_power_select (void)
{
	self.wfs = advanceweaponframe(103,122);
	self.weaponmodel = "models/sucwp4.mdl";
	self.th_weapon=lightning_power_select;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		lightning_ready_power();
	}
}
void lightning_jellyfingers_normal ()
{
	self.wfs = advanceweaponframe(21,50);
	self.th_weapon=lightning_jellyfingers_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			lightning_ready_power();
		else
			lightning_ready_normal();
}

void lightning_jellyfingers_power ()
{
	self.wfs = advanceweaponframe(123,152);
	self.th_weapon=lightning_jellyfingers_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			lightning_ready_power();
		else
			lightning_ready_normal();
}
*/
