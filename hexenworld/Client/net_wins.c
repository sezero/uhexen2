// net_wins.c

#include "quakedef.h"
#include "quakeinc.h"

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
#endif

#ifdef _WIN32
// Hrmph....
typedef int	socklen_t;
#endif

int LastCompMessageSize = 0;

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
#if _DEBUG
static int HuffIn=0;
static int HuffOut=0;
#endif

/*static float HuffFreq[256]=
{
	0.27588720,
	0.04243389,
	0.01598893,
	0.00737722,
	0.00557754,
	0.00547342,
	0.00823988,
	0.00449177,
	0.00986108,
	0.00560728,
	0.00654431,
	0.00376298,
	0.00498260,
	0.00400095,
	0.00655918,
	0.00232025,
	0.00504209,
	0.00285570,
	0.00124937,
	0.00147247,
	0.00226076,
	0.00141298,
	0.00467026,
	0.00336139,
	0.00123449,
	0.00126424,
	0.00166582,
	0.00129399,
	0.00114525,
	0.00116013,
	0.00078829,
	0.00080317,
	0.00639557,
	0.00123449,
	0.00127911,
	0.00102627,
	0.00182943,
	0.00141298,
	0.00269209,
	0.00127911,
	0.00224589,
	0.00093703,
	0.01972216,
	0.00135348,
	0.00477437,
	0.00337627,
	0.00743671,
	0.00765981,
	0.01951394,
	0.00319779,
	0.00330190,
	0.00267722,
	0.00235000,
	0.00159146,
	0.00285570,
	0.00141298,
	0.00151709,
	0.00139810,
	0.00468513,
	0.00117500,
	0.00202279,
	0.00544367,
	0.00096677,
	0.00136836,
	0.00913228,
	0.00316804,
	0.00138323,
	0.00078829,
	0.00942975,
	0.00590475,
	0.00168070,
	0.00089241,
	0.00095190,
	0.00166582,
	0.00077342,
	0.00071392,
	0.00086266,
	0.00060981,
	0.00075854,
	0.00065443,
	0.00126424,
	0.00047595,
	0.00068418,
	0.00099652,
	0.00065443,
	0.00096677,
	0.00072880,
	0.00050570,
	0.00071392,
	0.00102627,
	0.00120475,
	0.00056519,
	0.00281108,
	0.00175506,
	0.00047595,
	0.00154684,
	0.00160633,
	0.01091710,
	0.00365886,
	0.00355475,
	0.00937026,
	0.01105096,
	0.00404557,
	0.00206741,
	0.00389684,
	0.00456614,
	0.00215665,
	0.00191867,
	0.01072374,
	0.01051551,
	0.00467026,
	0.00867121,
	0.00542880,
	0.00105601,
	0.00649969,
	0.01280602,
	0.00510159,
	0.00316804,
	0.00456614,
	0.00523545,
	0.00157658,
	0.00191867,
	0.00121962,
	0.00065443,
	0.00080317,
	0.00052057,
	0.00080317,
	0.00147247,
	0.02236963,
	0.00258798,
	0.00059494,
	0.00060981,
	0.00038671,
	0.00044620,
	0.00055032,
	0.00093703,
	0.00072880,
	0.00062468,
	0.00059494,
	0.00047595,
	0.00044620,
	0.00182943,
	0.00066930,
	0.00077342,
	0.00114525,
	0.00062468,
	0.00059494,
	0.00049082,
	0.00059494,
	0.00074367,
	0.00090728,
	0.00069905,
	0.00075854,
	0.00077342,
	0.00069905,
	0.00055032,
	0.00075854,
	0.00049082,
	0.00104114,
	0.00062468,
	0.00397121,
	0.00080317,
	0.00072880,
	0.00062468,
	0.00163608,
	0.00046108,
	0.00055032,
	0.00056519,
	0.00102627,
	0.00071392,
	0.00092215,
	0.00078829,
	0.00660380,
	0.00058006,
	0.00065443,
	0.00049082,
	0.00118987,
	0.00077342,
	0.00065443,
	0.00077342,
	0.00151709,
	0.00083291,
	0.00074367,
	0.00098165,
	0.00108576,
	0.00081804,
	0.00145760,
	0.00144272,
	0.00394146,
	0.00104114,
	0.00099652,
	0.00209715,
	0.00502722,
	0.00309367,
	0.00123449,
	0.00096677,
	0.00090728,
	0.00206741,
	0.00147247,
	0.00147247,
	0.00129399,
	0.00087753,
	0.00120475,
	0.00116013,
	0.00145760,
	0.00120475,
	0.00138323,
	0.00123449,
	0.00166582,
	0.00087753,
	0.00171044,
	0.00239462,
	0.00156171,
	0.00154684,
	0.00217152,
	0.00226076,
	0.00211203,
	0.00153196,
	0.00174019,
	0.00145760,
	0.00168070,
	0.00145760,
	0.00123449,
	0.00165095,
	0.00188893,
	0.00138323,
	0.00120475,
	0.00154684,
	0.00138323,
	0.00191867,
	0.01866615,
	0.00139810,
	0.00153196,
	0.00093703,
	0.00121962,
	0.00116013,
	0.00075854,
	0.00105601,
	0.00432817,
	0.00315317,
	0.00407532,
	0.00227563,
	0.00081804,
	0.00121962,
	0.00110063,
	0.00090728,
	0.00108576,
	0.00065443,
	0.00096677,
	0.00655918,
	0.00153196,
	0.00251361,
	0.00312342,
	0.00243924,
	0.00660380,
	0.01700033,
};
*/

