; asm_nasm.inc -- common header for NASM format x86-assembly sources
; Copyright (C) 2008-2012  O.Sezer <sezero@users.sourceforge.net>
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

;
; underscore prefix handling:
; unless we receive _NO_PREFIX, we shall prefix C-shared
; symbols with '_'. for ELF targets, or whenever necessary,
; you must specifically add -D_NO_PREFIX to your NASMFLAGS.
;
%ifndef _NO_PREFIX
	%macro _sym_prefix 1
	%define %1   _ %+ %1
	%endmacro
%endif

;
; other definitions / macros
;
%idefine offset

;
; OS/2 special handlings
;(see the nasm documentation for obj format.)
;
%ifdef __OS2__
BITS 32
; declare segments with proper attributes for OS/2 builds
SEGMENT .data CLASS=DATA ALIGN=16 USE32 FLAT
SEGMENT .text CLASS=CODE ALIGN=16 USE32 FLAT
%endif

