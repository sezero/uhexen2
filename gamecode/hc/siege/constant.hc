/*
 * $Header: /cvsroot/uhexen2/gamecode/hc/siege/constant.hc,v 1.2 2005-09-19 19:50:10 sezero Exp $
 */

//
// constants
//

float FALSE					= 0;
float TRUE					= 1;

float HX_FRAME_TIME			= 0.05;
float HX_FPS				= 20;

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
float FL_FIRERESIST				= 65536;	// resistant to fire and heat and lava
float FL_FIREHEAL				= 131072;	// healed by fire, heat, and lava
float FL_IGNORESIZEOFS			= 262144;	// Force physics vs BSP to use it's hull, ignore size
float FL_COLDHEAL				= 524288;	// healed by freezing
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
float FL2_HARDFALL				= 256;		//Hit hard- Siege
float FL_TORNATO_SAFE			= 512;
float FL2_DEADMEAT				= 1024;		//Tagged for death
float FL_CHAINED				= 2048;		//Held by chains
float FL2_CROUCHED				= 4096;
float FL2_CROUCH_TOGGLE			= 8192;
float FL2_FIRERESIST			= 16384;	// resistant to fire and heat and lava
float FL2_WALLCLIMB				= 32768;	// can climb walls
float FL2_EXCALIBUR				= 65536;	// Using excalibur
float FL2_REPLACEMENT			= 131072;	// replacement throne rrom key
float FL2_POISONED				= 262144;	// healed by freezing
float FL2_HASKEY				= 524288;
float FL2_PARRIED				= 1048576;
float FL2_ONFIRE				= 4194304;  // on fire

// edict.drawflags
float MLS_MASKIN			= 7;	// MLS: Model Light Style
float MLS_MASKOUT			= 248;
float MLS_NONE				= 0;
float MLS_FULLBRIGHT		= 1;
float MLS_POWERMODE			= 2;
float MLS_TORCH				= 3;
float MLS_FIREFLICKER		= 4;
float MLS_INVIS				= 5;
//float MLS_INVIS				= 6;
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
float PARTICLETYPE_STATIC		= 0;
float PARTICLETYPE_GRAV			= 1;
float PARTICLETYPE_FASTGRAV		= 2;
float PARTICLETYPE_SLOWGRAV		= 3;
float PARTICLETYPE_FIRE			= 4;
float PARTICLETYPE_EXPLODE		= 5;
float PARTICLETYPE_EXPLODE2		= 6;
float PARTICLETYPE_BLOB			= 7;
float PARTICLETYPE_BLOB2		= 8;
float PARTICLETYPE_RAIN			= 9;
float PARTICLETYPE_C_EXPLODE	= 10;
float PARTICLETYPE_C_EXPLODE2	= 11;
float PARTICLETYPE_SPIT			= 12;
float PARTICLETYPE_FIREBALL		= 13;
float PARTICLETYPE_ICE			= 14;
float PARTICLETYPE_SPELL		= 15;
//MISSION PACK
float PARTICLETYPE_DARKEN		= 24;	//Particle will darken to darkest color of that shade, valid only for colors <= 232
float PARTICLETYPE_REDFIRE		= 26;	//Particle will darken to darkest color of that shade, valid only for colors <= 232
float PARTICLETYPE_ACIDBALL		= 27;	//Built-in model trail
float PARTICLETYPE_BLUESTEP		= 28;	//Built-in model trail

// Hexen hull constants
float HULL_IMPLICIT			= 0;	//Choose the hull based on bounding box- like in Quake
float HULL_POINT			= 1;	//0 0 0, 0 0 0
float HULL_PLAYER			= 2;	//'-16 -16 0', '16 16 56'
float HULL_SCORPION			= 3;	//'-24 -24 -20', '24 24 20'
float HULL_CROUCH			= 4;	//'-16 -16 0', '16 16 28'
//Next 2 clip though world?
float HULL_HYDRA			= 5;	//'-28 -28 -24', '28 28 24'
float HULL_GOLEM			= 6;	//???,???

