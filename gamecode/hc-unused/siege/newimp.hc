/*
 * $Header: /HexenWorld/Siege/newimp.hc 4     6/01/98 2:49a Mgummelt $
 */
/*
==============================================================================

ICE IMP

==============================================================================
*/

// For building the model
$cd c:\model\imp		// Directory to find model in
$origin 0 0 0		
// baseframe is in iceimp.3ds
$base iceimp
$flags 0
// skin is in iceimp.lbm
$skin skin
$skin skinice

$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      

//
$frame impfir1      impfir2      impfir3      impfir4      impfir5      
$frame impfir6      impfir7      impfir8      impfir9      impfir10     
$frame impfir11     impfir12     impfir13     impfir14     impfir15     
$frame impfir16     impfir17     impfir18     impfir19     impfir20     
$frame impfir21     

//
$frame impfly1      impfly2      impfly3      impfly4      impfly5      
$frame impfly6      impfly7      impfly8      impfly9      impfly10     
$frame impfly11     impfly12     impfly13     impfly14     impfly15     
$frame impfly16     impfly17     impfly18     impfly19     impfly20     

//
$frame impup1       impup2       impup3       impup4       impup5       
$frame impup6       impup7       impup8       impup9       impup10      
$frame impup11      impup12      impup13      impup14      impup15      
$frame impup16      impup17      impup18      impup19      impup20      
$frame impup21      impup22      impup23      

//
$frame impwat1      impwat2      impwat3      impwat4      impwat5      
$frame impwat6      impwat7      impwat8      impwat9      impwat10     
$frame impwat11     impwat12     impwat13     impwat14     impwat15     
$frame impwat16     impwat17     impwat18     impwat19     impwat20     
$frame impwat21     impwat22     impwat23     impwat24     

//
$frame swoop1       swoop2       swoop3       swoop4       swoop5       
$frame swoop6       swoop7       swoop8       swoop9       swoop10      
$frame swoop11      swoop12      swoop13      swoop14      swoop15      
$frame swoop16      swoop17      swoop18      swoop19      swoop20      

//
$frame swpcyc1      swpcyc2      swpcyc3      swpcyc4      

//
$frame swpend1      swpend2      swpend3      swpend4      swpend5      
$frame swpend6      swpend7      swpend8      swpend9      swpend10     
$frame swpend11     swpend12     swpend13     swpend14     swpend15     

//
$frame swpout1      swpout2      swpout3      swpout4      swpout5      
$frame swpout6      swpout7      swpout8      swpout9      swpout10     
$frame swpout11     swpout12     swpout13     swpout14     swpout15     



//============================================================================


float MONSTER_STAND_GROUND = 1;
float MONSTER_HOVER        = 2;
float SF_IMP_DORMANT		= 16;
float PICKUP				= 32;
float AS_FERRY		= 6;

void()imp_fly;
void()imp_hover;
void()imp_abort_swoop;
float()imp_find_target;
/*
================================================================
checkenemy()
Checks to see if enemy is of the same monstertype and old enemy
is alive and visible.  If so, changes back to it's last enemy.
================================================================
*/
void checkenemy (void)
{
entity oldtarget;
	if(self.enemy.classname=="player"&&self.enemy.flags2&FL_ALIVE)
		return;

	if(self.oldenemy.classname=="player"&&(self.oldenemy.flags2&FL_ALIVE)&&visible(self.oldenemy))
	{	//FIXME: Whoops!  These are unions!!!
		if((self.spiderType&&self.enemy.spiderType)||(self.scorpionType&&self.enemy.scorpionType))
			self.enemy=self.oldenemy;
		else
		{
			oldtarget=self.enemy;
			self.enemy=self.oldenemy;
			self.oldenemy=oldtarget;
		}
		self.goalentity=self.enemy;
	}
}

/*
================================================================
fov()

Field-Of-View

Returns TRUE if vector from entity "from" to entity "targ" is
within "scope" degrees of entity "from"'s forward angle.
================================================================
*/
float fov(entity targ,entity from,float scope)
{
vector spot1,spot2;
float dot;
	spot1=from.origin+from.proj_ofs;

	spot2=(targ.absmin+targ.absmax)*0.5;

	if(from.classname=="player")
		makevectors(from.v_angle);
	else
		makevectors(from.angles);

//	scope=1 - (scope/180);//converts angles into %
	dot=normalize(spot2-spot1)*v_forward;
	dot=180 - (dot*180);
//	dprintf("FOV value : %s\n",dot);
	if(dot<=scope)
		return TRUE;

	return FALSE;
}

void check_pos_enemy ()
{
	if(!self.mintel)
		return;

	if(!visible(self.enemy))
	{
		self.attack_state = AS_STRAIGHT;
		SetNextWaypoint();
	}
	else
	{
		self.goalentity=self.enemy;
	    self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
	}
}

