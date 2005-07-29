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
	if(self.viewentity.classname=="chasecam")
	{
//Turn off camera view
		CameraViewPort(self,self);
		CameraViewAngles(self,self);
		self.attack_finished=0;
		W_SetCurrentAmmo();
		remove(self.viewentity);
		self.viewentity=self;
		self.view_ofs=self.proj_ofs+'0 0 6';
	}
	else
	{
		self.lastweapon=self.weaponmodel;
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
}
