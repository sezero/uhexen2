/*
==============================================================================

Q:\art\models\monsters\SerpentR\final\big\bigeido.hc
MG
==============================================================================
*/
/*
// For building the model
$cd Q:\art\models\monsters\SerpentR\final\big
$origin 0 0 0
$base BASE SKIN
$skin SKIN
$skin SKIN2
$flags 0
$scale 2.5
*/
//Shared
$frame painA1       painA2       painA3       painA4       painA5       
$frame painA6       painA7       painA8       painA9       

//
$frame spell1       spell2       spell3       spell4       spell5       
$frame spell6       spell7       spell8       spell9       spell10      
$frame spell11      spell12      spell13      spell14      spell15      
$frame spell16      spell17      spell18      spell19      spell20      

//
$frame tranA1       tranA2       tranA3       tranA4       tranA5       
$frame tranA6       tranA7       tranA8       

//
$frame tranB1       tranB2       tranB3       tranB4       tranB5       
$frame tranB6       tranB7       tranB8       

//
$frame wait1        wait2        wait3        wait4        wait5        
$frame wait6        wait7        wait8        wait9        wait10       
$frame wait11       wait12       wait13       wait14       wait15       
$frame wait16       

//
$frame walk1        walk2        walk3        walk4        walk5        
$frame walk6        walk7        walk8        walk9        walk10       
$frame walk11       walk12       walk13       walk14       walk15       
$frame walk16       walk17       walk18       walk19       walk20       
$frame walk21       walk22       walk23       walk24       

//
$frame howl1        howl2        howl3        howl4        howl5        
$frame howl6        howl7        howl8        howl9        howl10       
$frame howl11       howl12       howl13       howl14       howl15       
$frame howl16       howl17       howl18       howl19       howl20       
$frame howl21       howl22       howl23       howl24       howl25       
$frame howl26       howl27       howl28       howl29       howl30       
$frame howl31       howl32       howl33       howl34       howl35       
$frame howl36       howl37       howl38       howl39       howl40       
$frame howl41       howl42       howl43       howl44       howl45       
$frame howl46       howl47       howl48       howl49       howl50       
$frame howl51       howl52       howl53       howl54       howl55       
$frame howl56       howl57       howl58       howl59       howl60       

$framesave x

//SMALL
//
//
$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      death15      
$frame death16      death17      death18      death19      death20      
$frame death21      death22      death23      death24      death25      
$frame death26      death27      death28      death29      death30      

//
$frame dwait1       dwait3       dwait5       
$frame dwait7       dwait9       
$frame dwait11      dwait13      dwait15      
$frame dwait17      dwait19      
$frame dwait21      dwait23      dwait25      
$frame dwait27      dwait29      


//
$frame grow1        grow3        grow5        
$frame grow7        grow9        
$frame grow11       grow13       grow15       
$frame grow17       grow19       
$frame grow21       grow23       grow25       
$frame grow27       grow29       
$frame grow31       grow33       grow35       
$frame grow37       grow39       
$frame grow41       grow43       grow45       
$frame grow47       grow49       
$frame grow51       grow53       grow55       
$frame grow57       grow59       
$frame grow61       grow63       grow65       
$frame grow67       grow69       
$frame grow71       grow72       grow73       grow74       grow75       
$frame grow76       grow77       grow78       grow79       grow80       
$frame grow81       grow82       grow83       grow84       grow85       
$frame grow86       grow87       grow88       grow89       grow90       
$frame grow91       grow92       grow93       grow94       grow95       
$frame grow96       grow97       grow98       grow99       grow100      


$framerestore x

//BIG
//
$frame breath1      breath2      breath3      breath4      breath5      
$frame breath6      breath7      breath8      breath9      breath10     
$frame breath11     breath12     breath13     breath14     breath15     
$frame breath16     breath17     breath18     breath19     breath20     
$frame breath21     breath22     breath23     breath24     breath25     
$frame breath26     breath27     breath28     breath29     breath30     
$frame breath31     breath32     breath33     breath34     breath35     
$frame breath36     breath37     breath38     breath39     breath40     
$frame breath41     breath42     breath43     breath44     breath45     
$frame breath46     breath47     breath48     breath49     breath50     

//
$frame painB1       painB2       painB3       painB4       painB5       
$frame painB6       painB7       painB8       painB9       painB10      
$frame painB11      painB12      painB13      painB14      painB15      
$frame painB16      painB17      painB18      painB19      painB20      

//
$frame power1       power2       power3       power4       power5       
$frame power6       power7       power8       power9       power10      
$frame power11      power12      power13      power14      power15      
$frame power16      power17      power18      power19      power20      

/*========================================================*/

void()orb_wait;
void()eidolon_orb_pain;
void()eidolon_run;
void()eidolon_roar;
void()eidolon_ready_roar;
void()eidolon_grow;
void(entity attacker,float total_damage)eidolon_check_fake;
void()eidolon_power;
void()eidolon_face_orb;

void orb_die()
{
	self.owner.health=4000+skill*2000;
	self.owner.th_save=eidolon_ready_roar;
	self.owner.controller=world;
	self.owner.goalentity=self.owner.enemy;
	self.owner.think=multiplayer_health;
	thinktime self.owner : 0;
	sound(self,CHAN_AUTO,"eidolon/orbxpld.wav",1,ATTN_NONE);
	MonsterQuake(500);
	MultiExplode();
}

void orb_tint_flash_from_red() [++ 0 .. 35]
{
	self.colormap+=1;
	if(self.colormap==143)
		self.think=orb_wait;
}

