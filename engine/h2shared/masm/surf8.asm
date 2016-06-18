;
; surf8.asm -- for MASM
; x86 assembly-language 8 bpp surface block drawing code.
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
sb_v dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_Surf8Start
_R_Surf8Start:
 align 4
 public _R_DrawSurfaceBlock8_mip0
_R_DrawSurfaceBlock8_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip0:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,4
 or ebp,0F0000000h
 sar ebx,4
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip0:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[14+esi]
 sar ebp,4
 mov bh,dh
 mov bl,ds:byte ptr[15+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch0:
 mov bl,ds:byte ptr[13+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch1:
 mov cl,ds:byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch2:
 mov bl,ds:byte ptr[11+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch3:
 mov cl,ds:byte ptr[10+esi]
 mov ds:dword ptr[12+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch4:
 mov bl,ds:byte ptr[9+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch5:
 mov cl,ds:byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch6:
 mov bl,ds:byte ptr[7+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch7:
 mov cl,ds:byte ptr[6+esi]
 mov ds:dword ptr[8+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch8:
 mov bl,ds:byte ptr[5+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch9:
 mov cl,ds:byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch10:
 mov bl,ds:byte ptr[3+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch11:
 mov cl,ds:byte ptr[2+esi]
 mov ds:dword ptr[4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch12:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch13:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch14:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch15:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 add esi,ds:dword ptr[_sourcetstep]
 add edi,ds:dword ptr[_surfrowbytes]
 add edx,ds:dword ptr[_lightrightstep]
 add ebp,ds:dword ptr[_lightdeltastep]
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip0
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip0
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip0:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _R_DrawSurfaceBlock8_mip1
_R_DrawSurfaceBlock8_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip1:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,3
 or ebp,070000000h
 sar ebx,3
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip1:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[6+esi]
 sar ebp,3
 mov bh,dh
 mov bl,ds:byte ptr[7+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch22:
 mov bl,ds:byte ptr[5+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch23:
 mov cl,ds:byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch24:
 mov bl,ds:byte ptr[3+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch25:
 mov cl,ds:byte ptr[2+esi]
 mov ds:dword ptr[4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch26:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch27:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch28:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch29:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightrightstep]
 add edx,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 add ebp,eax
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip1
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip1
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip1:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _R_DrawSurfaceBlock8_mip2
_R_DrawSurfaceBlock8_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip2:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,2
 or ebp,030000000h
 sar ebx,2
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip2:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[2+esi]
 sar ebp,2
 mov bh,dh
 mov bl,ds:byte ptr[3+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch18:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch19:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LBPatch20:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch21:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightrightstep]
 add edx,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 add ebp,eax
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip2
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip2
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip2:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _R_DrawSurfaceBlock8_mip3
_R_DrawSurfaceBlock8_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip3:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_lightdelta],ebp
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,1
 sar ebx,1
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 sar ebp,1
 or ebx,0F0000000h
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
 mov bl,ds:byte ptr[1+esi]
 sub ecx,ecx
 mov bh,dh
 mov cl,ds:byte ptr[esi]
 add edx,ebp
 mov ch,dh
 mov al,ds:byte ptr[12345678h+ebx]
LBPatch16:
 mov edx,ds:dword ptr[_lightright]
 mov ds:byte ptr[1+edi],al
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch17:
 mov ds:byte ptr[edi],al
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 mov ebp,ds:dword ptr[_lightdelta]
 mov cl,ds:byte ptr[esi]
 add ebp,eax
 mov eax,ds:dword ptr[_lightrightstep]
 sar ebp,1
 add edx,eax
 mov bh,dh
 mov bl,ds:byte ptr[1+esi]
 add edx,ebp
 mov ch,dh
 mov al,ds:byte ptr[12345678h+ebx]
LBPatch30:
 mov edx,ds:dword ptr[_sourcetstep]
 mov ds:byte ptr[1+edi],al
 mov al,ds:byte ptr[12345678h+ecx]
LBPatch31:
 mov ds:byte ptr[edi],al
 mov ebp,ds:dword ptr[_surfrowbytes]
 add esi,edx
 add edi,ebp
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip3
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip3:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 public _R_Surf8End
_R_Surf8End:
_TEXT ENDS
_DATA SEGMENT
 align 4
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
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_Surf8Patch
_R_Surf8Patch:
 push ebx
 mov eax,ds:dword ptr[_colormap]
 mov ebx,offset LPatchTable8
 mov ecx,32
LPatchLoop8:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop8
 pop ebx
 ret
_TEXT ENDS
 END
