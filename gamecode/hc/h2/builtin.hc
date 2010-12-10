
//**************************************************************************
//**
//** builtin.hc
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/h2/builtin.hc,v 1.1.1.1 2004-11-29 11:37:19 sezero Exp $
//**
//**************************************************************************

// Sets v_forward, etc.
void makevectors(vector ang) : 1;

void setorigin(entity e, vector o) : 2;

// Set movetype and solid before calling.
void setmodel(entity e, string m) : 3;

void setsize(entity e, vector min, vector max) : 4;
void lightstylestatic(float style, float value) : 5;
void debugbreak(void) : 6;

// Returns 0 - 1.
float random(void) : 7;

void sound(entity e, float chan, string samp, float vol, float atten) : 8;
vector normalize(vector v) : 9;
void error(string e) : 10;
void objerror(string e) : 11;
float vlen(vector v) : 12;
float vectoyaw(vector v) : 13;
entity spawn(void) : 14;
void remove(entity e) : 15;

// Sets trace_* globals.
void traceline(vector v1, vector v2, float nomonsters,
	entity forent) : 16;

// Returns a client to look for.
entity checkclient(void) : 17;

entity find(entity start, .string fld, string match) : 18;

// For sounds in demo and retail version.
string precache_sound(string s) : 19;

// For models in demo and retail version.
string precache_model(string s) : 20;

void stuffcmd(entity client, string s) : 21;
entity findradius(vector org, float rad) : 22;
void bprint(string s) : 23;
void sprint(entity client, string s) : 24;
void dprint(string s) : 25;
string ftos(float f) : 26;
string vtos(vector v) : 27;

// Prints all edicts.
void coredump(void) : 28;

void traceon(void) : 29;
void traceoff(void) : 30;

// Prints an entire edict.
void eprint(entity e) : 31;

// Returns TRUE or FALSE.
float walkmove (float yaw, float dist, float set_trace) : 32;

float tracearea(vector v1, vector v2, vector mins, vector maxs,
	float nomonsters, entity forent) : 33;

// TRUE if landed on floor.
float droptofloor(void) : 34;

void lightstyle(float style, string value) : 35;

// Round to nearest int.
float rint(float v) : 36;

// Largest integer <= v.
float floor(float v) : 37;

// Smallest integer >= v.
float ceil(float v) : 38;

// Award experience to player.
//void AwardExperience(entity ToEnt, entity FromEnt, float Amount) : 39;

// TRUE if self is on ground.
float checkbottom(entity e) : 40;

// Returns a CONTENT_*.
float pointcontents(vector v) : 41;

// Start a particle effect.
void particle2(vector o, vector dmin, vector dmax, float color,
	float type, float count) : 42;

float fabs(float f) : 43;

// Returns a shooting vector.
vector aim(entity e,vector d,float speed) : 44;

// Returns the cvar value.
float cvar(string s) : 45;

// Put a string into local que.
void localcmd(string s) : 46;

// For looping through all ents.
entity nextent(entity e) : 47;

// Start a particle effect.
void particle(vector o, vector d, float color, float count) : 48;

// Turn towards self.ideal_yaw at self.yaw_speed.
float ChangeYaw(void) : 49;

// Calculate distance, ignoring z.
float vhlen(vector v) : 50;

vector vectoangles(vector v) : 51;

void WriteByte(float to, float f) : 52;
void WriteChar(float to, float f) : 53;
void WriteShort(float to, float f) : 54;
void WriteLong(float to, float f) : 55;
void WriteCoord(float to, float f) : 56;
void WriteAngle(float to, float f) : 57;
void WriteString(float to, string s) : 58;
void WriteEntity(float to, entity s) : 59;
void dprintf(string s, float num) : 60;
float cos(float angle) : 61;
float sin(float angle) : 62;
float AdvanceFrame(float start, float end) : 63;
void dprintv(string s, vector vec) : 64;
float RewindFrame(float start, float end) : 65;

void setclass(entity e, float value) : 66;

void movetogoal(float step) : 67;

// For files in demo and retail version.
string precache_file(string s) : 68;

void makestatic(entity e) : 69;
void changelevel(string level, string spot) : 70;

// Returns the current value of a light style.
float lightstylevalue(float style) : 71;

// Sets a cvar value.
void cvar_set(string var, string val) : 72;

// Same as sprint(), but centered.
void centerprint(entity client, string s) : 73;

void ambientsound(vector pos, string samp, float vol, float atten) : 74;

// For models only in retail version.
string precache_model2(string s) : 75;

// For sounds only in retail version.
string precache_sound2(string s) : 76;

// For files only in retail version.
string precache_file2(string s) : 77;


// Sets parm1... to the values at level start for coop respawn.
void setspawnparms(entity e) : 78;

void plaque_draw(float to, float index) : 79;

// Create rain.
void rain_go(vector v1, vector v2, vector e_size, vector dir,
	float color, float count) : 80;

// Particle explosion.
void particleexplosion(vector v,float f,float c,float s) : 81;

// Move a step.
float movestep(float x, float y, float z, float set_trace) : 82;

// Returns TRUE or FALSE (move other).
float advanceweaponframe(float startframe, float endframe) : 83;

float sqrt(float num1)   : 84;

void particle3(vector o, vector box, float color, float type, float count) : 85;
void particle4(vector o, float radius, float color, float type, float count) : 86;

// m will used as:  models/puzzle/m.mdl
void setpuzzlemodel(entity e, string m) : 87;

//float starteffect(float to, float effect) : 88;
float starteffect(...) : 88;
float endeffect(float to, float effect_id) : 89;

string precache_puzzle_model(string s) : 90;
vector concatv(vector in, vector limit) : 91;
string getstring(float id) : 92;
entity spawn_temp(void) : 93;
vector v_factor(vector factor) : 94;
	// returns (v_right * factor_x) + (v_forward * factor_y) + (v_up * factor_z)
vector v_factorrange(vector start, vector end) : 95;

string precache_sound3(string s) : 96;
string precache_model3(string s) : 97;
string precache_file3(string s) : 98;
