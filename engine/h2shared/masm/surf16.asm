;
; surf16.asm -- for MASM
; x86 assembly-language 16 bpp surface block drawing code.
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
 externdef _prowdestbase:dword
 externdef _pbasesource:dword
 externdef _lightright:dword
 externdef _lightrightstep:dword
 externdef _lightleft:dword
 externdef _lightleftstep:dword
 externdef _lightdeltastep:dword
 externdef _lightdelta:dword
 externdef _sourcetstep:dword
 externdef _surfrowbytes:dword
 externdef _colormap:dword
 externdef _blocksize:dword
 externdef _sourcesstep:dword
 externdef _blockdivshift:dword
 externdef _blockdivmask:dword
 externdef _r_lightptr:dword
 externdef _r_lightwidth:dword
 externdef _r_numvblocks:dword
 externdef _r_sourcemax:dword
 externdef _r_stepback:dword

; externs from ASM-only code

_DATA SEGMENT
k dd 0
loopentry dd 0
 align 4
blockjumptable16:
 dd LEnter2_16
 dd LEnter4_16
 dd 0, LEnter8_16
 dd 0, 0, 0, LEnter16_16
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_Surf16Start
_R_Surf16Start:
 align 4
 public _R_DrawSurfaceBlock16
_R_DrawSurfaceBlock16:
 push ebp
 push edi
 push esi
 push ebx
 mov eax,ds:dword ptr[_blocksize]
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
 mov ebx,ds:dword ptr[_sourcesstep]
 mov ecx,ds:dword ptr[blockjumptable16-4+eax*2]
 mov ds:dword ptr[k],eax
 mov ds:dword ptr[loopentry],ecx
 mov edx,ds:dword ptr[_lightleft]
 mov ebp,ds:dword ptr[_lightright]
Lblockloop16:
 sub ebp,edx
 mov cl,ds:byte ptr[_blockdivshift]
 sar ebp,cl
 jns Lp1_16
 test ebp,ds:dword ptr[_blockdivmask]
 jz Lp1_16
 inc ebp
Lp1_16:
 sub eax,eax
 sub ecx,ecx
 jmp dword ptr[loopentry]
 align 4
LEnter16_16:
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch0:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch1:
 mov ds:word ptr[2+edi],cx
 add edi,04h
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch2:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch3:
 mov ds:word ptr[2+edi],cx
 add edi,04h
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch4:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch5:
 mov ds:word ptr[2+edi],cx
 add edi,04h
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch6:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch7:
 mov ds:word ptr[2+edi],cx
 add edi,04h
LEnter8_16:
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch8:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch9:
 mov ds:word ptr[2+edi],cx
 add edi,04h
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch10:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch11:
 mov ds:word ptr[2+edi],cx
 add edi,04h
LEnter4_16:
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch12:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch13:
 mov ds:word ptr[2+edi],cx
 add edi,04h
LEnter2_16:
 mov al,ds:byte ptr[esi]
 mov cl,ds:byte ptr[esi+ebx]
 mov ah,dh
 add edx,ebp
 mov ch,dh
 lea esi,ds:dword ptr[esi+ebx*2]
 mov ax,ds:word ptr[12345678h+eax*2]
LBPatch14:
 add edx,ebp
 mov ds:word ptr[edi],ax
 mov cx,ds:word ptr[12345678h+ecx*2]
LBPatch15:
 mov ds:word ptr[2+edi],cx
 add edi,04h
 mov esi,ds:dword ptr[_pbasesource]
 mov edx,ds:dword ptr[_lightleft]
 mov ebp,ds:dword ptr[_lightright]
 mov eax,ds:dword ptr[_sourcetstep]
 mov ecx,ds:dword ptr[_lightrightstep]
 mov edi,ds:dword ptr[_prowdestbase]
 add esi,eax
 add ebp,ecx
 mov eax,ds:dword ptr[_lightleftstep]
 mov ecx,ds:dword ptr[_surfrowbytes]
 add edx,eax
 add edi,ecx
 mov ds:dword ptr[_pbasesource],esi
 mov ds:dword ptr[_lightright],ebp
 mov eax,ds:dword ptr[k]
 mov ds:dword ptr[_lightleft],edx
 dec eax
 mov ds:dword ptr[_prowdestbase],edi
 mov ds:dword ptr[k],eax
 jnz Lblockloop16
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 public _R_Surf16End
_R_Surf16End:
_TEXT ENDS
_DATA SEGMENT
 align 4
LPatchTable16:
 dd LBPatch0-4
 dd LBPatch1-4
 dd LBPatch2-4
 dd LBPatch3-4
 dd LBPatch4-4
 dd LBPatch5-4
 dd LBPatch6-4
 dd LBPatch7-4
 dd LBPatch8-4
 dd LBPatch9-4
 dd LBPatch10-4
 dd LBPatch11-4
 dd LBPatch12-4
 dd LBPatch13-4
 dd LBPatch14-4
 dd LBPatch15-4
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_Surf16Patch
_R_Surf16Patch:
 push ebx
 mov eax,ds:dword ptr[_colormap]
 mov ebx,offset LPatchTable16
 mov ecx,16
LPatchLoop16:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop16
 pop ebx
 ret
_TEXT ENDS
 END
