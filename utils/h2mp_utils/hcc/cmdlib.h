/*
 * cmdlib.h
 *
 * $Header: /home/ozzie/Download/0000/uhexen2/utils/h2mp_utils/hcc/cmdlib.h,v 1.5 2005-09-17 08:19:05 sezero Exp $
 */

#ifdef _WIN32
#define Q_strncasecmp	strnicmp
#define Q_strcasecmp	stricmp
#else
#define Q_strncasecmp	strncasecmp
#define Q_strcasecmp	strcasecmp
#endif

