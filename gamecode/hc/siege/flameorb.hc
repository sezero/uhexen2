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

/*
void burner_think ()
{
	if(self.lifetime<time||self.enemy.health<0)
	{
		stopSound(self.enemy,CHAN_BODY);
		//sound(self.enemy,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		self.enemy.effects (-) EF_ONFIRE;
		remove(self);
		return;
	}
	else if(pointcontents(self.enemy.origin)==CONTENT_WATER)
	{
		sound (self.enemy, CHAN_BODY, "misc/fout.wav", 1, ATTN_NORM);
		smolder((self.enemy.absmin+self.enemy.absmax)*0.5);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		self.enemy.effects (-) EF_ONFIRE;
		remove(self);
		return;
	}
	else
	{
		//org=(self.enemy.absmin+self.enemy.absmax)*0.5+randomv(self.enemy.size*-0.25,self.enemy.size*0.25);
		//vel=randomv('-3 -3 0','3 3 7'); 

		//starteffect(CE_ONFIRE, org,vel, 0);
		//sound(self.enemy,CHAN_BODY,"raven/fire1.wav",1,ATTN_NORM);

		thinktime self : random(0.5);
		T_Damage(self.enemy,self,self.owner,self.enemy.fire_damage + random(1));
	}
}

void spawn_burner (entity loser)
{
	if (loser.flags2 & FL2_ONFIRE)
	{
		loser.fire_damage += 2;
		return;
	}

	if (coop && loser.classname == "player" && loser.team == self.owner.team && teamplay)
		return;

	loser.fire_damage = 2;

	entity burner;
	burner=spawn();
	burner.owner=self.owner;
	burner.enemy=loser;
	burner.lifetime=time+random(5)+5;
	burner.think=burner_think;
	burner.effects (+) EF_NODRAW;
	burner.enemy.effects (+) EF_DIMLIGHT;
	thinktime burner : 0;
	burner.enemy.flags2 (+) FL2_ONFIRE;
	burner.enemy.effects (+) EF_ONFIRE;
	//sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NORM);
	starteffect(CE_LG_EXPLOSION , self.origin);
}
*/














float isFlammable()
{
	if (other.thingtype == THINGTYPE_FLESH || 
		other.thingtype == THINGTYPE_WOOD || 
		other.thingtype == THINGTYPE_HAY ||
		other.thingtype == THINGTYPE_LEAVES ||
		other.thingtype == THINGTYPE_CLOTH ||
		other.thingtype == THINGTYPE_WOOD_LEAF ||
		other.thingtype == THINGTYPE_WOOD_METAL ||
		other.thingtype == THINGTYPE_WOOD_STONE ||
		other.thingtype == THINGTYPE_METAL_CLOTH)
	{
		return 1;
	}

	return 0;
}

void flamestream_touch ()
{
	self.velocity = '0 0 0';

	if(other.classname=="flamestream")
		return;

	if(other.takedamage)
	{
		float old_health;
		vector other_org;
	
		if (isFlammable())
		{
			spawn_burner(other,FALSE);
		}

		other_org=other.origin;
		old_health=other.health;
		T_Damage(other,self,self.owner,self.dmg + random(1,5));
		if(other.health<=0&&old_health>0)
			smolder(other_org);
		
	}
	else
	{
		T_RadiusDamage(self,self.owner,40,self.owner);	
	}

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_FIREWALL_IMPACT);
	WriteCoord (MSG_MULTICAST, self.origin_x - self.movedir_x * 24);
	WriteCoord (MSG_MULTICAST, self.origin_y - self.movedir_y * 24);
	WriteCoord (MSG_MULTICAST, self.origin_z - self.movedir_z * 24);
	multicast(self.origin,MULTICAST_PHS_R);

	remove(self);
}

