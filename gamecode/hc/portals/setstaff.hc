/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/setstaff.hc,v 1.5 2007-02-07 16:59:36 sezero Exp $
 */

/*
==============================================================================

Q:\art\models\weapons\setstaff\newstff\scarabst.hc

==============================================================================
*/

// For building the model
$cd Q:\art\models\weapons\setstaff\newstff
$origin 0 0 0
$base BASE skin
$skin skin
$skin SKIN2
$flags 0

//
$frame build1       build2       build3       build4       build5       
$frame build6       build7       build8       build9       build10      
$frame build11      build12      build13      build14      build15      

//
$frame chain1       chain2       chain3       chain4       chain5       
$frame chain6       chain7       chain8       chain9       

//
$frame rootpose     

//
$frame scarab1      scarab2      scarab3      scarab4      scarab5      
$frame scarab6      scarab7      

//
$frame select1      select2      select3      select4      select5      
$frame select6      select7      select8      select9      select10     
$frame select11     select12     



void setstaff_decide_attack (void);
void setstaff_idle (void);

void DrawLinks(void)
{
//These seem to lock up game if there are too many of them
vector org, dir;
	org = self.origin;
	dir=normalize(self.view_ofs-org);
	org+=dir*15;
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, TE_STREAM_CHAIN);
	WriteEntity (MSG_BROADCAST, self);
	WriteByte (MSG_BROADCAST, 1+STREAM_ATTACHED);
	WriteByte (MSG_BROADCAST, 1);
	WriteCoord (MSG_BROADCAST, org_x);
	WriteCoord (MSG_BROADCAST, org_y);
	WriteCoord (MSG_BROADCAST, org_z);
	WriteCoord (MSG_BROADCAST, self.view_ofs_x);
	WriteCoord (MSG_BROADCAST, self.view_ofs_y);
	WriteCoord (MSG_BROADCAST, self.view_ofs_z);
}

void() DeactivateGhook =
{
	self.aflag=FALSE;
	CreateRedFlash(self.origin);
	sound (self, CHAN_BODY, "items/itmspawn.wav", 1, ATTN_NORM);
	remove(self);	
};

void PullBack (void)
{
	if(self.enemy!=world)
	{
		self.enemy.flags2(-)FL_CHAINED;
		if(self.enemy.flags2&FL_ALIVE)
		{
//			dprintf("Restoring old movetype: %s\n",self.enemy.oldmovetype);
			self.enemy.movetype=self.enemy.oldmovetype;
		}
		else
		{
//			dprint("Setting movetype to bounce\n");
			self.enemy.movetype=MOVETYPE_BOUNCE;
		}
		self.enemy=world;
	}
//	else
//		dprint("No enemy\n");

	self.movetype=MOVETYPE_NOCLIP;
	self.solid=SOLID_NOT;
	self.velocity=normalize(self.view_ofs-self.origin)*350;
	self.flags(-)FL_ONGROUND;
	DrawLinks();
	if(vlen(self.origin-self.view_ofs)<48||self.lifetime<time)
		self.think=DeactivateGhook;
	else 
		self.think=PullBack;
	thinktime self : 0.05;
}

