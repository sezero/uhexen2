;
; r_drawa.asm
; x86 assembly-language edge clipping and emission code
; this file uses NASM syntax.
; $Id: r_drawa.asm,v 1.8 2008-03-16 14:30:46 sezero Exp $
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
 _sym_prefix r_refdef
 _sym_prefix ycenter
 _sym_prefix xcenter
 _sym_prefix r_leftclipped
 _sym_prefix r_leftenter
 _sym_prefix r_rightclipped
 _sym_prefix r_rightenter
 _sym_prefix modelorg
 _sym_prefix xscale
 _sym_prefix yscale
 _sym_prefix r_leftexit
 _sym_prefix r_rightexit
 _sym_prefix r_lastvertvalid
 _sym_prefix cacheoffset
 _sym_prefix newedges
 _sym_prefix removeedges
 _sym_prefix r_pedge
 _sym_prefix r_framecount
 _sym_prefix r_u1
 _sym_prefix r_emitted
 _sym_prefix edge_p
 _sym_prefix surface_p
 _sym_prefix surfaces
 _sym_prefix r_lzi1
 _sym_prefix r_v1
 _sym_prefix r_ceilv1
 _sym_prefix r_nearzi
 _sym_prefix r_nearzionly
 _sym_prefix vright
 _sym_prefix vup
 _sym_prefix vpn
; C-shared globals:
 _sym_prefix R_ClipEdge
%endif	; _sym_prefix

; externs from C code
 extern r_refdef
 extern ycenter
 extern xcenter
 extern r_leftclipped
 extern r_leftenter
 extern r_rightclipped
 extern r_rightenter
 extern modelorg
 extern xscale
 extern yscale
 extern r_leftexit
 extern r_rightexit
 extern r_lastvertvalid
 extern cacheoffset
 extern newedges
 extern removeedges
 extern r_pedge
 extern r_framecount
 extern r_u1
 extern r_emitted
 extern edge_p
 extern surface_p
 extern surfaces
 extern r_lzi1
 extern r_v1
 extern r_ceilv1
 extern r_nearzi
 extern r_nearzionly
 extern vright
 extern vup
 extern vpn

; externs from ASM-only code
 extern float_point5
 extern float_1
 extern float_minus_1
 extern float_0
 extern fp_1m
 extern fp_1m_minus_1
 extern fp_8
 extern fp_16
 extern fp_64k
 extern fp_64kx64k
 extern ceil_cw
 extern single_cw


SEGMENT .data

Ld0 dd 0.0
Ld1 dd 0.0
Lstack dd 0
Lfp_near_clip dd 0.01
Lceilv0 dd 0
Lv dd 0
Lu0 dd 0
Lv0 dd 0
Lzi0 dd 0


SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_ClipEdge
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_ClipEdge
R_ClipEdge:
 push esi
 push edi
 push ebx
 mov  dword [Lstack],esp
 mov ebx, dword [12+12+esp]
 mov esi, dword [4+12+esp]
 mov edx, dword [8+12+esp]
 test ebx,ebx
 jz Lemit
Lcliploop:
 fld  dword [0+0+esi]
 fmul  dword [0+0+ebx]
 fld  dword [0+4+esi]
 fmul  dword [0+4+ebx]
 fld  dword [0+8+esi]
 fmul  dword [0+8+ebx]
 fxch st1
 faddp st2,st0
 fld  dword [0+0+edx]
 fmul  dword [0+0+ebx]
 fld  dword [0+4+edx]
 fmul  dword [0+4+ebx]
 fld  dword [0+8+edx]
 fmul  dword [0+8+ebx]
 fxch st1
 faddp st2,st0
 fxch st3
 faddp st2,st0
 faddp st2,st0
 fsub  dword [12+ebx]
 fxch st1
 fsub  dword [12+ebx]
 fxch st1
 fstp  dword [Ld0]
 fstp  dword [Ld1]
 mov eax, dword [Ld0]
 mov ecx, dword [Ld1]
 or ecx,eax
 js near Lp2
