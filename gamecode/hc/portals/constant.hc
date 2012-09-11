/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/portals/constant.hc,v 1.2 2005-09-19 19:50:10 sezero Exp $
 */

//
// constants
//

float FALSE					= 0;
float TRUE					= 1;

float HX_FRAME_TIME			= 0.05;
//float HX_FPS				= 20;

// edict.flags
float FL_FLY					= 1;
float FL_SWIM					= 2;
float FL_PUSH					= 4;		// Object is pushable
float FL_CLIENT					= 8;		// set for all client edicts
float FL_INWATER				= 16;		// for enter / leave water splash
float FL_MONSTER				= 32;
float FL_GODMODE				= 64;		// player cheat
float FL_NOTARGET				= 128;		// player cheat
float FL_ITEM					= 256;		// extra wide size for bonus items
float FL_ONGROUND				= 512;		// standing on something
float FL_PARTIALGROUND			= 1024;		// not all corners are valid
float FL_WATERJUMP				= 2048;		// player jumping out of water
float FL_JUMPRELEASED			= 4096;		// for jump debouncing
float FL_FLASHLIGHT				= 8192;		// quake 2 thingy
float FL_ARTIFACTUSED			= 16384;	// an artifact was just used
float FL_MOVECHAIN_ANGLE		= 32768;    // when in a move chain, will update the angle
float FL_HUNTFACE				= 65536;	//Makes monster go for enemy view_ofs thwn moving
float FL_NOZ					= 131072;	//Monster will not automove on Z if flying or swimming
float FL_SET_TRACE				= 262144;
float FL_MISMATCHEDBOUNDS		= 524288;
float FL_ARCHIVE_OVERRIDE		= 1048576;	// quake 2 thingy
float FL_CLASS_DEPENDENT		= 2097152;  // model will appear different to each player
float FL_SPECIAL_ABILITY1		= 4194304;  // has 1st special ability
float FL_SPECIAL_ABILITY2		= 8388608;  // has 2nd special ability

//edict.flags2
//FIXME: Shielded and small may be able to be determined by
//other means...
float FL2_ADJUST_MON_DAM		= 1;		//Do more damage to monsters
float FL_NODAMAGE				= 2;		//Special flag put on a missle to make it not do damage- used only by mezzoman
float FL_SMALL					= 4;		//Small enough to be crsuhed underfoot
float FL_ALIVE					= 8;		//Dead or alive.
float FL_FAKE_WATER				= 16;		//Fake water
float FL_SUMMONED				= 32;		//Summoned monster, stops it from precaching
float FL_LEDGEHOLD				= 64;		//Can realistically pull yourself up over ledges, etc.
float FL2_FADE_UP				= 128;		//Succ.
float FL2_RESPAWN				= 256;		//Monster that respawns
float FL_TORNATO_SAFE			= 512;
float FL2_DEADMEAT				= 1024;		//Tagged for death
float FL_CHAINED				= 2048;		//Held by chains
float FL2_CROUCHED				= 4096;
float FL2_CROUCH_TOGGLE			= 8192;
float FL2_FIRERESIST			= 16384;	// resistant to fire and heat and lava
float FL2_FIREHEAL				= 32768;	// healed by fire, heat, and lava
float FL2_COLDHEAL				= 65536;	// healed by freezing
float FL2_TEST_TRACE			= 131072;	// healed by freezing
float FL2_POISONED				= 262144;	// healed by freezing
float FL2_ONFIRE				= 4194304;  // on fire

// edict.drawflags
float MLS_MASKIN			= 7;	// MLS: Model Light Style
float MLS_MASKOUT			= 248;
float MLS_NONE				= 0;
float MLS_FULLBRIGHT		= 1;
float MLS_POWERMODE			= 2;
float MLS_TORCH				= 3;
float MLS_FIREFLICKER		= 4;
float MLS_CRYSTALGOLEM		= 5;
float MLS_ABSLIGHT			= 7;
float SCALE_TYPE_MASKIN		= 24;
float SCALE_TYPE_MASKOUT	= 231;
float SCALE_TYPE_UNIFORM	= 0;	// Scale X, Y, and Z
float SCALE_TYPE_XYONLY		= 8;	// Scale X and Y
float SCALE_TYPE_ZONLY		= 16;	// Scale Z
float SCALE_ORIGIN_MASKIN	= 96;
float SCALE_ORIGIN_MASKOUT	= 159;
float SCALE_ORIGIN_CENTER	= 0;	// Scaling origin at object center
float SCALE_ORIGIN_BOTTOM	= 32;	// Scaling origin at object bottom
float SCALE_ORIGIN_TOP		= 64;	// Scaling origin at object top
float DRF_TRANSLUCENT		= 128;

