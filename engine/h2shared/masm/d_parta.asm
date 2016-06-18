;
; d_parta.asm -- for MASM
; x86 assembly-language 8-bpp particle-drawing code.
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
 externdef _d_pzbuffer:dword
 externdef _d_zrowbytes:dword
 externdef _d_viewbuffer:dword
 externdef _d_scantable:dword
 externdef _r_origin:dword
 externdef _r_ppn:dword
 externdef _r_pup:dword
 externdef _r_pright:dword
 externdef _ycenter:dword
 externdef _xcenter:dword
 externdef _d_vrectbottom_particle:dword
 externdef _d_vrectright_particle:dword
 externdef _d_vrecty:dword
 externdef _d_vrectx:dword
 externdef _d_pix_shift:dword
 externdef _d_pix_min:dword
 externdef _d_pix_max:dword
 externdef _d_y_aspect_shift:dword
 externdef _screenwidth:dword
 externdef _transTable:dword

; externs from ASM-only code
 externdef float_point5:dword
 externdef izistep:dword
 externdef izi:dword
 externdef float_1:dword
 externdef float_particle_z_clip:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef DP_Count:dword
 externdef DP_u:dword
 externdef DP_v:dword
 externdef DP_32768:dword
 externdef DP_Color:dword
 externdef DP_Pix:dword
 externdef DP_EntryTable:dword
 externdef DP_EntryTransTable:dword

_TEXT SEGMENT
 align 4
 public _D_DrawParticle
_D_DrawParticle:
 push ebp
 push edi
 push ebx
 mov edi,ds:dword ptr[12+4+esp]
 fld ds:dword ptr[_r_origin]
 fsubr ds:dword ptr[0+edi]
 fld ds:dword ptr[0+4+edi]
 fsub ds:dword ptr[_r_origin+4]
 fld ds:dword ptr[0+8+edi]
 fsub ds:dword ptr[_r_origin+8]
 fxch st(2)
 fld ds:dword ptr[_r_ppn]
 fmul st(0),st(1)
 fld ds:dword ptr[_r_ppn+4]
 fmul st(0),st(3)
 fld ds:dword ptr[_r_ppn+8]
 fmul st(0),st(5)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fld st(0)
 fdivr ds:dword ptr[float_1]
 fxch st(1)
 fcomp ds:dword ptr[float_particle_z_clip]
 fxch st(3)
 fld ds:dword ptr[_r_pup]
 fmul st(0),st(2)
 fld ds:dword ptr[_r_pup+4]
 fnstsw ax
 test ah,1
 jnz LPop6AndDone
 fmul st(0),st(4)
 fld ds:dword ptr[_r_pup+8]
 fmul st(0),st(3)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(3)
 fmul ds:dword ptr[_r_pright+4]
 fxch st(2)
 fmul ds:dword ptr[_r_pright]
 fxch st(1)
 fmul ds:dword ptr[_r_pright+8]
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fsubr ds:dword ptr[_ycenter]
 fxch st(1)
 fadd ds:dword ptr[_xcenter]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(2)
 fmul ds:dword ptr[DP_32768]
 fxch st(2)
 fistp ds:dword ptr[DP_u]
 fistp ds:dword ptr[DP_v]
 mov eax,ds:dword ptr[DP_u]
 mov edx,ds:dword ptr[DP_v]
 mov ebx,ds:dword ptr[_d_vrectbottom_particle]
 mov ecx,ds:dword ptr[_d_vrectright_particle]
 cmp edx,ebx
 jg LPop1AndDone
 cmp eax,ecx
 jg LPop1AndDone
 mov ebx,ds:dword ptr[_d_vrecty]
 mov ecx,ds:dword ptr[_d_vrectx]
 cmp edx,ebx
 jl LPop1AndDone
 cmp eax,ecx
 jl LPop1AndDone
 fld ds:dword ptr[12+edi]
 fistp ds:dword ptr[DP_Color]
 mov ebx,ds:dword ptr[_d_viewbuffer]
 add ebx,eax
 mov edi,ds:dword ptr[_d_scantable+edx*4]
 imul edx,ds:dword ptr[_d_zrowbytes]
 lea edx,ds:dword ptr[edx+eax*2]
 mov eax,ds:dword ptr[_d_pzbuffer]
 fistp ds:dword ptr[izi]
 add edi,ebx
 add edx,eax
 mov eax,ds:dword ptr[izi]
 mov ecx,ds:dword ptr[_d_pix_shift]
 shr eax,cl
 mov ebp,ds:dword ptr[izi]
 mov ebx,ds:dword ptr[_d_pix_min]
 mov ecx,ds:dword ptr[_d_pix_max]
 cmp eax,ebx
 jnl LTestPixMax
 mov eax,ebx
 jmp LTestDone
