/*
	main.c
	hexen2 launcher: main loop

	$Id$

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#include "common.h"
#include "q_endian.h"
#include <stdarg.h>

#if USE_PASSWORD_FILE
#include <pwd.h>
#endif

#include "launcher_defs.h"
#include "games.h"
#include "config_file.h"
#include "launcher_ui.h"

char		basedir[MAX_OSPATH];
char		userdir[MAX_OSPATH];

static void
   __attribute__((__format__(__printf__,2,3), __noreturn__))
Sys_Error (int exitcode, const char *error, ...)
{
	va_list		argptr;
	char		text[4096];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	fprintf(stderr, "%s\n", text);
	ui_error (text);
	ui_quit ();	/* shouldn't be necessary.. */

	exit (exitcode);
}

static char *Sys_SearchCommand (char *filename)
{
	static char	pathname[MAX_OSPATH];
	char	buff[MAX_OSPATH];
	char	*path;
	size_t		m, n;

	memset (pathname, 0, sizeof(pathname));

	if (filename[0] == '/' || filename[0] == '.' || strchr(filename, '/') != NULL)
	{
		if ( realpath(filename, pathname) == NULL )
		{
			printf ("Unable to resolve pathname %s\n", filename);
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
			return NULL;
		strncpy(buff, path, n);

		if (n && buff[n - 1] != '/')
		{
			buff[n++] = '/';
		}

		if (strlen(filename) >= sizeof(buff) - n)
			return NULL;
		strcpy(buff + n, filename);

		if (!access(buff, F_OK))
		{
			if ( realpath(buff, pathname) == NULL )
				return NULL;
			return pathname;
		}
	}

	return NULL;
}

static void Sys_FindBinDir (char *filename, char *out)
{
	char	*cmd, *last, *tmp;

	cmd = Sys_SearchCommand (filename);
	if (cmd == NULL)
		Sys_Error (1, "Unable to determine realpath for %s", filename);

	last = cmd;
	tmp = cmd;

	while (*tmp)
	{
		if (*tmp == '/')
			last = tmp + 1;
		tmp++;
	}

	printf("Launcher : %s\n", last);
	if (last > cmd && last - 1 != cmd)
		last--;		/* exclude the trailing slash */
	while (cmd < last)
		*out++ = *cmd++;
}

static int Sys_mkdir (char *path)
{
	int	rc;

	rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;

	return rc;
}

static int Sys_GetUserdir (char *dst, size_t dstsize)
{
	char		*home_dir = NULL;
#if USE_PASSWORD_FILE
	struct passwd	*pwent;

	pwent = getpwuid( getuid() );
	if (pwent == NULL)
		perror("getpwuid");
	else
		home_dir = pwent->pw_dir;
#endif
	if (home_dir == NULL)
		home_dir = getenv("HOME");
	if (home_dir == NULL)
		return 1;

	if (strlen(home_dir) + strlen(AOT_USERDIR) + strlen(LAUNCHER_CONFIG_FILE) + 2 >= dstsize)
		return 1;

	snprintf (dst, dstsize, "%s/%s", home_dir, AOT_USERDIR);
	return Sys_mkdir(dst);
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
		host_byteorder = -1;	/* error out */
		break;
	default:
		tmp = endianism[3];
		break;
	}
	if (host_byteorder < 0)
		Sys_Error (1, "Unsupported byte order [%s]", tmp);
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
		Sys_Error (1, "Detected byte order %s doesn't match compiled %s order!", tmp, tmp2);
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
		Sys_Error (ret, "Couldn't determine userspace directory");

	memset(basedir, 0, sizeof(basedir));
	Sys_FindBinDir (argv[0], basedir);
	printf ("Basedir  : %s\n", basedir);
	printf ("Userdir  : %s\n", userdir);

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

