/*
	server.h

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server.h,v 1.6 2005-05-19 16:47:18 sezero Exp $
*/

typedef struct
{
	int			maxclients;
	int			maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int			serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================

typedef enum {ss_loading, ss_active} server_state_t;

typedef struct
{
	qboolean	active;				// false if only a net client

	qboolean	paused;
	qboolean	loadgame;			// handle connections specially

	double		time;
	
	int			lastcheck;			// used by PF_checkclient
	double		lastchecktime;
	
	char		name[64];			// map name
	char		midi_name[128];     // midi file name
	byte		cd_track;			// cd track number

	char		startspot[64];
	char		modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s 	*worldmodel;
	char		*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	char		*sound_precache[MAX_SOUNDS];	// NULL terminated
	char		*lightstyles[MAX_LIGHTSTYLES];
	struct EffectT Effects[MAX_EFFECTS];
	client_state2_t *states;
	int			num_edicts;
	int			max_edicts;
	edict_t		*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
	server_state_t	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[NET_MAXMESSAGE];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[NET_MAXMESSAGE];

	sizebuf_t	signon;
	byte		signon_buf[NET_MAXMESSAGE];
} server_t;


#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		16

typedef struct client_s
{
	qboolean		active;				// false = client is free
	qboolean		spawned;			// false = don't send datagrams
	qboolean		dropasap;			// has been told to go to another level
	qboolean		sendsignon;			// only valid before spawned

	double			last_message;		// reliable messages must be sent
										// periodically

	struct qsocket_s *netconnection;	// communications handle

 	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd

	sizebuf_t		message;			// can be added to at any time,
										// copied and clear once per frame
	byte			msgbuf[MAX_MSGLEN];

	sizebuf_t		datagram;
	byte			datagram_buf[NET_MAXMESSAGE];

	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// for printing to other people
	int				colors;
	float			playerclass;
		
	float			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas	
	int				old_frags;
	entvars_t		old_v;
	qboolean        send_all_v;

	byte			current_frame, last_frame;
	byte			current_sequence, last_sequence;

	long			info_mask, info_mask2;
} client_t;


//=============================================================================

// edict->movetype values
#define	MOVETYPE_NONE			0		// never moves
#define	MOVETYPE_ANGLENOCLIP	1
#define	MOVETYPE_ANGLECLIP		2
#define	MOVETYPE_WALK			3		// gravity
#define	MOVETYPE_STEP			4		// gravity, special edge handling
#define	MOVETYPE_FLY			5
#define	MOVETYPE_TOSS			6		// gravity
#define	MOVETYPE_PUSH			7		// no clip to world, push and crush
#define	MOVETYPE_NOCLIP			8
#define	MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define	MOVETYPE_BOUNCE			10
//#ifdef QUAKE2
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// track movement of aiment
//#endif
#define MOVETYPE_PUSHPULL		13		// pushable/pullable object
#define MOVETYPE_SWIM			14		// should keep the object in water

// edict->solid values
#define	SOLID_NOT				0		// no interaction with other objects
#define	SOLID_TRIGGER			1		// touch on edge, but not blocking
#define	SOLID_BBOX				2		// touch on edge, block
#define	SOLID_SLIDEBOX			3		// touch on edge, but not an onground
#define	SOLID_BSP				4		// bsp clip, touch on edge, block
#define	SOLID_PHASE				5		// won't slow down when hitting entities flagged as FL_MONSTER

// edict->deadflag values
#define	DEAD_NO					0
#define	DEAD_DYING				1
#define	DEAD_DEAD				2

#define	DAMAGE_NO				0		// Cannot be damaged
#define	DAMAGE_YES				1		// Can be damaged
#define	DAMAGE_NO_GRENADE		2		// Will not set off grenades

// edict->flags
#define	FL_FLY					1
#define	FL_SWIM					2
//#define	FL_GLIMPSE				4
#define	FL_CONVEYOR				4
#define	FL_CLIENT				8
#define	FL_INWATER				16
#define	FL_MONSTER				32
#define	FL_GODMODE				64
#define	FL_NOTARGET				128
#define	FL_ITEM					256
#define	FL_ONGROUND				512
#define	FL_PARTIALGROUND		1024	// not all corners are valid
#define	FL_WATERJUMP			2048	// player jumping out of water
#define	FL_JUMPRELEASED			4096	// for jump debouncing
#define FL_FLASHLIGHT			8192
#define FL_ARCHIVE_OVERRIDE		1048576
#define	FL_ARTIFACTUSED			16384
#define FL_MOVECHAIN_ANGLE		32768    // when in a move chain, will update the angle
#define	FL_HUNTFACE				65536	//Makes monster go for enemy view_ofs thwn moving
#define	FL_NOZ					131072	//Monster will not automove on Z if flying or swimming
#define	FL_SET_TRACE			262144	//Trace will always be set for this monster (pentacles)
#define FL_CLASS_DEPENDENT		2097152  // model will appear different to each player
#define FL_SPECIAL_ABILITY1		4194304  // has 1st special ability
#define FL_SPECIAL_ABILITY2		8388608  // has 2nd special ability

#define	FL2_CROUCHED			4096


// edict->drawflags
#define MLS_MASKIN				7	// MLS: Model Light Style
#define MLS_MASKOUT				248
#define MLS_NONE				0
#define MLS_FULLBRIGHT			1
#define MLS_POWERMODE			2
#define MLS_TORCH				3
#define MLS_TOTALDARK			4
#define MLS_ABSLIGHT			7
#define SCALE_TYPE_MASKIN		24
#define SCALE_TYPE_MASKOUT		231
#define SCALE_TYPE_UNIFORM		0	// Scale X, Y, and Z
#define SCALE_TYPE_XYONLY		8	// Scale X and Y
#define SCALE_TYPE_ZONLY		16	// Scale Z
#define SCALE_ORIGIN_MASKIN		96
#define SCALE_ORIGIN_MASKOUT	159
#define SCALE_ORIGIN_CENTER		0	// Scaling origin at object center
#define SCALE_ORIGIN_BOTTOM		32	// Scaling origin at object bottom
#define SCALE_ORIGIN_TOP		64	// Scaling origin at object top
#define DRF_TRANSLUCENT			128
#define DRF_ANIMATEONCE			256


// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8
#define EF_DARKLIGHT			16
#define EF_DARKFIELD			32
#define EF_LIGHT				64
#define EF_NODRAW				128

// Player Classes
#define CLASS_PALADIN				1
#define CLASS_CLERIC 				2
#define CLASS_NECROMANCER			3
#define CLASS_THEIF   				4
#define CLASS_DEMON					5

// Built-in Spawn Flags
#define SPAWNFLAG_NOT_PALADIN       0x00000100
#define SPAWNFLAG_NOT_CLERIC		0x00000200
#define SPAWNFLAG_NOT_NECROMANCER	0x00000400
#define SPAWNFLAG_NOT_THEIF			0x00000800
#define	SPAWNFLAG_NOT_EASY			0x00001000
#define	SPAWNFLAG_NOT_MEDIUM		0x00002000
#define	SPAWNFLAG_NOT_HARD		    0x00004000
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00008000
#define SPAWNFLAG_NOT_COOP			0x00010000
#define SPAWNFLAG_NOT_SINGLE		0x00020000
#define SPAWNFLAG_NOT_DEMON			0x00040000


// server flags
#define	SFL_EPISODE_1		1
#define	SFL_EPISODE_2		2
#define	SFL_EPISODE_3		4
#define	SFL_EPISODE_4		8
#define	SFL_NEW_UNIT		16
#define	SFL_NEW_EPISODE		32
#define	SFL_CROSS_TRIGGERS	65280

//============================================================================

extern	cvar_t	teamplay;
extern	cvar_t	skill;
extern	cvar_t	deathmatch;
extern	cvar_t	randomclass;
extern	cvar_t	coop;
extern	cvar_t	fraglimit;
extern	cvar_t	timelimit;

extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server

extern	client_t	*host_client;

extern	jmp_buf 	host_abortserver;

extern	double		host_time;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);

