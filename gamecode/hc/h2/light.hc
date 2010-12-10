/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/h2/light.hc,v 1.2 2007-02-07 16:57:07 sezero Exp $
 */
/*
==========================================================
LIGHT.HC
MG

Lights can be toggled/faded, shot out, etc.
==========================================================
*/

float START_LOW		= 1;

void initialize_lightstyle (void)
{
	if(self.spawnflags&START_LOW)
		if(self.lightvalue1<self.lightvalue2)
			lightstylestatic(self.style, self.lightvalue1);
		else
			lightstylestatic(self.style, self.lightvalue2);
	else
		if(self.lightvalue1<self.lightvalue2)
			lightstylestatic(self.style, self.lightvalue2);
		else
			lightstylestatic(self.style, self.lightvalue1);
}

void fadelight()
{
	self.frags+=self.cnt;
	self.light_lev+=self.frags;
	lightstylestatic(self.style, self.light_lev);
	self.count+=1;
	//dprint(ftos(self.light_lev));
	//dprint("\n");
	if(self.count/20>=self.fadespeed)
	{
		//dprint("light timed out\n");
		remove(self);
	}
	else if((self.cnt<0&&self.light_lev<=self.level)||(self.cnt>0&&self.light_lev>=self.level))
	{
		//dprint("light fade done\n");
		lightstylestatic(self.style, self.level);
		remove(self);
	}
	else
	{
		self.nextthink=time+0.05;
		self.think=fadelight;
	}
}

void lightstyle_change_think()
{
	//dprint("initializing light change\n");
	self.speed=self.lightvalue2 - self.lightvalue1;
	self.light_lev=lightstylevalue(self.style);
	if(self.light_lev==self.lightvalue1)
		self.level = self.lightvalue2;
	else if(self.light_lev==self.lightvalue2)
		self.level = self.lightvalue1;
	else if(self.speed>0)
		if(self.light_lev<self.lightvalue1+self.speed*0.5)
			self.level=self.lightvalue2;
		else
			self.level=self.lightvalue1;
	else if(self.speed<0)
		if(self.light_lev<self.lightvalue2-self.speed*0.5)
			self.level=self.lightvalue1;
		else
			self.level=self.lightvalue2;

	self.cnt=(self.level-self.light_lev)/self.fadespeed/20;
	self.think = fadelight;
	self.nextthink = time;
	/*dprint("light style # ");
	dprint(ftos(self.style));
	dprint(":\n");
	dprint("value1: ");
	dprint(ftos(self.lightvalue1));
	dprint("\n");
	dprint("value2: ");
	dprint(ftos(self.lightvalue2));
	dprint("\n");
	dprint("difference: ");
	dprint(ftos(self.speed));
	dprint("\n");
	dprint("fadespeed: ");
	dprint(ftos(self.fadespeed));
	dprint("\n");
	dprint("current light level: ");
	dprint(ftos(self.light_lev));
	dprint("\n");
	dprint("target light level: ");
	dprint(ftos(self.level));
	dprint("\n");
	dprint("luminosity interval: ");
	dprint(ftos(self.cnt));
	dprint("\n");*/
}

void lightstyle_change (entity light_targ)
{
//dprint("spawning light changer\n");
	newmis=spawn();
	newmis.lightvalue1=light_targ.lightvalue1;
	newmis.lightvalue2=light_targ.lightvalue2;
	newmis.fadespeed=light_targ.fadespeed;
	newmis.style=self.style;
	newmis.think=lightstyle_change_think;
	newmis.nextthink=time;
}

void torch_death ()
{
	lightstylestatic(self.style,0);
	chunk_death();
}

void torch_think (void)
{
float lightstate;
	lightstate=lightstylevalue(self.style);
	if(!lightstate)			//Use "off" frames
	{
		if(self.mdl)
			setmodel(self,self.mdl);
		self.drawflags(-)MLS_ABSLIGHT;
	}
	else
	{
		if(self.weaponmodel)
			setmodel(self,self.weaponmodel);
		self.drawflags(+)MLS_ABSLIGHT;
	}
	if(time>self.fadespeed)
		self.nextthink=-1;
	else
		self.nextthink=time+0.05;
	self.think=torch_think;
}

void torch_use (void)
{
	self.fadespeed=time+other.fadespeed+1;
	torch_think();
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
Non-displayed fading light.
Default light value is 300
Default style is 0
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void light()
{
	if (self.targetname == "")
	{
		remove(self);
	}
	else
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
	}
}

/*QUAK-ED light_globe (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
Sphere globe light.
Default light value is 300
Default style is 0
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
/*
void() light_globe =
{
	precache_model ("models/s_light.spr");
	setmodel (self, "models/s_light.spr");
	if(self.targetname)
		self.use=torch_use;
	self.mdl = "models/null.spr";
	self.weaponmodel = "models/s_light.spr";
	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};
*/

void() FireAmbient =
{
//FIXME: remove ambient sound if light is off, start it again if turned back on
	precache_sound ("raven/flame1.wav");
// attenuate fast
	ambientsound (self.origin, "raven/flame1.wav", 0.5, ATTN_STATIC);
};

/*QUAK-ED light_torch_small_walltorch (0 .5 0) (-10 -10 -20) (10 10 20) START_LOW
Short wall torch
Default light value is 200
Default style is 0
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void() light_torch_small_walltorch =
{
	precache_model ("models/flame.mdl");
	FireAmbient ();
	if(self.targetname)
		self.use=torch_use;
	self.mdl = "models/null.spr";
	self.weaponmodel = "models/flame.mdl";

	self.abslight = .75;

	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};

/*QUAKED light_flame_large_yellow (0 1 0) (-10 -10 -12) (12 12 18) START_LOW
Large yellow flame
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void() light_flame_large_yellow =
{
	precache_model ("models/flame1.mdl");
	FireAmbient ();
	if(self.targetname)
		self.use=torch_use;

	self.abslight = .75;

	self.mdl = "models/null.spr";
	self.weaponmodel = "models/flame1.mdl";
	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};

/*QUAKED light_flame_small_yellow (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
Small yellow flame ball
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void() light_flame_small_yellow =
{
	precache_model ("models/flame2.mdl");
	FireAmbient ();
	if(self.targetname)
		self.use=torch_use;

	self.abslight = .75;

	self.mdl = "models/null.spr";
	self.weaponmodel = "models/flame2.mdl";
	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};

/*QUAK-ED light_flame_small_white (0 1 0) (-10 -10 -40) (10 10 40) START_LOW
Small white flame ball
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
/*
void() light_flame_small_white =
{
	precache_model ("models/flame2.mdl");
	FireAmbient ();
	if(self.targetname)
		self.use=torch_use;

	self.abslight = .75;

	self.mdl = "models/null.spr";
	self.weaponmodel = "models/flame2.mdl";
	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};
*/

/*QUAKED light_gem (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
A gem that displays light.
Default light value is 300
Default style is 0
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
.lightvalue1 (default 0) 
.lightvalue2 (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void() light_gem =
{
	precache_model ("models/gemlight.mdl");
	if(self.targetname)
		self.use=torch_use;
	self.mdl = "models/null.spr";
	self.weaponmodel = "models/gemlight.mdl";

	self.abslight = .75;

	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		self.nextthink = time+1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		makestatic (self);
	}
};

