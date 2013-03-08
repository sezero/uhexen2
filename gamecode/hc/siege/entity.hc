
//**************************************************************************
//**
//** entity.hc
//**
//** $Header: /cvsroot/uhexen2/gamecode/hc/siege/entity.hc,v 1.3 2007-02-07 16:25:00 sezero Exp $
//**
//**************************************************************************

// SYSTEM FIELDS -----------------------------------------------------------
// (entvars_t C structure, *** = don't modify in HexC) ---------------------

// *** Model index in the precached list.
.float modelindex;

// *** Origin + mins / maxs
.vector absmin, absmax;

// Local time for entity.
.float ltime;

.float          lastruntime;    // *** to allow entities to run out of sequence

.float movetype;
.float solid;

// ***
.vector	origin;

// ***
.vector	oldorigin;

.vector	velocity;
.vector	angles;
.vector	avelocity;

// Temp angle adjust from damage or recoil.
.vector punchangle;

// Spawn function.
.string classname;

.string model;
.float frame;
.float skin;
.float effects;
.float scale;
.float drawflags;
.float abslight;

// Bounding box extents relative to origin.
.vector	mins, maxs;

// maxs - mins
.vector size;

// Which clipping hull to use.
.float	hull;

.void() touch;
.void() use;
.void() think;

// For doors or plats, called when can't push other.
.void()	blocked;

.float nextthink;
.entity groundentity;

// Stats
.float stats_restored;

.float frags;
.float weapon;
.string weaponmodel;
.float weaponframe;

.float health;				// HP
.float max_health;			// Max HP
.float playerclass;			// 0 (none), 1-6
.float next_playerclass;	// 0 (none), 1-6
.float has_portals;			// 1 if user has portals expansion
.float bluemana;			// Blue mana
.float greenmana;			// Green mana
.float max_mana;			// Maximum amount of mana for current class / level
.float armor_amulet;		// Health of amulet armor
.float armor_bracer;		// Health of bracer armor
.float armor_breastplate;	// Health of breastplate armor
.float armor_helmet;		// Health of helmet armor
.float level;				// Player level
.float intelligence;		// Player INT
.float wisdom;				// Player WIS
.float dexterity;			// Player DEX
.float strength;			// Player STR
.float experience;			// Accumulated experience points

.float ring_flight;			// Health of rings 0 - 100
.float ring_water;			// 
.float ring_turning;		//
.float ring_regeneration;	//

.float haste_time;			// When hast is depleted
.float tome_time;			// When tome of power is depleted
.string puzzle_inv1;		// Puzzle piece inventory...
.string puzzle_inv2;
.string puzzle_inv3;
.string puzzle_inv4;
.string puzzle_inv5;
.string puzzle_inv6;
.string puzzle_inv7;
.string puzzle_inv8;

// Experience this entity is worth when killed or used.
.float experience_value;

// Bit flags.
.float items;

.float takedamage;
.entity chain;
.float deadflag;

// Add to origin to get eye point.
.vector view_ofs;

// Fire.
.float button0;

// Use.
.float button1;

// Jump.
.float button2;

// Weapon changes, misc.
.float impulse;

.float fixangle;

// View / targeting angle for players.
.vector v_angle;

// Calculated pitch angle for slopes.
.float idealpitch;

.float idealroll;
.float hoverz;

.string	netname;

.entity enemy;

.float flags;
.float flags2;
.float artifact_flags;

.float colormap;
.float team;
.float light_level;

.float wpn_sound;

.float	targAng;
.float	targPitch;
.float	targDist;

// Don't back up.
.float teleport_time;

// Save this fraction of incoming damage.
.float armortype;

.float armorvalue;

// 0 = not in, 1 = feet, 2 = waist, 3 = eyes.
.float waterlevel;

// A contents value.
.float watertype;

// 0 = not in a friction entity, else the friction of the entity.
.float friction;

.float	ideal_yaw;
.float	yaw_speed;

//rj.entity	aiment;

// A pathentity or an enemy. also used by axeblade for it's tail
.entity goalentity;

.float spawnflags;

// The target of this entity.
.string	target;

