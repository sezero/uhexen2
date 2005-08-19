/*
 * cmdlib.h
 *
 * $Header: /home/ozzie/Download/0000/uhexen2/utils/h2_utils/hcc/cmdlib.h,v 1.2 2005-08-19 17:50:43 sezero Exp $
 */

#ifndef __GNUC__
#define Q_strncasecmp(s1,s2,n) strnicmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) stricmp((s1),(s2))
#else
#define Q_strncasecmp(s1,s2,n) strncasecmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) strcasecmp((s1),(s2))
#endif

