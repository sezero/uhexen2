#ifndef _w32_WATTCP_H
#define _w32_WATTCP_H

/*!\file wattcp.h
 */

#if defined(WIN32) || defined(_WIN32)
  #undef WIN32
  #undef _WIN32
  #define WIN32  1
  #define _WIN32 1

  /*
   * This must come before "target.h".
   * Prevent including <winsock*.h>.
   */
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif

  /* Required by bsdname.c + winpcap.c
   */
  #if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
  #undef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
  #endif

  #include <windows.h>

  /*
   * Wattcp basic types.
   */
#else
  typedef unsigned char   BYTE;    /**<\typedef 8 bits    */
  typedef unsigned short  WORD;    /**<\typedef 16 bits   */
  typedef unsigned long   DWORD;   /**<\typedef 32 bits   */
  typedef unsigned int    UINT;    /**<\typedef 16/32 bit */
#endif

#ifndef WATT32_BUILD
#define WATT32_BUILD
#endif

typedef BYTE  eth_address[6];      /**<\typedef Ether address */
typedef BYTE  tok_address[6];      /**<\typedef TokenRing address */
typedef BYTE  fddi_address[6];     /**<\typedef FDDI address */
typedef BYTE  ax25_address[7];     /**<\typedef AX-25 address */
typedef BYTE  arcnet_address;      /**<\typedef ARCNET address */
typedef BYTE  ip6_address[16];     /**<\typedef IPv6 address */

#define mac_address eth_address  /* !!fix-me: breaks AX25 drivers */

/**<\typedef protocol handler callback type.
 */
typedef int (*ProtoHandler) (void *sock, const void *data, unsigned len,
                                   const void *tcp_pseudo_hdr, const void *udp_hdr);

/**<\typedef user-data callback type.
 */
typedef int (*UserHandler) (void *sock);

/**<\typedef Callback type for ICMP event.
 */
typedef int (*icmp_upcall) (void *socket, BYTE icmp_type, BYTE icmp_code);

/**<\typedef 64-bit types (compiler dependant).
 */
#if defined(__HIGHC__) || defined(__GNUC__) || defined(__CCDL__) || \
    defined(__LCC__)   || defined(__POCC__)
  typedef unsigned long long  uint64;  /**< our unsigned "long long" type */
  typedef long long           int64;   /**< our signed "long long" type */
  #define HAVE_UINT64                  /**< have a compiler with 64-bit ints */

#elif defined(__DMC__) && (__INTSIZE == 4)
  typedef unsigned long long  uint64;
  typedef long long           int64;
  #define HAVE_UINT64

#elif defined(__WATCOMC__) && defined(__WATCOM_INT64__) && !(defined(__SMALL__) || defined(__LARGE__))
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64

#elif defined(_MSC_VER) && (_MSC_VER >= 900)
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64

#elif defined(__BORLANDC__) && defined(WIN32)
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64
#endif

struct ulong_long {
       DWORD lo;
       DWORD hi;
      };


/**
 * Namespace prefix "_w32_".
 *
 * Until C compilers support C++ namespaces, we use this
 * prefix for our namespace.
 */
#define NAMESPACE(x)  _w32_ ## x

#define STATIC        /* ease disassembly */

#define loBYTE(w)     (BYTE)(w)
#define hiBYTE(w)     (BYTE)((WORD)(w) >> 8)
#define DIM(x)        (int) (sizeof(x) / sizeof((x)[0]))
#define SIZEOF(x)     (int) sizeof(x)

#ifdef __LCC__
  #define ARGSUSED(foo)  foo = foo
  #define ATOI(x)        atoi ((char*)(x))
  #define ATOL(x)        atol ((char*)(x))
#else
  #define ARGSUSED(foo)  (void)foo
  #define ATOI(x)        atoi (x)
  #define ATOL(x)        atol (x)
#endif

/**
 * Compiler and target definitions.
 */
#include "target.h"          /**< portability macros & defines. */
#include "config.h"          /**< options & features to include. */

#include <sys/w32api.h>      /**< export/import decorations */
#include <sys/werrno.h>      /**< errno stuff */

#if defined(USE_CRTDBG)      /**< use CrtDebug in MSVC debug-mode */
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>

#elif defined(USE_ZONEMEM)
  #include "zonemem.h"       /**< use Z_Malloc & co */
#elif defined(USE_FORTIFY)
  #include "fortify.h"       /**< use Fortify malloc library. */
