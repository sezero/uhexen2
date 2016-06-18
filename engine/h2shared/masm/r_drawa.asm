;
; r_drawa.asm -- for MASM
; x86 assembly-language edge clipping and emission code
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
 externdef _ycenter:dword
 externdef _xcenter:dword
 externdef _r_leftclipped:dword
 externdef _r_leftenter:dword
 externdef _r_rightclipped:dword
 externdef _r_rightenter:dword
 externdef _modelorg:dword
 externdef _xscale:dword
 externdef _yscale:dword
 externdef _r_leftexit:dword
 externdef _r_rightexit:dword
 externdef _r_lastvertvalid:dword
 externdef _cacheoffset:dword
 externdef _newedges:dword
 externdef _removeedges:dword
 externdef _r_pedge:dword
 externdef _r_framecount:dword
 externdef _r_u1:dword
 externdef _r_emitted:dword
 externdef _edge_p:dword
 externdef _surface_p:dword
 externdef _surfaces:dword
 externdef _r_lzi1:dword
 externdef _r_v1:dword
 externdef _r_ceilv1:dword
 externdef _r_nearzi:dword
 externdef _r_nearzionly:dword
 externdef _vright:dword
 externdef _vup:dword
 externdef _vpn:dword

; externs from ASM-only code
 externdef float_point5:dword
 externdef float_1:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef fp_1m:dword
 externdef fp_1m_minus_1:dword
 externdef fp_8:dword
 externdef fp_16:dword
 externdef fp_64k:dword
 externdef fp_64kx64k:dword
 externdef ceil_cw:dword
 externdef single_cw:dword

_DATA SEGMENT
Ld0 dd 0.0
Ld1 dd 0.0
Lstack dd 0
Lfp_near_clip dd 0.01
Lceilv0 dd 0
Lv dd 0
Lu0 dd 0
Lv0 dd 0
Lzi0 dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_ClipEdge
_R_ClipEdge:
 push esi
 push edi
 push ebx
 mov ds:dword ptr[Lstack],esp
 mov ebx,ds:dword ptr[12+12+esp]
 mov esi,ds:dword ptr[4+12+esp]
 mov edx,ds:dword ptr[8+12+esp]
 test ebx,ebx
 jz Lemit
Lcliploop:
 fld ds:dword ptr[0+0+esi]
 fmul ds:dword ptr[0+0+ebx]
 fld ds:dword ptr[0+4+esi]
 fmul ds:dword ptr[0+4+ebx]
 fld ds:dword ptr[0+8+esi]
 fmul ds:dword ptr[0+8+ebx]
 fxch st(1)
 faddp st(2),st(0)
 fld ds:dword ptr[0+0+edx]
 fmul ds:dword ptr[0+0+ebx]
 fld ds:dword ptr[0+4+edx]
 fmul ds:dword ptr[0+4+ebx]
 fld ds:dword ptr[0+8+edx]
 fmul ds:dword ptr[0+8+ebx]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(3)
 faddp st(2),st(0)
 faddp st(2),st(0)
 fsub ds:dword ptr[12+ebx]
 fxch st(1)
 fsub ds:dword ptr[12+ebx]
 fxch st(1)
 fstp ds:dword ptr[Ld0]
 fstp ds:dword ptr[Ld1]
 mov eax,ds:dword ptr[Ld0]
 mov ecx,ds:dword ptr[Ld1]
 or ecx,eax
 js Lp2
Lcontinue:
 mov ebx,ds:dword ptr[16+ebx]
 test ebx,ebx
 jnz Lcliploop
Lemit:
 fldcw ds:word ptr[ceil_cw]
 cmp ds:dword ptr[_r_lastvertvalid],0
 jz LCalcFirst
 mov eax,ds:dword ptr[_r_lzi1]
 mov ecx,ds:dword ptr[_r_u1]
 mov ds:dword ptr[Lzi0],eax
 mov ds:dword ptr[Lu0],ecx
 mov ecx,ds:dword ptr[_r_v1]
 mov eax,ds:dword ptr[_r_ceilv1]
 mov ds:dword ptr[Lv0],ecx
 mov ds:dword ptr[Lceilv0],eax
 jmp LCalcSecond
LCalcFirst:
 call near ptr LTransformAndProject
 fst ds:dword ptr[Lv0]
 fxch st(2)
 fstp ds:dword ptr[Lu0]
 fstp ds:dword ptr[Lzi0]
 fistp ds:dword ptr[Lceilv0]