Lcontinue:
 mov ebx, dword [16+ebx]
 test ebx,ebx
 jnz Lcliploop
Lemit:
 fldcw  word [ceil_cw]
 cmp  dword [r_lastvertvalid],0
 jz LCalcFirst
 mov eax, dword [r_lzi1]
 mov ecx, dword [r_u1]
 mov  dword [Lzi0],eax
 mov  dword [Lu0],ecx
 mov ecx, dword [r_v1]
 mov eax, dword [r_ceilv1]
 mov  dword [Lv0],ecx
 mov  dword [Lceilv0],eax
 jmp LCalcSecond
LCalcFirst:
 call LTransformAndProject		; call near LTransformAndProject
 fst  dword [Lv0]
 fxch st2
 fstp  dword [Lu0]
 fstp  dword [Lzi0]
 fistp  dword [Lceilv0]
LCalcSecond:
 mov esi,edx
 call LTransformAndProject		; call near LTransformAndProject
 fld  dword [Lu0]
 fxch st3
 fld  dword [Lzi0]
 fxch st3
 fld  dword [Lv0]
 fxch st3
 fist  dword [r_ceilv1]
 fldcw  word [single_cw]
 fst  dword [r_v1]
 fxch st4
 fcom st1
 fnstsw ax
 test ah,1
 jz LP0
 fstp st0
 fld st0
LP0:
 fxch st1
 fstp  dword [r_lzi1]
 fxch st1
 fst  dword [r_u1]
 fxch st1
 fcom  dword [r_nearzi]
 fnstsw ax
 test ah,045h
 jnz LP1
 fst  dword [r_nearzi]
LP1:
 mov eax, dword [r_nearzionly]
 test eax,eax
 jz LP2
LPop5AndDone:
 mov eax, dword [cacheoffset]
 mov edx, dword [r_framecount]
 cmp eax,07FFFFFFFh
 jz LDoPop
 and edx,07FFFFFFFh
 or edx,080000000h
 mov  dword [cacheoffset],edx
LDoPop:
 fstp st0
 fstp st0
 fstp st0
 fstp st0
 fstp st0
 jmp Ldone
LP2:
 mov ebx, dword [Lceilv0]
 mov edi, dword [edge_p]
 mov ecx, dword [r_ceilv1]
 mov edx,edi
 mov esi, dword [r_pedge]
 add edx,32
 cmp ebx,ecx
 jz LPop5AndDone
 mov eax, dword [r_pedge]
 mov  dword [28+edi],eax
 fstp  dword [24+edi]
 jc LSide0
LSide1:
 fsubp st3,st0
 fsub st0,st1
 fdivp st2,st0
 mov  dword [r_emitted],1
 mov  dword [edge_p],edx
 mov eax, dword [edx]
 mov eax,ecx
 lea ecx, [-1+ebx]
 mov ebx,eax
 mov eax, dword [surface_p]
 mov esi, dword [surfaces]
 sub edx,edx
 sub eax,esi
 shr eax,6
 mov  dword [16+edi],edx
 mov  dword [16+2+edi],eax
 sub esi,esi
 mov  dword [Lv],ebx
 fild  dword [Lv]
 fsubrp st1,st0
 fmul st0,st1
 fadd  dword [r_u1]
 jmp LSideDone
LSide0:
 fsub st0,st3
 fxch st2
 fsub st0,st1
 fdivp st2,st0
 mov  dword [r_emitted],1
 mov  dword [edge_p],edx
 mov eax, dword [edx]
 dec ecx
 mov eax, dword [surface_p]
 mov esi, dword [surfaces]
 sub edx,edx
 sub eax,esi
 shr eax,6
 mov  dword [16+2+edi],edx
 mov  dword [16+edi],eax
 mov esi,1
 mov  dword [Lv],ebx
 fild  dword [Lv]
 fsubrp st1,st0
 fmul st0,st1
 faddp st2,st0
 fxch st1
