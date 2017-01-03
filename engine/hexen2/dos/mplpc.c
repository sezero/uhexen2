/*
 * mplpc.c -- support library for dosquake MPATH network driver.
 * from quake1 source with minor adaptations for uhexen2.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "mplib.c"

#include <go32.h>
#include "mpdosock.h"

#include "compiler.h"
FUNC_NORETURN extern void Sys_Error (const char *error, ...) FUNC_PRINTF(1,2);

/*#include "lpc.h"*/
typedef struct {
	short	version;	/* version of LPC requested */
	short	sizeOfArgs;	/* size of arguments */
	short	service;	/* service # requested */
	char	Data[1];	/* data */
} LPCData;

typedef struct {
	short	version;	/* LPC version */
	short	sizeOfReturn;	/* return data size */
	short	error;		/* any error codes */
	short	noRet;		/* number of returns */
	char	Data[1];	/* data */
} LPCReturn;

/*#include "services.h"*/
#define	MAXSOCKETS		20

/* services */
#define	LPC_SOCKBIND		4
#define	LPC_SOCKGETHOSTBYNAME	5
#define	LPC_SOCKGETHOSTNAME	6
#define	LPC_SOCKGETHOSTBYADDR	7
#define	LPC_SOCKCLOSE		8
#define	LPC_SOCKSOCKET		9
#define	LPC_SOCKRECVFROM	10
#define	LPC_SOCKSENDTO		11
#define	LPC_SOCKIOCTL		12
#define	LPC_SOCKGETSOCKNAME	13
#define	LPC_SOCKFLUSH		14
#define	LPC_SOCKSETOPT		15
#define	LPC_SOCKGETLASTERROR	16
#define	LPC_SOCKINETADDR	17

/* htons, ntohs, htonl, ntohl implemented locally */

/* errors */
#define LPC_UNRECOGNIZED_SERVICE	(-1)
#define LPC_NOERROR			0

/* structures for support */
typedef struct {
	SOCKET		s;
	int	namelen;
	char	name[1];
} BindArgs;

typedef struct {
	SOCKET		s;
	long	cmd;
	char	data[1];
} IoctlArgs;

typedef struct {
	int	retVal;
	int	namelen;
	char	name[1];
} GetSockNameRet;

typedef GetSockNameRet	GetHostNameRet;

typedef struct {
	int	retVal;
	int	h_addr_0;	/* that's the only important value */
} GetHostByNameRet;

typedef struct {
	int	len;
	int	type;
	char	addr[1];
} GetHostByAddrArgs;

typedef struct {
	int	retVal;
	char	h_name[1];	/* h_name is the only important value */
} GetHostByAddrRet;

typedef struct {
	SOCKET		s;
	int	flags;
} RecvFromArgs;

typedef struct {
	int	retVal;
	int	errCode;
	int	len; /* message len */
	struct sockaddr	sockaddr;
	int	sockaddrlen;
	char	Data[1];
} RecvFromRet;

typedef struct {
	SOCKET		s;
	int	flags;
	int	len;
	struct sockaddr	sockaddr;
	int	sockaddrlen;
	char	Data[1];
} SendToArgs;

typedef struct {
	int	retVal;
	int	errCode;
} SendToRet;

typedef struct {
	int	bufflen;
	SOCKET		s;
	int	len;
	int	sockaddrlen;
	struct sockaddr	address;
	char	data[1];
} SocketChannelData;

typedef struct {
	int	af;
	int	type;
	int	protocol;
} SocketArgs;

typedef struct {
	SOCKET		s;
	int	len;
	int	flags;
	int	addrlen;
	struct sockaddr	addr;
	char	data[1];
} WinSockData;

typedef struct {
	SOCKET		s;
	int	level;
	int	optname;
	int	optlen;
	char	optval[1];
} SetSockOptArgs;

typedef struct {
	SOCKET	sock[MAXSOCKETS];
} SocketMap;

