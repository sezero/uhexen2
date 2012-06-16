;
; d_polysa5.asm
; x86 assembly-language polygon model drawing code
; with translucency handling, #5.
; this file uses NASM syntax.
; $Id: d_polysa5.asm,v 1.8 2008-03-20 14:07:51 sezero Exp $
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

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
 _sym_prefix d_viewbuffer
 _sym_prefix d_scantable
 _sym_prefix r_refdef
 _sym_prefix a_sstepxfrac
 _sym_prefix r_affinetridesc
 _sym_prefix acolormap
 _sym_prefix d_pcolormap
 _sym_prefix d_sfrac
 _sym_prefix d_ptex
 _sym_prefix d_pedgespanpackage
 _sym_prefix d_tfrac
 _sym_prefix d_light
 _sym_prefix d_zi
 _sym_prefix d_pdest
 _sym_prefix d_pz
 _sym_prefix d_aspancount
 _sym_prefix erroradjustup
 _sym_prefix erroradjustdown
 _sym_prefix errorterm
 _sym_prefix d_xdenom
 _sym_prefix r_p0
 _sym_prefix r_p1
 _sym_prefix r_p2
 _sym_prefix a_tstepxfrac
 _sym_prefix a_ststepxwhole
 _sym_prefix zspantable
 _sym_prefix skintable
 _sym_prefix d_countextrastep
 _sym_prefix ubasestep
 _sym_prefix a_spans
 _sym_prefix d_pdestextrastep
 _sym_prefix d_pzextrastep
 _sym_prefix d_sfracextrastep
 _sym_prefix d_ptexextrastep
 _sym_prefix d_tfracextrastep
 _sym_prefix d_lightextrastep
 _sym_prefix d_ziextrastep
 _sym_prefix d_pdestbasestep
 _sym_prefix d_pzbasestep
 _sym_prefix d_sfracbasestep
 _sym_prefix d_ptexbasestep
 _sym_prefix d_tfracbasestep
 _sym_prefix d_lightbasestep
 _sym_prefix d_zibasestep
 _sym_prefix r_lstepx
 _sym_prefix r_lstepy
 _sym_prefix r_sstepx
 _sym_prefix r_sstepy
 _sym_prefix r_tstepx
 _sym_prefix r_tstepy
 _sym_prefix r_zistepx
 _sym_prefix r_zistepy
 _sym_prefix transTable
 _sym_prefix D_PolysetSetEdgeTable
 _sym_prefix D_RasterizeAliasPolySmooth
; C-shared globals:
 _sym_prefix D_PolysetAff8StartT5
 _sym_prefix D_PolysetCalcGradientsT5
 _sym_prefix D_PolysetRecursiveTriangleT5
 _sym_prefix D_PolysetDrawSpans8T5
 _sym_prefix D_Aff8PatchT5
 _sym_prefix D_PolysetDrawT5
 _sym_prefix D_PolysetScanLeftEdgeT5
 _sym_prefix D_PolysetDrawFinalVertsT5
 _sym_prefix D_DrawNonSubdivT5
 _sym_prefix D_PolysetAff8EndT5
 _sym_prefix R_TranPatch6
%endif	; _sym_prefix

; externs from C code
 extern d_viewbuffer
 extern d_scantable
 extern r_refdef
 extern a_sstepxfrac
 extern r_affinetridesc
 extern acolormap
 extern d_pcolormap
 extern d_sfrac
 extern d_ptex
 extern d_pedgespanpackage
 extern d_tfrac
 extern d_light
 extern d_zi
 extern d_pdest
 extern d_pz
 extern d_aspancount
 extern erroradjustup
 extern erroradjustdown
 extern errorterm
 extern d_xdenom
 extern r_p0
 extern r_p1
 extern r_p2
 extern a_tstepxfrac
 extern a_ststepxwhole
 extern zspantable
 extern skintable
 extern d_countextrastep
 extern ubasestep
 extern a_spans
 extern d_pdestextrastep
 extern d_pzextrastep
 extern d_sfracextrastep
 extern d_ptexextrastep
 extern d_tfracextrastep
 extern d_lightextrastep
 extern d_ziextrastep
 extern d_pdestbasestep
 extern d_pzbasestep
 extern d_sfracbasestep
 extern d_ptexbasestep
 extern d_tfracbasestep
 extern d_lightbasestep
 extern d_zibasestep
 extern r_lstepx
 extern r_lstepy
 extern r_sstepx
 extern r_sstepy
 extern r_tstepx
 extern r_tstepy
 extern r_zistepx
 extern r_zistepy
 extern transTable
 extern D_PolysetSetEdgeTable
 extern D_RasterizeAliasPolySmooth

