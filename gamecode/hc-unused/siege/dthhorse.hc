/*
 * $Header: /HexenWorld/Siege/dthhorse.hc 3     5/25/98 1:38p Mgummelt $
 */

/*
==============================================================================

Q:\art\models\monsters\RdrDeath\Horse\

==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\RdrDeath\Horse\Final
$origin 0 0 0
$base base skin
$skin skin
$flags 0

// Horse frames
$frame Hgall1       Hgall2       Hgall3       Hgall4       Hgall5       
$frame Hgall6       Hgall7       Hgall8       Hgall9       Hgall10      
$frame Hgall11      Hgall12      

$framevalue 0

// Rider Frames
$frame Rgall1       Rgall2       Rgall3       Rgall4       Rgall5       
$frame Rgall6       Rgall7       Rgall8       Rgall9       Rgall10      
$frame Rgall11      Rgall12      

//
$frame Rsickl1      Rsickl2      Rsickl3      Rsickl4      Rsickl5      
$frame Rsickl6      Rsickl7      Rsickl8      Rsickl9      Rsickl10     
$frame Rsickl11     Rsickl12     







float death_color[12] =
{
	15,
	14,
	13,
	12,
	10,
	8,
	6,
	5,
	7,
	10,
	13,
	0
};

/*	15,
	13,
	11,
	9,
	7,
	5,
	3,
	1,
	5,
	10,
	12,
	0
*/
float death_start[1] =
{
	$Hgall1
};

float death_end[1] =
{
	$Hgall12
};

float death_speed[1] =
{
	12
};

// Array to align frames
float DeathRiderFrames[4] =
{
	$Rgall1,   // Animation for gallop

	$Rsickl1,	// Animation for attack #1
	$Rsickl8,	// Attack Frame 1
	$Rsickl9	// Attack Frame 2
};

float DH_STAGE_NORMAL		= 0;
float DH_STAGE_ATTACK1		= 4;
float DH_STAGE_ATTACK2		= 8;
float DH_STAGE_ATTACK3		= 16;

void die_out ()
{
	if(self.owner.cnt>0)
		self.owner.cnt -= 1;
	sound(self,CHAN_BODY,"death/fout.wav",1,ATTN_NORM);
	remove(self);
}

void circle_of_fire ()
{
	self.angles_y+=15;
	if(self.angles_y>360)
		self.angles_y-=360;

	if (self.dmg >= 80)
	{
		self.think=die_out;
		thinktime self : 4.2;
	}
	else
	{
		setorigin(self,self.o_angle-'0 0 76');
		self.dmg+=3.33;
		T_RadiusDamage(self,self.owner,self.dmg/3,self.owner);

		makevectors(self.angles);
		setorigin(self,self.o_angle+v_forward*100);
		SpawnMummyFlame();

		self.think=circle_of_fire;
		thinktime self : 0.05;
	}
}

void go_boom()
{
	sound (newmis, CHAN_AUTO, "weapons/expsmall.wav", 1, ATTN_NORM);
	BecomeExplosion(CE_FLOOR_EXPLOSION);
}

void fire_circ_hit ()
{
	self.touch=SUB_Null;
	self.solid=SOLID_NOT;
	self.movetype=MOVETYPE_NONE;
	self.effects=EF_NODRAW;
	self.angles='0 0 0';
	sound(self,CHAN_VOICE,"eidolon/flamend.wav",1,ATTN_NORM);
	sound(self,CHAN_BODY,"misc/fburn_bg.wav",1,ATTN_NORM);
	if(other.takedamage)
	{
		setorigin(self,other.origin+'0 0 100');
		self.angles_y=other.angles_y;
	}
	self.o_angle=self.origin;
	self.ideal_yaw=self.angles_y;

	newmis=spawn();
	newmis.owner = self.owner;
	traceline(self.origin,self.origin-'0 0 600',TRUE,self);
	setorigin(newmis,trace_endpos);
	newmis.think=go_boom;
	thinktime newmis : 0;

	self.dmg=0;
	self.think=circle_of_fire;
	thinktime self : 0;
}