LCalcSecond:
 mov esi,edx
 call near ptr LTransformAndProject
 fld ds:dword ptr[Lu0]
 fxch st(3)
 fld ds:dword ptr[Lzi0]
 fxch st(3)
 fld ds:dword ptr[Lv0]
 fxch st(3)
 fist ds:dword ptr[_r_ceilv1]
 fldcw ds:word ptr[single_cw]
 fst ds:dword ptr[_r_v1]
 fxch st(4)
 fcom st(1)
 fnstsw ax
 test ah,1
 jz LP0
 fstp st(0)
 fld st(0)
LP0:
 fxch st(1)
 fstp ds:dword ptr[_r_lzi1]
 fxch st(1)
 fst ds:dword ptr[_r_u1]
 fxch st(1)
 fcom ds:dword ptr[_r_nearzi]
 fnstsw ax
 test ah,045h
 jnz LP1
 fst ds:dword ptr[_r_nearzi]
LP1:
 mov eax,ds:dword ptr[_r_nearzionly]
 test eax,eax
 jz LP2
LPop5AndDone:
 mov eax,ds:dword ptr[_cacheoffset]
 mov edx,ds:dword ptr[_r_framecount]
 cmp eax,07FFFFFFFh
 jz LDoPop
 and edx,07FFFFFFFh
 or edx,080000000h
 mov ds:dword ptr[_cacheoffset],edx
LDoPop:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 jmp Ldone
LP2:
 mov ebx,ds:dword ptr[Lceilv0]
 mov edi,ds:dword ptr[_edge_p]
 mov ecx,ds:dword ptr[_r_ceilv1]
 mov edx,edi
 mov esi,ds:dword ptr[_r_pedge]
 add edx,32
 cmp ebx,ecx
 jz LPop5AndDone
 mov eax,ds:dword ptr[_r_pedge]
 mov ds:dword ptr[28+edi],eax
 fstp ds:dword ptr[24+edi]
 jc LSide0
LSide1:
 fsubp st(3),st(0)
 fsub st(0),st(1)
 fdivp st(2),st(0)
 mov ds:dword ptr[_r_emitted],1
 mov ds:dword ptr[_edge_p],edx
 mov eax,ds:dword ptr[edx]
 mov eax,ecx
 lea ecx,ds:dword ptr[-1+ebx]
 mov ebx,eax
 mov eax,ds:dword ptr[_surface_p]
 mov esi,ds:dword ptr[_surfaces]
 sub edx,edx
 sub eax,esi
 shr eax,6
 mov ds:dword ptr[16+edi],edx
 mov ds:dword ptr[16+2+edi],eax
 sub esi,esi
 mov ds:dword ptr[Lv],ebx
 fild ds:dword ptr[Lv]
 fsubrp st(1),st(0)
 fmul st(0),st(1)
 fadd ds:dword ptr[_r_u1]
 jmp LSideDone
LSide0:
 fsub st(0),st(3)
 fxch st(2)
 fsub st(0),st(1)
 fdivp st(2),st(0)
 mov ds:dword ptr[_r_emitted],1
 mov ds:dword ptr[_edge_p],edx
 mov eax,ds:dword ptr[edx]
 dec ecx
 mov eax,ds:dword ptr[_surface_p]
 mov esi,ds:dword ptr[_surfaces]
 sub edx,edx
 sub eax,esi
 shr eax,6
 mov ds:dword ptr[16+2+edi],edx
 mov ds:dword ptr[16+edi],eax
 mov esi,1
 mov ds:dword ptr[Lv],ebx
 fild ds:dword ptr[Lv]
 fsubrp st(1),st(0)
 fmul st(0),st(1)
 faddp st(2),st(0)
 fxch st(1)
LSideDone:
 fmul ds:dword ptr[fp_1m]
 fxch st(1)
 fmul ds:dword ptr[fp_1m]
 fxch st(1)
 fadd ds:dword ptr[fp_1m_minus_1]
 fxch st(1)
 fistp ds:dword ptr[4+edi]
 fistp ds:dword ptr[0+edi]
 mov eax,ds:dword ptr[0+edi]
 mov edx,ds:dword ptr[_r_refdef+76]
 cmp eax,edx
 jl LP4
 mov edx,ds:dword ptr[_r_refdef+80]
 cmp eax,edx
 jng LP5