.string	targetname;

// Damage is accumulated through a frame and sent as one single
// message, so the super shotgun doesn't generate huge messages.
.float dmg_take;
.float dmg_save;
.entity dmg_inflictor;

// Who launched a missile.
.entity	owner;

// Mostly or doors, but also used for waterjump.
.vector	movedir;

// Trigger messages.
.float	message;

// Either a CD track number or a sound number.
.float soundtype;

// Contains names of .WAVs to play.
.string	noise, noise1, noise2, noise3;

.float rings;				// Which rings hero has
.float rings_active;		// Shows which rings have been activated
.float rings_low;			// Shows which rings are low on power

.float artifacts;			// Which artifact hero has
.float artifact_active;		// Shows which artifact have been activated
.float artifact_low;		// Shows which artifact is running out

.float hasted;				// % of normal speed player has been hasted

.float inventory;			// Which item is currently chosen?
//rj.float ordr_cnt;			// Number of items in order

// make sure you change references to:
//    max_ammo2()   DropBackpack()   BackpackTouch()
// when adding or changing inventory fields
.float cnt_torch;			// Count of inventory item - Torch
.float cnt_h_boost;			// Count of inventory item - Health Boost
.float cnt_sh_boost;		// Count of inventory item - Super Health Boost
.float cnt_mana_boost;		// Count of inventory item - Mana Boost
.float cnt_teleport;		// Count of inventory item - Teleport
.float cnt_tome;			// Count of inventory item - Tome of Power
.float cnt_summon;			// Count of inventory item - Summon
.float cnt_invisibility;	// Count of inventory item - Invisibility
.float cnt_glyph;			// Count of inventory item - Glyph of the Ancients
.float cnt_haste;			// Count of inventory item - Haste
.float cnt_blast;			// Count of inventory item - Blast Radius
.float cnt_polymorph;		// Count of inventory item - Polymorph
.float cnt_flight;			// Count of inventory item - Flight
.float cnt_cubeofforce;		// Count of inventory item - Cube of Force
.float cnt_invincibility;	// Count of inventory item - Invincibility

.entity cameramode;

.entity movechain;
.void() chainmoved;

.float string_index;		// Index used for global string table
.float gravity;			//Gravity, duh

.float siege_team;		//ST_ATTACKER or ST_DEFENDER
// END SYSTEM FIELDS -------------------------------------------------------

// Flag the compiler.
void end_sys_fields;

// World fields
.string	wad;
.string map;
.float worldtype; // 0=medieval 1=metal 2=base

.string killtarget;

// QuakeEd fields
.float light_lev; // Not used by game, but parsed by light util
.float style;

// Monster AI, doubled over for player
.void() th_stand;
.void() th_walk;	//player_crouch_move
.void() th_run;
.void() th_missile;	//player_attack
.void() th_melee;	
.void(entity attacker, float damage) th_pain;
.void() th_die;
.void() th_save; // In case you need to save/restore a thinking state

// Mad at this player before taking damage.
.entity oldenemy;

.float speed;
.float lefty;
.float search_time;
.float attack_state;

// Monster AI stuff
.float monster_stage;
.float monster_duration; // Stage duration
.float monster_awake;
.float monster_check;
.vector monster_last_seen;


// because of how physics works, certain calls to the touch
// function of other entities involving the player do not
// allow you to adjust the velocity, so you have to do it
// outside of the inner physics stuff
.vector	adjust_velocity;

