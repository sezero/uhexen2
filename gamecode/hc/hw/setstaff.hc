/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/setstaff.hc,v 1.2 2007-02-07 16:58:01 sezero Exp $
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

void() DeactivateGhook =//don't worry about the effect--it'll get rid of itself
{
	self.aflag=FALSE;

	remove(self);	
};

void PullBack (void)
{
	if(self.enemy!=world)
	{
		if(self.enemy.flags2&FL_ALIVE)
		{
			if (self.enemy.flags&FL_CLIENT)
			{
				if(!(self.enemy.rings&RING_FLIGHT))
					self.enemy.movetype=MOVETYPE_WALK;
			}
			else
			{
				if (!(self.enemy.flags & FL_FLY))
					self.enemy.movetype = MOVETYPE_STEP;
//				self.enemy.movetype = self.enemy.oldmovetype;
			}
		}
		else
		{
			self.enemy.movetype=MOVETYPE_BOUNCE;
		}
		self.enemy.velocity_z-=100;
		self.enemy.flags2(-)FL_CHAINED;
		self.enemy=world;
	}

	self.movetype=MOVETYPE_NOCLIP;
	self.solid=SOLID_NOT;
	self.velocity=normalize(self.view_ofs-self.origin)*350;
	self.flags(-)FL_ONGROUND;
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
			updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
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
		
			//fixme: attach to effect
	        SpawnPuff (self.origin, self.velocity, 10,self.enemy);
	        if(random()<0.1)
				sound(self.enemy,CHAN_BODY,"assassin/chntear.wav",1,ATTN_NORM);

        }

		if(self.enemy.health<=2&&self.frags)
	    {
			updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
		    T_Damage (self.enemy, self, self.owner, 5000);
			self.lifetime=time+2;
			self.think=PullBack;
			thinktime self : 0;
			return;
		}
        else
			T_Damage (self.enemy, self, self.owner, self.health/200);

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
			updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
			self.lifetime=time+2;
			self.think=PullBack;
	        thinktime self : 0;
		}
		else
		{
			self.think=Yank;
//			thinktime self : 0.05;
			self.nextthink=time;
		}
};

void(entity bound) Grab=
{
		float ttype;

		if (bound != self.enemy)
		{
			//highest 4 bits of short indicate thingtype--can't use THINGTYPE_ consts because there are too many
			ttype = GetImpactType(bound);
			updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,bound, ttype*4096);
		}

		self.wait=time+0.3;

        self.velocity = '0 0 0';
        self.avelocity = '0 0 0';
		self.movedir=normalize(self.origin-self.view_ofs);
		setorigin(self,(bound.absmin+bound.absmax)*0.5+self.movedir*-10);
		self.movedir=normalize(self.origin-self.view_ofs);

        self.enemy=bound;
		if(!(bound.flags2&FL_CHAINED))
		{
			if(other.flags&FL_CLIENT)
			{
				if(bound.rings&RING_FLIGHT)
					bound.oldmovetype=MOVETYPE_FLY;
				else
					bound.oldmovetype=MOVETYPE_WALK;
			}
			else
			{
				if (other.flags & FL_FLY)
				{
					bound.oldmovetype=MOVETYPE_FLY;
				}
				else
				{
					bound.oldmovetype=MOVETYPE_STEP;
				}
//				bound.oldmovetype=bound.movetype;
			}
			bound.movetype=MOVETYPE_FLY;
		}

		bound.flags2(+)FL_CHAINED;
		self.o_angle=bound.v_angle;
		self.health=bound.health;

//		self.attack_finished=time+10;
		self.attack_finished=time+4;
        self.think=Yank;
//        thinktime self : 0;
		self.nextthink=time;

		T_Damage (bound, self, self.owner, 3);
};