#endif

#if defined(WATT32_DOS_DLL) && defined(__DJGPP__)
#include "dxe_sym.h"         /**< making a djgpp DXE module */
#endif

#if defined(MAKE_TSR)        /**< Exclude some more code for TSRs */
#undef HAVE_UINT64
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif


/**
 * Sizes and protocols we use.
 */
#define ETH_MAX_DATA     1500
#define ETH_MIN          60
#define ETH_MAX          (ETH_MAX_DATA + sizeof(eth_Header))

#define TOK_MAX_DATA     ETH_MAX_DATA  /* could be much larger */
#define TOK_MIN          sizeof(tok_Header)
#define TOK_MAX          (TOK_MAX_DATA + sizeof(tok_Header))

#define FDDI_MAX_DATA    ETH_MAX_DATA  /* really is 4470 */
#define FDDI_MIN         (3 + sizeof(fddi_Header))
#define FDDI_MAX         (FDDI_MAX_DATA + sizeof(fddi_Header))

#define ARCNET_MAX_DATA  512    /* Long frame or Exception frame */
#define ARCNET_MIN       257
#define ARCNET_MAX       (ARCNET_MAX_DATA + sizeof(arcnet_Header))

#define AX25_MAX_DATA    ETH_MAX_DATA
#define AX25_MIN         sizeof(ax25_Header)
#define AX25_MAX         (AX25_MAX_DATA + sizeof(ax25_Header))

#define PPPOE_MAX_DATA   (ETH_MAX_DATA - 8)
#define PPPOE_MIN        sizeof(pppoe_Header)
#define PPPOE_MAX        (PPPOE_MAX_DATA + sizeof(pppoe_Header))

#define VLAN_MAX_DATA    (ETH_MAX_DATA - 4)
#define VLAN_MAX         (VLAN_MAX_DATA + sizeof(vlan_Header))
#define VLAN_MIN         sizeof(vlan_Header)

#define TCP_OVERHEAD     (sizeof(in_Header) + sizeof(tcp_Header))
#define UDP_OVERHEAD     (sizeof(in_Header) + sizeof(udp_Header))


/** Ether-protocol numbers.
 * \note these are in network (big-endian) order
 */
#define IP4_TYPE         0x0008
#define IP6_TYPE         0xDD86
#define IEEE802_1Q_TYPE  0x0081
#define ARP_TYPE         0x0608
#define RARP_TYPE        0x3580
#define PPPOE_DISC_TYPE  0x6388
#define PPPOE_SESS_TYPE  0x6488

/** ARCNET-protocol numbers from EtheReal.
 * \note these are all a single byte on wire
 */
#define ARCNET_DP_BOOT    0
#define ARCNET_DP_MOUNT   1
#define ARCNET_PL_BEACON  8
#define ARCNET_PL_BEACON2 243

#define ARCNET_DIAG       128
#define ARCNET_IP6        196
#define ARCNET_BACNET     205
#define ARCNET_IP_1201    212
#define ARCNET_ARP_1201   213
#define ARCNET_RARP_1201  214
#define ARCNET_ATALK      221
#define ARCNET_ETHER      232
#define ARCNET_NOVELL     236
#define ARCNET_IP_1051    240
#define ARCNET_ARP_1051   241
#define ARCNET_BANYAN     247
#define ARCNET_IPX        250
#define ARCNET_LANSOFT    251


#if (DOSX)
  #define MAX_FRAGMENTS   45UL
  #define MAX_WINDOW      (64*1024U)   /**< max TCP window */
#else
  #define MAX_FRAGMENTS   30UL
  #define MAX_WINDOW      (32*1024U)
#endif

/** This should really be a function of current MAC-driver.
 */
#define MAX_IP4_DATA      (ETH_MAX_DATA - sizeof(in_Header))
#define MAX_IP6_DATA      (ETH_MAX_DATA - sizeof(in6_Header))
#define MAX_FRAG_SIZE     (MAX_FRAGMENTS * MAX_IP4_DATA)

#define MAX_NAMELEN       80           /**< max length of a wattcp.cfg keyword */
#define MAX_VALUELEN      80           /**< max length of a wattcp.cfg value */
#define MAX_PATHLEN       256          /**< should be enough for most... */

#define MAX_COOKIES       10
#define MAX_HOSTLEN       80           /**< most strings are limited */
#define MAX_ADDRESSES     10           /**< # of addresses in resolvers */

