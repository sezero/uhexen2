/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/sheep.hc,v 1.1.1.1 2004-11-29 11:40:29 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\players\SHEEP\ambient\sheep.hc
MG
==============================================================================
*/

// For building the model
$cd Q:\art\models\players\SHEEP\ambient
$origin 20 0 11
$base BASE SKIN
$skin SKIN
$flags 0

//
$frame grazeA1      grazeA2      grazeA3      grazeA4      grazeA5      
$frame grazeA6      grazeA7      grazeA8      grazeA9      grazeA10     
$frame grazeA11     grazeA12     grazeA13     grazeA14     grazeA15     
$frame grazeA16     grazeA17     grazeA18     grazeA19     grazeA20     
$frame grazeA21     grazeA22     grazeA23     grazeA24     grazeA25     
$frame grazeA26     grazeA27     grazeA28     grazeA29     grazeA30     
$frame grazeA31     grazeA32     grazeA33     grazeA34     grazeA35     
$frame grazeA36     grazeA37     grazeA38     grazeA39     grazeA40     

//
$frame grazeB1      grazeB2      grazeB3      grazeB4      grazeB5      
$frame grazeB6      grazeB7      grazeB8      grazeB9      grazeB10     
$frame grazeB11     grazeB12     grazeB13     grazeB14     grazeB15     
$frame grazeB16     grazeB17     grazeB18     grazeB19     grazeB20     
$frame grazeB21     grazeB22     grazeB23     grazeB24     grazeB25     
$frame grazeB26     grazeB27     grazeB28     grazeB29     grazeB30     
$frame grazeB31     grazeB32     grazeB33     grazeB34     grazeB35     
$frame grazeB36     grazeB37     grazeB38     grazeB39     grazeB40     

//
$frame gstepA10     gstepA11     gstepA12     gstepA13     gstepA14     
$frame gstepA15     gstepA16     gstepA17     gstepA18     gstepA19     
$frame gstepA20     gstepA21     gstepA22     gstepA23     gstepA24     
$frame gstepA25     gstepA26     gstepA27     gstepA28     gstepA29     
$frame gstepA30     

//
$frame gstepB10     gstepB11     gstepB12     gstepB13     gstepB14     
$frame gstepB15     gstepB16     gstepB17     gstepB18     gstepB19     
$frame gstepB20     gstepB21     gstepB22     gstepB23     gstepB24     
$frame gstepB25     gstepB26     gstepB27     gstepB28     gstepB29     
$frame gstepB30     

//
$frame lookup1      lookup2		 lookup3      lookup4      lookup5      
$frame lookup6      lookup7      lookup8      lookup9      lookup10      
$frame lookup11     lookup12     look1        look2        look3        
$frame look4        look5        look6        look7        look8        
$frame look9        look10       look11       look12       look13       
$frame look14       look15       look16       look17       look18       
$frame look19       look20       look21       look22       look23       
$frame look24       look25       look26       look27       look28       
$frame look29       look30       look31       look32       look33       
$frame look34       look35       look36       look37       look38       
$frame look39       look40       look41       look42       look43       
$frame look44       look45       look46       look47       look48       
$frame look49       look50       look51       look52       look53       
$frame look54       look55       look56       look57       look58       
$frame look59       look60       

//
$frame pain1        pain2        pain3        pain4        pain5        
$frame pain6        pain7        pain8        

//
$frame tranA1       tranA2       tranA3       tranA4       tranA5       
$frame tranA6       tranA7       tranA8       tranA9       tranA10      

//
$frame tranB1       tranB2       tranB3       tranB4       tranB5       
$frame tranB6       tranB7       tranB8       tranB9       tranB10      
$frame tranB11      tranB12      

//
$frame trot1        trot2        trot3        trot4        trot5        
$frame trot6        trot7        trot8        trot9        trot10       

//
$frame wait1        wait2        wait3        wait4        wait5        
$frame wait6        wait7        wait8        wait9        wait10       
$frame wait11       wait12       wait13       wait14       wait15       
$frame wait16       

float stationary = 1;
/*QUAKED player_sheep (0.3 0.1 0.6) (-8 -8 -0) (8 8 32) stationary STUCK JUMP PLAY_DEAD DORMANT NODROP
A sheep player model
-------------------------FIELDS-------------------------
stationary- sheep will not wander around or change angles
nodrop- won't drop to floor (and through other entities!) on spawn
--------------------------------------------------------
*/
void()sheep_tranA;
void()sheep_tranB;
void()sheep_graze_a;
void()sheep_graze_b;
void()sheep_gstep_a;
void()sheep_gstep_b;
void()sheep_trot;
void()sheep_wait;
void()sheep_pain;
void()sheep_lookdown;
void()sheep_look;
void()sheep_lookup;

