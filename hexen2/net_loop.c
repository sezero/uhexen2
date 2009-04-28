/*
	net_loop.c
	network loop driver

	$Id: net_loop.c,v 1.9 2009-04-28 14:00:32 sezero Exp $
*/

#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "quakedef.h"
#include "net_defs.h"
#include "net_loop.h"

static qboolean	localconnectpending = false;
static qsocket_t	*loop_client = NULL;
static qsocket_t	*loop_server = NULL;

int Loop_Init (void)
{
	if (cls.state == ca_dedicated)
		return -1;
	return 0;
}


void Loop_Shutdown (void)
{
}


void Loop_Listen (qboolean state)
{
}


void Loop_SearchForHosts (qboolean xmit)
{
	if (!sv.active)
		return;

	hostCacheCount = 1;
	if (strcmp(hostname.string, "UNNAMED") == 0)
		strcpy(hostcache[0].name, "local");
	else
		strcpy(hostcache[0].name, hostname.string);
	strcpy(hostcache[0].map, sv.name);
	hostcache[0].users = net_activeconnections;
	hostcache[0].maxusers = svs.maxclients;
	hostcache[0].driver = net_driverlevel;
	strcpy(hostcache[0].cname, "local");
}


qsocket_t *Loop_Connect (const char *host)
{
	if (strcmp(host,"local") != 0)
		return NULL;

	localconnectpending = true;

	if (!loop_client)
	{
		if ((loop_client = NET_NewQSocket ()) == NULL)
		{
			Con_Printf("%s: no qsocket available\n", __thisfunc__);
			return NULL;
		}
		strcpy (loop_client->address, "localhost");
	}
	loop_client->receiveMessageLength = 0;
	loop_client->sendMessageLength = 0;
	loop_client->canSend = true;

	if (!loop_server)
	{
		if ((loop_server = NET_NewQSocket ()) == NULL)
		{
			Con_Printf("%s: no qsocket available\n", __thisfunc__);
			return NULL;
		}
		strcpy (loop_server->address, "LOCAL");
	}
	loop_server->receiveMessageLength = 0;
	loop_server->sendMessageLength = 0;
	loop_server->canSend = true;

	loop_client->driverdata = (void *)loop_server;
	loop_server->driverdata = (void *)loop_client;

	return loop_client;
}


qsocket_t *Loop_CheckNewConnections (void)
{
	if (!localconnectpending)
		return NULL;

	localconnectpending = false;
	loop_server->sendMessageLength = 0;
	loop_server->receiveMessageLength = 0;
	loop_server->canSend = true;
	loop_client->sendMessageLength = 0;
	loop_client->receiveMessageLength = 0;
	loop_client->canSend = true;
	return loop_server;
}


static int IntAlign(int value)
{
	return (value + (sizeof(int) - 1)) & (~(sizeof(int) - 1));
}


int Loop_GetMessage (qsocket_t *sock)
{
	int		ret;
	int		length;

	if (sock->receiveMessageLength == 0)
		return 0;

	ret = sock->receiveMessage[0];
	length = sock->receiveMessage[1] + (sock->receiveMessage[2] << 8);
	// alignment byte skipped here
	SZ_Clear (&net_message);
	SZ_Write (&net_message, &sock->receiveMessage[4], length);

	length = IntAlign(length + 4);
	sock->receiveMessageLength -= length;

	if (sock->receiveMessageLength)
		memmove (sock->receiveMessage, &sock->receiveMessage[length], sock->receiveMessageLength);

	if (sock->driverdata && ret == 1)
		((qsocket_t *)sock->driverdata)->canSend = true;

	return ret;
}


int Loop_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	byte *buffer;
	int  *bufferLength;

	if (!sock->driverdata)
		return -1;

	bufferLength = &((qsocket_t *)sock->driverdata)->receiveMessageLength;

	if ((*bufferLength + data->cursize + 4) > NET_MAXMESSAGE)
		Sys_Error("%s: overflow", __thisfunc__);

	buffer = ((qsocket_t *)sock->driverdata)->receiveMessage + *bufferLength;

	// message type
	*buffer++ = 1;

	// length
	*buffer++ = data->cursize & 0xff;
	*buffer++ = data->cursize >> 8;

	// align
	buffer++;

	// message
	memcpy(buffer, data->data, data->cursize);
	*bufferLength = IntAlign(*bufferLength + data->cursize + 4);

	sock->canSend = false;
	return 1;
}


int Loop_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data)
{
	byte *buffer;
	int  *bufferLength;

	if (!sock->driverdata)
		return -1;

	bufferLength = &((qsocket_t *)sock->driverdata)->receiveMessageLength;

	if ((*bufferLength + data->cursize + sizeof(byte) + sizeof(short)) > NET_MAXMESSAGE)
		return 0;

	buffer = ((qsocket_t *)sock->driverdata)->receiveMessage + *bufferLength;

	// message type
	*buffer++ = 2;

	// length
	*buffer++ = data->cursize & 0xff;
	*buffer++ = data->cursize >> 8;

	// align
	buffer++;

	// message
	memcpy(buffer, data->data, data->cursize);
	*bufferLength = IntAlign(*bufferLength + data->cursize + 4);
	return 1;
}


qboolean Loop_CanSendMessage (qsocket_t *sock)
{
	if (!sock->driverdata)
		return false;
	return sock->canSend;
}


qboolean Loop_CanSendUnreliableMessage (qsocket_t *sock)
{
	return true;
}


void Loop_Close (qsocket_t *sock)
{
	if (sock->driverdata)
		((qsocket_t *)sock->driverdata)->driverdata = NULL;
	sock->receiveMessageLength = 0;
	sock->sendMessageLength = 0;
	sock->canSend = true;
	if (sock == loop_client)
		loop_client = NULL;
	else
		loop_server = NULL;
}

