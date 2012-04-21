/*
	host_string.h
	internationalized string resource shared between client and server

	$Id$
*/

#ifndef HOST_STRING_H
#define HOST_STRING_H

extern	int		host_string_count;

void Host_LoadStrings (void);
const char *Host_GetString (int idx);

#endif	/* HOST_STRING_H */

