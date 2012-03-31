/*
	dosisms.h :   I'd call it dos.h, but the name's taken
	DOS / DJGPP system header.
	From quake1 source with minor adaptations for uhexen2.

	$Id$

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#ifndef _DOSISMS_H_
#define _DOSISMS_H_

#include <dpmi.h>

#if defined(DEBUG) &&		\
   !defined(DJGPP_NO_INLINES)
#define DJGPP_NO_INLINES	1
#endif

#if 0	/* same as __dpmi_regs */
typedef union {
	struct {
		unsigned long edi;
		unsigned long esi;
		unsigned long ebp;
		unsigned long res;
		unsigned long ebx;
		unsigned long edx;
		unsigned long ecx;
		unsigned long eax;
	} d;
	struct {
		unsigned short di, di_hi;
		unsigned short si, si_hi;
		unsigned short bp, bp_hi;
		unsigned short res, res_hi;
		unsigned short bx, bx_hi;
		unsigned short dx, dx_hi;
		unsigned short cx, cx_hi;
		unsigned short ax, ax_hi;
		unsigned short flags;
		unsigned short es;
		unsigned short ds;
		unsigned short fs;
		unsigned short gs;
		unsigned short ip;
		unsigned short cs;
		unsigned short sp;
		unsigned short ss;
	} x;
	struct {
		unsigned char edi[4];
		unsigned char esi[4];
		unsigned char ebp[4];
		unsigned char res[4];
		unsigned char bl, bh, ebx_b2, ebx_b3;
		unsigned char dl, dh, edx_b2, edx_b3;
		unsigned char cl, ch, ecx_b2, ecx_b3;
		unsigned char al, ah, eax_b2, eax_b3;
	} h;
} regs_t;
#endif	/* #if 0 */

int dos_lockmem (void *addr, int size);
int dos_unlockmem (void *addr, int size);

void *dos_getmemory (int size);
void dos_freememory (void *ptr);

#define dos_getheapsize _go32_dpmi_remaining_physical_memory

unsigned int ptr2real (void *ptr);
void *real2ptr (unsigned int real);
void *far2ptr (unsigned int farptr);
unsigned int ptr2far (void *ptr);

#if !defined(DJGPP_NO_INLINES)
/* shortcuts for when we aren't debugging */
#include <dos.h>

#define dos_inportb		inportb
#define dos_inportw		inportw
#define dos_outportb		outportb
#define dos_outportw		outportw

#else	/* DJGPP_NO_INLINES */

int dos_inportb (int port);
int dos_inportw (int port);
void dos_outportb (int port, int val);
void dos_outportw (int port, int val);
#endif	/* DJGPP_NO_INLINES */

void dos_irqenable (void);
void dos_irqdisable (void);
void dos_registerintr (int intr, void (*handler)(void));
void dos_restoreintr (int intr);

int dos_int86 (int vec);
int dos_int386 (int vec, __dpmi_regs *inregs, __dpmi_regs *outregs);

/* global variables: */
extern __dpmi_regs	regs;

/* memory setup: */
#define	LEAVE_FOR_CACHE		0x80000	/* 512K - FIXME: tune */

#define	MALLOC_NEEDMEM		0x20000	/* 128K - FIXME: tune */
#if defined(USE_WATT32)
#define	LOCKED_FOR_MALLOC	(MALLOC_NEEDMEM + WATT32_NEEDMEM)
#else
#define	LOCKED_FOR_MALLOC	(MALLOC_NEEDMEM)
#endif

#endif	/* _DOSISMS_H_ */

