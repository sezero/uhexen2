; ftol.asm -- for MASM.
; Replacement for Visual C++ ftol.  Doesn't muck with the fpu control word
; or any of that other time consuming stuff.
;
; Copyright (C) 1997-1998  Raven Software Corp.
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

.386
.MODEL FLAT
OPTION OLDSTRUCTS

.DATA

ftolData		dq	?

.CODE

PUBLIC __ftol
__ftol:
	
	fistp QWORD PTR [ftolData]
	mov eax, DWORD PTR [ftolData]	
	mov edx, DWORD PTR [ftolData+4]

	ret

END