void firecirc_fall_think()
{
	if (self.lifetime < time || vlen(self.enemy.origin - self.origin) < 40)
	{
		other=self.enemy;
		self.think=self.touch;
		thinktime self : 0;
		return;
	}

	HomeThink();
	self.angles=vectoangles(self.velocity);

	AdvanceFrame(0,9);

	self.think = firecirc_fall_think;
	thinktime self : 0.1;
}

void drop_fire_circ ()
{
	self.cnt += 1;
	self.attack_finished = time + 10;

	if(!self.enemy.flags2&FL_ALIVE)
		self.enemy=find(world,classname,"player");

	if(!self.enemy)
		return;

	sound (self, CHAN_WEAPON, "death/dthfire.wav", 1, ATTN_NONE);	

	Create_Missile(self,self.origin + v_forward*4 + v_right * 10 + v_up * 36, 
		self.movechain.enemy.origin+self.movechain.enemy.view_ofs + self.movechain.size+'0 0 30',"models/fireball.mdl","circfire",0,700,fire_circ_hit);

	newmis.enemy=newmis.lockentity=self.enemy;
	newmis.movetype=MOVETYPE_FLYMISSILE;
	
	newmis.speed=700;	//Speed
	newmis.turn_time=0.5;	//Lower the number, tighter the turn

	newmis.lifetime = time + 3;
	newmis.think=firecirc_fall_think;
	newmis.owner = self;
	thinktime newmis : 0;
}

void bone_think(void)
{
	remove(self);
}

void bone_touch(void)
{
	self.flags (-) FL_ONGROUND;

	if (self.classname == "b4" || self.classname == "b5")
		remove(self);

	if (self.count) 
		remove(self);

	self.avelocity_x /= 2;
	self.avelocity_y /= 2;
	self.avelocity_z /= 2;
	if (self.classname == "b6")
		self.velocity_z /= 2;
	else
		self.velocity_z /= 4;

	particle4(self.origin + '0 0 10', random(20), 256+random(80, 95), PARTICLETYPE_GRAV, random(10));

	self.think = chunk_death;
	thinktime self : 0.1;

	T_Damage(other, self, self.owner, random(1,3));
}

void generate_bone(void)
{
	entity bone;
	float chance,c2;
	vector place;

	bone = spawn();
	bone.owner = self.owner;
	bone.solid = SOLID_BBOX;
	bone.movetype = MOVETYPE_BOUNCE;

	chance = self.enemy.angles_y + random(-50,50);
	c2 = random(170,250);
	place_x = c2 * cos(chance);
	place_y = c2 * sin(chance);
	place_z = -10;
	place += self.maxs;

	bone.avelocity = randomv('-400 -400 -400', '400 400 400');

	chance = random();
	if (chance < 0.1) // rib
	{
		setmodel(bone,"models/boss/bone1.mdl");
		bone.classname = "b1";
	}
	else if (chance < 0.2)  // femer?
	{
		setmodel(bone,"models/boss/bone2.mdl");
		bone.classname = "b2";
	}
	else if (chance < 0.3)  // skull
	{
		setmodel(bone,"models/boss/bone6.mdl");
		bone.classname = "b6";
	}
	else if (chance < 0.6)  // Small piece
	{
		setmodel(bone,"models/boss/bone4.mdl");
		bone.classname = "b4";
	}
	else  // Sharp peice
	{
		setmodel(bone,"models/boss/bone5.mdl");
		bone.classname = "b5";
		bone.avelocity = '0 0 0';
	}

	setsize(bone, '0 0 0', '0 0 0');
	setorigin(bone, place);
	
	place = self.enemy.origin - bone.origin;
	place_z = 0;
	place = normalize(place);
	place *= random(200,280);
	place_z = random(-400, -600);
	bone.velocity = place;

	if (random() < 0.8)
		self.count = 1;

	bone.netname="deathbone";
	bone.touch = bone_touch;
	bone.think = bone_think;
	thinktime bone : 5;
}