LTestPixMax:
 cmp eax,ecx
 jng LTestDone
 mov eax,ecx
LTestDone:
 mov cx,ds:word ptr[DP_Color]
 mov ebx,ds:dword ptr[_d_y_aspect_shift]
 test ebx,ebx
 jnz LDefault
 cmp eax,4
 ja LDefault

 test ch,ch
 jnz Trans

 jmp dword ptr[DP_EntryTable-4+eax*4]
Trans:
 and ecx, 0ffh
 mov ch, cl
 jmp dword ptr[DP_EntryTransTable-4+eax*4]



 
 public DP_1x1
DP_1x1:
 cmp ds:word ptr[edx],bp
 jg LDone
 mov ds:word ptr[edx],bp
 mov ds:byte ptr[edi],cl
 jmp LDone
 public DP_2x2

DP_2x2:
 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L2x2_1
 mov ds:word ptr[edx],bp
 mov ds:byte ptr[edi],cl
L2x2_1:
 cmp ds:word ptr[2+edx],bp
 jg L2x2_2
 mov ds:word ptr[2+edx],bp
 mov ds:byte ptr[1+edi],cl
L2x2_2:
 cmp ds:word ptr[edx+esi*1],bp
 jg L2x2_3
 mov ds:word ptr[edx+esi*1],bp
 mov ds:byte ptr[edi+ebx*1],cl
L2x2_3:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L2x2_4
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:byte ptr[1+edi+ebx*1],cl
L2x2_4:
 pop esi
 jmp LDone
 public DP_3x3

DP_3x3:
 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L3x3_1
 mov ds:word ptr[edx],bp
 mov ds:byte ptr[edi],cl
L3x3_1:
 cmp ds:word ptr[2+edx],bp
 jg L3x3_2
 mov ds:word ptr[2+edx],bp
 mov ds:byte ptr[1+edi],cl
L3x3_2:
 cmp ds:word ptr[4+edx],bp
 jg L3x3_3
 mov ds:word ptr[4+edx],bp
 mov ds:byte ptr[2+edi],cl
L3x3_3:
 cmp ds:word ptr[edx+esi*1],bp
 jg L3x3_4
 mov ds:word ptr[edx+esi*1],bp
 mov ds:byte ptr[edi+ebx*1],cl
L3x3_4:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L3x3_5
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:byte ptr[1+edi+ebx*1],cl
L3x3_5:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L3x3_6
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:byte ptr[2+edi+ebx*1],cl
L3x3_6:
 cmp ds:word ptr[edx+esi*2],bp
 jg L3x3_7
 mov ds:word ptr[edx+esi*2],bp
 mov ds:byte ptr[edi+ebx*2],cl
L3x3_7:
 cmp ds:word ptr[2+edx+esi*2],bp
 jg L3x3_8
 mov ds:word ptr[2+edx+esi*2],bp
 mov ds:byte ptr[1+edi+ebx*2],cl
