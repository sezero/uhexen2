;
; d_copy.asm
; x86 assembly-language screen copying code.
; this file uses NASM syntax.
; $Id$
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

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
_sym_prefix VGA_bufferrowbytes
_sym_prefix VGA_rowbytes
_sym_prefix VGA_pagebase
_sym_prefix VGA_height
_sym_prefix VGA_width
; C-shared globals:
_sym_prefix VGA_UpdatePlanarScreen
_sym_prefix VGA_UpdateLinearScreen
%endif	; _sym_prefix

; externs from C code
;
	extern	VGA_bufferrowbytes
	extern	VGA_rowbytes
	extern	VGA_pagebase
	extern	VGA_height
	extern	VGA_width

; externs from ASM-only code
;


SEGMENT .data

	LCopyWidth	dd	0
	LBlockSrcStep	dd	0
	LBlockDestStep	dd	0
	LSrcDelta	dd	0
	LDestDelta	dd	0

; copies 16 rows per plane at a pop; idea is that 16*512 = 8k, and since
; no Mode X mode is wider than 360, all the data should fit in the cache for
; the passes for the next 3 planes

SEGMENT .text

; void VGA_UpdatePlanarScreen(void *bufptr);
global VGA_UpdatePlanarScreen
VGA_UpdatePlanarScreen:
	push	ebp			; preserve caller's stack frame
	push	edi
	push	esi			; preserve register variables
	push	ebx

	mov	eax, [VGA_bufferrowbytes]
	shl	eax,1
	mov	[LBlockSrcStep],eax
	mov	eax, [VGA_rowbytes]
	shl	eax,1
	mov	[LBlockDestStep],eax

	mov	edx,03C4h
	mov	al,2
	out	dx,al			; point the SC to the Map Mask
	inc	edx

	mov	esi, [esp+4+16]		; bufptr
	mov	edi, [VGA_pagebase]
	mov	ebp, [VGA_height]
	shr	ebp,1

	mov	ecx, [VGA_width]
	mov	eax, [VGA_bufferrowbytes]
	sub	eax,ecx
	mov	[LSrcDelta],eax
	mov	eax, [VGA_rowbytes]
	shl	eax,2
	sub	eax,ecx
	mov	[LDestDelta],eax
	shr	ecx,4
	mov	[LCopyWidth],ecx

LRowLoop:
	mov	al,1

LPlaneLoop:
	out	dx,al
	mov	ah,2

	push	esi
	push	edi
LRowSetLoop:
	mov	ecx, [LCopyWidth]
LColumnLoop:
	mov	bh, [esi+12]
	mov	bl, [esi+8]
	shl	ebx,16
	mov	bh, [esi+4]
	mov	bl, [esi]
	mov	[edi],ebx
	add	esi,16
	add	edi,4
	dec	ecx
	jnz	LColumnLoop

	add	edi, [LDestDelta]
	add	esi, [LSrcDelta]
	dec	ah
	jnz		LRowSetLoop

	pop	edi
	pop	esi
	inc	esi

	shl	al,1
	cmp	al,16
	jnz	LPlaneLoop

	sub	esi,4
	add	esi, [LBlockSrcStep]
	add	edi, [LBlockDestStep]
	dec	ebp
	jnz		LRowLoop

	pop	ebx			; restore register variables
	pop	esi
	pop	edi
	pop	ebp			; restore the caller's stack frame

	ret


; void VGA_UpdateLinearScreen(	void *srcptr, void *destptr,
;				int width, int height,
;				int srcrowbytes, int destrowbytes);
global VGA_UpdateLinearScreen
VGA_UpdateLinearScreen:
	push	ebp			; preserve caller's stack frame
	push	edi
	push	esi			; preserve register variables
	push	ebx

	cld
	mov	esi, [esp+4+16]		; srcptr
	mov	edi, [esp+8+16]		; destptr
	mov	ebx, [esp+12+16]	; width
	mov	eax, [esp+20+16]	; srcrowbytes
	sub	eax,ebx
	mov	edx, [esp+24+16]	; destrowbytes
	sub	edx,ebx
	shr	ebx,2
	mov	ebp, [esp+16+16]	; height
LLRowLoop:
	mov	ecx,ebx
	rep	movsd	; rep/movsl (%esi),(%edi)
	add	esi,eax
	add	edi,edx
	dec	ebp
	jnz		LLRowLoop

	pop	ebx			; restore register variables
	pop	esi
	pop	edi
	pop	ebp			; restore the caller's stack frame

	ret

