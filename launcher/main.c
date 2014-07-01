/*
 * main.c
 * hexen2 launcher: main loop
 *
 * $Id$
 *
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "common.h"
#include "q_endian.h"
#include <stdarg.h>
#include <pwd.h>

#include "launcher_defs.h"
#include "games.h"
#include "config_file.h"
#include "launcher_ui.h"

char		basedir[MAX_OSPATH];
char		userdir[MAX_OSPATH];

static void
__attribute__ ((__format__(__printf__,1,2), __noreturn__))
Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[4096];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	fprintf(stderr, "%s\n", text);
	ui_error (text);
	ui_quit ();	/* shouldn't be necessary.. */

	exit (EXIT_FAILURE);
}

static char *Sys_SearchCommand (const char *filename)
{
	static char	pathname[PATH_MAX];
	char	buff[PATH_MAX];
	char	*path;
	size_t	l, m, n;

	memset (pathname, 0, sizeof(pathname));
	l = strlen(filename);

	if (filename[0] == '/' || filename[0] == '.' || strchr(filename, '/') != NULL)
	{
		if ( realpath(filename, pathname) == NULL )
		{
			perror("realpath");
			return NULL;
		}
		return pathname;
	}

	for (path = getenv("PATH"); path && *path; path += m)
	{
		if (strchr(path, ':'))
		{
			n = strchr(path, ':') - path;
			m = n + 1;
		}
		else
		{
			m = n = strlen(path);
		}

		if (n >= sizeof(buff))
		{
			Sys_Error ("%s (%d): too small bufsize %d. needed %d.",
					__FILE__, __LINE__, (int)sizeof(buff), (int)n);
		}
		strncpy(buff, path, n);

		if (n && buff[n - 1] != '/')
			buff[n++] = '/';
		if (l + n >= sizeof(buff))
		{
			Sys_Error ("%s (%d): too small bufsize %d. needed %d.",
					__FILE__, __LINE__, (int)sizeof(buff), (int)(l + n));
		}
		strcpy(&buff[n], filename);

		if (!access(buff, F_OK))
		{
			if ( realpath(buff, pathname) == NULL )
			{
				perror("realpath");
				return NULL;
			}
			return pathname;
		}
	}

	return NULL;
}

static void Sys_FindBinDir (const char *filename, char *out, size_t outsize)
{
	char	*cmd, *last, *tmp;
	size_t	n;

	cmd = Sys_SearchCommand (filename);
	if (cmd == NULL)
		Sys_Error ("Unable to determine realpath for %s", filename);

	last = cmd;
	tmp = cmd;

	while (*tmp)
	{
		if (*tmp == '/')
			last = tmp + 1;
		tmp++;
	}

	printf("Launcher : %s\n", last);

	if (last == cmd)	/* shouldn't actually happen */
	{
		out[0] = '.';
		out[1] = '\0';
		return;
	}
	if (last - 1 != cmd)
		last--;		/* exclude the trailing slash */
	n = last - cmd;
	if (n >= outsize)
	{
		Sys_Error ("%s (%d): too small bufsize %d. needed %d.",
				__FILE__, __LINE__, (int)outsize, (int)n);
	}
	strncpy (out, cmd, n);
}

static void Sys_mkdir (const char *path)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
	{
		struct stat st;
		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
			rc = 0;
	}
	if (rc != 0)
	{
		rc = errno;
		Sys_Error ("Unable to create directory %s: %s", path, strerror(rc));
	}
}

static int Sys_GetUserdir (char *dst, size_t dstsize)
{
	size_t		n;
	const char	*home_dir = NULL;
	struct passwd	*pwent;

	pwent = getpwuid(getuid());
	if (pwent == NULL)
		perror("getpwuid");
	else	home_dir = pwent->pw_dir;
	if (home_dir == NULL)
		home_dir = getenv("HOME");
	if (home_dir == NULL)
		return 1;

	n = strlen(home_dir) + strlen(AOT_USERDIR) + strlen(LAUNCHER_CONFIG_FILE) + 2;
	if (n >= dstsize)
	{
		Sys_Error ("%s (%d): too small bufsize %d. needed %d.",
				__FILE__, __LINE__, (int)dstsize, (int)n);
	}

	snprintf (dst, dstsize, "%s/%s", home_dir, AOT_USERDIR);
	return 0;
}

static void ValidateByteorder (void)
{
	const char	*endianism[] = { "BE", "LE", "PDP", "Unknown" };
	const char	*tmp;

	ByteOrder_Init ();
	switch (host_byteorder)
	{
	case BIG_ENDIAN:
		tmp = endianism[0];
		break;
	case LITTLE_ENDIAN:
		tmp = endianism[1];
		break;
	case PDP_ENDIAN:
		tmp = endianism[2];
		host_byteorder = -1;	/* not supported */
		break;
	default:
		tmp = endianism[3];
		break;
	}
	if (host_byteorder < 0)
		Sys_Error ("Unsupported byte order [%s]", tmp);
	printf("Detected byte order: %s\n", tmp);
#if !ENDIAN_RUNTIME_DETECT
	if (host_byteorder != BYTE_ORDER)
	{
		const char	*tmp2;
		switch (BYTE_ORDER)
		{
		case BIG_ENDIAN:
			tmp2 = endianism[0];
			break;
		case LITTLE_ENDIAN:
			tmp2 = endianism[1];
			break;
		case PDP_ENDIAN:
			tmp2 = endianism[2];
			break;
		default:
			tmp2 = endianism[3];
			break;
		}
		Sys_Error ("Detected byte order %s doesn't match compiled %s order!", tmp, tmp2);
	}
#endif	/* ENDIAN_RUNTIME_DETECT */
}


int main (int argc, char **argv)
{
	int	ret;

	printf("Hexen II: Hammer of Thyrion Launcher, version %i.%i.%i\n",
		LAUNCHER_VERSION_MAJ, LAUNCHER_VERSION_MID, LAUNCHER_VERSION_MIN);

/* initialize the user interface */
	ret = ui_init(&argc, &argv);
	if (ret != 0)
	{
		fprintf (stderr, "Couldn't initialize user interface\n");
		exit (ret);
	}

	ValidateByteorder ();

	ret = Sys_GetUserdir(userdir, sizeof(userdir));
	if (ret != 0)
		Sys_Error ("Couldn't determine userspace directory");

	Sys_FindBinDir (argv[0], basedir, sizeof(basedir));
	printf ("Basedir  : %s\n", basedir);
	printf ("Userdir  : %s\n", userdir);
	Sys_mkdir(userdir);

/* go into the binary's directory */
	chdir (basedir);

	cfg_read_basedir();
	scan_game_installation();
	read_config_file();

/* run the graphical user interface */
	ret = ui_main ();
	if (ret == 0)
		ret = write_config_file ();

	return ret;
}

