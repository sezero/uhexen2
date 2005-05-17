// cmdlib.h

/*
 * $Header: /home/ozzie/Download/0000/uhexen2/utils/h2mp_utils/hcc/cmdlib.h,v 1.2 2005-05-17 19:35:36 sezero Exp $
 */

#ifndef __GNUC__
#define Q_strcasecmp stricmp
#define Q_strncasecmp strnicmp
#else
#define Q_strcasecmp strcasecmp
#define Q_strncasecmp strncasecmp
#endif

