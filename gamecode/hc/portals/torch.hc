/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/torch.hc,v 1.3 2007-02-07 16:59:37 sezero Exp $
 */
/*
==========================================================
TORCH.HC
MG

Torches can be toggled/faded, shot out, etc.
==========================================================
*/

void Init_Torch ()
{
	if(self.targetname)
		self.use=torch_use;

	self.solid		= SOLID_BBOX;
	self.movetype	= MOVETYPE_NONE;
	if(self.health)
	{
//		self.skin=1;
		if(!self.targetname)
			dprint("No targetname for breakable torch\n");
		if(self.style<32)
		{
			dprint(ftos(self.style));
			dprint(": Bad lightstyle for breakable torch\n");
		}
		self.takedamage=DAMAGE_YES;
		self.th_die=torch_death;
		self.classname="breakable_torch";
	}

	if(self.style>=32)
	{
		if(!self.lightvalue2)
			self.lightvalue2=11;
		if(!self.fadespeed)
			self.fadespeed = 1;
		initialize_lightstyle();
		self.think = torch_think;
		thinktime self : 1;
	}
	else
	{
		self.drawflags(+)MLS_ABSLIGHT;
		setmodel(self,self.weaponmodel);
		if(self.health!=0||self.targetname!="")
		{
			dprint(ftos(self.style));
			dprint(": Bad lightstyle for breakable torch\n");
		}
		else
			makestatic(self);
	}
}

/*QUAKED light_candle (0 1 0) (-7 -7 -15) (7 7 31) START_LOW
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
"abslight" You can give it explicit lighting so it doesn't glow (0 to 2.5)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
.fadespeed (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
--------------------------------------------------------
*/
void light_candle (void)
{
	precache_model4("models/candle.mdl");
	self.drawflags(+)MLS_ABSLIGHT;
	if(!self.abslight)
		self.abslight = .75;

	self.mdl = "models/candle.mdl";
	self.weaponmodel = "models/candle.mdl";	//FIXME: Flame On!

	self.thingtype	= THINGTYPE_GREYSTONE;
	setsize(self, '-6 -6 -8','6 6 8');

	FireAmbient();
	Init_Torch();
	self.solid=SOLID_BBOX;
}

/*QUAKED light_torch_eqypt (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
An Egyptian style torch that displays light
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
"abslight" You can give it explicit lighting so it doesn't glow (0 to 2.5)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
"fadespeed" (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void light_torch_eqypt()
{
	precache_model2("models/egtorch.mdl");
	precache_model2("models/eflmtrch.mdl");
	precache_sound("raven/flame1.wav");

	self.abslight = .75;

	self.mdl = "models/egtorch.mdl";
	self.weaponmodel = "models/eflmtrch.mdl";

	self.thingtype	= THINGTYPE_WOOD;
	setsize(self, '-6 -6 -8','6 6 8');

	Init_Torch();
}


/*QUAKED light_torch_castle (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
The Castle style torch that displays light
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
"fadespeed" (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void light_torch_castle()
{
	precache_model("models/castrch.mdl");
	precache_model("models/cflmtrch.mdl");
	precache_sound("raven/flame1.wav");

	self.weaponmodel="models/cflmtrch.mdl";
	self.mdl= "models/castrch.mdl";

	self.thingtype	= THINGTYPE_METAL;
	setsize(self, '-6 -6 -8','6 6 8');

	self.abslight = .75;

	Init_Torch();

}


/*QUAKED light_torch_meso (0 1 0) (-12 -12 -16) (12 12 16) START_LOW
The Meso style torch that displays light
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
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
void light_torch_meso()
{
	precache_model2("models/mesotrch.mdl");
	precache_model2("models/mflmtrch.mdl");
	precache_sound("raven/flame1.wav");

	self.weaponmodel="models/mflmtrch.mdl";
	self.mdl	= "models/mesotrch.mdl";

	self.abslight = .75;

	self.thingtype	= THINGTYPE_BROWNSTONE;
	setsize(self, '-12 -12 -16','12 12 16');

	Init_Torch();
}


/*QUAKED light_torch_rome (0 1 0) (-8 -8 -8) (8 8 8) START_LOW
The Roman style torch that displays light
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
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

void light_torch_rome()
{
	precache_model("models/rometrch.mdl");
	precache_model("models/rflmtrch.mdl");
	precache_sound("raven/flame1.wav");

	self.weaponmodel="models/rflmtrch.mdl";
	self.mdl	= "models/rometrch.mdl";

	self.thingtype	= THINGTYPE_GREYSTONE;
	setsize(self, '-6 -6 -8','6 6 8');

	self.abslight = .75;

	Init_Torch();

}

/*QUAKED light_burner (0 1 0) (-16 -18 -52) (16 18 0) START_LOW DAMAGE
A brazier
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
"abslight" You can give it explicit lighting so it doesn't glow (0 to 2.5)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
"fadespeed" (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
--------------------------------------------------------
*/
void light_burner (void)
{
	precache_model4("models/burner.mdl");
	precache_model4("models/burnerfl.mdl");
	precache_model ("models/flame2.mdl");
	if(!self.abslight)
		self.abslight = .75;
	self.drawflags(+)MLS_ABSLIGHT;

	self.mdl = "models/burner.mdl";
	self.weaponmodel = "models/burnerfl.mdl";	//Flame On!

	self.movechain=spawn();
	setorigin(self.movechain,self.origin+'0 0 6');

	if(self.spawnflags&2)
		spawn_burnfield(self.movechain.origin);
	else
		self.dmg=0;

	self.thingtype	= THINGTYPE_CLAY;
	setsize(self, '-17 -17 -52','17 17 0');

	self.solid=SOLID_BBOX;

//Put Flame on top

	self.movechain.abslight = .75;

	setmodel(self.movechain,"models/flame2.mdl");
	self.movechain.drawflags(+)MLS_ABSLIGHT;
	FireAmbient();

	Init_Torch();
	makestatic (self.movechain);
}