void HookHome (void)
{
	vector org;

	org = self.enemy.origin;
	org_z += 0.5*self.enemy.maxs_z;
    setorigin (self, org);

	if(self.pain_finished<time)
	{
		self.pain_finished=time+0.5;
	}
	if((self.lifetime<time&&!self.frags) || (!self.enemy.health&&self.enemy!=world) )
	{
		updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
		self.lifetime=time+2;
		self.think=PullBack;
        thinktime self : 0;
	}

//from HookHit:
	if(self.enemy==self.owner)
	{
	   self.touch = SUB_Null;
		updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
//		endeffect(MSG_ALL, self.xbo_effect_id);
		DarkExplosion();
		return;
	}

    self.touch = SUB_Null;

	if(self.enemy.takedamage&&self.enemy.flags2&FL_ALIVE&&self.enemy.health<1000)
        Grab(self.enemy);
    else
	{
		updateeffect(self.xbo_effect_id,CE_SCARABCHAIN,0,0);
//		endeffect(MSG_ALL, self.xbo_effect_id);
		DarkExplosion();
	}
}

void(vector startpos, vector endpos, entity loser, entity winner,float gibhook, float mode) Hook =
{
	entity ghook;
	float ttype;

	ghook=spawn();
    ghook.movetype=MOVETYPE_NOCLIP;
    ghook.solid=SOLID_NOT;
//    ghook.touch=HookHit;
    ghook.classname="hook";
    ghook.speed=8;

	ghook.owner=winner;
//    ghook.owner=world;//testing it on myself--ss

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
		
//	setmodel(ghook,"models/scrbpbdy.mdl");
    setsize(ghook,'0 0 0', '0 0 0');
    setorigin (ghook, startpos + ghook.movedir*6);

	ttype = GetImpactType(loser);
	ghook.xbo_effect_id = starteffect(CE_SCARABCHAIN, ghook.origin, ghook.enemy, ttype*4096, mode);

	ghook.think=HookHome;
    setorigin (ghook, endpos);
	ghook.velocity = '0 0 0';
	thinktime ghook : vlen(endpos-startpos)*0.002;
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
	Hook(trace_endpos,org,self.enemy,self.owner,TRUE, mode);
}