#define SAFETY_TCP        0x538F25A3L  /**< marker signatures */
#define SAFETY_UDP        0x3E45E154L

/**
 * The IP protocol numbers we need (see RFC-1700).
 */
#define UDP_PROTO         17
#define TCP_PROTO         6
#define ICMP_PROTO        1
#define IGMP_PROTO        2
#define IPCOMP_PROTO      108

/**
 * _udp_Socket/_tcp_Socket 'sockmode' values
 */
#define SOCK_MODE_BINARY  0x01    /**< default mode (ASCII mode if 0) */
#define SOCK_MODE_UDPCHK  0x02    /**< UDP: do checksum checks (default) */
#define SOCK_MODE_NAGLE   0x04    /**< Nagle algorithm (default) */
#define SOCK_MODE_LOCAL   0x08    /**< set by sock_noflush() */
#define SOCK_MODE_SAWCR   0x10    /**< for ASCII sockets; saw a newline */
#define SOCK_MODE_MASK    0x07    /**< mask for sock_mode() */


/**
 * UDP/TCP socket local flags (locflags) bits.
 * Mostly used to support the BSD-socket API.
 */
#define LF_WINUPDATE    0x00001   /**< need to send a window-update */
#define LF_NOPUSH       0x00002   /**< don't push on write */
#define LF_NOOPT        0x00004   /**< don't use tcp options */
#define LF_REUSEADDR    0x00008   /**< \todo Reuse address not supported */
#define LF_KEEPALIVE    0x00010   /**< we got a keepalive ACK */
#define LF_LINGER       0x00020
#define LF_NOCLOSE      0x00040
#define LF_NO_IPFRAGS   0x00080
#define LF_OOBINLINE    0x00100
#define LF_SNDTIMEO     0x00200
#define LF_RCVTIMEO     0x00400
#define LF_GOT_FIN      0x00800
#define LF_GOT_PUSH     0x01000
#define LF_GOT_ICMP     0x02000   /**< got an ICMP port/dest unreachable */
#define LF_USE_TSTAMP   0x04000   /**< send a TS option on next send */
#define LF_RCVD_SCALE   0x08000   /**< a win-scale was received in SYN */
#define LF_IS_SERVER    0x10000   /**< socket is a server (listening)  */
#define LF_SACK_PERMIT  0x20000

/**
 * Socket-states for sock_sselect().
 * Not used by BSD-socket API
 */
#define SOCKESTABLISHED  1
#define SOCKDATAREADY    2
#define SOCKCLOSED       4


#include <sys/packon.h>  /**< align structs on byte boundaries */

/*!\struct in_Header
 *
 * The Internet (ip) Header.
 */
typedef struct in_Header {
#if defined(USE_BIGENDIAN)
        BYTE   ver    : 4;
        BYTE   hdrlen : 4;
#elif defined(OLD_TURBOC) || defined(__CCDL__)
        int    hdrlen : 4;
        int    ver    : 4;
#else
        BYTE   hdrlen : 4;     /* Watcom requires BYTE here */
        BYTE   ver    : 4;
#endif
        BYTE   tos;
        WORD   length;
        WORD   identification;
        WORD   frag_ofs;
        BYTE   ttl;
        BYTE   proto;
        WORD   checksum;
        DWORD  source;
        DWORD  destination;
      } in_Header;

/** `in_Header.frag_ofs' bits.
 */
#define IP_CE      0x8000     /**< Congestion Experienced */
#define IP_DF      0x4000     /**< Don't Fragment */
#define IP_MF      0x2000     /**< More Fragments */
#define IP_OFFMASK 0x1FFF     /**< Offset mask value */

/** `in_Header.tos' bits.
 */
#define IP_MINCOST     0x02
#define IP_RELIABILITY 0x04
#define IP_THROUGHPUT  0x08
#define IP_LOWDELAY    0x10
#define IP_TOSMASK    (IP_MINCOST|IP_RELIABILITY|IP_THROUGHPUT|IP_LOWDELAY)

#define in_GetHdrLen(ip) ((ip)->hdrlen << 2)  /**< # of bytes in IP-header */

#define IP_BCAST_ADDR    0xFFFFFFFFUL

/*!\struct in6_Header
 *
 * IPv6 header.
 */