/*QUAKED light_lantern (0 1 0) (-11 -11 -41) (11 11 5) START_LOW
A castle lantern that hangs on the wall
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
"abslight" You can give it explicit lighting so it doesn't glow (0 to 2.5)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
"fadespeed" (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
*/
void light_lantern ()
{
	precache_model4("models/lantern.mdl");//fixme-no flame this version
	precache_model4("models/lantern.mdl");
	precache_sound("raven/flame1.wav");

	self.drawflags(+)MLS_ABSLIGHT;
	self.abslight = .75;

	self.mdl = "models/lantern.mdl";
	self.weaponmodel = "models/lantern.mdl";

	self.thingtype	= THINGTYPE_METAL;
	setsize(self, '-11 -11 -41','11 11 5');

	Init_Torch();
	FireAmbient();
}


/*QUAKED light_palace_torch (0 1 0) (-19 -19 0) (19 19 35) START_LOW DAMAGE
A palatial wall torch of some sort
Default light value is 300

.health = If you give the torch health, it can be shot out.  It will automatically select it's second skin (the beat-up torch look)
You must give it a targetname too, just any junk targetname will do like "junk"
----------------------------------
If triggered, will toggle between lightvalue1 and lightvalue2
"lightvalue1" (default 0) 
"lightvalue2" (default 11, equivalent to 300 brightness)
"abslight" You can give it explicit lighting so it doesn't glow (0 to 2.5)
Two values the light will fade-toggle between, 0 is black, 25 is brightest, 11 is equivalent to a value of 300.
"fadespeed" (default 1) = How many seconds it will take to complete the desired lighting change
The light will start on at a default of the higher light value unless you turn on the startlow flag.
START_LOW = will make the light start at the lower of the lightvalues you specify (default uses brighter)

NOTE: IF YOU DON'T PLAN ON USING THE DEFAULTS, ALL LIGHTS IN THE BANK OF LIGHTS NEED THIS INFO
--------------------------------------------------------
*/
void light_palace_torch (void)
{
	precache_model4("models/palight.mdl");
	precache_model4("models/palight.mdl");
	precache_model ("models/flame2.mdl");
	if(!self.abslight)
		self.abslight = .75;
	self.drawflags(+)MLS_ABSLIGHT;

	self.mdl = "models/burner.mdl";
	self.weaponmodel = "models/palight.mdl";	//Flame On!

	self.movechain=spawn();
	setorigin(self.movechain,self.origin+'0 0 32');

	if(self.spawnflags&2)
		spawn_burnfield(self.movechain.origin);
	else
		self.dmg=0;

	self.thingtype	= THINGTYPE_BROWNSTONE;
	setsize(self, '-19 -19 0','19 19 35');

	self.solid=SOLID_BBOX;

//Put Flame on top

	self.movechain.abslight = .75;

	setmodel(self.movechain,"models/flame2.mdl");
	self.movechain.drawflags(+)MLS_ABSLIGHT;
	FireAmbient();

	Init_Torch();
	makestatic (self.movechain);
}

