/*!\file inc/tcp.h
 * Watt-32 public API.
 */

/*
 * Waterloo TCP
 *
 * Copyright (c) 1990-1993 Erick Engelke
 *
 * Portions copyright others, see copyright.h for details.
 *
 * This library is free software; you can use it or redistribute under
 * the terms of the license included in LICENSE.H.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE.H for more details.
 *
 */

#ifndef __WATT_TCP_H
#define __WATT_TCP_H

/*
 * Version (major.minor.dev-rel), 8-bit each.
 */
#define WATTCP_MAJOR_VER  2
#define WATTCP_MINOR_VER  2
#define WATTCP_DEVEL_REL  10

#define WATTCP_VER  ((WATTCP_MAJOR_VER << 16) + \
                     (WATTCP_MINOR_VER << 8) +  \
                     WATTCP_DEVEL_REL)

#define WATTCP_VER_STRING  "2.2.10"

#if !defined(RC_INVOKED)

#include <stdio.h>
#include <sys/wtypes.h>  /* fd_set, iovec */
#include <sys/wtime.h>   /* struct timeval, cdecl */
#include <sys/swap.h>    /* intel(), intel16() */
#include <sys/w32api.h>  /* W32_FUNC, W32_DATA etc. */

#ifdef __WATCOMC__
  #pragma read_only_directory;
#endif

