/*
	client.h
	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/client.h,v 1.13 2006-03-24 15:05:39 sezero Exp $
*/

typedef struct
{
	vec3_t	viewangles;

// intended velocities
	float	forwardmove;
	float	sidemove;
	float	upmove;
	byte	lightlevel;
} usercmd_t;

typedef struct
{
	int		length;
	char	map[MAX_STYLESTRING];
} lightstyle_t;

typedef struct
{
	char	name[MAX_SCOREBOARDNAME];
	float	entertime;
	int		frags;
	int		colors;			// two 4 bit fields
	byte	translations[VID_GRADES*256];
	float	playerclass;
} scoreboard_t;

typedef struct
{
	int		destcolor[3];
	int		percent;		// 0-256
} cshift_t;

#define	CSHIFT_CONTENTS		0
#define	CSHIFT_DAMAGE		1
#define	CSHIFT_BONUS		2
#define	CSHIFT_POWERUP		3
#define	CSHIFT_INTERVENTION	4
#define	NUM_CSHIFTS		5

#define	NAME_LENGTH	64


//
// client_state_t should hold all pieces of the client state
//

#define	SIGNONS		4			// signon messages to receive before connected

#define	MAX_DLIGHTS		32
typedef struct
{
	vec3_t	origin;
	float	radius;
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	minlight;			// don't add when contributing less
	int		key;
	qboolean	dark;			// subtracts light instead of adding
	float	color[4];			// LordHavoc: colored lights support
} dlight_t;

#define	MAX_EFRAGS		640

#define	MAX_MAPSTRING	2048
#define	MAX_DEMOS		8
#define	MAX_DEMONAME	16

typedef enum {
ca_dedicated, 		// a dedicated server with no ability to start a client
ca_disconnected, 	// full screen console with no connection
ca_connected		// valid netcon, talking to a server
} cactive_t;

//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
typedef struct
{
	cactive_t	state;

// personalization data sent to server	
	char		mapstring[MAX_QPATH];
	char		spawnparms[MAX_MAPSTRING];	// to restart a level

// demo loop control
	int			demonum;		// -1 = don't play demos
	char		demos[MAX_DEMOS][MAX_DEMONAME];	// when not playing

// demo recording info must be here, because record is started before
// entering a map (and clearing client_state_t)
	qboolean	demorecording;
	qboolean	demoplayback;
	qboolean	timedemo;
	int			forcetrack;	// -1 = use normal cd track
	FILE		*demofile;
	FILE		*introdemofile;
	int			td_lastframe;	// to meter out one message a frame
	int			td_startframe;	// host_framecount at start
	float		td_starttime;		// realtime at second frame of timedemo


// connection information
	int			signon;		// 0 to SIGNONS
	struct qsocket_s	*netcon;
	sizebuf_t	message;		// writing buffer to send to server

} client_static_t;

extern client_static_t	cls;


//
// the client_state_t structure is wiped completely at every
// server signon
//
typedef struct
{
	int		movemessages;	// since connecting to this server
					// throw out the first couple, so the player
					// doesn't accidentally do something the 
					// first frame

	usercmd_t	cmd;		// last command sent to the server

// information for local display
	int		stats[MAX_CL_STATS];	// health, etc
	int		inv_order[MAX_INVENTORY];
	int		inv_count, inv_startpos, inv_selected;
	int		items;		// inventory bit flags
	float	item_gettime[32];	// cl.time of aquiring item, for blinking
	float	faceanimtime;		// use anim frame if cl.time < this

	entvars_t	v;	// NOTE: not every field will be update
				// you must specifically add them in
				// functions SV_WriteClientdatatToMessage()
				// and CL_ParseClientdata()

	cshift_t	cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups
	cshift_t	prev_cshifts[NUM_CSHIFTS];	// and content types

	char puzzle_pieces[8][10]; // puzzle piece names

// the client maintains its own idea of view angles, which are
// sent to the server each frame.  The server sets punchangle when
// the view is temporarliy offset, and an angle reset commands at the start
// of each level and after teleporting.

	vec3_t	mviewangles[2];	// during demo playback viewangles is lerped
				// between these
	vec3_t	viewangles;

	vec3_t	mvelocity[2];	// update by server, used for lean+bob
				// (0 is newest)
	vec3_t	velocity;	// lerped between mvelocity[0] and [1]

	vec3_t	punchangle;	// temporary offset

	float	idealroll;
	float	rollvel;

// pitch drifting vars
	float	idealpitch;
	float	pitchvel;
	qboolean	nodrift;
	float	driftmove;
	double	laststop;

	float	viewheight;
	float	crouch;		// local amount for smoothing stepups

	qboolean	paused;	// send over by server
	qboolean	onground;
	qboolean	inwater;

	int		intermission;	// don't change view angle, full screen, etc
	int		completed_time;	// latched at intermission start
	
	double	mtime[2];	// the timestamp of last two messages	
	double	time;		// clients view of time, should be between
				// servertime and oldservertime to generate
				// a lerp point for other data

	double	oldtime;	// previous cl.time, time-oldtime is used
				// to decay light values and smooth step ups

	float	last_received_message;	// (realtime) for net trouble icon

//
// information that is static for the entire time connected to a server
//
	struct model_s	*model_precache[MAX_MODELS];
	struct sfx_s	*sound_precache[MAX_SOUNDS];

	char		levelname[40];	// for display on solo scoreboard
	int		viewentity;	// cl_entitites[cl.viewentity] = player
	int		maxclients;
	int		gametype;

// refresh related state
	struct model_s	*worldmodel;	// cl_entitites[0].model
	struct efrag_s	*free_efrags;
	int		num_entities;	// held in cl_entities array
	int		num_statics;	// held in cl_staticentities array
	entity_t	viewent;	// the gun model
	struct EffectT Effects[MAX_EFFECTS];

	int		cdtrack, looptrack;	// cd audio
	char		midi_name[128];		// midi file name
	byte		current_frame, last_frame, reference_frame;
	byte		current_sequence, last_sequence;
	byte		need_build;

// frag scoreboard
	scoreboard_t	*scores;	// [cl.maxclients]

// light level at player's position including dlights
// this is sent back to the server each frame
// architectually ugly but it works
	int		light_level;

	client_frames2_t frames[3]; // 0 = base, 1 = building, 2 = 0 & 1 merged
	short	RemoveList[MAX_CLIENT_STATES], NumToRemove;

// mission pack, objectives strings
	long	info_mask, info_mask2;
} client_state_t;