void orb_tint_flash_to_red()  [++ 0 .. 35]
{
	self.colormap-=1;
	if(self.colormap==128)
		self.think=orb_tint_flash_from_red;
}

void orb_pain (entity attacker,float damage)
{
	if(attacker==self.owner||self.owner.think==eidolon_grow)
	{
		self.health+=damage;
		return;
	}
	if(self.pain_finished<time)
	{
		sound(self,CHAN_BODY,"eidolon/orbhurt.wav",1,ATTN_NONE);
		self.pain_finished=time+1.75;
	}
	else
		return;
	if(self.owner.level)
		self.owner.think=eidolon_orb_pain;
	self.colormap=143;
	self.think=orb_tint_flash_to_red;
	thinktime self : 0;
}

void orb_lightning_recharge ()  [++ 0 .. 35]
{
	if(!self.aflag)
	{
		sound(self,CHAN_AUTO,"eidolon/chrgstrt.wav",1,ATTN_NONE);
//		sound(self,CHAN_BODY,"eidolon/chrgloop.wav",1,ATTN_NONE);
		self.aflag=TRUE;
		self.movetype = MOVETYPE_NONE;
	}

	if(self.weaponframe_cnt<16)
		self.weaponframe_cnt+=1;
	else
		self.weaponframe_cnt=0;

	if(!self.colormap)
		self.colormap=159;
	else if(self.colormap>155)
		self.colormap-=1;

	self.v_angle=randomv('0 0 0','360 360 360');
	makevectors(self.v_angle);
	self.view_ofs=self.origin+'0 0 1'*self.absmax_z*0.6+v_forward*44;
	if(self.owner.think==eidolon_grow)
	{
		self.proj_ofs=(self.owner.absmin+self.owner.absmax)*0.5-'0 0 64';
		self.proj_ofs_x+=random(0-self.owner.size_x,self.owner.size_x);
		self.proj_ofs_y+=random(0-self.owner.size_x,self.owner.size_x);
		self.proj_ofs_z+=random(0-self.owner.size_x,self.owner.size_x);
	}
	else
	{
		makevectors(self.owner.angles);
		self.proj_ofs=self.owner.origin+self.owner.proj_ofs+v_forward*200+v_right*36+'0 0 100';
	}

	do_lightning (self.owner,self.weaponframe_cnt,0, 2, self.view_ofs, self.proj_ofs,0,TE_STREAM_LIGHTNING);
}

void orb_lightning_pattern ()  [++ 0 .. 35]
{
	if(self.frame==0)
		sound(self,CHAN_BODY,"eidolon/orbpulse.wav",1,ATTN_NONE);
	if(self.cnt)
	{
		if(self.weaponframe_cnt<16)
			self.weaponframe_cnt+=1;
		else
			self.weaponframe_cnt=0;
		self.cnt-=1;
		self.view_ofs=self.proj_ofs;
//		if(random()<0.5)
			self.v_angle+=randomv('15 15 15','45 45 45');
//		else
//			self.v_angle-=randomv('15 15 15','45 45 45');
		makevectors(self.v_angle);
		self.proj_ofs=self.origin+'0 0 1'*self.absmax_z*0.6+v_forward*54;
		do_lightning (self.owner,self.weaponframe_cnt,0, random(4), self.view_ofs, self.proj_ofs,0,TE_STREAM_LIGHTNING);
	}
	else
	{
		self.think=orb_wait;
		thinktime self : 0;
	}
}

void orb_lightning_pattern_init ()
{
//	dprint("Orb starting lightning\n");
	self.cnt=random(40,60);	
	self.weaponframe_cnt=0;
	self.v_angle=randomv('0 0 0','360 360 360');
	makevectors(self.v_angle);
	self.view_ofs=self.origin+'0 0 1'*self.absmax_z*0.6+v_forward*54;
	self.v_angle+=randomv('-45 -45 -45','45 45 45');
	makevectors(self.v_angle);
	self.proj_ofs=self.origin+'0 0 1'*self.absmax_z*0.6+v_forward*54;
	do_lightning (self.owner,self.weaponframe_cnt,0, random(3), self.view_ofs, self.proj_ofs,0,TE_STREAM_LIGHTNING);
	self.think=orb_lightning_pattern;
	thinktime self :0.05;
}

void orb_wait () [++ 0 .. 35]
{
	self.aflag=FALSE;
	if(self.frame==0)
		sound(self,CHAN_BODY,"eidolon/orbpulse.wav",1,ATTN_NONE);

	self.velocity='0 0 0';
	self.colormap=0;

	if(random()<0.1)
	{
		self.think=orb_lightning_pattern_init;
		thinktime self :0;
	}
}


void obj_chaos_orb_find_movechain ()
{	//So chaos orb moves with platform but doesn't block geometry
entity found;
	found=find(world,netname,self.netname);
	if(found)
	{
		found.movechain=self;
		self.flags(+)FL_MOVECHAIN_ANGLE;
	}
	self.think=orb_wait;
	thinktime self : 0;
}
	
