
//**************************************************************************
//**
//** global.hc
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/h2/global.hc,v 1.1.1.1 2004-11-29 11:38:23 sezero Exp $
//**
//**************************************************************************

// SYSTEM GLOBALS (globalvars_t C structure) -------------------------------

entity self;
entity other;
entity world;
float time;
float frametime;

// Force all entities to touch triggers next frame.  This is needed
// because non-moving things don't normally scan for triggers, and
// when a trigger is created (like a teleport trigger), it needs to
// catch everything.  Decremented each frame, so set to 2 to guarantee
// everything is touched.
float force_retouch;

string mapname;
string startspot;

float deathmatch;
float randomclass;
float coop;
float teamplay;

// Propagated from level to level, used to keep track of completed
// episodes.
float serverflags;

float total_secrets;
float total_monsters;

// Number of secrets found.
float found_secrets;

// Number of monsters killed.
float killed_monsters;
float chunk_cnt;			// # of chunks currently existing (don't want to exceed max)

// Set by monster spawner to make sure monster init functions don't
// precache models after level is started.
float done_precache;

// Spawnparms are used to encode information about clients across server
// level changes.
float parm1, parm2, parm4, parm5, parm6, parm7, parm8,
	parm9, parm10, parm11, parm12, parm13, parm14, parm15, parm16;

string parm3;

// Set by makevectors()
vector v_forward, v_up, v_right;

// Set by traceline().
float trace_allsolid;
float trace_startsolid;
float trace_fraction;
vector trace_endpos;
vector trace_plane_normal;
float trace_plane_dist;
entity trace_ent;
float trace_inopen;
float trace_inwater;

// Destination of single entity writes.
entity msg_entity;

// Set by OP_CSTATE ([++ $s..$e], [-- $s..$e]).
float cycle_wrapped;

float crouch_cnt;

float modelindex_assassin;
float modelindex_crusader;
float modelindex_paladin;
float modelindex_necromancer;
float modelindex_sheep;

float num_players;
float exp_mult;

// REQUIRED FUNCTIONS ------------------------------------------------------

// Only for testing
void main(void);

void StartFrame(void);
void PlayerPreThink(void);
void PlayerPostThink(void);
void ClientKill(void);
void ClientConnect(void);
void PutClientInServer(void);
void ClientReEnter(float TimeDiff);
void ClientDisconnect(void);
void ClassChangeWeapon(void);

// END SYSTEM GLOBALS ------------------------------------------------------

// Flag the compiler.
void end_sys_globals;

float movedist;

// Set when a rule exits.
float gameover;

// NULL string, nothing should be held here.
string string_null;

// Function launch_spike() sets this after spawning it.
entity newmis;

// The entity that activated a trigger or brush.
entity activator;

// Set by T_Damage().
entity damage_attacker;

float framecount;

float skill;

float wp_deselect;  // A flag showing a weapon is being deselected ignore impulse 10

