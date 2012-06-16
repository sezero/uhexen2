/*
 * litfile.h
 * $Id: litfile.h,v 1.1 2007-05-12 09:56:35 sezero Exp $
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

#ifndef __LITFILE_H
#define __LITFILE_H

#define LIT_VERSION		1

typedef struct litheader_s
{
	char	ident[4];
	int		version;
} litheader_t;

void MakeLITFile (const char *filename);

#endif	/* __LITFILE_H */