void() Yank =
{	
float dist;
vector dir;
		if(!self.enemy.health||!self.enemy.flags2&FL_ALIVE||!self.enemy.flags2&FL_CHAINED||self.attack_finished<time)
		{
			self.lifetime=time+2;
			self.think=PullBack;
	        thinktime self : 0;
			return;
		}

        setorigin (self, self.enemy.origin + self.enemy.mins +(self.enemy.size * 0.5));
        self.velocity = '0 0 0';

        if(self.enemy.v_angle!=self.o_angle)
	        self.enemy.v_angle=self.o_angle;

        if(random()<0.2&&random()<0.2)
        {
	        SpawnPuff (self.origin, self.velocity, 10,self.enemy);
	        if(random()<0.1)
				sound(self.enemy,CHAN_BODY,"assassin/chntear.wav",1,ATTN_NORM);
        }

		if((self.enemy.health<=self.health/100||self.enemy.health<=3)&&self.frags)
	    {
			T_Damage (self.enemy, self, self.owner, 5000);
			self.lifetime=time+2;
			self.think=PullBack;
			thinktime self : 0;
			return;
		}
        else
			T_Damage (self.enemy, self, self.owner, self.health/200);

		DrawLinks();

		if(self.enemy.classname!="player")
		{
	        if(self.enemy.nextthink<time+0.15)
				thinktime self.enemy : 0.15;
			if(self.enemy.attack_finished<time+0.15)
                self.enemy.attack_finished=time + 0.15;
	        if(self.enemy.pausetime<time+0.15)
		        self.enemy.pausetime=time + 0.15;
		}

		if(self.wait<=time)
		{
			dir=normalize(self.view_ofs - self.origin);
			self.angles=vectoangles('0 0 0'-dir);
		    dist = vlen (self.origin-self.view_ofs);
		    if (dist <= 100)
			    dir = dir*dist;
			if (dist > 100 )
		        dir = dir*300;
			if(self.enemy.flags&FL_ONGROUND)
	        {
			    self.enemy.flags(-)FL_ONGROUND;
				dir+='0 0 200';
			}
			if(self.enemy.flags&FL_CLIENT)
				self.enemy.adjust_velocity=(self.enemy.velocity+dir)*0.5;
			else
				self.enemy.velocity = (self.enemy.velocity+dir)*0.5;
		}
		else 
			self.enemy.velocity='0 0 0';

		if(!self.enemy.health||!self.enemy.flags2&FL_ALIVE||!self.enemy.flags2&FL_CHAINED||self.attack_finished<time)
		{
			self.lifetime=time+2;
			self.think=PullBack;
	        thinktime self : 0;
		}
		else
		{
			self.think=Yank;
			thinktime self : 0.05;
		}
//		if(self.attack_finished<time)
//			remove(self);
};

void(entity bound) Grab=
{
        sound(self, CHAN_AUTO, "assassin/chn2flsh.wav", 1, ATTN_NORM);
		self.wait=time+0.3;
        SpawnPuff (self.origin, self.velocity, 10,bound);
        self.velocity = '0 0 0';
        self.avelocity = '0 0 0';
		self.movedir=normalize(self.origin-self.view_ofs);
		setorigin(self,(bound.absmin+bound.absmax)*0.5+self.movedir*-10);
		self.movedir=normalize(self.origin-self.view_ofs);

        self.enemy=bound;
		if(!bound.flags2&FL_CHAINED)
		{
			if(other.flags&FL_CLIENT)
			{
				if(bound.rings&RING_FLIGHT)
					bound.oldmovetype=MOVETYPE_FLY;
				else
					bound.oldmovetype=MOVETYPE_WALK;
			}
			else
				bound.oldmovetype=bound.movetype;
			bound.movetype=MOVETYPE_FLY;
		}

		bound.flags2(+)FL_CHAINED;
		self.o_angle=bound.v_angle;
		self.health=bound.health;

		self.attack_finished=time+10;
        self.think=Yank;
        thinktime self : 0;

		T_Damage (bound, self, self.owner, 3);
};

void() HookHit =
{
		if(other==self.owner||other.classname==self.classname)
			return;

        self.touch = SUB_Null;

		if(other.takedamage&&other.flags2&FL_ALIVE&&other.health<1000)
            Grab(other);
        else
			DarkExplosion();
};

void HookHome (void)
{
vector destiny;
	if(self.enemy!=world)
	{
		destiny=self.enemy.origin;
		destiny_z=(self.enemy.absmax_z+self.enemy.absmin_z)*0.5;
		self.velocity=normalize(destiny - self.origin)*350;
		self.angles=vectoangles(self.velocity);
	}
	if(self.pain_finished<time)
	{
		sound(self,CHAN_BODY,"assassin/chain.wav",1,ATTN_NORM);
		self.pain_finished=time+0.5;
	}
	DrawLinks();
	if((self.lifetime<time&&!self.frags) || (!self.enemy.health&&self.enemy!=world) )
	{
		self.lifetime=time+2;
		self.think=PullBack;
        thinktime self : 0;
	}
	else
	{
		self.think=HookHome;
		thinktime self : 0.05;
	}
}

