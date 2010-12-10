/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/camera.hc,v 1.2 2007-02-07 16:56:59 sezero Exp $
 */
void(entity voyeur, entity viewthing) CameraViewPort =
{//FIXME: Doesn't seem to work if it's out of vis- only 
//	remembers last spot it was at last time it WAS in vis
	msg_entity = voyeur;                        
	WriteByte (MSG_ONE, SVC_SETVIEWPORT);   
	WriteEntity (MSG_ONE, viewthing);      
};

void(entity voyeur, entity viewthing) CameraViewAngles =
{//FIXME: Doesn't seem to work if it's out of vis- only 
//	remembers last angles it was at last time it WAS in vis
	msg_entity = voyeur;                        
	WriteByte (MSG_ONE, SVC_SETVIEWANGLES); 
	if(viewthing.classname=="camera_remote")
		WriteAngle(MSG_ONE, 360-viewthing.angles_x);
	else
		WriteAngle(MSG_ONE, viewthing.angles_x);
	WriteAngle(MSG_ONE, viewthing.angles_y);
	WriteAngle(MSG_ONE, viewthing.angles_z);
};

/*QUAKED target_null (1 0 0) (-8 -8 -8) (8 8 8)
A null target for the camera 
-------------------------FIELDS-------------------------
none
--------------------------------------------------------
*/
void target_null (void)
{
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_NONE;

	precache_model ("models/sprites/null.spr");
	setmodel(self,"models/sprites/null.spr");
}


/*-----------------------------------------
	play_camera - play noise for when popping in or out of camera mode
  -----------------------------------------*/
void() play_camera =
{
	sound (self, CHAN_VOICE, "misc/camera.wav", 1, ATTN_NORM);
	remove (self);
};


/*-----------------------------------------
	CameraUse - place player in camera remote
  -----------------------------------------*/
void CameraUse (void)
{
	entity snd_ent;

	other = other.enemy;	// Use the enemy of the trigger or button

	if (other.classname != "player")
		return;

	stuffcmd (other, "bf\n");

	if(deathmatch)
		other.view_ofs='0 0 0';

	other.cameramode = self;
	other.camera_time = time + self.wait;

	other.oldangles = other.angles;
	other.lastweapon = other.weaponmodel;
	other.weaponmodel = string_null;

	CameraViewPort(other,self);
	WriteByte (MSG_ONE, 10);				// 10 = SVC_SETVIEWANGLES
	WriteAngle (MSG_ONE,360 - self.angles_x);		// pitch
	WriteAngle (MSG_ONE,self.angles_y);		// yaw
	WriteAngle (MSG_ONE,self.angles_z);		// roll

	snd_ent = spawn ();
	snd_ent.origin = self.origin;

	thinktime snd_ent : HX_FRAME_TIME;
	snd_ent.think = play_camera;
}

/*-----------------------------------------
	CameraReturn - return the player to his body
  -----------------------------------------*/
void CameraReturn(void)
{
	entity snd_ent;

	self.cameramode = world;
	self.weaponmodel = self.lastweapon;
	self.view_ofs=self.proj_ofs+'0 0 6';

	stuffcmd (self, "bf\n");

	self.angles = self.oldangles;
	self.idealroll = 0;

	CameraViewPort(self,self);
	CameraViewAngles(self,self);

	snd_ent = spawn ();
	snd_ent.origin = self.origin;
	thinktime snd_ent : HX_FRAME_TIME;
	snd_ent.think = play_camera;
}

/*-----------------------------------------
	camera_target- point the camera at it's target
  -----------------------------------------*/
void camera_target (void)
{
	self.enemy = find (world,targetname,self.target);
	self.angles = vectoangles(self.enemy.origin - self.origin);
}

/*QUAKED camera_remote (1 0 0) (-8 -8 -8) (8 8 8)
A camera which the player becomes when triggered. 
-------------------------FIELDS-------------------------
"wait" - amount of time player is stuck in camera mode
   (default 3 seconds)

To point camera in a direction

target a "target_null" entity


or fill out the angle fields:
angles_x - pitch of camera
angles_y - yaw of camera


--------------------------------------------------------
*/
void camera_remote (void)
{
	if(deathmatch)
	{
		remove(self);
		return;
	}

	self.solid = SOLID_NOT;
	self.movetype = MOVETYPE_NONE;

	precache_model ("models/sprites/null.spr");
	setmodel(self,"models/sprites/null.spr");

	self.use = CameraUse;

	if (!self.wait)
		self.wait = 3;

	if (self.target)	
	{
		self.think = camera_target;
		self.nextthink = time + 0.02; // Give target a chance to spawn into the world
	}

	setsize(self, '-1 -1 0', '1 1 1');
}

void CameraThink ()
{
vector viewdir;
vector spot1,spot2;
	self.level=cvar("chase_back");
	if(self.cnt<self.level)
		self.cnt+=1;
	else if(self.cnt>self.level)
		self.cnt-=1;

	makevectors(self.owner.v_angle);
	viewdir=normalize(v_forward);
	spot1=self.owner.origin+self.owner.proj_ofs+'0 0 6';
	spot2=spot1-viewdir*self.cnt;
	traceline(spot1,spot2,TRUE,self.owner);

	viewdir=normalize(spot1-trace_endpos);
	setorigin(self,trace_endpos+viewdir*4);

	self.think=CameraThink;
	thinktime self : 0;
}

void MakeCamera ()
{
entity oself,nself;
	if(!self.flags&FL_CLIENT)
	{
		if(coop||deathmatch)
			return;
		nself=find(world,classname,"player");
		if(!nself.flags&FL_CLIENT)
			return;
		oself=self;
		self=nself;
	}

	if(self.viewentity.classname=="chasecam")
	{
//Turn off camera view
		CameraViewPort(self,self);
		CameraViewAngles(self,self);
		remove(self.viewentity);
		self.viewentity=self;
		self.view_ofs=self.proj_ofs+'0 0 6';
		self.attack_finished=0;
		self.weaponmodel=self.lastweapon;
		self.oldweapon=FALSE;
		W_SetCurrentWeapon();
	}
	else
	{
		self.lastweapon=self.weaponmodel;
		self.oldweapon=0;
		self.weaponmodel="";
		makevectors(self.v_angle);
		self.viewentity=spawn();
		self.viewentity.owner=self;
		self.viewentity.angles=self.angles;
		self.viewentity.level=cvar("chase_back");
		if(!self.viewentity.level)
			self.viewentity.level=68;
		self.viewentity.cnt=4;
		self.viewentity.classname="chasecam";
		self.view_ofs='0 0 0';

		setmodel(self.viewentity,"models/null.spr");
		setsize(self.viewentity, '0 0 0','0 0 0');
		setorigin(self.viewentity,self.origin+self.proj_ofs+'0 0 6'-v_forward*4);

		CameraViewPort(self,self.viewentity);
		CameraViewAngles(self,self.viewentity);

		self.viewentity.think=CameraThink;
		thinktime self.viewentity : 0;
	}
	if(oself)
		self=oself;
}

