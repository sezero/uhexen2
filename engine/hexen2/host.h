/*
	host.h
	public host structures and functions

	$Id$
*/

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

extern	qboolean	isDedicated;

extern	cvar_t		sys_ticrate;
extern	cvar_t		sys_throttle;
extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;

extern	cvar_t		pausable;

extern	qboolean	host_initialized;	// true if into command execution
extern	double		host_frametime;
extern	byte		*host_basepal;
extern	byte		*host_colormap;
extern	int		host_framecount;	// incremented every frame, never reset
extern	double		realtime;		// not bounded in any way, changed at
						// start of every frame, never reset

void Host_Init (void);
void Host_InitCommands (void);
void Host_Shutdown(void);
/* Host_Error and Host_EndGame doesn't return either due to Sys_Error() or longjmp() */
void Host_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Host_EndGame (const char *message, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (const char *fmt, ...) __attribute__((__format__(__printf__,1,2)));
void Host_ShutdownServer (qboolean crash);

void Host_ClearMemory (void);

void Host_RemoveGIPFiles (const char *path);
void Host_DeleteSave (const char *savepath);
int Host_CopyFiles(const char *source, const char *pat, const char *dest);
int SaveGamestate (qboolean ClientsOnly);


extern	int		current_skill;	// skill level for currently loaded level (in case
					//  the user changes the cvar while the level is
					//  running, this reflects the level actually in use)

extern	int		sv_kingofhill;		// mission pack king of the hill.

extern	unsigned int	info_mask, info_mask2;	// mission pack objectives

#endif	/* __HX2_HOST_H */

