/*
	scriplib.h
	$Id: scriplib.h,v 1.2 2007-11-11 16:11:46 sezero Exp $
*/

#ifndef __SCRIPLIB_H
#define __SCRIPLIB_H

#define	MAXTOKEN	128

extern	char	token[MAXTOKEN];
extern	int		scriptline;
extern	qboolean	endofscript;

void LoadScriptFile (const char *filename);
qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

#endif	/* __SCRIPLIB_H */

