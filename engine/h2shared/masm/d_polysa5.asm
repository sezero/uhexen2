;
; d_polysa5.asm -- for MASM
; x86 assembly-language polygon model drawing code
; with translucency handling, #5.
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
 externdef _d_viewbuffer:dword
 externdef _d_scantable:dword
 externdef _r_refdef:dword
 externdef _a_sstepxfrac:dword
 externdef _r_affinetridesc:dword
 externdef _acolormap:dword
 externdef _d_pcolormap:dword
 externdef _d_sfrac:dword
 externdef _d_ptex:dword
 externdef _d_pedgespanpackage:dword
 externdef _d_tfrac:dword
 externdef _d_light:dword
 externdef _d_zi:dword
 externdef _d_pdest:dword
 externdef _d_pz:dword
 externdef _d_aspancount:dword
 externdef _erroradjustup:dword
 externdef _erroradjustdown:dword
 externdef _errorterm:dword
 externdef _d_xdenom:dword
 externdef _r_p0:dword
 externdef _r_p1:dword
 externdef _r_p2:dword
 externdef _a_tstepxfrac:dword
 externdef _a_ststepxwhole:dword
 externdef _zspantable:dword
 externdef _skintable:dword
 externdef _d_countextrastep:dword
 externdef _ubasestep:dword
 externdef _a_spans:dword
 externdef _d_pdestextrastep:dword
 externdef _d_pzextrastep:dword
 externdef _d_sfracextrastep:dword
 externdef _d_ptexextrastep:dword
 externdef _d_tfracextrastep:dword
 externdef _d_lightextrastep:dword
 externdef _d_ziextrastep:dword
 externdef _d_pdestbasestep:dword
 externdef _d_pzbasestep:dword
 externdef _d_sfracbasestep:dword
 externdef _d_ptexbasestep:dword
 externdef _d_tfracbasestep:dword
 externdef _d_lightbasestep:dword
 externdef _d_zibasestep:dword
 externdef _r_lstepx:dword
 externdef _r_lstepy:dword
 externdef _r_sstepx:dword
 externdef _r_sstepy:dword
 externdef _r_tstepx:dword
 externdef _r_tstepy:dword
 externdef _r_zistepx:dword
 externdef _r_zistepy:dword
 externdef _transTable:dword
 externdef _D_PolysetSetEdgeTable:dword
 externdef _D_RasterizeAliasPolySmooth:dword

; externs from ASM-only code
 externdef float_point5:dword
 externdef float_1:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef advancetable:dword
 externdef sstep:dword
 externdef tstep:dword
 externdef ceil_cw:dword
 externdef single_cw:dword

_DATA SEGMENT
 align 4
p10_minus_p20 dd 0
p01_minus_p21 dd 0
temp0 dd 0
temp1 dd 0
Ltemp dd 0
aff8entryvec_table dd LDraw8, LDraw7, LDraw6, LDraw5
 dd LDraw4, LDraw3, LDraw2, LDraw1
lzistepx dd 0
_DATA ENDS
_TEXT SEGMENT
 externdef _D_PolysetSetEdgeTable:dword
 externdef _D_RasterizeAliasPolySmooth:dword
 public _D_PolysetAff8StartT5
_D_PolysetAff8StartT5:
 public _D_PolysetCalcGradientsT5
