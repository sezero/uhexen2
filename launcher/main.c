
#include "common.h"

// whether to use the password file to determine
// the path to the home directory
#define USE_PASSWORD_FILE	0

#if USE_PASSWORD_FILE
#include <pwd.h>
#endif

#include "launcher_defs.h"
#include "config_file.h"
#include "interface.h"
//#include "support.h"

static char	bin_dir[MAX_OSPATH];
char		userdir[MAX_OSPATH];

static char *Sys_SearchCommand (char *filename)
{
	static char	pathname[MAX_OSPATH];
	char	buff[MAX_OSPATH];
	char	*path;
	int		m, n;

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
		{
			printf ("Insufficient buffer size for pathnames\n");
			return NULL;
		}
		strncpy(buff, path, n);

		if (n && buff[n - 1] != '/')
		{
			buff[n++] = '/';
		}

		if (strlen(filename) >= sizeof(buff)-n)
		{
			printf ("Insufficient buffer size for pathnames\n");
			return NULL;
		}
		strcpy(buff + n, filename);

		if (!access(buff, F_OK))
		{
			if ( realpath(buff, pathname) == NULL )
			{
				printf ("Unable to resolve pathname %s\n", buff);
				return NULL;
			}
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
	{
		printf ("Unable to determine realpath for %s\n", filename);
		exit (1);
	}
	last = cmd;
	tmp = cmd;

	while (*tmp)
	{
		if (*tmp == '/')
			last = tmp+1;
		tmp++;
	}

	printf("Launcher : %s\n",last);
	strncpy (out, cmd, (strlen(cmd)-strlen(last)) );
}

static int Sys_mkdir (char *path)
{
	int	rc;

	rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;

	return rc;
}

static int Sys_GetUserdir (char *buff, size_t path_len)
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

	if (strlen(home_dir) + strlen(AOT_USERDIR) + strlen(LAUNCHER_CONFIG_FILE) + 2 >= path_len)
	{
		printf ("Insufficient buffer size for user directory name\n");
		return 1;
	}

	snprintf (buff, path_len, "%s/%s", home_dir, AOT_USERDIR);
	return Sys_mkdir(buff);
}


int main (int argc, char *argv[])
{

	GtkWidget *window1;
/*
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif
	gtk_set_locale ();
*/

	gtk_init (&argc, &argv);

/*	add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
*/

	printf("Hexen II: Hammer of Thyrion Launcher, version %s\n", LAUNCHER_VERSION_STR);

	if ((Sys_GetUserdir(userdir, sizeof(userdir))) != 0)
	{
		fprintf (stderr,"Couldn't determine userspace directory");
		exit(1);
	}

	memset(bin_dir, 0, sizeof(bin_dir));
	Sys_FindBinDir(argv[0], bin_dir);
	printf("Basedir  : %s\n",bin_dir);
	printf("Userdir  : %s\n",userdir);

	read_config_file();

//	go into the binary's directory
	chdir(bin_dir);

/*
 * The following code was added by Glade to create one of each component
 * (except popup menus), just so that you see something after building
 * the project. Delete any components that you don't want shown initially.
 */
	window1 = create_window1 ();
	gtk_widget_show (window1);

	gtk_main ();

	return 0;
}