.union
{ // Entity type specific stuff
	struct // player stuff
	{		
		float splash_time;	    // When to generate the next splash
		float camera_time;      //
		float weaponframe_cnt;  //
		float attack_cnt;       // Shows which attack animation can be used
		float ring_regen_time;	// When to add the next point of health
		float ring_flight_time;	// When to update ring of flight health
		float ring_water_time;	// When to update ring of waterbreathing health
		float ring_turning_time;// When to update ring of turning health
		float super_damage;		// Player does this much more damage (Like Crusader with Special Ability #2)
		float super_damage_low; // Flag the super damage is low
		float puzzles_cheat;	// Allows player past puzzle triggers
		float camptime;			// Amount of time player has been motionless
		float crouch_time;		// Next time player should run crouch subroutine
		float crouch_stuck;		// If set this means the player has released the crouch key in an area too small to uncrouch in
		float divine_time;		// Amount of time flash happens in divine intervention
		float act_state;		// Anim info
		float raven_cnt;		// Number of raven's this guys has in the world
		float newclass;			// If doing a quick class change
		float poweredFlags;		// Which weapons are available for being powered up in tomeMode 2
		float last_use_time;	// when i last performed operation (inv. use, sheepify, suicide) that shouldn't rapid-fire.
		float jail_time;
	};
	struct
	{ // Fallen Angel
		float fangel_SaveFrame;
		float fangel_Count;
		float shoot_cnt;
		float shoot_time;	//	Time of last shot
		float z_movement;
		float z_duration;
		float drop_time;
	};
	struct
	{ // Fallen Angel's Spell
		float spell_angle;
	};
	struct
	{ // Hydra
		float hydra_FloatTo;
		float hydra_chargeTime;
	};
	struct
	{ // Spider
		float spiderType;			// SPIDER_? types
		float spiderActiveCount;	// Tallies "activity"
		float spiderGoPause;		// Active/pause threshold
		float spiderPauseLength;	// Pause duration in frames
		float spiderPauseCount;		// Tallies paused frames
	};
	struct
	{ // Scorpion
		float scorpionType;			// SCORPION_? types
		float scorpionRest;			// Resting state counter
		float scorpionWalkCount;	// Counts walking frames
	};
	struct
	{ // Golem
		float golemSlideCounter;
		float golemBeamDelay;
		float golemBeamOff1;
		float golemBeamOff2;
	};
	struct
	{ // Imp
		float impType;				// IMP_? types
	};
	struct
	{ // Mummy
		float parts_gone;
		float mummy_state;
		float mummy_state_time;
	};
	struct
	{ // Artifacts
		float artifact_respawn;		// Should respawn?
		float artifact_ignore_owner_time;
		float artifact_ignore_time;
		float artifact_name;
	};
	struct
	{ // Rider path
		float next_path_1;
		float next_path_2;
		float next_path_3;
		float next_path_4;
		float path_id;
		float next_path_5;
		float next_path_6;
	};
	struct
	{ // Rider triggers
		float rt_chance;
	};
	struct
	{ // Rider data
		float rider_gallop_mode;
		float rider_last_y_change;
		float rider_y_change;
		float rider_death_speed;
		float rider_path_distance;
		float rider_move_adjustment;
	};
	struct
	{ // War rider axe
		float waraxe_offset;
		float waraxe_horizontal;
		float waraxe_track_inc;
		float waraxe_track_limit;
		float waraxe_max_speed;
		float waraxe_max_height;
	};
	struct
	{ // War rider's quake
		float wrq_effect_id;
		float wrq_radius;
		float wrq_count;
	};
	struct
	{ // Rider's beam
		float beam_angle_a;
		float beam_angle_b;
		float beam_max_scale;
		float beam_direction;
		float beam_speed;
	};
	struct	
	{	// Used by smoke generator
		float z_modifier;	
	};
	struct
	{
		float last_health; // Used by bell entity
	};
	struct	// For raven staff Ravens
	{
		float idealpitch;
		float pitchdowntime;
		float searchtime;	// Amount of time bird has been searching
		float next_action;	// Next time to take action
		float searchtime;	// When search was first started
		float damage_max; // Amount of damage each raven can do before it has to leave
		float raven_effect_id; //effect id number
		entity raven_owner;	//owner field can get modified by refections
		vector last_vel;	// last updated velocity, tells when change needed
	};
	struct
	{	// fish
		float fish_speed;
		float fish_leader_count;
	};

	struct
	{	// Used by particle explosion entity.
		float exploderadius;
	};

	struct
	{	// Skull missiles from skullwizard
		float scream_time;
	};
	struct
	{
		float attack_cnt;
	};
	struct  
	{  // Pestalance's Hive
		float beginframe;
	};
	struct
	{   // Soul spheres
		float sound_time;
	};
	struct
	{	// Cube of force
		float shot_cnt;   // Number of shots the force cube has shot
	};
	struct
	{	// xbow bolts
		entity firstbolt; // maintain list of bolts that use same effect, so it can be removed when all bolts are gone
		entity nextbolt;

		float boltnum; // when i tell effect to change, it has to know which bolt changes
		float xbo_effect_id;

		vector xbo_startpos;//save where bolt is fired from; since bolts don't accelerate,
							//can just send the distance they travelled instead of ending position

		float fusetime;//determine how long tomed bolts wait til exploding when they are shot

		float xbo_teleported;//bolt has just been teleported.
	};
};

