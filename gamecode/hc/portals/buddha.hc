/*
==============================================================================

C:\projects\h2mp\data1\bossbudda\newguy\pravus.hc

==============================================================================
*/

// For building the model
$cd C:\projects\h2mp\data1\bossbudda\newguy
$origin 0 0 0
$base base skin
$skin skin
$flags 0

//
$frame attakC1      attakC2      attakC3      attakC4      attakC5      
$frame attakC6      attakC7      attakC8      attakC9      attakC10     
$frame attakC11     attakC12     attakC13     attakC14     attakC15     
$frame attakC16     attakC17     attakC18     attakC19     attakC20     

//
$frame beamB1       beamB2       beamB3       beamB4       beamB5       
$frame beamB6       beamB7       beamB8       

//
$frame birth1       birth2       birth3       birth4       birth5       
$frame birth6       birth7       birth8       birth9       birth10      
$frame birth11      birth12      birth13      birth14      birth15      
$frame birth16      birth17      birth18      birth19      birth20      
$frame birth21      birth22      birth23      birth24      

//
$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      

//
$frame point1       point2       point3       point4       point5       
$frame point6       point7       point8       point9       point10      
$frame point11      point12      

//
$frame pravus       

//
$frame push1        push2        push3        push4        push5        
$frame push6        push7        push8        push9        push10       
$frame push11       push12       push13       push14       push15       
$frame push16       push17       push18       push19       push20       
$frame push21       push22       push23       push24       push25       
$frame push26       push27       push28       push29       push30       
$frame push31       push32       push33       push34       push35       
$frame push36       push37       push38       push39       push40       
$frame push41       push42       push43       push44       push45       
$frame push46       push47       push48       push49       push50       
$frame push51       push52       push53       push54       push55       
$frame push56       push57       push58       push59       push60       
$frame push61       push62       push63       push64       push65       
$frame push66       push67       push68       push69       push70       

//
$frame ready1       ready2       ready3       ready4       ready5       
$frame ready6       ready7       ready8       ready9       ready10      
$frame ready11      ready12      ready13      ready14      ready15      
$frame ready16      ready17      ready18      ready19      ready20      

//
$frame rechar1      rechar2      rechar3      rechar4      rechar5      
$frame rechar6      rechar7      rechar8      rechar9      rechar10     
$frame rechar11     rechar12     rechar13     rechar14     rechar15     
$frame rechar16     rechar17     rechar18     rechar19     rechar20     
$frame rechar21     rechar22     rechar23     rechar24     rechar25     
$frame rechar26     rechar27     rechar28     rechar29     rechar30     

//
$frame rewait1      rewait2      rewait3      rewait4      rewait5      
$frame rewait6      rewait7      rewait8      rewait9      rewait10     
$frame rewait11     rewait12     rewait13     rewait14     rewait15     
$frame rewait16     rewait17     rewait18     rewait19     rewait20     
$frame rewait21     rewait22     rewait23     rewait24     rewait25     
$frame rewait26     rewait27     rewait28     rewait29     rewait30     

//
$frame tele1        tele2        tele3        tele4        tele5        
$frame tele6        tele7        tele8        tele9        tele10       
$frame tele11       tele12       tele13       tele14       tele15       
$frame tele16       tele17       tele18       tele19       tele20       
$frame tele21       tele22       tele23       tele24       tele25       
$frame tele26       

float BUDDHA_PROJ1		= 1;
float BUDDHA_PROJ2		= 2;
float BUDDHA_TELE_IN	= 4;
float BUDDHA_TELE_OUT	= 8;
float BUDDHA_RECHARGE	= 16;
float BUDDHA_TELE_GONE	= 32;

float SHARD_TINT_UP		= 2;
float SHARD_TINT_DOWN	= 4;

void buddha_do_firewall(void);
void buddha_seek (void);
void buddha_run (void);
void buddha_get_new_target(void);

void buddha_pain(entity attacker, float damg)
{
	if (self.enemy.health < 0 || self.enemy == world)
		self.enemy = attacker;

	if (random() < 0.2)
		sound(self, CHAN_AUTO, "buddha/laugh.wav", 1, ATTN_NONE);
}

void buddha_warp(void)
{
	local entity spot;
	local float cyc;

	if (self.velocity)
		self.velocity = '0 0 0';

	spot = find (world, classname, "teleport_buddha");
	if (spot)
	{	
		cyc = random(3);
		while (cyc > 1)
		{
			spot = find (spot, classname, "teleport_buddha");
			
			if (spot.origin == self.oldorigin)
				spot = find (spot, classname, "teleport_buddha");
			
			if (!spot)
				spot = find (spot, classname, "teleport_buddha");
			cyc = cyc - 1;
		}
	}

	self.oldorigin = spot.origin;
	setorigin(self, spot.origin);
	setorigin(self.controller, spot.origin);
}

void buddha_recharge_done(void) [++ $rechar20 .. $rechar30]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $rechar30)
	{
		self.drawflags (-) MLS_POWERMODE;
		self.attack_state (-) BUDDHA_RECHARGE;
		self.takedamage = DAMAGE_YES;
		self.think = buddha_do_firewall;
		thinktime self : 0.05;
	}
}

void buddha_recharge_wait(void) [++ $rewait1 .. $rewait30]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $rewait1 && self.lifetime < time)
	{
		self.lifetime = time + 4;
		self.drawflags (+) MLS_POWERMODE;
		starteffect(44, self.origin + '0 0 -20', 128, time + 3);
	}

	if (self.frame == $rewait30 && self.lifetime < time)
	{
		self.think = buddha_recharge_done;
		thinktime self : 0.05;
	}
}

