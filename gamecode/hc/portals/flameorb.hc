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


void burner_think ()
{
	vector org,vel;
	if(self.lifetime<time||self.enemy.health<0)
	{
		stopSound(self.enemy,CHAN_BODY);
		//sound(self.enemy,CHAN_BODY,"misc/null.wav",1,ATTN_NORM);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		remove(self);
		return;
	}
	else if(pointcontents(self.enemy.origin)==CONTENT_WATER)
	{
		sound (self.enemy, CHAN_BODY, "misc/fout.wav", 1, ATTN_NORM);
		smolder((self.enemy.absmin+self.enemy.absmax)*0.5);
		self.enemy.flags2 (-) FL2_ONFIRE;
		self.enemy.effects (-) EF_DIMLIGHT;
		remove(self);
		return;
	}
	else
	{
		org=(self.enemy.absmin+self.enemy.absmax)*0.5+randomv(self.enemy.size*-0.25,self.enemy.size*0.25);
		vel=randomv('-3 -3 0','3 3 7'); 

		starteffect(CE_ONFIRE, org,vel, 0);

		thinktime self : random(0.5);
		T_Damage(self.enemy,self,self.owner,self.enemy.fire_damage + random(1));
		sound(self.enemy,CHAN_BODY,"raven/fire1.wav",1,ATTN_NORM);
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
	burner.enemy.effects (+) EF_DIMLIGHT;
	thinktime burner : 0;
	burner.enemy.flags2 (+) FL2_ONFIRE;
	//sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NORM);
	starteffect(CE_LG_EXPLOSION , self.origin);
}

void flameball_touch()
{
	if (other.takedamage)
	{
		float old_health;
		vector other_org;
	
		sound(self,CHAN_BODY,"succubus/flamend.wav",0.5,ATTN_NORM);
		other_org=other.origin;
		old_health=other.health;
		T_Damage(other,self,self.owner,10);
		if(other.health<=0&&old_health>0)
			smolder(other_org);		
	}
	else
	{
		T_Damage(other, self.owner, self.owner, 10);
	}

	local float rand;

    rand = random();

	if (rand < 0.2)
		starteffect(CE_SM_EXPLOSION, self.origin-self.movedir*6, '0 0 6', 0);
	else if (rand < 0.3)
		starteffect(CE_FBOOM, self.origin-self.movedir*6, '0 0 6', 0);
	else
		starteffect(CE_BOMB, self.origin-self.movedir*6, '0 0 6', 0);
			
	remove(self);
}

void flameball_think()
{
	starteffect(CE_FLAMESTREAM, self.origin, '0 0 2', 0);

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

	setorigin(newmis, pos);

	pos = targ.origin - newmis.origin;

	pos = normalize(pos);

	pos *= 800 + random(100);

	newmis.velocity = pos;
	newmis.angles = vectoangles(newmis.velocity);
	newmis.movedir = normalize(newmis.velocity);

	starteffect(CE_FLAMESTREAM, newmis.origin, '0 0 0', 0);

	newmis.touch = flameball_touch;
	newmis.think = flameball_think;

	thinktime newmis : 0.05;
}

void flamestream_boom()
{
	local vector rand;

	if (self.lifetime < time || self.lockentity.origin == '0 0 0')
	{
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

	self.think = flamestream_boom;
	thinktime self : 0.1;
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
	
		sound(self,CHAN_BODY,"succubus/flamend.wav",0.5,ATTN_NORM);
		
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
			if (random() < 0.5 || deathmatch)
				spawn_burner(other);
		}

		other_org=other.origin;
		old_health=other.health;
		starteffect(CE_FBOOM, self.origin-self.movedir*6,'0 0 0', 0);
		T_Damage(other,self,self.owner,self.dmg + random(1,5));
		if(other.health<=0&&old_health>0)
			smolder(other_org);
		
		remove(self);
	}
	else
	{
		sound(self,CHAN_BODY,"succubus/flamend.wav",0.5,ATTN_NORM);
		starteffect(CE_BOMB, self.origin-self.movedir*6,'0 0 0', 0);
		T_RadiusDamage(self,self.owner,40,self.owner);	
		remove(self);
	}
}