//
// cvars
//
extern	cvar_t	cl_name;
extern	cvar_t	cl_color;
extern	cvar_t	cl_playerclass;

extern	cvar_t	cl_warncmd;
extern	cvar_t	cl_upspeed;
extern	cvar_t	cl_forwardspeed;
extern	cvar_t	cl_backspeed;
extern	cvar_t	cl_sidespeed;

extern	cvar_t	cl_movespeedkey;

extern	cvar_t	cl_yawspeed;
extern	cvar_t	cl_pitchspeed;

extern	cvar_t	cl_anglespeedkey;

extern	cvar_t	cl_autofire;

extern	cvar_t	cl_shownet;
extern	cvar_t	cl_nolerp;

extern	cvar_t	cl_pitchdriftspeed;
extern	cvar_t	lookspring;
extern	cvar_t	lookstrafe;
extern	cvar_t	sensitivity;

extern	cvar_t	m_pitch;
extern	cvar_t	m_yaw;
extern	cvar_t	m_forward;
extern	cvar_t	m_side;


#define	MAX_STATIC_ENTITIES	256	// torches, etc

extern	client_state_t	cl;

// FIXME, allocate dynamically
extern	efrag_t		cl_efrags[MAX_EFRAGS];
extern	entity_t	cl_entities[MAX_EDICTS];
extern	entity_t	cl_static_entities[MAX_STATIC_ENTITIES];
extern	lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
extern	dlight_t	cl_dlights[MAX_DLIGHTS];

//=============================================================================

//
// cl_main
//
dlight_t *CL_AllocDlight (int key);
void	CL_DecayLights (void);

void CL_Init (void);

void CL_EstablishConnection (char *host);
void CL_Signon1 (void);
void CL_Signon2 (void);
void CL_Signon3 (void);
void CL_Signon4 (void);
void CL_SignonReply (void);

void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_NextDemo (void);

qboolean CL_CopyFiles(char *source, char *pat, char *dest);
void CL_RemoveGIPFiles (char *path);

#define			MAX_VISEDICTS	256
extern	int				cl_numvisedicts;
extern	entity_t		*cl_visedicts[MAX_VISEDICTS];

//
// cl_cmd
//
void Cmd_ForwardToServer (void);
void CL_Cmd_Init (void);

