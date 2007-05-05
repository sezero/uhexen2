/*
	scriplib.h
	$Id: scriplib.h,v 1.1 2007-05-05 11:18:47 sezero Exp $
*/

#ifndef __SCRIPLIB_H
#define __SCRIPLIB_H

#define	MAXTOKEN	128

extern	char	token[MAXTOKEN];
extern	int		scriptline;
extern	qboolean	endofscript;

void LoadScriptFile (char *filename);
qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

#endif	/* __SCRIPLIB_H */

