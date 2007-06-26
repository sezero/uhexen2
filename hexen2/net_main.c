/*
	net_main.c
	main networking module

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/net_main.c,v 1.24 2007-06-26 13:37:19 sezero Exp $
*/

#include "quakedef.h"

qsocket_t	*net_activeSockets = NULL;
qsocket_t	*net_freeSockets = NULL;
int			net_numsockets = 0;

qboolean	ipxAvailable = false;
qboolean	tcpipAvailable = false;

int			net_hostport;
int			DEFAULTnet_hostport = 26900;

char		my_ipx_address[NET_NAMELEN];
char		my_tcpip_address[NET_NAMELEN];

static qboolean	listening = false;

qboolean	slistInProgress = false;
qboolean	slistSilent = false;
qboolean	slistLocal = true;
static double	slistStartTime;
static int		slistLastShown;

static void Slist_Send(void);
static void Slist_Poll(void);
static PollProcedure	slistSendProcedure = {NULL, 0.0, Slist_Send};
static PollProcedure	slistPollProcedure = {NULL, 0.0, Slist_Poll};

sizebuf_t		net_message;
static byte		net_message_buffer[NET_MAXMESSAGE];
int			net_activeconnections = 0;

int messagesSent = 0;
int messagesReceived = 0;
int unreliableMessagesSent = 0;
int unreliableMessagesReceived = 0;

static	cvar_t	net_messagetimeout = {"net_messagetimeout", "300", CVAR_NONE};
cvar_t	hostname = {"hostname", "UNNAMED", CVAR_NONE};

static qboolean	configRestored = false;

cvar_t	net_allowmultiple = {"net_allowmultiple", "0", CVAR_ARCHIVE};

// these two macros are to make the code more readable
#define sfunc	net_drivers[sock->driver]
#define dfunc	net_drivers[net_driverlevel]

/* NOTE: several sock->driver checks in the code serve the
   purpose of ignoring local connections, because the loop
   driver always takes number 0: it is the first member in
   the net_drivers[] array.  If you ever change that, such
   as by removing the loop driver, you must re-visit those
   checks and adjust them properly!.			*/

int	net_driverlevel;

double			net_time;

double SetNetTime(void)
{
	net_time = Sys_DoubleTime();
	return net_time;
}


/*
===================
NET_NewQSocket

Called by drivers when a new communications endpoint is required
The sequence and buffer fields will be filled in properly
===================
*/
qsocket_t *NET_NewQSocket (void)
{
	qsocket_t	*sock;

	if (net_freeSockets == NULL)
		return NULL;

	if (net_activeconnections >= svs.maxclients)
		return NULL;

	// get one from free list
	sock = net_freeSockets;
	net_freeSockets = sock->next;

	// add it to active list
	sock->next = net_activeSockets;
	net_activeSockets = sock;

	sock->disconnected = false;
	sock->connecttime = net_time;
	strcpy (sock->address,"UNSET ADDRESS");
	sock->driver = net_driverlevel;
	sock->socket = 0;
	sock->driverdata = NULL;
	sock->canSend = true;
	sock->sendNext = false;
	sock->lastMessageTime = net_time;
	sock->ackSequence = 0;
	sock->sendSequence = 0;
	sock->unreliableSendSequence = 0;
	sock->sendMessageLength = 0;
	sock->receiveSequence = 0;
	sock->unreliableReceiveSequence = 0;
	sock->receiveMessageLength = 0;

	return sock;
}


void NET_FreeQSocket(qsocket_t *sock)
{
	qsocket_t	*s;

	// remove it from active list
	if (sock == net_activeSockets)
		net_activeSockets = net_activeSockets->next;
	else
	{
		for (s = net_activeSockets; s; s = s->next)
		{
			if (s->next == sock)
			{
				s->next = sock->next;
				break;
			}
		}

		if (!s)
			Sys_Error ("%s: not active", __thisfunc__);
	}

	// add it to free list
	sock->next = net_freeSockets;
	net_freeSockets = sock;
	sock->disconnected = true;
}


static void NET_Listen_f (void)
{
	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"listen\" is \"%d\"\n", listening ? 1 : 0);
		return;
	}

	listening = atoi(Cmd_Argv(1)) ? true : false;

	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.Listen (listening);
	}
}