LP4:
 mov ds:dword ptr[0+edi],edx
 mov eax,edx
LP5:
 add eax,esi
 mov esi,ds:dword ptr[_newedges+ebx*4]
 test esi,esi
 jz LDoFirst
 cmp ds:dword ptr[0+esi],eax
 jl LNotFirst
LDoFirst:
 mov ds:dword ptr[12+edi],esi
 mov ds:dword ptr[_newedges+ebx*4],edi
 jmp LSetRemove
LNotFirst:
LFindInsertLoop:
 mov edx,esi
 mov esi,ds:dword ptr[12+esi]
 test esi,esi
 jz LInsertFound
 cmp ds:dword ptr[0+esi],eax
 jl LFindInsertLoop
LInsertFound:
 mov ds:dword ptr[12+edi],esi
 mov ds:dword ptr[12+edx],edi
LSetRemove:
 mov eax,ds:dword ptr[_removeedges+ecx*4]
 mov ds:dword ptr[_removeedges+ecx*4],edi
 mov ds:dword ptr[20+edi],eax
Ldone:
 mov esp,ds:dword ptr[Lstack]
 pop ebx
 pop edi
 pop esi
 ret
Lp2:
 test eax,eax
 jns Lp1
 mov eax,ds:dword ptr[Ld1]
 test eax,eax
 jns Lp3
 mov eax,ds:dword ptr[_r_leftclipped]
 mov ecx,ds:dword ptr[_r_pedge]
 test eax,eax
 jnz Ldone
 mov eax,ds:dword ptr[_r_framecount]
 and eax,07FFFFFFFh
 or eax,080000000h
 mov ds:dword ptr[_cacheoffset],eax
 jmp Ldone
Lp1:
 fld ds:dword ptr[Ld0]
 fld ds:dword ptr[Ld1]
 fsubr st(0),st(1)
 mov ds:dword ptr[_cacheoffset],07FFFFFFFh
 fdivp st(1),st(0)
 sub esp,12
 fld ds:dword ptr[0+8+edx]
 fsub ds:dword ptr[0+8+esi]
 fld ds:dword ptr[0+4+edx]
 fsub ds:dword ptr[0+4+esi]
 fld ds:dword ptr[0+0+edx]
 fsub ds:dword ptr[0+0+esi]
 mov edx,esp
 mov eax,ds:dword ptr[20+ebx]
 test al,al
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(2)
 fmulp st(3),st(0)
 fadd ds:dword ptr[0+0+esi]
 fxch st(1)
 fadd ds:dword ptr[0+4+esi]
 fxch st(2)
 fadd ds:dword ptr[0+8+esi]
 fxch st(1)
 fstp ds:dword ptr[0+0+esp]
 fstp ds:dword ptr[0+8+esp]
 fstp ds:dword ptr[0+4+esp]
 jz Ltestright
 mov ds:dword ptr[_r_leftclipped],1
 mov eax,ds:dword ptr[0+0+esp]
 mov ds:dword ptr[_r_leftexit+0+0],eax
 mov eax,ds:dword ptr[0+4+esp]
 mov ds:dword ptr[_r_leftexit+0+4],eax
 mov eax,ds:dword ptr[0+8+esp]
 mov ds:dword ptr[_r_leftexit+0+8],eax
 jmp Lcontinue
Ltestright:
 test ah,ah
 jz Lcontinue
 mov ds:dword ptr[_r_rightclipped],1
 mov eax,ds:dword ptr[0+0+esp]
 mov ds:dword ptr[_r_rightexit+0+0],eax
 mov eax,ds:dword ptr[0+4+esp]
 mov ds:dword ptr[_r_rightexit+0+4],eax
 mov eax,ds:dword ptr[0+8+esp]
 mov ds:dword ptr[_r_rightexit+0+8],eax
 jmp Lcontinue