float clear_path (entity targ)
{
vector destiny,org;
	destiny=targ.origin+targ.proj_ofs;
	org=(self.absmin+self.absmax)*0.5;

	self.attack_state = AS_STRAIGHT;
	tracearea (org, destiny, '-16 -16 0','16 16 28',FALSE,self);
	if (trace_ent != targ)
	{
		self.attack_state = AS_SLIDING;
		return FALSE;
	}
	return TRUE;
}

float imp_up_amounts[23] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	2,
	2,
	3,
	3,
	3,
	3,
	4,
	4,
	5,
	6,
	7,
	5,
	2
};

float imp_fly_amounts[20] = 
{
	-0.4,
	-0.2,
	0,
	0,
	0.2,
	0.4,
	0.4,
	0.6,
	0.6,
	0.8,
	0.6,
	0.4,
	0,
	0,
	-0.2,
	-0.4,
	-0.8,
	-0.8,
	-0.8,
	-0.4
};

void imp_drop (void)
{
	self.attack_state=AS_STRAIGHT;
	if(self.classname=="monster_imp_lord")
	{
		if(self.enemy.classname!="player")
			self.enemy.health=10;
		self.enemy.movetype=MOVETYPE_BOUNCE;
		self.enemy.mass=10000;
		self.enemy.velocity_z=-300;
	}
}

void imp_die ()
{
	if(self.health<-40)
	{
		chunk_death();
		return;
	}
	if(self.frame!=$death14)
		AdvanceFrame($death1,$death14);

	if(self.frame==$death1)
	{
		sound (self, CHAN_WEAPON, "imp/die.wav", 1, ATTN_NORM);
	
		makevectors (self.angles);
	
		self.movetype = MOVETYPE_BOUNCE;

		self.velocity_z = 0;
		self.velocity += v_forward*80;

		if(self.attack_state==AS_FERRY)
			imp_drop();
	}

	if(self.frame==$death8)
		if (self.flags & FL_ONGROUND)
		{
			// when he has finally rested on the ground, finish the death
			sound(self,CHAN_BODY,"player/land.wav",1,ATTN_NORM);
			self.velocity_x = self.velocity_y = 0.0;
		}
		else
			self.frame==$death7;
	
	if(self.flags & FL_ONGROUND&&!self.aflag)
	{
	vector new_angles,new_angles2,old_forward,old_right;
	float dot,mod;
//		dprint("Angles fixed\n");
		self.aflag=TRUE;
		makevectors(self.angles);
		old_forward=v_forward;
		old_right=v_right;
		traceline(self.origin,self.origin-'0 0 24',TRUE,self);

		new_angles=vectoangles(trace_plane_normal);
//		dprintf("Pitch of slope: %s\n",new_angles_x);

		new_angles_x=(90-new_angles_x)*-1;//Gets actual slope
		dprintf("actual slope: %s\n",new_angles_x);
		new_angles2='0 0 0';
		new_angles2_y=new_angles_y;
		makevectors(new_angles2);

		mod=v_forward*old_right;
//		dprintf("Mod: %s\n",mod);
		if(mod<0)
			mod=-1;
		else
			mod=1;

		dot=v_forward*old_forward;
//		dprintf("difference in yaw: %s\n",dot);

		self.angles_x=dot*new_angles_x*mod;
//		dprintf("New pitch: %s\n",self.angles_x);

		self.angles_z=(1-dot)*new_angles_x*mod*-1;
//		dprintf("New roll: %s\n",self.angles_z);
	}

	if(self.frame==$death14 &&self.flags&FL_ONGROUND)
		MakeSolidCorpse();
	else if(self.health<-40)
		chunk_death();
//check to see if health > 0.  if so, come back alive
//This could happen by magic or by fire for fire imp or cold for ice imp
	else if(self.health>0)
	{
		self.takedamage = DAMAGE_YES;
		self.flags (+) FL_FLY;
		self.flags2 (+) FL_ALIVE;
		self.solid = SOLID_SLIDEBOX;
		self.movetype = MOVETYPE_FLY;
		if(self.classname=="monster_imp_lord")
		{
			setsize (self, '-32 -32 0', '32 32 56');
			self.hull=HULL_SCORPION;
		}
		else
		{
			setsize (self, '-16 -16 0', '16 16 36');
			self.hull=HULL_CROUCH;
		}
		self.th_stand ();
		return;
	}
	else
	{
		self.think=imp_die;
		if (self.frame == $death7 || self.frame == $death8 )	// when cycling between these 2 frames during falling, make him flutter more randomly
			thinktime self :  random(0.05,0.217);
		else
			thinktime self : HX_FRAME_TIME;
	}
}

void imp_die_init ()
{
	setsize (self, '-16 -16 0', '16 16 24');
	self.hull=HULL_CROUCH;
	imp_die();
}

