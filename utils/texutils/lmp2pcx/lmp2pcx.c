/*
 * lmp2pcx.c
 * $Id$
 * Copyright (C) 2002-2007 Forest Hale
 * Copyright (C) 2007-2012 O.Sezer
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
#include "pathutil.h"
#include "util_io.h"
#include "q_endian.h"
#include "byteordr.h"
#include "filenames.h"


#define	OUTDIR_LEN	9 /* strlen (OUTPUT_DIR) + 1 */
static const char OUTPUT_DIR[OUTDIR_LEN] = "outfiles";

#define	ASTERIX_REPLACE			'_'
	/* character to replace '*' in texture names. */

#define	CONV_LMP			(1 << 0)
#define	CONV_MIP			(1 << 1)
#define	CONV_WAL			(1 << 2)
#define	CONV_WAD			(1 << 3)

typedef void (*convfunc_t) (const char *, int);
static void	ConvertLMP (const char *filename, int idx);
static void	ConvertWAD (const char *filename, int idx);
static void	ConvertMIP (const char *filename, int idx);

static struct
{
	unsigned int	flag;
	convfunc_t	func;
	const char	*matchpattern;
	const char	*datatype;
} convertdata[] =
{
	{ CONV_LMP, ConvertLMP, "*.lmp", "LMP"		},
	{ CONV_MIP, ConvertMIP, "*.mip", "MIPTEX"	},
	{ CONV_WAL, ConvertMIP, "*.wal", "Hexen II HWAL"},
	{ CONV_WAD, ConvertWAD, "*.wad", "WAD2"		},
	{ 0, NULL, NULL, NULL				}
};

static int	image_width, image_height;

typedef struct pcx_s
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin, ymin, xmax, ymax;
	unsigned short	hres, vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
} pcx_t;

void WritePCX (const char *filename, unsigned char *data, int width, int height, unsigned char *palette)
{
	int		i, y, run, pix;
	pcx_t	*pcx;
	unsigned char	*pack, *dataend;

	pcx = (pcx_t *) SafeMalloc (width*height*2 + 1000);

	pcx->manufacturer = 0x0A;	// PCX id
	pcx->version = 5;		// 256 color
	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;	// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)((width + 1) & ~1));
	pcx->palette_type = LittleShort(2);	// not a grey scale

// pack the image
	pack = (unsigned char*)&pcx[1];

	for (y = 0; y < height; y++)
	{
		for (dataend = data + width; data < dataend; )
		{
			for (pix = *data++, run = 0xC1; data < dataend && run < 0xFF && *data == pix; data++, run++)
				;
			if (run > 0xC1 || pix >= 0xC0)
				*pack++ = run;
			*pack++ = pix;
		}
		if (width & 1)
			*pack++ = 0;
	}

// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i = 0; i < 768; i++)
		*pack++ = *palette++;

// write output file
	SaveFile (filename, pcx, pack - (unsigned char *) pcx);

	free(pcx);
}