static float HuffFreq[256]=
{
 0.14473691,
 0.01147017,
 0.00167522,
 0.03831121,
 0.00356579,
 0.03811315,
 0.00178254,
 0.00199644,
 0.00183511,
 0.00225716,
 0.00211240,
 0.00308829,
 0.00172852,
 0.00186608,
 0.00215921,
 0.00168891,
 0.00168603,
 0.00218586,
 0.00284414,
 0.00161833,
 0.00196043,
 0.00151029,
 0.00173932,
 0.00218370,
 0.00934121,
 0.00220530,
 0.00381211,
 0.00185456,
 0.00194675,
 0.00161977,
 0.00186680,
 0.00182071,
 0.06421956,
 0.00537786,
 0.00514019,
 0.00487155,
 0.00493925,
 0.00503143,
 0.00514019,
 0.00453520,
 0.00454241,
 0.00485642,
 0.00422407,
 0.00593387,
 0.00458130,
 0.00343687,
 0.00342823,
 0.00531592,
 0.00324890,
 0.00333388,
 0.00308613,
 0.00293776,
 0.00258918,
 0.00259278,
 0.00377105,
 0.00267488,
 0.00227516,
 0.00415997,
 0.00248763,
 0.00301555,
 0.00220962,
 0.00206990,
 0.00270369,
 0.00231694,
 0.00273826,
 0.00450928,
 0.00384380,
 0.00504728,
 0.00221251,
 0.00376961,
 0.00232990,
 0.00312574,
 0.00291688,
 0.00280236,
 0.00252436,
 0.00229461,
 0.00294353,
 0.00241201,
 0.00366590,
 0.00199860,
 0.00257838,
 0.00225860,
 0.00260646,
 0.00187256,
 0.00266552,
 0.00242641,
 0.00219450,
 0.00192082,
 0.00182071,
 0.02185930,
 0.00157439,
 0.00164353,
 0.00161401,
 0.00187544,
 0.00186248,
 0.03338637,
 0.00186968,
 0.00172132,
 0.00148509,
 0.00177749,
 0.00144620,
 0.00192442,
 0.00169683,
 0.00209439,
 0.00209439,
 0.00259062,
 0.00194531,
 0.00182359,
 0.00159096,
 0.00145196,
 0.00128199,
 0.00158376,
 0.00171412,
 0.00243433,
 0.00345704,
 0.00156359,
 0.00145700,
 0.00157007,
 0.00232342,
 0.00154198,
 0.00140730,
 0.00288807,
 0.00152830,
 0.00151246,
 0.00250203,
 0.00224420,
 0.00161761,
 0.00714383,
 0.08188576,
 0.00802537,
 0.00119484,
 0.00123805,
 0.05632671,
 0.00305156,
 0.00105584,
 0.00105368,
 0.00099246,
 0.00090459,
 0.00109473,
 0.00115379,
 0.00261223,
 0.00105656,
 0.00124381,
 0.00100326,
 0.00127550,
 0.00089739,
 0.00162481,
 0.00100830,
 0.00097229,
 0.00078864,
 0.00107240,
 0.00084409,
 0.00265760,
 0.00116891,
 0.00073102,
 0.00075695,
 0.00093916,
 0.00106880,
 0.00086786,
 0.00185600,
 0.00608367,
 0.00133600,
 0.00075695,
 0.00122077,
 0.00566955,
 0.00108249,
 0.00259638,
 0.00077063,
 0.00166586,
 0.00090387,
 0.00087074,
 0.00084914,
 0.00130935,
 0.00162409,
 0.00085922,
 0.00093340,
 0.00093844,
 0.00087722,
 0.00108249,
 0.00098598,
 0.00095933,
 0.00427593,
 0.00496661,
 0.00102775,
 0.00159312,
 0.00118404,
 0.00114947,
 0.00104936,
 0.00154342,
 0.00140082,
 0.00115883,
 0.00110769,
 0.00161112,
 0.00169107,
 0.00107816,
 0.00142747,
 0.00279804,
 0.00085922,
 0.00116315,
 0.00119484,
 0.00128559,
 0.00146204,
 0.00130215,
 0.00101551,
 0.00091756,
 0.00161184,
 0.00236375,
 0.00131872,
 0.00214120,
 0.00088875,
 0.00138570,
 0.00211960,
 0.00094060,
 0.00088083,
 0.00094564,
 0.00090243,
 0.00106160,
 0.00088659,
 0.00114514,
	0.00095861,
	0.00108753,
	0.00124165,
	0.00427016,
	0.00159384,
	0.00170547,
	0.00104431,
	0.00091395,
	0.00095789,
	0.00134681,
	0.00095213,
	0.00105944,
	0.00094132,
	0.00141883,
	0.00102127,
	0.00101911,
	0.00082105,
	0.00158448,
	0.00102631,
	0.00087938,
	0.00139290,
	0.00114658,
	0.00095501,
	0.00161329,
	0.00126542,
	0.00113218,
	0.00123661,
	0.00101695,
	0.00112930,
	0.00317976,
	0.00085346,
	0.00101190,
	0.00189849,
	0.00105728,
	0.00186824,
	0.00092908,
	0.00160896,
};