float check_z_move()
{
float goaldist,moverange;
entity targ;
	if(self.enemy!=world)
		targ=self.enemy;
	else if(self.goalentity!=world)
		targ=self.goalentity;
	else
		return FALSE;

	if(fabs(targ.origin_z-self.origin_z)<48&&!visible(targ))
		return FALSE;	//FIXME: Find an up or down

	if(visible(targ))
		if(!clear_path(targ))
			return FALSE;

	if(targ.origin_z!=self.absmin_z)
	{
		goaldist=(targ.absmin_z+targ.absmax_z)*0.5-(self.absmax_z+self.absmin_z)*0.5;
		moverange=fabs(self.level);
		if(goaldist>0&&goaldist>moverange)
			goaldist=moverange;
		else if(goaldist<0&&goaldist<moverange*-1)
			goaldist=moverange*-1;
		movestep(0,0,goaldist, FALSE);
//		self.velocity_z=goaldist*3;
	}
//	else if(self.velocity_z>=1)
//		self.velocity_z/=2;
//	else
//		self.velocity_z=0;

	return TRUE;
}

void imp_ferry ()
{
float dist;
vector org;
	walkmove(self.angles_y, self.speed, FALSE);
	check_z_move();
	org=self.enemy.origin;
	org_z=self.enemy.absmax_z;
	dist=vlen(self.origin-org);
	if(dist>200)
	{
		imp_drop();
		return;
	}
	else// if(self.enemy!=self.movechain)
		self.enemy.velocity=normalize(self.origin-org)*dist*5;
	self.enemy.angles=self.angles;
}

void imp_pick_up (void)
{
	self.attack_state=AS_FERRY;
	self.goalentity=find(world,targetname,self.target);
}

void imp_up_down()
{	// Function to make the imp randomly change height
vector   end_vec,vec1;

//	dprint("CHECKING UP/DOWN\n");
	if (self.velocity_z > -10.0 && self.velocity_z < 10.0)
	{  // If we aren't moving up/down that much, maybe we want to change altitudes
		self.velocity_z = 0.0;
		if (random() < 0.3||self.attack_state==AS_SLIDING)
		{  // Should we change altitudes?
			makevectors (self.angles);
			if (random() < 0.5)
			{ // Go Down
				vec1 = self.origin + v_up*-48;
				traceline (self.origin,vec1,FALSE,self);
				if(trace_fraction==1)
				{
					end_vec=vec1+v_forward*32;
					traceline (vec1,end_vec,FALSE,self);
					if (trace_fraction == 1||random()<0.5)
					{
						//dprint("Goin down\n");
   						self.velocity_z = self.speed*-7;
					}
				}
			}
			else
			{ // Go Up
				vec1 = self.origin + v_up*88;
				traceline (self.origin,vec1,FALSE,self);
				if(trace_fraction==1)
				{
					end_vec=vec1+v_forward*32;
					traceline (vec1,end_vec,FALSE,self);
					if (trace_fraction == 1||random()<0.5)
					{
						//dprint("Goin up\n");
   						self.velocity_z = self.speed*7;
					}
				}
			}
			self.flags(-)FL_ONGROUND;
		}
	}
	else self.velocity_z = self.velocity_z / 1.05;
}

void imp_set_speeds ()
{
float anglediff,dist;
	dist=8;	//Movement distance this turn

	anglediff=	fabs(self.angles_y - self.ideal_yaw);	//How far we're trying to turn

	if (anglediff > 20)   // If it is a big distance to turn, then don't move as far forward
	   dist = dist / 1.5;

	self.level=imp_fly_amounts[self.frame - $impfly1];
	self.speed = dist + self.level*4;	//tweaks to speed based on anim frame

	if(!visible(self.enemy))
		self.level*=4;
}

float imp_check_too_close ()
{
float enemy_zdiff,enemy_hdist;
	if(!visible(self.enemy))
		return FALSE;

	enemy_zdiff=fabs(self.origin_z-self.enemy.origin_z);
	enemy_hdist=vhlen(self.enemy.origin-self.origin);
	if(enemy_zdiff>=77&&enemy_hdist<=77)
	{
//		dprint("too close!\n");
		return TRUE;
	}
	return FALSE;
}

void imp_move ()
{
float too_close;
	checkenemy();
	
	self.velocity*=1/1.05;
	if(imp_check_too_close())
	{
		self.ideal_yaw=self.angles_y;
		too_close=TRUE;
	}
	else
		ai_face();

	imp_set_speeds();

	if(self.attack_state==AS_FERRY)
	{
		imp_ferry();
		return;
	}

	if(self.attack_state==AS_STRAIGHT)
	{
		if(too_close)	
		{
			if(!walkmove(self.angles_y,self.speed,FALSE))
				movetogoal(self.speed);
		}
		else
			movetogoal(self.speed);
	}
	else if(self.attack_state==AS_SLIDING)
	{	//FIXME: make a more intelligent slide?
//		dprint("sliding\n");
		enemy_yaw = vectoyaw(self.enemy.origin - self.origin);
		movedist=self.speed;
		ai_run_slide();
	}

	movestep(0,0,self.level, FALSE);

	if(!check_z_move())
		imp_up_down();

	if(vlen(self.enemy.origin+self.enemy.proj_ofs-self.origin)<20&&random()<0.2&&self.target!=""&&self.origin_z>self.enemy.absmax_z - 8&&self.spawnflags&PICKUP)
		imp_pick_up();
}