void buddha_recharge(void) [++ $rechar1 .. $rechar20]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $rechar1)
	{
		sound(self, CHAN_AUTO, "buddha/recharge.wav", 1, ATTN_NONE);
		self.takedamage = DAMAGE_NO;
	}

	if (self.frame == $rechar20)
	{
		self.think = buddha_recharge_wait;
		thinktime self : 0.05;
	}
}

/*void buddha_tele_out(void) [-- $tele26 .. $tele1]
{
	if (self.velocity)
		self.velocity = '0 0 0';
	
	dprint("I'm not even getting here!\n");

	if (self.frame == $tele25)
	{
		dprint("Hi\n");
		sound(self,CHAN_VOICE,"buddha/tele_out.wav",1,ATTN_NONE);
	}
	
	if (self.frame == $tele1)
	{
		self.think = buddha_run;
		thinktime self : 0.1;
	}
}*/

void buddha_spindown(void)
{
	local vector vect;

	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame < $birth24)
		self.frame += 1;

	if (self.count <= 0)
	{
		vect = self.enemy.origin - self.origin;
		normalize(vect);
		self.ideal_yaw = vectoyaw(vect);

		if (FacingIdeal()) 
		{
			self.avelocity = '0 0 0';
			self.takedamage = DAMAGE_YES;
			self.solid = SOLID_BBOX;

			self.attack_state (-) BUDDHA_TELE_OUT;
			self.think = buddha_run;
			thinktime self : 0.1;
		}
		else
		{	
			self.think = buddha_spindown;
			thinktime self : 0.05;
		}

		return;
	}

	if (self.avelocity_y - self.count <= 150)
	{
		self.avelocity = '0 150 0';
		self.count = 0;
	}
	else
	{
		self.avelocity_y -= self.count;
		self.count -= 2.5;
	}

	self.think = buddha_spindown;
	thinktime self : 0.05;
}

void buddha_grow(void)
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.scale < 1)
	{
		self.scale += 0.1;

		if (self.scale < 0.2)
		{
			sound(self,CHAN_AUTO,"buddha/tele_out.wav",1,ATTN_NONE);
	
			self.attack_state (-) BUDDHA_TELE_GONE;
			self.attack_state (+) BUDDHA_TELE_OUT;
			self.frame = $birth15;
			setmodel(self, "models/pravus.mdl");

			starteffect(CE_FLOOR_EXPLOSION3, self.origin);
			particle4(self.origin,25,256+242+random(1,16),PARTICLETYPE_SLOWGRAV,random(35,40));
		}
	}
	else
	{		
		self.drawflags (-) DRF_TRANSLUCENT;
		self.think = buddha_spindown;
		thinktime self : 0.1;

		return;
	}

	self.think = buddha_grow;
	thinktime self : 0.05;
}

void buddha_shrink(void)
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.scale > 0.1)
	{
		self.scale -= 0.1;

		if (self.scale < 0.3 && self.scale > 0.2)
		{
			sound(self,CHAN_AUTO,"weapons/fbfire.wav",1,ATTN_NONE);
			starteffect(CE_FLOOR_EXPLOSION3, self.origin);
		}
	}
	else
	{
		self.frame = 0;
		setmodel (self, "models/null.spr");	
		buddha_warp();
		self.think = buddha_grow;
		

		self.attack_state (+) BUDDHA_TELE_GONE;
		self.attack_state (-) BUDDHA_TELE_IN;

		thinktime self : random(2,3);
		return;
	}

	self.think = buddha_shrink;
	thinktime self : 0.1;
}

void buddha_teleport(void) [++ $tele1 .. $tele26]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $tele1)
	{
		sound(self,CHAN_AUTO,"buddha/teleport.wav",1,ATTN_NONE);
		self.teleport_time = time + 10;
		self.takedamage = DAMAGE_NO;
		starteffect(CE_GRAVITYWELL, self.origin + '0 0 -20', random(208, 223), time + 0.8);
	}

	if (self.frame == $tele26)
	{
		self.drawflags (+) DRF_TRANSLUCENT;
				
		self.think = buddha_shrink;
		thinktime self : 0.1;
		return;
	}
	else
	{
		self.avelocity_y += self.count;
		self.count += 2.5;
	}
}

void firewall_touch(void)
{
	T_Damage(other, self, self.owner, (skill + 1));

	self.think = SUB_Remove;
	thinktime self : 0;
}

void firewalker_check_damage(void)
{
	local vector dif;
	local float  dist;
	local float  distx, disty;
	local float objx, objy;

	makevectors(self.angles);

	dif = self.enemy.origin - (self.origin + v_forward * -64);
	dif_z = 0;

	dist = vhlen(dif);

	dif = normalize(dif);

	objx = dif * v_forward;

	if (objx < 0) return;

	objy = dif * v_right;
	
	distx = fabs(objx * dist);
	disty = fabs(objy * dist);

	if (distx < 128 && disty < 200)
	{
		if (self.enemy.health > 0)
			T_Damage(self.enemy, self, self.owner, (skill + 1));
	}
}

