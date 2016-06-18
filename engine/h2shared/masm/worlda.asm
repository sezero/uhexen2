; worlda.asm -- for MASM
; x86 assembly-language server testing stuff
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

_DATA SEGMENT
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _SV_HullPointContents
_SV_HullPointContents:
 push edi
 mov eax,ds:dword ptr[8+4+esp]
 test eax,eax
 js Lhquickout
 push ebx
 mov ebx,ds:dword ptr[4+8+esp]
 push ebp
 mov edx,ds:dword ptr[12+12+esp]
 mov edi,ds:dword ptr[0+ebx]
 mov ebp,ds:dword ptr[4+ebx]
 sub ebx,ebx
 push esi
Lhloop:
 mov ecx,ds:dword ptr[0+edi+eax*8]
 mov eax,ds:dword ptr[4+edi+eax*8]
 mov esi,eax
 ror eax,16
 lea ecx,ds:dword ptr[ecx+ecx*4]
 mov bl,ds:byte ptr[16+ebp+ecx*4]
 cmp bl,3
 jb Lnodot
 fld ds:dword ptr[0+ebp+ecx*4]
 fmul ds:dword ptr[0+edx]
 fld ds:dword ptr[0+4+ebp+ecx*4]
 fmul ds:dword ptr[4+edx]
 fld ds:dword ptr[0+8+ebp+ecx*4]
 fmul ds:dword ptr[8+edx]
 fxch st(1)
 faddp st(2),st(0)
 faddp st(1),st(0)
 fsub ds:dword ptr[12+ebp+ecx*4]
 jmp Lsub
Lnodot:
 fld ds:dword ptr[12+ebp+ecx*4]
 fsubr ds:dword ptr[edx+ebx*4]
Lsub:
 sar eax,16
 sar esi,16
 fstp ds:dword ptr[Ltemp]
 mov ecx,ds:dword ptr[Ltemp]
 sar ecx,31
 and esi,ecx
 xor ecx,0FFFFFFFFh
 and eax,ecx
 or eax,esi
 jns Lhloop
Lhdone:
 pop esi
 pop ebp
 pop ebx
Lhquickout:
 pop edi
 ret
_TEXT ENDS
 END