//
// cl_input
//
typedef struct
{
	int		down[2];	// key nums holding it down
	int		state;		// low bit is down state
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_SendMove (usercmd_t *cmd);

void CL_ClearState (void);

int  CL_ReadFromServer (void);
void CL_WriteToServer (usercmd_t *cmd);
void CL_BaseMove (usercmd_t *cmd);


char *Key_KeynumToString (int keynum);

//
// cl_demo.c
//
void CL_StopPlayback (void);
int CL_GetMessage (void);

void CL_Stop_f (void);
void CL_Record_f (void);
void CL_PlayDemo_f (void);
void CL_TimeDemo_f (void);

//
// cl_parse.c
//
void CL_ParseServerMessage (void);

//
// view
//
void V_StartPitchDrift (void);
void V_StopPitchDrift (void);

void V_RenderView (void);
void V_UpdatePalette (void);
void V_Register (void);
void V_ParseDamage (void);
void V_SetContentsColor (int contents);

//
// cl_tent
//
void CL_InitTEnts(void);
void CL_ClearTEnts(void);
void CL_ParseTEnt(void);
void CL_UpdateTEnts(void);


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.12  2006/03/16 21:19:00  sezero
 * Restored net compatibility with SC2_OBJ and SC2_OBJ2:
 * SC2_OBJ and SC2_OBJ2 are actually only legible for the
 * mission pack (the objectives).  still keeping them as
 * H2-legible here, so that we remain net-compatible with
 * Hammer of Thyrion versions prior to 1.4.0 and with other
 * hexen2 source ports and demos recorded can still be
 * played back. The info_mask and infomask2 members of the
 * client structure will not be processed by the rest of
 * the code for pure hexen2 builds, though.
 * It would be much better if we'd made the engine handle
 * both H2 and H2MP cases *correctly*. Hmm...
 *
 * Revision 1.11  2006/03/14 11:44:18  sezero
 * arghhh... cvs changelogs...
 *
 * Revision 1.10  2006/03/13 22:28:51  sezero
 * removed the expansion pack only feature of objective strings from
 * hexen2-only builds (many new ifdef H2MP stuff). removed the expansion
 * pack only intermission picture and string searches from hexen2-only
 * builds.
 *
 * Revision 1.9  2006/03/10 08:08:45  sezero
 * Added support for colored lights and .lit files;. initially extracted
 * from jshexen2 (thanks Michal Wozniak). Colored lights and lit file
 * support is now added to hexenworld, as well, although the new cvars
 * gl_colored_dynamic_lights and gl_extra_dynamic_light aren't functional
 * for it: hexenworld had some sort of "colored" dynamic lights in it,
 * and they aren't changed by this patch. The cvars mentions are fully
 * functional for hexen2. Colored lights member of the dlight_t in h2
 * is changed to be an array of 4, instead of vec3_t as it used to appear
 * in jshexen2, so that things look more alike with the hw version. The
 * default lightmap format is changed to GL_RGBA, (previously it was
 * GL_LUMINANCE.) Command line arguments can be employed to change the
 * lightmap format:  -lm_1 : GL_LUMINANCE, -lm_4 : GL_RGBA (default).
 * (Note: Would it be a good (AND a feasible) idea to add a menu+cvar
 * control for the lightmap format?)
 *
 * Revision 1.8  2005/10/29 21:43:22  sezero
 * unified cmd layer
 *
 * Revision 1.7  2005/10/25 20:08:41  sezero
 * coding style and whitespace cleanup.
 *
 * Revision 1.6  2005/10/25 20:04:17  sezero
 * static functions part-1: started making local functions static,
 * killing nested externs, const vars clean-up.
 *
 * Revision 1.5  2005/05/19 16:41:50  sezero
 * removed all unused (never used) non-RJNET and non-QUAKE2RJ code
 *
 * Revision 1.4  2005/01/01 21:43:47  sezero
 * prototypes clean-up
 *
 * Revision 1.3  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:01:15  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 4     3/12/98 6:31p Mgummelt
 * 
 * 3     3/02/98 2:51p Jweier
 * 
 * 2     2/27/98 11:53p Jweier
 * 
 * 22    9/02/97 12:24a Rjohnson
 * Update
 * 
 * 21    8/30/97 6:17p Rjohnson
 * Network changes
 * 
 * 20    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 19    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 18    8/11/97 2:52p Rlove
 * 
 * 17    8/04/97 2:22p Rjohnson
 * Included light level for the player
 * 
 * 16    6/16/97 7:52a Bgokey
 * 
 * 15    6/09/97 1:56p Rjohnson
 * Increased max statics
 * 
 * 14    5/23/97 3:04p Rjohnson
 * Included some more quake2 things
 * 
 * 13    5/20/97 11:32a Rjohnson
 * Revised Effects
 * 
 * 12    5/20/97 11:19a Bgokey
 * 
 * 11    5/10/97 1:08p Bgokey
 * 
 * 10    4/09/97 2:34p Rjohnson
 * Revised inventory
 * 
 * 9     4/04/97 3:06p Rjohnson
 * Networking updates and corrections
 * 
 * 8     3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 7     2/27/97 4:11p Rjohnson
 * Midi file name is stored in the server and client structs
 * 
 * 6     2/19/97 11:20a Rjohnson
 * Id Updates
 * 
 * 5     2/10/97 2:05p Rlove
 * Inventory change
 * 
 * 4     1/30/97 3:09p Rlove
 * Refined flight mode and added sv_walkpitch, sv_flypitch,
 * sv_idealrollscale console varibles
 * 
 * 3     12/18/96 9:33a Rlove
 * Changes for inventory screen
 * 
 */