void firewalker_think(void)
{
	local vector vect, dif;
	local float  dist, rnd1, rnd2, rnd3;

	if (self.lifetime < time)
	{
		self.think = SUB_Remove;
		thinktime self : 0.1;
		return;
	}

	makevectors(self.angles);
	
	vect = '0 0 0';

	if (random() < 0.9)
	{
		rnd1 = random(0, 200);
		rnd2 = random(0, 10);
		rnd3 = random(0, 40);

		if (random() < 0.5)
		{
			vect += v_forward * rnd2;	
		}
		else
		{
			vect -= v_forward * rnd2;	
		}

		if (random() < 0.5)
		{
			vect += v_right * rnd1;	
		}
		else
		{
			vect -= v_right * rnd1;	
		}

		vect_z = 10;
		
		if (random() < 0.33)
			starteffect(CE_FIREWALL_LARGE, self.origin + vect, '0 0 1', 0);
		else if (random() < 0.66)
			starteffect(CE_FIREWALL_MEDIUM , self.origin + vect, '0 0 1', 0);
		else
			starteffect(CE_FLAMEWALL , self.origin + vect, '0 0 1', 0);
	}
	
	vect = v_up * -10000;
	traceline(self.origin, vect, FALSE, self);

	dif = self.origin - trace_endpos;
	dist = dif_z;
	
	if (dist > 20)
	{
		self.velocity_z -= 8;	
	}
	else
	{
		self.velocity_z = 0;
	}

	self.think = firewalker_think;
	thinktime self : 0.05;
}

void buddha_firewalker_spawn(void)
{
	local entity newmis;

	newmis = spawn();

	newmis.classname = "buddha_firewalker";
	newmis.effects = EF_NODRAW;
	newmis.movetype = MOVETYPE_FLY;
	newmis.solid = SOLID_TRIGGER;

	newmis.safe_time = time + 9999999;
	newmis.velocity = '0 0 0';
	newmis.owner = self;

	setmodel(newmis, "models/null.spr");
	setsize(newmis, '-100 -100 -100', '100 100 100');
	newmis.angles = self.angles;

	newmis.enemy = self.enemy;

	makevectors(newmis.angles);
	setorigin(newmis, self.origin + v_up * -48 + v_forward * 64);
	newmis.velocity = v_forward * (125 + (skill * 25));

	newmis.monsterclass = CLASS_BOSS;
	newmis.touch = firewalker_check_damage;

	newmis.lifetime = time + random(5,10);
	newmis.think = firewalker_think;
	thinktime newmis : 0.05;
}

void buddha_send_firewall(void) [++ $push33 .. $push70]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $push34)
	{
		buddha_firewalker_spawn();
	}
	else if (self.frame == $push70)
	{
		self.think = buddha_run;
		thinktime self : 0.1;
	}
}

void firepillar_think(void)
{
	local vector rand;
	local entity item;

	if (self.lifetime < time)
	{
		self.think = SUB_Remove;
		thinktime self : 0.1;
		return;
	}

	item = findradius(self.origin, 64);
	while (item)
	{
		if (item.classname == "player")
		{
			T_Damage(item, self, self.owner, 2);
		}

		item = item.chain;
	}

	if (random() < 0.5)
		rand = self.origin + randomv('0 0 32','32 32 64');
	else
		rand = self.origin - randomv('0 0 32','32 32 64');

	starteffect(CE_FLAMEWALL, rand, randomv('-1 -1 1', '1 1 2'), 0);

	self.think = firepillar_think;
	thinktime self : 0.1;
}

void buddha_spawn_firepillar(void)
{
	newmis = spawn();

	newmis.classname = "buddha_firepillar";
	
	setmodel(newmis, "models/null.spr");
	setsize(newmis, '0 0 0', '0 0 0');

	newmis.safe_time = time + 9999999;
	newmis.movetype = MOVETYPE_FLY;
	newmis.solid = SOLID_NOT;
	newmis.monsterclass = CLASS_BOSS;
	newmis.owner = self;

	makevectors(self.enemy.angles);
	
	vector ofs;
	
	ofs = v_forward * random(32, 64);

	traceline(self.enemy.origin, self.enemy.origin + '0 0 -9999', TRUE, self);

	starteffect(CE_FLOOR_EXPLOSION3, trace_endpos + ofs + '0 0 24');

	setorigin(newmis, trace_endpos);

	newmis.velocity = '0 0 100';

	newmis.lifetime = time + 2.5;
	
	sound (self, CHAN_AUTO, "weapons/fbfire.wav", 1, ATTN_NORM);

	newmis.think = firepillar_think;
	thinktime newmis : 0.2;
}

void buddha_do_firepillar(void) [++ $push1 .. $push30]
{
	local float rand;

	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.count == 0)
	{
		self.count = random(0,0.3) * 10;
	}

	ai_face();

	if (self.frame == $push4)
	{
		self.count -= 1;
		
		rand = random(1,2);
		while (rand > 0)
		{
			buddha_spawn_firepillar();
			rand -= 1;
		}
	}
	else if (self.frame == $push30)
	{
		if (self.count <= 0)
		{
			self.think = buddha_run;
			thinktime self : 0.05;
			return;
		}
	}
}

void buddha_do_firewall(void) [++ $push1 .. $push33]
{
	ai_face();
	
	if (self.velocity)
		self.velocity = '0 0 0';

	local vector vect;

	makevectors(self.angles);

	vect = self.origin + v_right * random(-200, 200) + v_forward * 32;

	particle2(vect,'0 0 25','0 0 25',256 + 168 + random(1,4),7,5);
	
	if (self.frame == $push1)
		sound(self, CHAN_AUTO, "buddha/firewall.wav", 1, ATTN_NONE);

	if (self.frame == $push33)
	{
		self.think = buddha_send_firewall;
		thinktime self : 0.05;
		return;
	}

	self.think = buddha_do_firewall;
	thinktime self : 0.05;
}

void buddha_proj_touch(void)
{
	if (other.owner == self.owner) 
		return;
	else
		T_Damage(other, self, self.owner, 10*random(2,3));

	CreateBSpark (self.origin); 

	remove(self);
}