// Keep around old constants until all references are removed
float HULL_OLD				= 0;
float HULL_SMALL			= 1;
float HULL_NORMAL			= 2;
float HULL_BIG				= 3;

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
float ARTIFACT_ARROWS					= 16;
float ARTIFACT_GRENADES					= 17;
float ARTIFACT_CLIMB					= 18;


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
float ARTFLAG_FROZEN				= 16;	// 32
float ARTFLAG_STONED				= 32;	// 64
float ARTFLAG_DIVINE_INTERVENTION	= 64;	// 128
float ART_CLIMB						= 128;	// 256


// Gobal skin textures
float GLOBAL_SKIN_STONE				= 100;
float GLOBAL_SKIN_ICE				= 101;
float GLOBAL_SKIN_NOTEAM			= 102;//just black

// Player Classes
float CLASS_NONE					= 0;
float CLASS_PALADIN					= 1;
float CLASS_CRUSADER				= 2;
float CLASS_NECROMANCER				= 3;
float CLASS_ASSASSIN				= 4;
//MP
float CLASS_SUCCUBUS				= 5;
//Siege
float CLASS_DWARF					= 6;


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
float IT_TESTWEAP					= 8;
float IT_WEAPON4_1					= 16;		// First half of weapon
float IT_WEAPON4_2					= 32;		// Second half of weapon


// paladin weapons
float IT_GAUNTLETS           = 4096;



float	IT_ARMOR1					= 8192;
float	IT_ARMOR2					= 16384;
float	IT_ARMOR3					= 32768;
float	IT_SUPERHEALTH				= 65536;


float	IT_INVISIBILITY			= 524288;
float	IT_INVULNERABILITY		= 1048576;
float	IT_SUIT						= 2097152;
float	IT_QUAD						= 4194304;

// rings - amount of time they work
float FLIGHT_TIME					= 30;
float WATER_TIME					= 30;
float ABSORPTION_TIME				= 30;
float REGEN_TIME					= 30;
float TURNING_TIME					= 30;

// artifacts - amount of time they work
float HASTE_TIME				= 15;
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
float DMG_ARCHER_PUNCH			= 4;
float DMG_MUMMY_PUNCH			= 8;
float DMG_MUMMY_BITE 			= 2;


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
float THINGTYPE_DIRT			= 25;


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
vector	VEC_HULL_MIN			= '-16 -16 -24';
vector	VEC_HULL_MAX			= '16 16 32';
//Temp- because player models origins are at feet,
//Above values raise them 12 above the floor!
//But what about monsters using this Hull size??
//vector	VEC_HULL_MIN			= '-16 -16 0';
//vector	VEC_HULL_MAX			= '16 16 56';

vector	VEC_HULL2_MIN			= '-32 -32 -24';
vector	VEC_HULL2_MAX			= '32 32 64';