LSideDone:
 fmul  dword [fp_1m]
 fxch st1
 fmul  dword [fp_1m]
 fxch st1
 fadd  dword [fp_1m_minus_1]
 fxch st1
 fistp  dword [4+edi]
 fistp  dword [0+edi]
 mov eax, dword [0+edi]
 mov edx, dword [r_refdef+76]
 cmp eax,edx
 jl LP4
 mov edx, dword [r_refdef+80]
 cmp eax,edx
 jng LP5
LP4:
 mov  dword [0+edi],edx
 mov eax,edx
LP5:
 add eax,esi
 mov esi, dword [newedges+ebx*4]
 test esi,esi
 jz LDoFirst
 cmp  dword [0+esi],eax
 jl LNotFirst
LDoFirst:
 mov  dword [12+edi],esi
 mov  dword [newedges+ebx*4],edi
 jmp LSetRemove
LNotFirst:
LFindInsertLoop:
 mov edx,esi
 mov esi, dword [12+esi]
 test esi,esi
 jz LInsertFound
 cmp  dword [0+esi],eax
 jl LFindInsertLoop
LInsertFound:
 mov  dword [12+edi],esi
 mov  dword [12+edx],edi
LSetRemove:
 mov eax, dword [removeedges+ecx*4]
 mov  dword [removeedges+ecx*4],edi
 mov  dword [20+edi],eax
Ldone:
 mov esp, dword [Lstack]
 pop ebx
 pop edi
 pop esi
 ret
Lp2:
 test eax,eax
 jns Lp1
 mov eax, dword [Ld1]
 test eax,eax
 jns near Lp3
 mov eax, dword [r_leftclipped]
 mov ecx, dword [r_pedge]
 test eax,eax
 jnz Ldone
 mov eax, dword [r_framecount]
 and eax,07FFFFFFFh
 or eax,080000000h
 mov  dword [cacheoffset],eax
 jmp Ldone
Lp1:
 fld  dword [Ld0]
 fld  dword [Ld1]
 fsubr st0,st1
 mov  dword [cacheoffset],07FFFFFFFh
 fdivp st1,st0
 sub esp,12
 fld  dword [0+8+edx]
 fsub  dword [0+8+esi]
 fld  dword [0+4+edx]
 fsub  dword [0+4+esi]
 fld  dword [0+0+edx]
 fsub  dword [0+0+esi]
 mov edx,esp
 mov eax, dword [20+ebx]
 test al,al
 fmul st0,st3
 fxch st1
 fmul st0,st3
 fxch st2
 fmulp st3,st0
 fadd  dword [0+0+esi]
 fxch st1
 fadd  dword [0+4+esi]
 fxch st2
 fadd  dword [0+8+esi]
 fxch st1
 fstp  dword [0+0+esp]
 fstp  dword [0+8+esp]
 fstp  dword [0+4+esp]
 jz Ltestright
 mov  dword [r_leftclipped],1
 mov eax, dword [0+0+esp]
 mov  dword [r_leftexit+0+0],eax
 mov eax, dword [0+4+esp]
 mov  dword [r_leftexit+0+4],eax
 mov eax, dword [0+8+esp]
 mov  dword [r_leftexit+0+8],eax
 jmp Lcontinue
Ltestright:
 test ah,ah
 jz near Lcontinue
 mov  dword [r_rightclipped],1
 mov eax, dword [0+0+esp]
 mov  dword [r_rightexit+0+0],eax
 mov eax, dword [0+4+esp]
 mov  dword [r_rightexit+0+4],eax
 mov eax, dword [0+8+esp]
 mov  dword [r_rightexit+0+8],eax
 jmp Lcontinue