typedef struct in6_Header {
#if defined(USE_BIGENDIAN)
        BYTE        ver : 4;
        BYTE        pri : 4;
#else
        BYTE        pri : 4;
        BYTE        ver : 4;
#endif
        BYTE        flow_lbl[3];
        WORD        len;
        BYTE        next_hdr;
        BYTE        hop_limit;
        ip6_address source;
        ip6_address destination;
      } in6_Header;

/*!\struct ip_Packet
 *
 * IPv4 packet including header and data.
 */
typedef struct ip_Packet {
        in_Header head;
        BYTE      data [MAX_IP4_DATA];
      } ip_Packet;


/*!\struct udp_Header
 *
 * The UDP header.
 */
typedef struct udp_Header {
        WORD   srcPort;
        WORD   dstPort;
        WORD   length;
        WORD   checksum;
      } udp_Header;


/*!\struct tcp_Header
 *
 * The TCP header.
 */
typedef struct tcp_Header {
        WORD   srcPort;
        WORD   dstPort;
        DWORD  seqnum;
        DWORD  acknum;

#if defined(USE_BIGENDIAN)
        BYTE   offset : 4;
        BYTE   unused : 4;
#elif defined(OLD_TURBOC) || defined(__CCDL__)
        int    unused : 4;
        int    offset : 4;
#else
        BYTE   unused : 4;   /* Watcom requires BYTE here */
        BYTE   offset : 4;
#endif
        BYTE   flags;
        WORD   window;
        WORD   checksum;
        WORD   urgent;
      } tcp_Header;

/**
 * tcp_Header::flags bits
 */
#define tcp_FlagFIN   0x01
#define tcp_FlagSYN   0x02
#define tcp_FlagRST   0x04
#define tcp_FlagPUSH  0x08
#define tcp_FlagACK   0x10
#define tcp_FlagURG   0x20
#define tcp_FlagECN   0x40    /* ECN-Echo */
#define tcp_FlagCWR   0x80    /* congestion window reduced */
#define tcp_FlagMASK  0x3F    /* ignore ECN/CWR for now */

/*!\struct tcp_PseudoHeader
 *
 * The TCP/UDP Pseudo Header (IPv4).
 */
typedef struct tcp_PseudoHeader {
        DWORD  src;
        DWORD  dst;
        BYTE   mbz;
        BYTE   protocol;
        WORD   length;
        WORD   checksum;
      } tcp_PseudoHeader;


/*!\struct tcp_PseudoHeader6
 *
 * The TCP/UDP Pseudo Header (IPv6).
 */
typedef struct tcp_PseudoHeader6 {
        ip6_address  src;
        ip6_address  dst;
        WORD         length;
        BYTE         zero[3];
        BYTE         next_hdr;
      } tcp_PseudoHeader6;


/*!\struct IGMP_packet
 *
 * Internet Group Management Protocol packet.
 */
typedef struct IGMP_packet {
#if defined(USE_BIGENDIAN)
        int    version : 4;
        int    type    : 4;
#else
        int    type    : 4;
        int    version : 4;
#endif
        BYTE   mbz;
        WORD   checksum;
        DWORD  address;
      } IGMP_packet;

#define IGMP_VERSION   1
#define IGMPv1_QUERY   1
#define IGMPv1_REPORT  2


/*!\struct arp_Header
 *
 * ARP/RARP header.
 */
typedef struct arp_Header {
        WORD        hwType;
        WORD        protType;
        BYTE        hwAddrLen;     /**< MAC addr. length (6) */
        BYTE        protoAddrLen;  /**< IP addr. length  (4) */
        WORD        opcode;
        eth_address srcEthAddr;
        DWORD       srcIPAddr;
        eth_address dstEthAddr;
        DWORD       dstIPAddr;
      } arp_Header;

#include <sys/packoff.h>           /**< restore default packing */

#define rarp_Header arp_Header

/**
 * ARP definitions.
 */
#define ARP_REQUEST    0x0100      /**< ARP/RARP op codes, Request. */
#define ARP_REPLY      0x0200      /**<                    Reply. */
#define RARP_REQUEST   0x0300
#define RARP_REPLY     0x0400


/**
 * TCP states, from tcp specification RFC-793.
 *
 * \note CLOSE-WAIT state is bypassed by automatically closing a connection
 *       when a FIN is received.  This is easy to undo.
 *       RESOLVE is a pseudo state before SYN is sent in tcp_Retransmitter().
 */
