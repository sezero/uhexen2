/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/mezzoman.hc,v 1.4 2007-02-07 16:57:07 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\monsters\mezzoman\FINAL\mezzoman.hc
MG!!!
==============================================================================
*/

// For building the model
$cd Q:\art\models\monsters\mezzoman\FINAL
$origin 0 0 -2
$base BASE skin1
$skin skin1
$skin Skin2
$flags 16384

//
$frame block1       block2       block3       block4       block5       
$frame block6       

//
$frame charge1      charge2      charge3      charge4      charge5      
$frame charge6      charge7      charge8      charge9      charge10     
$frame charge11     charge12     charge13     charge14     charge15     
$frame charge16     charge17     charge18     charge19     charge20     
$frame charge21     charge22     charge23     charge24     charge25     

//
$frame clober1      clober2      clober3      clober4      clober5      
$frame clober6      clober7      clober8      clober9      clober10     
$frame clober11     clober12     clober13     clober14     clober15     
$frame clober16     

//
$frame death1       death2       death3       death4       death5       
$frame death6       death7       death8       death9       death10      
$frame death11      death12      death13      death14      death15      
$frame death16      

//
$frame dive1        dive2        dive3        dive4        dive5        
$frame dive6        dive7        dive8        dive9        dive10       
$frame dive11       dive12       dive13       dive14       dive15       
$frame dive16       dive17       dive18       

//
$frame jump1        jump2        jump3        jump4        jump5        
$frame jump6        jump7        jump8        jump9        jump10       
$frame jump11       jump12       jump13       jump14       jump15       
$frame jump16       jump17       jump18       jump19       jump20       
$frame jump21       jump22       

//
$frame pain1        pain2        pain3        pain4        pain5        
$frame pain6        pain7        

//
$frame roar1        roar2        roar3        roar4        roar5        
$frame roar6        roar7        roar8        roar9        roar10       
$frame roar11       roar12       roar13       roar14       roar15       
$frame roar16       roar17       roar18       roar19       roar20       
$frame roar21       roar22       roar23       roar24       roar25       
$frame roar26       roar27       roar28       roar29       roar30       

//
$frame Roll1        Roll2        Roll3        Roll4        Roll5        
$frame Roll6        Roll7        Roll8        Roll9        Roll10       
$frame Roll11       Roll12       Roll13       Roll14       Roll15       
$frame Roll16       Roll17       Roll18       

//
$frame run1         run2         run3         run4         run5         
$frame run6         run7         run8         run9         run10        
$frame run11        run12        run13        run14        run15        
$frame run16        run17        run18        run19        run20        
$frame run21        run22        

//
$frame stand1       stand2       stand3       stand4       stand5       
$frame stand6       stand7       stand8       stand9       stand10      

//
$frame sword1       sword2       sword3       sword4       sword5       
$frame sword6       sword7       sword8       sword9       sword10      
$frame sword11      sword12      sword13      

//
$frame twirl1       twirl2       twirl3       twirl4       twirl5       
$frame twirl6       twirl7       twirl8       twirl9       twirl10      

//
$frame walk1        walk2        walk3        walk4        walk5        
$frame walk6        walk7        walk8        walk9        walk10       
$frame walk11       walk12       walk13       walk14       walk15       
$frame walk16       walk17       walk18       walk19       walk20       
$frame walk21       walk22       walk23       walk24       walk25       
$frame walk26       walk27       walk28       walk29       walk30       


void() mezzo_skid;
void() mezzo_block;
void() mezzo_block_wait;
void() mezzo_jump;
void() mezzo_roar;
void() mezzo_in_air;
void() mezzo_run_loop;
void()mezzo_charge;

void mezzo_idle_sound ()
{
string soundstr;
	if(random()<0.5)
		soundstr="mezzo/snort.wav";
	else
		soundstr="mezzo/growl.wav";
	sound(self,CHAN_VOICE,soundstr,1,ATTN_NORM);
}

/*
void mezzo_possum_up (void)// [-- $death14..$death1]
{
	if (cycle_wrapped)
		self.think=self.th_run;
}

void mezzo_playdead (void)
{
//	self.frame=$death14;
	self.think=mezzo_playdead;
	thinktime self : 0.1;
	ai_stand();
}
*/

void mezzo_roll_right () [-- $Roll18 .. $Roll1]
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
//SOUND?
vector rollangle;
	makevectors(self.angles);
	rollangle=vectoangles(v_right);
//	if(!walkmove(rollangle_y,7,FALSE)&&self.frame>$Roll5 &&self.flags&FL_ONGROUND)
//		self.frame=$Roll5;
	walkmove(rollangle_y,7,FALSE);
	if(cycle_wrapped)
	{
		thinktime self : 0;
		if(!self.flags&FL_ONGROUND)
			self.think=mezzo_in_air;
		else
			self.think=self.th_run;
	}
}

void mezzo_roll_left () [++ $Roll1 .. $Roll18]
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
//SOUND?
vector rollangle;
	makevectors(self.angles);
	rollangle=vectoangles(v_right);
//	if(!walkmove(rollangle_y,-7,FALSE)&&self.frame<$Roll14 &&self.flags&FL_ONGROUND)
//		self.frame=$Roll14;
	walkmove(rollangle_y,-7,FALSE);
	if(cycle_wrapped)
	{
		thinktime self : 0;
		if(!self.flags&FL_ONGROUND)
			self.think=mezzo_in_air;
		else
			self.think=self.th_run;
	}
}

