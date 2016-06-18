;
; r_aclipa.asm -- for MASM
; x86 assembly-language Alias model transform and project code.
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
 externdef _r_refdef:dword

; externs from ASM-only code
 externdef float_point5:dword

_DATA SEGMENT
Ltemp0 dd 0
Ltemp1 dd 0
_DATA ENDS
_TEXT SEGMENT
 public _R_Alias_clip_bottom
_R_Alias_clip_bottom:
 push esi
 push edi
 mov esi,ds:dword ptr[8+4+esp]
 mov edi,ds:dword ptr[8+8+esp]
 mov eax,ds:dword ptr[_r_refdef+52]
LDoForwardOrBackward:
 mov edx,ds:dword ptr[0+4+esi]
 mov ecx,ds:dword ptr[0+4+edi]
 cmp edx,ecx
 jl LDoForward
 mov ecx,ds:dword ptr[0+4+esi]
 mov edx,ds:dword ptr[0+4+edi]
 mov edi,ds:dword ptr[8+4+esp]
 mov esi,ds:dword ptr[8+8+esp]
LDoForward:
 sub ecx,edx
 sub eax,edx
 mov ds:dword ptr[Ltemp1],ecx
 mov ds:dword ptr[Ltemp0],eax
 fild ds:dword ptr[Ltemp1]
 fild ds:dword ptr[Ltemp0]
 mov edx,ds:dword ptr[8+12+esp]
 mov eax,2
 fdivrp st(1),st(0)
LDo3Forward:
 fild ds:dword ptr[0+0+esi]
 fild ds:dword ptr[0+0+edi]
 fild ds:dword ptr[0+4+esi]
 fild ds:dword ptr[0+4+edi]
 fild ds:dword ptr[0+8+esi]
 fild ds:dword ptr[0+8+edi]
 fxch st(5)
 fsub st(4),st(0)
 fxch st(3)
 fsub st(2),st(0)
 fxch st(1)
 fsub st(5),st(0)
 fxch st(6)
 fmul st(4),st(0)
 add edi,12
 fmul st(2),st(0)
 add esi,12
 add edx,12
 fmul st(5),st(0)
 fxch st(3)
 faddp st(4),st(0)
 faddp st(1),st(0)
 fxch st(4)
 faddp st(3),st(0)
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(3)
 fadd ds:dword ptr[float_point5]
 fxch st(2)
 fadd ds:dword ptr[float_point5]
 fxch st(3)
 fistp ds:dword ptr[0+0-12+edx]
 fxch st(1)
 fistp ds:dword ptr[0+4-12+edx]
 fxch st(1)
 fistp ds:dword ptr[0+8-12+edx]
 dec eax
 jnz LDo3Forward
 fstp st(0)
 pop edi
 pop esi
 ret
 public _R_Alias_clip_top
_R_Alias_clip_top:
 push esi
 push edi
 mov esi,ds:dword ptr[8+4+esp]
 mov edi,ds:dword ptr[8+8+esp]
 mov eax,ds:dword ptr[_r_refdef+20+4]
 jmp LDoForwardOrBackward
 public _R_Alias_clip_right
_R_Alias_clip_right:
 push esi
 push edi
 mov esi,ds:dword ptr[8+4+esp]
 mov edi,ds:dword ptr[8+8+esp]
 mov eax,ds:dword ptr[_r_refdef+48]
LRightLeftEntry:
 mov edx,ds:dword ptr[0+4+esi]
 mov ecx,ds:dword ptr[0+4+edi]
 cmp edx,ecx
 mov edx,ds:dword ptr[0+0+esi]
 mov ecx,ds:dword ptr[0+0+edi]
 jl LDoForward2
 mov ecx,ds:dword ptr[0+0+esi]
 mov edx,ds:dword ptr[0+0+edi]
 mov edi,ds:dword ptr[8+4+esp]
 mov esi,ds:dword ptr[8+8+esp]
LDoForward2:
 jmp LDoForward
 public _R_Alias_clip_left
_R_Alias_clip_left:
 push esi
 push edi
 mov esi,ds:dword ptr[8+4+esp]
 mov edi,ds:dword ptr[8+8+esp]
 mov eax,ds:dword ptr[_r_refdef+20+0]
 jmp LRightLeftEntry
_TEXT ENDS
 END
