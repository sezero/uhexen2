// net.c

#include "defs.h"
#include <errno.h>

// unix includes and compatibility macros
#if defined(PLATFORM_UNIX)
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#ifdef __sun__
#ifdef SUNOS
#include <sys/filio.h>
#endif
#ifdef __MORPHOS__
#include <proto/socket.h>
#endif

#define SOCKETERRNO errno

#if defined(__MORPHOS__)
//#undef SOCKETERRNO
//#define SOCKETERRNO Errno()
#define socklen_t int
#define ioctlsocket IoctlSocket
#define closesocket CloseSocket
#else
#define ioctlsocket ioctl
#define closesocket close
#endif

#endif	// end of unix stuff

// windows includes and compatibility macros
#ifdef _WIN32
#if !( defined(_WS2TCPIP_H) || defined(_WS2TCPIP_H_) )
// on win32, socklen_t seems to be a winsock2 thing
typedef int socklen_t;
#endif
#define SOCKETERRNO WSAGetLastError()
#define EWOULDBLOCK	WSAEWOULDBLOCK
#define ECONNREFUSED	WSAECONNREFUSED
#endif	// end of windows stuff


#ifdef _WIN32
static WSADATA	winsockdata;
#endif

static byte	net_message_buffer[MAX_UDP_PACKET];
static sizebuf_t	net_message;
int		net_socket;
static netadr_t	net_local_adr;
static netadr_t	net_from;

static int UDP_OpenSocket (int port);
static qboolean NET_CompareAdr (netadr_t a, netadr_t b);
static qboolean NET_CompareAdrNoPort (netadr_t a, netadr_t b);
static void NET_CopyAdr (netadr_t *a, netadr_t *b);
static void NET_GetLocalAddress (void);
static qboolean NET_StringToAdr (char *s, netadr_t *a);
static void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s);
static void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a);
static void NET_SendPacket (int length, void *data, netadr_t to);
static qboolean NET_GetPacket (void);

extern char	filters_file[256];

//=============================================================================
typedef struct filter_s
{
	netadr_t from;
	netadr_t to;
	struct filter_s *next;
	struct filter_s *previous;
}filter_t;

static filter_t	*filter_list = NULL;

static void FL_Remove(filter_t *filter)
{
	if(filter->previous)
		filter->previous->next = filter->next;
	if(filter->next)
		filter->next->previous = filter->previous;

	filter->next = NULL;
	filter->previous = NULL;

	if(filter_list==filter)
		filter_list = NULL;
}

static void FL_Clear(void)
{
	filter_t *filter;

	for(filter = filter_list;filter;)
	{
		if(filter)
		{
			filter_t *next = filter->next;
			FL_Remove(filter);
			free(filter);
			filter = NULL;
			filter = next;
		}
	}

	filter_list = NULL;
}

static filter_t* FL_New(netadr_t adr1,netadr_t adr2)
{
	filter_t *filter;

	filter = (filter_t *)malloc(sizeof(filter_t));
	NET_CopyAdr(&filter->from,&adr1);
	NET_CopyAdr(&filter->to,&adr2);

	return filter;
}

static void FL_Add(filter_t *filter)
{
	filter->next = filter_list;
	filter->previous = NULL;
	if(filter_list)
		filter_list->previous = filter;
	filter_list = filter;
}

static filter_t* FL_Find(netadr_t adr)
{
	filter_t *filter;

	for(filter = filter_list;filter;filter = filter->next)
	{
		if(NET_CompareAdrNoPort(filter->from,adr))
			return filter;
	}

	return NULL;
}

//=============================================================================

server_t *sv_list = NULL;

#if 0	// not used
void SVL_Clear(void)
{
	server_t *sv;

	for(sv = sv_list;sv;)
	{
		if(sv)
		{
			server_t *next = sv->next;
			SVL_Remove(sv);
			free(sv);
			sv = NULL;
			sv = next;
		}
	}

	sv_list = NULL;
}
#endif

static server_t* SVL_New(netadr_t adr)
{
	server_t *sv;

	sv = (server_t *)malloc(sizeof(server_t));
	sv->heartbeat = 0;
	sv->info[0] = 0;
	sv->ip.ip[0] = 
		sv->ip.ip[1] = 
		sv->ip.ip[2] = 
		sv->ip.ip[3] = 0;
	sv->ip.pad = 0;
	sv->ip.port = 0;
	sv->next = NULL;
	sv->previous = NULL;
	sv->players = 0;

	NET_CopyAdr(&sv->ip,&adr);

	return sv;
}

static void SVL_Add(server_t *sv)
{
	sv->next = sv_list;
	sv->previous = NULL;
	if(sv_list)
		sv_list->previous = sv;
	sv_list = sv;
}