#include <stdio.h>
#include <sys/farptr.h>

extern short	flat_selector;

#define	SOCKET_MAP_QUEUE	41

#define	IDLE_QUEUE		44
#define	REC_QUEUE		45
#define	SEND_QUEUE		46

/*  queue sizes */
#define	FREEQBASE		58
#define	FREEQ64			58
#define	FREEQ128		59
#define	FREEQ256		60
#define	FREEQ512		61
#define	FREEQ1024		62
#define	FREEQ2048		63

#define	NFREEQ			6

#define	QLIMIT			10

#define	PRIVATEQ		50

#define	FARPKL(x)	(_farnspeekl((unsigned long) (x)))
#define	FARPKB(x)	(_farnspeekb((unsigned long) (x)))
#define	FARPKS(x)	(_farnspeekw((unsigned long) (x)))

#define	FARPOKL(x, y)	(_farnspokel((unsigned long) (x), (unsigned long) (y)))
#define	FARPOKB(x, y)	(_farnspokeb((unsigned long) (x), (unsigned char) (y)))

static int	Qsizes[] = { 64, 128, 256, 512, 1024, 2048 };

static int	SocketError = 0;

static SocketMap	*SockMap;

#define	HOSTENT_ALIAS_LIMIT	5
#define	HOSTENT_STRLEN_LIMIT	50
#define	HOSTENT_ADDR_LIST_LIMIT	5

static struct hostent	HostEnt;

static char	HostEnt_hname[HOSTENT_STRLEN_LIMIT];
static char	*HostEnt_h_aliases[HOSTENT_ALIAS_LIMIT];
static char	HostEnt_names[HOSTENT_ALIAS_LIMIT][HOSTENT_STRLEN_LIMIT];
static struct in_addr	*HostEnt_addr_list[HOSTENT_ADDR_LIST_LIMIT];
static struct in_addr	HostEnt_addrs[HOSTENT_ADDR_LIST_LIMIT];

static void fmemcpyto (void *to, const void *from, int length)
{
	movedata(_my_ds(), (unsigned)from, flat_selector, (unsigned)to, length);
}

static void fmemcpyfrom (void *to, const void *from, int length)
{
	movedata(flat_selector, (unsigned)from, _my_ds(), (unsigned)to, length);
}

static void fstrcpyto (char *to, const char *from)
{
	while (*from)
	{
		FARPOKB(to, *from);
		to++;
		from++;
	}
	FARPOKB(to, 0);
}

#if 0	/* not used */
static void fstrncpyto (char *to, const char *from, int len)
{
	while (*from && len)
	{
		FARPOKB(to, *from);
		to++;
		from++;
		len--;
	}
	FARPOKB(to, 0);
}

static void fstrcpyfrom (char *to, const char *from)
{
	while (FARPKB(from))
	{
		*to = FARPKB(from);
		from++;
		to++;
	}
	*to = 0;
}
#endif	/* end of  unused */

static void fstrncpyfrom (char *to, const char *from, int len)
{
	while (FARPKB(from) && len)
	{
		*to = FARPKB(from);
		from++;
		to++;
		len--;
	}
	*to = 0;
}

static void GetSocketMap (void)
{
	RTQ_NODE *n = MGenGetNode(SOCKET_MAP_QUEUE);

	SockMap = (SocketMap *) FARPKL(&n->rtqDatum);
}

static void *GetFreeBufferToQueue (int q, int bufSize)
{
	int		i;

	for (i = 0; i < NFREEQ; i++)
	{
		if (Qsizes[i] >= bufSize && MGenGetQueueCtr(i + FREEQBASE))
		{
			RTQ_NODE *n = MGenMoveTo(i + FREEQBASE, q);
			if (!n)
				continue;
			FARPOKL(&n->rtqUpCtr, bufSize);
			return (void *) FARPKL(&n->rtqDatum);
		}
	}

	return NULL;
}