void(vector startpos, vector endpos, entity loser, entity winner,float gibhook) Hook =
{
entity ghook;

	ghook=spawn();
	sound(self, CHAN_AUTO, "misc/whoosh.wav", 1, ATTN_NORM);
    ghook.movetype=MOVETYPE_FLYMISSILE;
    ghook.solid=SOLID_PHASE;
    ghook.touch=HookHit;
    ghook.classname="hook";
    ghook.speed=8;
    ghook.owner=winner;
	ghook.enemy=loser;
	ghook.aflag=TRUE;
	ghook.view_ofs = startpos;
	ghook.frags=TRUE;
	ghook.lifetime=time+3;
	ghook.dmg=10;
	ghook.scale=2;

    ghook.movedir = normalize(endpos - startpos);
	ghook.velocity=ghook.movedir*500;
    ghook.angles = vectoangles(ghook.velocity);
		
	setmodel(ghook,"models/scrbpbdy.mdl");
    setsize(ghook,'0 0 0', '0 0 0');
    setorigin (ghook, startpos + ghook.movedir*6);

	ghook.think=HookHome;
	thinktime ghook : 0;
};


void DoHook(vector org, float mode)
{
	vector dir;
	
	dir = org;
	if (mode & 1)
		dir += v_right*random(300);
	else
		dir -= v_right*random(300);

	if (mode & 2)
		dir_z += random(300,1000);
	else
		dir_z += random(100);

	traceline(self.enemy.origin,dir,TRUE,self);
	Hook(trace_endpos,org,self.enemy,self.owner,TRUE);
}

void ChainsOfLove (void)
{
	vector org;

	self.enemy.velocity='0 0 0';
	self.enemy.oldmovetype=self.enemy.movetype;
	self.enemy.movetype=MOVETYPE_NONE;

	makevectors(self.enemy.angles);

	org=self.enemy.origin;
	org_z += 0.5*self.enemy.maxs_z;

	DoHook(org,0);
	DoHook(org,1);
	DoHook(org,2);
	DoHook(org,3);

	remove(self.movechain);
	remove(self);
}

void scarab_die ()
{
	if(self.lockentity.takedamage)
	{
		self.dmg=75;
		T_Damage(self.lockentity,self,self.owner,self.dmg);
	}
	if(self.movechain!=world)
	{
		remove(self.movechain);
		self.movechain = world;
	}
	MultiExplode();
}

void LatchOn (void)
{
	if(other.takedamage&&other.movetype&&other.health&&other.solid!=SOLID_BSP&&other.flags2&FL_ALIVE&&!other.artifact_active&ART_INVINCIBILITY)
	{
		if(other.health>150||(other.flags&FL_MONSTER&&other.monsterclass>=CLASS_BOSS))//other.classname!="player")
		{
			self.lockentity=other;
			scarab_die();
		}
		else
		{
			self.touch=SUB_Null;
			self.velocity='0 0 0';
			self.enemy=other;
			ChainsOfLove();
		}
	}
	else
		scarab_die();
}

void scarab_think ()
{
	self.frame+=1;
	if(self.frame>15)
		self.frame=8;
	self.movechain.frame=self.frame;
	if(self.pain_finished<=time)
	{
		HomeThink();
		self.angles=vectoangles(self.velocity);
		self.pain_finished=time+0.1;
		sound(self,CHAN_BODY,"assassin/scrbfly.wav",1,ATTN_NORM);
	}
//	particle4(self.origin,7,random(250,254),PARTICLETYPE_EXPLODE2,random(3,7));
	if(self.lifetime<time)
		self.think=scarab_die;
	thinktime self : 0.05;
}