void SVL_Remove(server_t *sv)
{
	if(sv_list==sv)
		sv_list = sv->next;

	if(sv->previous)
		sv->previous->next = sv->next;
	if(sv->next)
		sv->next->previous = sv->previous;

	sv->next = NULL;
	sv->previous = NULL;
}

static server_t* SVL_Find(netadr_t adr)
{
	server_t *sv;

	for(sv = sv_list;sv;sv = sv->next)
	{
		if(NET_CompareAdr(sv->ip,adr))
			return sv;
	}

	return NULL;
}

//=============================================================================

static void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = (byte *)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

static void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = (byte *)SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}


//=============================================================================

static void NET_Filter(void)
{
	filter_t	*filter;
	netadr_t	filter_adr;
	int		hold_port;

	hold_port = net_from.port;

	NET_StringToAdr("127.0.0.1:26950",&filter_adr);

	if(NET_CompareAdrNoPort(net_from,filter_adr))
	{
		NET_StringToAdr("0.0.0.0:26950",&filter_adr);
		if(!NET_CompareAdrNoPort(net_local_adr,filter_adr))
		{
			NET_CopyAdr(&net_from,&net_local_adr);
			net_from.port = hold_port;
		}
		return;
	}

	//if no compare with filter list
	if((filter = FL_Find(net_from)))
	{
		NET_CopyAdr(&net_from,&filter->to);
		net_from.port = hold_port;
	}
}

static void NET_Init (int port)
{
#ifdef _WIN32
	WORD	wVersionRequested;
	int		r;

	wVersionRequested = MAKEWORD(1, 1);

	r = WSAStartup (MAKEWORD(1, 1), &winsockdata);

	if (r)
		Sys_Error ("Winsock initialization failed.");
#endif

	//
	// open the single socket to be used for all communications
	//
	net_socket = UDP_OpenSocket (port);

	//
	// init the message buffer
	//
	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	//
	// determine my name & address
	//
	NET_GetLocalAddress ();

	printf("UDP Initialized\n");
}

void SV_InitNet (void)
{
	int	port;
	int	p;
	FILE	*filters;
	char	str[64];

	port = PORT_MASTER;

	p = COM_CheckParm ("-port");
	if (p && p < com_argc)
	{
		port = atoi(com_argv[p+1]);
		printf ("Port: %i\n", port);
	}
	NET_Init (port);

	//Add filters
	if((filters = fopen(filters_file,"rt")))
	{
		while(fgets(str,64,filters))
		{
			Cbuf_AddText("filter add ");
			Cbuf_AddText(str);
			Cbuf_AddText("\n");
		}

		fclose(filters);
	}
}

static int UDP_OpenSocket (int port)
{
	int	newsocket;
	struct sockaddr_in	address;
	unsigned long _true = true;
	int		i;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		Sys_Error ("UDP_OpenSocket: socket:", strerror(errno));

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO:", strerror(errno));

	address.sin_family = AF_INET;
	//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ip")) != 0 && i < com_argc - 1)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		if (address.sin_addr.s_addr == INADDR_NONE)
			Sys_Error ("%s is not a valid IP address", com_argv[i+1]);
		printf("Binding to IP Interface Address of %s\n", inet_ntoa(address.sin_addr));
	}
	else
	{
		address.sin_addr.s_addr = INADDR_ANY;
	}

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if (bind(newsocket, (struct sockaddr *)&address, sizeof(address)) == -1)
		Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));

	return newsocket;
}

void NET_Shutdown (void)
{
	closesocket (net_socket);
#ifdef _WIN32
	WSACleanup ();
#endif
}

static qboolean NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;

	return false;
}

static qboolean NET_CompareAdrNoPort (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
		return true;

	return false;
}

static void NET_CopyAdr (netadr_t *a, netadr_t *b)
{
	a->ip[0] = b->ip[0];
	a->ip[1] = b->ip[1];
	a->ip[2] = b->ip[2];
	a->ip[3] = b->ip[3];
	a->port = b->port;
	a->pad = b->pad;
}

static void NET_GetLocalAddress (void)
{
	char	buff[512];
	struct sockaddr_in	address;
	socklen_t		namelen;

	gethostname(buff, 512);
	buff[512-1] = 0;

	NET_StringToAdr (buff, &net_local_adr);

	namelen = sizeof(address);
	if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == -1)
		Sys_Error ("NET_Init: getsockname:", strerror(errno));

	net_local_adr.port = address.sin_port;

	printf("IP address %s\n", NET_AdrToString (net_local_adr) );
}

char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