void bones_think(void)
{
	if (time > self.monster_duration)
	{
		if(self.owner.cnt>0)
			self.owner.cnt -= 1;

		remove(self);
		return;
	}

	thinktime self : 0.1;

	traceline(self.enemy.origin,self.enemy.origin + '0 0 999', 1, self.enemy);
	self.maxs = trace_endpos;
	traceline(self.enemy.origin,self.enemy.origin - '0 0 999', 1, self.enemy);
	self.mins = trace_endpos;

	generate_bone();
	if (random() < 0.8) 
		generate_bone();
	if (random() < 0.5) 
		generate_bone();
}

void death_bones(void)
{
	entity dr;
	
	sound (self, CHAN_BODY, "death/dthlaugh.wav", 1, ATTN_NONE);	
	
	dr = spawn();
	dr.owner = self.owner;
	dr.solid = SOLID_NOT;
	dr.movetype = MOVETYPE_NONE;
	dr.effects = EF_NODRAW;
	dr.monster_duration = time + random(4,6);
	dr.enemy = self.enemy;
	dr.think = bones_think;
	thinktime dr : HX_FRAME_TIME;
}

void death_missile_die(void)
{
	if(self.owner.cnt>0)
		self.owner.cnt -= 1;

	CreateRedCloud (self.origin,'0 0 0',HX_FRAME_TIME);

	remove(self.trigger_field.trigger_field);
	remove(self.trigger_field);
	remove(self);
}

void death_missile_touch(void)
{
	remove(self.trigger_field.trigger_field);
	remove(self.trigger_field);

	if (other.classname != "player") 
	{//FIXME: temp effect
		if(self.owner.cnt>0)
			self.owner.cnt -= 1;
		MultiExplode();
		return;
	}
			
	death_bones();
	
	remove(self);
}

void death_missile_think(void)
{
//float diff,adjust;
	if(self.lifetime<=time)
	{
		remove(self.trigger_field.trigger_field);
		remove(self.trigger_field);
		remove(self);
	}
	else
	{
		thinktime self : 0.1;

		self.trigger_field.trigger_field.origin = self.trigger_field.origin;
		self.trigger_field.trigger_field.angles = self.trigger_field.angles;
		self.trigger_field.origin = self.wallspot;
		self.trigger_field.angles = self.o_angle;
		self.o_angle=self.angles;
		self.wallspot=self.origin;

		HomeThink();
					
		self.angles=vectoangles(self.velocity);
	}
/*
	if (self.lifetime < time) 
	{
		self.th_die();
		return;
	}

	self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
	self.count += 9;
	if (self.count > 360) 
		self.count = -360;

	adjust = sin(self.count) * 20;

	self.trigger_field.trigger_field.origin = self.trigger_field.origin;
	self.trigger_field.trigger_field.angles = self.trigger_field.angles;
	self.trigger_field.origin = self.origin;
	self.trigger_field.angles = self.angles;
	ChangeYaw();
	walkmove(self.angles_y + adjust, 25, TRUE);
	if (trace_ent)
	{
		other = trace_ent;
		death_missile_touch();
	}

	diff = self.enemy.origin_z - self.origin_z + self.enemy.view_ofs_z;
	if (diff > 10) 
		diff = 10;
	else if (diff < -10) 
		diff = -10;
	diff += adjust / 4;
	movestep(0,0,diff,FALSE);
*/
}

void death_missile_2_touch(void)
{
	float damg;

	if(self.owner.cnt>0)
		self.owner.cnt -= 1;

	if (pointcontents(self.origin) == CONTENT_SKY)
	{
		remove(self);
		return;
	}

	damg = random(4,8);

	T_Damage (other, self, self.owner, damg );

	self.origin = self.origin - 8 * normalize(self.velocity) - '0 0 40';
	sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);

	CreateRedSpark (self.origin); 

	remove(self);

}

