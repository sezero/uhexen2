#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

// com_sys.c
int Sys_GetUserdir(char *buff, unsigned int len);
void Sys_FindBinDir (char *filename, char *out);
int Sys_mkdir (char *path);
int Sys_GetUserdir(char *buff, unsigned int len);

// launch_bin.c
void CheckExe (void);
void launch_hexen2_bin();