void mezzo_roll_forward () [++ $dive1 .. $dive18]
{
//SOUND?
//vector rollangle;
//	if(!walkmove(self.angles_y,7,FALSE)&&self.frame<$dive12 &&self.flags&FL_ONGROUND)
//		self.frame=$dive12;
	if(!self.flags&FL_ONGROUND)
	{
		if(!infront(self.enemy))//stay facing enemy so if land behind him, will be facing him
			ai_face();
	}
	else
	{
		if(self.dflags)
		{
			sound(self,CHAN_BODY,"player/land.wav",1,ATTN_NORM);
			self.dflags=FALSE;
		}
		walkmove(self.angles_y,7,FALSE);
	}

	if(cycle_wrapped)
	{
		thinktime self : 0;
		if(!self.flags&FL_ONGROUND)
			self.think=mezzo_in_air;
		else
			self.think=self.th_run;
	}
}

void mezzo_duck () [++ $jump13 .. $jump22]
{
//FIXME:  Have him keep checking for befense when staying down for .5 sec
vector newmaxs;
	if(self.frame==$jump14)
	{
		newmaxs=self.maxs;
		newmaxs_z=self.maxs_z*0.5;
		setsize(self,self.mins,newmaxs);
	}
	else if(self.frame==$jump18)
	{
		newmaxs=self.maxs;
		newmaxs_z=self.maxs_z*2;
		setsize(self,self.mins,newmaxs);
	}
	else if(self.frame==$jump16)
		thinktime self : 0.5;
	else if(cycle_wrapped)
	{
		thinktime self : 0;
		self.think=self.th_run;
	}
}

float mezzo_check_duck (entity proj)
{
vector proj_mins,duck_hite,proj_dir;
	proj_mins=proj.origin;
	proj_mins_z=proj.origin_z - proj.mins_z;

	duck_hite=self.origin;
	duck_hite_z=self.origin_z + self.maxs_z/2;

	proj_dir=normalize(duck_hite-proj_mins);

	traceline(proj_mins,duck_hite+proj_dir*8,FALSE,self);

	if(trace_ent!=self||trace_endpos_z>duck_hite_z)
		return TRUE;
	else
		return FALSE;
}

float mezzo_check_jump (entity proj)
{
float impact_hite, jump_hite;
vector proj_dir, proj_top;

	if(!self.flags&FL_ONGROUND)
		return FALSE;

	proj_dir=normalize(proj.velocity);
	proj_top=proj.origin;
	proj_top_z=proj.absmax_z;
	traceline(proj_top,proj_top+proj_dir*1000,FALSE,proj);
	if(trace_ent!=self)
		return FALSE;

	impact_hite=trace_endpos_z;
	tracearea(self.origin,self.origin+'0 0 256',self.mins,self.maxs,FALSE,self);
	jump_hite=trace_fraction*256;
	if(jump_hite<24)
		return FALSE;
	else if(jump_hite>133)
		jump_hite=133;		
	else if(jump_hite<77)
		jump_hite=77;		

	if(self.origin_z+jump_hite/2>impact_hite+proj.maxs_z&&random()<0.7)
	{
		self.velocity_z=jump_hite*3;
		self.flags(-)FL_ONGROUND;
		return TRUE;
	}

	return FALSE;
}

void mezzo_choose_roll (entity proj)
{
float proj_dir;
	proj_dir=check_heading_left_or_right(proj);
	if(proj_dir==0)
	{
		if(mezzo_check_duck(proj))
		{
			thinktime self : 0;
			self.think=mezzo_duck;
		}
		else
		{
			thinktime self : 0;
			self.think=mezzo_block;
		}
		return;
	}
	else
	{
//FIXME: Probably shouldn't try to roll in the other direction
		makevectors(self.angles);
		if(solid_under(self.origin, self.origin+v_right*105*proj_dir))
			tracearea(self.origin, self.origin+v_right*105*proj_dir,self.mins,self.maxs,FALSE,self);
		else
			trace_fraction=FALSE;
		if(trace_fraction==1)
		{
			traceline(trace_endpos, trace_endpos-'0 0 300',TRUE,self);
			if(pointcontents(trace_endpos)!=CONTENT_EMPTY)
				trace_fraction=FALSE;
			else
				trace_fraction=TRUE;
		}
		if(trace_fraction==1)		
		{
			if(proj_dir>0)
			{
				thinktime self : 0;
				self.think=mezzo_roll_right;
			}
			else
			{
				thinktime self : 0;
				self.think=mezzo_roll_left;
			}
			return;
		}
		else if(mezzo_check_duck(proj))
		{
			thinktime self : 0;
			self.think=mezzo_duck;
		}
		else
		{
			thinktime self : 0;
			self.think=mezzo_block;
		}
/*		{
			tracearea(self.origin, self.origin+v_right*36*proj_dir*-1,self.mins,self.maxs,FALSE,self);
			if(trace_fraction==1)
			{
				if(proj_dir*-1>0)
				{
					thinktime self : 0;
					self.think=mezzo_roll_right;
				}
				else
				{
					thinktime self : 0;
					self.think=mezzo_roll_left;
				}
				return;
			}
		}
*/
	}
}

void mezzo_check_defense ()
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
//NOTE: Add random chance of failure based on difficulty level - highest diff means no chance of failure here
	if(skill+self.skin/5<random(6))
		return;

	if((self.enemy.last_attack+0.5<time&&self.oldenemy.last_attack+0.5<time)||self.aflag)
		return;