// protocol bytes
float	SVC_TEMPENTITY				= 23;
float	SVC_KILLEDMONSTER			= 27;
float	SVC_FOUNDSECRET				= 28;
float	SVC_INTERMISSION			= 30;
float	SVC_FINALE					= 31;
float	SVC_CDTRACK					= 32;
float	SVC_SELLSCREEN				= 33;
float	SVC_UPDATE_KINGOFHILL		= 51;//MP
float	SVC_SET_VIEW_TINT			= 53;
float	SVC_SET_VIEW_FLAGS			= 56;
float	SVC_CLEAR_VIEW_FLAGS		= 57;
float	SVC_MIDI_NAME				= 65;

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
float	CE_HWMISSILESTAR			= 42;
float	CE_HWEIDOLONSTAR			= 43;
float	CE_HWSHEEPINATOR			= 44;
float	CE_TRIPMINE					= 45;
float	CE_HWBONEBALL				= 46;
float	CE_HWRAVENSTAFF				= 47;
float	CE_TRIPMINESTILL			= 48;
float	CE_SCARABCHAIN				= 49;
float	CE_SM_EXPLOSION2			= 50;
float	CE_HWSPLITFLASH				= 51;
float	CE_HWXBOWSHOOT				= 52;
float	CE_HWRAVENPOWER				= 53;
float	CE_HWDRILLA					= 54;
float	CE_DEATHBUBBLES				= 55;
//MISSION PACK
float	CE_RIPPLE					= 56;
float	CE_BLDRN_EXPL				= 57;
float	CE_ACID_MUZZFL				= 58;
float	CE_ACID_HIT					= 59;
float	CE_FIREWALL_SMALL			= 60;
float	CE_FIREWALL_MEDIUM			= 61;
float	CE_FIREWALL_LARGE			= 62;
float	CE_LBALL_EXPL				= 63;
float	CE_ACID_SPLAT				= 64;
float	CE_ACID_EXPL				= 65;
float	CE_FBOOM					= 66;
float	CE_BOMB						= 67;
float	CE_BRN_BOUNCE				= 68;
float	CE_LSHOCK					= 69;
float	CE_FLAMEWALL				= 70;
float	CE_FLAMEWALL2				= 71;
float	CE_FLOOR_EXPLOSION3			= 72;
float	CE_ONFIRE					= 73;
float	CE_FLAMESTREAM				= 74;


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
float	TE_STREAM_CHAIN				= 25;
float	TE_STREAM_SUNSTAFF1			= 26;
float	TE_STREAM_SUNSTAFF2			= 27;
float	TE_STREAM_LIGHTNING			= 28;
float	TE_STREAM_COLORBEAM			= 29;
float	TE_STREAM_ICECHUNKS			= 30;
float	TE_STREAM_GAZE				= 31;
float	TE_STREAM_FAMINE			= 32;

float	TE_BIGGRENADE				= 33;
float	TE_CHUNK					= 34;
float	TE_HWBONEPOWER				= 35;
float	TE_HWBONEPOWER2				= 36;
float	TE_METEORHIT				= 37;
float	TE_HWRAVENDIE				= 38;
float	TE_HWRAVENEXPLODE			= 39;
float	TE_XBOWHIT					= 40;
float	TE_CHUNK2					= 41;
float	TE_ICEHIT					= 42;
float	TE_ICESTORM					= 43;
float	TE_HWMISSILEFLASH			= 44;
float	TE_SUNSTAFF_CHEAP			= 45;
float	TE_LIGHTNING_HAMMER			= 46;
float	TE_DRILLA_EXPLODE			= 47;
float	TE_DRILLA_DRILL				= 48;
float	TE_HWTELEPORT				= 49;
float	TE_SWORD_EXPLOSION			= 50;
float	TE_AXE_BOUNCE				= 51;
float	TE_AXE_EXPLODE				= 52;
float	TE_TIME_BOMB				= 53;
float	TE_FIREBALL					= 54;
float	TE_SUNSTAFF_POWER			= 55;
float	TE_PURIFY2_EXPLODE			= 56;
float	TE_PLAYER_DEATH				= 57;
float	TE_PURIFY1_EFFECT			= 58;
float	TE_TELEPORT_LINGER			= 59;
float	TE_LINE_EXPLOSION			= 60;
float	TE_METEOR_CRUSH				= 61;
//MP
float	TE_STREAM_LIGHTNING_SMALL	= 62;

float	TE_ACIDBALL					= 63;
float	TE_ACIDBLOB					= 64;
float	TE_FIREWALL					= 65;
float	TE_FIREWALL_IMPACT			= 66;
float   TE_HWBONERIC				= 67;
float	TE_POWERFLAME				= 68;
float	TE_BLOODRAIN				= 69;
float	TE_AXE						= 70;
float	TE_PURIFY2_MISSILE			= 71;
float	TE_SWORD_SHOT				= 72;
float	TE_ICESHOT					= 73;
float	TE_METEOR					= 74;
float	TE_LIGHTNINGBALL			= 75;
float	TE_MEGAMETEOR				= 76;
float	TE_CUBEBEAM					= 77;
float	TE_LIGHTNINGEXPLODE			= 78;
float	TE_ACID_BALL_FLY			= 79;
float	TE_ACID_BLOB_FLY			= 80;
float	TE_CHAINLIGHTNING			= 81;



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
float	CHAN_UPDATE					= 7;// All sounds on this channel are updated by the EF_UPDATESOUND flag
//MP
float	PHS_OVERRIDE_R				= 8;//CHAN_????+PHS_OVERRIDE_R Forces the sound channel to be reliable and not use the PHS

