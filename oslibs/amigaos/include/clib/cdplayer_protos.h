#ifndef CDPLAYER_PROTOS_H
#define CDPLAYER_PROTOS_H

/*
	Prototypes für die cdplayer.library  © 1995 by Patrick Hess
	$VER: cdplayer_protos.h 1.0 (29.05.1995)
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

BYTE CDEject (struct IOStdReq *);
BYTE CDPlay (UBYTE, UBYTE, struct IOStdReq *);
BYTE CDResume (BOOL, struct IOStdReq *);
BYTE CDStop (struct IOStdReq *);
BYTE CDJump (ULONG, struct IOStdReq *);
BOOL CDActive (struct IOStdReq *);
ULONG CDCurrentTitle (struct IOStdReq *);
BYTE CDTitleTime (struct CD_Time *, struct IOStdReq *);
BYTE CDGetVolume (struct CD_Volume *, struct IOStdReq *);
BYTE CDSetVolume (struct CD_Volume *, struct IOStdReq *);
BYTE CDReadTOC (struct CD_TOC *, struct IOStdReq *);
BYTE CDInfo (struct CD_Info *, struct IOStdReq *);

#endif