/*QUAKED obj_chaos_orb (1 0 0) (-100 -100 0) (100 100 200) JEAN LUC PICARD
Big round smooth thingie.
-------------------------FIELDS-------------------------
--------------------------------------------------------
*/
void obj_chaos_orb ()
{
	if(deathmatch)
	{
		remove(self);
		return;
	}
	precache_model2 ("models/boss/chaosorb.mdl");
	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NOCLIP;
	self.takedamage=DAMAGE_YES;
	self.thingtype=THINGTYPE_GLASS;

	setmodel (self, "models/boss/chaosorb.mdl");

	setsize (self, '-48 -48 0', '48 48 123');
	self.hull=HULL_POINT;
	self.monsterclass=CLASS_BOSS;
	self.health = self.max_health = 2000;

	self.mass = 5000;
	self.cnt=50;
	
	self.flags2(+)FL_ALIVE;
	self.dmg=200;
	self.th_die=orb_die;
	self.th_pain=orb_pain;
	self.drawflags(+)SCALE_ORIGIN_BOTTOM|MLS_POWERMODE;

	self.think=obj_chaos_orb_find_movechain;
	thinktime self : 0.1;
}

//====================================================================

void() eidolon_walk;
void()eidolon_wait;
float eidolon_check_attack()
{
vector	spot1, spot2;	
entity	targ;

	if(self.movetype)
		return FALSE;
		
	if(self.goalentity==self.controller)
		return FALSE;

	targ = self.enemy;
	
// see if any entities are in the way of the shot
	spot1 = self.origin + self.proj_ofs;
	spot2 = (targ.absmin+targ.absmax)*0.5;

	traceline (spot1, spot2, FALSE, self);

	if(trace_ent.thingtype>=THINGTYPE_WEBS)
		traceline (trace_endpos, spot2, FALSE, trace_ent);

	if (trace_ent != targ)
		if(trace_ent.health>200||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
			return FALSE;//Don't have a clear shot, and don't want to shoot obstruction
			
	enemy_range=vlen(self.enemy.origin-self.origin);
	if (enemy_range < 200&&self.scale>1)
	{
		self.th_melee();
		return TRUE;
	}

// missile attack
	if (time < self.attack_finished)
		return FALSE;
		
	if(random()<0.3 - skill/10&&self.controller.flags2&FL_ALIVE)
		return FALSE;

	self.th_missile ();
	SUB_AttackFinished (random(0,2));
	return TRUE;
}

void check_use_model (string whichmodel)
{
	if(self.model!=whichmodel)
		setmodel(self,whichmodel);

	if(!self.flags2&FL_SMALL)
		setsize(self,'-54 -54 0', '54 54 666');
	else
		setsize(self,'-16 -16 0', '16 16 200');
	self.hull=HULL_POINT;
}

float eidolon_riderpath_move(float move_speed)
{
entity next_path;
float distance;//, altitude, temp;
vector displace;

	if(self.movetype==MOVETYPE_NOCLIP)
		self.velocity='0 0 0';

	next_path=riderpath_findbest(self.path_last);
	distance = vhlen(self.origin - self.path_current.origin);
	if (distance < self.rider_path_distance)
	{
		if(next_path==world)	//Already on closest path
			return FALSE;
		else if(self.turn_time<time)
		{
			self.path_last=self.path_current;
			self.path_current=next_path;
			self.turn_time=time+1;//Change points only once every second
		}
		else
			return FALSE;
	}
/*	else if(self.path_current.path_id!=1)
		if(next_path!=self.path_current&&next_path!=world)
			if(self.turn_time<time)
			{
				next_path=self.path_last;
				self.path_last=self.path_current;
				self.path_current=next_path;
				self.turn_time=time+1;//Change points only once every second
			}
			else
				return FALSE;
*/			
	if(fov(self.path_current,self,30)||self.scale==1)
	{
		self.yaw_speed=10;
		move_speed=self.speed*self.scale;
	}
	else
	{
		self.yaw_speed=15;
		move_speed=self.speed*self.scale/2;
	}

	self.ideal_yaw = vectoyaw(self.path_current.origin - self.origin);
	ChangeYaw();

// if an Imp Lord hits Eidolon before the first roaring
// sequence, landing may fail (see devel/eidolon.txt) :
	// not really elegant, but short and effective
	if (checkbottom(self))
		self.flags(+)FL_ONGROUND;
	else
		droptofloor();

	if(!walkmove(self.angles_y, move_speed, TRUE))
	{
	float cant_move;
	entity found;
		if(trace_ent.movetype==MOVETYPE_NONE||trace_ent.movetype==MOVETYPE_PUSH||trace_ent.movetype==MOVETYPE_NOCLIP)
		{
			found=findradius(self.origin,self.size_x+move_speed);
			while(found)
			{
				if(found.classname=="player")
					if(fov(found,self,90))
						cant_move=TRUE;
				found=found.chain;
			}
			if(!cant_move)
			{
				makevectors(self.angles);
				setorigin(self,self.origin+v_forward*move_speed);
			}
		}
		else if(trace_ent!=world)
		{
			displace = normalize(trace_ent.origin - self.origin);
			if (infront(trace_ent))
			{
				trace_ent.velocity += displace*random(1000,1600);
				trace_ent.punchangle= randomv('-9 -10 -10','-1 10 10');
				T_Damage (trace_ent, self, self, random(25,35));
			}
			else
			{
				trace_ent.velocity += displace*random(700,900);
				trace_ent.punchangle = randomv('-3 -5 -5','2 5 5');
				T_Damage (trace_ent, self, self, random(15,20));
			}
		}
	}
	self.yaw_speed=10;
	return TRUE;
}

void eidolon_explode ()
{
//Stop lightning
vector org,from;
	self.lockentity.wait=100;
	self.lockentity.dmg=0;
	makevectors(self.angles);
	org=(self.absmin+self.absmax)*0.5;
	from=org+'0 0 500';
	do_lightning (self,1,0,4,org,from+v_forward*300,0,TE_STREAM_LIGHTNING);
	do_lightning (self,1,0,4,org,from+v_right*300,0,TE_STREAM_LIGHTNING);
	do_lightning (self,1,0,4,org,from-v_forward*300,0,TE_STREAM_LIGHTNING);
	do_lightning (self,1,0,4,org,from-v_up*300,0,TE_STREAM_LIGHTNING);
	sound(self,CHAN_BODY,"player/megagib.wav",1,ATTN_NONE);
	sound (self, CHAN_ITEM, "weapons/exphuge.wav", 1, ATTN_NONE);
	SpawnPuff(org,self.size,100,self);
	chunk_death();
	chunk_death();
	chunk_death();
	self.frame=0;
	check_use_model("models/null.spr");
	self.think=rider_die;
	thinktime self : 0;
}

void eidolon_shake () [-- $howl60 .. $howl1]
{
	if(self.frame>$howl5 && self.frame<$howl55 &&random()<0.3)
		self.frame+=random(-4,4);
	self.angles_y+=random(-11,9);
	if(self.scale>2.45)
		self.scale=2.3;
	else if(self.scale>1&&self.scale<=2.45)
		self.scale+=random(-0.1,0.05);
	if(self.abslight>2)
		self.abslight=1.7;
	else if(self.abslight>0.3&&self.abslight<=2)
		self.abslight+=random(-0.05,0.05);
	if(self.lifetime<time&&random()<0.1)
		eidolon_explode();
}

void eidolon_die () [++ $howl1 .. $howl60]
{
//FIXME: shakes, rattles and rolls
//			earthquakes, lighting, sinking into ground, explode,
//			huge gibs, blood, white flash
	if(self.frame==$howl60)
	{
		self.scale=2.3;
		self.lifetime=time+7;
		self.think=eidolon_shake;
		thinktime self : 0;
	}
	else if(self.frame==$howl1)
	{
//		check_use_model("models/boss/bigeido.mdl");
		sound(self,CHAN_AUTO,"eidolon/death.wav",1,ATTN_NONE);
		self.drawflags(+)MLS_ABSLIGHT;
		self.abslight=0.5;
		MonsterQuake(500);
		self.lockentity.wait=20;
		self.lockentity.dmg=30;
	}
}

void eidolon_spawn_lightning ()
{
entity oself;
	oself=self;
	self=self.lockentity;
	self.use();
	self=oself;
}

/* see devel/eidolon.txt for why god mode is needed. */
void eidolon_roar () [++ $howl1 .. $howl60]
{
	if(random()<0.5&&self.lockentity!=world&&self.frame>$howl13)
		eidolon_spawn_lightning();
	if(self.frame==$howl60)
	{
		self.movetype = MOVETYPE_NONE;
		self.flags(-)FL_GODMODE;
		self.th_pain = eidolon_check_fake;
		self.attack_finished=time+3;
		self.think=eidolon_guarding;
	}
}

void eidolon_ready_roar()
{
//	check_use_model("models/boss/bigeido.mdl");
	sound(self,CHAN_VOICE,"eidolon/roar.wav",1,ATTN_NONE);
	self.attack_finished=time+1.5;
	self.frame=$howl1;
	self.think=eidolon_roar;
	thinktime self : 0;
}

void eidolon_grow () [++ $grow1 .. $grow100]
{
//FIXME: If player too close, push away
entity found;
	if(self.frame<$grow71)
		thinktime self : 0.1;
//	check_use_model("models/boss/bigeido.mdl");
	if(self.scale<2.52)
		self.scale+=0.03;
	setsize (self, '-16 -16 0'*1.3333333*self.scale, '16 16 200'*1.3333333*self.scale);
	self.mass=2000*1.34*self.scale;
	self.hull=HULL_POINT;
	self.health=self.max_health;
	found=findradius(self.origin,self.size_x+25);
	while(found)
	{
		if(found!=self&&found.solid&&found.movetype&&found.health&&found.flags2&FL_ALIVE)
		{
			found.velocity=normalize(found.origin-self.origin)*100;
			found.velocity_z+=100;
			found.flags(-)FL_ONGROUND;
			T_Damage(found,self,self,3);
		}
		found=found.chain;
	}
	if(cycle_wrapped)
	{
		self.scale=2.55;
		self.rider_path_distance=64;
		self.weapon=0;
		self.health=10000;
		self.experience_value=self.init_exp_val=100000;
		self.drawflags(-)MLS_POWERMODE;
		self.flags2(-)FL_SMALL;
		self.controller.think=orb_wait;
		thinktime self.controller : 0;
		self.proj_ofs=self.view_ofs='0 0 200';
		check_use_model("models/boss/bigeido.mdl");
		self.frame=$howl1;
		self.think=eidolon_ready_roar;
		thinktime self : 1;
	}
}

void eidolon_ready_grow () [++ $dwait1 .. $dwait29]
{
	thinktime self : 0.1;
	if(self.frame==$dwait29 &&self.lifetime<time)
	{
		sound(self,CHAN_VOICE,"eidolon/growl.wav",1,ATTN_NONE);
		self.drawflags(+)MLS_POWERMODE;
		self.controller.think=orb_lightning_recharge;
		thinktime self.controller : 0;
		self.lockentity.wait=33;
		self.lockentity.dmg=10;
		self.think=eidolon_grow;
	}
}

void eidolon_darken_sky () [++ $dwait1 .. $dwait29]
{
float lightval;
entity watcher;
//	check_use_model("models/boss/smaleido.mdl");
	lightval=lightstylevalue(self.lockentity.style);
//	else if(self.frame==$dwait29 &&lineofsight(self,self.enemy))
	if(lightval>2)
	{
		lightval-=1;
		lightstylestatic(self.lockentity.style,lightval);
	}
	else if(self.frame==$dwait29)
/*		if(infront_of_ent(self,self.enemy))
		if(infront_of_ent(self.controller,self.enemy))
		if(vlen(self.enemy.origin-self.origin)<1500)
		{
			self.velocity='0 0 0';
			self.movetype=MOVETYPE_NOCLIP;
			self.flags(+)FL_FLY;
			MonsterQuake(500);
			SUB_UseTargets();
			self.target="";
			self.lifetime=time+2;
			self.think=eidolon_ready_grow;
		}
	self.health=self.max_health;
	self.lockentity.lightvalue1=lightval;
*/
	{
		watcher=self.enemy;
		if(self.enemy.classname=="monster_imp_lord")//if enemy an imp, look for it's owner
			watcher=self.enemy.controller;
		if(!watcher.flags2&FL_ALIVE)
		{//If enemy not alive, look for other players
			watcher=find(world,classname,"player");
			while(watcher!=world&&!watcher.flags2&FL_ALIVE)
				watcher=find(watcher,classname,"player");
		}
		if(infront_of_ent(self,watcher))
			if(infront_of_ent(self.controller,watcher))
				if(vlen(self.enemy.origin-self.origin)<1500)
				{
					self.velocity='0 0 0';
					self.movetype=MOVETYPE_NOCLIP;
					self.flags(+)FL_FLY;
					MonsterQuake(500);
					SUB_UseTargets();
					self.target="";
					self.lifetime=time+2;
					self.think=eidolon_ready_grow;
				}
	}
	self.health=self.max_health;
	self.lockentity.lightvalue1=lightval;
}

void eidolon_fake_die () [++ $death1 .. $death30]
{
//	check_use_model("models/boss/smaleido.mdl");
	self.health=self.max_health;
	if(self.frame==$death30)
		if(self.lockentity!=world)
		{
			self.lockentity.wait=100;
			self.lockentity.dmg=0;
			self.think=eidolon_darken_sky;
		}
		else
		{
			self.flags(-)FL_GODMODE;
			self.th_pain=eidolon_check_fake;
			self.think=eidolon_run;
		}
}

void eidolon_orb_pain () [++ $painB1 .. $painB20]
{
	if(self.frame==$painB1)
	{
//		check_use_model("models/boss/bigeido.mdl");
		if(self.controller.think==orb_lightning_recharge)
		{
			self.controller.think=orb_wait;
			thinktime self.controller : 0;
		}
	}
	if(self.frame==$painB20)
	{
		self.weapon=0;
		self.goalentity=self.enemy;
		self.colormap=0;
		self.level=FALSE;
		self.think=eidolon_run;
	}
}

void()eidolon_face_orb;
void eidolon_pain () [++ $painA1 .. $painA9]
{
//	if(self.frame==$painA1)
//		check_use_model("models/boss/smaleido.mdl");

	if(self.frame==$painA9)
	{
		if(self.weapon>=1000&&self.controller.flags2&FL_ALIVE)
		{
			self.weapon=0;
			self.think=eidolon_face_orb;
		}
		else
			self.think=eidolon_run;
	}
}

void eidolon_check_fake (entity attacker,float total_damage)
{
float pain_chance;
	if(self.controller.flags2&FL_ALIVE||self.scale<1)
	{//orb alive or still small
		self.dmg+=self.max_health-self.health;
		self.health=self.max_health;
		if(self.scale>1)
		{
			pain_chance=0.1;
			self.weapon+=total_damage;
		}
		else
		{
			pain_chance=0.6;
			self.weapon=0;
		}
	}
	else
	{//Big and orb dead
		pain_chance=0.2;
		self.dmg=0;
	}
	pain_chance-=self.torncount*0.02;
	if(self.movetype!=MOVETYPE_NONE)
		return;

	if(self.pain_finished>time)
		return;

	self.pain_finished=time+3+skill;

	if(self.dmg>=2000&&self.scale<1)
	{
	/* see devel/eidolon.txt for why god mode is needed. */
		self.flags(+)FL_GODMODE;
		self.th_pain=SUB_Null;
		if(attacker.classname=="player")
			AwardExperience(attacker,self,self.experience_value);
		sound(self,CHAN_VOICE,"eidolon/fakedie.wav",1,ATTN_NONE);
		self.goalentity=self.enemy;
		self.think=eidolon_fake_die;
	}
	else if(random()<pain_chance/2)
	{
		self.goalentity=self.enemy;
		sound(self,CHAN_VOICE,"eidolon/pain.wav",1,ATTN_NONE);
		if(self.level)
			self.think=eidolon_orb_pain;
		else
			self.think=eidolon_pain;
	}
	else if(self.weapon>=1000)
	{
		self.goalentity=self.enemy;
		sound(self,CHAN_VOICE,"eidolon/pain.wav",1,ATTN_NONE);
		if(self.level)
			self.think=eidolon_orb_pain;
		else
			self.think=eidolon_pain;
	}
	else
		return;
	thinktime self : 0;
}

void eidolon_fireball (void)
{
entity missile;

	self.last_attack=time;
	self.cnt+=1;
	if(self.cnt==2)
	{
		self.attack_finished=time+7;
		self.cnt=0;
	}

	missile = spawn ();
	missile.owner = self;
	missile.movetype = MOVETYPE_FLYMISSILE;
	missile.solid = SOLID_BBOX;

	missile.classname = "eidolon fireball";
			
	// set missile speed	
	makevectors (self.angles);
	missile.enemy=self.enemy;

	setmodel (missile, "models/eidoball.mdl");
	setsize (missile, '0 0 0', '0 0 0');		
	setorigin (missile, self.origin + v_forward*128 + self.proj_ofs+v_right*24-v_up*12);

	missile.wallspot=normalize((self.enemy.absmin+self.enemy.absmax)*0.5-missile.origin);
	missile.movedir=v_forward;
	missile.movedir_z=missile.wallspot_z;
	missile.speed=1000;
    missile.velocity = missile.movedir*missile.speed;
	
	missile.touch = pmissile2_touch;
	missile.angles = vectoangles(missile.velocity);

	sound(self,CHAN_AUTO,"eidolon/fireball.wav",1,ATTN_NONE);


	thinktime missile : 0.15;
	missile.think = pmissile2_puff;
	missile.lifetime = time + 2;
	missile.drawflags(+)MLS_ABSLIGHT;
	missile.abslight=0.5;
//	missile.effects=EF_BRIGHTLIGHT;
	missile.scale=2;
//Homing stuff-------------------
	missile.veer=FALSE;	//No random wandering
	missile.turn_time=2;//Lower the number, tighter the turn
	missile.ideal_yaw=TRUE;//Only track things in front
//End homing stuff-------------------
}

void flame_stream_touch ()
{
	if(other.classname=="flamestream")
		return;
	self.effects(+)EF_MUZZLEFLASH;
	if(other.takedamage)
		T_Damage(other,self,self.owner,self.dmg);
	else
		T_RadiusDamage(self,self.owner,self.dmg*2,self.owner);	
	if(self.frame<24)
		self.frame=24;
}

void eidolon_power () [++ $power1 .. $power20]
{
//	check_use_model("models/boss/bigeido.mdl");
	if(self.frame==$power20)
	{
		self.colormap=0;
		self.level=FALSE;
		self.controller.think=orb_wait;
		self.goalentity=self.enemy;
		self.think=eidolon_run;
		thinktime self : 0;
	}
	if(self.frame==$power8)
	{
		self.controller.think=orb_lightning_recharge;
		thinktime self.controller : 0;
	}
	if(self.frame>=$power5 && self.frame<=$power15)
	{
		if(self.frame==$power10)
			thinktime self : 2;
		if(self.frame<$power10 )
			self.colormap=159 - (self.frame-$power5);
		else
			self.colormap=159 - ($power15 - self.frame);
	}
}

void eidolon_face_orb () [++ $walk1 .. $walk16]
{
//	check_use_model("models/boss/smaleido.mdl");

	self.ideal_yaw = vectoyaw(self.controller.origin - self.origin);
	ChangeYaw();
	if(self.angles_y>self.ideal_yaw - 10&&self.angles_y<self.ideal_yaw + 10)
	{
		self.level=TRUE;
		self.think=eidolon_power;
		thinktime self : 0;
	}
}

void fire_anim() [++ 0 .. 26]
{
	self.frags+=2;
	self.velocity_z+=self.frags;
	if(self.lifetime<time)
		remove(self);
	if(cycle_wrapped)
		remove(self);
	thinktime self : 0.025;
}

void eidolon_flames () [++ $breath1 .. $breath50]
{
float dot;
vector forward_dir;
//	check_use_model("models/boss/bigeido.mdl");
	ai_face();

	if(self.frame==$breath1)
		self.attack_finished=random(10);

	if(vlen(self.enemy.origin-self.origin)<200)
	{
		self.enemy.velocity=normalize(self.enemy.origin-self.origin)*100;
		if(self.enemy.flags&FL_ONGROUND)
			self.enemy.velocity_z=150;
		self.enemy.flags(-)FL_ONGROUND;
	}

	if(self.frame>=$breath25 &&self.frame<=$breath41)
	{
		makevectors(self.angles);
		if(!self.aflag)
		{
//			sound(self,CHAN_BODY,"misc/combust.wav",1,ATTN_NONE);
//			bprint("Flame start\n");
			sound(self,CHAN_BODY,"eidolon/flamstrt.wav",1,ATTN_NONE);
			self.aflag=TRUE;
		}	

		if(self.t_width<time)
		{
//			sound(self,CHAN_VOICE,"misc/fburn_bg.wav",1,ATTN_NONE);
			sound(self,CHAN_VOICE,"eidolon/flambrth.wav",1,ATTN_NONE);
			self.t_width=time+0.5;
		}
		self.effects(+)EF_MUZZLEFLASH;
		newmis=spawn_temp();
		newmis.classname="eidolon flames";
		newmis.owner=self;
		newmis.movetype=MOVETYPE_FLYMISSILE;
		newmis.solid=SOLID_BBOX;
		newmis.touch=flame_stream_touch;
		newmis.dmg=30;
		newmis.lifetime=time+2;

		forward_dir=v_forward;
		newmis.o_angle=self.origin+self.proj_ofs+forward_dir*128+v_right*20-v_up*12;
		newmis.wallspot=normalize(self.enemy.origin-newmis.o_angle);
		newmis.movedir=forward_dir;
		newmis.movedir_z=newmis.wallspot_z*1.7;

		newmis.wallspot_z=0;
		makevectors(newmis.wallspot);
		dot=forward_dir*v_forward;
		if(dot>0.85)
		{
			newmis.movedir_y=newmis.wallspot_y;
			newmis.movedir_x=newmis.wallspot_x;
		}

		newmis.speed=300+random(50);
		newmis.velocity=newmis.movedir*newmis.speed;
		setmodel(newmis,"models/eidoflam.spr");
		newmis.think=fire_anim;	
		thinktime newmis : 0;

		setsize(newmis,'0 0 0','0 0 0');
		setorigin(newmis,newmis.o_angle);
		self.attack_finished+=1;	
	}
	if(self.frame==$breath50)
	{
		sound(self,CHAN_VOICE,"eidolon/flamend.wav",1,ATTN_NONE);
		self.aflag=FALSE;
		self.attack_finished=time+10;
		self.think=eidolon_guarding;
	}

	if(self.frame>$breath24 && self.frame<$breath41)
	{
		if(!self.frags)
		{
			self.frame-=1;
			self.frags=TRUE;
		}
		else
			self.frags=FALSE;
		thinktime self : 0.025;
	}
}

void eidolon_fireballs () [++ $breath1 .. $breath50]
{
//	check_use_model("models/boss/bigeido.mdl");
	ai_face();
	if(self.frame==$breath36)
		eidolon_fireball();
	if(random()<(0.1+skill/10)&&self.frame>$breath24 && self.frame<$breath41)
		eidolon_fireball();
	if(self.frame==$breath50)
	{
		self.attack_finished=time+3;
		self.think=eidolon_guarding;
	}
}

void EidoPoly ()
{
vector forward_dir;
float dot;
	makevectors(self.angles);
	newmis=spawn();
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.owner=self;
	newmis.touch=poly_touch;

	newmis.speed=700;
	forward_dir=v_forward;
	newmis.o_angle=self.origin+self.proj_ofs+forward_dir*220+v_right*36;
	newmis.wallspot=normalize(self.enemy.origin-newmis.o_angle);
	newmis.movedir=forward_dir;
	newmis.movedir_z=newmis.wallspot_z*1.7;
	newmis.wallspot_z=0;
	makevectors(newmis.wallspot);
	dot=forward_dir*v_forward;
	if(dot>0.85)
	{
		newmis.movedir_y=newmis.wallspot_y;
		newmis.movedir_x=newmis.wallspot_x;
	}
	newmis.velocity=newmis.movedir*newmis.speed+v_right*random(-100,100);
	
	newmis.drawflags=MLS_POWERMODE;

	sound(newmis,CHAN_BODY,"necro/mmfire.wav",1,ATTN_NORM);
	newmis.think=polymorph_anim;
	thinktime newmis : 0;
	newmis.scale=2;
	setmodel(newmis,"models/polymrph.spr");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,newmis.o_angle);
}