#ifdef _DEBUG
static int freqs[256];
void ZeroFreq(void)
{
	memset(freqs, 0, 256*sizeof(int));
}


void CalcFreq(unsigned char *packet, int packetlen)
{
	int ix;

	for (ix=0; ix<packetlen; ix++)
	{
		freqs[packet[ix]]++;
	}
}

void PrintFreqs(void)
{
	int ix;
	float total=0;
	char string[100];

	for (ix=0; ix<256; ix++)
	{
		total += freqs[ix];
	}
	if (total>.01)
	{
		for (ix=0; ix<256; ix++)
		{
			sprintf(string, "\t%.8f,\n",((float)freqs[ix])/total);
			OutputDebugString(string);
		}
	}
	ZeroFreq();
}
#endif

netadr_t	net_local_adr;

netadr_t	net_from;
sizebuf_t	net_message;
int			net_socket;

#define	MAX_UDP_PACKET	(MAX_MSGLEN+9)	// one more than msg + header
byte		net_message_buffer[MAX_UDP_PACKET];

#ifdef _WIN32
WSADATA		winsockdata;
#endif

//=============================================================================

void FindTab(huffnode_t *tmp,int len,unsigned int bits)
{
	if(!tmp)
		Sys_Error("no huff node");
	if (tmp->zero)
	{
		if(!tmp->one)
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
static unsigned char Masks[8]=
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

void PutBit(unsigned char *buf,int pos,int bit)
{
	if (bit)
		buf[pos/8]|=Masks[pos%8];
	else
		buf[pos/8]&=~Masks[pos%8];
}

int GetBit(unsigned char *buf,int pos)
{
	if (buf[pos/8]&Masks[pos%8])
		return 1;
	else
		return 0;
}


void BuildTree(float *freq)
{
	float min1,min2;
	int i,j,minat1,minat2;
	huffnode_t *work[256];	
	huffnode_t *tmp;	
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
#if _DEBUG
	for (i=0;i<256;i++)
	{
		if(!HuffLookup[i].len&&HuffLookup[i].len<=32)
			Sys_Error("bad frequency table");
//		CON_Printf("%d %d %2X\n", HuffLookup[i].len, HuffLookup[i].bits, i);
	}
#endif
}

void HuffDecode(unsigned char *in,unsigned char *out,int inlen,int *outlen)
{
	int bits,tbits;
	huffnode_t *tmp;	
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

void HuffEncode(unsigned char *in,unsigned char *out,int inlen,int *outlen)
{
	int i,j,bitat;
	unsigned int t;
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
#if _DEBUG

	HuffIn+=inlen;
	HuffOut+=*outlen;

	{
		unsigned char *buf;
		int tlen;
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
	}
#endif
}


void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	*(int *)&a->ip = *(int *)&s->sin_addr;
	a->port = s->sin_port;
}


qboolean	NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;
	return false;
}

char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];
	
	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