void death_missile_2_think (void)
{
	if (self.lifetime < time || !self.enemy.flags2 & FL_ALIVE)
	{
		if(self.owner.cnt>0)
			self.owner.cnt -= 1;
		sound (self, CHAN_WEAPON, "weapons/explode.wav", 1, ATTN_NORM);
		CreateRedSpark (self.origin); 
		remove(self);
	}
	else
	{
		HomeThink();
		self.angles = vectoangles(self.velocity);
		self.think=famine_missile_think;
		thinktime self : 0.1;
	}
}

void death_missile_2(float dir)
{
	entity newmis;
	vector diff;

	if(!self.enemy.flags2&FL_ALIVE)
		self.enemy=find(world,classname,"player");

	if(!self.enemy)
		return;

	self.cnt += 1;
	self.attack_finished = time + 7;

	newmis = spawn ();
	newmis.enemy=newmis.lockentity=self.enemy;
	newmis.owner = self;
	newmis.movetype = MOVETYPE_FLYMISSILE;
	newmis.solid = SOLID_BBOX;
		
    setmodel (newmis, "models/famshot.mdl");
	setsize (newmis, '0 0 0', '0 0 0');		
	setorigin (newmis, self.origin + '0 0 120');

	newmis.angles = self.angles;

	makevectors(self.angles);

//	if (infront(self.enemy))
		diff = v_forward * 16;
//	else
//		diff = v_forward * -16;

	newmis.velocity = normalize(diff);
	newmis.angles = vectoangles(newmis.velocity);

	if (dir == 0)
		newmis.angles_y -= 25;
	else if (dir == 2)
		newmis.angles_y += 25;
	makevectors (newmis.angles);
	

	newmis.velocity = normalize(v_forward);
	
	if (dir ==1)
		newmis.speed=800;	//Speed
	else
		newmis.speed=700;	//Speed
	
	newmis.classname = "deathmissile";
	newmis.angles = vectoangles(newmis.velocity);

	newmis.veer=FALSE;	//No random wandering
	newmis.turn_time=2.25;	//Lower the number, tighter the turn
	newmis.lifetime = time + 3;
	newmis.think=famine_missile_think;
	thinktime newmis : 0.1;

	newmis.touch = famine_missile_touch;
}

void death_missile(void)
{
	entity dm;


	if(!self.enemy.flags2&FL_ALIVE)
		self.enemy=find(world,classname,"player");

	if(!self.enemy)
		return;

	self.cnt += 1;
	self.attack_finished = time + 10;

	dm = spawn();
	dm.enemy=dm.lockentity=self.enemy;
	dm.solid = SOLID_BBOX;
	dm.movetype = MOVETYPE_FLYMISSILE;
	dm.classname = "death_missile";
//	dm.flags (+) FL_FLY;
	dm.drawflags (+) DRF_TRANSLUCENT | MLS_POWERMODE;
	dm.enemy = self.enemy;
//	dm.yaw_speed = 8;
	dm.takedamage = DAMAGE_YES;
	dm.health = 10;
	dm.owner = self;
	dm.scale = 1.5;
	setmodel(dm,"models/boss/bone3.mdl");
	setorigin(dm, self.movechain.origin+self.movechain.size);
	dm.think = death_missile_think;
	dm.touch = death_missile_touch;
	dm.th_die = death_missile_die;
	dm.lifetime = time + 5;
	
	dm.speed=500;
	dm.veer=50;	//some random wandering
	dm.turn_time=2;	//Lower the number, tighter the turn

	setsize(dm,'0 0 0', '0 0 0');
	thinktime dm : HX_FRAME_TIME;

	dm.trigger_field = spawn();
	dm = dm.trigger_field;
	dm.classname = "death_missile";
	dm.solid = SOLID_NOT;
	dm.movetype = MOVETYPE_FLYMISSILE;
	dm.flags (+) FL_FLY;
	dm.drawflags (+) DRF_TRANSLUCENT | MLS_POWERMODE | SCALE_TYPE_UNIFORM;
	dm.enemy = self.enemy;
	dm.owner = dm.trigger_field;
	dm.lifetime = time + 5;
	setmodel(dm,"models/boss/bone3.mdl");
	setsize(dm,'0 0 0', '0 0 0');
	setorigin(dm, self.origin);

	dm.trigger_field = spawn();
	dm = dm.trigger_field;
	dm.classname = "death_missile";
	dm.solid = SOLID_NOT;
	dm.movetype = MOVETYPE_FLYMISSILE;
	dm.flags (+) FL_FLY;
	dm.drawflags (+) DRF_TRANSLUCENT | MLS_POWERMODE | SCALE_TYPE_UNIFORM;
	dm.enemy = self.enemy;
	dm.owner = dm.trigger_field;
	dm.scale = .5;
	dm.lifetime = time + 5;
	setmodel(dm,"models/boss/bone3.mdl");
	setsize(dm,'0 0 0', '0 0 0');
	setorigin(dm, self.origin);
	
	sound (dm, CHAN_BODY, "ambience/moan1.wav", 1, ATTN_NORM);	
}

