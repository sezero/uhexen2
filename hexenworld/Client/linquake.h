/*
	linquake.h
	Unix-specific Quake header file

	$Id: linquake.h,v 1.2 2005-05-21 17:04:17 sezero Exp $
*/

#include "linux_inc.h"

void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);
void IN_MouseEvent (int mstate);

extern cvar_t		_enable_mouse;

extern qboolean	mouseinitialized;

struct sockaddr;
int (PASCAL FAR *pWSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
int (PASCAL FAR *pWSACleanup)(void);
int (PASCAL FAR *pWSAGetLastError)(void);
SOCKET (PASCAL FAR *psocket)(int af, int type, int protocol);
int (PASCAL FAR *pioctlsocket)(SOCKET s, long cmd, u_long FAR *argp);
int (PASCAL FAR *psetsockopt)(SOCKET s, int level, int optname,
							  const char FAR * optval, int optlen);
int (PASCAL FAR *precvfrom)(SOCKET s, char FAR * buf, int len, int flags,
							struct sockaddr FAR *from, int FAR * fromlen);
int (PASCAL FAR *psendto)(SOCKET s, const char FAR * buf, int len, int flags,
						  const struct sockaddr FAR *to, int tolen);
int (PASCAL FAR *pclosesocket)(SOCKET s);
int (PASCAL FAR *pgethostname)(char FAR * name, int namelen);
struct hostent FAR * (PASCAL FAR *pgethostbyname)(const char FAR * name);
struct hostent FAR * (PASCAL FAR *pgethostbyaddr)(const char FAR * addr,
												  int len, int type);
int (PASCAL FAR *pgetsockname)(SOCKET s, struct sockaddr FAR *name,
							   int FAR * namelen);

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/05/20 15:27:37  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 *
 * 2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * 2004/12/18 13:44:12  sezero
 * Clean-up and kill warnings 1:
 * Kill two pragmas that are ignored anyway.
 *
 * 2004/12/04 19:51:43  sezero
 * Kill more warnings (add a forward declaration of
 * struct sockaddr in winquake.h)
 *
 * 2004/11/28 00:08:26  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 2001/12/02 04:59:43  theoddone33
 * Fix nvidia extention problem and a whole bunch of other stuff too apparently
 *
 * 5     7/17/97 2:00p Rjohnson
 * Added a security means to control the running of the game
 * 
 * 4     3/07/97 2:34p Rjohnson
 * Id Updates
 */
