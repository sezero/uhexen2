/*
	sv_init.c
	server spawning

	$Id: sv_init.c,v 1.16 2007-09-14 14:11:24 sezero Exp $
*/

#include "quakedef.h"

server_static_t	svs;			// persistant server info
server_t		sv;		// local server

char	localmodels[MAX_MODELS][5];	// inline model names for precache

char	localinfo[MAX_LOCALINFO_STRING+1]; // local game info


/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (const char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i = 1; i < MAX_MODELS && sv.model_precache[i][0]; i++)
	{
		if (!strcmp(sv.model_precache[i], name))
			return i;
	}

	if (i == MAX_MODELS || !sv.model_precache[i][0])
		SV_Error ("%s: model %s not precached", __thisfunc__, name);

	return i;
}

/*
================
SV_FlushSignon

Moves to the next signon buffer if needed
================
*/
void SV_FlushSignon (void)
{
	if (sv.signon.cursize < sv.signon.maxsize - 100)
		return;

	if (sv.num_signon_buffers == MAX_SIGNON_BUFFERS-1)
		SV_Error ("sv.num_signon_buffers == MAX_SIGNON_BUFFERS-1");

	sv.signon_buffer_size[sv.num_signon_buffers-1] = sv.signon.cursize;
	sv.signon.data = sv.signon_buffers[sv.num_signon_buffers];
	sv.num_signon_buffers++;
	sv.signon.cursize = 0;
}

/*
================
SV_CreateBaseline

Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
static void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;

	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		// create baselines for all player slots,
		// and any other edict that has a visible model
		if (entnum > MAX_CLIENTS && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = svent->v.frame;
		svent->baseline.skinnum = svent->v.skin;
		if (entnum > 0 && entnum <= MAX_CLIENTS)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndex("models/paladin.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(PR_GetString(svent->v.model));
		}

		svent->baseline.scale = (int)(svent->v.scale*100.0)&255;
		svent->baseline.drawflags = svent->v.drawflags;
		svent->baseline.abslight = (int)(svent->v.abslight*255.0)&255;
		//
		// flush the signon message out to a seperate buffer if
		// nearly full
		//
		SV_FlushSignon ();

		//
		// add to the message
		//
		MSG_WriteByte (&sv.signon,svc_spawnbaseline);
		MSG_WriteShort (&sv.signon,entnum);

		MSG_WriteShort (&sv.signon, svent->baseline.modelindex);
		MSG_WriteByte (&sv.signon, svent->baseline.frame);
		MSG_WriteByte (&sv.signon, svent->baseline.colormap);
		MSG_WriteByte (&sv.signon, svent->baseline.skinnum);
		MSG_WriteByte (&sv.signon, svent->baseline.scale);
		MSG_WriteByte (&sv.signon, svent->baseline.drawflags);
		MSG_WriteByte (&sv.signon, svent->baseline.abslight);
		for (i = 0; i < 3; i++)
		{
			MSG_WriteCoord(&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle(&sv.signon, svent->baseline.angles[i]);
		}
	}
}


/*
================
SV_SaveSpawnparms

Grabs the current state of the progs serverinfo flags
and each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	if (!sv.state)
		return;		// no progs loaded yet

	// serverflags is the only game related thing maintained
	svs.serverflags = PR_GLOBAL_STRUCT(serverflags);

	for (i = 0, host_client = svs.clients; i < MAX_CLIENTS; i++, host_client++)
	{
		if (host_client->state != cs_spawned)
			continue;

		// needs to reconnect
		host_client->state = cs_connected;

		// call the progs to get default spawn parms for the new client
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct->SetChangeParms);
		for (j = 0; j < NUM_SPAWN_PARMS; j++)
			host_client->spawn_parms[j] = (&pr_global_struct->parm1)[j];
	}
}

/*
================
SV_CalcPHS

Expands the PVS and calculates the PHS
(Potentially Hearable Set)
================
*/
static void SV_CalcPHS (void)
{
	int		rowbytes, rowwords;
	int		i, j, k, l, idx, num;
	int		bitbyte;
	unsigned int	*dest, *src;
	byte	*scan;
	int		count, vcount;

	Con_Printf ("Building PHS...\n");

	num = sv.worldmodel->numleafs;
	rowwords = (num+31)>>5;
	rowbytes = rowwords*4;

	sv.pvs = (byte *) Hunk_AllocName (rowbytes*num, "pvs");
	scan = sv.pvs;
	vcount = 0;
	for (i = 0; i < num; i++, scan += rowbytes)
	{
		memcpy (scan, Mod_LeafPVS(sv.worldmodel->leafs+i, sv.worldmodel),
			rowbytes);
		if (i == 0)
			continue;
		for (j = 0; j < num; j++)
		{
			if ( scan[j>>3] & (1<<(j&7)) )
			{
				vcount++;
			}
		}
	}

	sv.phs = (byte *) Hunk_AllocName (rowbytes*num, "phs");
	count = 0;
	scan = sv.pvs;
	dest = (unsigned int *)sv.phs;
	for (i = 0; i < num; i++, dest += rowwords, scan += rowbytes)
	{
		memcpy (dest, scan, rowbytes);
		for (j = 0; j < rowbytes; j++)
		{
			bitbyte = scan[j];
			if (!bitbyte)
				continue;
			for (k = 0; k < 8; k++)
			{
				if (! (bitbyte & (1<<k)) )
					continue;
				// or this pvs row into the phs
				// +1 because pvs is 1 based
				idx = ((j<<3) + k + 1);
				if (idx >= num)
					continue;
				src = (unsigned int *)sv.pvs + idx*rowwords;
				for (l = 0; l < rowwords; l++)
					dest[l] |= src[l];
			}
		}

		if (i == 0)
			continue;
		for (j = 0; j < num; j++)
		{
			if ( ((byte *)dest)[j>>3] & (1<<(j&7)) )
				count++;
		}
	}

	Con_Printf ("Average leafs visible / hearable / total: %i / %i / %i\n",
						vcount/num, count/num, num);
}

