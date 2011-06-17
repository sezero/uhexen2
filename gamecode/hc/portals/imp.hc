/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/imp.hc,v 1.2 2006-10-12 13:03:42 sezero Exp $
 */
/*
==============================================================================

IMP
MG&RJ

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


//============================================================
float MONSTER_STAND_GROUND = 1;
float MONSTER_HOVER        = 2;
float SF_IMP_DORMANT		= 16;
float PICKUP				= 32;

void()imp_fly;
void()imp_hover;
void()imp_abort_swoop;
void()imp_touch;
float()imp_find_target;
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
	self.enemy.flags(-)FL_FLY;
	if(self.classname=="monster_imp_lord")
	{
		if(self.enemy.classname!="player")
			self.enemy.health=10;
		self.enemy.movetype=MOVETYPE_BOUNCE;
		self.enemy.mass=10000;
		self.enemy.velocity_z=-300;
	}
}

void summoned_imp_die () [-- $impup23 .. $impup1]
{
	if(self.health<-40)
	{
		stopSound(self,CHAN_BODY);
		//sound (self, CHAN_BODY, "misc/null.wav", 1, ATTN_NORM);
		self.flags2(-)FL_ALIVE; /* THOMAS: otherwise Eidolon gets confused. */
		chunk_death();
		return;
	}
	
	if(self.frame==$impup23)
	{
		self.velocity='0 0 0';
		self.effects=EF_DIMLIGHT;
		self.movetype=MOVETYPE_NOCLIP;
		self.drawflags=SCALE_ORIGIN_CENTER;
		self.flags=0;
		self.solid=SOLID_NOT;
		self.avelocity_y=-200;
		self.count=0;
		sound (self, CHAN_VOICE, "imp/diebig.wav", 1, ATTN_NORM);
	}

	self.velocity='0 0 0';
	self.scale-=0.025;
	self.avelocity_y-=10;

	self.count+=1;
	if(self.count==4)
		self.count=0;
	else if(self.frame<$impup23)
		self.frame+=1;

	if(self.frame==$impup1)
	{
		sound (self, CHAN_BODY, "items/itmspawn.wav", 1, ATTN_NORM);
		CreateRedFlash(self.origin);
		self.flags2(-)FL_ALIVE; /* THOMAS: otherwise Eidolon gets confused. */
		remove(self);
	}
}