void eidolon_spell () [++ $spell1 .. $spell20]
{
//	check_use_model("models/boss/smaleido.mdl");
	ai_face();
	if((random()<(0.2+skill/10)&&self.frame>=$spell8 &&self.frame<=$spell16)||self.frame==$spell10)
	{
		makevectors(self.angles);
		self.movedir=normalize((self.enemy.absmax+self.enemy.absmin)*0.5-(self.origin+self.proj_ofs+v_forward*64));
		self.v_angle=v_forward;
		self.v_angle_z=self.movedir_z;
		if(self.veer)
			EidoPoly();
		else
			FireMagicMissile(0);
	}
	if(self.frame==$spell20)
	{
		self.veer=FALSE;
		self.attack_finished=time+2;
		self.think=eidolon_guarding;
	}
}

void eidolon_fire ()
{
float enemy_dist;
//NOTE: Use special pain- if hit him in the mouth as he's
//			about to use fireball or flames- hurts bad
	if(self.scale>1)
		if(random()<0.1)
			if(random()<0.1)
			{
				self.veer=TRUE;
				eidolon_spell();
				return;
			}
	enemy_dist=vlen(self.enemy.origin-self.origin);
	if(self.scale==0.75)
	{
		if(range(self.enemy)<=RANGE_MELEE)
			UseBlast();
		self.artifact_active(+)ART_TOMEOFPOWER;
		eidolon_spell();
	}
	else if(enemy_dist>128&&enemy_dist<424)
		self.think=eidolon_flames;
	else if(random()<0.2)
		self.think=eidolon_ready_roar;
	else
	{
		enemy_vis=visible(self.enemy);
		enemy_infront=infront(self.enemy);
		if(enemy_vis&&enemy_infront)
			self.think=eidolon_fireballs;
		else
			self.cnt=FALSE;
	}
}