entity enemy_proj;
float r;
	enemy_proj=look_projectiles();

	if(!enemy_proj)
		if(lineofsight(self,self.enemy))
		{
			enemy_proj=self.enemy;
			self.level=vlen(self.enemy.origin-self.origin)/1000;
		}
		else
			return;

	if(self.flags&FL_ONGROUND)
		self.velocity='0 0 0';//Clear velocity from last jump so he doesn't go nuts

	r=range(enemy_proj);
	if(self.enemy.weapon==IT_WEAPON1&&r<=RANGE_NEAR)
	{
		thinktime self : 0;
		if(r==RANGE_MELEE)
		{
			if(random()<0.7||self.enemy.v_angle_x>=0)
				self.think=mezzo_duck;
			else if(self.think==mezzo_block_wait)
			{
				self.t_width=time+1;
				return;
			}
			else if(self.think==mezzo_run_loop||random()<0.3)
				self.think=mezzo_charge;
			else
				self.think=mezzo_block;
			return;
		}
		else if(random()<0.5)
			mezzo_choose_roll(enemy_proj);
		else if(random()<0.7||self.enemy.v_angle_x>=0)
			self.think=mezzo_duck;
		else if(self.think==mezzo_block_wait)
		{
			self.t_width=time+1;
			return;
		}
		else if(self.think==mezzo_block_wait)
		{
			self.t_width=time+1;
			return;
		}
		else if(self.think==mezzo_run_loop||random()<0.3)
			self.think=mezzo_charge;
		else
			self.think=mezzo_block;
		return;
	}

	if(self.level>0.3)//I've got 0.3 seconds before impact
	{
		mezzo_choose_roll(enemy_proj);
		return;
	}
	else if(mezzo_check_duck(enemy_proj))
	{
		thinktime self : 0;
		tracearea(self.origin,self.origin+v_forward*64,self.mins,'16 16 20',FALSE,self);
		if(trace_fraction<1||random()<0.2||!infront(enemy_proj))
			self.think=mezzo_duck;
		else
			self.think=mezzo_roll_forward;
		return;
	}
	else if(mezzo_check_jump(enemy_proj))
	{
		self.think=mezzo_jump;
		enemy_infront=infront(self.enemy);
		enemy_vis=visible(self.enemy);
		trace_fraction=0;
		if((random()<0.3||self.think==mezzo_run_loop)&&enemy_infront&&enemy_vis&&self.enemy!=world)//Jump towards enemy
		{
		vector enemy_dir;
			enemy_dir=normalize(self.enemy.origin-self.origin);
			traceline(self.origin,self.origin+enemy_dir*64+'0 0 56',FALSE,self);
			if(trace_fraction==1)
			{
				traceline(trace_endpos,trace_endpos-'0 0 300',TRUE,self);
				if(pointcontents(trace_endpos)==CONTENT_EMPTY)
				{
					self.velocity_x=self.velocity_y=0;
					self.velocity+=enemy_dir*vlen(self.enemy.origin-self.origin);
					trace_fraction=1;
					if(random()<0.7)
						self.think=mezzo_roll_forward;
					else
						self.think=self.th_jump;
				}
				else
				{
//					dprint("might land in water or lava\n");
					trace_fraction=0;
				}
			}
			else
			{
//				dprint("not enough room in front \n");
				trace_fraction=0;
			}
		}
//		dprint(ftos(trace_fraction));
//		dprint(" is the trace_fraction\n");
		if(random()<0.5&&trace_fraction<1)//Jump to side
		{
//			dprint("checking to sides\n");
			if(random()<0.5)
				r=1;
			else
				r=-1;
			makevectors(self.angles);
			traceline(self.origin,self.origin+v_right*36*r+'0 0 56',FALSE,self);
			if(trace_fraction<1)
			{
				traceline(self.origin,self.origin-v_right*36*r+'0 0 56',FALSE,self);
				if(trace_fraction<1)
				{
//					dprint("not enough room to jump on that side\n");
					self.think=mezzo_jump;
				}
				else 
				{
					traceline(trace_endpos,trace_endpos-'0 0 300',TRUE,self);
					if(pointcontents(trace_endpos)!=CONTENT_EMPTY)
					{
//						dprint("might jump into water or lava\n");
						self.think=mezzo_jump;
					}
					else
					{
						self.velocity-=v_right*r*200;
						if(r*-1>0)
							self.think=mezzo_roll_right;
						else
							self.think=mezzo_roll_left;
					}
				}
			}
			else 
			{
				traceline(trace_endpos,trace_endpos-'0 0 300',TRUE,self);
				if(pointcontents(trace_endpos)!=CONTENT_EMPTY)
				{
//					dprint("might jump into water or lava\n");
					self.think=mezzo_jump;
				}
				else
				{
					self.velocity+=v_right*r*200;
					if(r>0)
						self.think=mezzo_roll_right;
					else
						self.think=mezzo_roll_left;
				}
			}
		}
		thinktime self : 0;
		if(self.think!=mezzo_jump&&self.think!=mezzo_roll_right&&self.think!=mezzo_roll_left&&self.think!=mezzo_roll_forward)
		{
//			dprint("What the FUCK!!!\n");
			self.think=mezzo_jump;
		}
//		return;
	}
	else if(infront(enemy_proj)&&random()<0.5)
	{
		thinktime self : 0;
		if(self.think==mezzo_block_wait)
		{
			self.t_width=time+1;
			return;
		}
		else if(self.think==mezzo_run_loop||random()<0.3)
			self.think=mezzo_charge;
		else
			self.think=mezzo_block;
		return;
	}
}

