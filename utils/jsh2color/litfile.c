/* litfile.c
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "q_endian.h"
#include "pathutil.h"
#include "bspfile.h"
#include "litfile.h"
#include "jscolor.h"

void MakeLITFile (const char *filename)
{
	char	litname[1024];
	FILE	*litfile;
	litheader_t	litheader;

	strcpy (litname, filename);
	StripExtension (litname);
	DefaultExtension (litname, ".lit", sizeof(litname));
	litfile = fopen (litname, "wb");

	if (!litfile)
	{
		printf ("Unable to create %s\n", litname);
		return;
	}

	litheader.ident[0] = 'Q';
	litheader.ident[1] = 'L';
	litheader.ident[2] = 'I';
	litheader.ident[3] = 'T';
	litheader.version = LittleLong(LIT_VERSION);

	fwrite (&litheader, sizeof(litheader), 1, litfile);
	fwrite (newdlightdata, newlightdatasize, 1, litfile);

	fclose (litfile);
	printf ("Wrote litfile: %s\n", litname);
}