void buddha_missile_think(void)
{
	if (self.lifetime < time)
	{
		self.touch();
	}
	
	HomeThink();	

	self.think = buddha_missile_think;
	thinktime self : 0.1;
}

void buddha_create_missile(vector spot1, vector spot2, float ofs, float homing)
{
	local vector vect;

	newmis = spawn();
	newmis.classname = "buddha_missile";

	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;

	newmis.drawflags (+) DRF_TRANSLUCENT;

	makevectors(self.angles);

	setmodel(newmis, "models/budd_star.mdl");
	setsize(newmis, '0 0 0', '0 0 0');
	setorigin(newmis, self.origin + v_forward * 164);
	
	newmis.owner = self;
	newmis.enemy = newmis.goalentity = self.enemy;
	newmis.yaw_speed = 3;
	
	newmis.hull = HULL_POINT;

	makevectors(self.angles);
	
	vect = (spot2 - spot1);

	vect = normalize(vect);

	vect += v_right * ofs;

	makevectors(vect);
	vect *= 400;

	newmis.velocity = vect;
	newmis.angles = vectoangles(newmis.velocity);

	newmis.speed = 400;

	newmis.drawflags (+) MLS_POWERMODE;

	newmis.veer=1;				//slight veering, random course modifications
	newmis.turn_time = 0.2;
	newmis.hoverz=TRUE;			//slow down on turns
	newmis.ideal_yaw=TRUE;

	newmis.lifetime = time + 5;
	newmis.touch = buddha_proj_touch;
	
	newmis.think = buddha_missile_think;

	thinktime newmis : 0.1;
}

void buddha_launch_proj(void)
{
	local vector spot1, spot2;

	if (self.velocity)
		self.velocity = '0 0 0';

	makevectors(self.angles);

	spot1 = self.origin + v_forward * 256;
	spot2 = self.enemy.origin + self.enemy.view_ofs;

	sound (self, CHAN_WEAPON, "buddha/shoot.wav", 1, ATTN_NORM);
	
	buddha_create_missile(spot1, spot2, self.dflags, FALSE);
}

void buddha_do_point(void) [++ $point1 .. $point12]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	ai_face();

	if (self.frame == $point1)
		self.dflags = -0.45;

	if (self.frame == $point2)
	{
		buddha_launch_proj();
		self.dflags = 0;
	}
	if (self.frame == $point6)
	{
		buddha_launch_proj();
		self.dflags = 0.45;
	}
	if (self.frame == $point9)
		buddha_launch_proj();

	if (self.frame == $point12)
	{
		self.think = buddha_run;
		thinktime self : 0.1;
	}
}

void buddha_end_lightning(void)[++ $attakC13 .. $attakC20]
{
	self.frags += 1;
	
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frags > 16) 
		self.frags = 16;

	self.colormap = 143 + self.frags;

	if (self.frame == $attakC14)
		self.drawflags (-) DRF_TRANSLUCENT | MLS_POWERMODE;

	if (self.frame == $attakC20)
	{		
		self.frags = 16;
		self.colormap = 0;
		
		self.think = buddha_run;
		thinktime self : 0.05;
	}
}

void buddha_bring_lightning(void)
{
	local entity oself;

	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.wait < time)
	{
		self.think = buddha_end_lightning;
		thinktime self : 0.05;
		return;
	}

	if (random() < 0.7)
	{
		oself=self;
		self=self.lockentity;
		self.use();
		self=oself;
	}

	self.think = buddha_bring_lightning;
	thinktime self : 0.05;
}

void buddha_do_conjure(void) [++ $attakC1 .. $attakC13]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $attakC13)
	{
		self.drawflags (+) MLS_POWERMODE;

		self.wait = time + random(2,4);
		self.think = buddha_bring_lightning;
		thinktime self : 0.05;
	}

	self.frags -= 1;

	if (self.frags < 12)
		self.frags = 12;

	self.colormap = 143 + self.frags;
}

float buddha_decide_attack (void)
{
	local float rand;
	
	if (self.velocity)
		self.velocity = '0 0 0';

	rand = random();

	if (rand > 0.9)
	{
		if (self.teleport_time < time)
		{
			self.attack_state (+) BUDDHA_TELE_IN;

			self.think = buddha_teleport;
			thinktime self : 0.1;
			return 1;
		}
		else
			return 1;
	}
	else if (rand > 0.7)
	{
		self.attack_state (+) BUDDHA_PROJ1;

		self.think = buddha_do_point;
		thinktime self : 0.1;

		return 1;
	}
	else if (rand > 0.5)
	{
		self.attack_state (+) BUDDHA_PROJ2;

		self.think = buddha_do_firepillar;
		thinktime self : 0.1;

		return 1;
	}
	else if (rand > 0.3)
	{
		self.attack_state (+) BUDDHA_PROJ1;

		self.think = buddha_do_firewall;
		thinktime self : 0.1;

		return 1;
	}
	else if (rand > 0.2)
	{
		self.attack_state (+) BUDDHA_PROJ2;

		self.think = buddha_do_conjure;
		thinktime self : 0.1;

		return 1;
	}
	else return 0;
}

