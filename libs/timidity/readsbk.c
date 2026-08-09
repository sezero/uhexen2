/* libTiMidity is licensed under the terms of the GNU Lesser General
 * Public License: see COPYING for details.
 *
 * Note that the included TiMidity source, based on timidity-0.2i, was
 * originally licensed under the GPL, but the author extended it so it
 * can also be used separately under the GNU LGPL or the Perl Artistic
 * License: see the notice by Tuukka Toivonen as it appears on the web
 * at http://ieee.uwaterloo.ca/sca/www.cgs.fi/tt/timidity/ .
 */

/*
 * TiMidity -- Experimental MIDI to WAVE converter
 * Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
 *
 * readsbk.c: read soundfont file
 * Copyright (C) 1996,1997 Takashi Iwai
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "timidity_internal.h"
#include "common.h"
#include "sbk.h"

/*----------------------------------------------------------------
 * function prototypes
 *----------------------------------------------------------------*/

#define NEW(type,nums)	(type*)timi_calloc((nums), sizeof(type))

static int READCHUNK(tchunk *vp, FILE *fd)
{
	if (fread(vp, 8, 1, fd) != 1) return -1;
	vp->size = SWAPLE32(vp->size);
	return 1;
}

static int READDW(sint32 *vp, FILE *fd)
{
	if (fread(vp, 4, 1, fd) != 1) return -1;
	*vp = SWAPLE32(*vp);
	return 1;
}

static int READW(uint16 *vp, FILE *fd)
{
	if (fread(vp, 2, 1, fd) != 1) return -1;
	*vp = SWAPLE16(*vp);
	return 1;
}

static int READSTR(char *str, FILE *fd)
{
	int n;
	if (fread(str, 20, 1, fd) != 1) return -1;
	str[19] = '\0';
	n = strlen(str);
	while (n > 0 && str[n - 1] == ' ')
		n--;
	str[n] = '\0';
	return n;
}

#define READID(var,fd)	fread(var, 1, 4, fd)
#define READB(var,fd)	fread(var, 1, 1, fd)
#define SKIPB(fd)	fseek(fd, 1, SEEK_CUR)
#define SKIPW(fd)	fseek(fd, 2, SEEK_CUR)
#define SKIPDW(fd)	fseek(fd, 4, SEEK_CUR)

static long FILESIZE(FILE *fd)
{
	long len, pos;

	pos = ftell(fd);
	fseek(fd, 0, SEEK_END);
	len = ftell(fd) - pos;
	fseek(fd, pos, SEEK_SET);
	return len;
}

static int getchunk(const char *id);
static void process_chunk(int id, int s, SFInfo *sf, FILE *fd);
static void load_sample_names(int size, SFInfo *sf, FILE *fd);
static void load_preset_header(int size, SFInfo *sf, FILE *fd);
static void load_inst_header(int size, SFInfo *sf, FILE *fd);
static void load_bag(int size, SFInfo *sf, FILE *fd, int *totalp, uint16 **bufp);
static void load_gen(int size, SFInfo *sf, FILE *fd, int *totalp, tgenrec **bufp);
static void load_sample_info(int size, SFInfo *sf, FILE *fd);


enum {
	/* level 0 */
	UNKN_ID, RIFF_ID, LIST_ID, SFBK_ID,
	/* level 1 */
	INFO_ID, SDTA_ID, PDTA_ID,
	/* info stuff */
	IFIL_ID, ISNG_ID, IROM_ID, INAM_ID, IVER_ID, IPRD_ID, ICOP_ID,
	ICRD_ID, IENG_ID, ISFT_ID, ICMT_ID,
	/* sample data stuff */
	SNAM_ID, SMPL_ID,
	/* preset stuff */
	PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,
	/* inst stuff */
	INST_ID, IBAG_ID, IMOD_ID, IGEN_ID,
	/* sample header */
	SHDR_ID
};


/*----------------------------------------------------------------
 * debug routine
 *----------------------------------------------------------------*/

#if 0
static void debugid(const char *tag, const char *p)
{
	char buf[5]; timi_strxcpy(buf, p, sizeof(buf));
	fprintf(stderr,"[%s:%s]\n", tag, buf);
}

static void debugname(const char *tag, const char *p)
{
	char buf[21]; timi_strxcpy(buf, p, sizeof(buf));
	fprintf(stderr,"[%s:%s]\n", tag, buf);
}

static void debugval(const char *tag, int v)
{
	fprintf(stderr, "[%s:%d]\n", tag, v);
}
#else
#define debugid(t,s) /**/
#define debugname(t,s) /**/
#define debugval(t,v) /**/
#endif


/*----------------------------------------------------------------
 * load sbk file
 *----------------------------------------------------------------*/