void create_deathrider(entity horse)
{
	entity rider;

	rider = spawn();

	rider.monsterclass = CLASS_BOSS;
	rider.solid = SOLID_NOT;
	rider.movetype = MOVETYPE_NONE;
	rider.origin = horse.origin;
	rider.angles = self.angles;

	setmodel (rider, "models/boss/dthrider.mdl");
	rider.skin = 0;

	horse.movechain = rider;
	self.cnt = 0;

	rider.flags (+) FL_MOVECHAIN_ANGLE;
}

void ghost_tint ()
{
	if(self.cnt<12)
	{
		if(!self.cnt)
		{
			self.touch=SUB_Null;
			self.effects=EF_NODRAW;
		}
		self.enemy.colormap = self.enemy.movechain.colormap = 2*16+
			death_color[self.cnt];
		self.cnt+=1;
		thinktime self : 0.05;
	}
	else
	{
		self.enemy.colormap = self.enemy.movechain.colormap = 0;
		remove(self);	
	}
}

void ghost_touch ()
{
	if(other.classname!="rider_death"||self.lifespan>time)
		return;
	else
	{
		if(!self.owner.flags2&FL_ALIVE)
			self.owner.enemy=other;
		sound(self.enemy,CHAN_VOICE,"death/victory.wav",1,ATTN_NONE);
		self.think=ghost_tint;
		thinktime self : 0;
	}
}

void ghost_think ()
{
	if(self.scale<=2.4)
		self.scale+=0.1;
	if(self.speed<333)
		self.speed+=7;
	HomeThink();
	self.flags(-)FL_ONGROUND;
	self.angles=vectoangles(self.velocity);
	thinktime self : 0.1;
}

void spawn_ghost (entity attacker)
{
float r;
	newmis=spawn();
	newmis.movetype=MOVETYPE_NOCLIP;
	newmis.solid=SOLID_TRIGGER;
	newmis.classname=newmis.netname="Booberry";
	newmis.owner=self;

	newmis.drawflags(+)MLS_POWERMODE|DRF_TRANSLUCENT;
	newmis.scale=0.5;

	newmis.enemy=newmis.lockentity=attacker;
	self.enemy=newmis;
	newmis.flags2(+)FL_ALIVE;
	newmis.veer=100;
	newmis.speed=100;
	newmis.lifespan=time+7;
	newmis.turn_time=2;
	newmis.hoverz=TRUE;
	newmis.touch=ghost_touch;
	newmis.think=ghost_think;

	makevectors(self.angles);
	newmis.velocity=v_forward*150+v_up*30;
	newmis.angles=vectoangles(newmis.velocity);

	newmis.think=ghost_think;
	thinktime newmis : 1;

	setmodel(newmis,"models/booberry.mdl");
	setsize(newmis,'-8 -8 -8','8 8 8');
	newmis.hull=HULL_POINT;
	setorigin(newmis,(self.absmin+self.absmax)*0.5);

	r=random();
	if(r<0.33)
		newmis.noise="ambience/moan1.wav";
	else if(r<0.66)
		newmis.noise="ambience/moan2.wav";
	else
		newmis.noise="ambience/moan3.wav";
	sound(newmis,CHAN_AUTO,newmis.noise,1,ATTN_NONE);
}