char	*NET_BaseAdrToString (netadr_t a)
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
qboolean	NET_StringToAdr (char *s, netadr_t *a)
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
		if (*colon == ':')
		{
			*colon = 0;
			sadr.sin_port = htons((short)atoi(colon+1));	
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
	int 	ret;
#ifdef _WIN32
	int 	err;
#endif
	struct sockaddr_in	from;
	socklen_t		fromlen;

	fromlen = sizeof(from);
	ret = recvfrom (net_socket,(char *) huffbuff, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	if (ret == -1)
	{
#ifdef _WIN32
		err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return false;
		if (err == WSAEMSGSIZE) {
			Con_Printf ("Oversize packet from %s\n", NET_AdrToString (net_from));
			return false;
		}
		Sys_Error ("NET_GetPacket: %s", strerror(err));
#else
		if (errno == EWOULDBLOCK || errno == ECONNREFUSED)
			return false;
		Sys_Error ("NET_GetPacket: %s", strerror(errno));
#endif
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
	int ret, outlen;
#ifdef _WIN32
	int 	err;
#endif
	struct sockaddr_in	addr;
#ifdef _DEBUG
	char string[120];
#endif

	NetadrToSockadr (&to, &addr);
	HuffEncode((unsigned char *)data,huffbuff,length,&outlen);

#ifdef _DEBUG
	sprintf(string,"in: %d  out: %d  ratio: %f\n",HuffIn, HuffOut, 1-(float)HuffOut/(float)HuffIn);
	OutputDebugString(string);

	CalcFreq((unsigned char *)data, length);
#endif

	ret = sendto (net_socket, (char *) huffbuff, outlen, 0, (struct sockaddr *)&addr, sizeof(addr) );
	if (ret == -1)
	{
#ifdef _WIN32
		err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;
		Con_Printf ("NET_SendPacket ERROR: %i", err);
#else
		if (errno == EWOULDBLOCK || errno == ECONNREFUSED)
			return;
		Con_Printf ("NET_SendPacket ERROR: %i", errno);
#endif
	}
}

//=============================================================================

int UDP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		Sys_Error ("UDP_OpenSocket: socket:", strerror(errno));

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO:", strerror(errno));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);
	if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
		Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));

	return newsocket;
}

void NET_GetLocalAddress (void)
{
	char	buff[512];
	struct sockaddr_in	address;
	socklen_t		namelen;

	if (gethostname(buff, 512) != 0)
		Sys_Error("gethostname failed,  errno = %i,\nCannot continue, bailing out...\n",errno);
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

#ifdef _DEBUG
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

#ifdef _DEBUG
	PrintFreqs();
#endif
}