int load_sbk(FILE *fd, SFInfo *sf)
{
	tchunk chunk, subchunk;
	long len;

	len = FILESIZE(fd);
	if (len < 32) /* better?? */
		return -1;

	READCHUNK(&chunk, fd);
	if (getchunk(chunk.id) != RIFF_ID) return -1;
	if (chunk.size != len - 8) return -1;

	READID(chunk.id, fd);
	if (getchunk(chunk.id) != SFBK_ID) return -1;

	sf->in_rom = 1;
	while (! feof(fd)) {
		READID(chunk.id, fd);
		switch (getchunk(chunk.id)) {
		case LIST_ID:
			READDW(&chunk.size, fd);
			READID(subchunk.id, fd);
			process_chunk(getchunk(subchunk.id), chunk.size - 4, sf, fd);
			break;
		}
	}

	return 0;
}


/*----------------------------------------------------------------
 * free buffer
 *----------------------------------------------------------------*/

void free_sbk(SFInfo *sf)
{
	timi_free(sf->samplenames);
	timi_free(sf->presethdr);
	timi_free(sf->sampleinfo);
	timi_free(sf->insthdr);
	timi_free(sf->presetbag);
	timi_free(sf->instbag);
	timi_free(sf->presetgen);
	timi_free(sf->instgen);
	/*timi_free(sf->sf_name);*/
	memset(sf, 0, sizeof(*sf));
}



/*----------------------------------------------------------------
 * get id value
 *----------------------------------------------------------------*/

static int getchunk(const char *id)
{
	static struct idstring {
		const char *str;
		int id;
	} idlist[] = {
		{"RIFF", RIFF_ID},
		{"LIST", LIST_ID},
		{"sfbk", SFBK_ID},
		{"INFO", INFO_ID},
		{"sdta", SDTA_ID},
		{"snam", SNAM_ID},
		{"smpl", SMPL_ID},
		{"pdta", PDTA_ID},
		{"phdr", PHDR_ID},
		{"pbag", PBAG_ID},
		{"pmod", PMOD_ID},
		{"pgen", PGEN_ID},
		{"inst", INST_ID},
		{"ibag", IBAG_ID},
		{"imod", IMOD_ID},
		{"igen", IGEN_ID},
		{"shdr", SHDR_ID},
		{"ifil", IFIL_ID},
		{"isng", ISNG_ID},
		{"irom", IROM_ID},
		{"iver", IVER_ID},
		{"INAM", INAM_ID},
		{"IPRD", IPRD_ID},
		{"ICOP", ICOP_ID},
		{"ICRD", ICRD_ID},
		{"IENG", IENG_ID},
		{"ISFT", ISFT_ID},
		{"ICMT", ICMT_ID},
	};
	static const int listsize = (int) sizeof(idlist)/sizeof(idlist[0]);

	int i;

	for (i = 0; i < listsize; i++) {
		if (memcmp(id, idlist[i].str, 4) == 0) {
			debugid("ok", id);
			return idlist[i].id;
		}
	}

	debugid("xx", id);
	return UNKN_ID;
}


static void load_sample_names(int size, SFInfo *sf, FILE *fd)
{
	int i;
	sf->nrsamples = size / 20;
	sf->samplenames = NEW(tsamplenames, sf->nrsamples);
	for (i = 0; i < sf->nrsamples; i++) {
		READSTR(sf->samplenames[i].name, fd);
	}
}

static void load_preset_header(int size, SFInfo *sf, FILE *fd)
{
	int i;
	sf->nrpresets = size / 38;
	sf->presethdr = NEW(tpresethdr, sf->nrpresets);
	for (i = 0; i < sf->nrpresets; i++) {
		READSTR(sf->presethdr[i].name, fd);
		READW(&sf->presethdr[i].preset, fd);
		READW(&sf->presethdr[i].bank, fd);
		READW(&sf->presethdr[i].bagNdx, fd);
		SKIPDW(fd); /* lib */
		SKIPDW(fd); /* genre */
		SKIPDW(fd); /* morph */
	}
}

static void load_inst_header(int size, SFInfo *sf, FILE *fd)
{
	int i;

	sf->nrinsts = size / 22;
	sf->insthdr = NEW(tinsthdr, sf->nrinsts);
	for (i = 0; i < sf->nrinsts; i++) {
		READSTR(sf->insthdr[i].name, fd);
		READW(&sf->insthdr[i].bagNdx, fd);
	}
}

static void load_bag(int size, SFInfo *sf, FILE *fd, int *totalp, uint16 **bufp)
{
	uint16 *buf;
	int i;

	TIMI_UNUSED(sf);
	debugval("bagsize", size);
	size /= 4;
	buf = NEW(uint16, size);
	for (i = 0; i < size; i++) {
		READW(&buf[i], fd);
		SKIPW(fd); /* mod */
	}
	*totalp = size;
	*bufp = buf;
}

static void load_gen(int size, SFInfo *sf, FILE *fd, int *totalp, tgenrec **bufp)
{
	tgenrec *buf;
	int i;

	TIMI_UNUSED(sf);
	debugval("gensize", size);
	size /= 4;
	buf = NEW(tgenrec, size);
	for (i = 0; i < size; i++) {
		READW((uint16 *)&buf[i].oper, fd);
		READW((uint16 *)&buf[i].amount, fd);
	}
	*totalp = size;
	*bufp = buf;
}