void deathhorse_move(void)
{
	float retval, r;
	
	if (self.velocity) 
		self.velocity = self.velocity * 0.9;

	if (coop)
		checkenemy();

	if(!self.enemy.flags2&FL_ALIVE&&self.enemy!=world)
		self.enemy=world;

	self.think = deathhorse_move;
	thinktime self : HX_FRAME_TIME;

	retval = AdvanceFrame(death_start[self.rider_gallop_mode],death_end[self.rider_gallop_mode]);

	self.movechain.angles = self.angles;
	self.movechain.origin = self.origin; 

	if (!self.path_current)
	{
		riderpath_init();
	}
		
	if (self.frame == 4 || self.frame == 6 || self.frame == 8 || self.frame == 10)
	{
		r = random();

		if (r < 0.2)
			sound (self, CHAN_BODY, "death/clop.wav", 0.3, ATTN_NORM);	
		else if (r < 0.4)
			sound (self, CHAN_BODY, "death/clop1.wav", 0.3, ATTN_NORM);	
		else if (r < 0.6)
			sound (self, CHAN_BODY, "death/clop2.wav", 0.3, ATTN_NORM);	
		else
			sound (self, CHAN_BODY, "death/clop3.wav", 0.3, ATTN_NORM);	
	}
		
	riderpath_move(self.speed);
	if (retval == AF_BEGINNING)
	{
		retval = fabs(self.rider_y_change);

		// Force a new gallop frame in
		self.frame = death_start[self.rider_gallop_mode];

		self.monster_stage = DH_STAGE_NORMAL;

		if (self.rider_gallop_mode == 0)
		{			
			if (!self.enemy)		
			{
				self.enemy = find(world, classname, "player");
				while(!self.enemy.flags2&FL_ALIVE&&self.enemy!=world)
					self.enemy = find(self.enemy, classname, "player");
			}

			if (self.enemy != world && random() < 0.7+skill/10&&!self.cnt)
			{
				r = random();

				if (r < 0.3)
					self.monster_stage = DH_STAGE_ATTACK1;
				else if (r < 0.6)
					self.monster_stage = DH_STAGE_ATTACK3;
				else
					self.monster_stage = DH_STAGE_ATTACK2;
			}
		}
	}

	if (self.cnt && self.attack_finished < time) self.cnt = 0;

	if (self.monster_stage && self.enemy.flags2 & FL_ALIVE)
	{
		if (self.rider_gallop_mode == 0)
		{
		
			if (self.monster_stage == DH_STAGE_ATTACK1)
			{
				self.movechain.frame = DeathRiderFrames[1] + 
					(self.frame - death_start[self.rider_gallop_mode]);
				if (self.movechain.frame == DeathRiderFrames[2])
				{
					sound (self, CHAN_WEAPON, "death/dthfire.wav", 1, ATTN_NONE);	
					death_missile();
				}
			}
			else if (self.monster_stage == DH_STAGE_ATTACK2)
			{
				self.movechain.frame = DeathRiderFrames[1] + 
					(self.frame - death_start[self.rider_gallop_mode]);
				if (self.movechain.frame == 18)
					sound (self, CHAN_WEAPON, "death/shot.wav", 1, ATTN_NORM);	

				if (self.movechain.frame == 18 || self.movechain.frame == 20 || self.movechain.frame == 22)
					death_missile_2(FALSE);
			}
			else if (self.monster_stage == DH_STAGE_ATTACK3)
			{
				self.movechain.frame = DeathRiderFrames[1] + 
					(self.frame - death_start[self.rider_gallop_mode]);
				if (self.movechain.frame == DeathRiderFrames[2])
					drop_fire_circ();
			}

	
			self.colormap = self.movechain.colormap = 
				death_color[self.frame - death_start[self.rider_gallop_mode]];
			if(self.colormap)
			{
				self.colormap+=8*16;
				self.movechain.colormap+=8*16;
			}
		}
	}
	else
	{
		self.movechain.frame = DeathRiderFrames[self.rider_gallop_mode] + 
			(self.frame - death_start[self.rider_gallop_mode]);
	}


	// make sure we use the last attack frame before we go out of the mode
	if (retval == AF_END)
	{
		self.monster_stage = DH_STAGE_NORMAL;
	}

	if (fabs(death_speed[self.rider_gallop_mode] - self.speed) < 0.2)
		self.speed = death_speed[self.rider_gallop_mode];
	else if (death_speed[self.rider_gallop_mode] > self.speed) 
		self.speed += 0.2;
	else
		self.speed -= 0.2;
}