void flamestreamThink()
{
	makevectors(self.angles);	
			
	float old_health;
	vector other_org;

	traceline(self.origin, self.origin + '0 0 -256', FALSE, self);

	if (trace_ent)
	{
		if (isFlammable())
		{
			spawn_burner(trace_ent,FALSE);
		}

		other_org=trace_ent.origin;
		old_health=trace_ent.health;
		if(trace_ent.health<=0&&old_health>0)
		{
			smolder(other_org);		
		}
	}

	self.angles = vectoangles(self.movedir);

	traceline(self.origin, self.origin + self.movedir * 450, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_FIREWALL);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 8.0);
	multicast(self.origin,MULTICAST_PVS);

	if (self.lifetime < time) self.touch();

	self.think = flamestreamThink;
	thinktime self : 0.3;
}

// regular attack
void flamestream_fire ()
{
	
	self.effects (+) EF_MUZZLEFLASH;
	self.greenmana-=4;

	self.punchangle_x = -2;
	makevectors(self.v_angle);
	self.velocity+=normalize(v_forward) * -100;
	self.flags(-)FL_ONGROUND;

	newmis=spawn();
	newmis.classname="flamestream";
	newmis.owner=self;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.touch=flamestream_touch;
	newmis.dmg=40;
	newmis.lifetime=time+2;
	newmis.o_angle=self.origin+self.proj_ofs+v_forward*16-v_right*16-v_up*16;
	newmis.movedir=v_forward;

	newmis.speed=1500+random(50);
	newmis.velocity=newmis.movedir*newmis.speed;
	setmodel(newmis,"models/null.spr");
	setsize(newmis,'-6 -6 -6','6 6 6');
	newmis.level=0;
	newmis.hull=HULL_POINT;
	setorigin(newmis,newmis.o_angle);
	newmis.wallspot=newmis.origin;
	newmis.count = 20;
	newmis.cnt = 0;
	newmis.effects (+) EF_NODRAW;

	newmis.think=flamestreamThink;	
	thinktime newmis : 0.3;

	newmis.angles = vectoangles(newmis.movedir);

	traceline(newmis.origin, newmis.origin + newmis.movedir * 450, FALSE, newmis);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_FIREWALL);
	WriteCoord (MSG_MULTICAST, newmis.origin_x);
	WriteCoord (MSG_MULTICAST, newmis.origin_y);
	WriteCoord (MSG_MULTICAST, newmis.origin_z);
	WriteByte (MSG_MULTICAST, newmis.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, newmis.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 8.0);
	multicast(self.origin,MULTICAST_PVS);
}















void flameball_touch()
{
	if (other.takedamage)
	{
		float old_health;
		vector other_org;
	
//		sound(self,CHAN_BODY,"succubus/flamend.wav",0.5,ATTN_NORM);
		other_org=other.origin;
		old_health=other.health;
		T_Damage(other,self,self.owner,random(10,20));
		if(other.health<=0&&old_health>0)
			smolder(other_org);		
	}
/*	else//no need for this why damage it if it can't take damage?
	{
		T_Damage(other, self.owner, self.owner, 10);
	}*/

 // these'll be made on the client

/*    rand = random();

	if (rand < 0.2)
		starteffect(CE_SM_EXPLOSION, self.origin-self.movedir*6, '0 0 6', 0);
	else if (rand < 0.3)
		starteffect(CE_FBOOM, self.origin-self.movedir*6, '0 0 6', 0);
	else
		starteffect(CE_BOMB, self.origin-self.movedir*6, '0 0 6', 0);*/
			
	remove(self);
}

void flameball_think()
{
//	starteffect(CE_FLAMESTREAM, self.origin, '0 0 2', 0);

	self.think = SUB_Remove;
	thinktime self : 2;
}