void restore_monster ()
{
//SOUND and MODEL
	newmis=spawn();
	thinktime newmis : 0;
	newmis.think=self.th_spawn;
	newmis.skin=self.skin;
	newmis.health=self.max_health;
	remove(self);
}

void sheep_sound (float vol)
{
float r;
string playsound;
	if(self.noise!=""&&self.classname!="player")
		playsound=self.noise;
	else
	{
		r=rint(random(1,3));
		if(r==1)
			playsound="misc/sheep1.wav";
		else if(r==2)
			playsound="misc/sheep2.wav";
		else
			playsound="misc/sheep3.wav";
	}
	sound(self,CHAN_VOICE,playsound,vol,ATTN_NORM);
	self.pain_finished=time + 1;
}

void sheep_turn (void)
{
	if(random()<0.5)
		self.angles_y+=random()*self.yaw_speed+self.yaw_speed/2;
	else
		self.angles_y-=random()*self.yaw_speed+self.yaw_speed/2;

}

void sheep_move (float dist)
{
vector best_yaw;
	best_yaw=normalize(self.goalentity.origin-self.origin);
	best_yaw=vectoangles(best_yaw);
	self.ideal_yaw=best_yaw_y;
	ChangeYaw();
	movetogoal(dist);
}

void sheep_think (void)
{
	MonsterCheckContents();
	if(!self.spawnflags&stationary&&(self.think==sheep_trot||self.think==sheep_gstep_a||self.think==sheep_gstep_b)&&random()<0.1)
		sheep_turn();

	if(random()<0.1&&random()<0.2&&self.pain_finished<time)
		sheep_sound(random(0.2,0.8));

	if(self.flags&FL_ONGROUND)
		self.last_onground=time;
}

void()sheep_tranA = [++ $tranA1 .. $tranA10]
{
float r;
	if(self.target)
		sheep_move(3);
	else if(!walkmove(self.angles_y,3,FALSE))
		sheep_turn();

	sheep_think();
	if(cycle_wrapped)
	{
		r=rint(random(1,3));
		if(r==1)
			sheep_wait();
		else if(r==2)
			sheep_tranB();
		else if(r==3)
			sheep_lookdown();
	}
};

void()sheep_tranB = [++ $tranB1 .. $tranB12]
{
	if(self.target)
		sheep_move(3);
	else if(!walkmove(self.angles_y,3,FALSE))
		sheep_turn();

	sheep_think();
	if(cycle_wrapped)
		sheep_trot();
};

void()sheep_graze_a = [++ $grazeA1 .. $grazeA40]
{
	sheep_think();
	if(self.frame== $grazeA9 &&random()<0.5&&!self.spawnflags&1)
		sheep_gstep_b();
	else if(cycle_wrapped&&random()<0.5)
		sheep_lookup();
};

void () sheep_graze_b = [++ $grazeB1 .. $grazeB40]
{
	sheep_think();
	if(self.frame== $grazeB9 &&random()<0.5&&!self.spawnflags&1)
		sheep_gstep_a();
};

void () sheep_gstep_a = [++ $gstepA10 .. $gstepA30]
{
float r;
	sheep_think();
	if(self.target)
		sheep_move(0.3);
	else if(!walkmove(self.angles_y,0.3,FALSE))
		sheep_turn();

	if(cycle_wrapped)
	{
		r=rint(random())+1;
		if(r==1)
		{
			self.frame==$grazeA31;
			sheep_graze_a();
		}
		else
			sheep_gstep_b();
	}
};

void () sheep_gstep_b = [++ $gstepB10 .. $gstepB30]
{
float r;
	sheep_think();
	if(self.target)
		sheep_move(0.3);
	else if(!walkmove(self.angles_y,0.3,FALSE))
		sheep_turn();

	if(cycle_wrapped)
	{
		r=rint(random())+1;
		if(r==1)
		{
			self.frame==$grazeB31;
			sheep_graze_b();
		}
		else
			sheep_gstep_a();
	}
};

void()sheep_trot = [++ $trot1 .. $trot10]
{
	if(self.target)
		sheep_move(3);
	else if(!walkmove(self.angles_y,3,FALSE))
		sheep_turn();

	sheep_think();
	if(cycle_wrapped)
		sheep_tranA();
};