void ChainsOfLove (void)
{
	vector org;

	self.enemy.velocity='0 0 0';

	//not useful, and dangerous too
//	self.enemy.oldmovetype=self.enemy.movetype;
//	self.enemy.movetype=MOVETYPE_NONE;

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

void ChainThinker()
{
	entity loser;
	float bestdist,dist;
	vector org,tvec;


	if (self.lifetime < time)
	{
		remove(self);
		return;
	}

	bestdist = 250;
	loser=findradius(self.origin,bestdist);
	while (loser)
	{
		if(loser.health&&loser.takedamage&&(loser.flags2&FL_ALIVE)&&visible(loser)&&loser!=self&&loser!=world&&loser!=self.owner&&!(loser.effects&EF_NODRAW)&&!(loser.artifact_active&ART_INVINCIBILITY))
		{
			if(teamplay&&loser.classname=="player"&&((loser.team==self.owner.team&&self.owner.classname=="player")||(loser.team==self.controller.team&&self.owner.classname=="player")))
				dprint("");//targeting teammate\n");
			else if(coop&&loser.classname=="player"&&(self.owner.classname=="player"||self.controller.classname=="player"))
				dprint("");//target coop player\n");
			else
			{
			//make it wait for closest (by vlen) or just go for first found?
				dist=vlen(self.origin-loser.origin);
				if(dist<bestdist)
				{
					bestdist=dist;
					self.enemy=loser;
				}
			}
		}
		loser=loser.chain;
	}

	if (self.enemy)
	{
		self.enemy.velocity='0 0 0';

		//not useful, and dangerous too
//		self.enemy.oldmovetype=self.enemy.movetype;
//		self.enemy.movetype=MOVETYPE_NONE;

		org=self.enemy.origin;
		org_z += 0.5*self.enemy.maxs_z;

		tvec=self.origin;

		Hook(tvec,org,self.enemy,self.owner,TRUE, 0);

		remove(self);
		return;
	}

	thinktime self : 0.05;
}

void scarab_die ()
{
	if(self.movechain!=world)
	{
		remove(self.movechain);
		self.movechain = world;
	}

	if(self.lockentity.takedamage)
	{
		self.dmg=75;
		T_Damage(self.lockentity,self,self.owner,self.dmg);
//these commented statements left over from fixing the scarabs that wouldn't go away
//dprint(self.lockentity.classname);
//dprint(" is lockent\n");
//if (self.think==scarab_die){dprint(ftos(self.nextthink-time));dprint(" secs til next think!\n");}
///*
		self.think = SUB_Remove;
		self.touch=SUB_Null;
		self.velocity='0 0 0';
		self.th_die = SUB_Null;
		thinktime self : 0.05;
//*/
		return;
	}

	self.touch=SUB_Null;
	self.velocity='0 0 0';
	self.think=ChainThinker;
	self.th_die = SUB_Null;
	self.lifetime = time + 5;
	self.enemy = world;

	thinktime self : 0.05;
}

void LatchOn (void)
{

//testing: scarabs are screwy, just won't go away
//self.lockentity=self.owner;
//scarab_die();
///*
	if(other.takedamage&&other.movetype&&other.health&&other.solid!=SOLID_BSP&&other.flags2&FL_ALIVE&&!(other.artifact_active&ART_INVINCIBILITY))
	{
		if(other.health>150||(other.flags&FL_MONSTER&&other.monsterclass>=CLASS_BOSS))
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
//*/
}

void scarab_think ()
{
	self.frame+=1;
	if(self.frame>15)
		self.frame=8;

	if (self.movechain)
	{
		self.movechain.frame=self.frame;
	}

	if(self.pain_finished<=time)
	{
		HomeThink();
		self.angles=vectoangles(self.velocity);
		self.pain_finished=time+0.1;
	}

	if(self.lifetime<time)
		self.think=scarab_die;
	thinktime self : 0.05;
}

void TheOldBallAndChain (void)
{
	entity wings;
//FIXME: Sound
	weapon_sound(self, "assassin/scarab.wav");
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

	newmis.o_angle=newmis.velocity=normalize(v_forward)*600;//was 800

	newmis.speed=600;	//Speed- was 900
	newmis.veer=50;
	newmis.hoverz=TRUE;
	newmis.turn_time=2;
//	newmis.turn_time=1;
	newmis.lifetime=time+5;
	newmis.pain_finished=time+0.2;

	newmis.effects(+)EF_BEETLE_EFFECTS;
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

void DrillaTurn(entity bolt)
{
	vector dir;

	bolt.xbo_startpos = bolt.origin;
//	self.movedir=self.velocity*(1/self.speed);
	dir = vectoangles(bolt.velocity);
	updateeffect(bolt.xbo_effect_id, CE_HWDRILLA, 1, dir_x, dir_y, bolt.origin);
}

void pincer_touch ()
{
	float ttype;

	if(other==world||other.solid==SOLID_BSP||other.mass>300)
		DarkExplosion();
	else if(other!=self.nextbolt)
	{
		if(other.takedamage)
		{
			self.nextbolt=other;
			makevectors(self.velocity);
			T_Damage(other,self,self.owner,self.dmg);
			if(self.dmg<10)
			{
				T_Damage(other,self,self.owner,10);
				DarkExplosion();
			}
			else
			{
				//fixme: add to effect
//				if(other.thingtype==THINGTYPE_FLESH)
//				{
//					MeatChunks(self.origin+v_forward*36, self.velocity*0.2+v_right*random(-30,150)+v_up*random(-30,150),5,other);
//				}

				ttype = GetImpactType(other);

				updateeffect(self.xbo_effect_id, CE_HWDRILLA, 0, self.origin, ttype);

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
	}

	if(self.lifetime<time||self.flags&FL_ONGROUND)
		DarkExplosion();
	else
	{
//		if(self.velocity!=self.movedir*self.speed)
//			self.velocity=self.movedir*self.speed;
		self.think=pincer_think;
		thinktime self : 0.1;
	}
}

void Drilla (float power_value)
{
	if((self.weaponframe>=$build2)&&(self.weaponframe<=$build15))
	{
		stopSound(self,CHAN_UPDATE);//weapon_sound(self, "misc/null.wav");
		self.effects(-)EF_UPDATESOUND;
		self.t_width=SOUND_STOPPED;
	}
	makevectors(self.v_angle);
	self.punchangle_x=power_value*-1;
	self.effects(+)EF_MUZZLEFLASH;
	newmis = spawn();
    newmis.owner = self;
	newmis.enemy = world;
	newmis.nextbolt = world;
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

//	setmodel(newmis,"models/scrbstp1.mdl");
	setsize(newmis,'0 0 0','0 0 0');
	setorigin(newmis,self.origin+self.proj_ofs+v_forward*8);

	newmis.xbo_effect_id = starteffect(CE_HWDRILLA, newmis.origin, self.v_angle, newmis.speed);

    self.attack_finished = time + power_value/10 + 0.5;
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
	self.wfs = advanceweaponframe($scarab1,$scarab7);
	self.th_weapon = setstaff_powerfire;
	if (self.weaponframe== $scarab2)
	{
		TheOldBallAndChain();
		self.greenmana -= 30;
		self.bluemana -= 30;
	}

	if (self.wfs == WF_CYCLE_WRAPPED)
		setstaff_idle();
}

void setstaff_settle2 ()
{
	self.effects(-)EF_MUZZLEFLASH;
	self.wfs = advanceweaponframe($chain1,$chain9);
	self.th_weapon = setstaff_settle2;

	if (self.wfs == WF_LAST_FRAME)
		setstaff_idle();
}

void setstaff_settle ()
{
	self.wfs = advanceweaponframe($chain1,$chain9);
	self.th_weapon = setstaff_settle2;//after a frame, turn off light

	if (self.wfs == WF_LAST_FRAME)
		setstaff_idle();
}

void setstaff_readyfire (void)
{
	if(self.weaponframe>$build15)
		self.weaponframe=$build1;

	if(self.weaponframe==$build2)
	{
		//weapon_sound(self, "assassin/build.wav");
		sound(self, CHAN_UPDATE+PHS_OVERRIDE_R,"assassin/build.wav",1,ATTN_LOOP);
		self.effects(+)EF_UPDATESOUND;
	}


	if (self.weaponframe >= $build1 && self.weaponframe < $build15)
	{
//		updateSoundPos(self,CHAN_WEAPON);//FIXME: make this built into client sound code- maybe some sort of flag or field?
		self.weaponframe_cnt +=1;
		if (self.weaponframe_cnt > 3)
		{
			self.wfs = advanceweaponframe($build1,$build15);
			self.weaponframe_cnt =0;
		}
		else if(self.weaponframe_cnt==1)
		{
			if (self.weaponframe == $build1)
			{
				self.greenmana-=3;
		 		if(self.greenmana<0)
					self.greenmana=0;

				self.bluemana-=3;
				if(self.bluemana<0)
					self.bluemana=0;
			}
			else
			{
		 		if(self.greenmana>=1)
					self.greenmana-=1;
				if(self.bluemana>=1)
					self.bluemana-=1;
			}
		}
		if(self.weaponframe==$build15)
			self.weaponframe_cnt=time+0.8;
	}
	else if(self.weaponframe_cnt<time)
	{
		if(self.t_width!=SOUND_STARTED)
		{
			sound(self, CHAN_UPDATE+PHS_OVERRIDE_R,"misc/pulsel.wav",1,ATTN_LOOP);
			self.effects(+)EF_UPDATESOUND;
			self.t_width=SOUND_STARTED;
		}
		//weapon_sound(self, "misc/pulse.wav");
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
		self.t_width=FALSE;
		self.weaponframe_cnt=0;
		Drilla(14 - ($build15 - self.weaponframe));
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
	self.effects(-)EF_MUZZLEFLASH;
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
		setstaff_idle();
	}
}

void setstaff_deselect (void)
{
	self.effects(-)EF_MUZZLEFLASH;
	self.wfs = advanceweaponframe($select1,$select12);
	self.th_weapon=setstaff_deselect;
	if (self.wfs == WF_LAST_FRAME)
		W_SetCurrentAmmo();
}

void setstaff_decide_attack (void)
{
	self.attack_finished = time + 0.5;
}