// Once we can do unions above end_sys, have this with the field 'playerclass'
.float monsterclass;

// FIXME: Remove the ring of spell turning and all references to this
.float turn_time;

// Triggers / doors
.string puzzle_piece_1;
.string puzzle_piece_2;
.string puzzle_piece_3;
.string puzzle_piece_4;
.float no_puzzle_msg;

// Puzzle Item
.string puzzle_id;

// More rider stuff that can't be in the union
.entity path_current;

.vector	oldangles;
.string lastweapon;			// Weapon model player had before changing to camera mode

.float lifetime;
.float lifespan;

.float walkframe;
.float wfs;				// Weapon frame state

.float attack_finished;
.float pain_finished;

.float invisible_finished;

.float invincible_time, invincible_sound;
.float invisible_time;
.float super_damage_time;

// Set to time+0.2 whenever a client fires a weapon or takes damage.
// Used to alert monsters that otherwise would let the player go.
.float show_hostile;

// Player jump flag.
.float jump_flag;

// Player swimming sound flag.
.float swim_flag;

// When time > air_finished, start drowning.
.float air_finished;

// Keeps track of the number of bubbles.
.float bubble_count;

// Keeps track of how the player died.
.string deathtype;

// Object stuff.
.string mdl;
.vector mangle; // Angle at start

// Only used by secret door.
.vector oldorigin;

.float t_length, t_width;

// Things color.
.float color;

// Count of things (used by rain entity)
.float counter;

// Can these be made part of a union??
.float plaqueflg;			// 0 if not using a plaque, 1 if using a plaque
.vector plaqueangle;		// Angle player was facing when the plaque was touched


// Doors, etc.
.vector dest, dest1, dest2;
.float wait;					// Time from firing to restarting
.float delay;					// Time from activation to firing
.entity trigger_field;			// Door's trigger entity
.string noise4;

// Monsters.
.float pausetime;
.entity pathentity;

// Doors.
.float aflag;
.float dmg; // Damage done by door when hit

// Misc flag.
.float cnt;

// What type of thing is this?
.float thingtype;


// Amount of time left on torch.
.float torchtime;

// Next torch think.
.void() torchthink;

// Amount of time left on the super health.
.float healthtime;

// Subs
.void() think1;
.vector finaldest, finalangle;

// For counting triggers
.float count;

.float spawn_health;	// Set to >0 to spawn instant health

// Plats/doors/buttons
.float lip;
.float state;
.vector pos1, pos2; // Top and bottom positions
.float height;

// Sounds
//.float waitmin, waitmax;
//.float distance;
//.float volume;

.vector orgnl_mins, orgnl_maxs;	// original bounding box

.float veer;		//Amount of veer when Veer function called (included in HomeThink Function)
				//The higher the number, the more drastic the wander is.
.float homerate;//Turning rate on homing missiles, is used as the nextthink time
				//so the lower the number, the tighter thr turn radius.
				//From the SpiralThink function, a value of FALSE will
				//stop it from randomly homing while spiraling,
				//a value of TRUE will allow it to randomly Home, but
				//does not effect rate of homing since it only calls
				//it randomly.
.float mass;	//NOTE: 1 = @18.5 pounds.
				//How much they weigh- should be used in all velocity mods
				//(pushing, impact, throwing). Used as a divider, so
				//the higher the number, the higher the mass, the less
				//distance it will go.  Make sure it's >0
				//Buttons and pressure plates can use this too so that
				//if a light object is placed on it, it won't activate
				//(but it should be cumulative so that you can stack several
				//light objects to increase mass and activate it)
				//Also, a light player (featherfall?) could get around
				//certain traps?