void flameball_spawn(vector pos, entity targ)
{
	newmis = spawn();
	newmis.classname = "flameball";
	newmis.owner = self.owner;

	setmodel(newmis, "models/sucwp1p.mdl");
	setsize(newmis, '-3 -3 -3', '3 3 3');
	newmis.hull = HULL_POINT;
	newmis.solid = SOLID_BBOX;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.drawflags (+) MLS_ABSLIGHT;
	newmis.abslight = 0.5;
	newmis.frame = 4;
	newmis.effects (+) EF_NODRAW; // these'll be made on the client

	setorigin(newmis, pos);

	pos = targ.origin - newmis.origin;

	pos = normalize(pos);

	pos *= 800 + random(100);

	newmis.velocity = pos;
	newmis.angles = vectoangles(newmis.velocity);
	newmis.movedir = normalize(newmis.velocity);

//	starteffect(CE_FLAMESTREAM, newmis.origin, '0 0 0', 0);

	newmis.touch = flameball_touch;
	newmis.think = flameball_think;

	thinktime newmis : 0.05;
}

void flameswarmBoom()
{
	local vector rand;

	if (self.lifetime < time || self.lockentity.origin == '0 0 0')
	{
		self.lockentity.effects (-) EF_POWERFLAMEBURN;
		remove(self);
		return;
	}
	
	if (self.attack_finished < time)
	{
		sound(self,CHAN_BODY,"succubus/flamend.wav",0.5,ATTN_NORM);
		self.attack_finished = time + 1.5;
	}
	
	makevectors(self.lockentity.angles);

	rand = self.lockentity.origin + '0 0 200' + randomv('-140 -140 -140', '140 140 140');
	
	traceline(self.lockentity.origin, rand, TRUE, self);
	
	flameball_spawn(trace_endpos, self.lockentity);

	self.think = flameswarmBoom;
	thinktime self : 0.1;
}

void flameswarm_touch ()
{
	entity found,loser;
	float lastdist,dist;
	float old_health;
	vector loser_org;

	self.velocity = '0 0 0';

	if(other.classname=="flamestream")
		return;
	
	loser=other;
	if(!loser.takedamage)
	{
		found=findradius(self.origin,200);
		lastdist=200;
		dist=0;
		while(found)
		{
			dist=vlen((loser.absmin+loser.absmax)*0.5-self.origin);
			if(dist<lastdist)
			{
				lastdist=dist;
				loser=found;
			}
			found=found.chain;
		}
		sound(self,CHAN_BODY,"succubus/flampow.wav",0.5,ATTN_NORM);

		starteffect(CE_FBOOM, self.origin-self.movedir*6,'0 0 0', 0);
		T_RadiusDamage(self,self.owner,50+random(1,10),self.owner);	
		remove(self);
	}
	else
	{
		loser_org=loser.origin;
		old_health=loser.health;
		//starteffect(CE_FBOOM, self.origin-self.movedir*6,'0 0 0', 0);
		T_Damage(other, self.owner, self.owner, 20);
		if(loser.health<=0&&old_health>0)
			smolder(loser_org);

		sound(self,CHAN_BODY,"succubus/flampow.wav",0.5,ATTN_NORM);

		self.lockentity = loser;
		self.lifetime = time + 3;
		
		self.solid = SOLID_NOT;
		self.effects (+) EF_NODRAW;
		loser.effects (+) EF_POWERFLAMEBURN;
		self.think = flameswarmBoom;
		thinktime self : 0.1;
		return;
	}
}






void flameswarmThink()
{
	self.angles = vectoangles(self.movedir);
	makevectors(self.angles);

	traceline(self.origin, self.origin + self.movedir * 450, FALSE, self);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_POWERFLAME);
	WriteCoord (MSG_MULTICAST, self.origin_x);
	WriteCoord (MSG_MULTICAST, self.origin_y);
	WriteCoord (MSG_MULTICAST, self.origin_z);
	WriteByte (MSG_MULTICAST, self.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, self.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 8.0);
	WriteLong (MSG_MULTICAST, time);
	multicast(self.origin,MULTICAST_PVS);

	if (self.lifetime < time) self.touch();

	self.think = flameswarmThink;
	thinktime self : 0.3;
}