static void FreeBufferFromQueue (int q)
{
	int		i;
	RTQ_NODE	*n = MGenGetNode(q);

	for (i = 0; i < NFREEQ; i++)
	{
		if (Qsizes[i] == FARPKS(&n->rtqLen))
		{
			MGenMoveTo(q, i + FREEQBASE);
			return;
		}
	}
}

static void SetLPCData (LPCData *lpc)
{
	FARPOKL(&(lpc->version), 1);
	FARPOKL(&(lpc->sizeOfArgs), 0);
	FARPOKL(&(lpc->service), 0);
}

int WSLIB_bind (SOCKET s, const struct sockaddr *name, int namelen)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	BindArgs	*bargs;
	int		retVal;

	_farsetsel(flat_selector);
	SocketError = 0;
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKBIND);
	bargs = (BindArgs *) p->Data;
	FARPOKL(&bargs->s, s);
	FARPOKL(&bargs->namelen, namelen);
	fmemcpyto(bargs->name, name, namelen);
	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
	{
		return -1;
	}

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

int WSLIB_closesocket (SOCKET s)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	int		retVal;

	_farsetsel(flat_selector);
	SocketError = 0;
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKCLOSE);
	FARPOKL(p->Data, s);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
		return -1;

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

static void ZapHostEnt (void)
{
	/* do nothing */
}

static void ReconstructHostEnt (struct hostent *s, void *flattened)
{
	struct hostent	*old = (struct hostent *) flattened;
	int		i;
	char	**ptr;

	s->h_name = HostEnt_hname;
	fstrncpyfrom(s->h_name, (char *) FARPKL(&old->h_name), HOSTENT_STRLEN_LIMIT-1);
	s->h_name[HOSTENT_STRLEN_LIMIT-1] = 0;

	s->h_aliases = HostEnt_h_aliases;
	ptr = (char **) FARPKL(&old->h_aliases);
	for (i = 0; i < (HOSTENT_ALIAS_LIMIT-1) && FARPKL(ptr); i++, ptr++)
	{
		s->h_aliases[i] = HostEnt_names[i];
	/*	fstrncpyfrom(s->h_aliases[i], (void *) FARPKL(ptr), HOSTENT_STRLEN_LIMIT-1);*/
		s->h_aliases[i][HOSTENT_STRLEN_LIMIT-1] = 0;
	}
	s->h_aliases[i] = 0;

	s->h_addrtype = FARPKS(&old->h_addrtype);
	s->h_length = FARPKS(&old->h_length);

	if (FARPKS(&old->h_length) != sizeof(struct in_addr))
	{
		Sys_Error ("MPATH: h_length != sizeof(struct in_addr)");
	}

	s->h_addr_list = (char **) HostEnt_addr_list;
	ptr = (char **) FARPKL(&old->h_addr_list);
	for (i = 0; i < (HOSTENT_ADDR_LIST_LIMIT - 1) && FARPKL(ptr); i++, ptr++)
	{
		s->h_addr_list[i] = (char *) &(HostEnt_addrs[i]);
		fmemcpyfrom(s->h_addr_list[i], (void *) FARPKL(ptr), s->h_length);
	}
	s->h_addr_list[i] = 0;
}

int WSLIB_getsockname (SOCKET s, struct sockaddr *name, int *namelen)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	GetSockNameRet	*ret;
	int		retVal;

	SocketError = 0;
	_farsetsel(flat_selector);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKGETSOCKNAME);
	FARPOKL(p->Data, s);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
		return -1;

	ret = (GetSockNameRet *) r->Data;
	retVal = FARPKL(&ret->retVal);
	fmemcpyfrom(name, ret->name, FARPKL(&ret->namelen));
	*namelen = FARPKL(&ret->namelen);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

