/*
	common.h
	misc utilities used in client and server

	$Id: common.h,v 1.8 2010-08-31 13:32:08 sezero Exp $
*/

#ifndef __HX2_COMMON_H
#define __HX2_COMMON_H

#undef	min
#undef	max
#define	q_min(a, b)	(((a) < (b)) ? (a) : (b))
#define	q_max(a, b)	(((a) > (b)) ? (a) : (b))


extern	char		com_token[1024];

const char *COM_Parse (const char *data);

extern	int		com_argc;
extern	char		**com_argv;

int COM_CheckParm (const char *parm);

#endif	/* __HX2_COMMON_H */

