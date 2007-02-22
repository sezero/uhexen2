/*
	cfgfile.h
	misc reads from the config file

	$Id: cfgfile.h,v 1.1 2007-02-22 07:36:22 sezero Exp $
*/

#ifndef __CFGFILE_H
#define __CFGFILE_H

int CFG_OpenConfig (const char *cfg_name);
// opens the given config file and keeps it open
// until CFG_CloseConfig is called

void CFG_CloseConfig (void);
// closes the currently open config file

void CFG_ReadCvars (const char **vars, const int num_vars);
// reads the values of cvars in the given list
// from the currently open config file

#endif	/* __CFGFILE_H */