#define tcp_StateLISTEN   0      /* listening for connection */
#define tcp_StateRESOLVE  1      /* resolving IP, waiting on ARP reply */
#define tcp_StateSYNSENT  2      /* SYN sent, active open */
#define tcp_StateSYNREC   3      /* SYN received, ACK+SYN sent. */
#define tcp_StateESTAB    4      /* established */
#define tcp_StateESTCL    5      /* established, but will FIN */
#define tcp_StateFINWT1   6      /* sent FIN */
#define tcp_StateFINWT2   7      /* sent FIN, received FINACK */
#define tcp_StateCLOSWT   8      /* received FIN waiting for close */
#define tcp_StateCLOSING  9      /* sent FIN, received FIN (waiting for FINACK) */
#define tcp_StateLASTACK  10     /* FIN received, FINACK+FIN sent */
#define tcp_StateTIMEWT   11     /* dally after sending final FINACK */
#define tcp_StateCLOSED   12     /* FIN+ACK received */

#define tcp_MaxBufSize    2048   /* maximum bytes to buffer on input */
#define udp_MaxBufSize    1520
#define tcp_MaxTxBufSize  tcp_MaxBufSize  /* and on tcp output */

/**
 * Fields common to UDP & TCP socket definition.
 *
 * Tries to keep members on natural boundaries (words on word-boundary,
 * dwords on dword boundary)
 */

#define UDP_TCP_COMMON                                                       \
        WORD         ip_type;          /* UDP_PROTO,TCP_PROTO or IPx_TYPE */ \
        BYTE         ttl;              /* Time To Live */                    \
        BYTE         fill_1;                                                 \
        const char  *err_msg;          /* NULL when all is okay */           \
        char         err_buf[100];     /* room for error message */          \
        void       (*usr_yield)(void); /* yield while waiting */             \
        icmp_upcall  icmp_callb;       /* socket-layer callback (icmp) */    \
        BYTE         rigid;                                                  \
        BYTE         stress;                                                 \
        WORD         sockmode;         /* a logical OR of bits */            \
        WORD         fill_2;                                                 \
        DWORD        usertimer;        /* ip_timer_set, ip_timer_timeout */  \
        ProtoHandler protoHandler;     /* called with incoming data */       \
        eth_address  his_ethaddr;      /* peer's ethernet address */         \
                                                                             \
        DWORD        myaddr;           /* my IPv4-address */                 \
        DWORD        hisaddr;          /* peer's IPv4 address */             \
        WORD         hisport;          /* peer's source port */              \
        WORD         myport;           /* my source port */                  \
        DWORD        locflags;         /* local option flags */              \
        BOOL         is_ip6;           /* TRUE if IPv6 socket */             \
        int          rx_datalen;       /* Rx length, must be signed */       \
        UINT         max_rx_data;      /* Last index for rx_data[] */        \
        BYTE        *rx_data           /* Rx data buffer (default rx_buf[]) */

/*!\struct _udp_Socket
 *
 * UDP socket definition.
 */
typedef struct _udp_Socket {
        struct _udp_Socket *next;
        UDP_TCP_COMMON;
        BYTE rx_buf[udp_MaxBufSize+1]; /**< received data buffer */

#if defined(USE_IPV6)
        ip6_address  my6addr;          /**< my ip6-address */
        ip6_address  his6addr;         /**< peer's ip-6 address */
#endif
        DWORD        safetysig;        /**< magic marker */
      } _udp_Socket;


/*!\struct _tcp_Socket
 *
 * TCP Socket definition (fields common to _udp_Socket must come first).
 */