void eidolon_find_lightning ()
{
entity found;
	found=find(world,classname,"light_thunderstorm");
	if(found)
		self.lockentity=found;
}

void eidolon_find_orb ()
{
entity found;
	found=find(world,classname,"obj_chaos_orb");
	if(found)
	{
		found.owner=self;
		self.controller=found;
	}
}

void eidolon_walk () [++ $walk1 .. $walk24]
{
//	check_use_model("models/boss/smaleido.mdl");
	if(self.scale>1&&(self.frame==$walk2 ||self.frame==$walk14))
		sound(self,CHAN_BODY,"eidolon/stomp.wav",1,ATTN_NONE);
	if(!self.lockentity)
		eidolon_find_lightning();
	if(!self.controller)
		eidolon_find_orb();
	ai_walk(self.speed*self.scale);
}

void eidolon_enemy ()
{
	if (self.enemy == world)
	{
		if (self.oldenemy != world && self.oldenemy.flags2 & FL_ALIVE)
		{
			self.enemy = self.oldenemy;
			self.goalentity = self.enemy;
		}
		else
			self.think = eidolon_wait;
	}
	else if (!self.enemy.flags2 & FL_ALIVE)
	{
		self.think = eidolon_ready_roar;
		self.enemy = world;
	}
	else
		ai_run(self.speed * self.scale);
}

