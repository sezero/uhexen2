// net_wins.c

#include "quakedef.h"
#include "quakeinc.h"

// unix includes and compatibility macros
#ifdef PLATFORM_UNIX
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define ioctlsocket(A,B,C) ioctl(A,B,C)
#define closesocket(A) close(A)
#define CopyMemory(A,B,C) memcpy(A,B,C)
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#define UINT unsigned int
#define WORD unsigned short
#define SOCKET int
#define SOCKADDR_IN struct sockaddr_in
#define LPSOCKADDR struct sockaddr*
#define LPHOSTENT struct hostent*
#define HOSTENT struct hostent
#define LPINADDR struct in_addr*
#define LPIN_ADDR struct in_addr*
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif	// end of unix stuff

// socklen_t: on win32, it seems to be a winsock2 thing
#if defined(_WIN32) && !( defined(_WS2TCPIP_H) || defined(_WS2TCPIP_H_) )
typedef int	socklen_t;
#endif


//=============================================================================

//
// huffman types and vars
//

typedef struct huffnode_s
{
	struct huffnode_s *zero;
	struct huffnode_s *one;
	unsigned char val;
	float freq;
} huffnode_t;

typedef struct
{
	unsigned int bits;
	int len;
} hufftab_t;

static huffnode_t *HuffTree=0;
static hufftab_t HuffLookup[256];

static float HuffFreq[256] =
{
#	include "hufffreq.h"
};


//=============================================================================

//
// huffman debugging
//
#ifdef DEBUG_BUILD
static int HuffIn = 0;
static int HuffOut= 0;
static int freqs[256];

static void ZeroFreq (void)
{
	memset(freqs, 0, 256*sizeof(int));
}

static void CalcFreq (unsigned char *packet, int packetlen)
{
	int		ix;

	for (ix=0; ix<packetlen; ix++)
	{
		freqs[packet[ix]]++;
	}
}

static void PrintFreqs (void)
{
	int		ix;
	float	total = 0;
	char	string[100];

	for (ix=0; ix<256; ix++)
	{
		total += freqs[ix];
	}

	if (total>.01)
	{
		for (ix=0; ix<256; ix++)
		{
			sprintf(string, "\t%.8f,\n", ((float)freqs[ix])/total);
#	ifdef _WIN32
			OutputDebugString(string);
#	else
			fprintf(stderr, "%s", string);
#	endif
		}
	}

	ZeroFreq();
}
#endif	// DEBUG_BUILD


//=============================================================================

//
// huffman functions
//

static void FindTab (huffnode_t *tmp, int len, unsigned int bits)
{
	if (!tmp)
		Sys_Error("no huff node");

	if (tmp->zero)
	{
		if (!tmp->one)
			Sys_Error("no one in node");
		if(len>=32)
			Sys_Error("compression screwd");
		FindTab(tmp->zero,len+1,bits<<1);
		FindTab(tmp->one,len+1,(bits<<1)|1);
		return;
	}

	HuffLookup[tmp->val].len=len;
	HuffLookup[tmp->val].bits=bits;
	return;
}

static unsigned char Masks[8] =
{
	0x1,
	0x2,
	0x4,
	0x8,
	0x10,
	0x20,
	0x40,
	0x80
};

static void PutBit (unsigned char *buf, int pos, int bit)
{
	if (bit)
		buf[pos/8]|=Masks[pos%8];
	else
		buf[pos/8]&=~Masks[pos%8];
}

static int GetBit (unsigned char *buf, int pos)
{
	if (buf[pos/8]&Masks[pos%8])
		return 1;
	else
		return 0;
}