void flamestream_touch2 ()
{
	entity found,loser;
	float lastdist,dist;
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
	}
				

	if(loser.takedamage)
	{
		float old_health;
		vector loser_org;
		loser_org=loser.origin;
		old_health=loser.health;
		starteffect(CE_FBOOM, self.origin-self.movedir*6,'0 0 0', 0);
		T_Damage(other, self.owner, self.owner, 20);
		if(loser.health<=0&&old_health>0)
			smolder(loser_org);

		sound(self,CHAN_BODY,"succubus/flampow.wav",0.5,ATTN_NORM);

		self.lockentity = loser;
		self.lifetime = time + 3;
		
		self.solid = SOLID_NOT;
		self.think = flamestream_boom;
		thinktime self : 0.1;
		return;
	}
	else
	{
		sound(self,CHAN_BODY,"succubus/flampow.wav",0.5,ATTN_NORM);

		starteffect(CE_FBOOM, self.origin-self.movedir*6,'0 0 0', 0);
		T_RadiusDamage(self,self.owner,50+random(1,10),self.owner);	
		remove(self);
	}
}

void flamestream_anim()
{
	makevectors(self.angles);	
			
	float rand;
	float old_health;
	vector other_org;

	traceline(self.origin, self.origin + '0 0 -256', FALSE, self);

	if (trace_ent)
	{
		if (rand < 0.33)
			starteffect(CE_FIREWALL_SMALL, trace_endpos, '0 0 0', 0);
		else if (rand < 0.66)
			starteffect(CE_FIREWALL_MEDIUM, trace_endpos, '0 0 0', 0);
		else
			starteffect(CE_FIREWALL_LARGE, trace_endpos, '0 0 0', 0);

		if (trace_ent.thingtype == THINGTYPE_FLESH || 
			trace_ent.thingtype == THINGTYPE_WOOD || 
			trace_ent.thingtype == THINGTYPE_HAY ||
			trace_ent.thingtype == THINGTYPE_LEAVES ||
			trace_ent.thingtype == THINGTYPE_CLOTH ||
			trace_ent.thingtype == THINGTYPE_WOOD_LEAF ||
			trace_ent.thingtype == THINGTYPE_WOOD_METAL ||
			trace_ent.thingtype == THINGTYPE_WOOD_STONE ||
			trace_ent.thingtype == THINGTYPE_METAL_CLOTH)
		{
			if (random() < 0.5 || deathmatch)
				spawn_burner(trace_ent);
		}

		other_org=trace_ent.origin;
		old_health=trace_ent.health;
		if(trace_ent.health<=0&&old_health>0)
			smolder(other_org);		
	}
	else 
	{
		traceline(self.origin, self.origin + '0 0 -256', TRUE, self);

		if (trace_fraction < 1)
		{			
		
			trace_endpos += v_up * 24;

			rand = random();
			
			starteffect(CE_FLAMESTREAM, self.origin, randomv('-4 -4 -3', '4 4 2'), 0);
			starteffect(CE_FLAMESTREAM, self.origin, randomv('-4 -4 -3', '4 4 2'), 0);
			
			if (rand < 0.33)
				starteffect(CE_FIREWALL_SMALL, trace_endpos, '0 0 0', 0);
			else if (rand < 0.66)
				starteffect(CE_FIREWALL_MEDIUM, trace_endpos, '0 0 0', 0);
			else
				starteffect(CE_FIREWALL_LARGE, trace_endpos, '0 0 0', 0);
		}
		else
		{
			starteffect(CE_FLAMESTREAM, self.origin, randomv('-4 -4 -3', '4 4 2'), 0);
			starteffect(CE_FLAMESTREAM, self.origin, randomv('-4 -4 -3', '4 4 2'), 0);
		}
	}

	if (self.lifetime < time) self.touch();

	self.think = flamestream_anim;
	thinktime self : 0.05;
}

