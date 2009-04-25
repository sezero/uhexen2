/*
	host.h
	public host structures and functions

	$Id: host.h,v 1.10 2009-04-25 11:07:34 sezero Exp $
*/

#if !defined(SERVERONLY)
#error "this header is for hw server only"
#endif	/* SERVERONLY */

#ifndef __HX2_HOST_H
#define __HX2_HOST_H

// quakeparms structure specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct quakeparms_s
{
	const char *basedir;
	const char *userdir;	// user's directory on UNIX platforms.
				// if user directories are enabled, basedir
				// and userdir will point to different
				// memory locations, otherwise to the same.
	int	argc;
	char	**argv;
	void	*membase;
	int	memsize;
} quakeparms_t;


extern	quakeparms_t	*host_parms;

#define	isDedicated	1	/* compatibility */

extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;

extern	qboolean	host_initialized;	// true if into command execution
extern	double		host_frametime;
extern	double		realtime;		// not bounded in any way, changed at
						// start of every frame, never reset

void SV_Init (void);
void SV_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
#define Host_Error	SV_Error

#endif	/* __HX2_HOST_H */