static void BuildTree (float *freq)
{
	float	min1, min2;
	int	i, j, minat1, minat2;
	huffnode_t	*work[256];
	huffnode_t	*tmp;

	for (i=0;i<256;i++)
	{
		work[i]=malloc(sizeof(huffnode_t));
		work[i]->val=(unsigned char)i;
		work[i]->freq=freq[i];
		work[i]->zero=0;
		work[i]->one=0;
		HuffLookup[i].len=0;
	}

	for (i=0;i<255;i++)
	{
		minat1=-1;
		minat2=-1;
		min1=1E30;
		min2=1E30;

		for (j=0;j<256;j++)
		{
			if (!work[j])
				continue;
			if (work[j]->freq<min1)
			{
				minat2=minat1;
				min2=min1;
				minat1=j;
				min1=work[j]->freq;
			}
			else if (work[j]->freq<min2)
			{
				minat2=j;
				min2=work[j]->freq;
			}
		}
		if (minat1<0)
			Sys_Error("minatl: %f",minat1);
		if (minat2<0)
			Sys_Error("minat2: %f",minat2);
		tmp=malloc(sizeof(huffnode_t));
		tmp->zero=work[minat2];
		tmp->one=work[minat1];
		tmp->freq=work[minat2]->freq+work[minat1]->freq;
		tmp->val=0xff;
		work[minat1]=tmp;
		work[minat2]=0;
	}

	HuffTree=tmp;
	FindTab(HuffTree,0,0);

#if DEBUG_BUILD
	for (i=0;i<256;i++)
	{
		if(!HuffLookup[i].len&&HuffLookup[i].len<=32)
			Sys_Error("bad frequency table");
		//	Con_Printf("%d %d %2X\n", HuffLookup[i].len, HuffLookup[i].bits, i);
	}
#endif
}

static void HuffDecode (unsigned char *in, unsigned char *out, int inlen, int *outlen)
{
	int	bits,tbits;
	huffnode_t	*tmp;

	if (*in==0xff)
	{
		memcpy(out,in+1,inlen-1);
		*outlen=inlen-1;
		return;
	}

	tbits=(inlen-1)*8-*in;
	bits=0;
	*outlen=0;

	while (bits<tbits)
	{
		tmp=HuffTree;
		do
		{
			if (GetBit(in+1,bits))
				tmp=tmp->one;
			else
				tmp=tmp->zero;
			bits++;
		} while (tmp->zero);

		*out++=tmp->val;
		(*outlen)++;
	}
}

static void HuffEncode (unsigned char *in, unsigned char *out, int inlen, int *outlen)
{
	int	i, j, bitat;
	unsigned int	t;
#ifdef DEBUG_BUILD
	unsigned char	*buf;
	int	tlen;
#endif	// DEBUG_BUILD

	bitat=0;

	for (i=0;i<inlen;i++)
	{
		t=HuffLookup[in[i]].bits;
		for (j=0;j<HuffLookup[in[i]].len;j++)
		{
			PutBit(out+1,bitat+HuffLookup[in[i]].len-j-1,t&1);
			t>>=1;
		}
		bitat+=HuffLookup[in[i]].len;
	}

	*outlen=1+(bitat+7)/8;
	*out=8*((*outlen)-1)-bitat;

	if(*outlen >= inlen+1)
	{
		*out=0xff;
		memcpy(out+1,in,inlen);
		*outlen=inlen+1;
	}

#ifdef DEBUG_BUILD
	HuffIn+=inlen;
	HuffOut+=*outlen;

	buf=malloc(inlen);
	HuffDecode(out,buf,*outlen,&tlen);
	if(!tlen==inlen)
		Sys_Error("bogus compression");
	for (i=0;i<inlen;i++)
	{
		if(in[i]!=buf[i])
			Sys_Error("bogus compression");
	}
	free(buf);
#endif	// DEBUG_BUILD
}


//=============================================================================

//
// net driver vars
//

int LastCompMessageSize = 0;

netadr_t	net_local_adr;
netadr_t	net_from;
sizebuf_t	net_message;
int			net_socket;

#ifdef _WIN32
static WSADATA	winsockdata;
#endif

#define	MAX_UDP_PACKET	(MAX_MSGLEN+9)	// one more than msg + header
static byte	net_message_buffer[MAX_UDP_PACKET];


//=============================================================================

//
// net driver functions
//

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

qboolean NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;
	return false;
}

char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

char *NET_BaseAdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);

	return s;
}