static void MaxPlayers_f (void)
{
	int	n;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"maxplayers\" is \"%d\"\n", svs.maxclients);
		return;
	}

	if (sv.active)
	{
		Con_Printf ("maxplayers can not be changed while a server is running.\n");
		return;
	}

	n = atoi(Cmd_Argv(1));
	if (n < 1)
		n = 1;
	if (n > svs.maxclientslimit)
	{
		n = svs.maxclientslimit;
		Con_Printf ("\"maxplayers\" set to \"%d\"\n", n);
	}

	if ((n == 1) && listening)
		Cbuf_AddText ("listen 0\n");

	if ((n > 1) && (!listening))
		Cbuf_AddText ("listen 1\n");

	svs.maxclients = n;
	if (n == 1)
	{
		Cvar_SetValue ("deathmatch", 0);
		Cvar_SetValue ("coop", 0);
	}
	else
	{
		if (coop.integer)
			Cvar_SetValue ("deathmatch", 0);
		else
			Cvar_SetValue ("deathmatch", 1);
	}
}


static void NET_Port_f (void)
{
	int	n;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"port\" is \"%d\"\n", net_hostport);
		return;
	}

	n = atoi(Cmd_Argv(1));
	if (n < 1 || n > 65534)
	{
		Con_Printf ("Bad value, must be between 1 and 65534\n");
		return;
	}

	DEFAULTnet_hostport = n;
	net_hostport = n;

	if (listening)
	{
		// force a change to the new port
		Cbuf_AddText ("listen 0\n");
		Cbuf_AddText ("listen 1\n");
	}
}


static void PrintSlistHeader(void)
{
	Con_Printf("Server      Connect  Map         Users\n");
	Con_Printf("----------- -------- ----------- -----\n");
	slistLastShown = 0;
}


static void PrintSlist(void)
{
	int		n;
	char	*name;

	for (n = slistLastShown; n < hostCacheCount; n++)
	{
		if (hostcache[n].driver == 0)
			name = net_drivers[hostcache[n].driver].name;
		else
			name = net_landrivers[hostcache[n].ldriver].name;

		if (hostcache[n].maxusers)
			Con_Printf("%-11.11s %-8.8s %-10.10s %2u/%2u\n", hostcache[n].name, name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			Con_Printf("%-11.11s %-8.8s %-10.10s\n", hostcache[n].name, name, hostcache[n].map);
	}
	slistLastShown = n;
}


static void PrintSlistTrailer(void)
{
	if (hostCacheCount)
		Con_Printf("== end list ==\n\n");
	else
		Con_Printf("No Hexen II servers found.\n\n");
}


void NET_Slist_f (void)
{
	if (slistInProgress)
		return;

	if (! slistSilent)
	{
		Con_Printf("Looking for Hexen II servers...\n");
		PrintSlistHeader();
	}

	slistInProgress = true;
	slistStartTime = Sys_DoubleTime();

	SchedulePollProcedure(&slistSendProcedure, 0.0);
	SchedulePollProcedure(&slistPollProcedure, 0.1);

	hostCacheCount = 0;
}


static void Slist_Send(void)
{
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && net_driverlevel == 0)
			continue;
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.SearchForHosts (true);
	}

	if ((Sys_DoubleTime() - slistStartTime) < 0.5)
		SchedulePollProcedure(&slistSendProcedure, 0.75);
}


static void Slist_Poll(void)
{
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && net_driverlevel == 0)
			continue;
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.SearchForHosts (false);
	}

	if (! slistSilent)
		PrintSlist();

	if ((Sys_DoubleTime() - slistStartTime) < 1.5)
	{
		SchedulePollProcedure(&slistPollProcedure, 0.1);
		return;
	}

	if (! slistSilent)
		PrintSlistTrailer();
	slistInProgress = false;
	slistSilent = false;
	slistLocal = true;
}


/*
===================
NET_Connect
===================
*/

int hostCacheCount = 0;
hostcache_t hostcache[HOSTCACHESIZE];

