/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/hw/camera.hc,v 1.2 2007-02-07 16:57:50 sezero Exp $
 */
void(entity voyeur, entity viewthing) CameraViewPort =
{//FIXME: Doesn't work in H2W
	msg_entity = voyeur;                        
	WriteByte (MSG_ONE, SVC_SETVIEWPORT);   
	WriteEntity (MSG_ONE, viewthing);      
};

void(entity voyeur, entity viewthing) CameraViewAngles =
{//FIXME: Doesn't work in H2W
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

/*-----------------------
AllyVision (see through teammate's eyes)
------------------------*/

void AllyVision (entity voyeur,entity ally)
{
entity snd_ent;

//	if(deathmatch)
	if(voyeur.cameramode==world||voyeur.cameramode==voyeur)	
		if (voyeur.weaponmodel!= string_null)
		{
			voyeur.lastweapon = voyeur.weaponmodel;
			voyeur.weaponmodel = string_null;
		}

	voyeur.view_ofs=ally.view_ofs;

	voyeur.cameramode = ally;
	voyeur.camera_time = time + 10;
	voyeur.attack_finished = voyeur.camera_time;

	voyeur.oldangles = voyeur.angles;

	stuffcmd (voyeur, "bf\n");

	msg_entity = voyeur;                        
	
	CameraViewPort(voyeur,ally);
	WriteByte (MSG_ONE, 10);				// 10 = SVC_SETVIEWANGLES
	WriteAngle (MSG_ONE,ally.v_angle_x);		// pitch
	WriteAngle (MSG_ONE,ally.v_angle_y);		// yaw
	WriteAngle (MSG_ONE,ally.v_angle_z);		// roll
//put back in
	snd_ent = spawn ();
	snd_ent.origin = ally.origin;

	thinktime snd_ent : HX_FRAME_TIME;
	snd_ent.think = play_camera;

}
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

void ToggleChaseCam (entity voyeur)
{
	if(voyeur.viewentity.classname=="chasecam")
	{
//Turn off camera view
		CameraViewPort(voyeur,voyeur);
		CameraViewAngles(voyeur,voyeur);
		remove(voyeur.viewentity);
		voyeur.viewentity=voyeur;
		voyeur.view_ofs=voyeur.proj_ofs+'0 0 6';
		voyeur.attack_finished=0;
		voyeur.weaponmodel=voyeur.lastweapon;
		voyeur.oldweapon=FALSE;
		W_SetCurrentAmmo();
	}
	else
	{
		if(voyeur.cameramode!=voyeur&&voyeur.cameramode!=world)
			centerprint(voyeur,"Chase camera not available while in another camera mode\n");

		voyeur.lastweapon=voyeur.weaponmodel;
		voyeur.oldweapon=0;
		voyeur.weaponmodel="";
		makevectors(voyeur.v_angle);
		voyeur.viewentity=spawn();
		voyeur.viewentity.owner=voyeur;
		voyeur.viewentity.angles=voyeur.angles;
		voyeur.viewentity.level=cvar("chase_back");
		if(!voyeur.viewentity.level)
			voyeur.viewentity.level=68;
		voyeur.viewentity.cnt=4;
		voyeur.viewentity.classname="chasecam";
		voyeur.view_ofs='0 0 0';

		setmodel(voyeur.viewentity,"models/null.spr");
		setsize(voyeur.viewentity, '0 0 0','0 0 0');
		setorigin(voyeur.viewentity,voyeur.origin+voyeur.proj_ofs+'0 0 6'-v_forward*4);

		CameraViewPort(voyeur,voyeur.viewentity);
		CameraViewAngles(voyeur,voyeur.viewentity);

		voyeur.viewentity.think=CameraThink;
		thinktime voyeur.viewentity : 0;
	}
}

