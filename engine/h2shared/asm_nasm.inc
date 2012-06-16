;
; asm_nasm.inc -- common header for NASM format x86-assembly sources
; $Id: asm_nasm.inc,v 1.1 2008-03-16 14:32:15 sezero Exp $
;
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