void()sheep_wait = [++ $wait1 .. $wait16]
{
float r;
	sheep_think();
	if(cycle_wrapped)
	{
		r=rint(random(2))+1;
		if(r==1)
			sheep_wait();
		else if(r==2&&!self.spawnflags&1)
			sheep_tranB();
		else if(r==3)
			sheep_lookdown();
	}
};

void()sheep_pain = [++ $pain1 .. $pain8]
{
float r;
	ai_pain(1);
	if(cycle_wrapped)
	{
		r=rint(random(2))+1;
		if(r==1)
			sheep_wait();
		else if(r==2&&!self.spawnflags&1)
			sheep_tranB();
		else if(r==3)
			sheep_lookdown();
	}
	else if(self.frame==$pain1)
		sheep_sound(1);
};

void()sheep_lookdown = [-- $lookup12 .. $lookup1]
{
float r;
	if(cycle_wrapped)
	{
		r=rint(random(4))+1;
		if(r==1)
			sheep_graze_a();
		else if(r==2)
			sheep_graze_b();
		else if(r==3&&!self.spawnflags&1)
			sheep_gstep_a();
		else if(r==4&&!self.spawnflags&1)
			sheep_gstep_b();
		else
			sheep_lookup();
	}
};

void () sheep_look = [++ $look1 .. $look60]
{
float r;
	sheep_think();
	if(cycle_wrapped)
	{
		r=rint(random(2))+1;
		if(r==1)
			sheep_wait();
		else if(r==2&&!self.spawnflags&1)
			sheep_tranB();
		else if(r==3)
			sheep_lookdown();
	}
};

void () sheep_lookup = [++ $lookup1 .. $lookup12]
{
	sheep_think();
	if(cycle_wrapped)
		sheep_look();
};

void monster_sheep_bite ()
{
	makevectors(self.angles);
	traceline(self.origin+self.view_ofs,self.origin+self.view_ofs+v_forward*36,FALSE,self);
	if(trace_ent.takedamage)//classname=="obj_catapult2")
	{
		sound(self,CHAN_VOICE,"spider/bite.wav",1,ATTN_NORM);
		SpawnPuff (trace_endpos, '0 0 0', 3,trace_ent);
		T_Damage(trace_ent,self,self,5);
		self.attack_finished=time+random(0.2,1);
	}
}

void monster_sheep_run () [++ $trot1 .. $trot10]
{
	MonsterCheckContents();
	if(!self.enemy.flags2&FL_ALIVE||!visible(self.enemy))
	{
		self.think=sheep_trot;
		thinktime self : 0;
	}
	else
	{
		if(random()<0.1&&random()<0.2)
			sheep_sound(1);
		ai_face();
		movetogoal(4);
//		if(self.frame==$trot1 ||self.frame==$trot6)
//			sound(self, CHAN_BODY, "scorpion/walk.wav", random(0.7, 1), ATTN_NORM);
		if(infront(self.enemy))
			if(range(self.enemy)==RANGE_MELEE&&self.attack_finished<time)
				monster_sheep_bite();
	}
}

void player_sheep (void)
{
float r;

	if(self.th_spawn==SUB_Null)
		self.th_spawn=player_sheep;
	else
	{
		self.health=self.max_health;
		if(self.health>25)
			self.health=25;
	}

	CreateEntityNew(self,ENT_SHEEP,"models/sheep.mdl",chunk_death);

	self.th_pain = sheep_pain;
	self.touch	= obj_push;
	self.flags(+)FL_PUSH;	
	self.flags2(+)FL_ALIVE;
	self.yaw_speed=2;

//FIXME: Don't allow trans, trot, etc, if stationary flag on
	r=rint(random(1,3));
	if(r==1)
		self.noise="misc/sheep1.wav";
	else if(r==2)
		self.noise="misc/sheep2.wav";
	else
		self.noise="misc/sheep3.wav";

	if(self.enemy)
		self.think=monster_sheep_run;
	else
	{
		r=rint(random(1,11));
		if(r==1)
			self.think=sheep_graze_a;
		else if(r==2)
			self.think=sheep_graze_b;
		else if(r==3&&!self.spawnflags&1)
			self.think=sheep_gstep_a;
		else if(r==4&&!self.spawnflags&1)
			self.think=sheep_gstep_b;
		else if(r==5)
			self.think=sheep_look;
		else if(r==6)
			self.think=sheep_lookup;
		else if(r==7&&!self.spawnflags&1)
			self.think=sheep_trot;
		else if(r==8)
			self.think=sheep_wait;
		else if(r==9)
			self.think=sheep_lookdown;
		else if(r==10&&!self.spawnflags&1)
			self.think=sheep_tranA;
		else if(!self.spawnflags&1)
			self.think=sheep_tranB;
	}

	if(!self.think)
		self.think=sheep_wait;

	self.th_walk=self.th_stand=self.think;
	walkmonster_start();
}

