/*
	info_str.h
	Hexen2World info strings handling

	$Id: info_str.h,v 1.2 2007-09-30 11:05:49 sezero Exp $
*/

#ifndef __H2W_INFOSTR_H
#define __H2W_INFOSTR_H

#define	MAX_INFO_STRING			196
#define	MAX_SERVERINFO_STRING		512
#define	MAX_LOCALINFO_STRING		32768

char *Info_ValueForKey (const char *s, const char *key);
void Info_RemoveKey (char *s, const char *key);
void Info_RemovePrefixedKeys (char *start, char prefix);
void Info_SetValueForKey (char *s, const char *key, const char *value, size_t maxsize);
void Info_SetValueForStarKey (char *s, const char *key, const char *value, size_t maxsize);
void Info_Print (const char *s);

#endif	/* __H2W_INFOSTR_H */

