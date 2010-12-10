/*
==============================================================================

 $Header: /cvsroot/uhexen2/gamecode/hc/siege/lightwp.hc,v 1.1 2005-01-26 17:26:11 sezero Exp $

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
	float zap_other;

	if(other.classname==self.classname&&other.owner==self.owner)
		return;

	self.level=FALSE;
	if(other.takedamage)
	{
		T_Damage(other,self,self.owner,self.dmg);
		if(other.flags&FL_MONSTER)
			zap_other=TRUE;
	}
	self.dmg = random(70, 100);
	T_RadiusDamage(self,self.owner,self.dmg,other);
//	stopSound(self,0);
//	sound(self,CHAN_AUTO,"crusader/lghtn2.wav",1,ATTN_NORM);
//	starteffect(CE_LBALL_EXPL,self.origin-self.movedir*8,0.05);
/*	if(zap_other)
	{
		org=self.origin;
		tospot=normalize((other.absmin+other.absmax)*0.5-org);
		tospot=org+tospot*(random(75)+75);
		//do_lightning (self.owner,zap_cnt,0,4,org,tospot,100,TE_STREAM_LIGHTNING);

		damage_dir=normalize(tospot-org);
		LightningDamage (org-damage_dir*15, tospot+damage_dir*15, self.owner, 100,"lightning");
	}

	while(zap_cnt<3)//8)
	{
		self.angles=randomv('0 0 0','360 360 360');
		makevectors(self.angles);
		org=self.origin;
		tospot=org+v_forward*(random(75)+75);
		//do_lightning (self.owner,zap_cnt,0,4,org,tospot,50,TE_STREAM_LIGHTNING);

		damage_dir=normalize(tospot-org);
		LightningDamage (org-damage_dir*15, tospot+damage_dir*15, self.owner, 100,"lightning");

		zap_cnt+=1;
	}*/
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_LIGHTNINGEXPLODE);
	WriteEntity (MSG_MULTICAST, self.owner);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	multicast(self.origin,MULTICAST_PHS_R);

	remove(self);
}

void lightningHomeThink()
{
	vector	moveAng;

	HomeThink();

	moveAng = vectoangles(self.velocity);

	traceline(self.origin, self.origin + self.velocity*.2, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_LIGHTNINGBALL);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, moveAng_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, moveAng_x*256.0/360.0);
	WriteShort(MSG_MULTICAST, vlen(self.velocity));
	WriteByte (MSG_MULTICAST, trace_fraction * 100);
	multicast(self.origin,MULTICAST_PVS);

	thinktime self : 0.2;

	if (self.lifetime < time)
		SUB_Remove();
}

void FireLightningBall ()
{
	entity lball;

	makevectors(self.v_angle);
	self.effects(+)EF_MUZZLEFLASH;
	stuffcmd (self, "bf\n");
	lball=spawn();
	lball.classname="lightning ball";
	lball.owner=self;
	lball.drawflags(+)SCALE_ORIGIN_CENTER;
	lball.movetype=MOVETYPE_FLYMISSILE;
	lball.solid=SOLID_BBOX;
	lball.level=TRUE;

	lball.touch=LightningBallTouch;
	lball.dmg=random(60,100);

	lball.speed=1000;
	lball.velocity=normalize(v_forward)*lball.speed;
	lball.angles=randomv('-600 -600 -600','600 600 600');

	setmodel(lball,"models/lball.mdl");
	setsize(lball,'0 0 0','0 0 0');

	lball.scale=0.75;
	setorigin(lball,self.origin+self.proj_ofs+v_forward*10);
//	sound(self,CHAN_AUTO,"succubus/firelbal.wav",1,ATTN_NORM);

	lball.turn_time=2;
//	lball.dmg=random(45,55);
	lball.effects=EF_DIMLIGHT|EF_UPDATESOUND;
	lball.frags=TRUE;
	lball.veer=100;
	lball.homerate=0.1;
	lball.lifetime=time+2.5;
	lball.th_die=lball_remove;
	//lball.think=HomeThink;
	lball.hoverz=TRUE;
	//thinktime lball : 0.2;
	lball.t_width=time+random(0.02,0.5);
//	sound(lball,CHAN_UPDATE+PHS_OVERRIDE_R,"succubus/buzz2.wav",1,ATTN_LOOP);

	lball.effects(+)EF_NODRAW;

	entity oldself;
	oldself = self;
	self = lball;

	lball.think = lightningHomeThink;
	lball.think();

	self = oldself;
}





void LilLightningThink()
{
	T_Damage(self.enemy, self.owner, self.owner, self.dmg);

	remove(self);
}

void AttachLilLightning(float damage, entity nTarg, entity from)
{
	entity newGuy;

	newGuy = spawn();
	newGuy.effects (+) EF_NODRAW;

	newGuy.dmg = damage;
	newGuy.owner = from;
	newGuy.enemy = nTarg;

	newGuy.think = LilLightningThink;
	thinktime newGuy : 0.0;
}

