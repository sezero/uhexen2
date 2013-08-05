/*=========================================
FUNCTIONS THAT ALL PLAYERS WILL CALL
===========================================*/
void() bubble_bob;

void PlayerSpeed_Calc (void)
{
	switch (self.playerclass)
	{
	case CLASS_ASSASSIN:
	case CLASS_SUCCUBUS:
		self.hasted=1;
	break;
	case CLASS_PALADIN:
		self.hasted=.96;
	break;
	case CLASS_CRUSADER:
		self.hasted=.93;
	break;
	case CLASS_NECROMANCER:
		self.hasted=.9;
	break;
	}

	if (self.artifact_active & ART_HASTE)
		self.hasted *= 2.9;

	if (self.hull==HULL_CROUCH)   // Player crouched
		self.hasted *= .6;
}

vector VelocityForDamage (float dm)
{
	local vector v;

	v = randomv('-100 -100 200', '100 100 300');

	if (dm > -50)
		v = v * 0.7;
	else if (dm > -200)
		v = v * 2;
	else
		v = v * 10;

	return v;
}


void ReadySolid ()
{
	if(!self.headmodel)
		self.headmodel="models/flesh1.mdl";//Temp until player head models are in
	MakeSolidCorpse ();
}

void StandardPain(void)
{
	if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
	{
		if (random() > 0.5)
			sound (self, CHAN_VOICE, "player/asspain1.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "player/asspain2.wav", 1, ATTN_NORM);
	}
	else if (random() > 0.5)
		sound (self, CHAN_VOICE, "player/palpain1.wav", 1, ATTN_NORM);
	else
		sound (self, CHAN_VOICE, "player/palpain2.wav", 1, ATTN_NORM);
}

void PainSound (void)
{
	if (self.health <= 0)
		return;

	if (self.deathtype == "teledeath"||self.deathtype == "teledeath2"||self.deathtype == "teledeath3"||self.deathtype == "teledeath4")
	{
		sound (self, CHAN_VOICE, "player/telefrag.wav", 1, ATTN_NONE);
		return;
	}

	if (self.pain_finished > time)
		return;

	self.pain_finished = time + 0.5;

	// FIXME:  Are we doing seperate sounds for these different pains????
	if (self.model=="models/sheep.mdl")
		sheep_sound(1);
	else if (/*self.watertype == CONTENT_WATER &&*/ self.waterlevel == 3)
	{
		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
			sound (self, CHAN_VOICE, "player/assdrown.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "player/paldrown.wav", 1, ATTN_NORM);
	}
	else //if (self.watertype == CONTENT_SLIME)
	{
		StandardPain();
	}
/*	else if (self.watertype == CONTENT_LAVA)
	{
		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
		{
			if (random() > 0.5)
				sound (self, CHAN_VOICE, "player/asspain1.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "player/asspain2.wav", 1, ATTN_NORM);
		}
		else if (random() > 0.5)
			sound (self, CHAN_VOICE, "player/palpain1.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "player/palpain2.wav", 1, ATTN_NORM);
	}
	else
	{
		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
		{
			if (random() > 0.5)
				sound (self, CHAN_VOICE, "player/asspain1.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "player/asspain2.wav", 1, ATTN_NORM);
		}
		else if (random() > 0.5)
			sound (self, CHAN_VOICE, "player/palpain1.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "player/palpain2.wav", 1, ATTN_NORM);
	}*/
}

void player_pain (entity attacker,float total_damage)
{
//FIX this = need to check if firing, else make idle frames of all
//	weapons frame 0?
//if (self.weaponframe)
//		return;

	if(self.playerclass==CLASS_SUCCUBUS)
	{
	//	if(self.flags&FL_SPECIAL_ABILITY2)
		/* Pa3PyX: Made this work only when low on health (less than half),
		 * as described in the manual:  "This is purely a survival measure
		 * and occurs only when her health is low." Amounts were apparently
		 * botched, so new skill stats:
		 * clvl         |    6|    7|    8|    9|  10+
		 * -------------+-----+-----+-----+-----+-----
		 * %absorb:blue |   20|   40|   60|   80|  100
		 * %absorb:green|    0|   25|   50|   75|  100 */
		if (self.flags&FL_SPECIAL_ABILITY2 && (self.health < self.max_health / 2.0))
		{//Sound &/or effect?
			if(self.level>9)
			{
				self.greenmana+=total_damage;
				self.bluemana+=total_damage;
			}
			else
			{
			//	if(self.level>6)
			//		self.greenmana+=total_damage*(self.level - 2/10);
			//	self.bluemana+=total_damage*(self.level/10);
				/* Pa3PyX: This is wrong, green mana should not fill up
				 * from zero in 3 archer hits. The intention was probably
				 * (l - 2)/10 rather than l - 2/10.  We'll make both grow
				 * from 0 at clvl 5 and 7 resp to total dmg at clvl 10. */
				self.bluemana+=total_damage*(self.level - 5)/5;
				if (self.level>6)
					self.greenmana+=total_damage*(self.level - 6)/4;
			}
			if(self.bluemana>self.max_mana)
				self.bluemana=self.max_mana;
			if(self.greenmana>self.max_mana)
				self.greenmana=self.max_mana;
		}
	}

	if (self.last_attack + 0.5 > time || self.button0)
		return;

	PainSound();

//	self.weaponframe=0;//Why?

	if (self.hull==HULL_PLAYER)
		self.act_state=ACT_PAIN;
	else
		self.act_state=ACT_CROUCH_MOVE;//No pain animation for crouch- maybe jump?
								//Make it make you stand up?
}

void DeathBubblesSpawn ()
{
entity	bubble;
vector	offset;

	offset_x = random(18,-18);
	offset_y = random(18,-18);

	if (pointcontents(self.owner.origin+self.owner.view_ofs)!=CONTENT_WATER)
	{
		remove(self);
		return;
	}

	bubble = spawn_temp();
	setmodel (bubble, "models/s_bubble.spr");
	setorigin (bubble, self.owner.origin+self.owner.view_ofs+offset);
	bubble.movetype = MOVETYPE_NOCLIP;
	bubble.solid = SOLID_NOT;
	bubble.velocity = '0 0 17';
	thinktime bubble : 0.5;
	bubble.think = bubble_bob;
	bubble.classname = "bubble";
	bubble.frame = 0;
	bubble.cnt = 0;
	bubble.abslight=0.5;
	bubble.drawflags(+)DRF_TRANSLUCENT|MLS_ABSLIGHT;
	setsize (bubble, '-8 -8 -8', '8 8 8');
	thinktime self : 0.1;
	self.think = DeathBubblesSpawn;
	self.air_finished = self.air_finished + 1;
	if (self.air_finished >= self.bubble_count)
		remove(self);
}

void DeathBubbles (float num_bubbles)
{
entity	bubble_spawner, bubble_owner;

	if(self.classname=="contents damager")
		bubble_owner = self.enemy;
	else
		bubble_owner = self;
	bubble_spawner = spawn();
	setorigin (bubble_spawner, bubble_owner.origin+bubble_owner.view_ofs);
	bubble_spawner.movetype = MOVETYPE_NONE;
	bubble_spawner.solid = SOLID_NOT;
	bubble_spawner.owner = bubble_owner;
	thinktime bubble_spawner : 0.1;
	bubble_spawner.think = DeathBubblesSpawn;
	bubble_spawner.air_finished = 0;
	bubble_spawner.bubble_count = num_bubbles;
}


void DeathSound ()
{
// water death sounds
	if (self.waterlevel == 3)
	{
		DeathBubbles(20);
		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
			sound (self, CHAN_VOICE, "player/assdieh2.wav", 1, ATTN_NONE);
		else
			sound (self, CHAN_VOICE, "player/paldieh2.wav", 1, ATTN_NONE);
		return;
	}
	else
	{
		if(self.playerclass==CLASS_ASSASSIN||self.playerclass==CLASS_SUCCUBUS)
		{
			if (random() > 0.5)
				sound (self, CHAN_VOICE, "player/assdie1.wav", 1, ATTN_NORM);
			else
				sound (self, CHAN_VOICE, "player/assdie2.wav", 1, ATTN_NORM);
		}
		else if (random() > 0.5)
			sound (self, CHAN_VOICE, "player/paldie1.wav", 1, ATTN_NORM);
		else
			sound (self, CHAN_VOICE, "player/paldie2.wav", 1, ATTN_NORM);
	}
}

void PlayerDead ()
{
	self.nextthink = -1;
// allow respawn after a certain time
	self.deadflag = DEAD_DEAD;

	if(self.model!=self.headmodel)
	{
		self.angles_x=self.angles_z=0;
		pitch_roll_for_slope('0 0 0',self);
	}
}

void ThrowGib (string gibname, float dm)
{
entity new;

	new = spawn_temp();
	new.origin = (self.absmin+self.absmax)*0.5;
	setmodel (new, gibname);
	setsize (new, '0 0 0', '0 0 0');
	new.velocity = VelocityForDamage (dm);
	new.movetype = MOVETYPE_BOUNCE;
	new.solid = SOLID_NOT;
	new.avelocity_x = random(600);
	new.avelocity_y = random(600);
	new.avelocity_z = random(600);
	new.think = SUB_Remove;
	new.ltime = time;
	thinktime new : random(20,10);
	new.scale=random(.5,.9);
	new.frame = 0;
	new.flags = 0;
}

void ThrowHead (string gibname, float dm)
{
vector org;
	if(self.decap==2)
	{//Brains!
		if(self.movedir=='0 0 0')
		{
			self.movedir=normalize(self.origin+self.view_ofs-self.enemy.origin+self.enemy.proj_ofs);
			self.movedir_z=0;
		}
		traceline(self.origin + self.view_ofs, self.origin+self.view_ofs+self.movedir*100, FALSE, self);
		if (trace_fraction < 1&&!trace_ent.flags2&FL_ALIVE&&trace_ent.solid==SOLID_BSP)
		{
			self.wallspot=trace_endpos;
			ZeBrains(trace_endpos, trace_plane_normal, random(1.3,2), rint(random(1)),random(360));
		}
		else
			self.wallspot='0 0 0';
	}

	setmodel (self, gibname);
	self.frame = 0;
	self.takedamage = DAMAGE_NO;
	if(self.classname!="player")
		self.solid = SOLID_BBOX;
	self.movetype = MOVETYPE_BOUNCE;

	self.mass = 1;
	self.view_ofs = '0 0 8';
	self.proj_ofs='0 0 2';
	org=self.origin;
	org_z=self.absmax_z - 4;
//This may be bad...
	setsize (self, '-4 -4 -4', '4 4 4');
	self.hull=HULL_POINT;
	setorigin(self,org);
	self.flags(-)FL_ONGROUND;
	self.avelocity = randomv('0 -600 0', '0 600 0');

	if(self.decap==2)
		self.velocity = VelocityForDamage (dm)+'0 0 50';
	else
		self.velocity = VelocityForDamage (dm)+'0 0 200';

	if(self.decap==2||(self.decap==1&&vlen(self.velocity)>300))
	{
		if(self.wallspot=='0 0 0')
			self.wallspot=org;
		self.pausetime=time+5;//watch splat or body
	}

	self.think=PlayerDead;
	thinktime self : 1;
}


void PlayerUnCrouching ()
{
	tracearea (self.origin,self.origin+'0 0 28','-16 -16 0','16 16 28',FALSE,self);
	if (trace_fraction < 1)
	{
		centerprint(self,STR_NOROOM);
		self.crouch_stuck = 1;
		return;
	}

	setsize (self, '-16 -16 0', '16 16 56');
	self.hull=HULL_PLAYER;
	if (self.viewentity.classname=="chasecam")
		self.view_ofs = '0 0 0';

	PlayerSpeed_Calc();
	self.crouch_time = time;

	if (self.velocity_x || self.velocity_y)
		self.act_state=ACT_RUN;
	else
		self.act_state=ACT_STAND;
}

void PlayerCrouching ()
{
	if (self.health <= 0)
		return;

	setsize (self,'-16 -16 0','16 16 28');
	self.hull=HULL_CROUCH;
	if (self.viewentity.classname=="chasecam")
		self.view_ofs = '0 0 0';
	self.absorb_time=time + 0.3;

	PlayerSpeed_Calc();
	self.crouch_time = time;

	self.crouch_stuck = 0;

	self.act_state=ACT_CROUCH_MOVE;
}

/*
void PlayerCrouch ()
{
	if (self.hull==HULL_PLAYER)
		PlayerCrouching();
	else if (self.hull==HULL_CROUCH)
		PlayerUnCrouching();
}
*/


void GibPlayer ()
{
	ThrowHead (self.headmodel, self.health);
	ThrowGib ("models/flesh1.mdl", self.health);
	ThrowGib ("models/flesh2.mdl", self.health);
	ThrowGib ("models/flesh3.mdl", self.health);
	ThrowGib ("models/flesh1.mdl", self.health);
	ThrowGib ("models/flesh2.mdl", self.health);
	ThrowGib ("models/flesh3.mdl", self.health);

	self.deadflag = DEAD_DEAD;

	if (self.deathtype == "teledeath"||self.deathtype == "teledeath2"||self.deathtype == "teledeath3"||self.deathtype == "teledeath4")
	{
		sound (self, CHAN_VOICE, "player/telefrag.wav", 1, ATTN_NONE);
		return;
	}

	if(self.health<-80)
		sound (self, CHAN_VOICE, "player/megagib.wav", 1, ATTN_NONE);
	else if (random() < 0.5)
		sound (self, CHAN_VOICE, "player/gib1.wav", 1, ATTN_NONE);
	else
		sound (self, CHAN_VOICE, "player/gib2.wav", 1, ATTN_NONE);
}

void DecapPlayer ()
{
entity headless;
	headless=spawn();
	headless.classname="headless";
	headless.decap=TRUE;
	headless.movetype=MOVETYPE_STEP;
	headless.solid=SOLID_PHASE;
	headless.frame=50;
	headless.skin=self.skin;
//Took this out so you can't fall "into" it...
//	headless.owner=self;
	headless.thingtype=self.thingtype;
	headless.angles_y=self.angles_y;

	setmodel(headless,self.model);
	setsize(headless,'-16 -16 0','16 16 36');
	setorigin(headless,self.origin);

	headless.playerclass=self.playerclass;
	headless.think=self.th_goredeath;
	thinktime headless : 0;

	self.health=self.health*4;
	if(self.health>-30)
		self.health=-30;
	if(self.decap==2)
	{
		ThrowHead ("models/flesh1.mdl", self.health);
		SpawnPuff(self.origin+self.view_ofs,'0 0 0',fabs(self.health),self);
	}
	else
		ThrowHead (self.headmodel, self.health);
	ThrowGib ("models/flesh1.mdl", self.health);
	ThrowGib ("models/flesh2.mdl", self.health);
	ThrowGib ("models/flesh3.mdl", self.health);

	self.deadflag = DEAD_DEAD;
	if (random() < 0.5)
		sound(self,CHAN_VOICE,"player/decap.wav",1,ATTN_NORM);
	else if (random() < 0.5)
		sound (self, CHAN_VOICE, "player/gib1.wav", 1, ATTN_NONE);
	else
		sound (self, CHAN_VOICE, "player/gib2.wav", 1, ATTN_NONE);
}

void PlayerDie ()
{
	if(self.viewentity!=self)
	{
		if(self.viewentity.classname=="chasecam")
			remove(self.viewentity);
		self.viewentity=self;
		CameraViewPort(self,self);
		CameraViewAngles(self,self);
	}

	if(self.gravity!=self.standard_grav)
		self.gravity=self.standard_grav;

	msg_entity=self;
	WriteByte(MSG_ONE, SVC_CLEAR_VIEW_FLAGS);
	WriteByte(MSG_ONE,255);

	self.artifact_low =
	self.artifact_active =
	self.invisible_time =
	self.effects=
	self.colormap=0;

	if (deathmatch || coop)
		DropBackpack();

	if(self.model=="models/sheep.mdl")
		self.headmodel="";

	self.weaponmodel="";
	self.deadflag = DEAD_DYING;
	self.solid = SOLID_NOT;
	self.flags(-)FL_ONGROUND;
	self.movetype = MOVETYPE_TOSS;
	self.attack_finished=self.teleport_time=self.pausetime=time;
	self.drawflags=self.effects=FALSE;
	if (self.velocity_z < 10)
		self.velocity_z += random(300);

	self.artifact_active = 0;
	self.rings_active =0;

	if (self.deathtype == "teledeath"||self.deathtype == "teledeath2"||self.deathtype == "teledeath3"||self.deathtype == "teledeath4")
	{
		self.decap=0;
		self.health=-99;
	}

	if(self.deathtype=="ice shatter"||self.deathtype=="stone crumble")
	{
		shatter();
		ThrowHead(self.headmodel,self.health);
		if(self.health<-99)
			self.health=-99;
		return;
	}
	else if(self.decap)
	{
		DecapPlayer();
		if(self.health<-99)
			self.health=-99;
		return;
	}
	else if(self.health < -40||self.model=="models/sheep.mdl")//self.modelindex==modelindex_sheep)
	{
		GibPlayer ();
		if(self.health<-99)
			self.health=-99;
		return;
	}

	DeathSound();

	self.angles_x = 0;
	self.angles_z = 0;

	if(self.bloodloss==666)
		DecapPlayer();
	else
	{
		self.act_state=ACT_DEAD;
		player_frames();
	}
	if(self.health<-99)
		self.health=-99;
}

void set_suicide_frame ()
{	// used by kill command and disconnect command
	if (self.model != self.init_model)
		return;	// already gibbed
//have a self.deathframe value?  Or just if-thens
//	self.frame = $deatha11;
	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_TOSS;
	self.deadflag = DEAD_DEAD;
	self.nextthink = -1;
}

void Head ()
{
	ThrowSolidHead(0);
}

void Corpse ()
{
	MakeSolidCorpse();
}

void SolidPlayer ()
{
entity corpse;
	corpse = spawn();
	if(self.angles_x>15||self.angles_x<-15)
		self.angles_x=0;
	if(self.angles_z>15||self.angles_z<-15)
		self.angles_z=0;
	corpse.angles = self.angles;
	setmodel(corpse,self.model);
	corpse.frame = self.frame;
	corpse.colormap = self.colormap;
	corpse.movetype = self.movetype;
	corpse.velocity = self.velocity;
	corpse.flags = 0;
	corpse.effects = 0;
	corpse.skin = self.skin;
	corpse.controller = self;
	corpse.thingtype=self.thingtype;
	setorigin (corpse, self.origin);
	if(self.model==self.headmodel)
	{
		self.classname="head";//So they don't get mixed up with players
		corpse.think=Head;
	}
	else
	{
		self.classname="corpse";//So they don't get mixed up with players
		corpse.think=Corpse;
	}
	thinktime corpse : 0;
}

void player_behead ()
{
	self.frame=self.level+self.cnt;
	makevectors(self.angles);
	if(!self.cnt)
		MeatChunks (self.origin + '0 0 50',v_up*200, 3,self);
	else if (self.cnt==1)
	{
		SpawnPuff (self.origin+v_forward*8, '0 0 48', 30,self);
		sound (self, CHAN_AUTO, "misc/decomp.wav", 1, ATTN_NORM);
	}
	else if (self.cnt==3)
	{
		SpawnPuff (self.origin+v_forward*16, '0 0 36'+v_forward*16, 20,self);
		sound (self, CHAN_AUTO, "misc/decomp.wav", 1, ATTN_NORM);
	}
	else if (self.cnt==5)
	{
		SpawnPuff (self.origin+v_forward*28, '0 0 20'+v_forward*32, 15,self);
		sound (self, CHAN_AUTO, "misc/decomp.wav", 0.8, ATTN_NORM);
	}
	else if (self.cnt==8)
	{
		SpawnPuff (self.origin+v_forward*40, '0 0 10'+v_forward*40, 10,self);
		sound (self, CHAN_AUTO, "misc/decomp.wav", 0.6, ATTN_NORM);
	}
	if (self.frame==self.dmg)
	{
		SpawnPuff (self.origin+v_forward*56, '0 0 -5'+v_forward*40, 5,self);
		sound (self, CHAN_AUTO, "misc/decomp.wav", 0.4, ATTN_NORM);
		ReadySolid();
	}
	else
	{
		self.think=player_behead;
		thinktime self : 0.1;
	}
	self.cnt+=1;
}