// Artifact Flags
float AFL_CUBE_RIGHT			= 1;
float AFL_CUBE_LEFT				= 2;
float AFL_TORCH					= 4;
float AFL_SUPERHEALTH			= 8;

// edict.movetype values
float	MOVETYPE_NONE				=  0;		// never moves
//float	MOVETYPE_ANGLENOCLIP		=  1;
//float	MOVETYPE_ANGLECLIP			=  2;
float	MOVETYPE_WALK				=  3;		// players only
float	MOVETYPE_STEP				=  4;		// discrete, not real time unless fall
float	MOVETYPE_FLY				=  5;
float	MOVETYPE_TOSS				=  6;		// gravity
float	MOVETYPE_PUSH				=  7;		// no clip to world, push and crush
float	MOVETYPE_NOCLIP				=  8;
float	MOVETYPE_FLYMISSILE			=  9;		// fly with extra size against monsters
float	MOVETYPE_BOUNCE				= 10;
float	MOVETYPE_BOUNCEMISSILE		= 11;		// bounce with extra size and no gravity
float	MOVETYPE_PUSHPULL			= 13;		// pushable/pullable object
float	MOVETYPE_SWIM				= 14;		// object won't move out of water

// particle types
//float PARTICLETYPE_STATIC		= 0;
float PARTICLETYPE_GRAV			= 1;
float PARTICLETYPE_FASTGRAV		= 2;
float PARTICLETYPE_SLOWGRAV		= 3;
//float PARTICLETYPE_FIRE			= 4;
//float PARTICLETYPE_EXPLODE		= 5;
float PARTICLETYPE_EXPLODE2		= 6;
//float PARTICLETYPE_BLOB			= 7;
//float PARTICLETYPE_BLOB2		= 8;
//float PARTICLETYPE_RAIN			= 9;
float PARTICLETYPE_C_EXPLODE	= 10;
//float PARTICLETYPE_C_EXPLODE2	= 11;
//float PARTICLETYPE_SPIT			= 12;
float PARTICLETYPE_FIREBALL		= 13;
//float PARTICLETYPE_ICE			= 14;
float PARTICLETYPE_SPELL		= 15;
//float PARTICLETYPE_TEST			= 16;
//float PARTICLETYPE_QUAKE		= 17;
//float PARTICLETYPE_RIDERDEATH	= 18;
//float PARTICLETYPE_VORPAL		= 19;
//float PARTICLETYPE_SETSTAFF		= 20;
//float PARTICLETYPE_MAGICMISSILE	= 21;
//float PARTICLETYPE_BONESHARD	= 22;
//float PARTICLETYPE_SCARAB		= 23;
//float PARTICLETYPE_ACIDBALL		= 24;
float PARTICLETYPE_DARKEN		= 25;	//Particle will darken to darkest color of that shade, valid only for colors <= 232
float PARTICLETYPE_REDFIRE		= 28;	//Particle will darken to darkest color of that shade, valid only for colors <= 232

// Hexen hull constants
float HULL_IMPLICIT			= 0;	//Choose the hull based on bounding box- like in Quake
float HULL_POINT			= 1;	//0 0 0, 0 0 0
float HULL_PLAYER			= 2;	//'-16 -16 0', '16 16 56'
float HULL_SCORPION			= 3;	//'-24 -24 -20', '24 24 20'
float HULL_CROUCH			= 4;	//'-16 -16 0', '16 16 28'
//Next 2 clip though world?
float HULL_HYDRA			= 5;	//'-40 -40 -42', '40 40 42' - replace me in MP with a '-8 -8 -8' '8 8 8' hull for pentacles
float HULL_GOLEM			= 6;	//'-48 -48 -50', '48 48 50' - maybe change to '-23 -23 -40', '23 23 40' for yakman

// edict.solid values
float	SOLID_NOT					= 0;		// no interaction with other objects
float	SOLID_TRIGGER				= 1;		// touch on edge, but not blocking
float	SOLID_BBOX					= 2;		// touch on edge, block
float	SOLID_SLIDEBOX				= 3;		// touch on edge, but not an onground
float	SOLID_BSP					= 4;		// bsp clip, touch on edge, block
float	SOLID_PHASE					= 5;		// will interact with all objects except entities with FL_MONSTER & FL_CLIENT - those it will pass through

