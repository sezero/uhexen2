;
; r_aliasa.asm -- for MASM
; x86 assembly-language Alias model transform and project code.
;
; Copyright (C) 1996-1997  Id Software, Inc.
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

 .386P
 .model FLAT
; externs from C code
 externdef _r_apverts:dword
 externdef _r_anumverts:dword
 externdef _aliastransform:dword
 externdef _r_avertexnormals:dword
 externdef _r_plightvec:dword
 externdef _r_ambientlight:dword
 externdef _r_shadelight:dword
 externdef _aliasxcenter:dword
 externdef _aliasycenter:dword

; externs from ASM-only code

_DATA SEGMENT
Lfloat_1 dd 1.0
Ltemp dd 0
Lcoords dd 0, 0, 0
_DATA ENDS
_TEXT SEGMENT
 public _R_AliasTransformAndProjectFinalVerts
_R_AliasTransformAndProjectFinalVerts:
 push ebp
 push edi
 push esi
 mov esi,dword ptr[_r_apverts]
 mov ebp,dword ptr[12+8+esp]
 mov edi,dword ptr[12+4+esp]
 mov ecx,dword ptr[_r_anumverts]
 sub edx,edx
Lloop:
 mov dl,byte ptr[esi]
 mov byte ptr[Lcoords],dl
 fild dword ptr[Lcoords]
 mov dl,byte ptr[1+esi]
 mov byte ptr[Lcoords+4],dl
 fild dword ptr[Lcoords+4]
 mov dl,byte ptr[2+esi]
 mov byte ptr[Lcoords+8],dl
 fild dword ptr[Lcoords+8]
 fld st(2)
 fmul dword ptr[_aliastransform+32]
 fld st(2)
 fmul dword ptr[_aliastransform+36]
 fxch st(1)
 fadd dword ptr[_aliastransform+44]
 fld st(2)
 fmul dword ptr[_aliastransform+40]
 fxch st(1)
 faddp st(2),st(0)
 mov dl,byte ptr[3+esi]
; mov eax,dword ptr[4+ebp]	;load .s
; mov dword ptr[0+8+edi],eax	;store .s
 faddp st(1),st(0)
; mov eax,dword ptr[8+ebp]	;.t
; mov dword ptr[0+12+edi],eax	;.t
 fdivr dword ptr[Lfloat_1]
; mov eax,dword ptr[0+ebp]	; .onseam
; mov dword ptr[24+edi],eax
 
 mov eax,dword ptr[32+edi]
 mov eax,dword ptr[12+ebp]
 mov eax,dword ptr[4+esi]
 lea eax,dword ptr[edx+edx*2]
 fxch st(3)
 fld dword ptr[_r_avertexnormals+eax*4]
 fmul dword ptr[_r_plightvec]
 fld dword ptr[_r_avertexnormals+4+eax*4]
 fmul dword ptr[_r_plightvec+4]
 fld dword ptr[_r_avertexnormals+8+eax*4]
 fmul dword ptr[_r_plightvec+8]
 fxch st(1)
 faddp st(2),st(0)
 fld st(2)
 fmul dword ptr[_aliastransform+0]
 fxch st(2)
 faddp st(1),st(0)
 fst dword ptr[Ltemp]
 mov eax,dword ptr[_r_ambientlight]
 mov dl,byte ptr[Ltemp+3]
 test dl,080h
 jz Lsavelight
 fmul dword ptr[_r_shadelight]
 fistp dword ptr[Ltemp]
 add eax,dword ptr[Ltemp]
 jns Lp1
 sub eax,eax
Lp1:
 fxch st(1)
 fmul dword ptr[_aliastransform+16]
 fxch st(3)
 fld st(0)
 fmul dword ptr[_aliastransform+4]
 fxch st(1)
 mov dword ptr[0+16+edi],eax
 fmul dword ptr[_aliastransform+20]
 fxch st(2)
 fadd dword ptr[_aliastransform+12]
 fxch st(4)
 fadd dword ptr[_aliastransform+28]
 fxch st(3)
 fld st(0)
 fmul dword ptr[_aliastransform+8]
 fxch st(1)
 fmul dword ptr[_aliastransform+24]
 fxch st(5)
 faddp st(2),st(0)
 fxch st(3)
 faddp st(2),st(0)
 add esi,4
 faddp st(2),st(0)
 faddp st(2),st(0)
 add ebp,12
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fadd dword ptr[_aliasxcenter]
 fxch st(1)
 fadd dword ptr[_aliasycenter]
 fxch st(2)
 fistp dword ptr[0+20+edi]
 fistp dword ptr[0+0+edi]
 fistp dword ptr[0+4+edi]
 add edi,32
 dec ecx
 jnz Lloop
 pop esi
 pop edi
 pop ebp
 ret
Lsavelight:
 fstp st(0)
 jmp Lp1
_TEXT ENDS
 END