void() mezzo_charge_stop;
void mezzo_slam ()
{
	if(!other.movetype||other.mass>100||other.solid==SOLID_BSP)
		return;

	if(!infront(other)||other.safe_time>time)
		return;

	if(other.origin_z>self.absmax_z - 6||other.absmax_z<self.origin_z + 6)
		return;

	sound(self,CHAN_VOICE,"mezzo/slam.wav",1,ATTN_NORM);
	
float inertia;
	if(other.mass<10)
		inertia=1;
	else
		inertia=other.mass/10;

vector punchdir;
	makevectors(self.angles);
	punchdir=v_forward*300+'0 0 100';
	T_Damage(other,self,self,5*(self.skin+1)*(self.aflag+1)*(coop + 1));
	other.velocity+=punchdir*(1/inertia);
	other.flags(-)FL_ONGROUND;

	self.ltime+=1;
	other.safe_time=time+1.25;//So can't kill them instantly if they're moving against him or pinned on a wall
	if(self.think!=mezzo_charge_stop)
	{
		thinktime self : 0;
		self.think=mezzo_charge_stop;
	}
}

void mezzo_reflect_trig_touch ()
{
//vector newv;
vector  org, vec, dir;//, endspot,endplane, dif;
float magnitude;//remainder, reflect_count, 

	if (other.flags & FL_MONSTER || other.flags & FL_CLIENT || !other || other == self) return;

	if (other.safe_time>time) return;

	if(!self.owner)	// fix the "assignment to world entity" bug
	{
		remove(self);
		return;
	}

	if(!self.owner.flags2&FL_ALIVE||self.owner.frozen>0)
	{
		if(self.owner.movechain==self)
			self.owner.movechain=world;
		remove(self);
	//	return;	// fix the "assignment to world entity" bug
	}

	if(other.classname=="funnal"||other.classname=="tornato")
		return;

	dir = normalize(other.velocity);
	magnitude=vlen(other.velocity);
	org = other.origin;
	vec = org + dir*100;
	traceline (org, vec, FALSE, other);	

	if(trace_ent!=self.owner)
		return;

	if(self.owner.classname=="monster_mezzoman")
		sound(self,CHAN_AUTO,"mezzo/slam.wav",1,ATTN_NORM);

	if(!self.owner.skin&&self.owner.classname=="monster_mezzoman")
	{//Just block it
		if(!other.flags2&FL_ALIVE)
			other.flags2(+)FL_NODAMAGE;
	}
	else
	{//reflect!
		if(self.owner.classname!="monster_mezzoman")
		{
			sound (self, CHAN_WEAPON, "fangel/deflect.wav", 1, ATTN_NORM);
			CreateWhiteFlash(trace_endpos);
			if(self.owner.classname=="monster_fallen_angel")
			{
				dir=dir*-1;
				makevectors(dir);
				dir=v_forward + v_up*random(-0.75,.75) + v_right*random(-0.75,.75);
				dir=normalize(dir);
			}
			else// if(visible(other.owner))
			{
				v_forward=normalize(other.owner.origin+other.owner.view_ofs-other.origin);
				dir+= 2*v_forward;
				dir=normalize(dir);
			}
//			else
//				dir=dir*-1;
		}
		else
		{
			sound(self,CHAN_AUTO,"mezzo/reflect.wav",1,ATTN_NORM);
			starteffect(CE_MEZZO_REFLECT,self.origin);
			makevectors(trace_ent.angles);
			dir+= 2*v_forward;
			dir=normalize(dir);
		}

		if(other.movedir)
			other.movedir=dir;
		if(other.o_angle)
			other.o_angle=dir;

		if(magnitude<other.speed)
		{
//			dprintf("Low mag : %s\n",magnitude);
			magnitude=other.speed;
		}
		other.velocity = dir*magnitude;
		other.angles = vectoangles(other.velocity);

		self.owner.last_attack=time;
		other.safe_time=time+100/magnitude;

		if(!other.controller)
			other.controller=other.owner;
		if(other.enemy==self.owner)
			other.enemy=other.owner;
		if(other.goalentity==self.owner)
			other.goalentity=other.owner;
		other.owner=self.owner;
	}
}

void reflect_think ()
{
	makevectors(self.owner.angles);
	setorigin(self,self.owner.origin+ v_forward*48+'0 0 40');
	self.think=reflect_think;
	thinktime self : 0.05;
}

void spawn_reflect ()
{
//FIXME: Picks up enemy missile as shield!  shield not necc?
	makevectors(self.angles);
	newmis=spawn();
	self.shield=newmis;
	newmis.movetype = MOVETYPE_NOCLIP;
	newmis.solid = SOLID_TRIGGER;
	newmis.owner = self;
	newmis.touch = mezzo_reflect_trig_touch;
	newmis.classname="mezzo_reflect";
	newmis.effects=EF_NODRAW;

	setmodel(newmis,"models/null.spr");
	if(self.classname=="monster_mezzoman")
	{
		setsize (newmis, '-32 -32 -10','32 32 30');
		setorigin(newmis,self.origin+ v_forward*48+'0 0 40');
  
		newmis.think=reflect_think;
		thinktime newmis : 0;
	}
	else
	{
		self.movechain=newmis;
		setsize (newmis, '-48 -48 -64','48 48 50');
		setorigin(newmis,self.origin);
	}
}