// range values
float	RANGE_MELEE					= 0;
float	RANGE_NEAR					= 1;
float	RANGE_MID					= 2;
float	RANGE_FAR					= 3;

// deadflag values

float	DEAD_NO						= 0;
float	DEAD_DYING					= 1;
float	DEAD_DEAD					= 2;
float	DEAD_RESPAWNABLE			= 3;

// takedamage values

float	DAMAGE_NO					= 0;	// Entity cannot be hurt
float	DAMAGE_YES					= 1;	// Can be hurt 
float	DAMAGE_NO_GRENADE			= 2;	// Will not trigger a grenade to explode


// use inventory flags to show which item is the current item
float INV_NONE 						= 0;
float INV_TORCH						= 1;
float INV_HP_BOOST					= 2;
float INV_SUPER_HP_BOOST			= 3;
float INV_MANA_BOOST				= 4;
float INV_TELEPORT					= 5;
float INV_TOME						= 6;
float INV_SUMMON					= 7;
float INV_INVISIBILITY				= 8;
float INV_GLYPH						= 9;
float INV_HASTE						= 10;
float INV_BLAST						= 11;
float INV_POLYMORPH					= 12;
float INV_FLIGHT					= 13;
float INV_CUBEOFFORCE				= 14;
float INV_INVINCIBILITY				= 15;

float ARTIFACT_TORCH					= 1;
float ARTIFACT_HP_BOOST					= 2;
float ARTIFACT_SUPER_HP_BOOST			= 3;
float ARTIFACT_MANA_BOOST				= 4;
float ARTIFACT_TELEPORT					= 5;
float ARTIFACT_TOME						= 6;
float ARTIFACT_SUMMON					= 7;
float ARTIFACT_INVISIBILITY				= 8;
float ARTIFACT_GLYPH					= 9;
float ARTIFACT_HASTE					= 10;
float ARTIFACT_BLAST					= 11;
float ARTIFACT_POLYMORPH				= 12;
float ARTIFACT_FLIGHT					= 13;
float ARTIFACT_CUBEOFFORCE				= 14;
float ARTIFACT_INVINCIBILITY			= 15;


// Use ring flags to show which rings hero carries
float RING_NONE						= 0;
float RING_FLIGHT					= 1;
float RING_WATER					= 2;
float RING_REGENERATION				= 4;
float RING_TURNING					= 8;


// Use artifact flags to show which artifacts are in use
float ART_NONE						= 0;
float ART_HASTE						= 1;
float ART_INVINCIBILITY				= 2;
float ART_TOMEOFPOWER  				= 4;
float ART_INVISIBILITY				= 8;
float ARTFLAG_FROZEN				= 128;
float ARTFLAG_STONED				= 256;
float ARTFLAG_DIVINE_INTERVENTION	= 512;


// Gobal skin textures
float GLOBAL_SKIN_STONE				= 100;
float GLOBAL_SKIN_ICE				= 101;


// Player Classes
float CLASS_NONE					= 0;
float CLASS_PALADIN					= 1;
float CLASS_CRUSADER				= 2;
float CLASS_NECROMANCER				= 3;
float CLASS_ASSASSIN				= 4;
float CLASS_SUCCUBUS				= 5;


// Monster Classes
float CLASS_GRUNT   				= 1;
float CLASS_HENCHMAN   				= 2;
float CLASS_LEADER   				= 3;
float CLASS_BOSS     				= 4;
float CLASS_FINAL_BOSS				= 5;


float MAX_HEALTH					= 200;

// Player Mode
float	MODE_NORMAL					= 0;		// normal play mode
float	MODE_CAMERA  				= 1;		// player is a camera right now

float AS_STRAIGHT	= 1;
float AS_SLIDING	= 2;
float AS_MELEE		= 3;
float AS_MISSILE	= 4;
float AS_WAIT		= 5;
float AS_FERRY		= 6;

// Generic Weapon Names
float IT_WEAPON1					= 4096;
float IT_WEAPON2					= 1;
float IT_WEAPON3					= 2;
float IT_WEAPON4					= 4;
//float IT_TESTWEAP					= 8;
float IT_WEAPON4_1					= 16;		// First half of weapon
float IT_WEAPON4_2					= 32;		// Second half of weapon