/*
============================
PLAYER SHEEP
============================
*/
void()player_sheep_run;

void player_sheep_snout_slowpuff ()
{
	self.wfs = advanceweaponframe(22,31);
	self.th_weapon=player_sheep_snout_slowpuff;
}

void player_sheep_snout_fastpuff ()
{
	self.wfs = advanceweaponframe(17,21);
	self.th_weapon=player_sheep_snout_fastpuff;
	if(self.wfs==WF_CYCLE_WRAPPED)
		self.weaponframe_cnt+=1;
	if(self.weaponframe_cnt>5)
	{
		self.weaponframe_cnt=0;
		player_sheep_snout_slowpuff();
	}
}

void player_sheep_snout_bite ()
{
	self.wfs = advanceweaponframe(8,16);
	self.th_weapon=player_sheep_snout_bite;
	if(self.wfs==WF_CYCLE_WRAPPED)
		player_sheep_snout_fastpuff();
}

void player_sheep_snout_pain1 ()
{
	self.wfs = advanceweaponframe(0,3);
	self.th_weapon=player_sheep_snout_pain1;
	if(self.wfs==WF_CYCLE_WRAPPED)
		player_sheep_snout_fastpuff();
}

void player_sheep_snout_pain2 ()
{
	self.wfs = advanceweaponframe(4,7);
	self.th_weapon=player_sheep_snout_pain2;
	if(self.wfs==WF_CYCLE_WRAPPED)
		player_sheep_snout_fastpuff();
}

void()	player_sheep_stand =[++$wait1..$wait16]
{
	self.th_weapon();
	if (self.velocity_x || self.velocity_y)
		self.think=player_sheep_run;
};

void()	player_sheep_run =[++$trot1..$trot10]
{
	self.th_weapon();
	if (!self.velocity_x && !self.velocity_y)
		self.think=player_sheep_stand;
};

void player_sheep_baa ()
{
	self.th_weapon=player_sheep_snout_bite;
	self.th_weapon();
	traceline(self.origin+self.view_ofs,self.origin+self.view_ofs+v_forward*36,FALSE,self);
	if(trace_ent.takedamage)//classname=="obj_catapult2")
	{
		sound(self,CHAN_VOICE,"spider/bite.wav",1,ATTN_NORM);
		SpawnPuff (trace_endpos, '0 0 0', 3,trace_ent);
		T_Damage(trace_ent,self,self,5);
	}
	else
		sheep_sound(1);

	self.attack_finished=time+0.5;

	if(!self.velocity_x && !self.velocity_y)
		self.think=player_sheep_stand;
	else
		self.think=player_sheep_run;
	thinktime self : 0;
}

void()	player_sheep_pain=[++$pain1..$pain8]
{
	if(self.pain_finished<time)
		if(random()<0.5)
			self.th_weapon=player_sheep_snout_pain1;
		else
			self.th_weapon=player_sheep_snout_pain2;
	self.th_weapon();
	if(cycle_wrapped)
	{
		if(!self.velocity_x && !self.velocity_y)
			self.think=player_sheep_stand;
		else
			self.think=player_sheep_run;
	}
	else if(self.frame==$pain1)
		sheep_sound(1);
};

void()	player_sheep_jump=[++$trot1..$trot10]//[++$jump1..$jump10]
{
	self.th_weapon();
	if(cycle_wrapped)
	{
		if(!self.velocity_x && !self.velocity_y)
			self.think=player_sheep_stand;
		else
			self.think=player_sheep_run;
	}
};