typedef struct _tcp_Socket {
        struct _tcp_Socket *next;      /**< link to next tcp-socket */
        UDP_TCP_COMMON;

        BYTE rx_buf[tcp_MaxBufSize+1]; /**< received data buffer */

#if defined(USE_IPV6)
        ip6_address  my6addr;          /**< our IPv6 address */
        ip6_address  his6addr;         /**< peer's IPv6 address */
#endif
        UINT         state;            /**< tcp connection state */
        DWORD        recv_next;        /**< SEQ number we expect to receive */
        DWORD        send_next;        /**< SEQ we send but not ACK-ed by peer */
        long         send_una;         /**< unacked send data, must be signed */

#if defined(USE_DEBUG)
        DWORD        last_acknum[2];   /**< for pcdbug.c; to follow SEQ/ACK */
        DWORD        last_seqnum[2];   /**< increments */
#endif
        DWORD        timeout;          /**< timer for retrans etc. */
        BYTE         unhappy;          /**< flag, indicates retransmitting segt's */
        BYTE         recent;           /**< 1 if recently transmitted */
        WORD         flags;            /**< TCP flags used in next Tx */

        UINT         window;           /**< other guy's window */
        UINT         adv_win;          /**< our last advertised window */

        BYTE         cwindow;          /**< Congestion window */
        BYTE         wwindow;          /**< Van Jacobson's algorithm */
        WORD         fill_4;

        DWORD        vj_sa;            /**< VJ's alg, standard average   (SRTT) */
        DWORD        vj_sd;            /**< VJ's alg, standard deviation (RTTVAR) */
        DWORD        vj_last;          /**< last transmit time */
        UINT         rto;              /**< retransmission timeout */
        BYTE         karn_count;       /**< count of packets */
        BYTE         tos;              /**< TOS from IP-header */
        WORD         fill_5;

        DWORD        rtt_time;         /**< Round Trip Time value */
        DWORD        rtt_lasttran;     /**< RTT at last transmission */

        DWORD        ts_sent;          /**< last TimeStamp value sent */
        DWORD        ts_recent;        /**< last TimeStamp value received */
        DWORD        ts_echo;          /**< last TimeStamp echo received */

        UINT         max_seg;          /**< MSS for this connection */

        /** S. Lawson - handle one dropped segment.
         * \todo Make a prober re-assembly queue.
         * missed_seq[0] is left edge of missing segment.
         * missed_seq[1] is right edge (in peer's absolute SEQ space)
         */
        DWORD        missed_seq[2];
     /* void        *reasm_buf; */     /**< linked-list of frags; not yet */

#if defined(USE_TCP_MD5)
        char        *secret;           /**< Secret for MD5 finger-print */
#endif
        DWORD        inactive_to;      /**< inactive timer (no Rx data) */
        DWORD        datatimer;        /**< inactive timer (no Tx data) */
     /* int          sock_delay; ?? */

        BYTE         tx_wscale;        /**< \todo window scales shifts, Tx/Rx */
        BYTE         rx_wscale;
        UINT         tx_queuelen;      /**< optional Tx queue length */
        const BYTE  *tx_queue;

        UINT         tx_datalen;       /**< number of bytes of data to send */
        UINT         max_tx_data;      /**< Last index for tx_data[] */
        BYTE        *tx_data;          /**< Tx data buffer (default tx_buf[]) */
        BYTE         tx_buf[tcp_MaxTxBufSize+1]; /**< data for transmission */
        DWORD        safetysig;        /**< magic marker */
        DWORD        safetytcp;        /**< extra magic marker */
      } _tcp_Socket;


/*!\struct _raw_Socket
 *
 * Raw IPv4 socket definition. Only used in BSD-socket API.
 */
typedef struct _raw_Socket {
        struct _raw_Socket *next;
        WORD   ip_type;                  /**< same ofs as for udp/tcp Socket */
        BOOL   used;                     /**< used flag; packet not read yet */
        DWORD  seq_num;                  /**< counter for finding oldest pkt */
        struct in_Header ip;
        BYTE   rx_data [MAX_FRAG_SIZE];  /**< room for 1 jumbo IP packet */
      } _raw_Socket;

/*!\struct _raw6_Socket
 *
 * Raw IPv6 socket definition. Only used in BSD-socket API.
 */
typedef struct _raw6_Socket {
        struct _raw6_Socket *next;
        WORD   ip_type;
        BOOL   used;
        DWORD  seq_num;
        struct in6_Header ip6;
        BYTE   rx_data [MAX_IP6_DATA];
      } _raw6_Socket;

/*!\union sock_type
 *
 * sock_type used for socket I/O.
 */
typedef union sock_type {
        _udp_Socket  udp;
        _tcp_Socket  tcp;
        _raw_Socket  raw;
        _raw6_Socket raw6;
      } sock_type;

/*!\struct watt_sockaddr
 *
 * \deprecated Old style sockaddr
 */
typedef struct watt_sockaddr {
        WORD   s_type;
        WORD   s_port;
        DWORD  s_ip;
        BYTE   s_spares[6];    /**< unused in TCP realm */
      } watt_sockaddr;

#endif /* _w32_WATTCP_H */