L3x3_8:
 cmp ds:word ptr[4+edx+esi*2],bp
 jg L3x3_9
 mov ds:word ptr[4+edx+esi*2],bp
 mov ds:byte ptr[2+edi+ebx*2],cl
L3x3_9:
 pop esi
 jmp LDone
 public DP_4x4

DP_4x4:
 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L4x4_1
 mov ds:word ptr[edx],bp
 mov ds:byte ptr[edi],cl
L4x4_1:
 cmp ds:word ptr[2+edx],bp
 jg L4x4_2
 mov ds:word ptr[2+edx],bp
 mov ds:byte ptr[1+edi],cl
L4x4_2:
 cmp ds:word ptr[4+edx],bp
 jg L4x4_3
 mov ds:word ptr[4+edx],bp
 mov ds:byte ptr[2+edi],cl
L4x4_3:
 cmp ds:word ptr[6+edx],bp
 jg L4x4_4
 mov ds:word ptr[6+edx],bp
 mov ds:byte ptr[3+edi],cl
L4x4_4:
 cmp ds:word ptr[edx+esi*1],bp
 jg L4x4_5
 mov ds:word ptr[edx+esi*1],bp
 mov ds:byte ptr[edi+ebx*1],cl
L4x4_5:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L4x4_6
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:byte ptr[1+edi+ebx*1],cl
L4x4_6:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L4x4_7
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:byte ptr[2+edi+ebx*1],cl
L4x4_7:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg L4x4_8
 mov ds:word ptr[6+edx+esi*1],bp
 mov ds:byte ptr[3+edi+ebx*1],cl
L4x4_8:
 lea edx,ds:dword ptr[edx+esi*2]
 lea edi,ds:dword ptr[edi+ebx*2]
 cmp ds:word ptr[edx],bp
 jg L4x4_9
 mov ds:word ptr[edx],bp
 mov ds:byte ptr[edi],cl
L4x4_9:
 cmp ds:word ptr[2+edx],bp
 jg L4x4_10
 mov ds:word ptr[2+edx],bp
 mov ds:byte ptr[1+edi],cl
L4x4_10:
 cmp ds:word ptr[4+edx],bp
 jg L4x4_11
 mov ds:word ptr[4+edx],bp
 mov ds:byte ptr[2+edi],cl
L4x4_11:
 cmp ds:word ptr[6+edx],bp
 jg L4x4_12
 mov ds:word ptr[6+edx],bp
 mov ds:byte ptr[3+edi],cl
L4x4_12:
 cmp ds:word ptr[edx+esi*1],bp
 jg L4x4_13
 mov ds:word ptr[edx+esi*1],bp
 mov ds:byte ptr[edi+ebx*1],cl
L4x4_13:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L4x4_14
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:byte ptr[1+edi+ebx*1],cl
L4x4_14:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L4x4_15
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:byte ptr[2+edi+ebx*1],cl
L4x4_15:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg L4x4_16
 mov ds:word ptr[6+edx+esi*1],bp
 mov ds:byte ptr[3+edi+ebx*1],cl
L4x4_16:
 pop esi
 jmp LDone



 public DP_T1x1
DP_T1x1:
 mov eax, ds:dword ptr[_transTable]

 cmp ds:word ptr[edx],bp
 jg LDone
 mov ds:word ptr[edx],bp
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi],cl
 jmp LDone
 public DP_T2x2

DP_T2x2:
 mov eax, ds:dword ptr[_transTable]

 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg LT2x2_1
 mov ds:word ptr[edx],bp
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi],cl

LT2x2_1:
 cmp ds:word ptr[2+edx],bp
 jg LT2x2_2
 mov ds:word ptr[2+edx],bp
 mov cl, ds:byte ptr[1+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi],cl

LT2x2_2:
 cmp ds:word ptr[edx+esi*1],bp
 jg LT2x2_3
 mov ds:word ptr[edx+esi*1],bp
 mov cl, ds:byte ptr[edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi+ebx*1],cl