static qboolean NET_StringToAdr (char *s, netadr_t *a)
{
	struct hostent	*h;
	struct sockaddr_in sadr;
	char	*colon;
	char	copy[128];

	memset (&sadr, 0, sizeof(sadr));
	sadr.sin_family = AF_INET;

	sadr.sin_port = 0;

	strcpy (copy, s);
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
	{
		if (*colon == ':')
		{
			*colon = 0;
			sadr.sin_port = htons((short)atoi(colon+1));
		}
	}

	if (copy[0] >= '0' && copy[0] <= '9')
	{
		*(int *)&sadr.sin_addr = inet_addr(copy);
	}
	else
	{
		if ((h = gethostbyname(copy)) == 0)
			return 0;

		*(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
	}

	SockadrToNetadr (&sadr, a);

	return true;
}

static void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

static void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	*(int *)&a->ip = *(int *)&s->sin_addr;
	a->port = s->sin_port;
}

static void NET_SendPacket (int length, void *data, netadr_t to)
{
	int	ret, err;
	struct sockaddr_in	addr;

	NetadrToSockadr (&to, &addr);

	ret = sendto (net_socket, (char *)data, length, 0, (struct sockaddr *)&addr, sizeof(addr) );
	if (ret == -1)
	{
		err = SOCKETERRNO;
		if (err == EWOULDBLOCK)
			return;
		printf ("NET_SendPacket ERROR: %i\n", err);
	}
}

static void AnalysePacket(void)
{
	byte	c;
	byte	*p;
	int		i;

	printf("%s sending packet:\n",NET_AdrToString(net_from));

	p = net_message.data;

	for(i = 0;i<net_message.cursize;i++,p++)
	{
		c = p[0];
		printf(" %3i ",c);

		if(i%8==7)
			printf("\n");
	}

	printf("\n\n");

	p = net_message.data;

	for(i = 0;i<net_message.cursize;i++,p++)
	{
		c = p[0];

		if(c=='\n')
			printf("  \\n ");
		else if(c>=32&&c<=127)
			printf("   %c ",c);
		else if(c<10)
			printf("  \\%1i ",c);
		else if(c<100)
			printf(" \\%2i ",c);
		else
			printf("\\%3i ",c);

		if(i%8==7)
			printf("\n");
	}

	printf("\n");
}

static void Mst_SendList(void)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;
	server_t	*sv;
	short int	sv_num = 0;

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;
	msg.allowoverflow = true;
	msg.overflowed = false;

	//number of servers:
	for(sv = sv_list;sv;sv = sv->next)
		sv_num++;

	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,255);
	MSG_WriteByte(&msg,'d');
	MSG_WriteByte(&msg,'\n');

	if(sv_num>0)
	{
		for(sv = sv_list;sv;sv = sv->next)
		{
			MSG_WriteByte(&msg,sv->ip.ip[0]);
			MSG_WriteByte(&msg,sv->ip.ip[1]);
			MSG_WriteByte(&msg,sv->ip.ip[2]);
			MSG_WriteByte(&msg,sv->ip.ip[3]);
			MSG_WriteShort(&msg,sv->ip.port);
		}
	}

	NET_SendPacket(msg.cursize,msg.data,net_from);
}

static void Mst_Packet(void)
{
	char		msg;
	server_t	*sv;

	//NET_Filter();

	msg = net_message.data[1];

	if(msg==A2A_PING)
	{
		NET_Filter();

		printf("%s >> A2A_PING\n",NET_AdrToString(net_from));
		if(!(sv = SVL_Find(net_from)))
		{
			sv = SVL_New(net_from);
			SVL_Add(sv);
		}
		sv->timeout = Sys_DoubleTime();
	}
	else if(msg==S2M_HEARTBEAT)
	{
		NET_Filter();
		printf("%s >> S2M_HEARTBEAT\n",NET_AdrToString(net_from));
		if(!(sv = SVL_Find(net_from)))
		{
			sv = SVL_New(net_from);
			SVL_Add(sv);
		}
		sv->timeout = Sys_DoubleTime();
	}
	else if(msg==S2M_SHUTDOWN)
	{
		NET_Filter();
		printf("%s >> S2M_SHUTDOWN\n",NET_AdrToString(net_from));
		if((sv = SVL_Find(net_from)))
		{
			SVL_Remove(sv);
			free(sv);
		}
	}
	else if(msg=='c')
	{
		printf("%s >> ",NET_AdrToString(net_from));
		printf("Gamespy server list request\n");
		Mst_SendList();
	}
	else
	{
		byte	*p;
		p = net_message.data;

		printf("%s >> ",NET_AdrToString(net_from));
		printf("Pingtool server list request\n");

		if(p[0]==0&&p[1]=='y')
		{
			Mst_SendList();
		}
		else
		{
			printf("%s >> ",NET_AdrToString(net_from));
			printf("%c\n",net_message.data[1]);
			AnalysePacket();
		}
	}
}

