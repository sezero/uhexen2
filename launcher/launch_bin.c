#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <errno.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "launch_bin.h"

#ifndef DEMOBUILD
extern int mp_support;
extern int with_om;
//extern int iamevil;
#endif
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int joystick;
extern int mouse;
extern int destiny;

pid_t pid=0;
char *h2_binary_names[]={

  "glh2mp",	/* OpenGL and Mission Pack */	/* 0 */
  "glhexen2",	/* GL and no MP */		/* 1 */
  "h2mp",	/* software and MP */		/* 2 */
  "hexen2"	/* software and no MP */	/* 3 */
};

char *hw_binary_names[]={

  "hwcl",	/* Software */	/* 0 */
  "glhwcl"	/* OpenGL */	/* 1 */
};

/* [with or without OpenGL][with or without MP] */
/* for example, table [1][0] is with openGL and without MP */
int table[2][2] = {  {3,2}, {1,0}  };

/* [resolution]
  -width values only. corresponding -height is is the game binary */
char *resolution_args[]={

  "320",
  "400",
  "512",
  "640",
  "800",
  "1024",
  "1280"
};

static char *bin_path;
extern char *argv_0;

static char * search_for_command(char * filename) {
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


char *SkipPath (char *pathname) {
  char *last;
	
  last = pathname;
  while (*pathname) {
      if (*pathname=='/')
	last = pathname+1;
      pathname++;
  }
  return last;
}


void SkipFilename (char *out, char *pathname, int size) {

  int base_size=0;
  char *name=0;

  name = SkipPath(pathname);
  base_size=strlen(pathname)-strlen(name);
  strncpy(out,bin_path,base_size);
}

void launch_hexen2_bin() {

  char directory_name[1024];
  char *binary_name;
  unsigned short i=0, i1=0;
  char *args[12];

  if (destiny == DEST_H2)
#ifndef DEMOBUILD
     binary_name=h2_binary_names[table[opengl_support][mp_support]];
#else
     binary_name=h2_binary_names[table[opengl_support][0]];
#endif
  else if (destiny == DEST_HW)
     binary_name=hw_binary_names[opengl_support];
  else {
     printf("Warning: unknown destiny choice, launhcing Hexen II\n");
#ifndef DEMOBUILD
     binary_name=h2_binary_names[table[opengl_support][mp_support]];
#else
     binary_name=h2_binary_names[table[opengl_support][0]];
#endif
  }

  memset(directory_name,0,1024);
  memset(args,0,1024);

  bin_path=search_for_command(argv_0);

  SkipFilename(directory_name,bin_path,1024);
  chdir(directory_name);

  args[i]=binary_name;	// i == 0

  i++;			// i == 1
  args[i]="-f";		// args[i]="--fullscreen";
  if (fullscreen == 0)
    args[i]="-w";	// args[i]="--windowed";

  i++;			// i == 2
  args[i]="-width";

  i++;			// i == 3
  args[i]=resolution_args[resolution];

  if (sound == 0) {
    i++;
    args[i]="-s";	// args[i]="--nosound";
  } else {
	if (midi == 0) {
	    i++;
	    args[i]="-nomidi";
	}
	if (cdaudio == 0) {
	    i++;
	    args[i]="-nocdaudio";
	}
  }

  if (joystick == 0) {
    i++;
    args[i]="-nojoy";
  }

  if (mouse == 0) {
    i++;
    args[i]="-nomouse";
  }

#ifndef DEMOBUILD
  if (mp_support) {
    if (with_om)  {
       i++;
       args[i]="-witholdmission";
/*     if (iamevil) {
         i++;
	 args[i]="-withdemoness";
       } */
    } else {
       i++;
       args[i]="-noold";
    }
  }
#endif

  i++;
  args[i]=NULL;

  printf("We are here: %s\n",bin_path);
  printf("Base       : %s\n",directory_name);
  printf("\nLaunching %s\n",binary_name);
  printf("Command line is :\n  ");
  for (i1 = 0; i1 <= i - 1; i1++)
    printf(" %s", args[i1]);
  printf("\n\n");

  pid=fork();

  if (pid == 0) {
	pid=fork();
	if (pid == 0) {
	      execv(binary_name,args);
	      gtk_main_quit();
	} else {
	     if (pid == -1)
		     perror(NULL);
	     gtk_main_quit();
	}
  } else {
	if (pid == -1)
	   perror(NULL);
	gtk_main_quit();
  }
}