void buddha_run (void) [++ $ready1 .. $ready20]
{
	local float ret;

	if (self.velocity)
		self.velocity = '0 0 0';

	self.goalentity = self.enemy;

	if (random() < 0.2)
	{
		if (self.camera_time < time)
		{
			if (random() < 0.5)
				sound(self, CHAN_AUTO, "buddha/random1.wav", 1, ATTN_NORM);
			else
				sound(self, CHAN_AUTO, "buddha/random2.wav", 1, ATTN_NORM);

			self.camera_time = time + 5;
		}
	}

	if (self.enemy.health <= 0 || self.enemy == world || !self.enemy.flags2 & FL_ALIVE)
	{
		if (self.oldenemy.health <=0 || self.oldenemy == world || !self.oldenemy.flags2 & FL_ALIVE)
		{
			self.enemy = world;
			self.think = buddha_get_new_target;
			thinktime self : 0.1;
			return;
		}
		else
		{
			self.enemy = self.oldenemy;
		}
	}

	if (self.health < self.max_health * (1 / 5) && self.dmgtime == 3)
	{
		self.dmgtime+=1;
		SUB_UseTargets();
	//	self.target = self.netname; // see notes by Thomas Freundt below
		self.think = buddha_recharge;
		thinktime self : 0.1;
		return;
	}
	else if (self.health < self.max_health * (2 / 5) && self.dmgtime == 2)
	{
		self.dmgtime+=1;
		SUB_UseTargets();
		self.target = self.targetname;
		self.think = buddha_recharge;
		thinktime self : 0.1;
		return;
	}
	else if (self.health < self.max_health * (3 / 5) && self.dmgtime == 1)
	{
		self.dmgtime+=1;
		self.think = buddha_recharge;
		thinktime self : 0.1;
		return;
	}
	else if (self.health < self.max_health * (4 / 5) && !self.dmgtime)
	{
		self.think = buddha_recharge;
		self.dmgtime+=1;
		thinktime self : 0.1;
		return;
	}

	ai_face();

	if (self.attack_finished < time)
		ret = buddha_decide_attack();
	
	if (!ret) 
		self.attack_finished = time + rint(random(6 / (skill) + 0.1));
	else
		return;

	self.think = buddha_run;
	thinktime self : 0.05;
}

void buddha_get_new_target(void)[++ $ready1 .. $ready20]
{
	entity enemy_proj;

	enemy_proj = look_projectiles();

	if (enemy_proj != world)
		self.enemy = enemy_proj.owner;

	if (!self.enemy.flags2&FL_ALIVE||(self.enemy.artifact_active&ARTFLAG_STONED&&self.classname!="monster_medusa"))
	{
		self.enemy = world;
		if (self.oldenemy.health > 0 && self.oldenemy != world)
		{
			self.enemy = self.oldenemy;
			self.think = buddha_run;
			thinktime self : 0.05;
			return;
		}
		else if(coop)
		{
			if(!FindTarget(TRUE))	//Look for other enemies in the area
			{
				self.think = buddha_run;
				thinktime self : 0.05;
				return;
			}
		}
	}
	else
	{
		self.oldenemy = self.enemy;
		self.think = buddha_run;
		thinktime self : 0.05;
		return;		
	}
}

/*
buddha_die() cleaned up and fixed by Thomas Freundt:

From: "Ozkan Sezer" <sezeroz@gmail.com>
[...]
While browsing the forums, I ran into this bug report:
http://quakeone.com/forums/quake-talk/other-games/5783-hexen-ii-community-work-discussion-59.html
... and I actually reproduced it.  On the console, do

playerclass 2  (to make it crusader)
map tibet10
impulse 43  (to get everything)
god

... then go and face praevus. Always use the ice mace in tome
of power mode and always keep a summoned imp around,
and then spam praevus with a lot of blizzard.

Result: the finale screen doesn't trigger.

[...]

From: "Thomas Freundt" <thf_ebay@yahoo.de>
To: "Ozkan Sezer" <sezeroz@gmail.com>
Subject: Re: praevus bug??
Date: Mon, 18 Apr 2011 09:07:02 +0200

Hi Ozkan

I think I found the reason for the bug. The following entity is assigned
to Praevus:

{
"classname" "monster_buddha"
"target" "t22"
"targetname" "t21"
"netname" "t51"
"angle" "0"
"origin" "-2720 1248 1456"
}

In 'buddha.hc' there is a function called 'buddha_run':

 if (self.health < self.max_health * (1 / 5) && self.dmgtime == 3)
 {
  self.dmgtime+=1;
  SUB_UseTargets();
  self.target = self.netname;
  self.think = buddha_recharge;
  thinktime self : 0.1;
  return;
 }
 else if (self.health < self.max_health * (2 / 5) && self.dmgtime == 2)
 {
  self.dmgtime+=1;
  SUB_UseTargets();
  self.target = self.targetname;
  self.think = buddha_recharge;
  thinktime self : 0.1;
  return;
 }

When Praevus' health drops to less than 40% SUB_UseTargets() fires
the standard "target" being "t22" which leads to the destruction of
the first pillar; then self.target is latched to self.targetname
which resolves to "target" "t21". Hence, when health drops to less
than 20% the second pillar is destroyed and self.target is latched
to self.netname which is equivalent to "target" "t51". When Praevus
dies 'monster_death_use' issues another call to SUB_UseTargets()
which activates the trigger_once entity with "targetname" "51" and
thereby initiates the final sequence with the flying skulls:

{
"classname" "trigger_once"
"target" "t52"
"targetname" "t51"
"delay" "6.5"
"model" "*13"
}

This is how it should work. However, only one function of Praevus'
behavioral repertoire can be active at a time and inserting debug
messages into 'buddha_run' reveals that this function is only active
during certain time slots; in the meantime Praevus can take considerable
damage or even die. This is nicely demonstrated when one quickly drains
his health level to near-death, then ceases fire, and only watches the
scenery. The lag causes the first pillar to fall asunder, invoke a
recharge sequence, followed by the second pillar being destroyed and
yet another recharge sequence.

If Praevus dies without execution of the <40% health if clause, the
default "target" "t22" is triggered by 'monster_death_use'; in other
words, the first pillar is always destroyed. The assignment
'self.target = self.netname' is also found in 'buddha_die', but this
function is called *after* 'monster_death_use' so the flying skulls
sequence never triggers.

To fix this bug one only has to make another call to SUB_UseTargets()
before Praevus' entity removes itself in 'buddha_die':

doWhiteFlash();
SUB_UseTargets();
remove(self);

Better still we remove 'self.target = self.targetname' from 'buddha_run'
not only to ascertain the function is called only once (calling it twice
shouldn't display any ill side effects, though; then again, I prefer not
to rely on assumptions :-), but also to have more accurate control of
timing. The default delay of 6.5s is now too long. To remedy this issue
we can either reduce delay to <1s by altering tibet10.ent or call
SUB_UseTargets about 6s before Praevus is removed:

if (self.count >= 4 && self.count < 4.1)
    SUB_UseTargets();

 if (self.count >= 10) 
 { ...

(There seems to be some rounding up with floating point, so
'if (self.count == 4)' is never executed)

~
ThF

[...]

After having completed the MP by defeating by far the toughest
adversary, waiting more than 6s without anything happening seems
somewhat irritating to me. For obvious reasons altering the entity
file isn't really an option.

As soon as 'buddha_die' sets self.count to 0, the function is called
another 100 times with 0.1s intervals. So the perfect point in time
to trigger the final sequence with its inherent 6.5s delay would be
when self.count is 3.5. I now use a prog.dat that has both original
'self.target = self.netname' statements commented out and instead
inserted

 if (self.count >= 3.5 && self.target != self.netname)
 {
   self.target = self.netname;
   SUB_UseTargets();
 }

before the statement originally at line 1087

if (self.count >= 10)
{ ...

This gets rid of floating point uncertainty and ensures that it is
only activated once with the right timing. I have tested this many
times, it always worked.

[...]

> I see. To confirm, it also is fine if one plays "decently", ie.
> without the Crusader's ice spam, yes?

The only error that can still occur with this amendment is that the
second pillar never breaks; but I find this rather unimportant.

> And do you mean that if you do the above changes, the SUB_UseTargets()
> call just after doWhiteFlash() is not needed?

Exactly.

> BTW, the if (self.count >= 10) {...}  construct contains some weird
> unnecessary if checks, such as the if (self.count == 3) check, as
> well as if (self.count == 0) check after the self.count = 0
> assignment.  I guess they can use a good cleanup while we are there.

I think so, too. It looks like a duplicate pasted mistakenly.
*/