// tome of power attack
void flameswarm_fire()
{
	makevectors(self.v_angle);
	self.greenmana-=8;
	self.effects(+)EF_MUZZLEFLASH;
	
	self.velocity+=normalize(v_forward) * -200;
	self.punchangle_x = -6;
	
	self.flags(-)FL_ONGROUND;

	newmis=spawn();
	newmis.classname="flamestream";
	newmis.owner=self;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.abslight=1;
	newmis.touch=flameswarm_touch;
	newmis.dmg=40;
	newmis.lifetime=time+2;

	newmis.o_angle=self.origin+self.proj_ofs+v_forward*16-v_right*24-v_up*16;	
	newmis.movedir=v_forward;

	newmis.speed=1250+random(50);
	newmis.velocity=newmis.movedir*newmis.speed;
	setmodel(newmis,"models/null.spr");
	setsize(newmis,'-6 -6 -6','6 6 6');
	newmis.level=0;
	newmis.hull=HULL_POINT;
	setorigin(newmis,newmis.o_angle);
	newmis.wallspot=newmis.origin;
	newmis.angles = vectoangles(newmis.velocity);
	newmis.effects (+) EF_NODRAW;
		
	newmis.think=flameswarmThink;	
	thinktime newmis : 0.3;

	newmis.angles = vectoangles(newmis.movedir);



	traceline(newmis.origin, newmis.origin + newmis.movedir * vlen(newmis.velocity)*.3, FALSE, newmis);

	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, TE_POWERFLAME);
	WriteCoord (MSG_MULTICAST, newmis.origin_x);
	WriteCoord (MSG_MULTICAST, newmis.origin_y);
	WriteCoord (MSG_MULTICAST, newmis.origin_z);
	WriteByte (MSG_MULTICAST, newmis.angles_y*256.0/360.0);
	WriteByte (MSG_MULTICAST, newmis.angles_x*256.0/360.0);
	WriteByte (MSG_MULTICAST, trace_fraction * 8.0);
	WriteLong (MSG_MULTICAST, time);
	multicast(self.origin,MULTICAST_PVS);
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


void()flameorb_ready_power;
void()flameorb_ready_normal;
void flameorb_fire (void)
{

	if(self.artifact_active&ART_TOMEOFPOWER)
		self.wfs = advanceweaponframe($powern135,$powern148);
	else
		self.wfs = advanceweaponframe($normal51,$normal64);
	/*if(self.button0&&self.weaponframe>$normal60 &&!self.artifact_active&ART_TOMEOFPOWER)
		self.weaponframe=$normal60;*/
	
	self.th_weapon=flameorb_fire;
	self.last_attack=time;
	if(self.wfs==WF_CYCLE_WRAPPED||self.greenmana<1||(self.greenmana<10&&self.artifact_active&ART_TOMEOFPOWER))
	{
		self.t_width=-1;
		self.weaponframe_cnt=0;
		if(!self.artifact_active&ART_TOMEOFPOWER)
		{
			self.aflag=FALSE;
			//self.attack_finished=time+0.2;
			self.attack_finished=time+0.5;	// 0.2 is REALLY fast for an attack this visually expensive
			flameorb_ready_normal();
		}
		else
		{
			self.attack_finished = time + 1;
			flameorb_ready_power();
		}
	}
	else if(self.weaponframe==$normal52)
	{
		if(self.t_width==-1)
		{
			sound(self,CHAN_BODY,"succubus/flamstrt.wav",0.5,ATTN_NORM);
			self.t_width=FALSE;
		}	

		if(self.t_width<time)
		{
			//sound(self,CHAN_WEAPON,"succubus/flamloop.wav",0.5,ATTN_NORM);
			self.t_width=time+0.45;
		}
		if(!self.weaponframe_cnt)
			flamestream_fire();
		self.weaponframe_cnt+=1;
		if(self.weaponframe_cnt==20)
			self.weaponframe_cnt=0;
	}
	else if(self.weaponframe == $powern136)//Fixme: hold this frame for a few
	{
		sound(self,CHAN_BODY,"succubus/flamstrt.wav",0.5,ATTN_NORM);
		flameswarm_fire();
	}
}










