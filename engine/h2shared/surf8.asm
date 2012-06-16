;
; surf8.asm
; x86 assembly-language 8 bpp surface block drawing code.
; this file uses NASM syntax.
; $Id: surf8.asm,v 1.5 2008-03-16 14:30:46 sezero Exp $
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
 _sym_prefix prowdestbase
 _sym_prefix pbasesource
 _sym_prefix lightright
 _sym_prefix lightrightstep
 _sym_prefix lightleft
 _sym_prefix lightleftstep
 _sym_prefix lightdeltastep
 _sym_prefix lightdelta
 _sym_prefix sourcetstep
 _sym_prefix surfrowbytes
 _sym_prefix colormap
 _sym_prefix blocksize
 _sym_prefix sourcesstep
 _sym_prefix blockdivshift
 _sym_prefix blockdivmask
 _sym_prefix r_lightptr
 _sym_prefix r_lightwidth
 _sym_prefix r_numvblocks
 _sym_prefix r_sourcemax
 _sym_prefix r_stepback
; C-shared globals:
 _sym_prefix R_Surf8Start
 _sym_prefix R_DrawSurfaceBlock8_mip0
 _sym_prefix R_DrawSurfaceBlock8_mip1
 _sym_prefix R_DrawSurfaceBlock8_mip2
 _sym_prefix R_DrawSurfaceBlock8_mip3
 _sym_prefix R_Surf8End
 _sym_prefix R_Surf8Patch
%endif	; _sym_prefix

; externs from C code
 extern prowdestbase
 extern pbasesource
 extern lightright
 extern lightrightstep
 extern lightleft
 extern lightleftstep
 extern lightdeltastep
 extern lightdelta
 extern sourcetstep
 extern surfrowbytes
 extern colormap
 extern blocksize
 extern sourcesstep
 extern blockdivshift
 extern blockdivmask
 extern r_lightptr
 extern r_lightwidth
 extern r_numvblocks
 extern r_sourcemax
 extern r_stepback

; externs from ASM-only code


SEGMENT .data

sb_v dd 0

SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf8Start
;;;;;;;;;;;;;;;;;;;;;;;;
 global R_Surf8Start
R_Surf8Start:

;;;;;;;;;;;;;;;;;;;;;;;;
; R_DrawSurfaceBlock8_mip0
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_DrawSurfaceBlock8_mip0
R_DrawSurfaceBlock8_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip0:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,4
 or ebp,0F0000000h
 sar ebx,4
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip0:
 mov  dword [lightdelta],ebp
 mov cl, byte [14+esi]
 sar ebp,4
 mov bh,dh
 mov bl, byte [15+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch0:
 mov bl, byte [13+esi]
 mov al, byte [12345678h+ecx]
LBPatch1:
 mov cl, byte [12+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch2:
 mov bl, byte [11+esi]
 mov al, byte [12345678h+ecx]
LBPatch3:
 mov cl, byte [10+esi]
 mov  dword [12+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch4:
 mov bl, byte [9+esi]
 mov al, byte [12345678h+ecx]
LBPatch5:
 mov cl, byte [8+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch6:
 mov bl, byte [7+esi]
 mov al, byte [12345678h+ecx]
LBPatch7:
 mov cl, byte [6+esi]
 mov  dword [8+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch8:
 mov bl, byte [5+esi]
 mov al, byte [12345678h+ecx]
LBPatch9:
 mov cl, byte [4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch10:
 mov bl, byte [3+esi]
 mov al, byte [12345678h+ecx]
LBPatch11:
 mov cl, byte [2+esi]
 mov  dword [4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch12:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch13:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch14:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch15:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 add esi, dword [sourcetstep]
 add edi, dword [surfrowbytes]
 add edx, dword [lightrightstep]
 add ebp, dword [lightdeltastep]
 mov  dword [lightright],edx
 jc near Lblockloop8_mip0
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip0
 sub esi, dword [r_stepback]
LSkip_mip0:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; R_DrawSurfaceBlock8_mip1
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_DrawSurfaceBlock8_mip1
R_DrawSurfaceBlock8_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip1:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,3
 or ebp,070000000h
 sar ebx,3
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip1:
 mov  dword [lightdelta],ebp
 mov cl, byte [6+esi]
 sar ebp,3
 mov bh,dh
 mov bl, byte [7+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch22:
 mov bl, byte [5+esi]
 mov al, byte [12345678h+ecx]
LBPatch23:
 mov cl, byte [4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch24:
 mov bl, byte [3+esi]
 mov al, byte [12345678h+ecx]
LBPatch25:
 mov cl, byte [2+esi]
 mov  dword [4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch26:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch27:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch28:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch29:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightrightstep]
 add edx,eax
 mov eax, dword [lightdeltastep]
 add ebp,eax
 mov  dword [lightright],edx
 jc near Lblockloop8_mip1
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip1
 sub esi, dword [r_stepback]
LSkip_mip1:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; R_DrawSurfaceBlock8_mip2
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_DrawSurfaceBlock8_mip2
R_DrawSurfaceBlock8_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip2:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,2
 or ebp,030000000h
 sar ebx,2
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip2:
 mov  dword [lightdelta],ebp
 mov cl, byte [2+esi]
 sar ebp,2
 mov bh,dh
 mov bl, byte [3+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah, byte [12345678h+ebx]
LBPatch18:
 mov bl, byte [1+esi]
 mov al, byte [12345678h+ecx]
LBPatch19:
 mov cl, byte [esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah, byte [12345678h+ebx]
LBPatch20:
 mov edx, dword [lightright]
 mov al, byte [12345678h+ecx]
LBPatch21:
 mov ebp, dword [lightdelta]
 mov  dword [edi],eax
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightrightstep]
 add edx,eax
 mov eax, dword [lightdeltastep]
 add ebp,eax
 mov  dword [lightright],edx
 jc Lblockloop8_mip2
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip2
 sub esi, dword [r_stepback]
LSkip_mip2:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; R_DrawSurfaceBlock8_mip3
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_DrawSurfaceBlock8_mip3
R_DrawSurfaceBlock8_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx, dword [r_lightptr]
 mov eax, dword [r_numvblocks]
 mov  dword [sb_v],eax
 mov edi, dword [prowdestbase]
 mov esi, dword [pbasesource]
Lv_loop_mip3:
 mov eax, dword [ebx]
 mov edx, dword [4+ebx]
 mov ebp,eax
 mov ecx, dword [r_lightwidth]
 mov  dword [lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx, [ebx+ecx*4]
 mov  dword [lightdelta],ebp
 mov  dword [r_lightptr],ebx
 mov ecx, dword [4+ebx]
 mov ebx, dword [ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,1
 sar ebx,1
 mov  dword [lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 sar ebp,1
 or ebx,0F0000000h
 mov  dword [lightdeltastep],ebx
 sub ebx,ebx
 mov bl, byte [1+esi]
 sub ecx,ecx
 mov bh,dh
 mov cl, byte [esi]
 add edx,ebp
 mov ch,dh
 mov al, byte [12345678h+ebx]
LBPatch16:
 mov edx, dword [lightright]
 mov  byte [1+edi],al
 mov al, byte [12345678h+ecx]
LBPatch17:
 mov  byte [edi],al
 mov eax, dword [sourcetstep]
 add esi,eax
 mov eax, dword [surfrowbytes]
 add edi,eax
 mov eax, dword [lightdeltastep]
 mov ebp, dword [lightdelta]
 mov cl, byte [esi]
 add ebp,eax
 mov eax, dword [lightrightstep]
 sar ebp,1
 add edx,eax
 mov bh,dh
 mov bl, byte [1+esi]
 add edx,ebp
 mov ch,dh
 mov al, byte [12345678h+ebx]
LBPatch30:
 mov edx, dword [sourcetstep]
 mov  byte [1+edi],al
 mov al, byte [12345678h+ecx]
LBPatch31:
 mov  byte [edi],al
 mov ebp, dword [surfrowbytes]
 add esi,edx
 add edi,ebp
 cmp esi, dword [r_sourcemax]
 jb LSkip_mip3
 sub esi, dword [r_stepback]
LSkip_mip3:
 mov ebx, dword [r_lightptr]
 dec  dword [sb_v]
 jnz near Lv_loop_mip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf8End
;;;;;;;;;;;;;;;;;;;;;;;;
 global R_Surf8End
R_Surf8End:

SEGMENT .data

 ALIGN 4

LPatchTable8:
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
 dd LBPatch16-4
 dd LBPatch17-4
 dd LBPatch18-4
 dd LBPatch19-4
 dd LBPatch20-4
 dd LBPatch21-4
 dd LBPatch22-4
 dd LBPatch23-4
 dd LBPatch24-4
 dd LBPatch25-4
 dd LBPatch26-4
 dd LBPatch27-4
 dd LBPatch28-4
 dd LBPatch29-4
 dd LBPatch30-4
 dd LBPatch31-4

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; R_Surf8Patch
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global R_Surf8Patch
R_Surf8Patch:
 push ebx
 mov eax, dword [colormap]
 mov ebx,offset LPatchTable8
 mov ecx,32
LPatchLoop8:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop8
 pop ebx
 ret

