void CheckAbilities ()
{
	if (self.model == "models/assassin.mdl")
	{
	float trans_limit;
		if (self.level < 3)
			return;
		trans_limit = 10 - self.level;
		if (trans_limit < 1)
			trans_limit = 1;
		if ((self.velocity == '0 0 0') && (self.last_attack < time - 2) &&
		    (self.light_level < 100))
		{
			if (self.still_time == -1)
				self.still_time = time + 10 - self.level;
			if (self.still_time < time)
			{
				msg_entity = self;
				WriteByte(MSG_ONE, SVC_SET_VIEW_FLAGS);
				WriteByte(MSG_ONE, DRF_TRANSLUCENT);
				if (!self.abslight)
					self.abslight = self.light_level/102;
				else if (self.abslight > 0.005*(8-trans_limit))
					self.abslight -= 0.005*(8-trans_limit);
				self.drawflags (+) (DRF_TRANSLUCENT | MLS_ABSLIGHT);
			}
		}
		else if (self.drawflags & (DRF_TRANSLUCENT | MLS_ABSLIGHT))
		{
			self.still_time = -1;
			if (self.abslight < 1)
				self.abslight += 0.02*trans_limit;
			else
			{
				self.drawflags (-) (MLS_ABSLIGHT | DRF_TRANSLUCENT);
				self.abslight = 0;
				msg_entity = self;
				WriteByte(MSG_ONE, SVC_CLEAR_VIEW_FLAGS);
				WriteByte(MSG_ONE, DRF_TRANSLUCENT);
			}
		}
	}
}