#ifdef __cplusplus
  extern "C" {
#endif

W32_DATA const char *wattcpCopyright;  /* "See COPYRIGHT.H for details" */

W32_FUNC const char *wattcpVersion (void);      /* WatTCP target version/date */
W32_FUNC const char *wattcpCapabilities (void); /* what's been compiled in */

#if !defined(WATT32_BUILD)

/*
 * Typedefs and constants
 */
#ifndef BYTE
#define BYTE   unsigned char
#endif

#ifndef WORD
#define WORD   unsigned short
#endif

#ifndef DWORD
#define DWORD  unsigned long
#endif

#ifndef BOOL
#define BOOL   int
#endif

#ifndef sock_type
#define sock_type  void
#endif

/*
 * Old compatibility
 */
#ifndef WATT32_NO_OLDIES
#ifndef byte
#define byte  unsigned char
#endif

#ifndef word
#define word  unsigned short
#endif

#ifndef dword
#define dword unsigned long
#endif

#ifndef longword
#define longword unsigned long
#endif
#endif

/*
 * Basic typedefs
 */
typedef BYTE  eth_address[6];
typedef BYTE  ip6_address[16];

typedef int (*ProtoHandler) (void *sock, const BYTE *data, unsigned len,
                             const void *tcp_phdr, const void *udp_hdr);

typedef int (*UserHandler)  (void *sock);

#endif /* WATT32_BUILD */


typedef struct {
        BYTE   undoc [4470];
      } tcp_Socket;

typedef struct {
        BYTE   undoc [1740];
      } udp_Socket;


/* Silly C++ compilers needs this to supress warning at max warning level
 */
typedef void (*VoidProc)(void);

#define MAX_COOKIES      10
#define MAX_NAMESERVERS  10
#define MAX_HOSTLEN      80

/* Modes for sock_mode()
 */
#define TCP_MODE_BINARY  0x01   /* deprecated */
#define TCP_MODE_ASCII   0x02   /* deprecated */
#define SOCK_MODE_BINARY 0x01   /* new name */
#define SOCK_MODE_ASCII  0x02   /* new name */
#define UDP_MODE_CHK     0x04   /* defaults to checksum */
#define UDP_MODE_NOCHK   0x08
#define TCP_MODE_NAGLE   0x10   /* Nagle's algorithm */
#define TCP_MODE_NONAGLE 0x20

/* wait-states for sock_sselect()
 */
#define SOCKESTABLISHED  1
#define SOCKDATAREADY    2
#define SOCKCLOSED       4

/*
 * Hide "private" symbols by prefixing with "_w32_"
 */
#undef  W32_NAMESPACE
#define W32_NAMESPACE(x)   _w32_ ## x

#define init_misc          W32_NAMESPACE (init_misc)
#define Random             W32_NAMESPACE (Random)
#define set_timeout        W32_NAMESPACE (set_timeout)
#define chk_timeout        W32_NAMESPACE (chk_timeout)
#define cmp_timeout        W32_NAMESPACE (cmp_timeout)
#define hires_timer        W32_NAMESPACE (hires_timer)
#define set_timediff       W32_NAMESPACE (set_timediff)
#define get_timediff       W32_NAMESPACE (get_timediff)
#define timeval_diff       W32_NAMESPACE (timeval_diff)

#define my_ip_addr         W32_NAMESPACE (my_ip_addr)
#define sin_mask           W32_NAMESPACE (sin_mask)
#define sock_delay         W32_NAMESPACE (sock_delay)
#define sock_inactive      W32_NAMESPACE (sock_inactive)
#define sock_data_timeout  W32_NAMESPACE (sock_data_timeout)
#define multihomes         W32_NAMESPACE (multihomes)
#define block_tcp          W32_NAMESPACE (block_tcp)
#define block_udp          W32_NAMESPACE (block_udp)
#define block_ip           W32_NAMESPACE (block_ip)
#define block_icmp         W32_NAMESPACE (block_icmp)
#define last_cookie        W32_NAMESPACE (last_cookie)
#define cookies            W32_NAMESPACE (cookies)
#define survive_eth        W32_NAMESPACE (survive_eth)
#define survive_bootp      W32_NAMESPACE (survive_bootp)
#define survive_dhcp       W32_NAMESPACE (survive_dhcp)
#define survive_rarp       W32_NAMESPACE (survive_rarp)
#define loopback_handler   W32_NAMESPACE (loopback_handler)
#define usr_init           W32_NAMESPACE (usr_init)
#define usr_post_init      W32_NAMESPACE (usr_post_init)
#define in_checksum        W32_NAMESPACE (in_checksum)
#define aton               W32_NAMESPACE (aton)
#define isaddr             W32_NAMESPACE (isaddr)
#define _printf            W32_NAMESPACE (_printf)
#define _outch             W32_NAMESPACE (_outch)
#define outs               W32_NAMESPACE (outs)
#define outsnl             W32_NAMESPACE (outsnl)
#define outsn              W32_NAMESPACE (outsn)
#define outhexes           W32_NAMESPACE (outhexes)
#define outhex             W32_NAMESPACE (outhex)
#define rip                W32_NAMESPACE (rip)
#define parse_config_table W32_NAMESPACE (parse_config_table)
#define init_timer_isr     W32_NAMESPACE (init_timer_isr)
#define exit_timer_isr     W32_NAMESPACE (exit_timer_isr)

#undef  sock_init
#define sock_init() watt_sock_init (sizeof(tcp_Socket), sizeof(udp_Socket))

W32_FUNC int         watt_sock_init (size_t, size_t);
W32_FUNC const char *sock_init_err  (int rc);

W32_FUNC void sock_exit (void);
W32_FUNC void dbug_init (void);  /* effective if compiled with `USE_DEBUG' */
W32_FUNC void init_misc (void);  /* may be called before sock_init() */
W32_FUNC void sock_sig_exit (const char *msg, int sigint);

#if !defined(WATT32_BUILD)
/*
 * `s' is the pointer to a udp or tcp socket
 */
W32_FUNC int    sock_read       (void *s, char *dp, int len);
W32_FUNC int    sock_preread    (void *s, char *dp, int len);
W32_FUNC int    sock_fastread   (void *s, char *dp, int len);
W32_FUNC int    sock_write      (void *s, const char *dp, int len);
W32_FUNC int    sock_enqueue    (void *s, const char *dp, int len);
W32_FUNC int    sock_fastwrite  (void *s, const char *dp, int len);
W32_FUNC size_t sock_setbuf     (void *s, char *buf, size_t len);
W32_FUNC void   sock_flush      (void *s);
W32_FUNC void   sock_noflush    (void *s);
W32_FUNC void   sock_flushnext  (void *s);
W32_FUNC int    sock_puts       (void *s, const char *dp);
W32_FUNC WORD   sock_gets       (void *s, char *dp, int n);
W32_FUNC BYTE   sock_putc       (void *s, char c);
W32_FUNC int    sock_getc       (void *s);
W32_FUNC WORD   sock_dataready  (void *s);
W32_FUNC int    sock_established(void *s);
W32_FUNC int    sock_close      (void *s);
W32_FUNC int    sock_abort      (void *s);
W32_FUNC void (*sock_yield      (void *s, VoidProc fn)) (void);
W32_FUNC int    sock_mode       (void *s, WORD mode);
W32_FUNC int    sock_sselect    (void *s, int waitstate);
W32_FUNC int    sock_timeout    (void *s, int seconds);
W32_FUNC int    sock_recv       (void *s, char *buf, unsigned len);
W32_FUNC int    sock_recv_init  (void *s, char *buf, unsigned len);
W32_FUNC int    sock_recv_from  (void *s, DWORD *ip, WORD *port, char *buf, unsigned len, int peek);
W32_FUNC int    sock_recv_used  (void *s);
W32_FUNC int    sock_keepalive  (void *s);

W32_FUNC size_t sock_rbsize     (const void *s);
W32_FUNC size_t sock_rbused     (const void *s);
W32_FUNC size_t sock_rbleft     (const void *s);
W32_FUNC size_t sock_tbsize     (const void *s);
W32_FUNC size_t sock_tbused     (const void *s);
W32_FUNC size_t sock_tbleft     (const void *s);

W32_FUNC int MS_CDECL sock_printf (void *s, const char *fmt, ...)
  #if defined(__GNUC__)
  __attribute__((format(printf,2,3)))
  #endif
  ;

W32_FUNC int MS_CDECL sock_scanf (void *s, const char *fmt, ...)
  #if defined(__GNUC__)
  __attribute__((format(scanf,2,3)))
  #endif
  ;

/*
 * TCP or UDP specific stuff, must be used for open's and listens, but
 * sock stuff above is used for everything else
 */
W32_FUNC int udp_open   (void *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int tcp_open   (void *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int udp_listen (void *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int tcp_listen (void *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler, WORD timeout);
W32_FUNC int tcp_established (const void *s);

#endif  /* WATT32_BUILD */


W32_FUNC DWORD resolve    (const char *name);
W32_FUNC int   resolve_ip (DWORD ip, char *name, int len);
W32_FUNC DWORD lookup_host(const char *host, char *ip_str);
W32_FUNC const char *dom_strerror (int err);

/*
 * less general functions
 */
W32_FUNC int   tcp_cbreak (int mode);

W32_FUNC DWORD aton       (const char *name);
W32_FUNC int   isaddr     (const char *name);
W32_FUNC char *rip        (char *s);
W32_FUNC int   watt_kbhit (void);

#define tcp_cbrk(mode)  tcp_cbreak(mode) /* old name */

/*
 * Set MD5 secret for TCP option 19 (RFC-2385).
 * Only if built with USE_TCP_MD5.
 */
W32_FUNC const char *tcp_md5_secret (void *s, const char *secret);

/*
 * timers
 */
W32_FUNC DWORD  set_timeout  (DWORD msec);
W32_FUNC int    chk_timeout  (DWORD value);
W32_FUNC int    cmp_timers   (DWORD t1, DWORD t2);
W32_FUNC int    set_timediff (long msec);
W32_FUNC long   get_timediff (DWORD now, DWORD t);
W32_FUNC int    hires_timer  (int on);
W32_FUNC double timeval_diff (const struct timeval *a, const struct timeval *b);

W32_FUNC void   init_timer_isr (void);
W32_FUNC void   exit_timer_isr (void);

W32_FUNC void   init_userSuppliedTimerTick (void);
W32_FUNC void   userTimerTick (DWORD);

W32_FUNC void   ip_timer_init (sock_type *s , unsigned delayseconds);
W32_FUNC int    ip_timer_expired (const sock_type *s);

/*
 * TCP/IP system variables
 */
W32_DATA DWORD my_ip_addr;
W32_DATA DWORD sin_mask;       /* eg.  0xFFFFFE00L */
W32_DATA int   sock_delay;
W32_DATA int   sock_inactive;
W32_DATA int   sock_data_timeout;
W32_DATA WORD  multihomes;

W32_DATA int   block_tcp;
W32_DATA int   block_udp;
W32_DATA int   block_icmp;
W32_DATA int   block_ip;
W32_DATA WORD  last_cookie;
W32_DATA DWORD cookies [MAX_COOKIES];

W32_DATA BOOL  survive_eth;
W32_DATA BOOL  survive_bootp;
W32_DATA BOOL  survive_dhcp;
W32_DATA BOOL  survive_rarp;

W32_DATA void (*loopback_handler) (void *ip);

/*
 * things you probably won't need to know about
 */

/*
 * sock_debugdump
 *      - dump some socket control block parameters
 * used for testing the kernal, not recommended
 */
W32_FUNC void sock_debugdump (const sock_type *s);

/*
 * tcp_config - read a configuration file
 *            - if special path desired, call after sock_init()
 *            - NULL reads path WATTCP.CFG env-var or from program's path
 * see sock_init();
 */
W32_FUNC long tcp_config (const char *path);

/*
 * tcp_tick - must be called periodically by user application.
 *          - returns NULL when our socket closes
 */
W32_FUNC WORD tcp_tick (sock_type *s);

/*
 * tcp_set_debug_state - set to 1,2 or reset 0
 */
W32_FUNC void tcp_set_debug_state (WORD x);


/*
 * name domain constants, etc.
 */
#define def_domain       W32_NAMESPACE (def_domain)
#define def_nameservers  W32_NAMESPACE (def_nameservers)
#define dns_timeout      W32_NAMESPACE (dns_timeout)
#define dom_errno        W32_NAMESPACE (dom_errno)
#define last_nameserver  W32_NAMESPACE (last_nameserver)
#define _mtu             W32_NAMESPACE (_mtu)
#define _mss             W32_NAMESPACE (_mss)
#define ctrace_on        W32_NAMESPACE (ctrace_on)
#define has_rdtsc        W32_NAMESPACE (has_rdtsc)
#define clocks_per_usec  W32_NAMESPACE (clocks_per_usec)
#define psocket          W32_NAMESPACE (psocket)
#define _inet_ntoa       W32_NAMESPACE (_inet_ntoa)
#define _inet_addr       W32_NAMESPACE (_inet_addr)

W32_DATA char         *def_domain;
W32_DATA int           dom_errno;
W32_DATA DWORD         def_nameservers [MAX_NAMESERVERS];
W32_DATA WORD          dns_timeout;
W32_DATA WORD          last_nameserver;
W32_DATA WORD         _watt_handle_cbreak;      /* ^C/^Break handle mode */
W32_DATA volatile int _watt_cbroke;             /* ^C/^Break occured */
W32_DATA unsigned     _mtu, _mss;
W32_DATA int           ctrace_on;
W32_DATA int           has_rdtsc;
W32_DATA DWORD         clocks_per_usec;

/* Old compatibility
 */
#define wathndlcbrk  _watt_handle_cbreak
#define watcbroke    _watt_cbroke

/*
 * sock_wait_ .. macros
 */

/*
 * sock_wait_established()
 *      - Waits until connected or aborts if timeout etc.
 *
 * sock_wait_input()
 *      - Waits for received input on socket 's'.
 *      - May not be valid input for sock_gets()..  check returned length.
 *
 * sock_tick()
 *      - Do tick and jump on abort.
 *
 * sock_wait_closed()
 *      - Close socket and wait until fully closed.
 *        Discards all received data.
 *
 * All these macros jump to label sock_err with contents of *statusptr
 * set to
 *       1 on closed normally.
 *      -1 on error, call sockerr(s) for cause.
 *
 */
#if !defined(WATT32_BUILD)

W32_FUNC int _ip_delay0 (void *s, int sec, UserHandler fn, void *statusptr);
W32_FUNC int _ip_delay1 (void *s, int sec, UserHandler fn, void *statusptr);
W32_FUNC int _ip_delay2 (void *s, int sec, UserHandler fn, void *statusptr);


#define sock_wait_established(s,seconds,fn,statusptr) \
        do {                                          \
           if (_ip_delay0 (s,seconds,fn,statusptr))   \
              goto sock_err;                          \
        } while (0)

#define sock_wait_input(s,seconds,fn,statusptr)       \
        do {                                          \
           if (_ip_delay1 (s,seconds,fn,statusptr))   \
              goto sock_err;                          \
        } while (0)

#define sock_tick(s, statusptr)                       \
        do {                                          \
           if (!tcp_tick(s)) {                        \
              if (statusptr) *statusptr = -1;         \
              goto sock_err;                          \
           }                                          \
        } while (0)

#define sock_wait_closed(s,seconds,fn,statusptr)      \
        do {                                          \
           if (_ip_delay2(s,seconds,fn,statusptr))    \
              goto sock_err;                          \
        } while (0)

#endif  /* WATT32_BUILD */

/*
 * User hook for WATTCP.CFG initialisation file.
 */
W32_DATA void (*usr_init) (const char *keyword, const char *value);
W32_DATA void (*usr_post_init) (void);

enum config_tab_types {
     ARG_ATOI,        /* convert to int */
     ARG_ATOB,        /* convert to 8-bit byte */
     ARG_ATOW,        /* convert to 16-bit word */
     ARG_ATOIP,       /* convert to ip-address on host order */
     ARG_ATOX_B,      /* convert to hex-byte */
     ARG_ATOX_W,      /* convert to hex-word */
     ARG_ATOX_D,      /* convert to hex-word */
     ARG_STRDUP,      /* duplicate string value */
     ARG_STRCPY,      /* copy string value */
     ARG_RESOLVE,     /* resolve host to ip-address */
     ARG_FUNC         /* call convertion function */
   };

struct config_table {
       const char            *keyword;
       enum config_tab_types  type;
       void                  *arg_func;
     };


W32_FUNC int parse_config_table (const struct config_table *tab,
                                 const char *section,
                                 const char *name,
                                 const char *value);

/*
 * Run with no config file (embedded/diskless)
 */
W32_DATA int _watt_no_config;

W32_FUNC void tcp_inject_config (
              const struct config_table *cfg,
              const char                *key,
              const char                *value);

typedef long (*WattUserConfigFunc) (int pass, const struct config_table *cfg);

W32_FUNC WattUserConfigFunc _watt_user_config (WattUserConfigFunc fn);


/*
 * Bypass standard handling of DHCP transient configuration
 */
#include <sys/packon.h>

struct DHCP_config {
       DWORD  my_ip;
       DWORD  netmask;
       DWORD  gateway;
       DWORD  nameserver;
       DWORD  server;
       DWORD  iplease;
       DWORD  renewal;
       DWORD  rebind;
       DWORD  tcp_keep_intvl;
       BYTE   default_ttl;
       char   hostname [MAX_HOSTLEN+1];
       char   domain [MAX_HOSTLEN+1];
       char   loghost [MAX_HOSTLEN+1]; /* Only used if USE_BSD_FUNC defined */
     };

#include <sys/packoff.h>

enum DHCP_config_op {
     DHCP_OP_READ  = 0,
     DHCP_OP_WRITE = 1,
     DHCP_OP_ERASE = 2
   };

typedef int (*WattDHCPConfigFunc) (enum DHCP_config_op op,
                                   struct DHCP_config *cfg);

W32_FUNC WattDHCPConfigFunc dhcp_set_config_func (WattDHCPConfigFunc fn);


/*
 * Various function-pointer hooks etc.
 */
W32_DATA int (MS_CDECL *_printf) (const char *, ...);
W32_DATA void (*_outch) (char c);
W32_DATA int  (*_resolve_hook) (void);
W32_DATA void (*wintr_chain) (void);

W32_DATA int (*tftp_writer) (const void *buf, unsigned length);
W32_DATA int (*tftp_terminator) (void);

W32_FUNC void  outs    (const char *s);
W32_FUNC void  outsnl  (const char *s);
W32_FUNC void  outsn   (const char *s, int n);
W32_FUNC void  outhexes(const char *s, int n);
W32_FUNC void  outhex  (char ch);

W32_FUNC int   wintr_enable (void);
W32_FUNC void  wintr_disable (void);
W32_FUNC void  wintr_shutdown (void);
W32_FUNC void  wintr_init (void);

W32_FUNC int   _ping     (DWORD host, DWORD num, const BYTE *pattern, long len);
W32_FUNC DWORD _chk_ping (DWORD host, DWORD *ping_num);

W32_FUNC int   _eth_init         (void);
W32_FUNC void  _eth_release      (void);
W32_FUNC void *_eth_formatpacket (const void *eth_dest, WORD eth_type);
W32_FUNC void  _eth_free         (const void *buf);
W32_FUNC void *_eth_arrived      (WORD *type, BOOL *brdcast);
W32_FUNC int   _eth_send         (WORD len, const void *sock, const char *file, unsigned line);
W32_FUNC int   _eth_set_addr     (const void *addr);
W32_FUNC BYTE  _eth_get_hwtype   (BYTE *hwtype, BYTE *hwlen);

W32_DATA void *(*_eth_recv_hook) (WORD *type);
W32_DATA int   (*_eth_recv_peek) (void *mac_buf);
W32_DATA int   (*_eth_xmit_hook) (const void *buf, unsigned len);
W32_FUNC WORD    in_checksum     (const void *buf, unsigned len);

#define inchksum(buf,len)  in_checksum(buf, len)


/*
 * BSD-socket similarities.
 * Refer <sys/socket.h> for the real thing.
 */
#if !defined(WATT32_BUILD)

struct watt_sockaddr {     /* for _getpeername, _getsockname */
       WORD   s_type;
       WORD   s_port;
       DWORD  s_ip;
       BYTE   s_spares[6]; /* unused */
     };

W32_FUNC DWORD _gethostid   (void);
W32_FUNC DWORD _sethostid   (DWORD ip);
W32_FUNC int   _getsockname (const void *s, void *dest, int *len);
W32_FUNC int   _getpeername (const void *s, void *dest, int *len);

W32_FUNC int   _chk_socket  (const void *s);
W32_FUNC void  psocket      (const void *s);

#endif

W32_FUNC char *_inet_ntoa   (char *s, DWORD x);
W32_FUNC DWORD _inet_addr   (const char *name);

W32_FUNC BOOL _arp_register (DWORD use, DWORD instead_of);
W32_FUNC BOOL _arp_resolve  (DWORD ina, eth_address *res);

W32_FUNC int   addwattcpd (VoidProc p);
W32_FUNC int   delwattcpd (VoidProc p);

W32_FUNC void _sock_debug_on  (void);
W32_FUNC void _sock_debug_off (void);

#if !defined(WATT32_BUILD)

W32_FUNC const char *sockerr  (const void *s);   /* UDP / TCP */
W32_FUNC void  sockerr_clear  (void *s);         /* UDP / TCP */

W32_FUNC const char *sockstate (const void *s);  /* UDP / TCP / Raw */

/*
 * Reduce internal states to "user-easy" states, GvB 2002-09
 */
enum TCP_SIMPLE_STATE {
     TCP_CLOSED,
     TCP_LISTENING,
     TCP_OPENING,
     TCP_OPEN,
     TCP_CLOSING
   };

W32_FUNC enum TCP_SIMPLE_STATE tcp_simple_state (const tcp_Socket *s);

#endif  /* WATT32_BUILD */


/*
 * BSD functions for read/write/select
 */
W32_FUNC int close_s  (int s);
W32_FUNC int write_s  (int s, const char *buf, int nbyte);
W32_FUNC int read_s   (int s,       char *buf, int nbyte);
W32_FUNC int writev_s (int s, const struct iovec *vector, size_t count);
W32_FUNC int readv_s  (int s, const struct iovec *vector, size_t count);

W32_FUNC const sock_type *__get_sock_from_s (int s, int proto);
                            /* Use 'IPPROTO_x' for proto */

W32_FUNC int select_s (int num_sockets,
                       fd_set *read_events,
                       fd_set *write_events,
                       fd_set *except_events,
                       struct timeval *timeout);

/* The only BSD/Winsock replacement in this file.
 * Normally belongs in djgpp's <unistd.h>.
 */
#ifndef __DJGPP__
W32_FUNC int W32_CALL select (int num_sockets,
                              fd_set *read_events,
                              fd_set *write_events,
                              fd_set *except_events,
                              struct timeval *timeout);
#endif

/* Duplicated from <sys/socket.h>
 */
W32_FUNC int W32_CALL gethostname (char *name, int len);

/*
 * Multicast stuff (if built with `USE_MULTICAST')
 */
W32_DATA int _multicast_on;

W32_FUNC int join_mcast_group  (DWORD ip);
W32_FUNC int leave_mcast_group (DWORD ip);
W32_FUNC int _ip4_is_multicast (DWORD ip);
W32_FUNC int multi_to_eth      (DWORD ip, eth_address *eth);
W32_FUNC int udp_SetTTL        (udp_Socket *s, BYTE ttl);


/*
 * Commandline parsing
 */
#if defined(__DJGPP__)
  #include <unistd.h>

#elif defined(__MINGW32__)
  #include <getopt.h>

#elif !defined(_GETOPT_H)  /* not using a local getopt.c */
  #define optarg    W32_NAMESPACE (optarg)
  #define optind    W32_NAMESPACE (optind)
  #define opterr    W32_NAMESPACE (opterr)
  #define optopt    W32_NAMESPACE (optopt)
  #define optswchar W32_NAMESPACE (optswchar)
  #define optmode   W32_NAMESPACE (optmode)
  #define getopt    W32_NAMESPACE (getopt)

  W32_DATA char *optarg;       /* argument of current option                    */
  W32_DATA int   optind;       /* index of next argument; default=0: initialize */
  W32_DATA int   opterr;       /* 0=disable error messages; default=1: enable   */
  W32_DATA int   optopt;       /* option char returned from getopt()            */
  W32_DATA char *optswchar;    /* characters introducing options; default="-"   */

  W32_DATA enum _optmode {
           GETOPT_UNIX,        /* options at start of argument list (default)   */
           GETOPT_ANY,         /* move non-options to the end                   */
           GETOPT_KEEP,        /* return options in order                       */
         } optmode;

  W32_FUNC int getopt (int argc, char *const *argv, const char *opt_str);
#endif


/*
 * Statistics printing
 */
W32_FUNC void print_mac_stats  (void);
W32_FUNC void print_pkt_stats  (void);
W32_FUNC void print_vjc_stats  (void);
W32_FUNC void print_arp_stats  (void);
W32_FUNC void print_pppoe_stats(void);
W32_FUNC void print_ip4_stats  (void);
W32_FUNC void print_ip6_stats  (void);
W32_FUNC void print_icmp_stats (void);
W32_FUNC void print_igmp_stats (void);
W32_FUNC void print_udp_stats  (void);
W32_FUNC void print_tcp_stats  (void);
W32_FUNC void print_all_stats  (void);
W32_FUNC void reset_stats      (void);

#if !defined(WATT32_BUILD)
W32_FUNC int sock_stats (void *s, DWORD *days, WORD *inactive,
                         WORD *cwindow, DWORD *avg,  DWORD *sd);
#endif


/*
 * Controlling timer interrupt handler for background processing.
 * Not recommended, little tested
 */
W32_FUNC void backgroundon  (void);
W32_FUNC void backgroundoff (void);
W32_FUNC void backgroundfn  (VoidProc func);


/*
 * Misc functions
 */
#if !defined(__DJGPP__) && !(defined(__WATCOMC__) && (__WATCOMC__ >= 1240))
  #define ffs W32_NAMESPACE (ffs)
  W32_FUNC int ffs (int mask);
#endif

#if defined (__HIGHC__)
  W32_FUNC int system (const char *cmd);
  pragma Alias (system, "_mw_watt_system");
#endif

W32_FUNC unsigned Random (unsigned a, unsigned b);


/*
 * Tracing to RS-232 serial port, by Gundolf von Bachhaus <GBachhaus@gmx.net>
 * Watt-32 library must be compiled with `USE_RS232_DBG' (see .\src\config.h)
 */
W32_FUNC int            trace2com_init (WORD portAddress, DWORD baudRate);
W32_FUNC int MS_CDECL __trace2com      (const char *fmt, ...)
  #if defined(__GNUC__)
  __attribute__((format(printf,1,2)))
  #endif
  ;

#ifdef __cplusplus
}
#endif

#endif  /* RC_INVOKED */
#endif  /* __WATT_TCP_H */