float	SOUND_STARTED				= 7777777;//Just used to keep track of whether I played a certain sound or not- 7777777 because t_dith is used as time+X elsewhere in code
float	SOUND_STOPPED				= 0;//clears out t_width fpr next playsound check

float	ATTN_NONE					= 0;
float	ATTN_NORM					= 1;
float	ATTN_IDLE					= 2;
float	ATTN_STATIC					= 3;
//MP
float	ATTN_LOOP					= 4;//Sound will start on client even if volume is 0- so if player comes into range, it will be playing- attenuates like ATTN_NORM

// update types
float	UPDATE_GENERAL				= 0;
float	UPDATE_STATIC				= 1;
float	UPDATE_BINARY				= 2;
float	UPDATE_TEMP					= 3;

// entity effects
/* The only difference between Raven's hw-0.15 binary release and the
 * later HexenC source release is the EF_BRIGHTFIELD and EF_ONFIRE values:
 * the original binary releases had them as 1 and 1024 respectively, but
 * the later hcode src releases have them flipped: EF_BRIGHTFIELD = 1024
 * and EF_ONFIRE = 1, which is a BIG BOO BOO. (On the other hand, Siege
 * binary and source releases have EF_BRIGHTFIELD and EF_ONFIRE values as
 * 1 and 1024, which makes the mess even messier.. Sigh..)
 * The hexenworld engine src release also have EF_BRIGHTFIELD as 1024 and
 * EF_ONFIRE as 1, therefore uHexen2 sticks to those values.
 */
//float	EF_ONFIRE					= 1;//on fire
float	EF_ONFIRE					= 1024;//on fire
float	EF_MUZZLEFLASH 				= 2;//used for sword impacts and firing
float	EF_BRIGHTLIGHT 				= 4;//used a lot
float	EF_TORCHLIGHT				= 6;//4 and 2?
float	EF_DIMLIGHT 				= 8;//torch
float	EF_INVINC_CIRC				= 16;//Succubus' invincibility effect
float	EF_DARKLIGHT				= 16;//same
float	EF_DARKFIELD				= 32;//haste
float	EF_LIGHT					= 64;//small light field
float	EF_NODRAW					= 128;//invis
//end of first byte sent to clients

//where is 256 and 512?
//float	EF_BRIGHTFIELD				= 1024;//wasn't being used, put here
float	EF_BRIGHTFIELD				= 1;//wasn't being used, put here
float	EF_POWERFLAMEBURN			= 2048;
float	EF_POWERFLAME				= 4096;
float	EF_UPDATESOUND				= 8192;


float	EF_POISON_GAS				= 2097152;//MP
float	EF_ACIDBLOB					= 4194304;//MP
//float	EF_PURIFY2_EFFECT			= 2097152;
//float	EF_AXE_EFFECT				= 4194304;
//float	EF_SWORD_EFFECT				= 8388608;
float	EF_TORNADO_EFFECT			= 16777216;
float	EF_ICESTORM_EFFECT			= 33554432;
//float	EF_ICEBALL_EFFECT			= 67108864;
//float	EF_METEOR_EFFECT			= 134217728;
float	EF_HAMMER_EFFECTS			= 268435456;
float	EF_BEETLE_EFFECTS			= 536870912;

// messages
float   MSG_BROADCAST   = 0;            // unreliable to all
float   MSG_ONE                 = 1;            // reliable to one (msg_entity)
float   MSG_ALL                 = 2;            // reliable to all
float   MSG_INIT                = 3;            // write to the init string
float   MSG_MULTICAST   = 4;            // for multicast() call