void mezzo_clobber() [++ $clober1 .. $clober16]
{
float zofs;
	ai_charge(1);
	if(self.frame==$clober7)
	{
		makevectors(self.angles);
		
		zofs = self.enemy.origin_z - self.origin_z;
		if(zofs>20)
			zofs=20;
		else if(zofs<-20)
			zofs=-20;

		traceline(self.origin+'0 0 30',self.origin+'0 0 30'+v_forward*36+v_up*zofs,FALSE,self);
		if(trace_fraction==1)
			return;

		sound(self,CHAN_VOICE,"mezzo/slam.wav",1,ATTN_NORM);

		if(trace_ent.movetype&&trace_ent.movetype!=MOVETYPE_PUSH)
			trace_ent.velocity+=v_forward*200-v_right*100+'0 0 100';
		if(trace_ent.takedamage)
			T_Damage(trace_ent,self,self,5*(self.skin+1)*(self.aflag+1)*(coop + 1));
		if(trace_ent.classname=="player")
			if(infront_of_ent(self,trace_ent))
				trace_ent.punchangle_y=4;
	}
	else if(cycle_wrapped)
	{
		self.attack_finished=time+0.5;
		thinktime self : 0;
		self.think=self.th_run;
	}
	else if(self.frame==$clober1)
	{
		self.last_attack=time;
		if(random()<0.5)
			sound(self,CHAN_VOICE,"mezzo/attack.wav",1,ATTN_NORM);
	}
}

void mezzo_sword() [++ $sword1 .. $sword13]
{
	ai_face();
	ai_charge(3);
	if(cycle_wrapped)
	{
		self.attack_finished=time+0.3;
		thinktime self : 0;
		self.think=self.th_run;
	}
	else if(self.frame==$sword1)
	{
		self.last_attack=time;
		sound(self,CHAN_WEAPON,"weapons/vorpswng.wav",1,ATTN_NORM);
		if(random()<0.5)
			sound(self,CHAN_VOICE,"mezzo/attack.wav",1,ATTN_NORM);
	}
	else if(self.frame>=$sword6 && self.frame<=$sword10)
	{
	float ofs,zofs;
	vector dir;
		makevectors(self.angles);
		ofs=($sword10 - self.frame)*4;
		dir_z=ofs - 8;
		dir+=v_right*(ofs - 8)+v_forward*(48 - fabs(16 - ofs));
		dir=normalize(dir);

		zofs = self.enemy.origin_z - self.origin_z;
		if(zofs>20)
			zofs=20;
		else if(zofs<-20)
			zofs=-20;

		traceline(self.origin+'0 0 37',self.origin+'0 0 37'+dir*48+v_up*zofs,FALSE,self);
		if(trace_fraction==1)
			return;

		if(self.t_width<time)
		{
			MetalHitSound(trace_ent.thingtype);
			self.t_width=time+1;
		}

		if(trace_ent.takedamage)
			T_Damage(trace_ent,self,self,2*(self.skin+1)*(self.aflag+1)*(coop + 1));
		if(trace_ent.thingtype==THINGTYPE_FLESH&&self.frame==$sword9)
		{
			MeatChunks (trace_endpos,v_right*random(-100,-300)+'0 0 200', 3,trace_ent);
			sound(self,CHAN_AUTO,"weapons/slash.wav",1,ATTN_NORM);
		}
		SpawnPuff (trace_endpos, '0 0 0', 3,trace_ent);
	}
}

void mezzo_melee ()
{
	if(random()<0.3)
		self.think=mezzo_clobber;
	else
		self.think=mezzo_sword;
	thinktime self : 0;
}

void mezzo_missile ()
{
	if(vlen(self.enemy.origin-self.origin)<84&&lineofsight(self.enemy,self))
		self.think=mezzo_charge;
	else
		self.think=self.th_run;
	thinktime self : 0;
}

void mezzo_die () [++ $death1 .. $death16]
{
	if(self.shield)
		remove(self.shield);
	if (self.health < -40)
	{
		chunk_death();
		return;
	}
	if(self.frame==$death1)
		sound(self,CHAN_VOICE,"mezzo/die.wav",1,ATTN_NORM);
	else if(self.frame==$death12)
		sound(self,CHAN_BODY,"player/land.wav",1,ATTN_NORM);
	else if(self.frame==$death16)
		MakeSolidCorpse();
	thinktime self : 0.1;
}

void mezzo_pain_seq () [++ $pain1 .. $pain7]
{
	ai_back(1);
	if(cycle_wrapped)
	{
		thinktime self : 0;
		if(!self.flags&FL_ONGROUND)
			self.think=mezzo_in_air;
		else
			self.think=self.th_run;
	}
}

void mezzo_pain (entity attacker, float damage)
{
	if(self.monster_awake)
		if(random(self.health)>damage*3||self.pain_finished>time)//only react to 33 percent of current health damage
			return;

	self.monster_awake=TRUE;

	if(self.shield)
		remove(self.shield);

	if(self.health<=100)
	{
		self.th_pain=SUB_Null;
		if(self.health<=100)
		{
			if(random()<0.5)
			{
				self.oldthink=self.th_run;
				self.think=mezzo_roar;
				self.speed=15;
				self.yaw_speed=20;
				self.aflag=TRUE;//Berzerk!
			}
			else if(!self.flags&FL_ONGROUND)
				self.think=mezzo_in_air;
			else
				self.think=self.th_run;
		}
	}
	else
	{
		sound(self,CHAN_VOICE,"mezzo/pain.wav",1,ATTN_NORM);
		if(!self.enemy||!visible(self.enemy))
		{
			if(self.enemy!=world&&self.enemy!=attacker)
				self.oldenemy=self.enemy;
			self.enemy=attacker;
		}
		self.pain_finished=time+1+self.skin;
		self.think=mezzo_pain_seq;
	}
	thinktime self : 0;
}

void mezzo_land () [++ $jump13 .. $jump22]
{
//SOUND?
//	dprint("landing\n");
	if(cycle_wrapped)
	{
		thinktime self : 0;
		self.think=self.th_run;
	}
	else if(self.frame==$jump13)
		sound(self,CHAN_BODY,"player/land.wav",1,ATTN_NORM);
}