int WSLIB_gethostname (char *name, int namelen)
{
	RTQ_NODE	*n;
	LPCData		*p;
	LPCReturn	*r;
	GetHostNameRet	*ret;
	int		retVal;
	char	*s;

	_farsetsel(flat_selector);
	SocketError = 0;
	n = (RTQ_NODE *) MGenGetNode(IDLE_QUEUE);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service,LPC_SOCKGETHOSTNAME);
	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = (RTQ_NODE *) (MGenGetNode(REC_QUEUE))) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
		return -1;

	ret = (GetHostNameRet *) r->Data;

	retVal = FARPKL(&ret->retVal);

	s = ret->name;

	fstrncpyfrom(name, s, namelen);

#if 0
	len = strlen(ret->name);

	if (len > namelen)
		memcpy(name, ret->name, ret->namelen);
	else
		strcpy(name, ret->name);
#endif

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

struct hostent *WSLIB_gethostbyname (const char *name)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	struct hostent	*retVal;

	_farsetsel(flat_selector);
	SocketError = 0;
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKGETHOSTBYNAME);
	fstrcpyto(p->Data, name);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);
	retVal = (struct hostent *) r->Data;

	if (FARPKL(&retVal->h_name) == 0)
	{
		retVal = 0;
	}
	else
	{
		ZapHostEnt();
		ReconstructHostEnt(&HostEnt, (void *) retVal);
		retVal = &HostEnt;
	}

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

struct hostent *WSLIB_gethostbyaddr (const char *addr, int len, int type)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	GetHostByAddrArgs	*args;
	struct hostent		*retVal;

	SocketError = 0;
	_farsetsel(flat_selector);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKGETHOSTBYADDR);
	args = (GetHostByAddrArgs *) p->Data;
	FARPOKL(&args->len, len);
	FARPOKL(&args->type, type);
	fmemcpyto(args->addr, addr, len);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();
	r = (LPCReturn *) FARPKL(&n->rtqDatum);
	retVal = (struct hostent *) r->Data;

	if (FARPKL(&retVal->h_name) == 0)
	{
		retVal = 0;
	}
	else
	{
		ZapHostEnt();

		ReconstructHostEnt(&HostEnt, (void *) retVal);
		retVal = &HostEnt;
	}

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

SOCKET WSLIB_socket (int af, int type, int protocol)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	SocketArgs	*args;
	SOCKET		retVal;

	_farsetsel(flat_selector);
	SocketError = 0;
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKSOCKET);
	args = (SocketArgs *) p->Data;
	FARPOKL(&args->af, af);
	FARPOKL(&args->type, type);
	FARPOKL(&args->protocol, protocol);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
		return INVALID_SOCKET;

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

void WSLIB_sockets_flush (void)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;

	SocketError = 0;
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKFLUSH);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);
}

int WSLIB_recvfrom (SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
	int		i;
	RTQ_NODE	*n;
	WinSockData	*data;
	int	bytesRead;

	SocketError = 0;
	_farsetsel(flat_selector);
	if (!SockMap)
		GetSocketMap();

	for (i = 0; i < MAXSOCKETS; i++)
	{
		if (FARPKL(&(SockMap->sock[i])) == s)
			break;
	}

	if (i == MAXSOCKETS)
		return SOCKET_ERROR;

	/* pick up node */
	n = MGenGetNode(i);
	if (n == 0)
	{
		SocketError = WSAEWOULDBLOCK;
		return -1;
	}

	data = (WinSockData *) FARPKL(&n->rtqDatum);
	bytesRead = FARPKL(&data->len);

	if (from)
	{
		fmemcpyfrom(from, &data->addr, sizeof(struct sockaddr));
	}

	if (fromlen)
	{
		*fromlen = FARPKL(&data->addrlen);
	}

	fmemcpyfrom(buf, data->data, len > bytesRead ? bytesRead : len);

	if ((flags & MSG_PEEK) == 0)
	{
		FreeBufferFromQueue(i);
	}

	return bytesRead;
}