; externs from ASM-only code
 extern float_point5
 extern float_1
 extern float_minus_1
 extern float_0
 extern advancetable
 extern sstep
 extern tstep
 extern ceil_cw
 extern single_cw


SEGMENT .data

 ALIGN 4

p10_minus_p20 dd 0
p01_minus_p21 dd 0
temp0 dd 0
temp1 dd 0
Ltemp dd 0
aff8entryvec_table dd LDraw8, LDraw7, LDraw6, LDraw5
 dd LDraw4, LDraw3, LDraw2, LDraw1
lzistepx dd 0

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetAff8StartT5
;;;;;;;;;;;;;;;;;;;;;;;;
 global D_PolysetAff8StartT5
D_PolysetAff8StartT5:

;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetCalcGradientsT5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_PolysetCalcGradientsT5
D_PolysetCalcGradientsT5:
 fild dword [r_p0+0]
 fild dword [r_p2+0]
 fild dword [r_p0+4]
 fild dword [r_p2+4]
 fild dword [r_p1+0]
 fild dword [r_p1+4]
 fxch st3
 fsub st0,st2
 fxch st1
 fsub st0,st4
 fxch st5
 fsubrp st4,st0
 fxch st2
 fsubrp st1,st0
 fxch st1
 fld dword [d_xdenom]
 fxch st4
 fstp dword [p10_minus_p20]
 fstp dword [p01_minus_p21]
 fxch st2
 fild dword [r_p2+16]
 fild dword [r_p0+16]
 fild dword [r_p1+16]
 fxch st2
 fld st0
 fsubp st2,st0
 fsubp st2,st0
 fld st0
 fmul st0,st5
 fxch st2
 fld st0
 fmul dword [p01_minus_p21]
 fxch st2
 fmul dword [p10_minus_p20]
 fxch st1
 fmul st0,st5
 fxch st2
 fsubrp st3,st0
 fsubp st1,st0
 fld st2
 fmul dword [float_minus_1]
 fxch st2
 fmul st0,st3
 fxch st1
 fmul st0,st2
 fldcw word [ceil_cw]
 fistp dword [r_lstepy]
 fistp dword [r_lstepx]
 fldcw word [single_cw]
 fild dword [r_p2+8]
 fild dword [r_p0+8]
 fild dword [r_p1+8]
 fxch st2
 fld st0
 fsubp st2,st0
 fsubp st2,st0
 fld st0
 fmul st0,st6
 fxch st2
 fld st0
 fmul dword [p01_minus_p21]
 fxch st2
 fmul dword [p10_minus_p20]
 fxch st1
 fmul st0,st6
 fxch st2
 fsubrp st3,st0
 fsubp st1,st0
 fmul st0,st2
 fxch st1
 fmul st0,st3
 fxch st1
 fistp dword [r_sstepy]
 fistp dword [r_sstepx]
 fild dword [r_p2+12]
 fild dword [r_p0+12]
 fild dword [r_p1+12]
 fxch st2
 fld st0
 fsubp st2,st0
 fsubp st2,st0
 fld st0
 fmul st0,st6
 fxch st2
 fld st0
 fmul dword [p01_minus_p21]
 fxch st2
 fmul dword [p10_minus_p20]
 fxch st1
 fmul st0,st6
 fxch st2
 fsubrp st3,st0
 fsubp st1,st0
 fmul st0,st2
 fxch st1
 fmul st0,st3
 fxch st1
 fistp dword [r_tstepy]
 fistp dword [r_tstepx]
 fild dword [r_p2+20]
 fild dword [r_p0+20]
 fild dword [r_p1+20]
 fxch st2
 fld st0
 fsubp st2,st0
 fsubp st2,st0
 fld st0
 fmulp st6,st0
 fxch st1
 fld st0
 fmul dword [p01_minus_p21]
 fxch st2
 fmul dword [p10_minus_p20]
 fxch st1
 fmulp st5,st0
 fxch st5
 fsubp st1,st0
 fxch st3
 fsubrp st4,st0
 fxch st1
 fmulp st2,st0
 fmulp st2,st0
 fistp dword [r_zistepx]
 fistp dword [r_zistepy]
 mov eax, dword [r_sstepx]
 mov edx, dword [r_tstepx]
 shl eax,16
 shl edx,16
 mov dword [a_sstepxfrac],eax
 mov dword [a_tstepxfrac],edx
 mov ecx, dword [r_sstepx]
 mov eax, dword [r_tstepx]
 sar ecx,16
 sar eax,16
 imul dword [4+0+esp]
 add eax,ecx
 mov dword [a_ststepxwhole],eax
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetRecursiveTriangleT5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_PolysetRecursiveTriangleT5
D_PolysetRecursiveTriangleT5:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [8+16+esp]
 mov ebx, dword [4+16+esp]
 mov edi, dword [12+16+esp]
 mov eax, dword [0+esi]
 mov edx, dword [0+ebx]
 mov ebp, dword [4+esi]
 sub eax,edx
 mov ecx, dword [4+ebx]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit
 mov eax, dword [0+edi]
 inc ebp
 cmp ebp,2
 ja LSplit
 mov edx, dword [0+esi]
 mov ebp, dword [4+edi]
 sub eax,edx
 mov ecx, dword [4+esi]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit2
 mov eax, dword [0+ebx]
 inc ebp
 cmp ebp,2
 ja LSplit2
 mov edx, dword [0+edi]
 mov ebp, dword [4+ebx]
 sub eax,edx
 mov ecx, dword [4+edi]
 sub ebp,ecx
 inc eax
 inc ebp
 mov edx,ebx
 cmp eax,2
 ja LSplit3
 cmp ebp,2
 jna near LDone