void mezzo_in_air ()
{
//	dprint("in air\n");
	self.frame=$jump12;
	if(self.flags&FL_ONGROUND)
	{
		thinktime self : 0;
		self.think=mezzo_land;
	}
	else 
	{
		if(self.velocity=='0 0 0')
			self.velocity='0 0 -60';
		self.think=mezzo_in_air;
		if(vlen(self.velocity)>300)
		{
			if(random()<0.5)
			{
				self.dflags=TRUE;//in air
				self.think=mezzo_roll_forward;
			}
		}
		thinktime self : 0.05;
	}
}

void mezzo_jump () [++ $jump1 .. $jump11]
{
//SOUND?
//	dprint("jumping\n");
	ai_face();
	if(self.flags&FL_ONGROUND)
	{
		thinktime self : 0;
		self.think=mezzo_land;
	}
	else if(self.frame==$jump11)
	{
		thinktime self : 0.05;
		self.think=mezzo_in_air;
	}
}

void mezzo_charge_stop () [++ $charge15 .. $charge25]
{
	if(cycle_wrapped)
	{
		self.touch=obj_push;
		thinktime self : 0;
		self.think=self.th_run;
	}
	if(!walkmove(self.angles_y,$charge25 - self.frame,FALSE))
	{
		if(!self.ltime)
			self.think=mezzo_pain_seq;
		else
			self.think=self.th_run;
		thinktime self : 0;
	}
}

void mezzo_charge_leap () [++ $charge9 .. $charge14]
{
//SOUND?
	if(cycle_wrapped)
	{
		thinktime self : 0;
		self.think=mezzo_charge_stop;
	}
	else if(self.frame==$charge9)
	{
		makevectors(self.angles);
		traceline(self.origin+'0 0 25',self.origin+'0 0 25'+v_forward*256,FALSE,self);
//Used to make him not charge if you stepped aside, now
//only checks if will fall
		if(!trace_ent.takedamage)
		{
			self.think=mezzo_skid;
			thinktime self : 0;
		}
		else
		{
			traceline(trace_endpos,trace_endpos-'0 0 300',TRUE,self);
			if(pointcontents(trace_endpos)!=CONTENT_EMPTY||trace_fraction==1)
			{
				self.think=mezzo_skid;
				thinktime self : 0;
			}
			else if(self.flags&FL_ONGROUND)
		 	{
				if(random()<0.5)
					sound(self,CHAN_VOICE,"mezzo/attack.wav",1,ATTN_NORM);
				self.velocity=v_forward*700+'0 0 133';
				self.flags(-)FL_ONGROUND;
			}
			else
			{
				self.think=mezzo_in_air;
				thinktime self : 0;
			}
		}
	}
}

void mezzo_charge () [++ $charge1 .. $charge8]
{
	if(cycle_wrapped)
	{
		thinktime self : 0;
		self.think=mezzo_charge_leap;
	}
	else if(self.frame==$charge1)
	{
		self.last_attack=time;
		self.ltime=0;
		self.touch=mezzo_slam;
		self.attack_finished=time+1.25;
	}
	walkmove(self.angles_y,15,FALSE);
}

void mezzo_block_return () [-- $block6 .. $block1]
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
	if(cycle_wrapped)
	{
	float r;
		if(self.shield)
			remove(self.shield);
		r=vlen(self.enemy.origin-self.origin);
		if(infront(self.enemy)&&r<100)
		{
			thinktime self : 0;
			self.think=self.th_melee;
		}
//		else if(random()<0.2&&r<177)
//		{
//			thinktime self : 0;
//			self.think=self.th_stand;
//		}
		else
		{
			thinktime self : 0;
			self.think=self.th_run;
		}
	}
}

void mezzo_block_wait ()
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
	self.think=mezzo_block_wait;
	if(self.t_width<time)
	{
		thinktime self : 0;
		self.think=mezzo_block_return;
	}
	else
	{
		self.frame=$block6;
		ai_face();
		self.think=mezzo_block_wait;
		thinktime self : 0.05;
	}
	if(range(self.enemy)==RANGE_MELEE)
		if(CheckAnyAttack())
			return;
	if(!self.flags&FL_ONGROUND)
	{
//		dprint("what the fuck?!  off ground while blocking?!\n");
		if(!self.velocity_x&&!self.velocity_y)
			self.think=mezzo_in_air;
		else
			self.think=mezzo_roll_forward;
		thinktime self : 0;
	}
	self.shield.oldthink=self.shield.think;
	self.shield.think=SUB_Remove;
	thinktime self.shield : 0.2;
//	dprint("checking defense from block\n");
	mezzo_check_defense();
	if(self.think==mezzo_block_wait)
	{
		self.shield.think=self.shield.oldthink;
		thinktime self.shield : 0;
	}
//	else
//		dprint("wigging!\n");
}

void mezzo_block () [++ $block1 .. $block6]
{
	walkmove(self.angles_y,-1,FALSE);
//	dprint("blocking\n");

//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
	if(cycle_wrapped)
	{
		if(self.th_pain==SUB_Null&&self.health>77)
			self.th_pain=mezzo_pain;
		self.t_width=time+1;
		thinktime self : 0;
		self.think=mezzo_block_wait;
	}
	else if(self.frame==$block1)
		spawn_reflect();
}