void Suc_Forb_Fire()
{
	flameorb_fire();

	thinktime self : 0;
}

void flameorb_jellyfingers_normal ()
{
	self.wfs = advanceweaponframe($idlebn01,$idlebn25);
	self.th_weapon=flameorb_jellyfingers_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			flameorb_ready_power();
		else
			flameorb_ready_normal();
}

void flameorb_jellyfingers_power ()
{
	self.wfs = advanceweaponframe($pidleb085,$pidleb108);
	self.th_weapon=flameorb_jellyfingers_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		if(self.artifact_active&ART_TOMEOFPOWER)
			flameorb_ready_power();
		else
			flameorb_ready_normal();
}

void flameorb_to_power (void)
{
	self.wfs = advanceweaponframe($topowr65,$topowr84);
	self.th_weapon=flameorb_to_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		flameorb_ready_power();
}

void flameorb_to_normal (void)
{
	self.wfs = advanceweaponframe($tonrml149,$tonrml168);
	self.th_weapon=flameorb_to_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		flameorb_ready_normal();
}

void flameorb_ready_normal (void)
{
	self.wfs = advanceweaponframe($idlean26,$idlean50);
	if(random()<0.1&&self.weaponframe==$idlean50)
		self.th_weapon=flameorb_jellyfingers_normal;
	else
		self.th_weapon=flameorb_ready_normal;
	if(self.artifact_active&ART_TOMEOFPOWER)
	{
		self.weaponframe=$topowr65;
		flameorb_to_power();
	}
}

void flameorb_ready_power (void)
{
	self.wfs = advanceweaponframe($pidlea110,$pidlea134);
	if(random()<0.1&&self.weaponframe==$pidlea134)
		self.th_weapon=flameorb_jellyfingers_power;
	else
		self.th_weapon=flameorb_ready_power;
	if(!self.artifact_active&ART_TOMEOFPOWER)
	{
		self.weaponframe=$tonrml149;
		flameorb_to_normal();
	}
}

void flameorb_select_normal (void)
{
	self.wfs = advanceweaponframe($ndesel223,$ndesel213);
	self.weaponmodel = "models/sucwp3.mdl";
	self.th_weapon=flameorb_select_normal;
	self.t_width=-1;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		flameorb_ready_normal();
	}
}

void flameorb_select_power (void)
{
	self.wfs = advanceweaponframe($pselon224,$pselon234);
	self.weaponmodel = "models/sucwp3.mdl";
	self.th_weapon=flameorb_select_power;
	self.t_width=-1;
	if(self.wfs==WF_CYCLE_WRAPPED)
	{
		self.attack_finished = time - 1;
		flameorb_ready_power();
	}
}

void flameorb_select (void)
{
	self.weaponframe_cnt = 0;

	if(self.artifact_active&ART_TOMEOFPOWER)
		flameorb_select_power();
	else
		flameorb_select_normal();
}

void flameorb_deselect_normal (void)
{
	self.wfs = advanceweaponframe($ndesel213,$ndesel223);
	self.th_weapon=flameorb_deselect_normal;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void flameorb_deselect_power (void)
{
	self.wfs = advanceweaponframe($pselon234,$pselon224);
	self.th_weapon=flameorb_deselect_power;
	if(self.wfs==WF_CYCLE_WRAPPED)
		W_SetCurrentAmmo();
}

void flameorb_deselect (void)
{
	if(self.artifact_active&ART_TOMEOFPOWER)
		flameorb_deselect_power();
	else
		flameorb_deselect_normal();
}