// message levels
float   PRINT_LOW               = 0;            // pickup messages
float   PRINT_MEDIUM    	= 1;            // death messages
float   PRINT_HIGH              = 2;            // critical messages
float   PRINT_CHAT              = 3;            // also goes to chat console

// multicast sets
float   MULTICAST_ALL   = 0;            // every client
float   MULTICAST_PHS   = 1;            // within hearing
float   MULTICAST_PVS   = 2;            // within sight
float   MULTICAST_ALL_R = 3;            // every client, reliable
float   MULTICAST_PHS_R = 4;            // within hearing, reliable
float   MULTICAST_PVS_R = 5;            // within sight, reliable

float STEP_HEIGHT					= 18;		// Max step height

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
float	SFL_EPISODE_1		= 1;
float	SFL_EPISODE_2		= 2;
float	SFL_EPISODE_3		= 4;
float	SFL_EPISODE_4		= 8;
float	SFL_NEW_UNIT		= 16;
float	SFL_NEW_EPISODE		= 32;
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
float attck_cnt;

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

//spawnflag for all monsters
float JUMP	= 4;	    //Gives monster the ability to jump
float PLAY_DEAD	= 8;	//Makes a monster play dead at start
float NO_DROP	= 32;	//Keeps them from dropping to the ground at spawntime

//spawnflag for items, weapons, artifacts
float FLOATING	=	1;	//Keeps them from dropping to the ground at spawntime

//Spawnflags for barrels
float BARREL_DOWNHILL		= 1;
float BARREL_NO_DROP		= 2;
float ON_SIDE				= 4;
float BARREL_SINK			= 8;		
float DROP_USE				= 16;//Barrel won't drop unless used
float BARREL_RESPAWN		= 32;//Upon death, barrel will respawn at it's initial origin
//Barrel types
float BARREL_UNBREAKABLE	= 16;
float BARREL_NORMAL			= 32;
float BARREL_EXPLODING		= 64;
float BARREL_INDESTRUCTIBLE = 128;		
float BARREL_GFIRE			= 256;		

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

float SVC_SETVIEWPORT = 5;			// Net.Protocol 0x05- for camera
float SVC_SETVIEWANGLES = 10;		// Net.Protocol 0x0A- for camera

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

float XBOW_IMPACT_DEFAULT		= 0;
float XBOW_IMPACT_GREENFLESH	= 2;
float XBOW_IMPACT_REDFLESH		= 4;
float XBOW_IMPACT_WOOD			= 6;
float XBOW_IMPACT_STONE			= 8;
float XBOW_IMPACT_METAL			= 10;
float XBOW_IMPACT_ICE			= 12;
float XBOW_IMPACT_MUMMY			= 14;

float DM_CAPTURE_THE_TOKEN		= 1;
float DM_HUNTER					= 2;
float DM_SIEGE					= 3;

float GF_HAS_TOKEN				= 1;
float GF_HAS_LEFT_HUNTER		= 2;

//MISSION PACK
//for worldspawn
float MISSIONPACK				= 1;//Spawnflag on world to use some mission-pack specific code for brush ents
float WSF_SIEGE					= 2;//Spawnflag on world to use some mission-pack specific code for brush ents

//SIEGE
float ST_DEFENDER				= 1;
float ST_ATTACKER				= 2;
float ACT_YAK_HIT				= 2;
float ACT_YAK_HOWL				= 9;

float IT_WEAPON5				= 32;
float IT_WEAPON6				= 64;
float IT_WEAPON7				= 128;
float IT_WEAPON8				= 256;

float WP_DEFENDERS				= 10;//Defenders win pic
float WP_ATTCROWN				= 11;//Attackers capture crown
float WP_ATTKILL				= 12;//Attackers wipe out defenders

float MAX_CLASS					= 6;//Dwarf

//Siege
float SVC_HASKEY				= 79;  // [byte] [byte]
float SVC_NONEHASKEY			= 80;  // [byte] [byte]
float SVC_ISDOC					= 81;  // [byte] [byte]
float SVC_NODOC					= 82;  // [byte] [byte]