static void load_sample_info(int size, SFInfo *sf, FILE *fd)
{
	int i;

	debugval("infosize", size);
	if (sf->version > 1) {
		sf->nrinfos = size / 46;
		sf->nrsamples = sf->nrinfos;
		sf->sampleinfo = NEW(tsampleinfo, sf->nrinfos);
		sf->samplenames = NEW(tsamplenames, sf->nrsamples);
	}
	else  {
		sf->nrinfos = size / 16;
		sf->sampleinfo = NEW(tsampleinfo, sf->nrinfos);
	}

	for (i = 0; i < sf->nrinfos; i++) {
		if (sf->version > 1)
			READSTR(sf->samplenames[i].name, fd);
		READDW(&sf->sampleinfo[i].startsample, fd);
		READDW(&sf->sampleinfo[i].endsample, fd);
		READDW(&sf->sampleinfo[i].startloop, fd);
		READDW(&sf->sampleinfo[i].endloop, fd);
		if (sf->version > 1) {
			READDW(&sf->sampleinfo[i].samplerate, fd);
			READB(&sf->sampleinfo[i].originalPitch, fd);
			READB(&sf->sampleinfo[i].pitchCorrection, fd);
			READW(&sf->sampleinfo[i].samplelink, fd);
			READW(&sf->sampleinfo[i].sampletype, fd);
		} else {
			if (sf->sampleinfo[i].startsample == 0)
				sf->in_rom = 0;
			sf->sampleinfo[i].startloop++;
			sf->sampleinfo[i].endloop += 2;
			sf->sampleinfo[i].samplerate = 44100;
			sf->sampleinfo[i].originalPitch = 60;
			sf->sampleinfo[i].pitchCorrection = 0;
			sf->sampleinfo[i].samplelink = 0;
			if (sf->in_rom)
				sf->sampleinfo[i].sampletype = 0x8001;
			else
				sf->sampleinfo[i].sampletype = 1;
		}
	}
}

static void process_chunk(int id, int s, SFInfo *sf, FILE *fd)
{
	int cid;
	tchunk subchunk;

	TIMI_UNUSED(s);

	switch (id) {
	case INFO_ID:
		READCHUNK(&subchunk, fd);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case IFIL_ID:
				READW(&sf->version, fd);
				READW(&sf->minorversion, fd);
				break;
			/*
			case INAM_ID:
				sf->sf_name = (char *)timi_malloc(subchunk.size + 1);
				fread(sf->sf_name, 1, subchunk.size, fd);
				sf->sf_name[subchunk.size] = 0;
				break;
			*/
			default:
				fseek(fd, subchunk.size, SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, fd);
			if (feof(fd))
				return;
		}
		fseek(fd, -8, SEEK_CUR); /* seek back */
		break;

	case SDTA_ID:
		READCHUNK(&subchunk, fd);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case SNAM_ID:
				if (sf->version > 1) {
					DEBUG_MSG("**** version 2 has obsolete format??\n");
					fseek(fd, subchunk.size, SEEK_CUR);
				} else
					load_sample_names(subchunk.size, sf, fd);
				break;
			case SMPL_ID:
				sf->samplepos = ftell(fd);
				sf->samplesize = subchunk.size;
				fseek(fd, subchunk.size, SEEK_CUR);
			}
			READCHUNK(&subchunk, fd);
			if (feof(fd))
				return;
		}
		fseek(fd, -8, SEEK_CUR); /* seek back */
		break;

	case PDTA_ID:
		READCHUNK(&subchunk, fd);
		while ((cid = getchunk(subchunk.id)) != LIST_ID) {
			switch (cid) {
			case PHDR_ID:
				load_preset_header(subchunk.size, sf, fd);
				break;

			case PBAG_ID:
				load_bag(subchunk.size, sf, fd,
					 &sf->nrpbags, &sf->presetbag);
				break;

			case PMOD_ID: /* ignored */
				fseek(fd, subchunk.size, SEEK_CUR);
				break;

			case PGEN_ID:
				load_gen(subchunk.size, sf, fd,
					 &sf->nrpgens, &sf->presetgen);
				break;

			case INST_ID:
				load_inst_header(subchunk.size, sf, fd);
				break;

			case IBAG_ID:
				load_bag(subchunk.size, sf, fd,
					 &sf->nribags, &sf->instbag);
				break;

			case IMOD_ID: /* ingored */
				fseek(fd, subchunk.size, SEEK_CUR);
				break;

			case IGEN_ID:
				load_gen(subchunk.size, sf, fd,
					 &sf->nrigens, &sf->instgen);
				break;

			case SHDR_ID:
				load_sample_info(subchunk.size, sf, fd);
				break;

			default:
				DEBUG_MSG("unknown id\n");
				fseek(fd, subchunk.size, SEEK_CUR);
				break;
			}
			READCHUNK(&subchunk, fd);
			if (feof(fd)) {
				debugid("file", "EOF");
				return;
			}
		}
		fseek(fd, -8, SEEK_CUR); /* rewind */
		break;
	}
}