/*
=============
NET_StringToAdr

idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
qboolean NET_StringToAdr (char *s, netadr_t *a)
{
	struct hostent		*h;
	struct sockaddr_in	sadr;
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
		h = gethostbyname (copy);
		if (!h)
			return 0;
		*(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
	}

	SockadrToNetadr (&sadr, a);

	return true;
}


//=============================================================================

static unsigned char huffbuff[65536];

qboolean NET_GetPacket (void)
{
	int	ret;
	struct sockaddr_in	from;
	socklen_t		fromlen;

	fromlen = sizeof(from);
	ret = recvfrom (net_socket,(char *) huffbuff, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	if (ret == -1)
	{
#	ifdef _WIN32
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return false;
		if (err == WSAEMSGSIZE)
		{
			Con_Printf ("Oversize packet from %s\n", NET_AdrToString (net_from));
			return false;
		}
		Sys_Error ("NET_GetPacket: %s", strerror(err));
#	else
		if (errno == EWOULDBLOCK || errno == ECONNREFUSED)
			return false;
		Sys_Error ("NET_GetPacket: %s", strerror(errno));
#	endif
	}

	SockadrToNetadr (&from, &net_from);

	if (ret == sizeof(net_message_buffer) )
	{
		Con_Printf ("Oversize packet from %s\n", NET_AdrToString (net_from));
		return false;
	}

	LastCompMessageSize += ret;//keep track of bytes actually received for debugging

	HuffDecode(huffbuff, (unsigned char *)net_message_buffer,ret,&ret);
	net_message.cursize = ret;

	return ret;
}


//=============================================================================

void NET_SendPacket (int length, void *data, netadr_t to)
{
	int	ret, outlen;
	struct sockaddr_in	addr;
#ifdef DEBUG_BUILD
	char	string[120];
#endif

	NetadrToSockadr (&to, &addr);
	HuffEncode((unsigned char *)data,huffbuff,length,&outlen);

#ifdef DEBUG_BUILD
	sprintf(string,"in: %d  out: %d  ratio: %f\n",HuffIn, HuffOut, 1-(float)HuffOut/(float)HuffIn);
#	ifdef _WIN32
	OutputDebugString(string);
#	else
	fprintf(stderr, "%s", string);
#	endif
	CalcFreq((unsigned char *)data, length);
#endif	// DEBUG_BUILD

	ret = sendto (net_socket, (char *) huffbuff, outlen, 0, (struct sockaddr *)&addr, sizeof(addr) );
	if (ret == -1)
	{
#	ifdef _WIN32
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;
		Con_Printf ("NET_SendPacket ERROR: %i\n", err);
#	else
		if (errno == EWOULDBLOCK || errno == ECONNREFUSED)
			return;
		Con_Printf ("NET_SendPacket ERROR: %i\n", errno);
#	endif
	}
}


//=============================================================================

static int UDP_OpenSocket (int port)
{
	int	i, newsocket;
	struct sockaddr_in	address;
	unsigned long _true = true;

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
		Con_Printf("Binding to IP Interface Address of %s\n", inet_ntoa(address.sin_addr));
	}
	else
	{
		address.sin_addr.s_addr = INADDR_ANY;
	}

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if ( bind(newsocket, (void *)&address, sizeof(address)) == -1 )
		Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));

	return newsocket;
}

static void NET_GetLocalAddress (void)
{
	char	buff[512];
	struct sockaddr_in	address;
	socklen_t		namelen;

	if (gethostname(buff, 512) != 0)
		Sys_Error("gethostname failed,  errno = %i,\nCannot continue, bailing out...", errno);
	buff[512-1] = 0;

	NET_StringToAdr (buff, &net_local_adr);

	namelen = sizeof(address);
	if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == -1)
		Sys_Error ("NET_Init: getsockname:", strerror(errno));
	net_local_adr.port = address.sin_port;

	Con_Printf("IP address %s\n", NET_AdrToString (net_local_adr) );
}

/*
====================
NET_Init
====================
*/
void NET_Init (int port)
{
#ifdef _WIN32
	WORD	wVersionRequested;
	int		r;
#endif

#ifdef DEBUG_BUILD
	ZeroFreq();
#endif

	BuildTree(HuffFreq);

#ifdef _WIN32
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

	Con_Printf("UDP Initialized\n");
}

/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
	closesocket (net_socket);
#ifdef _WIN32
	WSACleanup ();
#endif

#ifdef DEBUG_BUILD
	PrintFreqs();
#endif
}