/*QUAKED rider_death (1 0 0)  (-55 -55 -24) (55 55 100) TRIGGER_WAIT
Death rider monster.  You must place rider_path entites
on the map.  The rider will first proceed to the 
rider_path point with a path_id of 1.
-------------------------FIELDS-------------------------
map: next map to go to when you kill the rider
target: start spot on the next map
--------------------------------------------------------

*/
void rider_death(void)
{
	if (deathmatch)
	{
		remove(self);
		return;
	}

	precache_model2 ("models/boss/dthhorse.mdl");
	precache_model2 ("models/boss/dthrider.mdl");
	precache_model2 ("models/famshot.mdl");
    precache_model2 ("models/boss/bone1.mdl");
    precache_model2 ("models/boss/bone2.mdl");
    precache_model2 ("models/boss/bone3.mdl");
    precache_model2 ("models/boss/bone4.mdl");
    precache_model2 ("models/boss/bone5.mdl");
    precache_model2 ("models/boss/bone6.mdl");
	precache_model2 ("models/mumshot.mdl");
	precache_model2 ("models/booberry.mdl");
	
	precache_sound2 ("mummy/mislfire.wav");
	precache_sound2 ("eidolon/flamend.wav");
	precache_sound2 ("misc/fburn_bg.wav");
	precache_sound2 ("death/fout.wav");

	precache_sound2 ("death/dthdie.wav");
	precache_sound2 ("death/dthfire.wav");
	precache_sound2 ("death/victory.wav");
	precache_sound2 ("death/dthlaugh.wav");
	precache_sound2 ("death/clop.wav");
	precache_sound2 ("death/clop1.wav");
	precache_sound2 ("death/clop2.wav");
	precache_sound2 ("death/clop3.wav");
	precache_sound2 ("death/shot.wav");
	precache_sound2 ("ambience/moan1.wav");
	precache_sound2 ("ambience/moan2.wav");
	precache_sound2 ("ambience/moan3.wav");
	
	rider_init();

	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_FLY;
	self.thingtype = THINGTYPE_FLESH;
	self.yaw_speed = 4;

	setmodel (self, "models/boss/dthhorse.mdl");

	self.hull = HULL_POINT;

	self.skin = 0;
    self.flags (+) FL_FLY|FL_MONSTER;
	self.flags2(+) FL_ALIVE;
	self.monsterclass = CLASS_BOSS;

	setsize (self, '-55 -55 -24', '55 55 100');
	self.health = 3500;
	self.experience_value = 1000;

	self.dflags = 0;
	self.rider_gallop_mode = 0;
	self.speed = death_speed[self.rider_gallop_mode];
	self.rider_path_distance = 100;
	self.monster_stage = DH_STAGE_NORMAL;
	self.mass = 30000;

	create_deathrider(self);

	self.noise = "death/dthdie.wav";
	self.delay = 3;

	self.th_save = deathhorse_move;
	self.think = multiplayer_health;
	thinktime self : 1;
}

