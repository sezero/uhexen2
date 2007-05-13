/*
	cl_tent.c
	Client side temporary entity effects.

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/cl_tent.c,v 1.17 2007-05-13 11:58:28 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "quakedef.h"

// MACROS ------------------------------------------------------------------

#define MAX_STREAMS			32
#define MAX_STREAM_ENTITIES		128
#define STREAM_ATTACHED			16
#define STREAM_TRANSLUCENT		32

#define	TE_SPIKE			0
#define	TE_SUPERSPIKE			1
#define	TE_GUNSHOT			2
#define	TE_EXPLOSION			3
//#define	TE_TAREXPLOSION			4
#define	TE_LIGHTNING1			5
#define	TE_LIGHTNING2			6
#define	TE_WIZSPIKE			7
#define	TE_KNIGHTSPIKE			8
#define	TE_LIGHTNING3			9
#define	TE_LAVASPLASH			10
#define	TE_TELEPORT			11
//#define TE_EXPLOSION2			12
#define TE_STREAM_CHAIN			25
#define TE_STREAM_SUNSTAFF1		26
#define TE_STREAM_SUNSTAFF2		27
#define TE_STREAM_LIGHTNING		28
#define TE_STREAM_COLORBEAM		29
#define TE_STREAM_ICECHUNKS		30
#define TE_STREAM_GAZE			31
#define TE_STREAM_FAMINE		32
#define TE_STREAM_LIGHTNING_SMALL	24

// TYPES -------------------------------------------------------------------

typedef struct
{
	int		type;
	int		entity;
	int		tag;
	int		flags;
	int		skin;
	struct model_s *models[4];
	vec3_t	source;
	vec3_t	dest;
	vec3_t	offset;
	float	endTime;
	float	lastTrailTime;
} stream_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseStream(int type);
static stream_t *NewStream(int ent, int tag);
static entity_t *NewStreamEntity(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static stream_t cl_Streams[MAX_STREAMS];
static entity_t StreamEntities[MAX_STREAM_ENTITIES];
static int StreamEntityCount;
//static sfx_t *cl_sfx_wizhit;
//static sfx_t *cl_sfx_knighthit;
static sfx_t *cl_sfx_tink1;
static sfx_t *cl_sfx_ric1;
static sfx_t *cl_sfx_ric2;
static sfx_t *cl_sfx_ric3;
static sfx_t *cl_sfx_r_exp3;
#ifdef QUAKE2
static sfx_t *cl_sfx_imp;
static sfx_t *cl_sfx_rail;
#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
// CL_InitTEnts
//
//==========================================================================

void CL_InitTEnts(void)
{
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");
#ifdef QUAKE2
	cl_sfx_imp = S_PrecacheSound ("shambler/sattck1.wav");
	cl_sfx_rail = S_PrecacheSound ("weapons/lstart.wav");
#endif
}

//==========================================================================
//
// CL_ClearTEnts
//
//==========================================================================

void CL_ClearTEnts(void)
{
	memset(cl_Streams, 0, sizeof(cl_Streams));
}

//==========================================================================
//
// CL_ParseTEnt
//
//==========================================================================

void CL_ParseTEnt(void)
{
	int type;
	vec3_t pos;
#ifdef QUAKE2
	vec3_t endpos;
#endif
	dlight_t *dl;
	int rnd;
//	int colorStart, colorLength;

	type = MSG_ReadByte();
	switch (type)
	{
	case TE_WIZSPIKE:	// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);
//		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_KNIGHTSPIKE:	// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);
//		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;

	case TE_SPIKE:		// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 10);
		if (rand() % 5)
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_SUPERSPIKE:	// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);

		if (rand() % 5)
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_GUNSHOT:	// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);
		break;

	case TE_EXPLOSION:	// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion (pos);
	//	break;	// WTF ?.
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
#	ifdef GLQUAKE
		if (gl_colored_dynamic_lights.integer)
		{	// Make the dynamic light red
			dl->color[0] = 0.8;
			dl->color[1] = 0.2;
			dl->color[2] = 0.2;
			dl->color[3] = 0.7;
		}
#	endif
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	case TE_LIGHTNING1:
	case TE_LIGHTNING2:
	case TE_LIGHTNING3:
		MSG_ReadShort();
		MSG_ReadCoord();
		MSG_ReadCoord();
		MSG_ReadCoord();
		MSG_ReadCoord();
		MSG_ReadCoord();
		MSG_ReadCoord();
		break;

	case TE_STREAM_CHAIN:
	case TE_STREAM_SUNSTAFF1:
	case TE_STREAM_SUNSTAFF2:
	case TE_STREAM_LIGHTNING:
	case TE_STREAM_LIGHTNING_SMALL:
	case TE_STREAM_COLORBEAM:
	case TE_STREAM_ICECHUNKS:
	case TE_STREAM_GAZE:
	case TE_STREAM_FAMINE:
		ParseStream(type);
		break;

	case TE_LAVASPLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;

	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		break;
	default:
		Sys_Error ("%s: bad type", __thisfunc__);
	}
}

//==========================================================================
//
// ParseStream
//
//==========================================================================

static void ParseStream(int type)
{
	int		ent, tag, flags, skin;
	vec3_t	source, dest;
	float	duration;
	stream_t	*stream;
	model_t		*models[4];

	ent = MSG_ReadShort();
	flags = MSG_ReadByte();
	tag = flags&15;
	duration = (float)MSG_ReadByte()*0.05;
	skin = 0;
	if (type == TE_STREAM_COLORBEAM)
	{
		skin = MSG_ReadByte();
	}
	source[0] = MSG_ReadCoord();
	source[1] = MSG_ReadCoord();
	source[2] = MSG_ReadCoord();
	dest[0] = MSG_ReadCoord();
	dest[1] = MSG_ReadCoord();
	dest[2] = MSG_ReadCoord();

	models[1] = models[2] = models[3] = NULL;
	switch (type)
	{
	case TE_STREAM_CHAIN:
		models[0] = Mod_ForName("models/stchain.mdl", true);
		break;
	case TE_STREAM_SUNSTAFF1:
		models[0] = Mod_ForName("models/stsunsf1.mdl", true);
		models[1] = Mod_ForName("models/stsunsf2.mdl", true);
		models[2] = Mod_ForName("models/stsunsf3.mdl", true);
		models[3] = Mod_ForName("models/stsunsf4.mdl", true);
		break;
	case TE_STREAM_SUNSTAFF2:
		models[0] = Mod_ForName("models/stsunsf5.mdl", true);
		models[2] = Mod_ForName("models/stsunsf3.mdl", true);
		models[3] = Mod_ForName("models/stsunsf4.mdl", true);
		break;
	case TE_STREAM_LIGHTNING:
		models[0] = Mod_ForName("models/stlghtng.mdl", true);
//		duration*=2;
		break;
	case TE_STREAM_LIGHTNING_SMALL:
		models[0] = Mod_ForName("models/stltng2.mdl", true);
//		duration*=2;
		break;
	case TE_STREAM_FAMINE:
		models[0] = Mod_ForName("models/fambeam.mdl", true);
		break;
	case TE_STREAM_COLORBEAM:
		models[0] = Mod_ForName("models/stclrbm.mdl", true);
		break;
	case TE_STREAM_ICECHUNKS:
		models[0] = Mod_ForName("models/stice.mdl", true);
		break;
	case TE_STREAM_GAZE:
		models[0] = Mod_ForName("models/stmedgaz.mdl", true);
		break;
	default:
		models[0] = NULL;
		break;
	}
	if (models[0] == NULL)
		Sys_Error("%s: bad type", __thisfunc__);

	if ((stream = NewStream(ent, tag)) == NULL)
	{
		Con_Printf("stream list overflow\n");
		return;
	}
	stream->type = type;
	stream->tag = tag;
	stream->flags = flags;
	stream->entity = ent;
	stream->skin = skin;
	stream->models[0] = models[0];
	stream->models[1] = models[1];
	stream->models[2] = models[2];
	stream->models[3] = models[3];
	stream->endTime = cl.time+duration;
	stream->lastTrailTime = 0;
	VectorCopy(source, stream->source);
	VectorCopy(dest, stream->dest);
	if (flags & STREAM_ATTACHED)
	{
		VectorSubtract(source, cl_entities[ent].origin, stream->offset);
	}
}

//==========================================================================
//
// NewStream
//
//==========================================================================

static stream_t *NewStream(int ent, int tag)
{
	int			i;
	stream_t	*stream;

	// Search for a stream with matching entity and tag
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (stream->entity == ent && stream->tag == tag)
		{
			return stream;
		}
	}
	// Search for a free stream
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (!stream->models[0] || stream->endTime < cl.time)
		{
			return stream;
		}
	}
	return NULL;
}

//==========================================================================
//
// CL_UpdateTEnts
//
//==========================================================================

void CL_UpdateTEnts(void)
{
	int			i;
	stream_t	*stream;
	vec3_t	dist, org;
	float	d;
	entity_t	*ent;
	float	yaw, pitch, forward;
	int		offset;

	// Update streams
	StreamEntityCount = 0;
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (!stream->models[0])// || stream->endTime < cl.time)
		{ // Inactive
			continue;
		}
		if (stream->endTime < cl.time)
		{ // Inactive
			if (stream->type != TE_STREAM_LIGHTNING && stream->type != TE_STREAM_LIGHTNING_SMALL)
				continue;
			else if (stream->endTime + 0.25 < cl.time)
				continue;
		}

		if (stream->flags & STREAM_ATTACHED && stream->endTime >= cl.time)
		{ // Attach the start position to owner
			VectorAdd(cl_entities[stream->entity].origin, stream->offset, stream->source);
		}

		VectorSubtract(stream->dest, stream->source, dist);
		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
			{
				pitch = 90;
			}
			else
			{
				pitch = 270;
			}
		}
		else
		{
			yaw = (int)(atan2(dist[1], dist[0])*180/M_PI);
			if (yaw < 0)
			{
				yaw += 360;
			}
			forward = sqrt(dist[0]*dist[0]+dist[1]*dist[1]);
			pitch = (int)(atan2(dist[2], forward)*180/M_PI);
			if (pitch < 0)
			{
				pitch += 360;
			}
		}

		VectorCopy(stream->source, org);
		d = VectorNormalize(dist);

		if (stream->type == TE_STREAM_ICECHUNKS)
		{
			offset = (int)(cl.time*40)%30;
			for (i = 0; i < 3; i++)
			{
				org[i] += dist[i]*offset;
			}
		}

		while (d > 0)
		{
			ent = NewStreamEntity();

			if (!ent)
			{
				return;
			}

			VectorCopy(org, ent->origin);
			ent->model = stream->models[0];
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;

			switch (stream->type)
			{
			case TE_STREAM_CHAIN:
				ent->angles[2] = 0;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				break;
			case TE_STREAM_SUNSTAFF1:
				ent->angles[2] = (int)(cl.time*10)%360;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				//ent->frame = (int)(cl.time*20)%20;

				ent = NewStreamEntity();
				if (!ent)
				{
					return;
				}
				VectorCopy(org, ent->origin);
				ent->model = stream->models[1];
				ent->angles[0] = pitch;
				ent->angles[1] = yaw;
				ent->angles[2] = (int)(cl.time*50)%360;
				ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
				ent->abslight = 128;
				break;
			case TE_STREAM_SUNSTAFF2:
				ent->angles[2] = (int)(cl.time*10)%360;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				ent->frame = (int)(cl.time*10)%8;
				break;
			case TE_STREAM_LIGHTNING:
				if (stream->endTime < cl.time)
				{//fixme: keep last non-translucent frame and angle
					ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
					ent->abslight = 128 + (stream->endTime - cl.time)*192;
				}
				else
				{
					ent->angles[2] = rand() % 360;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->frame = rand() % 6;
				}
				break;
			case TE_STREAM_LIGHTNING_SMALL:
				if (stream->endTime < cl.time)
				{
					ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
					ent->abslight = 128 + (stream->endTime - cl.time)*192;
				}
				else
				{
					ent->angles[2] = rand() % 360;
					ent->frame = rand() % 6;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
				}
				break;
			case TE_STREAM_FAMINE:
				ent->angles[2] = rand() % 360;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				ent->frame = 0;
				break;
			case TE_STREAM_COLORBEAM:
				ent->angles[2] = 0;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				ent->skinnum = stream->skin;
				break;
			case TE_STREAM_GAZE:
				ent->angles[2] = 0;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				ent->frame = (int)(cl.time*40)%36;
				break;
			case TE_STREAM_ICECHUNKS:
				ent->angles[2] = rand() % 360;
				ent->drawflags = MLS_ABSLIGHT;
				ent->abslight = 128;
				ent->frame = rand() % 5;
				break;

			default:
				ent->angles[2] = 0;
			}

			for (i = 0; i < 3; i++)
			{
				org[i] += dist[i]*30;
			}

			d -= 30;
		}

		if (stream->type == TE_STREAM_SUNSTAFF1 || stream->type == TE_STREAM_SUNSTAFF2)
		{
			if (stream->lastTrailTime+0.2 < cl.time)
			{
				stream->lastTrailTime = cl.time;
				R_SunStaffTrail(stream->source, stream->dest);
			}

			ent = NewStreamEntity();
			if (ent == NULL)
			{
				return;
			}

			VectorCopy(stream->dest, ent->origin);
			ent->model = stream->models[2];
			ent->drawflags = MLS_ABSLIGHT;
			ent->abslight = 128;
			ent->scale = 80 + (rand() & 15);
			//ent->frame = (int)(cl.time*20)%20;

			ent = NewStreamEntity();
			if (ent == NULL)
			{
				return;
			}

			VectorCopy(stream->dest, ent->origin);
			ent->model = stream->models[3];
			ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
			ent->abslight = 128;
			ent->scale = 150 + (rand() & 15);
		}
	}
}

//==========================================================================
//
// NewStreamEntity
//
//==========================================================================

static entity_t *NewStreamEntity(void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
	{
		return NULL;
	}

	if (StreamEntityCount == MAX_STREAM_ENTITIES)
	{
		return NULL;
	}

	ent = &StreamEntities[StreamEntityCount++];
	memset(ent, 0, sizeof(*ent));
	cl_visedicts[cl_numvisedicts++] = ent;
	ent->colormap = vid.colormap;

	return ent;
}

