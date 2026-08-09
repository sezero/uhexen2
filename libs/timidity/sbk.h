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
 * sbk.h: SoundFont(tm) file format
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

#ifndef SBK_H_DEF
#define SBK_H_DEF

typedef struct _tchunk {
	char id[4];
	sint32 size;
} tchunk;

typedef struct _tsbkheader {
	char riff[4];	/* RIFF */
	sint32 size;	/* size of sbk after there bytes */
	char sfbk[4];	/* sfbk id */
} tsbkheader;

typedef struct _tsamplenames {
	char name[20];
} tsamplenames;

typedef struct _tpresethdr {
	char name[20];
	uint16 preset, bank, bagNdx;
	/*int lib, genre, morphology;*/ /* reserved */
} tpresethdr;

typedef struct _tsampleinfo {
	sint32 startsample, endsample;
	sint32 startloop, endloop;
	/* ver.2 additional info */
	sint32 samplerate;
	uint8 originalPitch;
	uint8 pitchCorrection;
	uint16 samplelink;
	uint16 sampletype;  /*1=mono, 2=right, 4=left, 8=linked, $8000=ROM*/
} tsampleinfo;

typedef struct _tinsthdr {
	char name[20];
	uint16 bagNdx;
} tinsthdr;

typedef struct _tgenrec {
	sint16 oper;
	sint16 amount;
} tgenrec;


typedef struct _SFInfo {
	uint16 version, minorversion;
	sint32 samplepos, samplesize;

	int nrsamples;
	tsamplenames *samplenames;

	int nrpresets;
	tpresethdr *presethdr;

	int nrinfos;
	tsampleinfo *sampleinfo;

	int nrinsts;
	tinsthdr *insthdr;

	int nrpbags, nribags;
	uint16 *presetbag, *instbag;

	int nrpgens, nrigens;
	tgenrec *presetgen, *instgen;

	/*tsbkheader sbkh;*/

	/*char *sf_name;*/

	int in_rom;
} SFInfo;


/*----------------------------------------------------------------
 * functions
 *----------------------------------------------------------------*/

#define load_sbk TIMI_NAMESPACE(load_sbk)
#define free_sbk TIMI_NAMESPACE(free_sbk)

int load_sbk(FILE *fp, SFInfo *sf);
void free_sbk(SFInfo *sf);

#endif
