//FIRE HURT FIELD========================================================
void fire_hurt_field_touch ()
{
	if(self.attack_finished>time)
		return;
	if(self.inactive)
		return;
	if(other.health<=0)
		return;

	self.attack_finished=time+HX_FRAME_TIME;
	T_Damage(other,self,self,self.dmg);
	if(self.owner.classname=="big greek fire")
		if(!other.flags2&FL2_ONFIRE);
		{//torch 'em
			if(flammable(other))
				spawn_burner(other,TRUE);
		}
		else
			other.fire_damage += 1;//burn more!

	if(self.t_width<time)
	{
		self.t_width=time+0.6;
		sound(self,CHAN_WEAPON,"crusader/sunhit.wav",1,ATTN_NORM);
	}
}

void init_fire_hurt_field ()
{
	InitTrigger();
	self.touch=fire_hurt_field_touch;
}

void spawn_burnfield (vector org)
{//Make a trigger_flame_hurt around flame
entity fhf;
	fhf=spawn();
	fhf.model=self.model;
	fhf.effects=EF_NODRAW;
	self.trigger_field=fhf;
	fhf.owner=self;
	//	setsize(fhf,'-3 -3 0','3 3 9');
	setorigin(fhf,org);
	if(self.dmg)
		fhf.dmg=self.dmg;
	else
		fhf.dmg=self.dmg=.2;

	fhf.classname="fire hurt field";
	fhf.think=init_fire_hurt_field;
	fhf.nextthink=time;
}

//BIG FIRE========================================================

void burn_out ()
{
	if(self.scale>0.1)
	{
		self.scale-=0.1;
		setsize(self.trigger_field,'-24 -24 0'*self.scale,'24 24 48'*self.scale);
		thinktime self : 0.01;
		self.dmg=self.trigger_field.dmg=self.scale*3;
	}
	else
	{
		remove(self.trigger_field);
		stopSound(self,0);
		remove(self);
	}
}

/*QUAKED light_newfire (0 1 0) (-10 -10 -13) (10 10 41) START_LOW HURT
Large yellow flame
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)
HURT = Will hurt things that touch it.
.dmg = how much to hurt people who touch fire - damage/20th of a second.  Default = .2;

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void spawn_big_fire(vector org)
{
entity bigfire;
	bigfire=spawn();
	
	oself=self;
	self = bigfire;

	self.dmg=2;
	self.classname="big greek fire";
	self.drawflags(+)SCALE_ORIGIN_BOTTOM|DRF_TRANSLUCENT|MLS_FIREFLICKER;
	self.scale = 1;
	setmodel(self,"models/newfire.mdl");
	setorigin(self,org);
	spawn_burnfield(org);
	setsize(self.trigger_field,'-24 -24 0','24 24 48');
	sound (self,CHAN_LOOP+PHS_OVERRIDE_R, "misc/flamloop.wav", 0.5, ATTN_LOOP);
	self.think = burn_out;
	thinktime self : 10;

	self=oself;
};