void TheOldBallAndChain (void)
{
	entity wings;
//FIXME: Sound
	sound(self,CHAN_WEAPON,"assassin/scarab.wav",1,ATTN_NORM);
	self.attack_finished=time + 0.5;
	makevectors(self.v_angle);
	self.punchangle_x=-6;
	self.effects(+)EF_MUZZLEFLASH;

	newmis=spawn();
	newmis.owner=self;
	newmis.classname="chainball";
	
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_BBOX;
	newmis.drawflags=MLS_ABSLIGHT;
	newmis.th_die=scarab_die;
	newmis.frags=TRUE;
	newmis.dmg=150;
	newmis.abslight=0.5;
	newmis.scale=2.5;

	newmis.touch=LatchOn;

	newmis.o_angle=newmis.velocity=normalize(v_forward)*800;

	newmis.speed=800;	//Speed
	newmis.veer=50;
	newmis.hoverz=TRUE;
	newmis.turn_time=2;
//	newmis.turn_time=1;
	newmis.lifetime=time+5;
	newmis.pain_finished=time+0.2;

	newmis.think=scarab_think;
	thinktime newmis : 0;

	setmodel(newmis,"models/scrbpbdy.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs+v_forward*12);

	wings=spawn();
	setmodel(wings,"models/scrbpwng.mdl");
	setsize(wings,'0 0 0','0 0 0');
	setorigin(wings,newmis.origin+'0 0 5'-v_forward*3);
	newmis.movechain=wings;
	wings.scale=2.5;
	wings.drawflags(+)MLS_ABSLIGHT|DRF_TRANSLUCENT;
	wings.abslight=0.5;
	wings.flags(+)FL_MOVECHAIN_ANGLE;
}

void pincer_touch ()
{
	if(other==world||other.solid==SOLID_BSP||other.mass>300)
		DarkExplosion();
	else if(other!=self.enemy)
	{
		if(other.takedamage)
		{
			self.enemy=other;
			makevectors(self.velocity);
			T_Damage(other,self,self.owner,self.dmg);
			if(self.dmg<10)
			{
				T_Damage(other,self,self.owner,10);
				DarkExplosion();
			}
			else
			{
		        SpawnPuff (self.origin, self.velocity, 10,other);
		        SpawnPuff (self.origin+v_forward*36, self.velocity, 10,other);
				if(other.thingtype==THINGTYPE_FLESH)
				{
					sound(self,CHAN_VOICE,"assassin/core.wav",1,ATTN_NORM);
					MeatChunks(self.origin+v_forward*36, self.velocity*0.2+v_right*random(-30,150)+v_up*random(-30,150),5,other);
				}
				if(other.classname=="player")
					T_Damage(other,self,self.owner,(self.dmg+self.frags*10)/3);
				else
					T_Damage(other,self,self.owner,self.dmg+self.frags*10);
				self.frags+=1;
				self.dmg-=10;
			}
		}
	}
}

void pincer_think ()
{
	if(self.frame<7)
		self.frame+=1;
	if(self.pain_finished<=time)
	{
		self.pain_finished=time+1;
		sound(self,CHAN_BODY,"assassin/spin.wav",1,ATTN_NORM);
	}

	if(self.lifetime<time||self.flags&FL_ONGROUND)
		DarkExplosion();
	else
	{
		if(self.velocity!=self.movedir*self.speed)
			self.velocity=self.movedir*self.speed;
		self.think=pincer_think;
		thinktime self : 0.1;
	}
}

void Drilla (float power_value)
{
	sound(self,CHAN_WEAPON,"assassin/pincer.wav",1,ATTN_NORM);
	makevectors(self.v_angle);
	self.punchangle_x=power_value*-1;
	self.effects(+)EF_MUZZLEFLASH;
	newmis = spawn();
    newmis.owner = newmis.enemy= self;
	newmis.classname="pincer";
	newmis.movetype=MOVETYPE_FLYMISSILE;
	newmis.solid=SOLID_PHASE;
	newmis.thingtype=1;
	newmis.touch=pincer_touch;
	newmis.dmg=power_value*17;
	if(newmis.dmg<33)
		newmis.dmg=33;
	newmis.th_die=DarkExplosion;

	newmis.drawflags=MLS_ABSLIGHT;
	newmis.abslight=0.5;
	newmis.scale=2;

	newmis.speed=750+30*power_value;
	newmis.movedir=v_forward;
	newmis.velocity=newmis.movedir*newmis.speed;
	newmis.angles=vectoangles(newmis.velocity);

	setmodel(newmis,"models/scrbstp1.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs+v_forward*8);

    self.attack_finished = time + power_value/10;
	newmis.lifetime=time+7;
	newmis.think=pincer_think;
	thinktime newmis : 0;
}

/*
=============================================
WEAPON MODEL CODE
=============================================
*/
void setstaff_powerfire (void)
{
	// Pa3PyX: limited rate by real time (observed at 20 fps)
	if (time - self.ltime >= HX_FRAME_TIME) {
		self.ltime = time;

		self.wfs = advanceweaponframe($scarab1,$scarab7);
		self.th_weapon = setstaff_powerfire;
		// Pa3PyX: we now have animation loop, so don't think
		thinktime self: 0;

		if (self.weaponframe== $scarab2)
		{
			TheOldBallAndChain();
			self.greenmana -= 30;
			self.bluemana -= 30;
		}
	}

	if (self.wfs == WF_CYCLE_WRAPPED)
		setstaff_idle();
}

void setstaff_settle ()
{
	self.wfs = advanceweaponframe($chain1,$chain9);
	self.th_weapon = setstaff_settle;

	if (self.wfs == WF_LAST_FRAME)
		setstaff_idle();
}

void setstaff_readyfire (void)
{
	// Pa3PyX: Added casting delay (0.5s). You can no
	// longer use set staff in place of a chaingun.
	if (self.attack_finished > time) {
		self.th_weapon = setstaff_readyfire;
		return;
	}

	if(self.weaponframe>$build15)
		self.weaponframe=$build1;

	if(self.weaponframe==$build1)
		sound(self,CHAN_WEAPON,"assassin/build.wav",1,ATTN_NORM);

	if (self.lifetime <= 0)
		self.lifetime = time;	// Pa3PyX

	if (self.weaponframe >= $build1 && self.weaponframe < $build15)
	{
		self.weaponframe_cnt +=1;
// Pa3PyX  //	if (self.weaponframe_cnt > 3)
		if (time - self.lifetime > HX_FRAME_TIME * 3)
		{
			self.wfs = advanceweaponframe($build1,$build15);
			self.weaponframe_cnt =0;
			self.lifetime = time;	// Pa3PyX
		}
		else if(self.weaponframe_cnt==1)
		{
		 	if(self.greenmana>=1)
				self.greenmana-=1;
			if(self.bluemana>=1)
				self.bluemana-=1;
		}
		if(self.weaponframe==$build15)
			self.weaponframe_cnt=0;
	}
	else if(self.weaponframe_cnt<time)
	{
		sound(self,CHAN_WEAPON,"misc/pulse.wav",1,ATTN_NORM);
		self.weaponframe_cnt=time+1.7;
	 	if(self.greenmana>=10)
			self.greenmana-=10;
		else
			self.button0=FALSE;
		if(self.bluemana>=10)
			self.bluemana-=10;
		else
			self.button0=FALSE;
	}
			
	self.th_weapon = setstaff_readyfire;

	if(!self.button0||self.greenmana<=0||self.bluemana<=0)
	{
		self.weaponframe_cnt=0;
		self.lifetime = 0;	// Pa3PyX

		Drilla(14 - ($build15 - self.weaponframe));

		self.attack_finished = time + 0.5;	// Pa3PyX: casting delay

		setstaff_settle();
	}
}

void() ass_setstaff_fire =
{
	if (self.artifact_active & ART_TOMEOFPOWER)  // Pause for firing in power up mode
		self.th_weapon=setstaff_powerfire;
	else
		self.th_weapon=setstaff_readyfire;

	thinktime self : 0;
	self.nextthink=time;
};


void setstaff_idle (void)
{
	self.weaponframe=$rootpose;
	self.th_weapon=setstaff_idle;
}


void setstaff_select (void)
{
	self.wfs = advanceweaponframe($select12,$select1);
	self.weaponmodel = "models/scarabst.mdl";
	self.th_weapon=setstaff_select;
	self.last_attack=time;

	if (self.wfs == WF_LAST_FRAME)
	{
		self.attack_finished = time - 1;
		self.ltime = -1;
		setstaff_idle();
	}
}

void setstaff_deselect (void)
{
	self.wfs = advanceweaponframe($select1,$select12);
	self.th_weapon=setstaff_deselect;
	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}

void setstaff_decide_attack (void)
{
	self.attack_finished = time + 0.5;
}

