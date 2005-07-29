/*
FORCE FIELD
MG
*/

void() ForceFieldDie =
{
	//FIXME: add more
	self.flags2-=FL_SHIELDED;
	remove(self.shield);
};

void() ForceFieldPain =
{
	if(self.shield.pain_finished>time)
		return;
	sound(self.shield,CHAN_AUTO,"misc/pulse.wav",1,ATTN_NORM);
	stuffcmd (self, "bf\n");
	self.shield.pain_finished=time + 1;
	if(self.shield.drawflags&MLS_POWERMODE)
	{
		self.shield.drawflags = MLS_ABSLIGHT + DRF_TRANSLUCENT;
		self.shield.abslight = 1;
	}
};

void() ForceFieldThink=
{
	if(self.pain_finished<time)
		if(self.drawflags&MLS_ABSLIGHT)
		{
			self.drawflags = MLS_POWERMODE + DRF_TRANSLUCENT;
			self.abslight = 0.5;
		}
	setorigin(self,(self.owner.absmax+self.owner.absmin)*0.5);
	self.think=ForceFieldThink;
	thinktime self : 0;
};

void() MakeForceField=
{
	if(self.flags2&FL_SHIELDED)
		return;
	centerprint(self,"Force Field active\n");
	sound(self,CHAN_AUTO,"misc/pulse.wav",1,ATTN_NORM);
	stuffcmd (self, "bf\n");
	self.shield=spawn();
	self.shield.owner=self;
	self.shield.movetype=MOVETYPE_NOCLIP;
	self.shield.solid=SOLID_NOT;
	self.shield.health=200;
	self.shield.th_die=ForceFieldDie;
	self.shield.th_pain=ForceFieldPain;
	self.flags2+=FL_SHIELDED;

	setmodel(self.shield,"models/blast.mdl");
	self.shield.scale=0.3;
	self.shield.skin=1;
	self.shield.drawflags=DRF_TRANSLUCENT+MLS_POWERMODE;
	setsize(self.shield,'0 0 0','0 0 0');
	setorigin(self.shield,(self.absmax+self.absmin)*0.5);

	self.shield.think=ForceFieldThink;
	self.shield.nextthink=time;
};