void WriteTGA (const char *filename, unsigned char *data, int width, int height, const unsigned char *palettergb)
{
	int		maxrun, run, c, y;
	unsigned char	*buffer, *in, *end, *out;

	for (c = 0; c < width*height; c++)
	{
		if (data[c] == 255)
			break;
	}

	if (c < width*height)
	{
		// contains transparent pixels
		// BGRA truecolor since some programs can't deal with BGRA colormaps
		// a buffer big enough to store the worst compression ratio possible (1 extra byte per pixel)
		buffer = (unsigned char *) SafeMalloc (18 + width*height*5);

		buffer[2] = 10;		// RLE truecolor
		buffer[12] = (width >> 0) & 0xFF;
		buffer[13] = (width >> 8) & 0xFF;
		buffer[14] = (height >> 0) & 0xFF;
		buffer[15] = (height >> 8) & 0xFF;
		buffer[16] = 32;	// pixel size
		buffer[17] = 8;		// 8 attribute bits per pixel, bottom left origin
		out = buffer + 18;

		// copy image
		for (y = 0; y < height; y++)
		{
			in = data + (height - 1 - y) * width;
			end = in + width;
			while (in < end)
			{
				maxrun = end - in;
				if (maxrun > 128)
					maxrun = 128;
				if (maxrun >= 2 && in[1] == in[0])
				{
					// run a loop that stops when the next byte is not the same
					for (run = 1; run < maxrun && in[run] == in[0]; run++)
						;
					*out++ = 0x80 + (run - 1);
					if (in[0] == 255)
					{
						*out++ = 0;
						*out++ = 0;
						*out++ = 0;
						*out++ = 0;
					}
					else
					{
						*out++ = palettergb[in[0]*3+2];
						*out++ = palettergb[in[0]*3+1];
						*out++ = palettergb[in[0]*3+0];
						*out++ = 255;
					}
					in += run;
				}
				else
				{
					// run a loop that stops when the next two bytes are the same
					for (run = 1; run < maxrun && (run >= maxrun - 1 || in[run+1] != in[run]); run++)
						;
					*out++ = 0x00 + (run - 1);
					for (c = 0; c < run; c++)
					{
						if (in[0] == 255)
						{
							*out++ = 0;
							*out++ = 0;
							*out++ = 0;
							*out++ = 0;
						}
						else
						{
							*out++ = palettergb[in[0]*3+2];
							*out++ = palettergb[in[0]*3+1];
							*out++ = palettergb[in[0]*3+0];
							*out++ = 255;
						}
						in++;
					}
				}
			}
		}
	}
	else
	{
#if 1
		buffer = (unsigned char *) SafeMalloc (18 + width*height*4);

		buffer[2] = 10;		// RLE truecolor
		buffer[12] = (width >> 0) & 0xFF;
		buffer[13] = (width >> 8) & 0xFF;
		buffer[14] = (height >> 0) & 0xFF;
		buffer[15] = (height >> 8) & 0xFF;
		buffer[16] = 24;	// pixel size
		buffer[17] = 0;		// 0 attribute bits per pixel, bottom left origin
		out = buffer + 18;

		// copy image
		for (y = 0; y < height; y++)
		{
			in = data + (height - 1 - y) * width;
			end = in + width;
			while (in < end)
			{
				maxrun = end - in;
				if (maxrun > 128)
					maxrun = 128;
				if (maxrun >= 2 && in[1] == in[0])
				{
					// run a loop that stops when the next byte is not the same
					for (run = 1; run < maxrun && in[run] == in[0]; run++)
						;
					*out++ = 0x80 + (run - 1);
					*out++ = palettergb[in[0]*3+2];
					*out++ = palettergb[in[0]*3+1];
					*out++ = palettergb[in[0]*3+0];
					in += run;
				}
				else
				{
					// run a loop that stops when the next two bytes are the same
					for (run = 1; run < maxrun && (run >= maxrun - 1 || in[run+1] != in[run]); run++)
						;
					*out++ = 0x00 + (run - 1);
					for (c = 0; c < run; c++)
					{
						*out++ = palettergb[in[0]*3+2];
						*out++ = palettergb[in[0]*3+1];
						*out++ = palettergb[in[0]*3+0];
						in++;
					}
				}
			}
		}
#else
		// contains only opaque pixels
		// a buffer big enough to store the worst compression ratio possible (2 bytes per pixel)
		buffer = (unsigned char *) SafeMalloc (18 + 768 + width*height*2);

		buffer[1] = 1;		// colormap type 1
		buffer[2] = 9;		// RLE compressed colormapped
		// colormap_index
		buffer[3] = (0 >> 0) & 0xFF;
		buffer[4] = (0 >> 8) & 0xFF;
		// colormap_length
		buffer[5] = (256 >> 0) & 0xFF;
		buffer[6] = (256 >> 8) & 0xFF;
		// colormap_size
		buffer[7] = 24;		// 32bit BGRA colormap entries
		buffer[12] = (width >> 0) & 0xFF;
		buffer[13] = (width >> 8) & 0xFF;
		buffer[14] = (height >> 0) & 0xFF;
		buffer[15] = (height >> 8) & 0xFF;
		buffer[16] = 8;		// pixel size
		buffer[17] = 0;		// 0 attribute bits per pixel, origin bottom left
		out = buffer + 18;

		// store BGRA palette of 256 RGB colors
		for (c = 0; c < 256; c++)
		{
			*out++ = palettergb[c*3+2];
			*out++ = palettergb[c*3+1];
			*out++ = palettergb[c*3+0];
		}

		// copy image
		for (y = 0; y < height; y++)
		{
			in = data + (height - 1 - y) * width;
			end = in + width;
			while (in < end)
			{
				maxrun = end - in;
				if (maxrun > 128)
					maxrun = 128;
				if (maxrun >= 2 && in[1] == in[0])
				{
					// run a loop that stops when the next byte is not the same
					for (run = 1; run < maxrun && in[run] == in[0]; run++)
						;
					*out++ = 0x80 + (run - 1);
					*out++ = in[0];
					in += run;
				}
				else
				{
					// run a loop that stops when the next two bytes are the same
					for (run = 1; run < maxrun && (run >= maxrun - 1 || in[run+1] != in[run]); run++)
						;
					*out++ = 0x00 + (run - 1);
					for (c = 0; c < run; c++)
						*out++ = *in++;
				}
			}
		}
#endif
	}

	SaveFile (filename, buffer, out - buffer);

	free (buffer);
}