void eidolon_run () [++ $walk1 .. $walk24]
{
//	dprint("Chasing\n");
//	check_use_model("models/boss/smaleido.mdl");
	if(self.scale>1&&(self.frame==$walk2 ||self.frame==$walk14))
		sound(self,CHAN_BODY,"eidolon/stomp.wav",1,ATTN_NONE);
	eidolon_enemy();
}

void eidolon_guarding () [++ $wait1 .. $wait16]
{
//	check_use_model("models/boss/smaleido.mdl");
//	dprint("Guarding\n");
	ai_face();
	eidolon_enemy();
}

void eidolon_wait () [++ $wait1 .. $wait16]
{
//	dprint("Waiting\n");
//	check_use_model("models/boss/smaleido.mdl");
	if(!self.lockentity)
		eidolon_find_lightning();
	if(!self.controller)
		eidolon_find_orb();

	ai_stand();
}

void multiplayer_health ()
{
entity lastent;
float num_players;
	if(coop)
	{
		lastent=nextent(world);
		num_players=0;
		while(lastent)
		{
			if(lastent.flags&FL_CLIENT)
				num_players+=1;
			lastent=find(lastent,classname,"player");
		}
		if(num_players>4)
			num_players=4;
		if(num_players>0)
		{
			self.max_health+=1000*num_players;
			self.torncount=num_players - 1;
		}
		self.health=self.max_health;
	}
	if(self.th_save!=SUB_Null)
		self.th_save();
}