LT2x2_3:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg LT2x2_4
 mov ds:word ptr[2+edx+esi*1],bp
 mov cl, ds:byte ptr[1+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi+ebx*1],cl

LT2x2_4:
 pop esi
 jmp LDone
 public DP_T3x3

DP_T3x3:
 mov eax, ds:dword ptr[_transTable]

 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg LT3x3_1
 mov ds:word ptr[edx],bp
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi],cl

LT3x3_1:
 cmp ds:word ptr[2+edx],bp
 jg LT3x3_2
 mov ds:word ptr[2+edx],bp
 mov cl, ds:byte ptr[1+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi],cl

LT3x3_2:
 cmp ds:word ptr[4+edx],bp
 jg LT3x3_3
 mov ds:word ptr[4+edx],bp
 mov cl, ds:byte ptr[2+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi],cl

LT3x3_3:
 cmp ds:word ptr[edx+esi*1],bp
 jg LT3x3_4
 mov ds:word ptr[edx+esi*1],bp
 mov cl, ds:byte ptr[edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi+ebx*1],cl

LT3x3_4:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg LT3x3_5
 mov ds:word ptr[2+edx+esi*1],bp
 mov cl, ds:byte ptr[1+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi+ebx*1],cl

LT3x3_5:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg LT3x3_6
 mov ds:word ptr[4+edx+esi*1],bp
 mov cl, ds:byte ptr[2+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi+ebx*1],cl

LT3x3_6:
 cmp ds:word ptr[edx+esi*2],bp
 jg LT3x3_7
 mov ds:word ptr[edx+esi*2],bp
 mov cl, ds:byte ptr[edi+ebx*2]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi+ebx*2],cl

LT3x3_7:
 cmp ds:word ptr[2+edx+esi*2],bp
 jg LT3x3_8
 mov ds:word ptr[2+edx+esi*2],bp
 mov cl, ds:byte ptr[1+edi+ebx*2]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi+ebx*2],cl

LT3x3_8:
 cmp ds:word ptr[4+edx+esi*2],bp
 jg LT3x3_9
 mov ds:word ptr[4+edx+esi*2],bp
 mov cl, ds:byte ptr[2+edi+ebx*2]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi+ebx*2],cl

LT3x3_9:
 pop esi
 jmp LDone
 public DP_T4x4

DP_T4x4:
 mov eax, ds:dword ptr[_transTable]

 push esi
 mov ebx,ds:dword ptr[_screenwidth]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg LT4x4_1
 mov ds:word ptr[edx],bp
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi],cl

LT4x4_1:
 cmp ds:word ptr[2+edx],bp
 jg LT4x4_2
 mov ds:word ptr[2+edx],bp
 mov cl, ds:byte ptr[1+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi],cl

LT4x4_2:
 cmp ds:word ptr[4+edx],bp
 jg LT4x4_3
 mov ds:word ptr[4+edx],bp
 mov cl, ds:byte ptr[2+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi],cl