void SV_ReadPackets (void)
{
	while (NET_GetPacket ())
	{
		Mst_Packet();
	}
}

static qboolean NET_GetPacket (void)
{
	int	ret, err;
	struct sockaddr_in	from;
	socklen_t		fromlen;

	fromlen = sizeof(from);
	ret = recvfrom (net_socket, (char *)net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	SockadrToNetadr (&from, &net_from);

	if (ret == -1)
	{
		err = SOCKETERRNO;
		if (err == EWOULDBLOCK)
			return false;
#ifdef _WIN32
		if (err == WSAEMSGSIZE)
		{
			printf ("Warning:  Oversize packet from %s\n",
				NET_AdrToString (net_from));
			return false;
		}
#endif
		//Sys_Error ("NET_GetPacket: %s", strerror(err));
		printf ("Warning:  Unrecognized recvfrom error, error code = %i\n",err);
		return false;
	}

	net_message.cursize = ret;
	if (ret == sizeof(net_message_buffer) )
	{
		printf ("Oversize packet from %s\n", NET_AdrToString (net_from));
		return false;
	}

	return ret;
}

#if 0	// not used
void SV_ConnectionlessPacket (void)
{
	printf("%s>>%s\n",NET_AdrToString(net_from),net_message.data);
}
#endif

static int argv_index_add;

static void Cmd_FilterAdd(void)
{
	filter_t	*filter;
	netadr_t	to, from;

	if(Cmd_Argc()<4+argv_index_add)
	{
		printf("Invalid command parameters. Usage:\nfilter add x.x.x.x:port x.x.x.x:port\n\n");
		return;
	}

	NET_StringToAdr(Cmd_Argv(2+argv_index_add),&from);
	NET_StringToAdr(Cmd_Argv(3+argv_index_add),&to);

	if(to.port==0)
		from.port = BigShort(PORT_SERVER);

	if(from.port==0)
		from.port = BigShort(PORT_SERVER);

	if(!(filter = FL_Find(from)))
	{
		printf("Added filter %s\t\t%s\n",Cmd_Argv(2+argv_index_add),Cmd_Argv(3+argv_index_add));

		filter = FL_New(from,to);
		FL_Add(filter);
	}
	else
	{
		printf("%s already defined\n\n",Cmd_Argv(2+argv_index_add));
	}
}

static void Cmd_FilterRemove(void)
{
	filter_t	*filter;
	netadr_t	from;

	if(Cmd_Argc()<3+argv_index_add)
	{
		printf("Invalid command parameters. Usage:\nfilter remove x.x.x.x:port\n\n");
		return;
	}

	NET_StringToAdr(Cmd_Argv(2+argv_index_add),&from);

	if((filter = FL_Find(from)))
	{
		printf("Removed %s\n\n",Cmd_Argv(2+argv_index_add));

		FL_Remove(filter);
		free(filter);
	}
	else
	{
		printf("Cannot find %s\n\n",Cmd_Argv(2+argv_index_add));
	}
}

static void Cmd_FilterList(void)
{
	filter_t	*filter;

	for(filter = filter_list;filter;filter = filter->next)
	{
		printf("%s",NET_AdrToString(filter->from));
		printf("\t\t%s\n",NET_AdrToString(filter->to));
	}

	if(filter_list==NULL)
		printf("No filter\n");

	printf("\n");
}

static void Cmd_FilterClear(void)
{
	printf("Removed all filters\n\n");
	FL_Clear();
}

void Cmd_Filter_f(void)
{
	argv_index_add = 0;

	if(!strcmp(Cmd_Argv(1),"add"))
	{
		Cmd_FilterAdd();
	}
	else if(!strcmp(Cmd_Argv(1),"remove"))
	{
		Cmd_FilterRemove();
	}
	else if(!strcmp(Cmd_Argv(1),"clear"))
	{
		Cmd_FilterClear();
	}
	else if(Cmd_Argc()==3)
	{
		argv_index_add = -1;
		Cmd_FilterAdd();
	}
	else if(Cmd_Argc()==2)
	{
		argv_index_add = -1;
		Cmd_FilterRemove();
	}
	else
	{
		Cmd_FilterList();
	}
}

void SV_WriteFilterList(void)
{
	FILE	*filters;
	filter_t	*filter;

	if((filters = fopen(filters_file,"wt")))
	{
		if(filter_list == NULL)
		{
			fclose(filters);
			return;
		}

		for(filter = filter_list;filter;filter = filter->next)
		{
			fprintf(filters,"%s",NET_AdrToString(filter->from));
			fprintf(filters," %s\n",NET_AdrToString(filter->to));
		}
		fclose(filters);
	}
}
