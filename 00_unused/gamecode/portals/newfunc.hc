//.float weapon_frame_state;
.float wfs;
//Valid Weapon fram states
float WF_NORMAL_ADVANCE = 0;
float WF_CYCLE_STARTED = 1;
float WF_CYCLE_WRAPPED = 2;
float WF_LAST_FRAME = 3;


//float (float startframe, float endframe) AdvanceWeaponFrame =
float (float startframe, float endframe) AWF =
{
	if( (endframe>startframe&&(self.weaponframe>endframe||self.weaponframe<startframe)) ||
	(endframe<startframe&&(self.weaponframe<endframe||self.weaponframe>startframe)) )
		{
			self.weaponframe=startframe;
			return WF_CYCLE_STARTED;
		}
	else if(self.weaponframe==endframe)
	{			  
		self.weaponframe=startframe;
		return WF_CYCLE_WRAPPED;
	}
	
	if(startframe>endframe)
		self.weaponframe=self.weaponframe - 1;
	else if(startframe<endframe)
		self.weaponframe=self.weaponframe + 1;

	if(self.weaponframe==endframe)
		return WF_LAST_FRAME;
	else 
		return WF_NORMAL_ADVANCE;
};