void buddha_die (void)[++ $death1 .. $death14]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	entity beam;
	entity save;
	entity found;
	vector new_origin;

//	self.target = self.netname;

	if (self.think != buddha_die)
	{
		self.think = buddha_die;
		self.count = 0;
		self.cnt = 0;
		thinktime self : HX_FRAME_TIME;
		self.rider_death_speed = 0.2;

		return;
	}

	if (self.cnt == 0)
	{
		sound(self, CHAN_VOICE, "buddha/die.wav", 1, ATTN_NONE);
		found=find(world,classname,"player");
		while(found)
		{//Give them all the exp
			AwardExperience(found,self,self.experience_value);
			found=find(found,classname,"player");
		}
		self.experience_value=	self.init_exp_val = FALSE;
		self.drawflags = self.drawflags | MLS_ABSLIGHT;
		self.abslight = 3;
		/*if(self.noise)
			sound(self,CHAN_VOICE,self.noise,1,ATTN_NONE);*/
		self.movetype=MOVETYPE_NONE;
		self.velocity='0 0 0';
	}

	if (self.count >= 3.5 && self.target != self.netname)
	{
		self.target = self.netname;
		SUB_UseTargets();
	}

	//thinktime self : self.rider_death_speed;
	self.rider_death_speed += 0.1;

	if (self.count >= 10)
	{
		self.count = 1;
		self.effects (-) EF_BRIGHTLIGHT;
		self.effects (+) EF_NODRAW;
		if (self.movechain != world)
			self.movechain.effects (+) EF_NODRAW;

		//thinktime self : 0.05;

		entity search;

		search = find(world, classname, "rider_temp");
		while (search != world)
		{
			remove(search);
			search = find(search, classname, "rider_temp");
		}

		doWhiteFlash();
		remove(self);

		return;
	}
	else
		self.effects = EF_BRIGHTLIGHT;

	self.count += 0.1;

	if (self.cnt < 8)
	{
		if (self.count == 3)
		{
			beam = spawn();

			new_origin = self.origin + '0 0 50';

			setmodel(beam,"models/boss/circle.mdl");
			setorigin(beam,new_origin);

			setsize (beam, '0 0 0', '0 0 0');
			beam.owner = self;
			beam.movetype = MOVETYPE_FLYMISSILE;
			beam.solid = SOLID_NOT;
			beam.drawflags = SCALE_TYPE_UNIFORM;
			beam.scale = .1;
			beam.skin = 0;
			beam.avelocity = '0 0 300';
			beam.think = circle_think;
			thinktime beam : HX_FRAME_TIME;
			//self.count = 13;
		}

		beam = spawn();

		makevectors(self.angles);
	//	new_origin = v_factorrange('-3 -25 45', '3 25 50') + self.origin;
		new_origin = self.origin;// + '0 0 50';

		setmodel(beam,"models/boss/shaft.mdl");
		setorigin(beam,new_origin);

		setsize (beam, '0 0 0', '0 0 0');
		beam.owner = self;
		beam.drawflags = SCALE_ORIGIN_BOTTOM | SCALE_TYPE_XYONLY;
		beam.movetype = MOVETYPE_NOCLIP;
		beam.solid = SOLID_NOT;
		beam.think = beam_move;
		beam.angles = '0 0 0';
		beam.angles_x = random(-50,50);
		beam.angles_z = random(-50,50);
		beam.beam_angle_a = random(360);
		beam.beam_angle_b = random(20,130);
		beam.scale = .1;
		beam.beam_max_scale = random(.5,1.5);
		beam.classname = "rider_temp";
		if (random() > 0.5) 
			beam.beam_direction = 1;

		beam.beam_speed = random(2,4.5);

		save = self;
		self = beam;
		beam_move();
		self = save;
		self.cnt += 1;
	}
}