LSplit3:
 mov ebx,edi
 mov edi,esi
 mov esi,edx
 jmp LSplit
LSplit2:
 mov eax,ebx
 mov ebx,esi
 mov esi,edi
 mov edi,eax
LSplit:
 sub esp,24
 mov eax, dword [8+ebx]
 mov edx, dword [8+esi]
 mov ecx, dword [12+ebx]
 add eax,edx
 mov edx, dword [12+esi]
 sar eax,1
 add ecx,edx
 mov dword [8+esp],eax
 mov eax, dword [20+ebx]
 sar ecx,1
 mov edx, dword [20+esi]
 mov dword [12+esp],ecx
 add eax,edx
 mov ecx, dword [0+ebx]
 mov edx, dword [0+esi]
 sar eax,1
 add edx,ecx
 mov dword [20+esp],eax
 mov eax, dword [4+ebx]
 sar edx,1
 mov ebp, dword [4+esi]
 mov dword [0+esp],edx
 add ebp,eax
 sar ebp,1
 mov dword [4+esp],ebp
 cmp dword [4+esi],eax
 jg LNoDraw
 mov edx, dword [0+esi]
 jnz LDraw
 cmp edx,ecx
 jl LNoDraw
LDraw:
 mov edx, dword [20+esp]
 mov ecx, dword [4+esp]
 sar edx,16
 mov ebp, dword [0+esp]
 mov eax, dword [zspantable+ecx*4]
 cmp dx, word [eax+ebp*2]
 jnge LNoDraw
;mov word [eax+ebp*2],dx
 mov eax, dword [12+esp]
 sar eax,16
 mov edx, dword [8+esp]
 sar edx,16
 sub ecx,ecx
 mov eax, dword [skintable+eax*4]
 mov ebp, dword [4+esp]
 mov dh, byte [eax+edx]  ; texture pixel
 or dh,dh
 jz Skip1B  ; color 0 = no draw
;mov edx, dword [d_pcolormap]
;mov dh, byte [edx+ecx]
 mov ecx, dword [0+esp]
 mov eax, dword [d_scantable+ebp*4]
 add ecx,eax
 mov eax, dword [d_viewbuffer]

