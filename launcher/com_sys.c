#include "com_sys.h"
#include "launcher_defs.h"


static char * Sys_SearchCommand(char * filename) {
	static char pathname[1024];
	char buff[1024];
	char *path;
	int m, n;

	if (strchr (filename, '/') && filename[0] != '.') {
		return filename;
	}

	if (filename[0] == '.') {
		char *cwd;

		cwd = malloc(sizeof(char)*1024);
		if (getcwd (cwd, 1024) == NULL) {
			perror("getcwd failed");
		}
		snprintf(pathname, 1024,"%s%s", cwd, filename+1);
		free(cwd);
		return pathname;
	}

	for (path = getenv("PATH"); path && *path; path += m) {

		if (strchr(path, ':')) {
			n = strchr(path, ':') - path;
			m = n + 1;
		} else {
			m = n = strlen(path);
		}

		strncpy(pathname, path, n);     

		if (n && pathname[n - 1] != '/') {
			pathname[n++] = '/';
		}

		strcpy(pathname + n, filename);

		if (!access(pathname, F_OK)) {

			strncpy(buff, pathname, 1024);

			if (readlink(buff, pathname, 1024) < 0) {
				if (errno == EINVAL) {
				  /* not a symbolic link */
				} else
					perror(NULL);
			}

			return pathname;
		}
	}
	return filename;
}

void Sys_FindBinDir (char *filename, char *out) {

	char *cmd, *last, *tmp;

	cmd = Sys_SearchCommand (filename);
	last = cmd;
	tmp = cmd;

	while (*tmp) {
		if (*tmp=='/')
			last = tmp+1;
		tmp++;
	}

	printf("Launcher : %s\n",last);
	strncpy(out,cmd,(strlen(cmd)-strlen(last)));
}

void Sys_mkdir (char *path) {

	mkdir (path, 0777);
}

int Sys_GetUserdir(char *buff, unsigned int len) {

	struct passwd *pwent;

	pwent = getpwuid( getuid() );

	if ( pwent == NULL ) {
		perror( "getpwuid" );
		return 0;
	}

	if ( strlen( pwent->pw_dir ) + strlen( AOT_USERDIR) + 2 > (unsigned)len ) {
		return 0;
	}

	sprintf( buff, "%s/%s", pwent->pw_dir, AOT_USERDIR );
	Sys_mkdir(buff);

	return 1;
}