static unsigned char *gamepalette;
static unsigned char gamepal[768] =
{
//#include		"quakepal.h"
#include		"hexen2pal.h"
};

/*
=============
LoadLMP
=============
*/
static unsigned char *LoadLMP (const char *filename, int idx)
{
	unsigned char		*data;
	void			*lmpdata;

	LoadFile (filename, &lmpdata);
	image_width = LittleLong(((int *)lmpdata)[0]);
	image_height = LittleLong(((int *)lmpdata)[1]);
	if (image_width <= 0 || image_height <= 0)
	{
		free (lmpdata);
		printf ("%s: \"%s\" [%dx%d] is not a %s file\n", __thisfunc__, filename, image_width, image_height, convertdata[idx].datatype);
		return NULL;
	}
	data = (unsigned char *) SafeMalloc (image_width*image_height);
	memcpy (data, (unsigned char *)lmpdata + 8, image_width*image_height);
	free (lmpdata);
	return data;
}

static void ConvertLMP (const char *filename, int idx)
{
	unsigned char		*data;
	char		tempname[1024];

	q_snprintf (tempname, sizeof(tempname), "%s%c%s",
				OUTPUT_DIR, DIR_SEPARATOR_CHAR, filename);
	StripExtension (tempname);
	// conchars = weird (hexen2 version)
	if (!strcmp(&tempname[OUTDIR_LEN], "conchars"))
	{
		void		*lmpdata;
		LoadFile (filename, &lmpdata);
		data = (unsigned char *)lmpdata;
		image_width = 256;
		image_height = 128;
	}
	// skip the palette file itself
	else if (!strcmp(&tempname[OUTDIR_LEN], "palette"))
	{
		return;
	}
	else
	{
		data = LoadLMP (filename, idx);
	}
	if (!data)
		return;
	q_strlcat (tempname, ".pcx", sizeof(tempname));
	WritePCX (tempname, data, image_width, image_height, gamepalette);
	StripExtension (tempname);
	q_strlcat (tempname, ".tga", sizeof(tempname));
	WriteTGA (tempname, data, image_width, image_height, gamepalette);
	free (data);
}

/*
=============
LoadMIP
=============
*/
// hexen II HWAL file detection:
#define IDWALHEADER	(('L'<<24)+('A'<<16)+('W'<<8)+'H')
#define WALVERSION	1
static unsigned char *LoadMIP (const char *filename, int idx)
{
	int			shift;
	unsigned char		*data;
	void			*mipdata;

	LoadFile (filename, &mipdata);
	shift = 0;
	if (convertdata[idx].flag == CONV_WAL)	// Hexen II HWAL?
	{
		shift = LittleLong(((int *)mipdata)[0]);
		if (shift != IDWALHEADER)
			shift = 0;
		else
		{
			shift = LittleLong(((int *)mipdata)[1]);
			if (shift != WALVERSION)
				shift = 0;
			else
				shift = 6;	/* (sizeof(miptex_wal_t) - sizeof(miptex_t)) / sizeof(int) */
		}
		if (shift == 0)
		{
			free (mipdata);
			printf ("%s: \"%s\" is not a %s file\n", __thisfunc__, filename, convertdata[idx].datatype);
			return NULL;
		}
	}
	image_width = LittleLong(((int *)mipdata)[4+shift]);
	image_height = LittleLong(((int *)mipdata)[5+shift]);
	if (image_width <= 0 || image_height <= 0 || (image_width & 15) || (image_height & 15))
	{
		free (mipdata);
		printf ("%s: \"%s\" [%dx%d] is not a %s file\n", __thisfunc__, filename, image_width, image_height, convertdata[idx].datatype);
		return NULL;
	}
	data = (unsigned char *) SafeMalloc (image_width*image_height);
	memcpy (data, (unsigned char *)mipdata + 40 + 4*shift, image_width*image_height);
	free (mipdata);
	return data;
}

static void ConvertMIP (const char *filename, int idx)
{
	unsigned char		*data;
	char		tempname[1024];

	data = LoadMIP (filename, idx);
	if (!data)
		return;
	q_snprintf (tempname, sizeof(tempname), "%s%c%s",
				OUTPUT_DIR, DIR_SEPARATOR_CHAR, filename);
	StripExtension (tempname);
	q_strlcat (tempname, ".pcx", sizeof(tempname));
	WritePCX (tempname, data, image_width, image_height, gamepalette);
	StripExtension (tempname);
	q_strlcat (tempname, ".tga", sizeof(tempname));
	WriteTGA (tempname, data, image_width, image_height, gamepalette);
	free (data);
}

