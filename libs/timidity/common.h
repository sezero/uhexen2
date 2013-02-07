/*
 * TiMidity -- Experimental MIDI to WAVE converter
 * Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
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
 *
 * common.h
 */

#ifndef TIMIDITY_COMMON_H
#define TIMIDITY_COMMON_H

#include <setjmp.h>

typedef struct _PathList {
  char *path;
  struct _PathList *next;
} PathList;

extern FILE *open_file(const char *name);
extern int add_to_pathlist(const char *s, size_t len);
extern void free_pathlist(void);

extern jmp_buf safe_malloc_jmp;

extern void *safe_malloc(size_t count);
#ifndef CANT_FREE_NULL
#define safe_free free
#else
#define safe_free(_X) if((_X)) free((_X))
#endif

#endif /* TIMIDITY_COMMON_H */
