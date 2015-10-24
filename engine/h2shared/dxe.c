/* Dynamic module loading/unloading with DJGPP DXE3
 * Copyright (C) 2015 Q2DOS developers.
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

#include <dlfcn.h>
#include <sys/dxe.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <setjmp.h>
#include <crt0.h>
#if 0
#include <ctype.h>
#endif

#include "quakedef.h"
#include "sys_dxe.h"

DXE_EXPORT_TABLE (syms)
	DXE_EXPORT (__dj_assert)
	DXE_EXPORT (__dj_huge_val)
	DXE_EXPORT (__dj_stderr)
#if 0
	DXE_EXPORT (__dj_ctype_tolower)
	DXE_EXPORT (__dj_ctype_toupper)
	DXE_EXPORT (__dj_ctype_flags)
#endif
	DXE_EXPORT (bsearch)
	DXE_EXPORT (acos)
	DXE_EXPORT (asin)
	DXE_EXPORT (atan)
	DXE_EXPORT (atan2)
	DXE_EXPORT (atof)
	DXE_EXPORT (atoi)
	DXE_EXPORT (ceil)
	DXE_EXPORT (cos)
	DXE_EXPORT (errno)
	DXE_EXPORT (exit)
	DXE_EXPORT (fclose)
	DXE_EXPORT (feof)
	DXE_EXPORT (fgetc)
	DXE_EXPORT (fgets)
	DXE_EXPORT (floor)
	DXE_EXPORT (fopen)
	DXE_EXPORT (fprintf)
	DXE_EXPORT (fputc)
	DXE_EXPORT (fputs)
	DXE_EXPORT (fread)
	DXE_EXPORT (free)
	DXE_EXPORT (fscanf)
	DXE_EXPORT (fseek)
	DXE_EXPORT (fwrite)
	DXE_EXPORT (getc)
	DXE_EXPORT (ungetc)
	DXE_EXPORT (malloc)
	DXE_EXPORT (calloc)
	DXE_EXPORT (realloc)
	DXE_EXPORT (memcmp)
	DXE_EXPORT (memcpy)
	DXE_EXPORT (memset)
	DXE_EXPORT (memmove)
	DXE_EXPORT (pow)
	DXE_EXPORT (printf)
	DXE_EXPORT (putc)
	DXE_EXPORT (puts)
	DXE_EXPORT (qsort)
	DXE_EXPORT (sin)
	DXE_EXPORT (sprintf)
	DXE_EXPORT (sqrt)
	DXE_EXPORT (sscanf)
	DXE_EXPORT (stpcpy)
	DXE_EXPORT (strcat)
	DXE_EXPORT (strchr)
	DXE_EXPORT (strcmp)
	DXE_EXPORT (strcpy)
	DXE_EXPORT (strdup)
	DXE_EXPORT (strlen)
	DXE_EXPORT (strncmp)
	DXE_EXPORT (strncpy)
	DXE_EXPORT (strrchr)
	DXE_EXPORT (strstr)
	DXE_EXPORT (strtod)
	DXE_EXPORT (strtol)
	DXE_EXPORT (tan)
	DXE_EXPORT (usleep)
	DXE_EXPORT (vsprintf)
	DXE_EXPORT (vsnprintf)
	DXE_EXPORT (__dpmi_int)
	DXE_EXPORT (__dpmi_physical_address_mapping)

	/* FS: ref_gl */
	DXE_EXPORT (dlclose)
	DXE_EXPORT (dlopen)
	DXE_EXPORT (dlsym)

	/* FS: 3dfx */
	DXE_EXPORT (__dj_stdout)
	DXE_EXPORT (__djgpp_base_address)
	DXE_EXPORT (__djgpp_nearptr_enable)
	DXE_EXPORT (__djgpp_nearptr_disable)
	DXE_EXPORT (__dpmi_free_physical_address_mapping)
	DXE_EXPORT (_crt0_startup_flags)
	DXE_EXPORT (abort)
	DXE_EXPORT (atol)
	DXE_EXPORT (clock)
	DXE_EXPORT (exp)
	DXE_EXPORT (fflush)
	DXE_EXPORT (frexp)
	DXE_EXPORT (freopen)
	DXE_EXPORT (getenv)
	DXE_EXPORT (enable)
	DXE_EXPORT (disable)
	DXE_EXPORT (int86)
	DXE_EXPORT (longjmp)
	DXE_EXPORT (putenv)
	DXE_EXPORT (setjmp)
	DXE_EXPORT (signal)
	DXE_EXPORT (strcspn)
	DXE_EXPORT (strncat)
	DXE_EXPORT (strtok)
	DXE_EXPORT (strtoul)
	DXE_EXPORT (vfprintf)

	DXE_EXPORT (ldexp)	/* for Mesa 6.4.x */
DXE_EXPORT_END


static int num_unres;

static int dxe_fail ()
{
	return 0;
}

static void *dxe_res (const char *sym)
{
	FILE *f = fopen ("dxe.log", "a");
	fprintf (f, "%s: unresolved symbol.\n", sym);
	fflush (f);
	fclose (f);
	++num_unres;
	return (void *)dxe_fail;
}

void Sys_InitDXE3 (void)
{
	remove ("dxe.log");

	/* Register the symbols exported into dynamic modules */
	dlregsym (syms);
}

void *Sys_dlopen (const char *filename, qboolean globalmode)
{
	void *lib;

	_dlsymresolver = dxe_res;
	num_unres = 0;
	lib = dlopen (filename, (globalmode)? RTLD_GLOBAL : 0);
	_dlsymresolver = NULL;
	if (num_unres)
		Sys_Error ("Unresolved symbol(s) in %s. See DXE.LOG for details.", filename);

	return lib;
}

void *Sys_dlsym (void *handle, const char *symbol)
{
	return dlsym (handle, symbol);
}

int Sys_dlclose (void *handle)
{
	return dlclose (handle);
}