Lp3:
 mov  dword [r_lastvertvalid],0
 fld  dword [Ld0]
 fld  dword [Ld1]
 fsubr st0,st1
 mov  dword [cacheoffset],07FFFFFFFh
 fdivp st1,st0
 sub esp,12
 fld  dword [0+8+edx]
 fsub  dword [0+8+esi]
 fld  dword [0+4+edx]
 fsub  dword [0+4+esi]
 fld  dword [0+0+edx]
 fsub  dword [0+0+esi]
 mov eax, dword [20+ebx]
 test al,al
 fmul st0,st3
 fxch st1
 fmul st0,st3
 fxch st2
 fmulp st3,st0
 fadd  dword [0+0+esi]
 fxch st1
 fadd  dword [0+4+esi]
 fxch st2
 fadd  dword [0+8+esi]
 fxch st1
 fstp  dword [0+0+esp]
 fstp  dword [0+8+esp]
 fstp  dword [0+4+esp]
 mov esi,esp
 jz Ltestright2
 mov  dword [r_leftclipped],1
 mov eax, dword [0+0+esp]
 mov  dword [r_leftenter+0+0],eax
 mov eax, dword [0+4+esp]
 mov  dword [r_leftenter+0+4],eax
 mov eax, dword [0+8+esp]
 mov  dword [r_leftenter+0+8],eax
 jmp Lcontinue
Ltestright2:
 test ah,ah
 jz near Lcontinue
 mov  dword [r_rightclipped],1
 mov eax, dword [0+0+esp]
 mov  dword [r_rightenter+0+0],eax
 mov eax, dword [0+4+esp]
 mov  dword [r_rightenter+0+4],eax
 mov eax, dword [0+8+esp]
 mov  dword [r_rightenter+0+8],eax
 jmp Lcontinue
LTransformAndProject:
 fld  dword [0+0+esi]
 fsub  dword [modelorg+0]
 fld  dword [0+4+esi]
 fsub  dword [modelorg+4]
 fld  dword [0+8+esi]
 fsub  dword [modelorg+8]
 fxch st2
 fld st0
 fmul  dword [vpn+0]
 fld st1
 fmul  dword [vright+0]
 fxch st2
 fmul  dword [vup+0]
 fld st3
 fmul  dword [vpn+4]
 fld st4
 fmul  dword [vright+4]
 fxch st5
 fmul  dword [vup+4]
 fxch st1
 faddp st3,st0
 fxch st3
 faddp st4,st0
 faddp st2,st0
 fld st3
 fmul  dword [vpn+8]
 fld st4
 fmul  dword [vright+8]
 fxch st5
 fmul  dword [vup+8]
 fxch st1
 faddp st2,st0
 fxch st4
 faddp st3,st0
 fxch st1
 faddp st3,st0
 fcom  dword [Lfp_near_clip]
 fnstsw ax
 test ah,1
 jz LNoClip
 fstp st0
 fld  dword [Lfp_near_clip]
LNoClip:
 fdivr  dword [float_1]
 fxch st1
 fld  dword [xscale]
 fmul st0,st2
 fmulp st1,st0
 fadd  dword [xcenter]
 fcom  dword [r_refdef+68]
 fnstsw ax
 test ah,1
 jz LClampP0
 fstp st0
 fld  dword [r_refdef+68]
LClampP0:
 fcom  dword [r_refdef+84]
 fnstsw ax
 test ah,045h
 jnz LClampP1
 fstp st0
 fld  dword [r_refdef+84]
LClampP1:
 fld st1
 fmul  dword [yscale]
 fmulp st3,st0
 fxch st2
 fsubr  dword [ycenter]
 fcom  dword [r_refdef+72]
 fnstsw ax
 test ah,1
 jz LClampP2
 fstp st0
 fld  dword [r_refdef+72]
LClampP2:
 fcom  dword [r_refdef+88]
 fnstsw ax
 test ah,045h
 jnz LClampP3
 fstp st0
 fld  dword [r_refdef+88]
LClampP3:
 ret

