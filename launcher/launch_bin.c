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

extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int sound;
extern int destiny;

pid_t pid=0;
char *h2_binary_names[]={

  "glh2mp",   /* OpenGL and Mission Pack */    /* 0 */
  "glhexen2", /* GL and no MP */               /* 1 */
  "h2mp",     /* software and MP */            /* 2 */
  "hexen2"    /* software and no MP */         /* 3 */

};

char *hw_binary_names[]={

  "hwcl",     /* Software */  /* 0 */
  "glhwcl",   /* OpenGL */    /* 1 */

};

/* [with or without OpenGL][with or without MP] */
/* for example, table [1][0] is with openGL and without MP */
int table[2][2] = { {3,2},{1,0} };

/* [resolution][fullscreen] */
char *gl_resolution_args[3][2]={

  {"640","480"}, {"800","600"}, {"1024","768"},

};

//char *nosound_flag="--nosound";
char *nosound_flag="-s";
//char *windowed_flag="--windowed";
char *windowed_flag="-w";

static char *bin_path;
extern char *argv_0;

static char * search_for_command(char * filename)
{
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
	if (readlink(buff,pathname,1024) < 0) {
	  if (errno == EINVAL)  /* not a symbolic link */ { }
	  else perror(NULL);
	}

	return pathname;
    }
  }
  return filename;
}


char *SkipPath (char *pathname)
{
  char *last;
	
  last = pathname;
  while (*pathname)
    {
      if (*pathname=='/')
	last = pathname+1;
      pathname++;
    }
  return last;
}

void SkipFilename (char *out, char *pathname, int size)
{
  int base_size=0;
  char *name=0;

  name = SkipPath(pathname);
  base_size=strlen(pathname)-strlen(name);
  strncpy(out,bin_path,base_size);
}

void launch_hexen2_bin()
{
  char directory_name[1024];
  char *binary_name;
  char *args[7];
  int i=0;

  if (destiny == DEST_H2)
    binary_name=h2_binary_names[table[opengl_support][mp_support]];

  else if (destiny == DEST_HW)
     binary_name=hw_binary_names[opengl_support];

  else {
    printf("Warning: unknown destiny choice, launhcing Hexen II\n");
    binary_name=h2_binary_names[table[opengl_support][mp_support]];
  }

  memset(directory_name,0,1024);
  memset(args,0,1024);

  bin_path=search_for_command(argv_0);

  SkipFilename(directory_name,bin_path,1024);
  chdir(directory_name);

  args[i]=binary_name; 
  i++;
  
  if (sound == 0) {
    args[i]=nosound_flag;
    i++;
  }

//args[i]="--fullscreen";
  args[i]="-f";
  if (fullscreen == 0)
    args[i]=windowed_flag;
  i++;

  if (opengl_support) {
    args[i]="-width";
    i++;
    args[i]=gl_resolution_args[resolution][0];
    i++;
    args[i]="-height";
    i++;
    args[i]=gl_resolution_args[resolution][1];
  }

// args[i]=NULL;

  printf("we are here: %s\n",bin_path);
  printf("base: %s\n",directory_name);
  printf("launching %s\n",binary_name);
  printf("using arguments :\n");
  for (i=0;i<=6;i++)
    printf("%d:  %s\n",i,args[i]);

  pid=fork();
  if (pid == 0) {
    pid=fork();
    if (pid == 0) {
      execv(binary_name,args);
    }
    else {if (pid == -1) perror(NULL); gtk_main_quit();}
  }
  else {if (pid == -1) perror(NULL); gtk_main_quit();}
}