Lp3:
 mov ds:dword ptr[_r_lastvertvalid],0
 fld ds:dword ptr[Ld0]
 fld ds:dword ptr[Ld1]
 fsubr st(0),st(1)
 mov ds:dword ptr[_cacheoffset],07FFFFFFFh
 fdivp st(1),st(0)
 sub esp,12
 fld ds:dword ptr[0+8+edx]
 fsub ds:dword ptr[0+8+esi]
 fld ds:dword ptr[0+4+edx]
 fsub ds:dword ptr[0+4+esi]
 fld ds:dword ptr[0+0+edx]
 fsub ds:dword ptr[0+0+esi]
 mov eax,ds:dword ptr[20+ebx]
 test al,al
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(2)
 fmulp st(3),st(0)
 fadd ds:dword ptr[0+0+esi]
 fxch st(1)
 fadd ds:dword ptr[0+4+esi]
 fxch st(2)
 fadd ds:dword ptr[0+8+esi]
 fxch st(1)
 fstp ds:dword ptr[0+0+esp]
 fstp ds:dword ptr[0+8+esp]
 fstp ds:dword ptr[0+4+esp]
 mov esi,esp
 jz Ltestright2
 mov ds:dword ptr[_r_leftclipped],1
 mov eax,ds:dword ptr[0+0+esp]
 mov ds:dword ptr[_r_leftenter+0+0],eax
 mov eax,ds:dword ptr[0+4+esp]
 mov ds:dword ptr[_r_leftenter+0+4],eax
 mov eax,ds:dword ptr[0+8+esp]
 mov ds:dword ptr[_r_leftenter+0+8],eax
 jmp Lcontinue
Ltestright2:
 test ah,ah
 jz Lcontinue
 mov ds:dword ptr[_r_rightclipped],1
 mov eax,ds:dword ptr[0+0+esp]
 mov ds:dword ptr[_r_rightenter+0+0],eax
 mov eax,ds:dword ptr[0+4+esp]
 mov ds:dword ptr[_r_rightenter+0+4],eax
 mov eax,ds:dword ptr[0+8+esp]
 mov ds:dword ptr[_r_rightenter+0+8],eax
 jmp Lcontinue
LTransformAndProject:
 fld ds:dword ptr[0+0+esi]
 fsub ds:dword ptr[_modelorg+0]
 fld ds:dword ptr[0+4+esi]
 fsub ds:dword ptr[_modelorg+4]
 fld ds:dword ptr[0+8+esi]
 fsub ds:dword ptr[_modelorg+8]
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[_vpn+0]
 fld st(1)
 fmul ds:dword ptr[_vright+0]
 fxch st(2)
 fmul ds:dword ptr[_vup+0]
 fld st(3)
 fmul ds:dword ptr[_vpn+4]
 fld st(4)
 fmul ds:dword ptr[_vright+4]
 fxch st(5)
 fmul ds:dword ptr[_vup+4]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(4),st(0)
 faddp st(2),st(0)
 fld st(3)
 fmul ds:dword ptr[_vpn+8]
 fld st(4)
 fmul ds:dword ptr[_vright+8]
 fxch st(5)
 fmul ds:dword ptr[_vup+8]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(4)
 faddp st(3),st(0)
 fxch st(1)
 faddp st(3),st(0)
 fcom ds:dword ptr[Lfp_near_clip]
 fnstsw ax
 test ah,1
 jz LNoClip
 fstp st(0)
 fld ds:dword ptr[Lfp_near_clip]
LNoClip:
 fdivr ds:dword ptr[float_1]
 fxch st(1)
 fld ds:dword ptr[_xscale]
 fmul st(0),st(2)
 fmulp st(1),st(0)
 fadd ds:dword ptr[_xcenter]
 fcom ds:dword ptr[_r_refdef+68]
 fnstsw ax
 test ah,1
 jz LClampP0
 fstp st(0)
 fld ds:dword ptr[_r_refdef+68]
LClampP0:
 fcom ds:dword ptr[_r_refdef+84]
 fnstsw ax
 test ah,045h
 jnz LClampP1
 fstp st(0)
 fld ds:dword ptr[_r_refdef+84]
LClampP1:
 fld st(1)
 fmul ds:dword ptr[_yscale]
 fmulp st(3),st(0)
 fxch st(2)
 fsubr ds:dword ptr[_ycenter]
 fcom ds:dword ptr[_r_refdef+72]
 fnstsw ax
 test ah,1
 jz LClampP2
 fstp st(0)
 fld ds:dword ptr[_r_refdef+72]
LClampP2:
 fcom ds:dword ptr[_r_refdef+88]
 fnstsw ax
 test ah,045h
 jnz LClampP3
 fstp st(0)
 fld ds:dword ptr[_r_refdef+88]
LClampP3:
 ret
_TEXT ENDS
 END
