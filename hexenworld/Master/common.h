/*
	common.h
	misc utilities used in client and server

	$Id: common.h,v 1.1 2007-02-12 16:54:48 sezero Exp $
*/

#ifndef __HX2_COMMON_H
#define __HX2_COMMON_H

extern	char		com_token[1024];

char *COM_Parse (char *data);

extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (const char *parm);
void COM_InitArgv (int argc, char **argv);

#endif	/* __HX2_COMMON_H */

