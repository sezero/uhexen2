;
; d_polysa.inc -- common header for NASM format d_polys*.asm
; based on asm_draw.h and d_ifacea.h from original Quake source
; $Id$
;
; Copyright (C) 1996-1997  Id Software, Inc.
; Copyright (C) 2012  O.Sezer <sezero@users.sourceforge.net>
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
;
; See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to:
; Free Software Foundation, Inc.
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
;

; !!! if this is changed, it must be changed in quakedef.h too !!!
%define CACHE_SIZE	32		; used to align key data structures

; !!! if this is changed, it must be changed in r_shared.h too !!!
%define MAXHEIGHT	1024

; !!! if this is changed, it must be changed in d_polyse.c too !!!
%define DPS_MAXSPANS	MAXHEIGHT+1

%define spanpackage_t_size				32
%define SPAN_SIZE	(((DPS_MAXSPANS + 1 + ((CACHE_SIZE - 1) / spanpackage_t_size)) + 1) * spanpackage_t_size)
;				(MAXHEIGHT+1+1+1)*32