qsocket_t *NET_Connect (char *host)
{
	qsocket_t		*ret;
	int				n;

	SetNetTime();

	if (host && *host == 0)
		host = NULL;

	if (host && hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (Q_strcasecmp (host, hostcache[n].name) == 0)
			{
				host = hostcache[n].cname;
				break;
			}
		}

		if (n < hostCacheCount)
			goto JustDoIt;
	}

	slistSilent = host ? true : false;
	NET_Slist_f ();

	while (slistInProgress)
		NET_Poll();

	if (host == NULL)
	{
		if (hostCacheCount != 1)
			return NULL;
		host = hostcache[0].cname;
		Con_Printf("Connecting to...\n%s @ %s\n\n", hostcache[0].name, host);
	}

	if (hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (Q_strcasecmp (host, hostcache[n].name) == 0)
			{
				host = hostcache[n].cname;
				break;
			}
		}
	}

JustDoIt:
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		ret = dfunc.Connect (host);
		if (ret)
			return ret;
	}

	if (host)
	{
		Con_Printf("\n");
		PrintSlistHeader();
		PrintSlist();
		PrintSlistTrailer();
	}

	return NULL;
}


/*
===================
NET_CheckNewConnections
===================
*/
qsocket_t *NET_CheckNewConnections (void)
{
	qsocket_t	*ret;

	SetNetTime();

	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		if (net_driverlevel && listening == false)
			continue;
		ret = dfunc.CheckNewConnections ();
		if (ret)
		{
			return ret;
		}
	}

	return NULL;
}

/*
===================
NET_Close
===================
*/
void NET_Close (qsocket_t *sock)
{
	if (!sock)
		return;

	if (sock->disconnected)
		return;

	SetNetTime();

	// call the driver_Close function
	sfunc.Close (sock);

	NET_FreeQSocket(sock);
}


/*
=================
NET_GetMessage

If there is a complete message, return it in net_message

returns 0 if no data is waiting
returns 1 if a message was received
returns -1 if connection is invalid
=================
*/
int	NET_GetMessage (qsocket_t *sock)
{
	int ret;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("%s: disconnected socket\n", __thisfunc__);
		return -1;
	}

	SetNetTime();

	ret = sfunc.QGetMessage(sock);

	// see if this connection has timed out
	if (ret == 0 && sock->driver)
	{
		if (net_time - sock->lastMessageTime > net_messagetimeout.value)
		{
			NET_Close(sock);
			return -1;
		}
	}

	if (ret > 0)
	{
		if (sock->driver)
		{
			sock->lastMessageTime = net_time;
			if (ret == 1)
				messagesReceived++;
			else if (ret == 2)
				unreliableMessagesReceived++;
		}
	}

	return ret;
}


/*
==================
NET_SendMessage

Try to send a complete length+message unit over the reliable stream.
returns 0 if the message cannot be delivered reliably, but the connection
		is still considered valid
returns 1 if the message was sent properly
returns -1 if the connection died
==================
*/
int NET_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("%s: disconnected socket\n", __thisfunc__);
		return -1;
	}

	SetNetTime();
	r = sfunc.QSendMessage(sock, data);
	if (r == 1 && sock->driver)
		messagesSent++;

	return r;
}


int NET_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("%s: disconnected socket\n", __thisfunc__);
		return -1;
	}

	SetNetTime();
	r = sfunc.SendUnreliableMessage(sock, data);
	if (r == 1 && sock->driver)
		unreliableMessagesSent++;

	return r;
}


/*
==================
NET_CanSendMessage

Returns true or false if the given qsocket can currently accept a
message to be transmitted.
==================
*/
qboolean NET_CanSendMessage (qsocket_t *sock)
{
	if (!sock)
		return false;

	if (sock->disconnected)
		return false;

	SetNetTime();

	return sfunc.CanSendMessage(sock);
}


int NET_SendToAll(sizebuf_t *data, int blocktime)
{
	double		start;
	int			i;
	int			count = 0;
	qboolean	state1 [MAX_CLIENTS];
	qboolean	state2 [MAX_CLIENTS];

	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (!host_client->netconnection)
			continue;
		if (host_client->active)
		{
			if (host_client->netconnection->driver == 0) // Loopback
			{
				NET_SendMessage(host_client->netconnection, data);
				state1[i] = true;
				state2[i] = true;
				continue;
			}
			count++;
			state1[i] = false;
			state2[i] = false;
		}
		else
		{
			state1[i] = true;
			state2[i] = true;
		}
	}

	start = Sys_DoubleTime();
	while (count)
	{
		count = 0;
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (! state1[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					state1[i] = true;
					NET_SendMessage(host_client->netconnection, data);
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}

			if (! state2[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					state2[i] = true;
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}
		}
		if ((Sys_DoubleTime() - start) > blocktime)
			break;
	}
	return count;
}


