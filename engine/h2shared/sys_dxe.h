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

#ifndef HX2_DXE_H
#define HX2_DXE_H

void Sys_InitDXE3 (void);

void *Sys_dlopen (const char *filename, qboolean globalmode);
int Sys_dlclose (void *handle);
void *Sys_dlsym (void *handle, const char *symbol);

#endif	/* HX2_DXE_H */