.float onfire;	//A value that, when FALSE means the object is not on
				//fire.  A greater than zero value indicates how fast
				//the thing is burning.  The higher the number, the higher
				//the damage and the more flames.

.vector o_angle;//Just to remember an old angle or vector

//Player
.float bloodloss;//For the Bleed() function which will remove health
				//and add graphic.  Set to 666 for beheading death.
.float oldweapon;//For remembering your last weapon, has many uses

//Monsters (and some projectiles)
.entity controller;	//What is the owner of this thing, this allows
					//it to touch it's owner if you set the owner
					//to self.

.float init_modelindex;//initial model index, so you can switch away and back
.string init_model;

//Player Only th_***
.void() th_swim;
.void() th_jump;
.void() th_fly;
.void() th_die1;
.void() th_die2;
.void() th_goredeath;

.void() th_possum;	//Monster playing dead
.void() th_possum_up;	//Monster getting up from playing dead

.float last_attack;	//Used for weapons that go into rest mode after
					//a while
.entity shield;
.float frozen;		//Can't be a flag, is a counter
.float oldskin;
.void() oldthink;
.void() th_weapon;
.float decap;		//To know if was beheaded, not a flag, set to 2 if
					//head should explode
.string headmodel;
.void() oldtouch;	//These two are for when you're frozen and thaw out
.float oldmovetype;
.float target_scale;
.float scalerate;
.float blizzcount;
.float tripwire_cnt;
.float imp_count;
.vector proj_ofs;	//Projectile offset, different from view_ofs.

.string spawnername;	//for monster spawner

.entity catapulter;
.float catapult_time;
.float last_onground;	//Timer- helps keep track of how long something has been in the air.
.vector pos_ofs;		//Position ofset
.vector angle_ofs;		//Angle offset
.float safe_time;		//How long after a tornado throws you that it cant pick you up again
.float absorb_time;		//for 0.3 seconds after crouching, you will absorb 1/2 of your falling damage upon impact
.float mintel;			//Monster intelligence- temp since entity.hc was checked out
.vector wallspot;		//Last place enemy was seen- for waypoint ai
.vector lastwaypointspot;//explains itself
.entity lockentity;		//for waypoint system
.float last_impact;		//Last time touch function was called

.float inactive;
.float msg2;
.string msg3;
.string nexttarget;		//For target transferral
.float upside_down;
.float lightvalue1;
.float lightvalue2;
.float fadespeed;
.float point_seq;		//Waypoint sequence number
.float sheep_time;		//How long you will be a sheep for
.float sheep_sound_time;
.float still_time;		//How long she's been standing still
.float visibility_offset;	//How hard it is to see and aim at entity, from 0 to 1
							//0 is totally visible, 1 is invisible
.float check_ok;			//For trigger check, instead of re-using aflag
.entity check_chain;		//for trigger_check, keeps track of it's targetted entities

.void() th_spawn;			//Monster function you spawned with
.float freeze_time;
.float level_frags;
.float visibility;

entity	sight_entity;	//So monsters wake up other monsters
.entity viewentity;
.float sv_flags;		//temp serverflags fix

.float	dmgtime;
.float	healamount, healtype;
.float anglespeed;
.float angletime;
.float movetime;
.float hit_z;
.float torncount;
.entity path_last;
.float dflags;
.float	gameFlags;
.entity targetPlayer;

//MISSION PACK
.float fire_damage;
.float standard_grav;
.float init_exp_val;
.entity credit_enemy;
.string close_target;

//SIEGE
.float cnt_grenades;
.float cnt_arrows;			
.float last_time;
.float beast_time;
.float climbing;
.vector climbspot;
.float last_climb;
.float fov_val;
.float zoom_time;
.float fail_chance;
.void() th_init;
.vector init_org;
.string ondeath_target;
.string pain_target;
.float last_up;