void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count);
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count);
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count);
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume, float attenuation);
void SV_StopSound (edict_t *entity, int channel);
void SV_UpdateSoundPos (edict_t *entity, int channel);

void SV_DropClient (qboolean crash);

void SV_Edicts(char *Name);

void SV_SendClientMessages (void);
void SV_ClearDatagram (void);

int SV_ModelIndex (char *name);

void SV_SetIdealPitch (void);

void SV_AddUpdates (void);

void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s	*ret);

void SV_ClientPrintf (char *fmt, ...);
void SV_BroadcastPrintf (char *fmt, ...);

void SV_Physics (void);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink, qboolean noenemy,
					  qboolean set_trace);

void SV_WriteClientdataToMessage (client_t *client, edict_t *ent, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms ();
void SV_SpawnServer (char *server, char *startspot);

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/05/19 16:41:50  sezero
 * removed all unused (never used) non-RJNET and non-QUAKE2RJ code
 *
 * Revision 1.4  2005/04/30 08:39:08  sezero
 * silenced shadowed decleration warnings about volume (now sfxvolume)
 *
 * Revision 1.3  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:07:15  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 6     2/27/98 11:53p Jweier
 * 
 * 5     1/22/98 5:01p Mgummelt
 * 
 * 4     1/22/98 4:29p Jweier
 * 
 * 3     1/18/98 8:06p Jmonroe
 * all of rick's patch code is in now
 * 
 * 41    10/28/97 2:58p Jheitzman
 * 
 * 39    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 38    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 37    8/26/97 10:29a Rjohnson
 * Made flags2 be set when a player crouches
 * 
 * 36    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 35    8/19/97 10:35p Rjohnson
 * Fix for buffer size
 * 
 * 34    7/24/97 5:21p Rlove
 * 
 * 33    7/15/97 1:58p Bgokey
 * 
 * 32    7/14/97 4:47p Rjohnson
 * Fix for movetype_phase
 * 
 * 31    7/01/97 10:00p Rjohnson
 * Fix for the size
 * 
 * 30    7/01/97 4:41p Rjohnson
 * Fixed a network buffer size problem
 * 
 * 29    6/05/97 11:07a Rjohnson
 * Models can be seen differently based upon class
 * 
 * 28    5/23/97 3:04p Rjohnson
 * Included some more quake2 things
 * 
 * 27    5/20/97 11:32a Rjohnson
 * Revised Effects
 * 
 * 26    5/19/97 2:54p Rjohnson
 * Added new client effects
 * 
 * 25    5/07/97 11:13a Rjohnson
 * Added a new field to the movement routines to allow setting of the
 * traceline info
 * 
 * 24    4/22/97 3:50p Rjohnson
 * Added some more particle commands to cut back on the networking
 * 
 * 23    4/15/97 9:02p Bgokey
 * 
 * 22    4/15/97 11:52a Rjohnson
 * Updates from quake2 for multi-level trigger stuff
 * 
 * 21    4/04/97 3:07p Rjohnson
 * Networking updates and corrections
 * 
 * 20    3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 19    3/26/97 4:05p Rjohnson
 * Added the ability to link entities so that they will move together
 * 
 * 18    3/25/97 12:50a Bgokey
 * 
 * 17    3/22/97 4:52p Rjohnson
 * Moved the stone ability to be based upon the skin number
 * 
 * 16    3/22/97 2:40p Rjohnson
 * Added the stone draw flag
 * 
 * 15    3/21/97 5:25p Rlove
 * New movetype PUSHPULL
 * 
 * 14    3/17/97 12:40p Rlove
 * New axe is in.
 * 
 * 13    3/15/97 3:08p Rlove
 * Added COMA console command
 * 
 * 12    3/13/97 9:57a Rlove
 * Changed constant DAMAGE_AIM  to DAMAGE_YES and the old DAMAGE_YES to
 * DAMAGE_NO_GRENADE
 * 
 * 11    3/12/97 10:58p Rjohnson
 * Revised the particle2 hexen-c command to allow a range for the velocity
 * - shouldn't be as taxing on the network to get better effects
 * 
 * 10    3/03/97 5:00p Rjohnson
 * Added spawn flags and code to prevent items flagged from being spawned
 * 
 * 9     3/03/97 4:03p Rjohnson
 * Added cd specifications to the world-spawn entity
 * 
 * 8     2/27/97 4:11p Rjohnson
 * Midi file name is stored in the server and client structs
 * 
 * 7     2/20/97 12:13p Rjohnson
 * Code fixes for id update
 * 
 * 6     2/19/97 12:00p Rjohnson
 * Id Updates
 * 
 * 5     2/17/97 12:17p Bgokey
 * 
 * 4     1/28/97 10:28a Rjohnson
 * Added experience and level advancement
 * 
 * 3     1/02/97 11:16a Rjohnson
 * Christmas work - added adaptive time, game delays, negative light,
 * particle effects, etc
 * 
 * 2     11/26/96 4:10p Rjohnson
 * Added sv_movestep to the quake c external functions.  Modified the
 * routine so that it can ignore enemy positions.
 */
