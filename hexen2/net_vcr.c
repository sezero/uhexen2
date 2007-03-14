// net_vcr.c

#include "h2option.h"


#if NET_USE_VCR

#include "quakedef.h"
#include "net_vcr.h"

extern FILE *vcrFile;

// This is the playback portion of the VCR.  It reads the file produced
// by the recorder and plays it back to the host.  The recording contains
// everything necessary (events, timestamps, and data) to duplicate the game
// from the viewpoint of everything above the network layer.

static struct
{
	double	time;
	int		op;
	long	session;
}	next;

int VCR_Init (void)
{
	net_drivers[0].Init = VCR_Init;

	net_drivers[0].SearchForHosts = VCR_SearchForHosts;
	net_drivers[0].Connect = VCR_Connect;
	net_drivers[0].CheckNewConnections = VCR_CheckNewConnections;
	net_drivers[0].QGetMessage = VCR_GetMessage;
	net_drivers[0].QSendMessage = VCR_SendMessage;
	net_drivers[0].CanSendMessage = VCR_CanSendMessage;
	net_drivers[0].Close = VCR_Close;
	net_drivers[0].Shutdown = VCR_Shutdown;

	fread (&next, 1, sizeof(next), vcrFile);
	return 0;
}

static void VCR_ReadNext (void)
{
	if (fread(&next, 1, sizeof(next), vcrFile) == 0)
	{
		next.op = 255;
		Sys_Error ("=== END OF PLAYBACK===\n");
	}
	if (next.op < 1 || next.op > VCR_MAX_MESSAGE)
		Sys_Error ("%s: bad op", __FUNCTION__);
}


void VCR_Listen (qboolean state)
{
}


void VCR_Shutdown (void)
{
}


int VCR_GetMessage (qsocket_t *sock)
{
	int	ret;

	if (host_time != next.time || next.op != VCR_OP_GETMESSAGE || next.session != *(long *) (char *) (&sock->driverdata))
		Sys_Error ("VCR missmatch");

	fread (&ret, 1, sizeof(int), vcrFile);
	if (ret != 1)
	{
		VCR_ReadNext ();
		return ret;
	}

	fread (&net_message.cursize, 1, sizeof(int), vcrFile);
	fread (net_message.data, 1, net_message.cursize, vcrFile);

	VCR_ReadNext ();

	return 1;
}


int VCR_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	int	ret;

	if (host_time != next.time || next.op != VCR_OP_SENDMESSAGE || next.session != *(long *) (char *) (&sock->driverdata))
		Sys_Error ("VCR missmatch");

	fread (&ret, 1, sizeof(int), vcrFile);

	VCR_ReadNext ();

	return ret;
}


qboolean VCR_CanSendMessage (qsocket_t *sock)
{
	qboolean	ret;

	if (host_time != next.time || next.op != VCR_OP_CANSENDMESSAGE || next.session != *(long *) (char *) (&sock->driverdata))
		Sys_Error ("VCR missmatch");

	fread (&ret, 1, sizeof(int), vcrFile);

	VCR_ReadNext ();

	return ret;
}


void VCR_Close (qsocket_t *sock)
{
}


void VCR_SearchForHosts (qboolean xmit)
{
}


qsocket_t *VCR_Connect (char *host)
{
	return NULL;
}


qsocket_t *VCR_CheckNewConnections (void)
{
	qsocket_t	*sock;

	if (host_time != next.time || next.op != VCR_OP_CONNECT)
		Sys_Error ("VCR missmatch");

	if (!next.session)
	{
		VCR_ReadNext ();
		return NULL;
	}

	sock = NET_NewQSocket ();
	*(long *) (char *) (&sock->driverdata) = next.session;

	fread (sock->address, 1, NET_NAMELEN, vcrFile);
	VCR_ReadNext ();

	return sock;
}

#endif	// NET_USE_VCR

