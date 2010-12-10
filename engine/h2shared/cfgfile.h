/*
	cfgfile.h
	misc reads from the config file

	$Id: cfgfile.h,v 1.2 2007-11-14 07:32:20 sezero Exp $
*/

#ifndef __CFGFILE_H
#define __CFGFILE_H

int CFG_OpenConfig (const char *cfg_name);
// opens the given config file and keeps it open
// until CFG_CloseConfig is called

void CFG_CloseConfig (void);
// closes the currently open config file

void CFG_ReadCvars (const char **vars, int num_vars);
// reads the values of cvars in the given list
// from the currently open config file

void CFG_ReadCvarOverrides (const char **vars, int num_vars);
// reads the "+" command line override values of cvars
// in the given list.  doesn't care about the config file.
// call this after CFG_ReadCvars() and before locking your
// cvars.

#endif	/* __CFGFILE_H */