// paladin weapons
//float IT_GAUNTLETS           = 4096;


// items
/*
float	IT_AXE						= 4096;
float	IT_SHOTGUN					= 1;
float	IT_SUPER_SHOTGUN			= 2;
float	IT_NAILGUN					= 4;
float	IT_SUPER_NAILGUN			= 8;
float	IT_GRENADE_LAUNCHER			= 16;
float	IT_ROCKET_LAUNCHER			= 32;
float	IT_LIGHTNING				= 64;
float	IT_EXTRA_WEAPON				= 128;
*/

//float	IT_ARMOR1					= 8192;
//float	IT_ARMOR2					= 16384;
//float	IT_ARMOR3					= 32768;
//float	IT_SUPERHEALTH				= 65536;


float	IT_INVISIBILITY			= 524288;
//float	IT_INVULNERABILITY		= 1048576;
//float	IT_SUIT						= 2097152;
//float	IT_QUAD						= 4194304;

// rings - amount of time they work
//float FLIGHT_TIME					= 30;
//float WATER_TIME					= 30;
//float ABSORPTION_TIME				= 30;
//float REGEN_TIME					= 30;
//float TURNING_TIME					= 30;

// artifacts - amount of time they work
//float HASTE_TIME				= 15;
float TOME_TIME					= 30;

float RESPAWN_TIME				= 30;

// weapon damage values
float WEAPON1_BASE_DAMAGE			= 12;
float WEAPON1_ADD_DAMAGE			= 12;
float WEAPON1_PWR_BASE_DAMAGE		= 30;
float WEAPON1_PWR_ADD_DAMAGE		= 20;
float WEAPON1_PUSH					= 5;


// glyph of the ancients
float GLYPH_BASE_DAMAGE			= 100;
float GLYPH_ADD_DAMAGE			= 20;

// Modifier for HASTE
//float HASTE_MOD				= 2;
float BLAST_RADIUS				= 200;
float BLASTDAMAGE				= 2; 

// Damage values for attacks from monsters
//float DMG_ARCHER_PUNCH			= 4;
float DMG_MUMMY_PUNCH			= 8;
//float DMG_MUMMY_BITE 			= 2;


//Thing Types
float THINGTYPE_GREYSTONE		= 1;
float THINGTYPE_WOOD			= 2;
float THINGTYPE_METAL			= 3;
float THINGTYPE_FLESH			= 4;
float THINGTYPE_FIRE			= 5;
float THINGTYPE_CLAY			= 6;
float THINGTYPE_LEAVES			= 7;
float THINGTYPE_HAY				= 8;
float THINGTYPE_BROWNSTONE		= 9;
float THINGTYPE_CLOTH			= 10;
float THINGTYPE_WOOD_LEAF		= 11;
float THINGTYPE_WOOD_METAL		= 12;
float THINGTYPE_WOOD_STONE		= 13;
float THINGTYPE_METAL_STONE		= 14;
float THINGTYPE_METAL_CLOTH 	= 15;
float THINGTYPE_WEBS		 	= 16;
float THINGTYPE_GLASS 			= 17;
float THINGTYPE_ICE 			= 18;
float THINGTYPE_CLEARGLASS 		= 19;
float THINGTYPE_REDGLASS 		= 20;
float THINGTYPE_ACID	 		= 21;
float THINGTYPE_METEOR	 		= 22;
float THINGTYPE_GREENFLESH 		= 23;
float THINGTYPE_BONE	 		= 24;


// point content values
float	CONTENT_EMPTY				= -1;
float	CONTENT_SOLID				= -2;
float	CONTENT_WATER				= -3;
float	CONTENT_SLIME				= -4;
float	CONTENT_LAVA				= -5;
float	CONTENT_SKY					= -6;

float	STATE_TOP					= 0;
float	STATE_BOTTOM				= 1;
float	STATE_UP					= 2;
float	STATE_DOWN					= 3;
float	STATE_MOVING				= 4;

vector	VEC_ORIGIN				= '0 0 0';
//vector	VEC_HULL_MIN			= '-16 -16 -24';
//vector	VEC_HULL_MAX			= '16 16 32';

//vector	VEC_HULL2_MIN			= '-32 -32 -24';
//vector	VEC_HULL2_MAX			= '32 32 64';