_D_PolysetCalcGradientsT5:
 fild dword ptr[_r_p0+0]
 fild dword ptr[_r_p2+0]
 fild dword ptr[_r_p0+4]
 fild dword ptr[_r_p2+4]
 fild dword ptr[_r_p1+0]
 fild dword ptr[_r_p1+4]
 fxch st(3)
 fsub st(0),st(2)
 fxch st(1)
 fsub st(0),st(4)
 fxch st(5)
 fsubrp st(4),st(0)
 fxch st(2)
 fsubrp st(1),st(0)
 fxch st(1)
 fld dword ptr[_d_xdenom]
 fxch st(4)
 fstp dword ptr[p10_minus_p20]
 fstp dword ptr[p01_minus_p21]
 fxch st(2)
 fild dword ptr[_r_p2+16]
 fild dword ptr[_r_p0+16]
 fild dword ptr[_r_p1+16]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(5)
 fxch st(2)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fxch st(2)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(5)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fld st(2)
 fmul dword ptr[float_minus_1]
 fxch st(2)
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(2)
 fldcw word ptr[ceil_cw]
 fistp dword ptr[_r_lstepy]
 fistp dword ptr[_r_lstepx]
 fldcw word ptr[single_cw]
 fild dword ptr[_r_p2+8]
 fild dword ptr[_r_p0+8]
 fild dword ptr[_r_p1+8]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(6)
 fxch st(2)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fxch st(2)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(6)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp dword ptr[_r_sstepy]
 fistp dword ptr[_r_sstepx]
 fild dword ptr[_r_p2+12]
 fild dword ptr[_r_p0+12]
 fild dword ptr[_r_p1+12]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(6)
 fxch st(2)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fxch st(2)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(6)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp dword ptr[_r_tstepy]
 fistp dword ptr[_r_tstepx]
 fild dword ptr[_r_p2+20]
 fild dword ptr[_r_p0+20]
 fild dword ptr[_r_p1+20]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmulp st(6),st(0)
 fxch st(1)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fxch st(2)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fmulp st(5),st(0)
 fxch st(5)
 fsubp st(1),st(0)
 fxch st(3)
 fsubrp st(4),st(0)
 fxch st(1)
 fmulp st(2),st(0)
 fmulp st(2),st(0)
 fistp dword ptr[_r_zistepx]
 fistp dword ptr[_r_zistepy]
 mov eax,dword ptr[_r_sstepx]
 mov edx,dword ptr[_r_tstepx]
 shl eax,16
 shl edx,16
 mov dword ptr[_a_sstepxfrac],eax
 mov dword ptr[_a_tstepxfrac],edx
 mov ecx,dword ptr[_r_sstepx]
 mov eax,dword ptr[_r_tstepx]
 sar ecx,16
 sar eax,16
 imul dword ptr[4+0+esp]
 add eax,ecx
 mov dword ptr[_a_ststepxwhole],eax
 ret
 public _D_PolysetRecursiveTriangleT5
_D_PolysetRecursiveTriangleT5:
 push ebp
 push esi
 push edi
 push ebx
 mov esi,dword ptr[8+16+esp]
 mov ebx,dword ptr[4+16+esp]
 mov edi,dword ptr[12+16+esp]
 mov eax,dword ptr[0+esi]
 mov edx,dword ptr[0+ebx]
 mov ebp,dword ptr[4+esi]
 sub eax,edx
 mov ecx,dword ptr[4+ebx]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit
 mov eax,dword ptr[0+edi]
 inc ebp
 cmp ebp,2
 ja LSplit
 mov edx,dword ptr[0+esi]
 mov ebp,dword ptr[4+edi]
 sub eax,edx
 mov ecx,dword ptr[4+esi]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit2
 mov eax,dword ptr[0+ebx]
 inc ebp
 cmp ebp,2
 ja LSplit2
 mov edx,dword ptr[0+edi]
 mov ebp,dword ptr[4+ebx]
 sub eax,edx
 mov ecx,dword ptr[4+edi]
 sub ebp,ecx
 inc eax
 inc ebp
 mov edx,ebx
 cmp eax,2
 ja LSplit3
 cmp ebp,2
 jna LDone
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
 mov eax,dword ptr[8+ebx]
 mov edx,dword ptr[8+esi]
 mov ecx,dword ptr[12+ebx]
 add eax,edx
 mov edx,dword ptr[12+esi]
 sar eax,1
 add ecx,edx
 mov dword ptr[8+esp],eax
 mov eax,dword ptr[20+ebx]
 sar ecx,1
 mov edx,dword ptr[20+esi]
 mov dword ptr[12+esp],ecx
 add eax,edx
 mov ecx,dword ptr[0+ebx]
 mov edx,dword ptr[0+esi]
 sar eax,1
 add edx,ecx
 mov dword ptr[20+esp],eax
 mov eax,dword ptr[4+ebx]
 sar edx,1
 mov ebp,dword ptr[4+esi]
 mov dword ptr[0+esp],edx
 add ebp,eax
 sar ebp,1
 mov dword ptr[4+esp],ebp
 cmp dword ptr[4+esi],eax
 jg LNoDraw
 mov edx,dword ptr[0+esi]
 jnz LDraw
 cmp edx,ecx
 jl LNoDraw