void flamestream_anim2()
{
	self.angles += '0 0 24';
	makevectors(self.angles);
	
	starteffect(CE_FLAMESTREAM, self.origin, v_right * 6, 0);
	starteffect(CE_FLAMESTREAM, self.origin, v_right * -6, 0);
	
	if (self.lifetime < time) self.touch();

	self.think = flamestream_anim2;
	thinktime self : 0.05;
}

void firestarter_think()
{
	local vector rand;

	if (self.lifetime < time)
	{
		self.think = SUB_Remove;
		thinktime self : 0;
		return;
	}

	rand = newmis.origin + randomv('-15 0 -2', '15 0 10');
	
	T_RadiusDamage(self,self.owner,30+random(10),self.owner);	
	
	if (random() < 0.5)
		starteffect(CE_FBOOM, rand,'0 0 4', 0);
	else
		starteffect(CE_SM_EXPLOSION, rand,'0 0 4', 0);

	self.think = firestarter_think;
	thinktime self : random(0.2, 0.6);
}

void flamestream_fire ()
{
	/*local vector endpos, startpos, vect;
	local float incr, dist, i;*/
		
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
	newmis.abslight=1;
	newmis.touch=flamestream_touch;
	newmis.dmg=40;
	newmis.lifetime=time+2;
	newmis.o_angle=self.origin+self.proj_ofs+v_forward*16-v_right*16-v_up*16;
	newmis.movedir=v_forward;

	newmis.speed=1500+random(50);
	newmis.velocity=newmis.movedir*newmis.speed;
	newmis.angles = vectoangles(newmis.velocity);
	setmodel(newmis,"models/null.spr");
	setsize(newmis,'-6 -6 -6','6 6 6');
	newmis.level=0;
	newmis.hull=HULL_POINT;
	setorigin(newmis,newmis.o_angle);
	newmis.wallspot=newmis.origin;
	newmis.count = 20;
	newmis.cnt = 0;
	newmis.effects (+) EF_DIMLIGHT;

	newmis.think=flamestream_anim;	
	thinktime newmis : 0.05;
	
	//Some would call it the Orb of... Tim?
	
	/*makevectors(self.v_angle);
	self.greenmana-=4;
	self.effects(+)EF_MUZZLEFLASH;

	endpos = v_forward * 99999;
	startpos = self.origin+self.proj_ofs+v_forward*16-v_right*24-v_up*16;

	traceline(startpos, endpos, FALSE, self);

	starteffect(CE_FBOOM, trace_endpos, '0 0 0', 0);
	
	vect = trace_endpos - startpos;
	
	dist = fabs(vlen(vect));
	
	vect = normalize(vect);
	vect = vectoangles(vect);

	vect_x *= -1;

	makevectors (vect);
	
	//starteffect(CE_FLAMESTREAM, startpos + (v_forward * 32), '0 0 2', 0);

	incr = dist / 6;
	i = 6;
	
	while (i > 0)
	{
		starteffect(CE_SLOW_WHITE_SMOKE, startpos + v_forward * (incr * i), '0 0 2', 0);
		i -= 1;
	}
	
	
	newmis = spawn();
	newmis.classname = "fire starter"; //Twisted fiyah-stahtah!
	setorigin(newmis, trace_endpos);
	setsize(newmis, '0 0 0', '0 0 0');
	newmis.solid = SOLID_NOT;
	newmis.movetype = MOVETYPE_NONE;

	newmis.lifetime = time + random(1,2);
	newmis.owner = self;
	newmis.think = firestarter_think;
	thinktime newmis : 0.1;

	if (trace_ent.takedamage == DAMAGE_YES)
	{
		T_Damage(trace_ent, self, self, 20);
	}*/
}


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
	newmis.touch=flamestream_touch2;
	//newmis.effects = EF_BRIGHTLIGHT;
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
	newmis.effects (+) EF_DIMLIGHT;
		
	newmis.think=flamestream_anim2;	
	thinktime newmis : 0.05;
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
	// Pa3PyX: rewrote the code for framerate independence
	local float advance_frames;
	local float cnt_frame;
	local float attackframe_passed;

	// Did the delay from previous attack expire yet?
	if ((time >= self.attack_finished) || (self.ltime > 0)) {
		if (self.ltime <= 0)
			self.ltime = time;
		// Animation loop factor
		advance_frames = rint(1.33 * (time - self.ltime) / HX_FRAME_TIME);
		if (advance_frames >= 1) {
			cnt_frame = 0;
			attackframe_passed = FALSE;
			if (self.artifact_active & ART_TOMEOFPOWER) {
				// Advance <advance_frames> frames
				while ((cnt_frame < advance_frames) && (self.wfs != WF_LAST_FRAME)) {
					self.wfs = advanceweaponframe($powern135, $powern147);
					self.weaponframe_cnt += 1;
					// Did we go over attack frame?
					if (self.weaponframe_cnt == 2) {
						attackframe_passed = TRUE;
					}
					cnt_frame += 1;
				}
				if (self.wfs == WF_LAST_FRAME) {
					// End of animation, clean up and exit
					self.wfs = WF_NORMAL_ADVANCE;
					self.weaponframe_cnt = 0;
					self.ltime = -1;
					// Total: 0.5 + 0.5 = 1.0 secs between
					// tomed shots
					self.attack_finished = time + 0.5;
					self.th_weapon = flameorb_ready_power;
				}
				else {
					self.ltime = time;
					self.th_weapon=flameorb_fire;
				}
			}
			else {
				while ((cnt_frame < advance_frames) && (self.wfs != WF_LAST_FRAME)) {
					self.wfs = advanceweaponframe($normal51, $normal63);
					self.weaponframe_cnt += 1;
					if (self.weaponframe_cnt == 2) {
						attackframe_passed = TRUE;
					}
					cnt_frame += 1;
				}
				if (self.wfs == WF_LAST_FRAME) {
					self.wfs = WF_NORMAL_ADVANCE;
					self.weaponframe_cnt=0;
					self.ltime = -1;
					// 0.5 secs between untomed fires
					self.attack_finished = time;
					self.th_weapon = flameorb_ready_normal;
				}
				else {
					self.ltime = time;
					self.th_weapon = flameorb_fire;
				}
			}
			// Out of mana?
			if((self.greenmana < 1) || ((self.greenmana < 10) && (self.artifact_active & ART_TOMEOFPOWER))) {
				self.wfs = WF_NORMAL_ADVANCE;
				self.weaponframe_cnt = 0;
				self.ltime = -1;
				self.attack_finished = time;
				if(!(self.artifact_active & ART_TOMEOFPOWER)) {
					self.th_weapon = flameorb_ready_normal;
				}	
				else {
					self.th_weapon = flameorb_ready_power;
				}
			}
			// Attack frame was encountered in frame advance --
			// perform attack
			else if (attackframe_passed && !(self.artifact_active & ART_TOMEOFPOWER)) {
				sound(self, CHAN_BODY, "succubus/flamstrt.wav", 0.5, ATTN_NORM);
				flamestream_fire();
			}
			else if (attackframe_passed && (self.artifact_active & ART_TOMEOFPOWER)) {
				sound(self,CHAN_BODY,"succubus/flamstrt.wav",0.5,ATTN_NORM);
				flameswarm_fire();
			}
		}
		else
			self.th_weapon = flameorb_fire;
	}
	else
		self.th_weapon = flameorb_fire;
	thinktime self: 0;

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
	// Pa3PyX
	self.ltime = -1;
	self.wfs = WF_NORMAL_ADVANCE;

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