// protocol bytes
float SVC_SETVIEWPORT = 5;			// Net.Protocol 0x05- for camera
float SVC_SETVIEWANGLES = 10;		// Net.Protocol 0x0A- for camera
float SVC_SETANGLESINTER = 50;		// Interpolating camera angles

float	SVC_TEMPENTITY				= 23;
float	SVC_KILLEDMONSTER			= 27;
float	SVC_FOUNDSECRET				= 28;
float	SVC_INTERMISSION			= 30;
float	SVC_FINALE					= 31;
float	SVC_CDTRACK					= 32;
float	SVC_SELLSCREEN				= 33;
float	SVC_SET_VIEW_FLAGS			= 40;
float	SVC_CLEAR_VIEW_FLAGS		= 41;
float	SVC_SET_VIEW_TINT			= 46;
float	SVC_UPDATE_KINGOFHILL		= 51;

// Client Effects
float	CE_RAIN						= 1;
float	CE_FOUNTAIN					= 2;
float	CE_QUAKE					= 3;
float	CE_WHITE_SMOKE				= 4;
float	CE_BLUESPARK				= 5;
float	CE_YELLOWSPARK				= 6;
float	CE_SM_CIRCLE_EXP			= 7;
float	CE_BG_CIRCLE_EXP			= 8;
float	CE_SM_WHITE_FLASH			= 9;
float	CE_WHITE_FLASH				= 10;
float	CE_YELLOWRED_FLASH			= 11;
float	CE_BLUE_FLASH				= 12;
float	CE_SM_BLUE_FLASH			= 13;
float	CE_RED_FLASH				= 14;
float	CE_SM_EXPLOSION				= 15;
float	CE_LG_EXPLOSION				= 16;
float	CE_FLOOR_EXPLOSION			= 17;
float   CE_RIDER_DEATH				= 18;
float	CE_BLUE_EXPLOSION 			= 19;
float	CE_GREEN_SMOKE    			= 20;
float	CE_GREY_SMOKE    			= 21;
float	CE_RED_SMOKE    			= 22;
float	CE_SLOW_WHITE_SMOKE			= 23;
float   CE_REDSPARK					= 24;
float   CE_GREENSPARK				= 25;
float   CE_TELESMK1					= 26;
float   CE_TELESMK2					= 27;
float   CE_ICE_HIT					= 28;//	icehit.spr	0-5
float   CE_MEDUSA_HIT				= 29;//	medhit.spr	0-6
float   CE_MEZZO_REFLECT			= 30;//	mezzoref.spr	0-5
float   CE_FLOOR_EXPLOSION2			= 31;//	flrexpl2.spr	0-19
float   CE_XBOW_EXPLOSION			= 32;//	xbowexpl.spr	0-16
float   CE_NEW_EXPLOSION			= 33;//	gen_expl.spr	0-13
float   CE_MAGIC_MISSILE_EXPLOSION	= 34;//	mm_expld.spr
float   CE_GHOST					= 35;//	ghost.spr- translucent
float   CE_BONE_EXPLOSION			= 36;//	bonexpld.spr
float   CE_REDCLOUD					= 37;//	rcloud.spr
float   CE_TELEPORTERPUFFS			= 38;
float   CE_TELEPORTERBODY			= 39;
float	CE_BONESHARD				= 40;
float	CE_BONESHRAPNEL				= 41;
float	CE_FLAMESTREAM				= 42;
float	CE_SNOW						= 43;
float	CE_GRAVITYWELL				= 44;
float	CE_BLDRN_EXPL				= 45;
float	CE_ACID_MUZZFL				= 46;
float	CE_ACID_HIT					= 47;
float	CE_FIREWALL_SMALL			= 48;
float	CE_FIREWALL_MEDIUM			= 49;
float	CE_FIREWALL_LARGE			= 50;
float	CE_LBALL_EXPL				= 51;
float	CE_ACID_SPLAT				= 52;
float	CE_ACID_EXPL				= 53;
float	CE_FBOOM					= 54;
float	CE_CHUNK					= 55;
float	CE_BOMB						= 56;
float	CE_BRN_BOUNCE				= 57;
float	CE_LSHOCK					= 58;
float	CE_FLAMEWALL				= 59;
float	CE_FLAMEWALL2				= 60;
float	CE_FLOOR_EXPLOSION3			= 61;
float	CE_ONFIRE					= 62;