void mezzo_skid () [++ $block1 .. $block6]
{
float skidspeed, anim_stretch;
	anim_stretch = 3;

	skidspeed=$block6 - self.frame + anim_stretch - self.level;
	if(walkmove(self.angles_y,skidspeed*2,FALSE))
	{
		particle(self.origin, '0 0 20'*skidspeed, 344, skidspeed);
		if(random()<0.2)
			CreateWhiteSmoke(self.origin,'0 0 8',HX_FRAME_TIME * 2);
	}
	else
	{
		thinktime self : 0;
		self.think=mezzo_block_return;
		return;
	}

	if(cycle_wrapped)
	{
		self.attack_finished=time+3;
		thinktime self : 0;
		self.think=mezzo_block_return;
	}
	else if(self.frame==$block1)
	{
		spawn_reflect();
		sound(self,CHAN_AUTO,"mezzo/skid.wav",1,ATTN_NORM);
	}
	else if(self.level<anim_stretch)
	{
		self.frame-=1;
		self.level+=1;
	}
	else
		self.level=0;
}

void mezzo_roar () [++ $roar1 .. $roar30] 
{
	self.health+=1.1;

	if(self.frame==$roar30)
	{
		if(!self.aflag)
			self.th_pain=mezzo_pain;
		if(!self.takedamage)
			self.takedamage=DAMAGE_YES;
		self.last_attack=time+3;
		thinktime self : 0;
		self.think=self.oldthink;
	}
	else if(self.frame==$roar1)
	{
		self.monster_awake=TRUE;
		if(self.health<100)
		{
			self.th_pain=SUB_Null;
			self.takedamage=DAMAGE_NO;
		}
		sound(self,CHAN_VOICE,"mezzo/roar.wav",1,ATTN_NORM);
	}
	else if(self.frame==$roar19)
		thinktime self : 2;

	if(self.takedamage)
		mezzo_check_defense();
}

void mezzo_run_think ()
{
//	if(self.shield.velocity!='0 0 0'&&self.shield.model!="models/null.spr")
//		dprint("what the?\n");
	mezzo_check_defense();
/*
	if(!self.flags&FL_ONGROUND)
	{
		if(self.velocity=='0 0 0')
			self.velocity='0 0 -60';
		else if(random()<0.5)
			self.think=mezzo_roll_forward;
		else
			self.think=mezzo_in_air;
		thinktime self : 0;
	}
*/ 
//	if(!self.flags2&FL_ALIVE)
//		dprint("Undead Mezzoman!!!\n");
	if(!self.takedamage)
	{
//		dprint("Invincible Mezzoman!!!\n");
		self.takedamage=DAMAGE_YES;
	}

	if(!self.enemy.flags2&FL_ALIVE&&self.enemy!=world)
	{
		self.monster_awake=FALSE;
		if(visible(self.enemy)&&infront(self.enemy))
		{
			self.oldthink=self.th_stand;
			self.think=mezzo_roar;
		}
		else
			self.think=self.th_stand;
		if(self.oldenemy.flags2&FL_ALIVE)
		{
			self.enemy=self.oldenemy;
			self.oldenemy=world;
			self.think=self.th_run;
		}
		else
			self.enemy=world;
		thinktime self : 0;
	}
	else if(self.enemy!=world)
	{
		if(visible(self.enemy))
		{
		float dist;
			dist=vlen(self.enemy.origin-self.origin);
			if(dist<177&&dist>33)
			{
				if(random()<0.5&&lineofsight(self.enemy,self))
				{
					thinktime self : 0;
					self.think=mezzo_charge;
				}
				else if(dist>84&&self.last_attack<time&&infront(self.enemy))
				{
					thinktime self : 0;
					self.think=mezzo_skid;
				}
			}
			else if(!infront(self.enemy))
			{
				if(self.last_attack<time&&random()<0.5)
				{
					if(!self.aflag)
						self.yaw_speed=10;
					self.last_attack=time+7;
					thinktime self : 0;
					self.think=mezzo_skid;
				}
				else
					self.yaw_speed=20;
			}
			else if(!self.aflag)
				self.yaw_speed=10;
		}
		else
			self.yaw_speed=10;
		ai_run(self.speed);
	}
	else
	{
		self.think=self.th_stand;
		thinktime self : 0;
	}
}

void mezzo_run_loop () [++ $run1 .. $run22]
{
	mezzo_run_think();
}

void mezzo_run () [++ $run6 .. $run22]
{
	mezzo_check_defense();
	self.last_attack=time+0.1;//So won't start running then skid suddenly
	if(!self.monster_awake)
		if(range(self.enemy)>RANGE_NEAR&&random()>0.3)
		{
			self.oldthink=self.th_run;
			self.think=mezzo_roar;
			thinktime self : 0;
			return;
		}
		else
		{
			sound(self,CHAN_VOICE,"mezzo/attack.wav",1,ATTN_NORM);
			self.monster_awake=TRUE;
		}

	if(cycle_wrapped)
	{
		self.think=mezzo_run_loop;
		thinktime self : 0;
	}
	else 
		mezzo_run_think();
}

void mezzo_walk () [++ $walk1 .. $walk30]
{
	if(self.frame==$stand3 &&random()<0.1)
		mezzo_idle_sound();
	mezzo_check_defense();
	ai_walk(3);
	if(self.enemy)
		if(CheckAnyAttack())
			return;
}

void()mezzo_stand;
void mezzo_twirl() [++ $twirl1 .. $twirl10]
{
	mezzo_check_defense();
	if(cycle_wrapped)
	{
		self.think=mezzo_stand;
		thinktime self : 0;
	}
	else if(self.frame==$twirl1)
	{
		sound(self,CHAN_WEAPON,"weapons/vorpswng.wav",0.7,ATTN_NORM);
		if(random()<0.5)
			mezzo_idle_sound();
	}
	ai_stand();
}