void buddha_birth (void);

void buddha_wait(void)
{
	if (self.velocity)
		self.velocity = '0 0 0';
	
	if (self.lifetime < time)
	{
		self.think = buddha_birth;
		thinktime self : 0.1;
		return;
	}

	self.think = buddha_wait;
	thinktime self : 0.1;
}

void buddha_birth (void) [++ $birth1 .. $birth24]
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if (self.frame == $birth16)
	{
		sound(self, CHAN_VOICE, "buddha/sight.wav", 1, ATTN_NONE);
		
		self.lifetime = time + random(1,2);
		self.think = buddha_wait;
		thinktime self : 0.1;
		return;
	}

	if (self.frame > $birth16)
	{
		self.origin_z += 4;
	}

	if (self.frame == $birth24)
	{
		self.think = buddha_run;
		thinktime self : 0.05;
	}
}

void buddha_seek (void)
{
	if (self.velocity)
		self.velocity = '0 0 0';

	self.frame = $birth1;

	entity enemy_proj;

	enemy_proj=look_projectiles();

	if (FindTarget(TRUE) || (enemy_proj != world) ) 
	{
		self.think = buddha_birth;
		thinktime self : 0.05;
		return;
	}
	
	
	self.think = buddha_seek;
	thinktime self : 0.1;
}

void shard_think(void)
{
	if (self.velocity)
		self.velocity = '0 0 0';

	if ((self.frame == 0 || self.frame == 11 || self.frame == 22) && !self.attack_state & 1)
	{
		self.lifetime = time + 1;
		self.attack_state (+) 1;
	}
	
	if (self.lifetime <= time)
	{
		self.frame += 1;
		
		if (self.frame > 32)
			self.frame = 0;

		self.attack_state (-) 1;
	}

	if (self.owner.attack_state & BUDDHA_TELE_GONE)
	{
		self.effects = EF_NODRAW;
	}
	else
	{
		self.effects = 0;
	}

	local vector vect;
	vect = self.controller.angles;
	vect_y += self.count;

	makevectors(vect);
	setorigin(self, self.controller.origin + v_forward * (self.cnt - self.strength));

	vect = self.origin - self.controller.origin;

	self.angles = vectoangles(vect);

	if ((self.attack_finished & 2) && self.colormap > 16) 
		self.colormap -= 0.05;
	else
	{
		self.colormap = 0;
		self.attack_state (-) 2;
	}

	if (self.owner.attack_state & BUDDHA_TELE_IN)
	{
		if (self.scale > 0.1)
		{
			self.scale -= 0.02;		
			if (self.strength < 120)
				self.strength += 3;
		}
		else
		{
			self.strength = 120;
		}
	}
	else if (self.owner.attack_state & BUDDHA_TELE_OUT)
	{		
		if (self.strength >= 0)
		{
			self.strength -= 4;
		}
		else
		{	
			self.strength = 0;
		}

		if (self.scale <= 1)
		{
			if (self.solid == SOLID_NOT)
			{
				setmodel(self, "models/shield.mdl");
				self.solid = SOLID_PHASE;
				self.scale = 0.1;
			}
			self.scale += 0.05;			
		}
	}
	else
	{
		self.scale = 1;
	}
	self.think = shard_think;
	thinktime self : 0.05;
}

void buddha_shard_touch(void)
{
	self.colormap = 32;	
	self.attack_state (+) 2;
}

void buddha_add_shard(float ofs, float dist, float sframe)
{
	//local vector vect;

	newmis = spawn();

	newmis.movetype = MOVETYPE_NOCLIP;
	newmis.solid = SOLID_PHASE;
	
	newmis.classname = "buddha_shield";
	
	newmis.angles_y += ofs;

	makevectors(newmis.angles);

	newmis.touch = buddha_shard_touch;
	newmis.owner = self.owner;
	newmis.controller = self;
	newmis.drawflags (+) MLS_ABSLIGHT;

	newmis.monsterclass = CLASS_BOSS;
	newmis.lifetime = 0;
	newmis.abslight = 1;
	newmis.cnt = dist;
	newmis.count = ofs;
	
	newmis.safe_time = time + 9999999;
	newmis.colormap = 0;
	newmis.takedamage = DAMAGE_YES;
	newmis.health = 500 + ((skill) * 200);
	newmis.th_die = SUB_Remove;

	newmis.drawflags (+) DRF_TRANSLUCENT;
	setmodel(newmis, "models/shield.mdl");
	setsize(newmis, '-32 -32 -64', '32 32 64');
	setorigin(newmis, self.origin + v_forward * dist);

	newmis.strength = 0;

	newmis.frame = sframe;
	newmis.scale = 1;
	newmis.drawflags (+) SCALE_ORIGIN_BOTTOM;

	newmis.think = shard_think;
	thinktime newmis : 0.1;
}

void shield_think(void)
{
	setorigin(self, self.owner.origin);
	
	self.think = shield_think;
	thinktime self : 0.1;
}