float	SFL_FLUFFY					= 1;// All largish flakes
float	SFL_MIXED					= 2;// Mixed flakes
float	SFL_HALF_BRIGHT				= 4;// All flakes start darker
float	SFL_NO_MELT					= 8;// Flakes don't melt when his surface, just go away
float	SFL_IN_BOUNDS				= 16;// Flakes cannot leave the bounds of their box
float	SFL_NO_TRANS				= 32;// All flakes start non-translucent

// Temporary entities
float	TE_SPIKE					= 0;
float	TE_SUPERSPIKE				= 1;
float	TE_GUNSHOT					= 2;
float	TE_EXPLOSION				= 3;
float	TE_TAREXPLOSION				= 4;
float	TE_LIGHTNING1				= 5;
float	TE_LIGHTNING2				= 6;
float	TE_WIZSPIKE					= 7;
float	TE_KNIGHTSPIKE				= 8;
float	TE_LIGHTNING3				= 9;
float	TE_LAVASPLASH				= 10;
float	TE_TELEPORT					= 11;
float	TE_STREAM_LIGHTNING_SMALL	= 24;
float	TE_STREAM_CHAIN				= 25;
float	TE_STREAM_SUNSTAFF1			= 26;
float	TE_STREAM_SUNSTAFF2			= 27;
float	TE_STREAM_LIGHTNING			= 28;
float	TE_STREAM_COLORBEAM			= 29;
float	TE_STREAM_ICECHUNKS			= 30;
float	TE_STREAM_GAZE				= 31;
float	TE_STREAM_FAMINE			= 32;

// Stream flags
float	STREAM_ATTACHED				= 16;
float	STREAM_TRANSLUCENT			= 32;


// sound channels
// channel 0 never willingly overrides
// other channels (1-7) always override a playing sound on that channel
float	CHAN_AUTO					= 0;
float	CHAN_WEAPON					= 1;
float	CHAN_VOICE					= 2;
float	CHAN_ITEM					= 3;
float	CHAN_BODY					= 4;

float	ATTN_NONE					= 0;
float	ATTN_NORM					= 1;
float	ATTN_IDLE					= 2;
float	ATTN_STATIC					= 3;
float	ATTN_LOOP					= 4;

// update types
//float	UPDATE_GENERAL				= 0;
//float	UPDATE_STATIC				= 1;
//float	UPDATE_BINARY				= 2;
//float	UPDATE_TEMP					= 3;

// entity effects
float	EF_BRIGHTFIELD				= 1;
float	EF_MUZZLEFLASH 				= 2;
float	EF_BRIGHTLIGHT 				= 4;
float	EF_TORCHLIGHT				= 6;
float	EF_DIMLIGHT 				= 8;
float	EF_DARKLIGHT				= 16;
float	EF_DARKFIELD				= 32;
float	EF_LIGHT					= 64;
float	EF_NODRAW					= 128;
float	EF_TEX_STOPF				= 256;
float	EF_TEX_STOPL				= 528;

// messages
float	MSG_BROADCAST				= 0;		// unreliable to all
float	MSG_ONE						= 1;		// reliable to one (msg_entity)
float	MSG_ALL						= 2;		// reliable to all
//float	MSG_INIT						= 3;		// write to the init string

//float STEP_HEIGHT					= 18;		// Max step height

// monster AI states
float AI_DECIDE						=    0;		// An action was just finished - time to decide what to do
float AI_STAND						=    1;		// Standing guard
float AI_WALK						=    2;		// Walking
float AI_CHARGE						=    4;     // Charging enemy
float AI_WANDER						=    8;     // Wandering around mindlessly
float AI_MELEE_ATTACK				=   16;     // 
float AI_MISSILE_ATTACK				=   32;     // 
float AI_MISSILE_REATTACK			=   64;		// Attacking again from attack stance (archer)
float AI_PAIN						=  128;		// Monster has only 1 type of pain
float AI_PAIN_CLOSE					=  256;		// Pain when close to enemy
float AI_PAIN_FAR					=  512;		// Pain when far from enemy
float AI_DEAD						= 1024;		// 
float AI_TURNLOOK					= 2048;		// Turning to look for enemy
float AI_DEAD_GIB					= 4096;		// Can be gibbed when killed
float AI_DEAD_TWITCH				= 8192;		// Twitches while dead

// Return values for AdvanceFrame()
float AF_NORMAL		= 0;
float AF_BEGINNING	= 1;
float AF_END		= 2;