LDraw:
 mov edx,dword ptr[20+esp]
 mov ecx,dword ptr[4+esp]
 sar edx,16
 mov ebp,dword ptr[0+esp]
 mov eax,dword ptr[_zspantable+ecx*4]
 cmp dx,word ptr[eax+ebp*2]
 jnge LNoDraw
 ;mov word ptr[eax+ebp*2],dx
 mov eax,dword ptr[12+esp]
 sar eax,16
 mov edx,dword ptr[8+esp]
 sar edx,16
 sub ecx,ecx
 mov eax,dword ptr[_skintable+eax*4]
 mov ebp,dword ptr[4+esp]
 mov dh,byte ptr[eax+edx]  ; texture pixel
 or dh,dh
 jz Skip1B  ; color 0 = no draw
 ;mov edx,dword ptr[_d_pcolormap]
 ;mov dh,byte ptr[edx+ecx]
 mov ecx,dword ptr[0+esp]
 mov eax,dword ptr[_d_scantable+ebp*4]
 add ecx,eax
 mov eax,dword ptr[_d_viewbuffer]


; trans stuff 
 mov dl,byte ptr[eax+ecx]
 and edx, 0ffffh
 mov dh,byte ptr[12345678h + edx]
TranPatch1:

 mov byte ptr[eax+ecx],dh
; rjr   distance
;  mov byte ptr[eax+ecx],0


Skip1B:
LNoDraw:
 push esp
 push ebx
 push edi
 call near ptr _D_PolysetRecursiveTriangleT5
 mov ebx,esp
 push esi
 push ebx
 push edi
 call near ptr _D_PolysetRecursiveTriangleT5
 add esp,24
LDone:
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret 12
 public _D_PolysetDrawSpans8T5
_D_PolysetDrawSpans8T5:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 mov edx,dword ptr[8+esi]
 mov dword ptr[lzistepx],ecx
LSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LRightEdgeStepped
LNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LRightEdgeStepped:
 cmp eax,1
 jl LNextSpan
 jz LExactlyOneLong
 mov ecx,dword ptr[_a_ststepxwhole]
 mov edx,dword ptr[_r_affinetridesc+8]
 mov dword ptr[advancetable+4],ecx
 add ecx,edx
 mov dword ptr[advancetable],ecx
 mov ecx,dword ptr[_a_tstepxfrac]
 mov cx,word ptr[_r_lstepx]
 mov edx,eax
 mov dword ptr[tstep],ecx
 add edx,7
 shr edx,3
 mov ebx,dword ptr[16+esi]
 mov bx,dx
 mov ecx,dword ptr[4+esi]
 neg eax
 mov edi,dword ptr[0+esi]
 and eax,7
 sub edi,eax
 sub ecx,eax
 sub ecx,eax
 mov edx,dword ptr[20+esi]
 mov dx,word ptr[24+esi]
 mov ebp,dword ptr[28+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[12+esi]
 jmp dword ptr[aff8entryvec_table+eax*4]
LDrawLoop:
LDraw8:
 cmp bp,word ptr[ecx]
 jl Lp1
 xor eax,eax
; mov ah,dh  ; light
 mov ah,byte ptr[esi]   ; texture pixel
 or ah,ah
 jz SkipA2  ; color 0 = no draw
; mov word ptr[ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch8:

; trans stuff 
 mov al,byte ptr[edi]
 mov ah,byte ptr[12345678h + eax]
TranPatch2:

 mov byte ptr[edi],ah
 ; rj   
 ;mov byte ptr[edi],0


SkipA2:
Lp1:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw7:
 cmp bp,word ptr[2+ecx]
 jl Lp2
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipB2  ; color 0 = no draw
; mov word ptr[2+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch7:

; trans stuff 
 mov al,byte ptr[edi+1]
 mov ah,byte ptr[12345678h + eax]
TranPatch3:

 mov byte ptr[1+edi],ah
 ; rj
 ;mov byte ptr[1+edi],0


SkipB2:
Lp2:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw6:
 cmp bp,word ptr[4+ecx]
 jl Lp3
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipC2  ; color 0 = no draw
; mov word ptr[4+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch6:

; trans stuff 
 mov al,byte ptr[edi+2]
 mov ah,byte ptr[12345678h + eax]
TranPatch4:

 mov byte ptr[2+edi],ah
 ; rj
 ;mov byte ptr[2+edi],0

SkipC2:
Lp3:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw5:
 cmp bp,word ptr[6+ecx]
 jl Lp4
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipD2  ; color 0 = no draw
; mov word ptr[6+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch5:

; trans stuff 
 mov al,byte ptr[edi+3]
 mov ah,byte ptr[12345678h + eax]
TranPatch5:

 mov byte ptr[3+edi],ah
 ; rj
 ;mov byte ptr[3+edi],0

SkipD2:
Lp4:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw4:
 cmp bp,word ptr[8+ecx]
 jl Lp5
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipE2  ; color 0 = no draw
; mov word ptr[8+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch4:

; trans stuff 
 mov al,byte ptr[edi+4]
 mov ah,byte ptr[12345678h + eax]
TranPatch6:

 mov byte ptr[4+edi],ah
 ; rj
 ;mov byte ptr[4+edi],0

SkipE2:
Lp5:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw3:
 cmp bp,word ptr[10+ecx]
 jl Lp6
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipF2  ; color 0 = no draw
; mov word ptr[10+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch3:

; trans stuff 
 mov al,byte ptr[edi+5]
 mov ah,byte ptr[12345678h + eax]
TranPatch7:

 mov byte ptr[5+edi],ah
 ; rj
 ;mov byte ptr[5+edi],0

SkipF2:
Lp6:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw2:
 cmp bp,word ptr[12+ecx]
 jl Lp7
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipG2  ; color 0 = no draw
; mov word ptr[12+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch2:

; trans stuff 
 mov al,byte ptr[edi+6]
 mov ah,byte ptr[12345678h + eax]
TranPatch8:

 mov byte ptr[6+edi],ah
 ; rj
 ;mov byte ptr[6+edi],0

SkipG2:
Lp7:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw1:
 cmp bp,word ptr[14+ecx]
 jl Lp8
 xor eax,eax
; mov ah,dh
 mov ah,byte ptr[esi] ; texture pixel
 or ah,ah
 jz SkipH2  ; color 0 = no draw
; mov word ptr[14+ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch1:

; trans stuff 
 mov al,byte ptr[edi+7]
 mov ah,byte ptr[12345678h + eax]
TranPatch9:

 mov byte ptr[7+edi],ah
 ; rj
 ;mov byte ptr[7+edi],0

SkipH2:
Lp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LDrawLoop
 pop esi
LNextSpan:
 add esi,32
LNextSpanESISet:
 mov edx,dword ptr[8+esi]
 cmp edx,offset -999999
 jnz LSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[28+esi]
 ror ebp,16
 mov ebx,dword ptr[12+esi]
 cmp bp,word ptr[ecx]
 jl LNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
; mov ah,byte ptr[24+1+esi]
 add esi,32
 mov ah,byte ptr[ebx] ; texture pixel
 or ah,ah
 jz SkipI2  ; color 0 = no draw
; mov word ptr[ecx],bp
; mov ah,byte ptr[12345678h+eax]
;LPatch9:

; trans stuff 
 mov al,byte ptr[edi]
 mov ah,byte ptr[12345678h + eax]
TranPatch10:

 mov byte ptr[edi],ah
 ; rjr
  ;mov byte ptr[edi],0

SkipI2:
 jmp LNextSpanESISet
 public _D_Aff8PatchT5
_D_Aff8PatchT5:
 ret
; mov eax,dword ptr[4+esp]
; mov dword ptr[LPatch1-4],eax
; mov dword ptr[LPatch2-4],eax
; mov dword ptr[LPatch3-4],eax
; mov dword ptr[LPatch4-4],eax
; mov dword ptr[LPatch5-4],eax
; mov dword ptr[LPatch6-4],eax
; mov dword ptr[LPatch7-4],eax
; mov dword ptr[LPatch8-4],eax
; mov dword ptr[LPatch9-4],eax
 ret
 public _D_PolysetDrawT5
_D_PolysetDrawT5:
 sub esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 mov eax,esp
 add eax,32 - 1
 and eax,offset not (32 - 1)
 mov dword ptr[_a_spans],eax
 mov eax,dword ptr[_r_affinetridesc+28]
 test eax,eax
 jz _D_DrawNonSubdivT5
; push ebp
; mov ebp,dword ptr[_r_affinetridesc+24]
 push esi
; shl ebp,4
 push ebx
 mov ebx,dword ptr[_r_affinetridesc+16]
 push edi
 mov edi,dword ptr[_r_affinetridesc+20]
Llooptop:
 xor ecx,ecx
 xor esi,esi
 xor edx,edx
 mov cx,word ptr[4+0+ebx]
 mov si,word ptr[4+2+ebx]
 mov dx,word ptr[4+4+ebx]
 shl ecx,5
 shl esi,5
 add ecx,edi
 shl edx,5
 add esi,edi
 add edx,edi
 fild dword ptr[0+4+ecx]
 fild dword ptr[0+4+esi]
 fild dword ptr[0+0+ecx]
 fild dword ptr[0+0+edx]
 fxch st(2)
 fsubr st(0),st(3)
 fild dword ptr[0+0+esi]
 fxch st(2)
 fsubr st(3),st(0)
 fild dword ptr[0+4+edx]
 fxch st(1)
 fsubrp st(3),st(0)
 fxch st(1)
 fmulp st(3),st(0)
 fsubp st(3),st(0)
 mov eax,dword ptr[0+16+ecx]
 and eax,0FF00h
 fmulp st(2),st(0)
 add eax,dword ptr[_acolormap]
 fsubrp st(1),st(0)
 mov dword ptr[_d_pcolormap],eax
 fstp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 sub eax,080000001h
 jc Lskip
 mov eax,dword ptr[0+ebx]
 test eax,eax
 jz Lfacesback
 push edx
 push esi
 push ecx
 call near ptr _D_PolysetRecursiveTriangleT5
;sub ebp,16
;jnz Llooptop
 jmp Ldone2
Lfacesback:
 mov eax,dword ptr[0+8+ecx]
 push eax
 mov eax,dword ptr[0+8+esi]
 push eax
 mov eax,dword ptr[0+8+edx]
 push eax
 push ecx
 push edx
 mov eax,dword ptr[_r_affinetridesc+32]
 test dword ptr[24+ecx],00020h
 jz Lp11
 add dword ptr[0+8+ecx],eax
Lp11:
 test dword ptr[24+esi],00020h
 jz Lp12
 add dword ptr[0+8+esi],eax
Lp12:
 test dword ptr[24+edx],00020h
 jz Lp13
 add dword ptr[0+8+edx],eax
Lp13:
 push edx
 push esi
 push ecx
 call near ptr _D_PolysetRecursiveTriangleT5
 pop edx
 pop ecx
 pop eax
 mov dword ptr[0+8+edx],eax
 pop eax
 mov dword ptr[0+8+esi],eax
 pop eax
 mov dword ptr[0+8+ecx],eax
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

 public _D_PolysetScanLeftEdgeT5
_D_PolysetScanLeftEdgeT5:
 push ebp
 push esi
 push edi
 push ebx
 mov eax,dword ptr[4+16+esp]
 mov ecx,dword ptr[_d_sfrac]
 and eax,0FFFFh
 mov ebx,dword ptr[_d_ptex]
 or ecx,eax
 mov esi,dword ptr[_d_pedgespanpackage]
 mov edx,dword ptr[_d_tfrac]
 mov edi,dword ptr[_d_light]
 mov ebp,dword ptr[_d_zi]
LScanLoop:
 mov dword ptr[12+esi],ebx
 mov eax,dword ptr[_d_pdest]
 mov dword ptr[0+esi],eax
 mov eax,dword ptr[_d_pz]
 mov dword ptr[4+esi],eax
 mov eax,dword ptr[_d_aspancount]
 mov dword ptr[8+esi],eax
 mov dword ptr[24+esi],edi
 mov dword ptr[28+esi],ebp
 mov dword ptr[16+esi],ecx
 mov dword ptr[20+esi],edx
 mov al,byte ptr[32+esi]
 add esi,32
 mov eax,dword ptr[_erroradjustup]
 mov dword ptr[_d_pedgespanpackage],esi
 mov esi,dword ptr[_errorterm]
 add esi,eax
 mov eax,dword ptr[_d_pdest]
 js LNoLeftEdgeTurnover
 sub esi,dword ptr[_erroradjustdown]
 add eax,dword ptr[_d_pdestextrastep]
 mov dword ptr[_errorterm],esi
 mov dword ptr[_d_pdest],eax
 mov eax,dword ptr[_d_pz]
 mov esi,dword ptr[_d_aspancount]
 add eax,dword ptr[_d_pzextrastep]
 add ecx,dword ptr[_d_sfracextrastep]
 adc ebx,dword ptr[_d_ptexextrastep]
 add esi,dword ptr[_d_countextrastep]
 mov dword ptr[_d_pz],eax
 mov eax,dword ptr[_d_tfracextrastep]
 mov dword ptr[_d_aspancount],esi
 add edx,eax
 jnc LSkip1
 add ebx,dword ptr[_r_affinetridesc+8]
LSkip1:
 add edi,dword ptr[_d_lightextrastep]
 add ebp,dword ptr[_d_ziextrastep]
 mov esi,dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret
LNoLeftEdgeTurnover:
 mov dword ptr[_errorterm],esi
 add eax,dword ptr[_d_pdestbasestep]
 mov dword ptr[_d_pdest],eax
 mov eax,dword ptr[_d_pz]
 mov esi,dword ptr[_d_aspancount]
 add eax,dword ptr[_d_pzbasestep]
 add ecx,dword ptr[_d_sfracbasestep]
 adc ebx,dword ptr[_d_ptexbasestep]
 add esi,dword ptr[_ubasestep]
 mov dword ptr[_d_pz],eax
 mov dword ptr[_d_aspancount],esi
 mov esi,dword ptr[_d_tfracbasestep]
 add edx,esi
 jnc LSkip2
 add ebx,dword ptr[_r_affinetridesc+8]
LSkip2:
 add edi,dword ptr[_d_lightbasestep]
 add ebp,dword ptr[_d_zibasestep]
 mov esi,dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

 _L_PSDFVertT5  PROC NEAR
 push esi
 push edi
 mov eax,dword ptr[0+0+ebx]
 mov edx,dword ptr[_r_refdef+40]
 cmp eax,edx
 jge LNextVert
 mov esi,dword ptr[0+4+ebx]
 mov edx,dword ptr[_r_refdef+44]
 cmp esi,edx
 jge LNextVert
 mov edi,dword ptr[_zspantable+esi*4]
 mov edx,dword ptr[0+20+ebx]
 shr edx,16
 cmp dx,word ptr[edi+eax*2]
 jl LNextVert
 ;mov word ptr[edi+eax*2],dx
 mov edi,dword ptr[0+12+ebx]
 shr edi,16
 mov edi,dword ptr[_skintable+edi*4]
 mov edx,dword ptr[0+8+ebx]
 shr edx,16
 mov dh,byte ptr[edi+edx]   ; texture pixel
 or dh,dh
 jz Skip2B  ; color 0 = no draw
 mov edi,dword ptr[0+16+ebx]
 and edi,0FF00h
 ;and edx,000FFh
 add edi,edx
 ;mov edx,dword ptr[_acolormap]
 ;mov dh,byte ptr[edx+edi*1]

 mov edi,dword ptr[_d_scantable+esi*4]
 mov esi,dword ptr[_d_viewbuffer]
 add edi,eax
; trans stuff 
 mov dl,byte ptr[esi+edi]
 and edx, 0ffffh
 mov dh,byte ptr[12345678h + edx]
TranPatch11:
 mov byte ptr[esi+edi],dh
 ; rjr   distance
 ;mov byte ptr[esi+edi],0
Skip2B:
LNextVert:
 pop edi
 pop esi
 ret
 _L_PSDFVertT5 ENDP

 public _D_PolysetDrawFinalVertsT5
_D_PolysetDrawFinalVertsT5:
 push ebp
 push ebx
 mov ebx,dword ptr[4+8+esp]	;pv1
 call _L_PSDFVertT5
 mov ebx,dword ptr[8+8+esp]	;pv2
 call _L_PSDFVertT5
 mov ebx,dword ptr[12+8+esp];pv3
 call _L_PSDFVertT5
 pop ebx
 pop ebp
 ret

 public _D_DrawNonSubdivT5
_D_DrawNonSubdivT5:
; push ebp
; mov ebp,dword ptr[_r_affinetridesc+24]
 push ebx
; shl ebp,4
 push esi
 mov esi,dword ptr[_r_affinetridesc+16]
 push edi
LNDLoop:
 mov edi,dword ptr[_r_affinetridesc+20]
 xor ecx,ecx;	//clear i1
 xor edx,edx;	//clear i2
 xor ebx,ebx;	//clear i3
 mov cx, word ptr[4+0+esi] ;ptri->vertindex[0]
 mov dx, word ptr[4+2+esi] ;ptri->vertindex[1]
 mov bx, word ptr[4+4+esi] ;ptri->vertindex[2]
 shl ecx,5
 shl edx,5
 shl ebx,5
 add ecx,edi
 add edx,edi
 add ebx,edi
 mov eax,dword ptr[0+4+ecx]
 mov esi,dword ptr[0+0+ecx]
 sub eax,dword ptr[0+4+edx]
 sub esi,dword ptr[0+0+ebx]
 imul eax,esi
 mov esi,dword ptr[0+0+ecx]
 mov edi,dword ptr[0+4+ecx]
 sub esi,dword ptr[0+0+edx]
 sub edi,dword ptr[0+4+ebx]
 imul edi,esi
 sub eax,edi
 jns LNextTri
 mov dword ptr[_d_xdenom],eax
 fild dword ptr[_d_xdenom]
 mov eax,dword ptr[0+0+ecx]
 mov esi,dword ptr[0+4+ecx]
 mov dword ptr[_r_p0+0],eax
 mov dword ptr[_r_p0+4],esi
 mov eax,dword ptr[0+8+ecx]
 mov esi,dword ptr[0+12+ecx]
 mov dword ptr[_r_p0+8],eax
 mov dword ptr[_r_p0+12],esi
 mov eax,dword ptr[0+16+ecx]
 mov esi,dword ptr[0+20+ecx]
 mov dword ptr[_r_p0+16],eax
 mov dword ptr[_r_p0+20],esi
 fdivr dword ptr[float_1]
 mov eax,dword ptr[0+0+edx]
 mov esi,dword ptr[0+4+edx]
 mov dword ptr[_r_p1+0],eax
 mov dword ptr[_r_p1+4],esi
 mov eax,dword ptr[0+8+edx]
 mov esi,dword ptr[0+12+edx]
 mov dword ptr[_r_p1+8],eax
 mov dword ptr[_r_p1+12],esi
 mov eax,dword ptr[0+16+edx]
 mov esi,dword ptr[0+20+edx]
 mov dword ptr[_r_p1+16],eax
 mov dword ptr[_r_p1+20],esi
 mov eax,dword ptr[0+0+ebx]
 mov esi,dword ptr[0+4+ebx]
 mov dword ptr[_r_p2+0],eax
 mov dword ptr[_r_p2+4],esi
 mov eax,dword ptr[0+8+ebx]
 mov esi,dword ptr[0+12+ebx]
 mov dword ptr[_r_p2+8],eax
 mov dword ptr[_r_p2+12],esi
 mov eax,dword ptr[0+16+ebx]
 mov esi,dword ptr[0+20+ebx]
 mov dword ptr[_r_p2+16],eax
 mov edi,dword ptr[_r_affinetridesc+16]
 mov dword ptr[_r_p2+20],esi
 mov eax,dword ptr[0+edi]
 test eax,eax
 jnz LFacesFront
 mov eax,dword ptr[24+ecx]
 mov esi,dword ptr[24+edx]
 mov edi,dword ptr[24+ebx]
 test eax,00020h
 mov eax,dword ptr[_r_affinetridesc+32]
 jz LOnseamDone0
 add dword ptr[_r_p0+8],eax
LOnseamDone0:
 test esi,00020h
 jz LOnseamDone1
 add dword ptr[_r_p1+8],eax
LOnseamDone1:
 test edi,00020h
 jz LOnseamDone2
 add dword ptr[_r_p2+8],eax
LOnseamDone2:
LFacesFront:
 fstp dword ptr[_d_xdenom]
 call near ptr _D_PolysetSetEdgeTable
 call near ptr _D_RasterizeAliasPolySmooth
LNextTri:
 mov esi,dword ptr[_r_affinetridesc+16]
; sub ebp,16
; jnz LNDLoop
 pop edi
 pop esi
 pop ebx
; pop ebp
 add esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 ret

 public _D_PolysetAff8EndT5
_D_PolysetAff8EndT5:
_TEXT ENDS
_DATA SEGMENT
 align 4
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
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_TranPatch6
_R_TranPatch6:
 push ebx
 mov eax,dword ptr[_transTable]
 mov ebx,offset LPatchTable
 mov ecx,11
LPatchLoop:
 mov edx,dword ptr[ebx]
 add ebx,4
 mov dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret
_TEXT ENDS
 END
