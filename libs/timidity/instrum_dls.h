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
 * instrum.h
 */

#ifndef TIMIDITY_INSTRUM_DLS_H
#define TIMIDITY_INSTRUM_DLS_H

extern void load_instrument_dls(MidSong *song, MidInstrument **out,
				int drum, int bank, int instrument);

#if !defined(TIMIDITY_USE_DLS)
#define load_instrument_dls(S, OUTINST, D, B, I)  *(OUTINST)=NULL
#endif

#endif /* TIMIDITY_INSTRUM_DLS_H */
