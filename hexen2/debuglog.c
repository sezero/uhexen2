/*
	debuglog.c
	logging console output to a file

	$Id: debuglog.c,v 1.12 2010-01-23 12:01:23 sezero Exp $
*/

#include "quakedef.h"
#include "debuglog.h"
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef PLATFORM_WINDOWS
#include <io.h>		/* write() */
#include "io_msvc.h"
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>	/* write() */
#endif
#ifdef PLATFORM_DOS	/* __DJGPP */
#include <unistd.h>	/* write() */
#endif


unsigned int		con_debuglog	= LOG_NONE;

static int			log_fd = -1;
static char		logfilename[MAX_OSPATH];	/* current logfile name	*/
static char		logbuff[MAX_PRINTMSG];		/* our log text buffer	*/

static const char	separator_line[] = "=======================================\n";

void LOG_Print (const char *logdata)
{
	if (!logdata || !*logdata)
		return;
	if (log_fd == -1)
		return;

	write (log_fd, logdata, strlen(logdata));
}

void LOG_Printf (const char *fmt, ...)
{
	va_list		argptr;

	va_start (argptr, fmt);
	q_vsnprintf (logbuff, sizeof(logbuff), fmt, argptr);
	va_end (argptr);
	LOG_Print (logbuff);
}

static void LOG_PrintVersion (void)
{
/* repeating the PrintVersion() messages from main() here */
#if HOT_VERSION_BETA
	LOG_Printf("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	LOG_Printf("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
#if defined(SERVERONLY) && !defined(H2W)
	LOG_Printf("Hexen II dedicated server %4.2f (%s)\n", ENGINE_VERSION, PLATFORM_STRING);
#else
	LOG_Printf("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, PLATFORM_STRING);
#endif
}

void LOG_Init (quakeparms_t *parms)
{
	time_t		inittime;
	int			i, j;
	char		session[24];

	inittime = time (NULL);
	strftime (session, sizeof(session), "%m/%d/%Y %H:%M:%S", localtime(&inittime));
	q_snprintf (logfilename, sizeof(logfilename), "%s/%s", parms->userdir, DEBUGLOG_FILENAME);

	if ( COM_CheckParm("-condebug") || COM_CheckParm("-debuglog") )
	{
		con_debuglog |= LOG_NORMAL;
	}
	if ( COM_CheckParm("-devlog") )
	{
	/* log the Con_DPrintf and Sys_DPrintf content when !developer.integer */
		con_debuglog |= LOG_DEVEL;
	}

	if (con_debuglog == LOG_NONE)
		return;

	Sys_unlink (logfilename);

	log_fd = open (logfilename, O_WRONLY | O_CREAT | O_APPEND, 0666);
	if (log_fd == -1)
	{
		con_debuglog = LOG_NONE;
		fprintf (stderr, "Error: Unable to create log file\n");
		return;
	}

	LOG_Printf("LOG started on: %s - LOG LEVEL: %s\n", session, (con_debuglog & LOG_DEVEL) ? "full" : "normal");

	/* build the commandline args as a string */
	q_strlcpy (logbuff, "Command line: ", sizeof(logbuff));
	for (i = 0, j = 0; i < parms->argc; i++)
	{
		if (parms->argv[i][0] && parms->argv[i][0] != ' ')
		{
			q_strlcat (logbuff, parms->argv[i], sizeof(logbuff));
			q_strlcat (logbuff, " ", sizeof(logbuff));
			j++;
		}
	}
	if (j)
	{
		logbuff[sizeof(logbuff)-2] = 0;
		q_strlcat (logbuff, "\n", sizeof(logbuff));
		LOG_Print (logbuff);
	}
	else
	{
		q_strlcat (logbuff, "(none)\n", sizeof(logbuff));
		LOG_Print (logbuff);
	}

	/* print the version information to the log */
	LOG_PrintVersion ();
	LOG_Print (separator_line);
}

void LOG_Close (void)
{
	if (log_fd == -1)
		return;
	close (log_fd);
	log_fd = -1;
}