; trans stuff 
 mov dl, byte [eax+ecx]
 and edx, 0ffffh
 mov dh, byte [12345678h+edx]
TranPatch1:
 mov byte [eax+ecx],dh
;rjr distance
;mov byte [eax+ecx],0

Skip1B:
LNoDraw:
 push esp
 push ebx
 push edi
 call D_PolysetRecursiveTriangleT5	; call near D_PolysetRecursiveTriangleT5
 mov ebx,esp
 push esi
 push ebx
 push edi
 call D_PolysetRecursiveTriangleT5	; call near D_PolysetRecursiveTriangleT5
 add esp,24
LDone:
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret 12


;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetDrawSpans8T5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_PolysetDrawSpans8T5
D_PolysetDrawSpans8T5:
 push esi
 push ebx
 mov esi, dword [4+8+esp]
 mov ecx, dword [r_zistepx]
 push ebp
 push edi
 ror ecx,16
 mov edx, dword [8+esi]
 mov dword [lzistepx],ecx
LSpanLoop:
 mov eax, dword [d_aspancount]
 sub eax,edx
 mov edx, dword [erroradjustup]
 mov ebx, dword [errorterm]
 add ebx,edx
 js LNoTurnover
 mov edx, dword [erroradjustdown]
 mov edi, dword [d_countextrastep]
 sub ebx,edx
 mov ebp, dword [d_aspancount]
 mov dword [errorterm],ebx
 add ebp,edi
 mov dword [d_aspancount],ebp
 jmp LRightEdgeStepped
LNoTurnover:
 mov edi, dword [d_aspancount]
 mov edx, dword [ubasestep]
 mov dword [errorterm],ebx
 add edi,edx
 mov dword [d_aspancount],edi
LRightEdgeStepped:
 cmp eax,1
 jl near LNextSpan
 jz near LExactlyOneLong
 mov ecx, dword [a_ststepxwhole]
 mov edx, dword [r_affinetridesc+8]
 mov dword [advancetable+4],ecx
 add ecx,edx
 mov dword [advancetable],ecx
 mov ecx, dword [a_tstepxfrac]
 mov cx, word [r_lstepx]
 mov edx,eax
 mov dword [tstep],ecx
 add edx,7
 shr edx,3
 mov ebx, dword [16+esi]
 mov bx,dx
 mov ecx, dword [4+esi]
 neg eax
 mov edi, dword [0+esi]
 and eax,7
 sub edi,eax
 sub ecx,eax
 sub ecx,eax
 mov edx, dword [20+esi]
 mov dx, word [24+esi]
 mov ebp, dword [28+esi]
 ror ebp,16
 push esi
 mov esi, dword [12+esi]
 jmp dword[aff8entryvec_table+eax*4]
LDrawLoop:
LDraw8:
 cmp bp, word [ecx]
 jl Lp1
 xor eax,eax
;mov ah,dh  ; light
 mov ah, byte [esi]   ; texture pixel
 or ah,ah
 jz SkipA2  ; color 0 = no draw
