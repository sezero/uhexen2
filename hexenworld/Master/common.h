/*
	common.h
	misc utilities used in client and server

	$Id: common.h,v 1.2 2007-04-08 08:50:42 sezero Exp $
*/

#ifndef __HX2_COMMON_H
#define __HX2_COMMON_H

extern	char		com_token[1024];

char *COM_Parse (char *data);

extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (const char *parm);

#endif	/* __HX2_COMMON_H */