int WSLIB_sendto (SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
	int		i;
	int		outQ;
	WinSockData	*data;

	SocketError = 0;
	_farsetsel(flat_selector);
	if (!SockMap)
		GetSocketMap();

	for (i = 0; i < MAXSOCKETS; i++)
	{
		if (FARPKL(&SockMap->sock[i]) == s)
			break;
	}

	if (i == MAXSOCKETS)
	{
		SocketError = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	outQ = i + MAXSOCKETS;

	if (MGenGetQueueCtr(outQ) >= QLIMIT)
	{
		SocketError = WSAEWOULDBLOCK;
		return SOCKET_ERROR;
	}

	data = (WinSockData *) GetFreeBufferToQueue(PRIVATEQ, len + sizeof(WinSockData));

	if (!data)
	{
		SocketError = WSAEWOULDBLOCK;
		return SOCKET_ERROR;
	}

	FARPOKL(&data->s, s);
	FARPOKL(&data->len, len);
	if (to)
	{
		fmemcpyto(&data->addr, to, tolen);
		FARPOKL(&data->addrlen, tolen);
	}
	else
	{
		FARPOKL(&data->addrlen, 0);
	}

	FARPOKL(&data->flags, flags);

	fmemcpyto(data->data, buf, len);

	MGenMoveTo(PRIVATEQ, outQ);

	return len;
}

int WSLIB_ioctlsocket (SOCKET s, long cmd, unsigned long *argp)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	IoctlArgs	*args;
	int		retVal;

	SocketError = 0;
	_farsetsel(flat_selector);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKIOCTL);
	args = (IoctlArgs *) p->Data;
	FARPOKL(&args->s, s);
	FARPOKL(&args->cmd, cmd);

	switch (cmd)
	{
	case FIONBIO:
		FARPOKL(args->data, *argp);
		break;
	default:
		return SOCKET_ERROR;
	}

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

int WSLIB_setsockopt (SOCKET s, int level, int optname, const char *optval, int optlen)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	SetSockOptArgs	*args;
	int		retVal;

	SocketError = 0;
	_farsetsel(flat_selector);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKSETOPT);
	args = (SetSockOptArgs *) p->Data;
	FARPOKL(&args->s, s);
	FARPOKL(&args->level, level);
	FARPOKL(&args->optname, optname);
	FARPOKL(&args->optlen, optlen);
	fmemcpyto(args->optval, optval, optlen);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

int WSLIB_WSAGetLastError (void)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	int		retVal;

	_farsetsel(flat_selector);
	if (SocketError)
	{
		int	err = SocketError;

		SocketError = 0;
		return err;
	}

	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKGETLASTERROR);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

#if 0	/* implemented in dos_inet.c */
unsigned long WSLIB_inet_addr (const char *cp)
{
	RTQ_NODE	*n = MGenGetNode(IDLE_QUEUE);
	LPCData		*p;
	LPCReturn	*r;
	int		retVal;

	SocketError = 0;
	_farsetsel(flat_selector);
	p = (LPCData *) FARPKL(&n->rtqDatum);
	SetLPCData(p);
	FARPOKL(&p->service, LPC_SOCKINETADDR);

	fstrcpyto(p->Data, cp);

	MGenMoveTo(IDLE_QUEUE, SEND_QUEUE);
	PostWindowsMessage();

	while ((n = MGenGetNode(REC_QUEUE)) == 0)
		__dpmi_yield();

	r = (LPCReturn *) FARPKL(&n->rtqDatum);

	if (FARPKS(&r->error) != LPC_NOERROR)
		return -1;

	retVal = FARPKL(r->Data);

	/* get ready for next call */
	MGenMoveTo(REC_QUEUE, IDLE_QUEUE);

	return retVal;
}

char *WSLIB_inet_ntoa (struct in_addr in)
{
	static char buf[32];

	sprintf(buf, "%u.%u.%u.%u", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2, in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
	return buf;
}
#endif	/* implemented in dos_inet.c */

