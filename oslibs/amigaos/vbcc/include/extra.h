#pragma begin_header
#ifndef __EXTRA_H
#define __EXTRA_H 1

#include <time.h>
#include <string.h>

int chdir(const char *);
clock_t clock(void);
int getch(void);
int isseparator(int);
int iswhitespace(int);
int stricmp(const char *,const char *);
int strnicmp(const char *,const char *,size_t);

#endif
#pragma end_header