void mezzo_stand2 () [-- $stand10 .. $stand1]
{
	mezzo_check_defense();

	if(self.level<3&&self.frame<$stand10)
	{
		self.level+=1;
		self.frame+=1;
	}
	else
		self.level=0;

	if(self.frame==$stand1)
	{
		if(random()<0.1||(self.monster_awake&&random()<0.5))
			self.think=mezzo_twirl;
		else
			self.think=mezzo_stand;
		thinktime self : 0;
		return;
	}
	else if(self.frame==$stand10 &&random()<0.1)
		mezzo_idle_sound();

/*	if(self.monster_awake)
	{
	float r;
		r=vlen(self.enemy.origin-self.origin);
		if(random()<0.1||r>177)
		{
			if(r>177)
				self.think=self.th_run;
			else if(enemy_infront&&enemy_vis&&r<133)
				self.think=mezzo_charge;
			else if(random()<0.5)
				self.think=self.th_run;
			else
				self.think=self.th_walk;
			thinktime self : 0;
		}
		else
		{
			if(random()<0.8&&r>100)
				self.attack_finished=time+0.1;
			ai_run(0);
		}
	}
	else
*/		ai_stand();
}

void mezzo_stand () [++ $stand1 .. $stand10]
{
	if(random()<0.5)
	{
		mezzo_check_defense();

		if((!self.enemy.flags2&FL_ALIVE&&self.enemy!=world)||self.enemy==world)
		{
			self.monster_awake=FALSE;
			if(self.oldenemy.flags2&FL_ALIVE)
			{
				self.enemy=self.oldenemy;
				self.oldenemy=world;
			}
			else
				self.enemy=world;
		}
	}

	if(self.level<3&&self.frame>$stand1)
	{
		self.level+=1;
		self.frame-=1;
	}
	else
		self.level=0;

	if(self.frame==$stand10)
	{
		self.think=mezzo_stand2;
		thinktime self : 0;
		return;
	}
	else if(self.frame==$stand1 &&random()<0.1)
		mezzo_idle_sound();

	if(random()<0.5)
		ai_stand();
}


/*QUAKED monster_werejaguar (1 0.3 0) (-16 -16 0) (16 16 56) AMBUSH STUCK JUMP PLAY_DEAD DORMANT
WereCat with jaguar skin
If they're targetted by a trigger and used, they'll atack the activator of the target
If they can't see the activator, they'll roll left or right (in the direction of the activator)
Their roll is 128 units long, so place the mezzoman 128 units away from where you want the roll to stop

My babies!!! - MG
*/
//IDEA: Have mezzoman do ai_face while in air or rolling so always faces you when lands/gets up?
void() monster_werejaguar =
{
   if (deathmatch)
   {
      remove(self);
      return;
   }

	if (!self.flags2&FL_SUMMONED)
	{
		precache_model2 ("models/mezzoman.mdl");
		precache_model2 ("models/mezzoref.spr");
		precache_model2 ("models/h_mez.mdl");
		precache_sound2 ("mezzo/skid.wav");
		precache_sound2 ("mezzo/roar.wav");
		precache_sound2 ("mezzo/reflect.wav");
		precache_sound2 ("mezzo/slam.wav");
		precache_sound2 ("mezzo/pain.wav");
		precache_sound2 ("mezzo/die.wav");
		precache_sound2 ("mezzo/growl.wav");
		precache_sound2 ("mezzo/snort.wav");
		precache_sound2 ("mezzo/attack.wav");
	}
	self.solid = SOLID_SLIDEBOX;
	self.takedamage=DAMAGE_YES;
	self.thingtype=THINGTYPE_FLESH;
	self.movetype = MOVETYPE_STEP;
	self.view_ofs = '0 0 53';
	self.speed=10;
	self.yaw_speed = 10;
	self.health = 250;
	self.experience_value = 150;
	self.monsterclass = CLASS_HENCHMAN;
	self.mass = 10;
	self.mintel = 15;//Animal sense of smell makes him a good tracker
	if(self.classname=="monster_werepanther")
	{
		self.monsterclass = CLASS_LEADER;
		self.experience_value = 300;
		self.health=400;
		self.skin=1;
	}

	self.classname="monster_mezzoman";

	self.th_stand=mezzo_stand;
	self.th_walk=mezzo_walk;
	self.th_run=mezzo_run;
	self.th_pain=mezzo_pain;
	self.th_melee=mezzo_melee;
	self.th_missile=mezzo_missile;
	self.th_jump=mezzo_jump;
	self.th_die=mezzo_die;
//	self.th_possum = mezzo_playdead;
//	self.th_possum_up = mezzo_possum_up;

	self.spawnflags (+) JUMP;

	setmodel (self, "models/mezzoman.mdl");
	self.headmodel="models/h_mez.mdl";

	setsize (self, '-16 -16 0', '16 16 56');

	self.frame=$stand1;

	walkmonster_start();
};

void monster_mezzoman (void)
{
	monster_werejaguar();
}

/*QUAKED monster_werepanther (1 0.3 0) (-16 -16 0) (16 16 56) AMBUSH STUCK JUMP PLAY_DEAD DORMANT
WereCat with panther skin
If they're targetted by a trigger and used, they'll atack the activator of the target
If they can't see the activator, they'll roll left or right (in the direction of the activator)
Their roll is 128 units long, so place the mezzoman 128 units away from where you want the roll to stop

My babies!!! - MG
*/
void monster_werepanther (void)
{
	monster_werejaguar();
}

