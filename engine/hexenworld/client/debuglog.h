/*
	debuglog.h
	logging console output to a file

	$Id: debuglog.h,v 1.3 2010-01-11 18:48:19 sezero Exp $
*/

#ifndef __DEBUGLOG_H
#define __DEBUGLOG_H

#define DEBUG_PREFIX		"DEBUG: "

/* log filenames: */
#if defined(H2W)
#define	DEBUGLOG_FILENAME	"debug_hw.log"
#elif defined(SERVERONLY)
#define	DEBUGLOG_FILENAME	"debugded.log"
#else
#define	DEBUGLOG_FILENAME	"debug_h2.log"
#endif

/* log level:	*/
#define	LOG_NONE		0	/* no logging	*/
#define	LOG_NORMAL		1	/* normal logging: what you see on the game console and terminal */
#define	LOG_DEVEL		2	/* log the _DPrintf content even if the developer cvar isn't set */

extern	unsigned int		con_debuglog;

void LOG_Print (const char *logdata);
void LOG_Printf(const char *fmt, ...) __attribute__((__format__(__printf__,1,2)));

struct quakeparms_s;
void LOG_Init (struct quakeparms_s *parms);
void LOG_Close (void);

#endif	/* __DEBUGLOG_H */