void Polymorph (entity loser)
{
//MODEL
	sound(loser,CHAN_VOICE,"misc/sheepfly.wav",1,ATTN_NORM);
//FIXME: Need to make it so this reverses after 30 sec.
	loser.sheep_time=time+30;
	GenerateTeleportEffect(loser.origin,1);

	if(loser.classname=="player")
	{
		if(loser.effects&EF_BRIGHTLIGHT&&loser.playerclass==CLASS_CRUSADER)
			loser.effects(-)EF_BRIGHTLIGHT;//Sunstaff

		if (loser.artifact_active & ART_TOMEOFPOWER)
			loser.tome_time=0;

		if(loser.hull!=HULL_CROUCH)
			if (loser.hasted)
				loser.hasted*= .6;
			else
				loser.hasted = .6;
		loser.mass=3;
		if(loser.health>25)
			loser.health=25;
		loser.th_missile=player_sheep_baa;
		loser.th_melee=player_sheep_baa;
		loser.th_stand=player_sheep_stand;
		loser.th_run=player_sheep_run;
		loser.th_walk=player_sheep_run;
		loser.th_pain=player_sheep_pain;
		loser.th_jump=player_sheep_jump;

		setmodel (loser, "models/sheep.mdl");
		setsize (loser,'-16 -16 0','16 16 28');
		loser.model="models/sheep.mdl";
//		loser.modelindex=modelindex_sheep;
//		loser.headmodel="models/h_sheep.mdl";
		loser.th_weapon=player_sheep_snout_pain1;
		loser.hull=HULL_CROUCH;
//		if(!loser.flags2&FL_CAMERA_VIEW)
			loser.view_ofs = '0 0 24';
		loser.proj_ofs='0 0 18';
		loser.attack_finished=0;
		loser.weapon=FALSE;
		loser.weaponmodel="models/snout.mdl";
		loser.weaponframe=0;
		loser.sheep_sound_time=FALSE;

		loser.think=player_sheep_stand;
		thinktime loser : 0;
	}
	else
	{
//Stop all auto-looping sounds on removed monster
		sound(loser,CHAN_BODY,"misc/null.wav",1,ATTN_NONE);
		sound(loser,CHAN_WEAPON,"misc/null.wav",1,ATTN_NONE);
		sound(loser,CHAN_ITEM,"misc/null.wav",1,ATTN_NONE);
		newmis=spawn();
		setorigin(newmis,loser.origin);

//For restoring monster:
		newmis.th_spawn=loser.th_spawn;
		newmis.skin=0;
		newmis.oldskin=loser.skin;
		newmis.max_health=loser.health;
		if(!loser.enemy)
			newmis.enemy=self.owner;
		else
			newmis.enemy=loser.enemy;
		newmis.goalentity=newmis.enemy;
		newmis.angles=loser.angles;
		newmis.target=loser.target;//So it will still activate targets
		newmis.killtarget=loser.killtarget;
		remove(loser);
		newmis.flags2(+)FL_SUMMONED;
		newmis.spawnflags(+)NO_DROP;
		newmis.think=player_sheep;
		thinktime newmis : 0;
	}
}

void poly_touch ()
{
	if(other.monsterclass >= CLASS_BOSS&&other!=self.owner)
	{
		self.velocity=normalize((self.owner.absmin+self.owner.absmax)*0.5-self.origin)*700;
		self.owner=other;
	}
	else if(other.flags2&FL_ALIVE&&other.model!="models/sheep.mdl"&&other.classname!="monster_golem_crystal")
	{
		self.touch=SUB_Null;
		Polymorph(other);
		remove(self);
	}
	else if(other.movetype!=MOVETYPE_FLYMISSILE&&other.movetype!=MOVETYPE_BOUNCE&&other.movetype!=MOVETYPE_BOUNCEMISSILE)
	{
		self.touch=SUB_Null;
		particleexplosion(self.origin,random(144,159),self.absmax_z-self.absmin_z,10);
		remove(self);
	}
}

void polymorph_anim () [++ 0 .. 3]
{
	particle4(self.origin,7,random(144,159),PARTICLETYPE_EXPLODE2,random(1,5));
}

void FirePoly (float ofs)
{
	makevectors(self.v_angle);
	newmis=spawn();
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.owner=self;
	newmis.touch=poly_touch;
	newmis.speed=700;
	newmis.velocity=v_forward*newmis.speed+v_right*ofs;
	
	newmis.drawflags=MLS_POWERMODE;

	newmis.think=polymorph_anim;
	thinktime newmis : 0;
	setmodel(newmis,"models/polymrph.spr");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs+v_forward*10);
}

void Use_Polymorph ()
{
	FirePoly(-150);
	FirePoly(-75);
	FirePoly(0);
	FirePoly(75);
	FirePoly(150);
	self.cnt_polymorph -= 1;
}