#define	CMP_NONE		0

#define	TYP_NONE		0
#define	TYP_LABEL		1

#define	TYP_LUMPY		64				// 64 + grab command number
#define	TYP_PALETTE		64
#define	TYP_QTEX		65
#define	TYP_QPIC		66
#define	TYP_SOUND		67
#define	TYP_MIPTEX		68

typedef struct
{
	int			width, height;
	unsigned char		data[4];	// variably sized
} qpic_t;

typedef struct
{
	char		identification[4];	// should be WAD2 or 2DAW
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;		// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];		// must be null terminated
} lumpinfo_t;

static void wad_cleanname (char *in, char *out)
{
	int	i, c;

	for (i = 0; i < 16 && *in; i++)
	{
		c = *in++;
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		*out++ = c;
	}
	for ( ; i < 17; i++ )
		*out++ = 0;
}

static void ConvertWAD (const char *filename, int idx)
{
	int			i, width, height;
	wadinfo_t		*wad;
	lumpinfo_t		*lump;
	void			*waddata;
	unsigned char		*data;
	char		tempname[1024], *ptr;

	LoadFile (filename, &waddata);
	wad = (wadinfo_t *) waddata;
	if (memcmp(wad->identification, "WAD2", 4))
	{
		printf("%s: \"%s\" is not a %s file\n", __thisfunc__, filename, convertdata[idx].datatype);
		return;
	}
	q_snprintf (tempname, sizeof(tempname), "%s%c", OUTPUT_DIR, DIR_SEPARATOR_CHAR);
	ptr = &tempname[OUTDIR_LEN];
	wad->numlumps = LittleLong(wad->numlumps);
	wad->infotableofs = LittleLong(wad->infotableofs);
	printf ("%s: converting \"%s\" (%i lumps)\n", __thisfunc__, filename, wad->numlumps);
	lump = (lumpinfo_t *)((unsigned char *)waddata + wad->infotableofs);
	for (i = 0; i < wad->numlumps; i++, lump++)
	{
		height = 0;
		if (lump->compression != CMP_NONE)
		{
			printf ("lump \"%s\" is compressed, compression is unsupported\n", lump->name);
			continue;
		}
		lump->filepos = LittleLong(lump->filepos);
		lump->disksize = LittleLong(lump->disksize);
		data = (unsigned char *) waddata + lump->filepos;
		wad_cleanname (lump->name, ptr);
		if (!strcmp(ptr, "conchars"))	// (quake) conchars = weird
			height = 128;
		else if (!strcmp(ptr, "tinyfont"))	// hexen2 tinyfonts = weird
			height = 32;
		if (height != 0)
		{
			q_strlcat (tempname, ".bin", sizeof(tempname));
			SaveFile (tempname, data, lump->disksize);
			StripExtension (tempname);
			q_strlcat (tempname, ".pcx", sizeof(tempname));
			WritePCX (tempname, data, 128, height, gamepalette);
			StripExtension (tempname);
			q_strlcat (tempname, ".tga", sizeof(tempname));
			WriteTGA (tempname, data, 128, height, gamepalette);
			continue;
		}
		switch (lump->type)
		{
		case TYP_NONE:
			printf ("encountered lump type '%s' named \"%s\"\n", "NONE", lump->name);
			break;
		case TYP_LABEL:
			printf ("encountered lump type '%s' named \"%s\"\n", "LABEL", lump->name);
			break;
		case TYP_LUMPY:
		//	printf ("encountered lump type '%s' named \"%s\"\n", "LUMPY", lump->name);
			wad_cleanname (lump->name, ptr);
			if (*ptr == '*')
				*ptr = ASTERIX_REPLACE;
			q_strlcat (tempname, ".bin", sizeof(tempname));
			SaveFile (tempname, data, lump->disksize);
			break;
		case TYP_QTEX:
			printf ("encountered lump type '%s' named \"%s\"\n", "QTEX", lump->name);
			break;
		case TYP_QPIC:
		//	printf ("encountered lump type '%s' named \"%s\"\n", "QPIC", lump->name);
			width = LittleLong(((int *)data)[0]);
			height = LittleLong(((int *)data)[1]);
			if (width <= 0 || height <= 0)
			{
				printf("\"%s\" [%dx%d] is not a valid qpic\n", lump->name, width, height);
				continue;
			}
			wad_cleanname (lump->name, ptr);
			if (*ptr == '*')
				*ptr = ASTERIX_REPLACE;
			q_strlcat (tempname, ".lmp", sizeof(tempname));
			SaveFile (tempname, data, lump->disksize);
			StripExtension (tempname);
			q_strlcat (tempname, ".pcx", sizeof(tempname));
			WritePCX (tempname, data+8, width, height, gamepalette);
			StripExtension (tempname);
			q_strlcat (tempname, ".tga", sizeof(tempname));
			WriteTGA (tempname, data+8, width, height, gamepalette);
			break;
		case TYP_SOUND:
			printf ("encountered lump type '%s' named \"%s\"\n", "SOUND", lump->name);
			break;
		case TYP_MIPTEX:
		//	printf ("encountered lump type '%s' named \"%s\"\n", "MIPTEX", lump->name);
			width = LittleLong(((int *)data)[4]);
			height = LittleLong(((int *)data)[5]);
			if (width <= 0 || height <= 0)
			{
				printf ("\"%s\" [%dx%d] is not a valid %s\n", lump->name, width, height, "MIPTEX");
				continue;
			}
			wad_cleanname (lump->name, ptr);
			if (*ptr == '*')
				*ptr = ASTERIX_REPLACE;
			q_strlcat (tempname, ".mip", sizeof(tempname));
			SaveFile (tempname, data, lump->disksize);
			StripExtension (tempname);
			q_strlcat (tempname, ".pcx", sizeof(tempname));
			WritePCX (tempname, data+40, width, height, gamepalette);
			StripExtension (tempname);
			q_strlcat (tempname, ".tga", sizeof(tempname));
			WriteTGA (tempname, data+40, width, height, gamepalette);
			break;
		default:
			printf ("encountered lump type '%s' named \"%s\"\n", "UNKNOWN", lump->name);
			break;
		}
	}

	free (waddata);
}