void(vector p1, vector p2, entity from, float damage) LightningDamage2 =
{
	entity	e1, e2;// swap;
	vector	f;
	float	inertia;//absorb;

	f = p2 - p1;
	normalize (f);
	f_x = 0 - f_y;
	f_y = f_x;
	f_z = 0;
	f = f*16;

	e1 = e2 = world;

	traceline (p1, p2, FALSE, self);

	if (trace_ent.takedamage)
	{
		if (trace_ent.mass<=10)
		{
			inertia=1;
		}
		else 
		{
			inertia = trace_ent.mass/10;
		}
		WriteCoord(MSG_MULTICAST, trace_ent.origin_x);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_y);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_z + 32);
		AttachLilLightning(damage, trace_ent, from);
	}

    e1 = trace_ent;
	traceline (p1 + f, p2 + f, FALSE, self);
	if(trace_ent != e1 && trace_ent.takedamage)
	{
		WriteCoord(MSG_MULTICAST, trace_ent.origin_x);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_y);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_z + 32);
		AttachLilLightning(damage, trace_ent, from);
	}

	e2 = trace_ent;
	traceline (p1 - f, p2 - f, FALSE, self);
    if (trace_ent != e1 && trace_ent != e2 && trace_ent.takedamage)
	{
		WriteCoord(MSG_MULTICAST, trace_ent.origin_x);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_y);
		WriteCoord(MSG_MULTICAST, trace_ent.origin_z + 32);
		AttachLilLightning(damage, trace_ent, from);
	}
};

void do_lightning2 (entity lowner,float tag, float lflags, float duration, vector spot1, vector spot2, float ldamg)
{
	vector damage_dir;

//	lowner=self.owner;

	damage_dir=normalize(spot2-spot1);
	LightningDamage2 (spot1-damage_dir*15, spot2+damage_dir*15, lowner, ldamg);
}

void branch_fire (vector org)
{
	vector tospot, lightn_dir;
	float num_branches;

	tospot=org+v_forward*1000;
	traceline(org,tospot,TRUE,self);
	tospot=trace_endpos;

	num_branches = 3;
	self.count=0;
	while(num_branches)
	{
		self.count+=1;
		if(self.count>=8)
			self.count=0;

		do_lightning2 (self,self.count,STREAM_ATTACHED,4,org,tospot,30);

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

void shebitch_chain_lightning_strike () 
{
	vector	org, tospot;
	float	damg,damg_thresh, zap_count,fov_check;
	entity	loser, lastloser,firstloser;
	float	numTargs;

	numTargs = 0;

	if(self.attack_finished>time)
		return;
	self.greenmana-=4;
	self.bluemana-=4;
	self.attack_finished=time+0.2;
	self.effects(+)EF_MUZZLEFLASH;
	makevectors(self.v_angle);
	org=self.origin+self.proj_ofs+v_forward*36;

	loser=findradius(org,1000);
	firstloser=lastloser=loser;

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_CHAINLIGHTNING);
	WriteEntity(MSG_MULTICAST, self);

	WriteCoord(MSG_MULTICAST, org_x);
	WriteCoord(MSG_MULTICAST, org_y);
	WriteCoord(MSG_MULTICAST, org_z);

	while((loser!=world)&&(numTargs < 5))
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
					damg_thresh=random(15,25);
				if(loser.health>damg_thresh)
					damg=damg_thresh;
				else
					damg=1000;
				self.count+=1;
				if(self.count>=8)
					self.count=0;
				tospot=(loser.absmin+loser.absmax)*0.5;
				zap_count+=1;

				do_lightning2 (self,self.count,STREAM_ATTACHED,4,org,tospot,damg);
				numTargs += 1;	// ensure that there are fewer than 5 targets hit...
				org=tospot;
				if(lastloser==self)
					firstloser=loser;
				lastloser=loser;
			}
		}			
		loser=loser.chain;
	}

	if(!zap_count)
	{
		branch_fire(org);

		traceline(org, org + v_forward * 128, TRUE, self);

		WriteCoord(MSG_MULTICAST, org_x + v_forward_x * 128 * trace_fraction);
		WriteCoord(MSG_MULTICAST, org_y + v_forward_y * 128 * trace_fraction);
		WriteCoord(MSG_MULTICAST, org_z + v_forward_z * 128 * trace_fraction);

	}

	WriteCoord(MSG_MULTICAST, 0);
	WriteCoord(MSG_MULTICAST, 0);
	WriteCoord(MSG_MULTICAST, 0);
	multicast(self.origin,MULTICAST_PVS);
}


void()lightning_ready_power;
void()lightning_ready_normal;
void lightning_fire_normal (void)
{
	if(self.weaponframe_cnt)
		self.wfs = advanceweaponframe($fidle1,$fidle16);
	else
	{
		self.wfs = advanceweaponframe($normal1,$normal16);
		if(self.weaponframe==$normal2)
		{
			if(self.effects&EF_DIMLIGHT)
				self.lefty=TRUE;
			else
				self.effects(+)EF_DIMLIGHT;
		}
		else if(self.weaponframe==$normal16)
		{
			if(!self.lefty)
				self.effects(-)EF_DIMLIGHT;
			else
				self.lefty=FALSE;
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
	{
		stopSound(self,CHAN_UPDATE);
		self.effects(-)EF_UPDATESOUND;
		self.t_width=SOUND_STOPPED;
		lightning_ready_normal();
	}
	else if(self.greenmana<4||self.bluemana<4||!self.button0)
	{
		stopSound(self,CHAN_UPDATE);
		self.effects(-)EF_UPDATESOUND;
		self.t_width=SOUND_STOPPED;
		lightning_ready_power();
	}
	else
	{
		if(self.t_width!=SOUND_STARTED)
		{
			sound(self,CHAN_UPDATE+PHS_OVERRIDE_R,"succubus/firelght.wav",1,ATTN_LOOP);
			self.effects(+)EF_UPDATESOUND;
			self.t_width=SOUND_STARTED;
		}
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
	}
}

void lightning_deselect (void)
{
	self.wfs = advanceweaponframe($select12,$select1);
	self.th_weapon=lightning_deselect;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}