//=============================================================================

/*
====================
NET_Init
====================
*/

void NET_Init (void)
{
	int			i;
	int			controlSocket;
	qsocket_t	*s;

	i = COM_CheckParm ("-port");
	if (!i)
		i = COM_CheckParm ("-udpport");
	if (!i)
		i = COM_CheckParm ("-ipxport");

	if (i)
	{
		if (i < com_argc-1)
			DEFAULTnet_hostport = atoi (com_argv[i+1]);
		else
			Con_Printf ("%s: ignoring -port argument\n", __thisfunc__);
	}
	net_hostport = DEFAULTnet_hostport;

	if (COM_CheckParm("-listen") || cls.state == ca_dedicated)
		listening = true;
	net_numsockets = svs.maxclientslimit;
	if (cls.state != ca_dedicated)
		net_numsockets++;

	SetNetTime();

	for (i = 0; i < net_numsockets; i++)
	{
		s = (qsocket_t *)Hunk_AllocName(sizeof(qsocket_t), "qsocket");
		s->next = net_freeSockets;
		net_freeSockets = s;
		s->disconnected = true;
	}

	// allocate space for network message buffer
	SZ_Init (&net_message, net_message_buffer, sizeof(net_message_buffer));

	Cvar_RegisterVariable (&net_messagetimeout);
	Cvar_RegisterVariable (&hostname);
	Cvar_RegisterVariable (&net_allowmultiple);

	Cmd_AddCommand ("slist", NET_Slist_f);
	Cmd_AddCommand ("listen", NET_Listen_f);
	Cmd_AddCommand ("maxplayers", MaxPlayers_f);
	Cmd_AddCommand ("port", NET_Port_f);

	// initialize all the drivers
	for (i = net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		controlSocket = net_drivers[net_driverlevel].Init();
		if (controlSocket == -1)
			continue;
		i++;
		net_drivers[net_driverlevel].initialized = true;
		net_drivers[net_driverlevel].controlSock = controlSocket;
		if (listening)
			net_drivers[net_driverlevel].Listen (true);
	}

	if (i == 0 && cls.state == ca_dedicated)
		Sys_Error("Network not available!");

	if (*my_ipx_address)
		Con_DPrintf("IPX address %s\n", my_ipx_address);
	if (*my_tcpip_address)
		Con_DPrintf("TCP/IP address %s\n", my_tcpip_address);
}

/*
====================
NET_Shutdown
====================
*/

void NET_Shutdown (void)
{
	qsocket_t	*sock;

	SetNetTime();

	for (sock = net_activeSockets; sock; sock = sock->next)
		NET_Close(sock);

//
// shutdown the drivers
//
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == true)
		{
			net_drivers[net_driverlevel].Shutdown ();
			net_drivers[net_driverlevel].initialized = false;
		}
	}
}


static PollProcedure *pollProcedureList = NULL;

void NET_Poll(void)
{
	PollProcedure *pp;

	if (!configRestored)
	{
		configRestored = true;
	}

	SetNetTime();

	for (pp = pollProcedureList; pp; pp = pp->next)
	{
		if (pp->nextTime > net_time)
			break;
		pollProcedureList = pp->next;
		pp->procedure(pp->arg);
	}
}


void SchedulePollProcedure(PollProcedure *proc, double timeOffset)
{
	PollProcedure *pp, *prev;

	proc->nextTime = Sys_DoubleTime() + timeOffset;
	for (pp = pollProcedureList, prev = NULL; pp; pp = pp->next)
	{
		if (pp->nextTime >= proc->nextTime)
			break;
		prev = pp;
	}

	if (prev == NULL)
	{
		proc->next = pollProcedureList;
		pollProcedureList = proc;
		return;
	}

	proc->next = pp;
	prev->next = proc;
}