/*QUAKED monster_eidolon (1 0 0) (-100 -100 0) (100 100 666) CUTE CUDDLY
The big bad ugly boss guy

-------------------------FIELDS-------------------------
--------------------------------------------------------

*/
void monster_eidolon(void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2 ("models/boss/smaleido.mdl");
	precache_model2 ("models/boss/bigeido.mdl");
	precache_model2 ("models/eidoball.mdl");
	precache_model2 ("models/eidoflam.spr");
	precache_model2 ("models/glowball.mdl");
    precache_model2 ("models/boss/shaft.mdl");
    precache_model2 ("models/boss/circle.mdl");
    precache_model2 ("models/boss/star.mdl");

	precache_sound2 ("eidolon/roar.wav");
	precache_sound2 ("eidolon/pain.wav");	//Hurt
	precache_sound2 ("eidolon/death.wav");	//Dies- long and agonizing
	precache_sound2 ("eidolon/fakedie.wav");//1st death- fake
	precache_sound2 ("eidolon/spell.wav");	//Spell attack (tracking globes)
	precache_sound2 ("eidolon/stomp.wav");	//Hot-steppin'
	precache_sound2 ("eidolon/fireball.wav");	//Launching Nasty fireballs
	precache_sound2 ("eidolon/flamstrt.wav");	//
	precache_sound2 ("eidolon/flambrth.wav");	//
	precache_sound2 ("eidolon/flamend.wav");	//
	precache_sound2 ("eidolon/growl.wav");		//
	precache_sound2 ("eidolon/chrgstrt.wav"); 	//Orb starts recharging Eido
	precache_sound2 ("eidolon/orbhurt.wav");	//Orb gets hit
	precache_sound2 ("eidolon/orbxpld.wav");	//Orb gets destroyed
	precache_sound2 ("eidolon/orbpulse.wav");	//Orb pulsating
	precache_sound2 ("famine/flashdie.wav");

	total_monsters += 1;

	self.speed=10.5;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;
	self.takedamage=DAMAGE_YES;
	self.monsterclass=CLASS_FINAL_BOSS;
	/* see devel/eidolon.txt for why god mode is needed. */
	self.flags(+)FL_MONSTER|FL_GODMODE;
	self.flags2(+)FL_ALIVE|FL_SMALL;
	self.thingtype=THINGTYPE_FLESH;

	setmodel (self, "models/boss/smaleido.mdl");
	self.skin = 0;

	setsize (self, '-40 -40 0', '40 40 150');
//	setsize (self, '-32 -32 0', '32 32 150');
	self.hull=HULL_GOLEM;
	self.health = self.max_health=3000+skill*1000;

	self.yaw_speed = 10;
	self.mass = 2000;

	self.rider_path_distance=30;

	self.proj_ofs=self.view_ofs='0 0 100';
	self.experience_value = self.init_exp_val=10000;

	self.th_stand = eidolon_wait;
	self.th_jump = eidolon_ready_roar;
	self.th_walk = eidolon_walk;
	self.th_run = eidolon_run;
	self.th_die = eidolon_die;
	self.th_melee = eidolon_flames;
	self.th_pain = eidolon_check_fake;
	self.th_missile = eidolon_fire;

	self.scale = 0.75;
	self.drawflags (+) SCALE_ORIGIN_BOTTOM;
	self.touch=SUB_Null;

	self.th_save=walkmonster_start;
	self.think=multiplayer_health;
	thinktime self : 2;
}