LT4x4_3:
 cmp ds:word ptr[6+edx],bp
 jg LT4x4_4
 mov ds:word ptr[6+edx],bp
 mov cl, ds:byte ptr[3+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[3+edi],cl

LT4x4_4:
 cmp ds:word ptr[edx+esi*1],bp
 jg LT4x4_5
 mov ds:word ptr[edx+esi*1],bp
 mov cl, ds:byte ptr[edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi+ebx*1],cl

LT4x4_5:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg LT4x4_6
 mov ds:word ptr[2+edx+esi*1],bp
 mov cl, ds:byte ptr[1+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi+ebx*1],cl

LT4x4_6:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg LT4x4_7
 mov ds:word ptr[4+edx+esi*1],bp
 mov cl, ds:byte ptr[2+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi+ebx*1],cl

LT4x4_7:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg LT4x4_8
 mov ds:word ptr[6+edx+esi*1],bp
 mov cl, ds:byte ptr[3+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[3+edi+ebx*1],cl

LT4x4_8:
 lea edx,ds:dword ptr[edx+esi*2]
 lea edi,ds:dword ptr[edi+ebx*2]
 cmp ds:word ptr[edx],bp
 jg LT4x4_9
 mov ds:word ptr[edx],bp
 mov cl, ds:byte ptr[edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi],cl

LT4x4_9:
 cmp ds:word ptr[2+edx],bp
 jg LT4x4_10
 mov ds:word ptr[2+edx],bp
 mov cl, ds:byte ptr[1+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi],cl

LT4x4_10:
 cmp ds:word ptr[4+edx],bp
 jg LT4x4_11
 mov ds:word ptr[4+edx],bp
 mov cl, ds:byte ptr[2+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi],cl

LT4x4_11:
 cmp ds:word ptr[6+edx],bp
 jg LT4x4_12
 mov ds:word ptr[6+edx],bp
 mov cl, ds:byte ptr[3+edi]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[3+edi],cl

LT4x4_12:
 cmp ds:word ptr[edx+esi*1],bp
 jg LT4x4_13
 mov ds:word ptr[edx+esi*1],bp
 mov cl, ds:byte ptr[edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[edi+ebx*1],cl

LT4x4_13:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg LT4x4_14
 mov ds:word ptr[2+edx+esi*1],bp
 mov cl, ds:byte ptr[1+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[1+edi+ebx*1],cl
LT4x4_14:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg LT4x4_15
 mov ds:word ptr[4+edx+esi*1],bp
 mov cl, ds:byte ptr[2+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[2+edi+ebx*1],cl

LT4x4_15:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg LT4x4_16
 mov ds:word ptr[6+edx+esi*1],bp
 mov cl, ds:byte ptr[3+edi+ebx*1]
 mov cl, ds:byte ptr[eax+ecx]
 mov ds:byte ptr[3+edi+ebx*1],cl

LT4x4_16:
 pop esi
 jmp LDone



LDefault:
 test ch,ch
 jnz LTDefault

 mov ebx,eax
 mov ds:dword ptr[DP_Pix],eax
 push cx
 mov cl,ds:byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 pop cx

LGenRowLoop:
 mov eax,ds:dword ptr[DP_Pix]
LGenColLoop:
 cmp ds:word ptr[-2+edx+eax*2],bp
 jg LGSkip
 mov ds:word ptr[-2+edx+eax*2],bp
 mov ds:byte ptr[-1+edi+eax*1],cl
LGSkip:
 dec eax
 jnz LGenColLoop
 add edx,ds:dword ptr[_d_zrowbytes]
 add edi,ds:dword ptr[_screenwidth]
 dec ebx
 jnz LGenRowLoop
 jmp LDone

LTDefault:
 push esi
 and ecx, 0ffh
 mov ch, cl
 mov esi, ds:dword ptr[_transTable]

 mov ebx,eax
 mov ds:dword ptr[DP_Pix],eax
 mov cl,ds:byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 
LTGenRowLoop:
 mov eax,ds:dword ptr[DP_Pix]
LTGenColLoop:
 cmp ds:word ptr[-2+edx+eax*2],bp
 jg LTGSkip
 mov ds:word ptr[-2+edx+eax*2],bp

 mov cl, ds:byte ptr[-1+edi+eax*1]
 mov cl, ds:byte ptr[esi+ecx]
 mov ds:byte ptr[-1+edi+eax*1],cl

LTGSkip:
 dec eax
 jnz LTGenColLoop
 add edx,ds:dword ptr[_d_zrowbytes]
 add edi,ds:dword ptr[_screenwidth]
 dec ebx
 jnz LTGenRowLoop

 pop esi

LDone:
 pop ebx
 pop edi
 pop ebp
 ret
LPop6AndDone:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
LPop1AndDone:
 fstp st(0)
 jmp LDone
_TEXT ENDS
 END
