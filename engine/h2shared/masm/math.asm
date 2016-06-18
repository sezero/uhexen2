;
; math.asm -- for MASM
; x86 assembly-language math routines.
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

 .386P
 .model FLAT
; externs from C code
 externdef _vright:dword
 externdef _vup:dword
 externdef _vpn:dword
 externdef _BOPS_Error:dword

; externs from ASM-only code

_DATA SEGMENT
 align 4
Ljmptab dd Lcase0, Lcase1, Lcase2, Lcase3
 dd Lcase4, Lcase5, Lcase6, Lcase7
_DATA ENDS
_TEXT SEGMENT
 public _Invert24To16
_Invert24To16:
 mov ecx,ds:dword ptr[4+esp]
 mov edx,0100h
 cmp ecx,edx
 jle LOutOfRange
 sub eax,eax
 div ecx
 ret
LOutOfRange:
 mov eax,0FFFFFFFFh
 ret
 align 2
 public _TransformVector
_TransformVector:
 mov eax,ds:dword ptr[4+esp]
 mov edx,ds:dword ptr[8+esp]
 fld ds:dword ptr[eax]
 fmul ds:dword ptr[_vright]
 fld ds:dword ptr[eax]
 fmul ds:dword ptr[_vup]
 fld ds:dword ptr[eax]
 fmul ds:dword ptr[_vpn]
 fld ds:dword ptr[4+eax]
 fmul ds:dword ptr[_vright+4]
 fld ds:dword ptr[4+eax]
 fmul ds:dword ptr[_vup+4]
 fld ds:dword ptr[4+eax]
 fmul ds:dword ptr[_vpn+4]
 fxch st(2)
 faddp st(5),st(0)
 faddp st(3),st(0)
 faddp st(1),st(0)
 fld ds:dword ptr[8+eax]
 fmul ds:dword ptr[_vright+8]
 fld ds:dword ptr[8+eax]
 fmul ds:dword ptr[_vup+8]
 fld ds:dword ptr[8+eax]
 fmul ds:dword ptr[_vpn+8]
 fxch st(2)
 faddp st(5),st(0)
 faddp st(3),st(0)
 faddp st(1),st(0)
 fstp ds:dword ptr[8+edx]
 fstp ds:dword ptr[4+edx]
 fstp ds:dword ptr[edx]
 ret
 align 2
 public _BoxOnPlaneSide
_BoxOnPlaneSide:
 push ebx
 mov edx,ds:dword ptr[4+12+esp]
 mov ecx,ds:dword ptr[4+4+esp]
 xor eax,eax
 mov ebx,ds:dword ptr[4+8+esp]
 mov al,ds:byte ptr[17+edx]
 cmp al,8
 jge Lerror
 fld ds:dword ptr[0+edx]
 fld st(0)
 jmp dword ptr[Ljmptab+eax*4]
Lcase0:
 fmul ds:dword ptr[ebx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ebx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase1:
 fmul ds:dword ptr[ecx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ebx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase2:
 fmul ds:dword ptr[ebx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ecx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase3:
 fmul ds:dword ptr[ecx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ecx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase4:
 fmul ds:dword ptr[ebx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ebx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase5:
 fmul ds:dword ptr[ecx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ebx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase6:
 fmul ds:dword ptr[ebx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ecx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ecx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
 jmp LSetSides
Lcase7:
 fmul ds:dword ptr[ecx]
 fld ds:dword ptr[0+4+edx]
 fxch st(2)
 fmul ds:dword ptr[ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[4+ecx]
 fld ds:dword ptr[0+8+edx]
 fxch st(2)
 fmul ds:dword ptr[4+ebx]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[8+ecx]
 fxch st(5)
 faddp st(3),st(0)
 fmul ds:dword ptr[8+ebx]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(2),st(0)
LSetSides:
 faddp st(2),st(0)
 fcomp ds:dword ptr[12+edx]
 xor ecx,ecx
 fnstsw ax
 fcomp ds:dword ptr[12+edx]
 and ah,1
 xor ah,1
 add cl,ah
 fnstsw ax
 and ah,1
 add ah,ah
 add cl,ah
 pop ebx
 mov eax,ecx
 ret
Lerror:
 call near ptr _BOPS_Error
_TEXT ENDS
 END