float imp_new_action ()
{
float too_close;
	enemy_vis=visible(self.enemy);
	too_close=imp_check_too_close();
	if((random()<0.7&&self.enemy.flags2&FL_ALIVE)||!enemy_vis||too_close)
	{
		if(self.think!=imp_fly)
		{
			self.think=imp_fly;
			thinktime self : 0;
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		if(self.think!=imp_hover)
		{
			self.think=imp_hover;
			thinktime self : 0;
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

void imp_strafe_left () [++ $impfly1 .. $impfly20]
{
vector dir;
	ai_face();
	if(cycle_wrapped)
	{
		self.think=imp_fly;
		thinktime self : 0;
		return;
	}
//	else if(self.frame==$impfly1)
//		dprint("strafing left\n");
	makevectors(self.angles);
	dir='0 0 0' - (v_right*(self.frame - $impfly1)*30);
	self.velocity_x=dir_x;
	self.velocity_y=dir_y;
	check_pos_enemy();
//FIXME: check for attack?
}

void imp_strafe_right () [++ $impfly1 .. $impfly20]
{
vector dir;
	ai_face();
	if(cycle_wrapped)
	{
		self.think=imp_fly;
		thinktime self : 0;
		return;
	}
//	else if(self.frame==$impfly1)
//		dprint("strafing right\n");
	makevectors(self.angles);
	dir=v_right*(self.frame - $impfly1)*30;
	self.velocity_x=dir_x;
	self.velocity_y=dir_y;
	check_pos_enemy();
//FIXME: check for attack?
}

void imp_rise () [++ $impfly1 .. $impfly20]
{
	check_pos_enemy();
	if(self.frame==$impfly1)
	{
//		dprint("shooting up\n");
		self.attack_finished=time+1.7;
		self.velocity_z=600;
	}
	else if(self.frame>$impfly14 &&self.attack_finished<time)
	{
		self.frame==$impfly14;
		self.velocity_z-=100;
	}
	else
		self.velocity_z+=10;

	if(self.frame==$impfly20)
	{
		self.velocity_z=0;
		self.think=imp_fly;
		thinktime self : 0.05;
	}
}

void imp_dive () [++ $swoop1 .. $swoop20]
{
	check_pos_enemy();
	if(self.frame==$swoop1)
	{
//		dprint("diving for cover\n");
		self.velocity_z=-500;
	}
	else if(self.frame<$swoop13)
		self.velocity_z-=20;
	else
		self.velocity_z+=100;

	if(self.frame==$swoop20)
	{
		self.velocity_z=0;
		self.think=imp_abort_swoop;
		thinktime self : 0;
	}
}

float imp_check_defense ()
{
vector org,destiny,dir,proj_spot,proj_spot_dir;
float dist,dot;
	if(self.enemy.last_attack<time - 1)
		return FALSE;
	
	if(fov(self,self.enemy,45))
	{
		org=self.enemy.origin+self.enemy.proj_ofs;	//enemy's firing point, approximately
		destiny=(self.absmin+self.absmax)*0.5;	//my center
		dir=normalize(destiny-org);//direction from enemy to me
		dist=vlen(destiny-org);	//distance between enemy and me
		proj_spot=org+dir*(dist);	//projected destination of projectile
		proj_spot_dir=normalize(proj_spot-destiny);	//that spot's position relativeto me

		makevectors(self.angles);

		dot = proj_spot_dir * v_right;
		if ( dot > 0.1)
		{
			imp_strafe_left();//Left because you want to go away from it
			return TRUE;
		}
		else if( dot < -0.1)
		{
			imp_strafe_right();
			return TRUE;
		}
		dot = proj_spot_dir * v_up;
		if ( dot > 0.2)
		{
			imp_dive();
			return TRUE;
		}
		else if( dot < 0)
		{
			imp_rise();
			return TRUE;
		}
//		dprint("no defense dir found\n");
		return FALSE;
	}
	return FALSE;
}

void imp_attack(void)
{
	if (self.origin_z - self.enemy.origin_z < 50 && vhlen(self.origin-self.enemy.origin) < 64)
	{
		self.v_angle=self.angles;
		FireMelee (20,10,64);
	}
	else if (self.classname == "monster_imp_ice"||(self.classname == "monster_imp_lord"&&random()<0.5))
	{
		sound (self, CHAN_WEAPON, "imp/shard.wav", 1, ATTN_NORM);

		makevectors (self.angles);
		do_shard('14 8 0',360 + random()*150, '0 0 0');
		do_shard('14 8 0',360 + random()*150, (v_forward * ((random() * 40) - 20)) +
														  (v_right * ((random() * 40) - 20)) +
														  (v_up * ((random() * 20) - 10)));
		do_shard('14 8 0',360 + random()*150, (v_forward * ((random() * 40) - 20)) +
														  (v_right * ((random() * 40) - 20)) +
														  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0',360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0',360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0',360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
	}
	else
	{
		sound (self, CHAN_WEAPON, "imp/fireball.wav", 1, ATTN_NORM);
		do_fireball('14 8 0');
	}
}

void imp_attack_anim() [++ $impfir1 .. $impfir21]
{
	check_pos_enemy();

	ai_face();

	if(imp_check_defense())
		return;

	if (self.frame == $impfir17)
		imp_attack();
	else if (cycle_wrapped)
	{
		self.think=imp_fly;
		if(visible(self.enemy)&&self.enemy.flags2&FL_ALIVE)
			if(random()<0.2+skill/10)
				self.think=imp_attack_anim;
		self.attack_finished=time + 1;
		thinktime self : 0;
	}
}

void imp_abort_swoop () [++ $swpout1 .. $swpout15]
{
	check_pos_enemy();
	if(self.velocity_x>10)
		self.velocity_x/=2;
	else
		self.velocity_x=0;

	if(self.velocity_x>10)
		self.velocity_y/=2;
	else
		self.velocity_y=0;

	if(self.frame==$swpout15)
	{
		self.attack_finished=time + 1;
		self.think=imp_fly;
		thinktime self : 0;
	}
}

void imp_swoop_end () [++ $swpend1 .. $swpend15]
{
	check_pos_enemy();
	self.flags (-) FL_ONGROUND;

	// swoop him back up and slow down his forward velocity (xy)
	self.velocity_z += 15;
	self.velocity_x /= 1.2;
	self.velocity_y /= 1.2;

	if (self.frame == $swpend15)
	{	// Finished swooping
		self.attack_finished=time + 1;
		self.velocity = '0 0 0';
		self.yaw_speed = 8;
		self.think=imp_hover;
		thinktime self : 0;
	}
}

void imp_swoop_charge () [++ $swpcyc1 .. $swpcyc4]
{
vector dir,destiny,org;
	check_pos_enemy();
	destiny=self.enemy.origin+self.enemy.proj_ofs;
	org=(self.absmin+self.absmax)*0.5;
	enemy_vis=visible(self.enemy);
	enemy_infront=infront(self.enemy);
	enemy_range=vlen(destiny - org);
	if(self.enemy.last_attack>time - 1 &&fov(self,self.enemy,45))
	{
		self.velocity_z+=200;
		imp_abort_swoop();
	}
	else if(enemy_vis&&enemy_infront&&enemy_range<2000)
	{
		dir=normalize(destiny-org);
		self.velocity=dir*(400+self.count*10);
		ai_face();

		self.count += 1;

		if (self.flags & FL_ONGROUND || self.count > 30)
		{  // Didn't hit our target, so go back up
			self.flags (-) FL_ONGROUND;
			imp_abort_swoop();
		}
	}
	else
		imp_abort_swoop();
}

void imp_enter_swoop () [++ $swoop1 .. $swoop20]
{
vector vec;
	check_pos_enemy();
	if(self.frame==$swoop1)
	{
		self.yaw_speed=15;
		self.count=140;
		self.velocity = '0 0 0';

		sound (self, CHAN_BODY, "imp/swoop.wav", 1, ATTN_NORM);
	}

	ai_face();
	self.count *= 1.15;

	if (self.frame >= $swoop12)
	{  // Start to swoop down
		vec = normalize(self.enemy.origin - self.origin + self.enemy.proj_ofs);
		self.velocity = vec*self.count;
		if (self.frame <= $swoop13 )
		{  // If we haven't pulled out yet, keep going straight down
			self.velocity_x = self.velocity_y = 0;
		}
		if(self.origin_z - self.enemy.absmax_z>50&&self.frame==$swoop13)
			self.frame=$swoop12;
	}

	if (self.frame == $swoop12 || self.frame == $swoop13 )
	{  // Swoop down cycling frames
		if (self.flags & FL_ONGROUND)
		{ // Ran into something on the way down
			self.flags (-) FL_ONGROUND;
			self.frame = $swoop14;
			self.count = 280;
		}
		if (self.origin_z - self.enemy.origin_z < 60)
		{ // Have to flatten out soon
			self.frame = $swoop14;
			self.count = 280;
		}
	}

	if(self.frame==$swoop20)
	{
		self.count=0;
		self.think=imp_swoop_charge;
		thinktime self : 0.05;
	}
}

void imp_straight_swoop () [-- $swpout15 .. $swpout1]
{
	check_pos_enemy();
	ai_face();
	if(imp_check_defense())
		return;

	if(self.frame==$swpout15)
	{
		self.yaw_speed=15;
		self.count=140;
		self.velocity = '0 0 0';

		sound (self, CHAN_BODY, "imp/swoop.wav", 1, ATTN_NORM);
	}

	if(self.frame==$swpout1)
	{
		self.count=0;
		self.think=imp_swoop_charge;
		thinktime self : 0.05;
	}
}

void() imp_touch =
{
float damg;
vector punch;

	if((self.frame >= $swpcyc1 && self.frame <= $swpcyc4 )||(self.frame>=$swoop16 &&self.frame<=$swoop20))
	{  // If we are in swoop attack frames
		self.flags (-) FL_ONGROUND;
		sound (self, CHAN_WEAPON, "imp/swoophit.wav", 1, ATTN_NORM);

		self.think=imp_swoop_end;
		if (other.takedamage)
		{  // We sucessfully hit something

			punch=normalize(self.angles)* -1;
			punch *= damg;
			other.punchangle=punch;
			if(self.classname == "monster_imp_lord")
			{
				damg = (20 + (self.count *3));
				T_Damage (other, self, self.owner, damg);
				other.velocity_x += self.velocity_x*2;
				other.velocity_y += self.velocity_y*2;
				if(other.movetype==MOVETYPE_FLY)
				{
					if(other.flags&FL_ONGROUND)
						other.velocity_z=200;
				}
				else
					other.velocity_z=200;
				other.flags(-)FL_ONGROUND;
			}
			else
			{
				damg = (10 + (self.count / 2));
				T_Damage (other, self, self.owner, damg);
			}
		}
		else if(vlen(self.velocity)>600)
		{
			makevectors(self.angles);
			SpawnPuff(self.origin+self.proj_ofs+v_forward*self.absmax_x,'0 0 0',10,self);
			self.pain_finished=-666;
			self.think=self.th_pain;
		}
		self.velocity=other.velocity;//??
	}
};

float imp_check_attack ()
{
float enemy_hdist,enemy_zdiff,swoop_no_drop;
vector destiny,org;

	if(!self.monster_awake||self.enemy==world)
		return FALSE;

	enemy_vis=visible(self.enemy);
	if (!enemy_vis)
	{
		if(self.mintel)
			SetNextWaypoint();
		self.attack_state = AS_STRAIGHT;
		return FALSE;
	}
	else if(self.mintel)
	{
		self.goalentity=self.enemy;
	    self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
	}

	if(self.attack_state==AS_FERRY)
		return FALSE;

	if (time < self.attack_finished)
		return FALSE;

	enemy_infront=infront(self.enemy);
	if(!enemy_infront)
		return FALSE;
	
	enemy_range=range(self.enemy);
	if (enemy_range == RANGE_FAR)
	{
		self.attack_state = AS_STRAIGHT;
		return FALSE;
	}
		
// see if any entities are in the way of the shot

	destiny=self.enemy.origin+self.enemy.proj_ofs;
	org=(self.absmin+self.absmax)*0.5;

	self.attack_state = AS_STRAIGHT;
	traceline (org, destiny, FALSE, self);
	if (trace_ent != self.enemy)
	{
//		dprint("Imp shot blocked by: ");
//		dprint(trace_ent.classname);
//		dprint("\n");
		if(trace_ent.health>25||!trace_ent.takedamage||(trace_ent.flags&FL_MONSTER&&trace_ent.classname!="player_sheep"))
		{//Don't have a clear shot, and don't want to shoot obstruction
//			dprint("\n");
			self.attack_state = AS_SLIDING;
			return FALSE;
		}
//		dprint("- trying to kill it\n");
	}
			
	if(random()<0.2+skill/10)
		return FALSE;

	if(!self.spawnflags & MONSTER_STAND_GROUND)
	{
	float swoop_no_drop;
		swoop_no_drop=FALSE;
		
		enemy_hdist=vhlen(destiny-org);
		enemy_zdiff=org_z - destiny_z;
		if(enemy_zdiff<=36&&random()<0.3)
			swoop_no_drop=TRUE;
		
		if((enemy_hdist>100&&enemy_zdiff>36)||swoop_no_drop)
		{
			tracearea(org,org-'0 0 1'*enemy_zdiff,'-16 -16 0','16 16 28',FALSE,self);
			if(trace_fraction==1||swoop_no_drop)
			{
				if(swoop_no_drop)
					tracearea(org,destiny,'-16 -16 0','16 16 28',FALSE,self);
				else
					tracearea(org-'0 0 1'*enemy_zdiff,destiny,'-16 -16 0','16 16 28',FALSE,self);
				if(trace_ent==self.enemy)
				{
					if(swoop_no_drop)
						self.think=imp_straight_swoop;
					else
						self.think=imp_enter_swoop;
					thinktime self : 0;
					return TRUE;
				}
			}
		}
	}
	
	self.think=imp_attack_anim;
	thinktime self : 0;
	return TRUE;
}

void imp_hover() [++ $impfly1 .. $impfly20]
{
float too_close;
	checkenemy();
	if(!self.enemy)
		if(imp_find_target())
			return;

	self.velocity*=1/1.05;
	ai_face();
	imp_set_speeds();
	check_z_move();
	
	if(imp_check_defense())
		return;

	if(imp_check_attack())
		return;

	if(self.enemy!=world)
	{
		too_close=imp_check_too_close();
		enemy_vis=visible(self.enemy);
		if(!enemy_vis||too_close||self.attack_state==AS_SLIDING)
		{
			self.think=imp_fly;
			thinktime self : 0;
			return;
		}
	}

	if(self.enemy!=world||self.goalentity!=world)
		if(imp_new_action())
			return;
}

void() stone_imp_awaken = [++ $impup7 .. $impup23]
{
	if(self.frame==$impup10)
	{
		self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
		self.count = 0;
		self.monster_awake = TRUE;
		setsize (self, '-16 -16 0', '16 16 36');
		self.hull=HULL_CROUCH;
		self.mass = 3;
		self.health=self.max_health;
		self.flags (+) FL_MONSTER | FL_FLY;
		self.movetype = MOVETYPE_FLY;
		self.takedamage=DAMAGE_YES;
		self.touch= imp_touch;
		self.th_die = imp_die_init;
		self.spawnflags (-) SF_IMP_DORMANT;

		self.artifact_active (-) ARTFLAG_STONED;
		sound (self, CHAN_VOICE, "fx/wallbrk.wav", 1, ATTN_NORM);
		while(chunk_cnt < CHUNK_MAX)
		{
			CreateModelChunks(self.size,.7);
			chunk_cnt+=1;
		}
		self.skin=self.oldskin;
		if(self.skin)
			self.classname="monster_imp_ice";
		else
			self.classname="monster_imp_fire";

		self.thingtype=THINGTYPE_FLESH;
		self.scale=1;
	}
	else if(self.frame==$impup23)
		self.think=imp_fly;
};

void imp_pain_anim2 () [++ $impup6 .. $impup14]
{
	check_pos_enemy();
	if(self.frame==$impup14)
	{
		if (self.spawnflags & SF_IMP_DORMANT) 
		{
			self.frame=$impup1;
			self.think=stone_imp_awaken;
		}
		else
			self.think=imp_hover;
		thinktime self : 0;
	}
}

void imp_pain_anim1 () [-- $impup14 .. $impup6]
{
	check_pos_enemy();
	if(self.frame==$impup6)
	{
		self.think=imp_pain_anim2;
		thinktime self : 0;
	}
}

void(entity attacker, float damage) imp_pain =
{
	if(self.pain_finished>time)
		return;

	if(self.targetname!=""&&self.skin==2)
	{
		self.think=SUB_Null;
		self.nextthink=-1;
		return;
	}

	if(random()<0.5&&self.pain_finished!=-666)//FIXME: make more logical
		return;

	self.pain_finished=time+3;//only react to pain once every 3 seconds max

	if(self.attack_state==AS_FERRY&&random()<0.2)
		imp_drop();

//FIXME: pain sound
	self.think=imp_pain_anim1;
	thinktime self : 0;
};

void imp_use (void)
{
	self.use=SUB_Null;
	self.targetname="";

	if(activator.classname=="player")
		self.enemy=self.goalentity=activator;
	else
		dprint("ERROR: monster not activated by player!\n");

	self.frame=$impup1;
	self.think=stone_imp_awaken;
	thinktime self : random();
}

float imp_find_target(void)
{	// Imp in waiting state

	if (self.spawnflags & SF_IMP_DORMANT)
		return FALSE;

	if (LocateTarget())
	{	// We found a target
		self.goalentity = self.enemy;
		self.think = self.th_run;
		thinktime self : 0;
		//self.nextthink = time + 6;//why 6?
		return TRUE;
	}
	return FALSE;
}

void imp_wait() [++ $impwat1 .. $impwat24]
{
	if(imp_find_target())
		return;
}

void imp_fly () [++ $impfly1 .. $impfly20]
{
	imp_move();

	if(imp_check_defense())
		return;

	if(imp_check_attack())
		return;

	if(imp_new_action())
		return;
}

void imp_awaken () [++ $impup1 .. $impup23]
{
	check_pos_enemy();
	movestep(0,0,imp_up_amounts[self.frame - $impup1], FALSE);
	walkmove(self.angles_y, imp_up_amounts[self.frame - $impup1] / 2.0, FALSE);

	if (self.frame == $impup23)
	{
		self.monster_awake = TRUE;
		self.think = imp_fly;
		thinktime self : 0;
	}
}


void impmonster_start_go ()
{
	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 5.0;
	self.view_ofs = '0 0 25';
	if(!self.use)
		self.use = monster_use;

	self.pausetime = 99999999;

	if(self.targetname)
	{
		self.frame=$impwat1;
		self.think=imp_use;
		self.nextthink=-1;
	}
	else
		self.th_stand ();
}

void impmonster_start ()
{
	thinktime self : random(0.5);

	self.think = impmonster_start_go;
	total_monsters = total_monsters + 1;
}

void init_imp (float which_skin)
{
	if (deathmatch&&self.wait!=-1&&self.classname!="monster_imp_lord")
	{
		remove(self);
		return;
	}

	if(!self.flags2&FL_SUMMONED)
	{
		precache_model2 ("models/imp.mdl");
		precache_model2 ("models/h_imp.mdl");//empty for now
		if (self.classname == "monster_imp_lord")
		{
			precache_model2 ("models/shardice.mdl");
			precache_model ("models/fireball.mdl");
		}
		else if (self.classname == "monster_ice_imp"||self.classname == "monster_imp_ice")
			precache_model2 ("models/shardice.mdl");
		else
			precache_model ("models/fireball.mdl");
		precache_model2 ("models/ring.mdl");
		precache_sound2("imp/up.wav");
		precache_sound2("imp/die.wav");
		precache_sound2("imp/swoophit.wav");
		precache_sound2("imp/swoop.wav");
		precache_sound2("imp/fly.wav");
		precache_sound2("imp/fireball.wav");
		precache_sound2("imp/shard.wav");
	}

	self.solid = SOLID_SLIDEBOX;
	setmodel (self, "models/imp.mdl");
	if (self.classname == "monster_imp_lord")
	{
		self.drawflags(+)SCALE_ORIGIN_BOTTOM;
		self.scale=2.5;
		setsize (self, '-32 -32 0', '32 32 56');
		self.hull=HULL_SCORPION;
	}
	else
	{
		setsize (self, '-16 -16 0', '16 16 36');
		self.hull=HULL_CROUCH;
	}
	self.headmodel = "models/h_imp.mdl";

	if(self.wait!=-1)
	{
		self.movetype = MOVETYPE_FLY;
		self.takedamage=DAMAGE_YES;

		self.skin = which_skin;
		self.impType = which_skin;
		if(which_skin==3)
			self.flags (+) FL_COLDHEAL|FL_FIREHEAL;
		else if(which_skin==1)
			self.flags (+) FL_COLDHEAL;
		else
			self.flags (+) FL_FIREHEAL;

		self.flags2 (+) FL_ALIVE;
		self.thingtype=THINGTYPE_FLESH;
		if (self.classname == "monster_imp_lord")
		{
			self.max_health=self.health = 400;
//			self.experience_value = 3000;
			self.mass = 10;
		}
		else
		{
			self.max_health=self.health = 100;
//			self.experience_value = 500;
			self.mass = 3;
		}
		self.mintel = 5;
		
		if (self.spawnflags & MONSTER_HOVER)
		{
			self.th_stand = imp_hover;
			self.th_walk = imp_fly;
		}	
		else
		{
			self.th_stand = imp_wait;
			self.th_walk = imp_awaken;
		}
		self.th_run = self.th_walk;
		self.th_pain = imp_pain;
		self.th_die = imp_die_init;
		self.th_missile = imp_enter_swoop;
		self.th_melee = imp_attack;
		self.touch = imp_touch;
	}

	self.yaw_speed=8;
	self.speed=10;
	self.attack_state = AS_STRAIGHT;
	self.level=0;
	self.monster_awake = FALSE;
	self.view_ofs=self.proj_ofs='0 0 25';

	if (self.spawnflags & SF_IMP_DORMANT) 
	{
		self.classname="gargoyle";
		self.scale=1.5;
		self.artifact_active (+) ARTFLAG_STONED;
		setsize (self, '-16 -16 0', '16 16 36');
		self.hull=HULL_CROUCH;
		self.takedamage=DAMAGE_NO_GRENADE;
		self.thingtype=THINGTYPE_GREYSTONE;
		self.movetype=MOVETYPE_PUSHPULL;
		self.touch=obj_push;
		self.health+=100;
		self.mass=100;
		self.th_die = chunk_death;
		if(self.wait!=-1)
		{
			self.use = imp_use;
			self.oldskin = self.skin;
			self.th_stand = imp_wait;
		}
		self.skin=2;
	}
	else
		self.flags (+) FL_MONSTER | FL_FLY;

	if(self.wait!=-1)
		impmonster_start();
	else
		self.frame = $impwat1;
}

/*QUAKED monster_imp_ice (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x DORMANT
Grunt monster - common.  Shoots multiple ice shards. Can only be killed by defrosting it.

DORMANT = uses the grey stone texture to make it look like a gargoyl- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_ice ()
{
	init_imp(1);
}

/*QUAKED monster_imp_fire (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x DORMANT
Grunt monster - common.  Shoots a fireball. Can only be killed by defrosting it.

DORMANT = uses the grey stone texture to make it look like a gargoyl- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_fire ()
{
	init_imp(0);
}

/*QUAKED monster_imp_lord (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x DORMANT
Grunt monster - common.  Shoots multiple ice shards. Can only be killed by defrosting it.

DORMANT = uses the grey stone texture to make it look like a gargoyl- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_lord ()
{
	init_imp(3);
}