void imp_die ()
{
	self.touch=SUB_Null;
	if(self.health<-30)
	{
		stopSound(self,CHAN_BODY);
		//sound (self, CHAN_BODY, "misc/null.wav", 1, ATTN_NORM);
		chunk_death();
		return;
	}
	if(self.frame!=$death14)
		AdvanceFrame($death1,$death14);

	if(self.frame==$death1)
	{
		self.flags(-)FL_ONGROUND;
		if(self.skin==3)
			sound (self, CHAN_BODY, "imp/diebig.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_BODY, "imp/die.wav", 1, ATTN_NORM);
	
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
	
	MonsterCheckContents();
	
	if(self.frame==$death14 &&self.flags&FL_ONGROUND)
		MakeSolidCorpse();
	else if(self.health<-30)
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
			self.hull=HULL_SCORPION;//HYDRA;
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
}

void imp_ferry ()
{
float dist;
vector org;
	movetogoal(self.speed);
/*
	if(!check_z_move(self.level))
		imp_up_down();
*/
	self.velocity_z=self.goalentity.origin_z-self.origin_z;
	if(fabs(self.velocity_z)>self.speed*30)
		if(self.velocity_z<0)
			self.velocity_z=self.speed*-30;
		else
			self.velocity_z=self.speed*30;
	self.flags(-)FL_ONGROUND;
//	dprintf("Boost : %s\n",self.velocity_z);

	org=self.goalentity.origin;
	dist=vlen(self.origin-org);
	if(dist<=self.size_x)
	{
		imp_drop();
		return;
	}

	org=self.enemy.origin;
	org_z=self.enemy.absmax_z;
	dist=vlen(self.origin-org);
	if(dist>200)
	{
		imp_drop();
		return;
	}
	else
		self.enemy.velocity=normalize(self.origin-org)*dist*5;
	self.enemy.angles=self.angles;

	MonsterCheckContents();
}

void imp_pick_up (void)
{
	self.attack_state=AS_FERRY;
	self.goalentity=find(world,targetname,self.target);
	self.enemy.flags(+)FL_FLY;
}

void imp_set_speeds ()
{
float anglediff,dist;
	if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
		dist=4;
	else	
		dist=8;	//Movement distance this turn

	anglediff=	fabs(self.angles_y - self.ideal_yaw);	//How far we're trying to turn

	if (anglediff > 20)   // If it is a big distance to turn, then don't move as far forward
	   dist = dist / 1.5;

	self.level=imp_fly_amounts[self.frame - $impfly1];
	self.speed = dist + self.level*4*self.scale;	//tweaks to speed based on anim frame

	if(!visible(self.enemy))
	{
		if(self.mintel)
			SetNextWaypoint();
		if(self.search_time<time&&self.goalentity==self.enemy&&self.trigger_field.classname=="waypoint")
			self.goalentity=self.trigger_field;
		self.level*=4;
	}
	else
	{
		self.search_time=time+5;	//If lose, right, keep searching for 5 secs
		self.goalentity=self.enemy;
	    self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
	}
}

float imp_check_too_close ()
{
float enemy_zdiff,enemy_hdist;
	if(!visible(self.enemy))
		return FALSE;

	if(self.attack_state==AS_FERRY)
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
	if((self.skin==3&&self.enemy==self.controller&&vlen(self.enemy.origin-self.origin)<128)||(self.goalentity==world||self.enemy==world))
	{
		self.think=self.th_stand;
		return;
	}

	if(self.attack_state!=AS_FERRY)
		checkenemy();
	
	self.velocity=self.velocity*(1/1.05);
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

/*	if(self.goalentity.classname=="waypoint")
	{
		dprint("Following waypoints");
		if(self.search_time<time)
			dprint("exclusively\n");
		else
			self.goalentity=self.enemy;
	}
*/
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
	if(self.attack_state==AS_SLIDING)
	{	
		enemy_yaw = vectoyaw(self.goalentity.origin - self.origin);
		movedist=self.speed;
		ai_run_slide();
	}

	movestep(0,0,self.level, FALSE);//slight up/down movement

	if(!check_z_move(self.level))
		imp_up_down();

	if(random()<0.2)
	if(self.enemy!=world&&self.search_time>=time)
	{
		enemy_vis=visible(self.enemy);
		if(!enemy_vis||!clear_path(self.enemy,FALSE))
		{
		vector go_dir;
		float r,minspeed,maxspeed;
			makevectors(self.angles);
			minspeed=100*self.scale;
			maxspeed=300*self.scale;
			r=random();
			if(r<0.25)
				go_dir=check_axis_move(v_right,minspeed,maxspeed);
			else if(r<=0.5)
				go_dir=check_axis_move(v_up,minspeed,maxspeed);
			else if(r<=0.75)
				go_dir=check_axis_move((v_right+v_up)*0.5,minspeed,maxspeed);
			else
				go_dir=check_axis_move((v_right*-1+v_up)*0.5,minspeed,maxspeed);
			if(go_dir!='0 0 0')
			{
				if(r<0.25)
				{
					self.velocity_x=go_dir_x;
					self.velocity_y=go_dir_y;
				}
				else if(r<=0.5)
					self.velocity_z=go_dir_z;
				else
					self.velocity=go_dir;
				self.flags(-)FL_ONGROUND;
			}
		}
	}

	if(self.spawnflags&PICKUP)
		if(random()<0.2)
			if(self.target!="")
				if(self.origin_z>self.enemy.absmax_z - 8)
					if(vlen(self.enemy.origin+self.enemy.proj_ofs-self.origin)<=self.size_x*1.5)
						imp_pick_up();

	MonsterCheckContents();
}

float imp_new_action ()
{
float too_close;
	enemy_vis=visible(self.enemy);
	too_close=imp_check_too_close();
	if((random()<0.7&&self.enemy.flags2&FL_ALIVE)||!enemy_vis||too_close||self.attack_state==AS_FERRY)
	{
		if(self.think!=imp_fly&&self.enemy!=world)
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
		if(skill>=4)
			self.attack_finished=0;
		else
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
	if(self.enemy.last_attack<time - 1||self.attack_state==AS_FERRY)
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

void imp_missile ()
{
	if(self.classname == "monster_imp_ice"||(self.classname == "monster_imp_lord"&&random()<0.5))
	{
		sound (self, CHAN_WEAPON, "imp/shard.wav", 1, ATTN_NORM);

		makevectors (self.angles);
		do_shard('14 8 0'*self.scale,360 + random()*150, '0 0 0');
		do_shard('14 8 0'*self.scale,360 + random()*150, (v_forward * ((random() * 40) - 20)) +
														  (v_right * ((random() * 40) - 20)) +
														  (v_up * ((random() * 20) - 10)));
		do_shard('14 8 0'*self.scale,360 + random()*150, (v_forward * ((random() * 40) - 20)) +
														  (v_right * ((random() * 40) - 20)) +
														  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0'*self.scale,360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0'*self.scale,360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
		if (random() < 0.5)
			do_shard('14 8 0'*self.scale,360 + random()*150, (v_forward * ((random() * 40) - 20)) +
															  (v_right * ((random() * 40) - 20)) +
															  (v_up * ((random() * 20) - 10)));
	}
	else
	{
		sound (self, CHAN_WEAPON, "imp/fireball.wav", 1, ATTN_NORM);
		do_fireball('14 8 0'*self.scale);
	}
}

void imp_melee ()
{
vector org,destiny;
float dist,damg;
	makevectors(self.angles);
	org=self.origin+self.proj_ofs+v_forward*16*self.scale;
	destiny=(self.enemy.absmin+self.enemy.absmax)*0.5;
	dist=vlen(destiny-org);
	traceline(org,destiny,FALSE,self);
	if(dist>48*self.scale||trace_fraction==1)
	{
		imp_missile();
		return;
	}
	if(trace_ent.takedamage)
	{
	string hitsound;
		if(self.skin==3)
		{
			if(trace_ent.thingtype==THINGTYPE_FLESH)
				MeatChunks (trace_endpos,v_right*random(-100,-300)+'0 0 200', 3,trace_ent);
			hitsound="weapons/slash.wav";
		}
		else
			hitsound="assassin/chntear.wav";
		sound(trace_ent,CHAN_AUTO,hitsound,1,ATTN_NORM);
		if(self.skin==3)
			damg=40;
		else
			damg=10*self.scale;
		T_Damage(trace_ent,self,self,damg);
	}
	else
		hitsound="weapons/gauntht2.wav";
	SpawnPuff(trace_endpos,v_right*-100,10*self.scale,trace_ent);
	sound(self,CHAN_AUTO,hitsound,1,ATTN_NORM);
}

void imp_attack(void)
{
	self.last_attack=time;
	if (vlen(self.origin-self.enemy.origin) <= 64*self.scale)
	{
		self.v_angle=self.angles;
		imp_melee ();
	}
	else
		imp_missile();
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
		if(skill>=4)
			self.attack_finished=0;
		else
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
		if(skill>=4)
			self.attack_finished=0;
		else
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
		if(skill>=4)
			self.attack_finished=0;
		else
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
	self.last_attack=time;
	check_pos_enemy();
	destiny=self.enemy.origin+self.enemy.proj_ofs;
	org=(self.absmin+self.absmax)*0.5;
	enemy_vis=visible(self.enemy);
	enemy_infront=infront(self.enemy);
	enemy_range=vlen(destiny - org);
	if(self.enemy.last_attack>time - 1 &&fov(self,self.enemy,45))
	{
		self.velocity_z+=150;
		imp_abort_swoop();
	}
	else if(enemy_vis&&enemy_infront&&enemy_range<2000)
	{
		dir=normalize(destiny-org);
		self.velocity=dir*(377+self.count*7);
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
vector vec,org;
	check_pos_enemy();
	if(self.frame==$swoop1)
	{
		self.yaw_speed=15;
		self.count=140;
		self.touch = imp_touch;
		self.velocity = '0 0 0';

		if(self.skin==3)
			sound (self, CHAN_VOICE, "imp/swoopbig.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "imp/swoop.wav", 1, ATTN_NORM);
	}

	ai_face();
	self.count *= 1.15;

	if (self.frame >= $swoop12)
	{  // Start to swoop down
		org=self.origin;
		org_z=self.absmin_z;
		vec = normalize(self.enemy.origin - org + self.enemy.proj_ofs);
		self.velocity = vec*self.count;
		if (self.frame <= $swoop13 )
		{  // If we haven't pulled out yet, keep going straight down
			self.velocity_x = self.velocity_y = 0;
		}
		if(self.absmin_z - self.enemy.absmax_z>50&&self.frame==$swoop13)
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
		self.touch = imp_touch;

		if(self.skin==3)
			sound (self, CHAN_VOICE, "imp/swoopbig.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "imp/swoop.wav", 1, ATTN_NORM);
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
float damg,damg_plus;
vector punch,dir;

	
	self.touch=SUB_Null;
	if((self.frame >= $swpcyc1 && self.frame <= $swpcyc4 )||(self.frame>=$swoop16 &&self.frame<=$swoop20))
	{  // If we are in swoop attack frames
		self.flags (-) FL_ONGROUND;
		sound (self, CHAN_WEAPON, "imp/swoophit.wav", 1, ATTN_NORM);

		self.think=imp_swoop_end;
		if (other.takedamage)
		{  // We sucessfully hit something

			if(self.frame >= $swpcyc1 && self.frame <= $swpcyc4)
				damg_plus=self.count;
			else
				damg_plus=(self.frame-$swoop16)*5;

			if(self.classname == "monster_imp_lord")
			{
				damg = (33 + (damg_plus *5));
				T_Damage (other, self, self.controller, damg);
				if(other.monsterclass<CLASS_BOSS)
				{
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
			}
			else
			{
				damg = (10 + (damg_plus / 2));
				T_Damage (other, self, self, damg);
			}
			if(other.classname=="player"&&other.flags2&FL_ALIVE)
			{
				makevectors(other.v_angle);
				dir=normalize(self.velocity)*-1;
				punch_y=v_forward*dir;
				punch_z=v_right*dir;
				punch_x=v_up*dir;
				other.punchangle=punch*(damg/10);
			}
		}
		else if(vlen(self.velocity)>400)
		{
			makevectors(self.angles);
			SpawnPuff(self.origin+self.proj_ofs+v_forward*self.absmax_x,'0 0 0',10,self);
			self.pain_finished=-666;
			self.think=self.th_pain;
		}
		self.velocity=other.velocity;//??
	}
	else if(vlen(self.velocity)>300)
		sound (self, CHAN_WEAPON, "imp/swoophit.wav", 1, ATTN_NORM);
};

float imp_check_attack ()
{
float enemy_hdist,enemy_zdiff,swoop_no_drop;
vector destiny,org;
	if(self.enemy==world)
	{
		self.think=self.th_stand;
		return FALSE;
	}

	if(self.enemy==self.controller||self.enemy==self)
		if(!LocateTarget())
			return FALSE;

	if(self.attack_state==AS_FERRY)
		return FALSE;

	enemy_vis=visible(self.enemy);
	if (!enemy_vis)
	{
		if(self.mintel)
			SetNextWaypoint();
		if(self.search_time<time&&self.goalentity==self.enemy&&self.trigger_field.classname=="waypoint")
			self.goalentity=self.trigger_field;
		self.attack_state = AS_STRAIGHT;
		return FALSE;
	}
	else if(self.mintel)
	{
		self.goalentity=self.enemy;
	    self.wallspot=(self.enemy.absmin+self.enemy.absmax)*0.5;
	}

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

	if(!clear_path (self.enemy,FALSE))
		if(!CanDamage(self.enemy,self))
			return FALSE;
	
	if(self.skin!=3)
		if(random()<0.5-skill/10)
			return FALSE;

	if(!self.spawnflags & MONSTER_STAND_GROUND)
	{
	float swoop_no_drop;
	vector min,max;
		swoop_no_drop=FALSE;
		
		enemy_hdist=vhlen(destiny-org);
		if(self.skin==3)
			enemy_zdiff=self.absmin_z - destiny_z;
		else
			enemy_zdiff=org_z - destiny_z;
		if(enemy_zdiff<=36&&random()<0.3)
			swoop_no_drop=TRUE;
		
		if(self.skin!=3)
		{
			min='-16 -16 0';
			max='16 16 28';
		}
		else
		{
			min='-48 -48 42';
			max='48 48 42';
		}
		
		if(enemy_hdist>70+30*self.scale&&(enemy_zdiff>36||swoop_no_drop))
		{
			tracearea(org,org-'0 0 1'*enemy_zdiff,min,max,FALSE,self);
			if(trace_fraction==1||swoop_no_drop)
			{
				if(swoop_no_drop)
					tracearea(org,destiny,min,max,FALSE,self);
				else
					tracearea(org-'0 0 1'*enemy_zdiff,destiny,min,max,FALSE,self);
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
	
	if(self.skin==0)//fire imps attack less- try to get close
		if(random()<0.5)
			return FALSE;

	self.think=imp_attack_anim;
	thinktime self : 0;
	return TRUE;
}

void imp_hover () [++ $impfly1 .. $impfly20]
{
float too_close;
	if (self.frame == $impfly1)
	{
		if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
			self.noise="hydra/turn-s.wav";
		else if(self.skin==3)
			self.noise="imp/flybig.wav";
		else
			self.noise="imp/fly.wav";
		sound (self, CHAN_BODY, self.noise, 1, ATTN_NORM);
	}

	if(self.skin==3)
		if(self.lifetime<time||!self.controller.flags2&FL_ALIVE||self.controller.imp_count!=self.imp_count)
		{
			self.think=summoned_imp_die;
			thinktime self : 0;
			return;
		}

	if(self.enemy)
		if(self.attack_state!=AS_FERRY)
			checkenemy();

	if(!self.enemy||(self.enemy==self.controller&&self.skin==3))
		if(imp_find_target())
			return;

	if(self.enemy)
	{
		self.velocity=self.velocity*(1/1.05);
		ai_face();
		imp_set_speeds();
		check_z_move(self.level);
	
		if(imp_check_defense())
			return;

		if(imp_check_attack())
			return;

		if(self.enemy!=world)
		{
			too_close=imp_check_too_close();
			enemy_vis=visible(self.enemy);
			if(!enemy_vis||too_close||self.attack_state!=AS_STRAIGHT)
			{
				self.think=imp_fly;
				thinktime self : 0;
				return;
			}
		}
	}

	if(self.enemy!=world||self.goalentity!=world)
		if(imp_new_action())
			return;

	MonsterCheckContents();
}

void() stone_imp_awaken = [++ $impup7 .. $impup23]
{
	if(self.frame==$impup10)
	{
		self.ideal_yaw = vectoyaw(self.enemy.origin - self.origin);
		self.count = 0;
		setsize (self, '-16 -16 0', '16 16 36');
		self.origin_z-=14;
		self.hull=HULL_CROUCH;
		self.mass = 3;
		self.health=self.max_health;
		self.flags (+) FL_MONSTER | FL_FLY;
		self.movetype = MOVETYPE_FLY;
		self.takedamage=DAMAGE_YES;
		self.touch= SUB_Null;
		self.th_die = imp_die_init;
		self.spawnflags (-) SF_IMP_DORMANT;

		self.artifact_active (-) ARTFLAG_STONED;
		sound (self, CHAN_VOICE, "fx/wallbrk.wav", 1, ATTN_NORM);
		while(chunk_cnt < CHUNK_MAX)
		{
			CreateModelChunks(self.size,.7, 1);
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
	ai_pain(2);
	if(self.frame==$impup14)
	{
		self.touch=SUB_Null;
		self.think=imp_hover;
		thinktime self : 0;
	}
}

void imp_pain_anim1 () [-- $impup14 .. $impup6]
{
	check_pos_enemy();
	ai_pain(2);
	if(self.frame==$impup6)
	{
		self.think=imp_pain_anim2;
		thinktime self : 0;
	}
}

void(entity attacker, float damage) imp_pain =
{
	if(self.monster_awake)
		if(self.pain_finished>time)
			return;

	if(self.targetname!=""&&self.skin==2)
	{
		self.think=SUB_Null;
		self.nextthink=-1;
		return;
	}

	if(random()<0.5&&self.pain_finished!=-666&&attacker!=world&&self.touch!=SUB_Null&&self.monster_awake)//FIXME: make more logical
		return;

	self.monster_awake=TRUE;

	self.pain_finished=time+3;//only react to pain once every 3 seconds max

	if(self.attack_state==AS_FERRY&&random()<0.2)
		imp_drop();

//FIXME: pain sound
	if (self.spawnflags & SF_IMP_DORMANT) 
	{
		self.frame=$impup1;
		self.think=stone_imp_awaken;
	}
	else
	{
		if(self.skin==3)
			sound(self,CHAN_VOICE,"imp/upbig.wav",1,ATTN_NORM);
		else
			sound(self,CHAN_VOICE,"imp/up.wav",1,ATTN_NORM);
		self.think=imp_pain_anim1;
	}
	thinktime self : 0;
};

void imp_use (void)
{
	if (!self.flags2&FL_ALIVE)
		return;
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
	if (LocateTarget())
	{	// We found a target
		if (self.skin==2) 
		{
			float self_infront, self_vis, enemy_dist, r1, r2;
			self_infront=infront_of_ent(self,self.enemy);
			self_vis=visible2ent(self,self.enemy);
			enemy_dist=vlen(self.origin-self.enemy.origin);
			r1=random();
			r2=random();
			if((self_infront&&self_vis&&r1<0.1&&r2<0.5&&enemy_dist<1000)||enemy_dist<=RANGE_MELEE)
			{
				self.goalentity = self.enemy;
				self.think=stone_imp_awaken;
				thinktime self : 0;
				return TRUE;
			}
			else
			{
				self.goalentity=self.enemy=world;
				return FALSE;
			}
		}
		else
		{
			self.goalentity = self.enemy;
			self.think = self.th_run;
			thinktime self : 0;
			return TRUE;
		}
	}
	else if(self.classname=="monster_imp_lord")
		self.enemy=self.goalentity=self.controller;
	return FALSE;
}

void imp_wait() [++ $impwat1 .. $impwat24]
{

	if(self.skin==2)
		self.frame=$impwat1;
	else if(!self.flags&FL_ONGROUND)
	{
		self.think=imp_hover;
		thinktime self : 0;
	}

	if(random()<0.5)
		if(imp_find_target())
			return;
}

void imp_fly () [++ $impfly1 .. $impfly20]
{
	if (self.frame == $impfly1)
	{
		if(pointcontents(self.origin+self.view_ofs)==CONTENT_WATER)
			self.noise="hydra/turn-s.wav";
		else if(self.skin==3)
			self.noise="imp/flybig.wav";
		else
			self.noise="imp/fly.wav";
		sound (self, CHAN_BODY, self.noise, 1, ATTN_NORM);
	}

	if(self.skin==3)
		if(self.lifetime<time||!self.controller.flags2&FL_ALIVE||self.controller.imp_count!=self.imp_count)
		{
			self.think=summoned_imp_die;
			thinktime self : 0;
			return;
		}

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
	if (self.frame == $impup1)
		if(self.skin==3)
			sound (self, CHAN_VOICE, "imp/upbig.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "imp/up.wav", 1, ATTN_NORM);

	check_pos_enemy();
	movestep(0,0,imp_up_amounts[self.frame - $impup1], FALSE);
	walkmove(self.angles_y, imp_up_amounts[self.frame - $impup1] / 2.0, FALSE);

	if (self.frame == $impup23)
	{
		self.think = imp_fly;
		thinktime self : 0;
	}
}

void impmonster_start_go ()
{
	self.ideal_yaw = self.angles * '0 1 0';
	if (!self.yaw_speed)
		self.yaw_speed = 5.0;
	if(!self.use)
		self.use = monster_use;

	self.pausetime = 99999999;

	if(self.targetname!="")
	{
		self.frame=$impwat1;
		self.think=imp_use;
		self.nextthink=-1;
	}
	else if(self.enemy!=world)
		imp_awaken();
	else
		self.th_stand();
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

	if (!self.flags2 & FL_SUMMONED&&!self.flags2&FL2_RESPAWN)
	{
		precache_model4 ("models/imp.mdl");//converted for MP
		precache_model ("models/h_imp.mdl");//empty for now
		if (self.classname == "monster_imp_lord")
		{
			precache_model ("models/shardice.mdl");
			precache_model ("models/fireball.mdl");
			precache_sound ("imp/upbig.wav");
			precache_sound ("imp/diebig.wav");
			precache_sound ("imp/swoopbig.wav");
			precache_sound ("imp/flybig.wav");
		}
		else
		{
			precache_sound ("imp/up.wav");
			precache_sound ("imp/die.wav");
			precache_sound ("imp/swoop.wav");
			precache_sound ("imp/fly.wav");
			if (self.classname == "monster_imp_ice")
				precache_model ("models/shardice.mdl");
			else
				precache_model ("models/fireball.mdl");
		}
		precache_sound ("imp/swoophit.wav");
		precache_sound ("imp/fireball.wav");
		precache_sound ("imp/shard.wav");
		precache_sound("hydra/turn-s.wav");
	}

	self.solid = SOLID_SLIDEBOX;
	setmodel (self, "models/imp.mdl");
	if (self.classname == "monster_imp_lord")
	{
		self.drawflags(+)SCALE_ORIGIN_CENTER;
		self.scale=2.3;//2?
		setsize (self, '-32 -32 -32', '32 32 32');
		self.hull=HULL_SCORPION;//HYDRA;
		self.view_ofs=self.proj_ofs='0 0 82';
	}
	else
	{
		self.scale=1;
		setsize (self, '-16 -16 0', '16 16 36');
		self.view_ofs=self.proj_ofs='0 0 33';
		self.hull=HULL_CROUCH;
	}
	self.headmodel = "models/h_imp.mdl";

	if(which_skin==3)
		self.flags2 (+) FL2_COLDHEAL|FL2_FIREHEAL;
	else if(which_skin==1)
		self.flags2 (+) FL2_COLDHEAL;
	else
		self.flags2 (+) FL2_FIREHEAL;

	if(self.wait!=-1)
	{
		self.movetype = MOVETYPE_FLY;
		self.takedamage=DAMAGE_YES;

		self.skin = which_skin;
		self.impType = which_skin;

		self.flags2 (+) FL_ALIVE;
		self.thingtype=THINGTYPE_FLESH;
		if (self.classname == "monster_imp_lord")
		{
			self.max_health=self.health = 600;
			self.experience_value = 3000;
			self.mass = 10;
			self.th_die = summoned_imp_die;
		}
		else
		{
			if(!self.health)
				self.max_health=self.health = 75+self.skin*25;
			self.experience_value = 400 +self.skin*100;
			self.mass = 3;
			self.th_die = imp_die_init;
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
		self.th_missile = imp_enter_swoop;
		self.th_melee = imp_attack;
	}

	self.yaw_speed=8;
	self.speed=10;
	self.attack_state = AS_STRAIGHT;
	self.level=0;

	if (self.spawnflags & SF_IMP_DORMANT) 
	{
		self.classname="gargoyle";
		self.scale=1.5;
		self.artifact_active (+) ARTFLAG_STONED;
		setsize (self, '-16 -16 -14', '16 16 22');
		self.origin_z+=14;
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

	if(!self.max_health)
		self.max_health=self.health;
	self.init_exp_val = self.experience_value;

	total_monsters += 1;
	if(self.enemy)
		self.th_run();
	else if(self.wait!=-1)
		impmonster_start();
	else
		self.frame = $impwat1;
}

/*QUAKED monster_imp_ice (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x gargoyle x FROZEN
Grunt monster - common.  Shoots multiple ice shards. Can only be killed by defrosting it.
immune to ice attacks

gargoyle = uses the grey stone texture to make it look like a gargoyle- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_ice ()
{
	if(!self.th_init)
	{
		self.th_init=monster_imp_ice;
		self.init_org=self.origin;
	}
	init_imp(1);
}

/*QUAKED monster_imp_fire (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x gargoyle x FROZEN
Grunt monster - common.  Shoots a fireball. Can only be killed by defrosting it.

gargoyle = uses the grey stone texture to make it look like a gargoyle- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_fire ()
{
	if(!self.th_init)
	{
		self.th_init=monster_imp_fire;
		self.init_org=self.origin;
	}
	init_imp(0);
}

/*QUAKED monster_imp_lord (1 0.3 0) (-16 -16 0) (16 16 55) STAND HOVER x x gargoyle x FROZEN
Big imp dude- kicks butt and takes names

gargoyle = uses the grey stone texture to make it look like a gargoyle- will wake up if the player looks at him long enough, gets close, or hurts him.
-------------------------FIELDS-------------------------
wait = if you give it a -1, the gargoyle will not come alive, it's just a decoration
--------------------------------------------------------

*/
void monster_imp_lord ()
{
	if(self.flags2&FL_SUMMONED)
		self.spawnflags(+)MONSTER_HOVER;
	init_imp(3);
}
