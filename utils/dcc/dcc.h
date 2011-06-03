/*
	dhcc.h

	$Id$
*/

#ifndef __DHCC_H__
#define __DHCC_H__

extern void	Init_Dcc (void);
extern void	DEC_ReadData (const char *srcfile);
extern void	Dcc_Functions (void);
extern void	FindBuiltinParameters (int func);
extern void	DccFunctionOP (unsigned short op);
extern void	PR_PrintFunction (const char *name);

extern int	FILE_NUM_FOR_NAME;

#endif	/* __DHCC_H__ */

