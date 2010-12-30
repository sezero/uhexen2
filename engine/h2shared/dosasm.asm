; dosasm.asm
;
; $Id$
; this file uses NASM syntax.
;
; Copyright (C) 1996-1997  Id Software, Inc.
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

; externs from C code
;

; externs from ASM-only code
;


SEGMENT .data

fpenv:	dd	0,0,0,0,0,0,0,0


SEGMENT .text

global StartMSRInterval
StartMSRInterval:
	mov	ecx,011h	; read the CESR
	db	00Fh
	db	032h		; RDMSR

	and	eax,0FE3FFE3Fh	; stop both counters
	db	00Fh
	db	030h		; WRMSR

	mov	eax, [esp+4]	; point counter 0 to desired event, with counters
	and	eax,03Fh			; still stopped
	mov	ecx,011h
	db	00Fh
	db	030h		; WRMSR

	mov	ecx,012h	; set counter 0 to the value 0
	sub	eax,eax
	sub	edx,edx
	db	00Fh
	db	030h		; WRMSR

	mov	eax, [esp+4]	; restart counter 0 with selected event
	and	eax,03Fh
	sub	edx,edx
	or	eax,0C0h
	mov	ecx,011h	; control and event select
	db	00Fh
	db	030h		; WRMSR

	ret

global EndMSRInterval
EndMSRInterval:
	mov	ecx,012h	; counter 0
	db	00Fh
	db	032h		; RDMSR

	ret				; lower 32 bits of count in %eax

%if 0

	SEGMENT .data
Lxxx:	dd	0

	SEGMENT .text

global setstackcheck
setstackcheck:

	mov	eax,esp
	sub	eax,038000h
	mov	dword [eax],05A5A5A5Ah
	mov	[Lxxx],eax

	ret


global dostackcheck
dostackcheck:

	mov	edx, [Lxxx]
	mov	eax,0

	cmp	dword [edx],05A5A5A5Ah
	jz		qqq
	inc	eax

qqq:
	ret

%endif