;mov word [ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch8:

; trans stuff 
 mov al, byte [edi]
 mov ah, byte [12345678h+eax]
TranPatch2:
 mov byte [edi],ah
;rj
;mov byte [edi],0

SkipA2:
Lp1:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw7:
 cmp bp, word [2+ecx]
 jl Lp2
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipB2  ; color 0 = no draw
;mov word [2+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch7:

; trans stuff 
 mov al, byte [edi+1]
 mov ah, byte [12345678h+eax]
TranPatch3:
 mov byte [1+edi],ah
;rj
;mov byte [1+edi],0

SkipB2:
Lp2:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw6:
 cmp bp, word [4+ecx]
 jl Lp3
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipC2  ; color 0 = no draw
;mov word [4+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch6:

; trans stuff 
 mov al, byte [edi+2]
 mov ah, byte [12345678h+eax]
TranPatch4:
 mov byte [2+edi],ah
;rj
;mov byte [2+edi],0

SkipC2:
Lp3:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw5:
 cmp bp, word [6+ecx]
 jl Lp4
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipD2  ; color 0 = no draw
;mov word [6+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch5:

; trans stuff 
 mov al, byte [edi+3]
 mov ah, byte [12345678h+eax]
TranPatch5:
 mov byte [3+edi],ah
;rj
;mov byte [3+edi],0

SkipD2:
Lp4:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw4:
 cmp bp, word [8+ecx]
 jl Lp5
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipE2  ; color 0 = no draw
;mov word [8+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch4:

; trans stuff 
 mov al, byte [edi+4]
 mov ah, byte [12345678h+eax]
TranPatch6:
 mov byte [4+edi],ah
;rj
;mov byte [4+edi],0

SkipE2:
Lp5:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw3:
 cmp bp, word [10+ecx]
 jl Lp6
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipF2  ; color 0 = no draw
;mov word [10+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch3:

; trans stuff 
 mov al, byte [edi+5]
 mov ah, byte [12345678h+eax]
TranPatch7:
 mov byte [5+edi],ah
;rj
;mov byte [5+edi],0

SkipF2:
Lp6:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw2:
 cmp bp, word [12+ecx]
 jl Lp7
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipG2  ; color 0 = no draw
;mov word [12+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch2:

; trans stuff 
 mov al, byte [edi+6]
 mov ah, byte [12345678h+eax]
TranPatch8:
 mov byte [6+edi],ah
;rj
;mov byte [6+edi],0

SkipG2:
Lp7:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
LDraw1:
 cmp bp, word [14+ecx]
 jl Lp8
 xor eax,eax
;mov ah,dh
 mov ah, byte [esi] ; texture pixel
 or ah,ah
 jz SkipH2  ; color 0 = no draw
;mov word [14+ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch1:

; trans stuff 
 mov al, byte [edi+7]
 mov ah, byte [12345678h+eax]
TranPatch9:
 mov byte [7+edi],ah
;rj
;mov byte [7+edi],0

SkipH2:
Lp8:
 add edx, dword [tstep]
 sbb eax,eax
 add ebp, dword [lzistepx]
 adc ebp,0
 add ebx, dword [a_sstepxfrac]
 adc esi, dword [advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz near LDrawLoop
 pop esi
LNextSpan:
 add esi,32
LNextSpanESISet:
 mov edx, dword [8+esi]
 cmp edx,offset -999999
 jnz near LSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LExactlyOneLong:
 mov ecx, dword [4+esi]
 mov ebp, dword [28+esi]
 ror ebp,16
 mov ebx, dword [12+esi]
 cmp bp, word [ecx]
 jl LNextSpan
 xor eax,eax
 mov edi, dword [0+esi]
;mov ah, byte [24+1+esi]
 add esi,32
 mov ah, byte [ebx] ; texture pixel
 or ah,ah
 jz SkipI2  ; color 0 = no draw
;mov word [ecx],bp
;mov ah, byte [12345678h+eax]
;LPatch9:

; trans stuff 
 mov al, byte [edi]
 mov ah, byte [12345678h+eax]
TranPatch10:
 mov byte [edi],ah
;rjr
;mov byte [edi],0

SkipI2:
 jmp LNextSpanESISet


;;;;;;;;;;;;;;;;;;;;;;;;
; D_Aff8PatchT5
;;;;;;;;;;;;;;;;;;;;;;;;
 global D_Aff8PatchT5
D_Aff8PatchT5:
 ret
;mov eax, dword [4+esp]
;mov dword [LPatch1-4],eax
;mov dword [LPatch2-4],eax
;mov dword [LPatch3-4],eax
;mov dword [LPatch4-4],eax
;mov dword [LPatch5-4],eax
;mov dword [LPatch6-4],eax
;mov dword [LPatch7-4],eax
;mov dword [LPatch8-4],eax
;mov dword [LPatch9-4],eax
 ret

;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetDrawT5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_PolysetDrawT5
D_PolysetDrawT5:
 sub esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 mov eax,esp
 add eax,32 - 1
 and eax,offset ~(32 - 1)
 mov dword [a_spans],eax
 mov eax, dword [r_affinetridesc+28]
 test eax,eax
 jz near D_DrawNonSubdivT5
;push ebp
;mov ebp, dword [r_affinetridesc+24]
 push esi
;shl ebp,4
 push ebx
 mov ebx, dword [r_affinetridesc+16]
 push edi
 mov edi, dword [r_affinetridesc+20]
Llooptop:
 xor ecx,ecx
 xor esi,esi
 xor edx,edx
 mov cx,word[4+0+ebx]
 mov si,word[4+2+ebx]
 mov dx,word[4+4+ebx]
 shl ecx,5
 shl esi,5
 add ecx,edi
 shl edx,5
 add esi,edi
 add edx,edi
 fild dword [0+4+ecx]
 fild dword [0+4+esi]
 fild dword [0+0+ecx]
 fild dword [0+0+edx]
 fxch st2
 fsubr st0,st3
 fild dword [0+0+esi]
 fxch st2
 fsubr st3,st0
 fild dword [0+4+edx]
 fxch st1
 fsubrp st3,st0
 fxch st1
 fmulp st3,st0
 fsubp st3,st0
 mov eax, dword [0+16+ecx]
 and eax,0FF00h
 fmulp st2,st0
 add eax, dword [acolormap]
 fsubrp st1,st0
 mov dword [d_pcolormap],eax
 fstp dword [Ltemp]
 mov eax, dword [Ltemp]
 sub eax,080000001h
 jc Lskip
 mov eax, dword [0+ebx]
 test eax,eax
 jz Lfacesback
 push edx
 push esi
 push ecx
 call D_PolysetRecursiveTriangleT5	; call near D_PolysetRecursiveTriangleT5
;sub ebp,16
;jnz Llooptop
 jmp Ldone2
Lfacesback:
 mov eax, dword [0+8+ecx]
 push eax
 mov eax, dword [0+8+esi]
 push eax
 mov eax, dword [0+8+edx]
 push eax
 push ecx
 push edx
 mov eax, dword [r_affinetridesc+32]
 test dword [24+ecx],00020h
 jz Lp11
 add dword [0+8+ecx],eax
Lp11:
 test dword [24+esi],00020h
 jz Lp12
 add dword [0+8+esi],eax
Lp12:
 test dword [24+edx],00020h
 jz Lp13
 add dword [0+8+edx],eax
Lp13:
 push edx
 push esi
 push ecx
 call D_PolysetRecursiveTriangleT5	; call near D_PolysetRecursiveTriangleT5
 pop edx
 pop ecx
 pop eax
 mov dword [0+8+edx],eax
 pop eax
 mov dword [0+8+esi],eax
 pop eax
 mov dword [0+8+ecx],eax
Lskip:
;sub ebp,16
;jnz Llooptop
Ldone2:
 pop edi
 pop ebx
 pop esi
;pop ebp
 add esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetScanLeftEdgeT5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_PolysetScanLeftEdgeT5
D_PolysetScanLeftEdgeT5:
 push ebp
 push esi
 push edi
 push ebx
 mov eax, dword [4+16+esp]
 mov ecx, dword [d_sfrac]
 and eax,0FFFFh
 mov ebx, dword [d_ptex]
 or ecx,eax
 mov esi, dword [d_pedgespanpackage]
 mov edx, dword [d_tfrac]
 mov edi, dword [d_light]
 mov ebp, dword [d_zi]
LScanLoop:
 mov dword [12+esi],ebx
 mov eax, dword [d_pdest]
 mov dword [0+esi],eax
 mov eax, dword [d_pz]
 mov dword [4+esi],eax
 mov eax, dword [d_aspancount]
 mov dword [8+esi],eax
 mov dword [24+esi],edi
 mov dword [28+esi],ebp
 mov dword [16+esi],ecx
 mov dword [20+esi],edx
 mov al, byte [32+esi]
 add esi,32
 mov eax, dword [erroradjustup]
 mov dword [d_pedgespanpackage],esi
 mov esi, dword [errorterm]
 add esi,eax
 mov eax, dword [d_pdest]
 js near LNoLeftEdgeTurnover
 sub esi, dword [erroradjustdown]
 add eax, dword [d_pdestextrastep]
 mov dword [errorterm],esi
 mov dword [d_pdest],eax
 mov eax, dword [d_pz]
 mov esi, dword [d_aspancount]
 add eax, dword [d_pzextrastep]
 add ecx, dword [d_sfracextrastep]
 adc ebx, dword [d_ptexextrastep]
 add esi, dword [d_countextrastep]
 mov dword [d_pz],eax
 mov eax, dword [d_tfracextrastep]
 mov dword [d_aspancount],esi
 add edx,eax
 jnc LSkip1
 add ebx, dword [r_affinetridesc+8]
LSkip1:
 add edi, dword [d_lightextrastep]
 add ebp, dword [d_ziextrastep]
 mov esi, dword [d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz near LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret
LNoLeftEdgeTurnover:
 mov dword [errorterm],esi
 add eax, dword [d_pdestbasestep]
 mov dword [d_pdest],eax
 mov eax, dword [d_pz]
 mov esi, dword [d_aspancount]
 add eax, dword [d_pzbasestep]
 add ecx, dword [d_sfracbasestep]
 adc ebx, dword [d_ptexbasestep]
 add esi, dword [ubasestep]
 mov dword [d_pz],eax
 mov dword [d_aspancount],esi
 mov esi, dword [d_tfracbasestep]
 add edx,esi
 jnc LSkip2
 add ebx, dword [r_affinetridesc+8]
LSkip2:
 add edi, dword [d_lightbasestep]
 add ebp, dword [d_zibasestep]
 mov esi, dword [d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz near LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetDrawFinalVertsT5
;;;;;;;;;;;;;;;;;;;;;;;;

L_PSDFVertT5:
 push esi
 push edi
 mov eax, dword [0+0+ebx]
 mov edx, dword [r_refdef+40]
 cmp eax,edx
 jge LNextVert
 mov esi, dword [0+4+ebx]
 mov edx, dword [r_refdef+44]
 cmp esi,edx
 jge LNextVert
 mov edi, dword [zspantable+esi*4]
 mov edx, dword [0+20+ebx]
 shr edx,16
 cmp dx, word [edi+eax*2]
 jl LNextVert
;mov word [edi+eax*2],dx
 mov edi, dword [0+12+ebx]
 shr edi,16
 mov edi, dword [skintable+edi*4]
 mov edx, dword [0+8+ebx]
 shr edx,16
 mov dh, byte [edi+edx]   ; texture pixel
 or dh,dh
 jz Skip2B  ; color 0 = no draw
 mov edi, dword [0+16+ebx]
 and edi,0FF00h
;and edx,000FFh
 add edi,edx
;mov edx, dword [acolormap]
;mov dh, byte [edx+edi*1]

 mov edi, dword [d_scantable+esi*4]
 mov esi, dword [d_viewbuffer]
 add edi,eax

; trans stuff 
 mov dl, byte [esi+edi]
 and edx, 0ffffh
 mov dh, byte [12345678h+edx]
TranPatch11:
 mov byte [esi+edi],dh
;rjr distance
;mov byte [esi+edi],0

Skip2B:
LNextVert:
 pop edi
 pop esi
 ret

 global D_PolysetDrawFinalVertsT5
D_PolysetDrawFinalVertsT5:
 push ebp
 push ebx
 mov ebx,dword[4+8+esp]	;pv1
 call L_PSDFVertT5
 mov ebx,dword[8+8+esp]	;pv2
 call L_PSDFVertT5
 mov ebx,dword[12+8+esp];pv3
 call L_PSDFVertT5
 pop ebx
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawNonSubdivT5
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_DrawNonSubdivT5
D_DrawNonSubdivT5:
;push ebp
;mov ebp, dword [r_affinetridesc+24]
 push ebx
;shl ebp,4
 push esi
 mov esi, dword [r_affinetridesc+16]
 push edi
LNDLoop:
 mov edi, dword [r_affinetridesc+20]
 xor ecx,ecx;	//clear i1
 xor edx,edx;	//clear i2
 xor ebx,ebx;	//clear i3
 mov cx, word[4+0+esi] ;ptri->vertindex[0]
 mov dx, word[4+2+esi] ;ptri->vertindex[1]
 mov bx, word[4+4+esi] ;ptri->vertindex[2]
 shl ecx,5
 shl edx,5
 shl ebx,5
 add ecx,edi
 add edx,edi
 add ebx,edi
 mov eax, dword [0+4+ecx]
 mov esi, dword [0+0+ecx]
 sub eax, dword [0+4+edx]
 sub esi, dword [0+0+ebx]
 imul eax,esi
 mov esi, dword [0+0+ecx]
 mov edi, dword [0+4+ecx]
 sub esi, dword [0+0+edx]
 sub edi, dword [0+4+ebx]
 imul edi,esi
 sub eax,edi
 jns near LNextTri
 mov dword [d_xdenom],eax
 fild dword [d_xdenom]
 mov eax, dword [0+0+ecx]
 mov esi, dword [0+4+ecx]
 mov dword [r_p0+0],eax
 mov dword [r_p0+4],esi
 mov eax, dword [0+8+ecx]
 mov esi, dword [0+12+ecx]
 mov dword [r_p0+8],eax
 mov dword [r_p0+12],esi
 mov eax, dword [0+16+ecx]
 mov esi, dword [0+20+ecx]
 mov dword [r_p0+16],eax
 mov dword [r_p0+20],esi
 fdivr dword [float_1]
 mov eax, dword [0+0+edx]
 mov esi, dword [0+4+edx]
 mov dword [r_p1+0],eax
 mov dword [r_p1+4],esi
 mov eax, dword [0+8+edx]
 mov esi, dword [0+12+edx]
 mov dword [r_p1+8],eax
 mov dword [r_p1+12],esi
 mov eax, dword [0+16+edx]
 mov esi, dword [0+20+edx]
 mov dword [r_p1+16],eax
 mov dword [r_p1+20],esi
 mov eax, dword [0+0+ebx]
 mov esi, dword [0+4+ebx]
 mov dword [r_p2+0],eax
 mov dword [r_p2+4],esi
 mov eax, dword [0+8+ebx]
 mov esi, dword [0+12+ebx]
 mov dword [r_p2+8],eax
 mov dword [r_p2+12],esi
 mov eax, dword [0+16+ebx]
 mov esi, dword [0+20+ebx]
 mov dword [r_p2+16],eax
 mov edi, dword [r_affinetridesc+16]
 mov dword [r_p2+20],esi
 mov eax, dword [0+edi]
 test eax,eax
 jnz LFacesFront
 mov eax, dword [24+ecx]
 mov esi, dword [24+edx]
 mov edi, dword [24+ebx]
 test eax,00020h
 mov eax, dword [r_affinetridesc+32]
 jz LOnseamDone0
 add dword [r_p0+8],eax
LOnseamDone0:
 test esi,00020h
 jz LOnseamDone1
 add dword [r_p1+8],eax
LOnseamDone1:
 test edi,00020h
 jz LOnseamDone2
 add dword [r_p2+8],eax
LOnseamDone2:
LFacesFront:
 fstp dword [d_xdenom]
 call D_PolysetSetEdgeTable		; call near D_PolysetSetEdgeTable
 call D_RasterizeAliasPolySmooth	; call near D_RasterizeAliasPolySmooth
LNextTri:
 mov esi, dword [r_affinetridesc+16]
;sub ebp,16
;jnz LNDLoop
 pop edi
 pop esi
 pop ebx
;pop ebp
 add esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 ret

;;;;;;;;;;;;;;;;;;;;;;;;
; D_PolysetAff8EndT5
;;;;;;;;;;;;;;;;;;;;;;;;
 global D_PolysetAff8EndT5
D_PolysetAff8EndT5:

SEGMENT .data

 ALIGN 4

LPatchTable:
 dd TranPatch1-4
 dd TranPatch2-4
 dd TranPatch3-4
 dd TranPatch4-4
 dd TranPatch5-4
 dd TranPatch6-4
 dd TranPatch7-4
 dd TranPatch8-4
 dd TranPatch9-4
 dd TranPatch10-4
 dd TranPatch11-4

SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_TranPatch6
;;;;;;;;;;;;;;;;;;;;;;;;
 global R_TranPatch6
R_TranPatch6:
 push ebx
 mov eax, dword [transTable]
 mov ebx,offset LPatchTable
 mov ecx,11
LPatchLoop:
 mov edx, dword [ebx]
 add ebx,4
 mov dword [edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret

