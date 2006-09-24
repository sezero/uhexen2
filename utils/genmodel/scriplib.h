// scriplib.h

#ifndef __SCRIPLIB_H
#define __SCRIPLIB_H

#ifndef __CMDLIB_H__
#include "cmdlib.h"
#endif

#define	MAXTOKEN	128

extern	char	token[MAXTOKEN];
extern	char	*scriptbuffer, *script_p, *scriptend_p;
extern	int		grabbed;
extern	int		scriptline;
extern	qboolean	endofscript;

void LoadScriptFile (char *filename);
qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

#endif	/* __SCRIPLIB_H */