static void print_usage (void)
{
	printf ("LMP2PCX v1.02 by Forest \"LordHavoc\" Hale.\n");
	printf ("Adapted to Hexen II for the Hammer of Thyrion project by O.Sezer\n");
	printf ("Converts all lmp, mip, wal and wad files in current directory to\n");
	printf ("pcx and tga files. New files are placed in \"%s\" directory\n", OUTPUT_DIR);
	printf ("The palette will be loaded from palette.lmp file if found in the\n");
	printf ("currect directory, otherwise the embedded hexen2 palette will be\n");
	printf ("used.\n");
	printf ("Usage: lmp2pcx [-lmp] [-wad] [-mip] [-wal] [-all]\n");
}

static char	cwd[1024];

int main (int argc, char **argv)
{
	int		i, j;
	unsigned int	flags;
	const char	*name;
	void		*pbuf;

	ValidateByteorder ();

	flags = 0;
	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i], "-all"))
			flags |= (CONV_LMP|CONV_MIP|CONV_WAL|CONV_WAD);
		else if (!strcmp(argv[i], "-lmp"))
			flags |= CONV_LMP;
		else if (!strcmp(argv[i], "-wad"))
			flags |= CONV_WAD;
		else if (!strcmp(argv[i], "-mip"))
			flags |= CONV_MIP;
		else if (!strcmp(argv[i], "-wal"))
			flags |= CONV_WAL;
		else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			print_usage ();
			exit (0);
		}
		else
		{
			print_usage ();
			COM_Error ("Unknown option \"%s\"", argv[i]);
		}
	}

	if (flags == 0)
		flags = (CONV_LMP|CONV_MIP|CONV_WAL|CONV_WAD);

	j = Q_FileType("palette.lmp");
	if (j != FS_ENT_FILE)
	{
		printf ("Using embedded hexen2 palette.\n");
		gamepalette = (unsigned char *) gamepal;
	}
	else
	{
		j = LoadFile ("palette.lmp", &pbuf);
		if (j != 768)
		{
			COM_Error ("palette.lmp has invalid size.");
			/*
			free (pbuf);
			gamepalette = (unsigned char *) gamepal;
			*/
		}
		else
		{
			printf ("Using palette from palette.lmp.\n");
			gamepalette = (unsigned char *) pbuf;
		}
	}

	Q_getwd (cwd, sizeof(cwd), false);
	Q_mkdir (OUTPUT_DIR);

	for (i = 0 ; convertdata[i].flag ; i++)
	{
		if (flags & convertdata[i].flag)
		{
			name = Q_FindFirstFile (cwd, convertdata[i].matchpattern);
			while (name)
			{
				convertdata[i].func (name, i);
				name = Q_FindNextFile();
			}
			Q_FindClose();
		}
	}

	return 0;
}