float CHUNK_MAX		= 30;	// Max number of chunks (models) that can be alive at one time
float MAX_LEVELS = 10;


// server flags
//float	SFL_EPISODE_1		= 1;
//float	SFL_EPISODE_2		= 2;
//float	SFL_EPISODE_3		= 4;
//float	SFL_EPISODE_4		= 8;
float	SFL_NEW_UNIT		= 16;
//float	SFL_NEW_EPISODE		= 32;
// = 64;
// = 128;
float	SFL_CROSS_TRIGGER_1 = 256;
float	SFL_CROSS_TRIGGER_2	= 512;
float	SFL_CROSS_TRIGGER_3	= 1024;
float	SFL_CROSS_TRIGGER_4	= 2048;
float	SFL_CROSS_TRIGGER_5	= 4096;
float	SFL_CROSS_TRIGGER_6	= 8192;
float	SFL_CROSS_TRIGGER_7	= 16384;
float	SFL_CROSS_TRIGGER_8	= 32768;

float	SFL_CROSS_TRIGGERS	= 65280;
//float attck_cnt;

float WF_NORMAL_ADVANCE = 0;		// States when using advanceweaponframe
float WF_CYCLE_STARTED = 1;
float WF_CYCLE_WRAPPED = 2;
float WF_LAST_FRAME = 3;

float WORLDTYPE_CASTLE = 0;
float WORLDTYPE_EGYPT  = 1;
float WORLDTYPE_MESO   = 2;
float WORLDTYPE_ROMAN  = 3;

//Spawnflags for monster spawners
float IMP		= 1;
float ARCHER	= 2;
float WIZARD	= 4;
float SCORPION	= 8;
float SPIDER	= 16;
float ONDEATH	= 32;
float QUIET		= 64;
float TRIGGERONLY  = 128;
//Spawnflags for MP monster spawners
float ICE_ARCHER = 1;
float ICE_IMP	= 2;
float SNOWLEOPARD	= 4;
float WERETIGER	= 8;
float YAKMAN	= 16;

//spawnflag for all monsters
float AMBUSH = 1;
float JUMP	= 4;	    //Gives monster the ability to jump
//float PLAY_DEAD	= 8;	//Makes a monster play dead at start
float NO_DROP	= 32;	//Keeps them from dropping to the ground at spawntime
float SF_FROZEN	= 64;	//Start frozen

float SLOPE = 16;		//Trains- follow angle for vec between path_corners

//spawnflag for items, weapons, artifacts
float FLOATING	=	1;	//Keeps them from dropping to the ground at spawntime

//Spawnflags for barrels
//float BARREL_DOWNHILL		= 1;
float BARREL_NO_DROP		= 2;
float ON_SIDE				= 4;
float BARREL_SINK			= 8;		
float DROP_USE				= 16;//Barrel won't drop unless used
float BARREL_RESPAWN				= 32;//Upon death, barrel will respawn at it's initial origin
//Barrel types
//float BARREL_UNBREAKABLE	= 16;
float BARREL_NORMAL			= 32;
float BARREL_EXPLODING		= 64;
float BARREL_INDESTRUCTIBLE = 128;		

//For func_rotate
float GRADUAL			= 32;
float TOGGLE_REVERSE	= 64;
float KEEP_START		= 128;

float NO_RESPAWN		= 0;	// For the spawning of artifacts	
float RESPAWN			= 1;	

float RING_REGENERATION_MAX = 150;	// Number of health points ring gives you back
float RING_FLIGHT_MAX = 60;			// Number of seconds you can fly
float RING_WATER_MAX = 60;			// Number of seconds you can stay under water
float RING_TURNING_MAX = 60;		// Number of seconds you can turn missiles

//act_states - for player anim
float ACT_STAND			= 0;
float ACT_RUN			= 1;
float ACT_SWIM_FLY		= 2;
float ACT_ATTACK		= 3;
float ACT_PAIN			= 4;
float ACT_JUMP			= 5;
float ACT_CROUCH_STAND	= 6;
float ACT_CROUCH_MOVE	= 7;
float ACT_DEAD			= 8;
float ACT_DECAP			= 9;

float MISSIONPACK		= 1;	//Spawnflag for world, telling us it's a Mission Pack map- used so certain code is used only for new levels
float SHEEPHUNT			= 2;	//Spawnflag for world, enables special sheep hunter code...