/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.

This is only called from the SV_Map_f() function.
================
*/
void SV_SpawnServer (const char *server, const char *startspot)
{
	edict_t		*ent;
	int			i;

	Con_DPrintf ("%s: %s\n", __thisfunc__, server);

	SV_SaveSpawnparms ();

	svs.spawncount++;	// any partially connected client will be
				// restarted

	sv.state = ss_dead;

	Mod_ClearAll ();
	Hunk_FreeToLowMark (host_hunklevel);

	// wipe the entire per-level structure
	memset (&sv, 0, sizeof(sv));

	SZ_Init (&sv.datagram, sv.datagram_buf, sizeof(sv.datagram_buf));
	sv.datagram.allowoverflow = true;

	SZ_Init (&sv.reliable_datagram, sv.reliable_datagram_buf, sizeof(sv.reliable_datagram_buf));
	SZ_Init (&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));
	SZ_Init (&sv.master, sv.master_buf, sizeof(sv.master_buf));
	SZ_Init (&sv.signon, sv.signon_buffers[0], sizeof(sv.signon_buffers[0]));
	sv.num_signon_buffers = 1;

	strcpy (sv.name, server);
	if (startspot)
	{
		strcpy(sv.startspot, startspot);
	}

	// load progs to get entity field count
	// which determines how big each edict is
	PR_LoadProgs ();
	PR_LoadStrings();

	// allocate edicts
	sv.edicts = (edict_t *) Hunk_AllocName (MAX_EDICTS*pr_edict_size, "edicts");

	// leave slots at start for clients only
	sv.num_edicts = MAX_CLIENTS + 1 + max_temp_edicts.integer;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
		//ZOID - make sure we update frags right
		svs.clients[i].old_frags = 0;
	}

	sv.time = 1.0;

	strcpy (sv.name, server);
	sprintf (sv.modelname,"maps/%s.bsp", server);
	sv.worldmodel = Mod_ForName (sv.modelname, true);
	SV_CalcPHS ();

	//
	// clear physics interaction links
	//
	SV_ClearWorld ();

	sv.sound_precache[0][0] = '\0';
	sv.model_precache[0][0] = '\0';
	Q_strlcpy (sv.model_precache[1], sv.modelname, sizeof(sv.model_precache[0]));
	sv.models[1] = sv.worldmodel;
	for (i = 1; i < sv.worldmodel->numsubmodels; i++)
	{
		Q_strlcpy (sv.model_precache[1+i], localmodels[i], sizeof(sv.model_precache[0]));
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

	//
	// spawn the rest of the entities on the map
	//

	// precache and static commands can be issued during
	// map initialization
	sv.state = ss_loading;

	ent = EDICT_NUM(0);
	ent->free = false;
	ent->v.model = PR_SetEngineString(sv.worldmodel->name);
	ent->v.modelindex = 1;	// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.integer)
		Cvar_SetValue ("deathmatch", 0);

	pr_global_struct->coop = coop.value;
	pr_global_struct->deathmatch = deathmatch.value;
	pr_global_struct->randomclass = randomclass.value;
	pr_global_struct->damageScale = damageScale.value;
	pr_global_struct->shyRespawn = shyRespawn.value;
	pr_global_struct->spartanPrint = spartanPrint.value;
	pr_global_struct->meleeDamScale = meleeDamScale.value;
	pr_global_struct->manaScale = manaScale.value;
	pr_global_struct->tomeMode = tomeMode.value;
	pr_global_struct->tomeRespawn = tomeRespawn.value;
	pr_global_struct->w2Respawn = w2Respawn.value;
	pr_global_struct->altRespawn = altRespawn.value;
	pr_global_struct->fixedLevel = fixedLevel.value;
	pr_global_struct->autoItems = autoItems.value;
	pr_global_struct->dmMode = dmMode.value;
	pr_global_struct->easyFourth = easyFourth.value;
	pr_global_struct->patternRunner = patternRunner.value;
	pr_global_struct->max_players = maxclients.value;

	pr_global_struct->startspot = PR_SetEngineString(sv.startspot);

	sv.current_skill = skill.integer;
	if (sv.current_skill < 0)
		sv.current_skill = 0;
	if (sv.current_skill > 3)
		sv.current_skill = 3;

	Cvar_SetValue ("skill", sv.current_skill);

	pr_global_struct->mapname = PR_SetEngineString(sv.name);
	// serverflags are for cross level information (sigils)
	pr_global_struct->serverflags = svs.serverflags;

	// run the frame start qc function to let progs check cvars
	SV_ProgStartFrame ();

	// load and spawn all other entities
	ED_LoadFromFile (sv.worldmodel->entities);

	// look up some model indexes for specialized message compression
	SV_FindModelNumbers ();

	// all spawning is completed, any further precache statements
	// or prog writes to the signon message are errors
	sv.state = ss_active;

	// run two frames to allow everything to settle
	host_frametime = HX_FRAME_TIME;
	SV_Physics ();
	SV_Physics ();

	// save movement vars
	SV_SetMoveVars();

	// create a baseline for more efficient communications
	SV_CreateBaseline ();
	sv.signon_buffer_size[sv.num_signon_buffers-1] = sv.signon.cursize;

	Info_SetValueForKey (svs.info, "map", sv.name, MAX_SERVERINFO_STRING);
	Con_DPrintf ("Server spawned.\n");

	svs.changelevel_issued = false;	// now safe to issue another
}