void shield_init(void)
{
	local float ang, incr;
	
	incr = 360 / 6;
	
	ang += incr;
	buddha_add_shard(ang, 125, 0);

	ang += incr;
	buddha_add_shard(ang, 125, 0);

	ang += incr;
	buddha_add_shard(ang, 125, 0);

	ang += incr;
	buddha_add_shard(ang, 125, 0);
	
	ang += incr;
	buddha_add_shard(ang, 125, 0);
	
	ang += incr;
	buddha_add_shard(ang, 125, 0);

	self.think = shield_think;
	thinktime self : 0.1;
}

void buddha_spawn_shield(void)
{
	if (self.velocity)
		self.velocity = '0 0 0';

	newmis = spawn();
	newmis.classname = "shield_hub";

	self.controller = newmis;

	newmis.effects = EF_NODRAW;
	newmis.movetype = MOVETYPE_NOCLIP;
	newmis.solid = SOLID_NOT;
	newmis.takedamage = DAMAGE_NO;

	setsize(newmis, '0 0 0', '0 0 0');

	newmis.owner = self;
	newmis.avelocity = '0 100 0';

	setorigin(newmis, self.origin);

	newmis.think = shield_init;
	thinktime newmis : 0;
}

void buddha_find_lightning ()
{
	local entity found;

	self.max_health = self.health = 5000 + ((skill) * 1000);

	found = find(world,classname,"light_thunderstorm");
	
	if(found)
	{
		self.lockentity=found;
		found.lockentity=self;
	}

	self.think = buddha_seek;
	thinktime self : 0.1;
}

void buddha_trigger_use(void)
{
		intermission_running = 1;

	intermission_exittime = time + 99999;

	//Remove cross-level trigger server flags for next hub
	serverflags(-)(SFL_CROSS_TRIGGER_1|
				SFL_CROSS_TRIGGER_2|
				SFL_CROSS_TRIGGER_3|
				SFL_CROSS_TRIGGER_4|
				SFL_CROSS_TRIGGER_5|
				SFL_CROSS_TRIGGER_6|
				SFL_CROSS_TRIGGER_7|
				SFL_CROSS_TRIGGER_8);

	entity search;

	search=find(world,classname,"player");
	while(search)
	{//Take away all their goodies
		search.puzzle_inv1 = string_null;
		search.puzzle_inv2 = string_null;
		search.puzzle_inv3 = string_null;
		search.puzzle_inv4 = string_null;
		search.puzzle_inv5 = string_null;
		search.puzzle_inv6 = string_null;
		search.puzzle_inv7 = string_null;
		search.puzzle_inv8 = string_null;
		search=find(search,classname,"player");
	}

	WriteByte (MSG_ALL, SVC_INTERMISSION);
	WriteByte (MSG_ALL, 10);
	
	FreezeAllEntities();
}

/*QUAKED buddha_trigger_endgame (1 0.4 1) (-16 -16 0) (16 16 50)
The Buddga's teleport locations
-------------------------FIELDS-------------------------
--------------------------------------------------------
*/
void buddha_trigger_endgame (void)
{
	self.use = buddha_trigger_use;
}

/*QUAKED teleport_buddha (1 0.4 1) (-16 -16 0) (16 16 50)
The Buddga's teleport locations
-------------------------FIELDS-------------------------
--------------------------------------------------------
*/
void teleport_buddha (void)
{
}

/*QUAKED monster_buddha (1 0.3 0) (-16 -16 0) (16 16 50)
The Big Baddie
-------------------------FIELDS-------------------------
--------------------------------------------------------
*/
void monster_buddha (void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model4("models/shield.mdl");
	precache_model4("models/budd_star.mdl");
	precache_model4("models/pravus.mdl");

	precache_model4("models/biggy.spr");
	precache_model4("models/firewal1.spr");
	precache_model4("models/firewal4.spr");
	precache_model4("models/firewal5.spr");

	precache_sound4("buddha/sight.wav");
	precache_sound4("buddha/die.wav");
	precache_sound4("buddha/laugh.wav");
	precache_sound4("buddha/shoot.wav");
	precache_sound4("buddha/teleport.wav");
	precache_sound4("buddha/tele_out.wav");
	precache_sound4("buddha/recharge.wav");
	precache_sound4("buddha/firewall.wav");
	precache_sound4("buddha/random1.wav");
	precache_sound4("buddha/random2.wav");

	precache_sound("weapons/fbfire.wav");

	CreateEntityNew(self,ENT_BUDDHA,"models/pravus.mdl",buddha_die);
	
	setorigin(self, self.origin - '0 0 32');
	
	rider_init();

	self.safe_time = time + 9999999;

	self.th_pain = buddha_pain;
	self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_NOCLIP;
	self.thingtype=THINGTYPE_FLESH;
	
	self.decap = 0;
	
	self.scale = 1;

	if(!self.yaw_speed)
		self.yaw_speed = 10;

	self.mass = 999999;
	self.takedamage = DAMAGE_YES;
	self.monsterclass = CLASS_FINAL_BOSS;
	self.experience_value = 9999;

	self.dmgtime = 0;

	self.drawflags (+) SCALE_TYPE_ZONLY | SCALE_ORIGIN_BOTTOM;

	self.flags2 (+) FL_ALIVE;
	self.flags (+) FL_MONSTER;

	self.view_ofs = '0 0 40';
	self.frame = $birth1;

	buddha_spawn_shield();
	
	self.th_run = buddha_birth;
	self.th_die = buddha_die;

	self.frags = 16;
	self.camera_time = time;

	self.think = buddha_find_lightning;
	thinktime self : 0.1;
}

