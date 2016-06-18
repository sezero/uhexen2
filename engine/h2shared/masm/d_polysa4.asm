;
; d_polysa4.asm -- for MASM
; x86 assembly-language polygon model drawing code
; with translucency handling, #3.
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
 public _D_PolysetCalcGradientsT3
_D_PolysetCalcGradientsT3:
 fild ds:dword ptr[_r_p0+0]
 fild ds:dword ptr[_r_p2+0]
 fild ds:dword ptr[_r_p0+4]
 fild ds:dword ptr[_r_p2+4]
 fild ds:dword ptr[_r_p1+0]
 fild ds:dword ptr[_r_p1+4]
 fxch st(3)
 fsub st(0),st(2)
 fxch st(1)
 fsub st(0),st(4)
 fxch st(5)
 fsubrp st(4),st(0)
 fxch st(2)
 fsubrp st(1),st(0)
 fxch st(1)
 fld ds:dword ptr[_d_xdenom]
 fxch st(4)
 fstp ds:dword ptr[p10_minus_p20]
 fstp ds:dword ptr[p01_minus_p21]
 fxch st(2)
 fild ds:dword ptr[_r_p2+16]
 fild ds:dword ptr[_r_p0+16]
 fild ds:dword ptr[_r_p1+16]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(5)
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[p01_minus_p21]
 fxch st(2)
 fmul ds:dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(5)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fld st(2)
 fmul ds:dword ptr[float_minus_1]
 fxch st(2)
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(2)
 fldcw ds:word ptr[ceil_cw]
 fistp ds:dword ptr[_r_lstepy]
 fistp ds:dword ptr[_r_lstepx]
 fldcw ds:word ptr[single_cw]
 fild ds:dword ptr[_r_p2+8]
 fild ds:dword ptr[_r_p0+8]
 fild ds:dword ptr[_r_p1+8]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(6)
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[p01_minus_p21]
 fxch st(2)
 fmul ds:dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(6)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[_r_sstepy]
 fistp ds:dword ptr[_r_sstepx]
 fild ds:dword ptr[_r_p2+12]
 fild ds:dword ptr[_r_p0+12]
 fild ds:dword ptr[_r_p1+12]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul st(0),st(6)
 fxch st(2)
 fld st(0)
 fmul ds:dword ptr[p01_minus_p21]
 fxch st(2)
 fmul ds:dword ptr[p10_minus_p20]
 fxch st(1)
 fmul st(0),st(6)
 fxch st(2)
 fsubrp st(3),st(0)
 fsubp st(1),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[_r_tstepy]
 fistp ds:dword ptr[_r_tstepx]
 fild ds:dword ptr[_r_p2+20]
 fild ds:dword ptr[_r_p0+20]
 fild ds:dword ptr[_r_p1+20]
 fxch st(2)
 fld st(0)
 fsubp st(2),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmulp st(6),st(0)
 fxch st(1)
 fld st(0)
 fmul ds:dword ptr[p01_minus_p21]
 fxch st(2)
 fmul ds:dword ptr[p10_minus_p20]
 fxch st(1)
 fmulp st(5),st(0)
 fxch st(5)
 fsubp st(1),st(0)
 fxch st(3)
 fsubrp st(4),st(0)
 fxch st(1)
 fmulp st(2),st(0)
 fmulp st(2),st(0)
 fistp ds:dword ptr[_r_zistepx]
 fistp ds:dword ptr[_r_zistepy]
 mov eax,ds:dword ptr[_r_sstepx]
 mov edx,ds:dword ptr[_r_tstepx]
 shl eax,16
 shl edx,16
 mov ds:dword ptr[_a_sstepxfrac],eax
 mov ds:dword ptr[_a_tstepxfrac],edx
 mov ecx,ds:dword ptr[_r_sstepx]
 mov eax,ds:dword ptr[_r_tstepx]
 sar ecx,16
 sar eax,16
 imul ds:dword ptr[4+0+esp]
 add eax,ecx
 mov ds:dword ptr[_a_ststepxwhole],eax
 ret
 public _D_PolysetRecursiveTriangleT3
_D_PolysetRecursiveTriangleT3:
 push ebp
 push esi
 push edi
 push ebx
 mov esi,ds:dword ptr[8+16+esp]
 mov ebx,ds:dword ptr[4+16+esp]
 mov edi,ds:dword ptr[12+16+esp]
 mov eax,ds:dword ptr[0+esi]
 mov edx,ds:dword ptr[0+ebx]
 mov ebp,ds:dword ptr[4+esi]
 sub eax,edx
 mov ecx,ds:dword ptr[4+ebx]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit
 mov eax,ds:dword ptr[0+edi]
 inc ebp
 cmp ebp,2
 ja LSplit
 mov edx,ds:dword ptr[0+esi]
 mov ebp,ds:dword ptr[4+edi]
 sub eax,edx
 mov ecx,ds:dword ptr[4+esi]
 sub ebp,ecx
 inc eax
 cmp eax,2
 ja LSplit2
 mov eax,ds:dword ptr[0+ebx]
 inc ebp
 cmp ebp,2
 ja LSplit2
 mov edx,ds:dword ptr[0+edi]
 mov ebp,ds:dword ptr[4+ebx]
 sub eax,edx
 mov ecx,ds:dword ptr[4+edi]
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
 mov eax,ds:dword ptr[8+ebx]
 mov edx,ds:dword ptr[8+esi]
 mov ecx,ds:dword ptr[12+ebx]
 add eax,edx
 mov edx,ds:dword ptr[12+esi]
 sar eax,1
 add ecx,edx
 mov ds:dword ptr[8+esp],eax
 mov eax,ds:dword ptr[20+ebx]
 sar ecx,1
 mov edx,ds:dword ptr[20+esi]
 mov ds:dword ptr[12+esp],ecx
 add eax,edx
 mov ecx,ds:dword ptr[0+ebx]
 mov edx,ds:dword ptr[0+esi]
 sar eax,1
 add edx,ecx
 mov ds:dword ptr[20+esp],eax
 mov eax,ds:dword ptr[4+ebx]
 sar edx,1
 mov ebp,ds:dword ptr[4+esi]
 mov ds:dword ptr[0+esp],edx
 add ebp,eax
 sar ebp,1
 mov ds:dword ptr[4+esp],ebp
 cmp ds:dword ptr[4+esi],eax
 jg LNoDraw
 mov edx,ds:dword ptr[0+esi]
 jnz LDraw
 cmp edx,ecx
 jl LNoDraw
LDraw:
 mov edx,ds:dword ptr[20+esp]
 mov ecx,ds:dword ptr[4+esp]
 sar edx,16
 mov ebp,ds:dword ptr[0+esp]
 mov eax,ds:dword ptr[_zspantable+ecx*4]
 cmp dx,ds:word ptr[eax+ebp*2]
 jnge LNoDraw
 mov ds:word ptr[eax+ebp*2],dx
 mov eax,ds:dword ptr[12+esp]
 sar eax,16
 mov edx,ds:dword ptr[8+esp]
 sar edx,16
 sub ecx,ecx
 mov eax,ds:dword ptr[_skintable+eax*4]
 mov ebp,ds:dword ptr[4+esp]
 mov cl,ds:byte ptr[eax+edx]
 or cl,cl
 jz Skip1B  ; color 0 = no draw
 mov edx,ds:dword ptr[_d_pcolormap]
 mov dl,ds:byte ptr[edx+ecx]
 mov ecx,ds:dword ptr[0+esp]
 mov eax,ds:dword ptr[_d_scantable+ebp*4]
 add ecx,eax
 mov eax,ds:dword ptr[_d_viewbuffer]
 mov ds:byte ptr[eax+ecx*1],dl
Skip1B:
LNoDraw:
 push esp
 push ebx
 push edi
 call near ptr _D_PolysetRecursiveTriangleT3
 mov ebx,esp
 push esi
 push ebx
 push edi
 call near ptr _D_PolysetRecursiveTriangleT3
 add esp,24
LDone:
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret 12
 public _D_PolysetAff8StartT3
_D_PolysetAff8StartT3:
 public _D_PolysetDrawSpans8T3
_D_PolysetDrawSpans8T3:
 push esi
 push ebx
 mov esi,ds:dword ptr[4+8+esp]
 mov ecx,ds:dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 mov edx,ds:dword ptr[8+esi]
 mov ds:dword ptr[lzistepx],ecx
LSpanLoop:
 mov eax,ds:dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,ds:dword ptr[_erroradjustup]
 mov ebx,ds:dword ptr[_errorterm]
 add ebx,edx
 js LNoTurnover
 mov edx,ds:dword ptr[_erroradjustdown]
 mov edi,ds:dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,ds:dword ptr[_d_aspancount]
 mov ds:dword ptr[_errorterm],ebx
 add ebp,edi
 mov ds:dword ptr[_d_aspancount],ebp
 jmp LRightEdgeStepped
LNoTurnover:
 mov edi,ds:dword ptr[_d_aspancount]
 mov edx,ds:dword ptr[_ubasestep]
 mov ds:dword ptr[_errorterm],ebx
 add edi,edx
 mov ds:dword ptr[_d_aspancount],edi
LRightEdgeStepped:
 cmp eax,1
 jl LNextSpan
 jz LExactlyOneLong
 mov ecx,ds:dword ptr[_a_ststepxwhole]
 mov edx,ds:dword ptr[_r_affinetridesc+8]
 mov ds:dword ptr[advancetable+4],ecx
 add ecx,edx
 mov ds:dword ptr[advancetable],ecx
 mov ecx,ds:dword ptr[_a_tstepxfrac]
 mov cx,ds:word ptr[_r_lstepx]
 mov edx,eax
 mov ds:dword ptr[tstep],ecx
 add edx,7
 shr edx,3
 mov ebx,ds:dword ptr[16+esi]
 mov bx,dx
 mov ecx,ds:dword ptr[4+esi]
 neg eax
 mov edi,ds:dword ptr[0+esi]
 and eax,7
 sub edi,eax
 sub ecx,eax
 sub ecx,eax
 mov edx,ds:dword ptr[20+esi]
 mov dx,ds:word ptr[24+esi]
 mov ebp,ds:dword ptr[28+esi]
 ror ebp,16
 push esi
 mov esi,ds:dword ptr[12+esi]
 jmp dword ptr[aff8entryvec_table+eax*4]
LDrawLoop:
LDraw8:
 cmp bp,ds:word ptr[ecx]
 jl Lp1
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipA2  ; color 0 = no draw
 mov ds:word ptr[ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch8:
 mov ds:byte ptr[edi],al
SkipA2:
Lp1:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw7:
 cmp bp,ds:word ptr[2+ecx]
 jl Lp2
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipB2  ; color 0 = no draw
 mov ds:word ptr[2+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch7:
 mov ds:byte ptr[1+edi],al
SkipB2:
Lp2:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw6:
 cmp bp,ds:word ptr[4+ecx]
 jl Lp3
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipC2  ; color 0 = no draw
 mov ds:word ptr[4+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch6:
 mov ds:byte ptr[2+edi],al
SkipC2:
Lp3:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw5:
 cmp bp,ds:word ptr[6+ecx]
 jl Lp4
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipD2  ; color 0 = no draw
 mov ds:word ptr[6+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch5:
 mov ds:byte ptr[3+edi],al
SkipD2:
Lp4:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw4:
 cmp bp,ds:word ptr[8+ecx]
 jl Lp5
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipE2  ; color 0 = no draw
 mov ds:word ptr[8+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch4:
 mov ds:byte ptr[4+edi],al
SkipE2:
Lp5:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw3:
 cmp bp,ds:word ptr[10+ecx]
 jl Lp6
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipF2  ; color 0 = no draw
 mov ds:word ptr[10+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch3:
 mov ds:byte ptr[5+edi],al
SkipF2:
Lp6:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw2:
 cmp bp,ds:word ptr[12+ecx]
 jl Lp7
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipG2  ; color 0 = no draw
 mov ds:word ptr[12+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch2:
 mov ds:byte ptr[6+edi],al
SkipG2:
Lp7:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LDraw1:
 cmp bp,ds:word ptr[14+ecx]
 jl Lp8
 xor eax,eax
 mov ah,dh
 mov al,ds:byte ptr[esi]
 or al,al
 jz SkipH2  ; color 0 = no draw
 mov ds:word ptr[14+ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch1:
 mov ds:byte ptr[7+edi],al
SkipH2:
Lp8:
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebp,ds:dword ptr[lzistepx]
 adc ebp,0
 add ebx,ds:dword ptr[_a_sstepxfrac]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LDrawLoop
 pop esi
LNextSpan:
 add esi,32
LNextSpanESISet:
 mov edx,ds:dword ptr[8+esi]
 cmp edx,offset -999999
 jnz LSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LExactlyOneLong:
 mov ecx,ds:dword ptr[4+esi]
 mov ebp,ds:dword ptr[28+esi]
 ror ebp,16
 mov ebx,ds:dword ptr[12+esi]
 cmp bp,ds:word ptr[ecx]
 jl LNextSpan
 xor eax,eax
 mov edi,ds:dword ptr[0+esi]
 mov ah,ds:byte ptr[24+1+esi]
 add esi,32
 mov al,ds:byte ptr[ebx]
 or al,al
 jz SkipI2  ; color 0 = no draw
 mov ds:word ptr[ecx],bp
 mov al,ds:byte ptr[12345678h+eax]
LPatch9:
 mov ds:byte ptr[edi],al
SkipI2:
 jmp LNextSpanESISet
 public _D_PolysetAff8EndT3
_D_PolysetAff8EndT3:
 public _D_Aff8PatchT3
_D_Aff8PatchT3:
 mov eax,ds:dword ptr[4+esp]
 mov ds:dword ptr[LPatch1-4],eax
 mov ds:dword ptr[LPatch2-4],eax
 mov ds:dword ptr[LPatch3-4],eax
 mov ds:dword ptr[LPatch4-4],eax
 mov ds:dword ptr[LPatch5-4],eax
 mov ds:dword ptr[LPatch6-4],eax
 mov ds:dword ptr[LPatch7-4],eax
 mov ds:dword ptr[LPatch8-4],eax
 mov ds:dword ptr[LPatch9-4],eax
 ret

 public _D_PolysetDrawT3
_D_PolysetDrawT3:
 sub esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 mov eax,esp
 add eax,32 - 1
 and eax,offset not (32 - 1)
 mov ds:dword ptr[_a_spans],eax
 mov eax,ds:dword ptr[_r_affinetridesc+28]
 test eax,eax
 jz _D_DrawNonSubdivT3
; push ebp
; mov ebp,ds:dword ptr[_r_affinetridesc+24]
 push esi
; shl ebp,4
 push ebx
 mov ebx,ds:dword ptr[_r_affinetridesc+16]
 push edi
 mov edi,ds:dword ptr[_r_affinetridesc+20]
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
 fild ds:dword ptr[0+4+ecx]
 fild ds:dword ptr[0+4+esi]
 fild ds:dword ptr[0+0+ecx]
 fild ds:dword ptr[0+0+edx]
 fxch st(2)
 fsubr st(0),st(3)
 fild ds:dword ptr[0+0+esi]
 fxch st(2)
 fsubr st(3),st(0)
 fild ds:dword ptr[0+4+edx]
 fxch st(1)
 fsubrp st(3),st(0)
 fxch st(1)
 fmulp st(3),st(0)
 fsubp st(3),st(0)
 mov eax,ds:dword ptr[0+16+ecx]
 and eax,0FF00h
 fmulp st(2),st(0)
 add eax,ds:dword ptr[_acolormap]
 fsubrp st(1),st(0)
 mov ds:dword ptr[_d_pcolormap],eax
 fstp ds:dword ptr[Ltemp]
 mov eax,ds:dword ptr[Ltemp]
 sub eax,080000001h
 jc Lskip
 mov eax,ds:dword ptr[0+ebx]
 test eax,eax
 jz Lfacesback
 push edx
 push esi
 push ecx
 call near ptr _D_PolysetRecursiveTriangleT3
; sub ebp,16
; jnz Llooptop
 jmp Ldone2
Lfacesback:
 mov eax,ds:dword ptr[0+8+ecx]
 push eax
 mov eax,ds:dword ptr[0+8+esi]
 push eax
 mov eax,ds:dword ptr[0+8+edx]
 push eax
 push ecx
 push edx
 mov eax,ds:dword ptr[_r_affinetridesc+32]
 test ds:dword ptr[24+ecx],00020h
 jz Lp11
 add ds:dword ptr[0+8+ecx],eax
Lp11:
 test ds:dword ptr[24+esi],00020h
 jz Lp12
 add ds:dword ptr[0+8+esi],eax
Lp12:
 test ds:dword ptr[24+edx],00020h
 jz Lp13
 add ds:dword ptr[0+8+edx],eax
Lp13:
 push edx
 push esi
 push ecx
 call near ptr _D_PolysetRecursiveTriangleT3
 pop edx
 pop ecx
 pop eax
 mov ds:dword ptr[0+8+edx],eax
 pop eax
 mov ds:dword ptr[0+8+esi],eax
 pop eax
 mov ds:dword ptr[0+8+ecx],eax
Lskip:
; sub ebp,16
; jnz Llooptop
Ldone2:
 pop edi
 pop ebx
 pop esi
; pop ebp
 add esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 ret

 public _D_PolysetScanLeftEdgeT3
_D_PolysetScanLeftEdgeT3:
 push ebp
 push esi
 push edi
 push ebx
 mov eax,ds:dword ptr[4+16+esp]
 mov ecx,ds:dword ptr[_d_sfrac]
 and eax,0FFFFh
 mov ebx,ds:dword ptr[_d_ptex]
 or ecx,eax
 mov esi,ds:dword ptr[_d_pedgespanpackage]
 mov edx,ds:dword ptr[_d_tfrac]
 mov edi,ds:dword ptr[_d_light]
 mov ebp,ds:dword ptr[_d_zi]
LScanLoop:
 mov ds:dword ptr[12+esi],ebx
 mov eax,ds:dword ptr[_d_pdest]
 mov ds:dword ptr[0+esi],eax
 mov eax,ds:dword ptr[_d_pz]
 mov ds:dword ptr[4+esi],eax
 mov eax,ds:dword ptr[_d_aspancount]
 mov ds:dword ptr[8+esi],eax
 mov ds:dword ptr[24+esi],edi
 mov ds:dword ptr[28+esi],ebp
 mov ds:dword ptr[16+esi],ecx
 mov ds:dword ptr[20+esi],edx
 mov al,ds:byte ptr[32+esi]
 add esi,32
 mov eax,ds:dword ptr[_erroradjustup]
 mov ds:dword ptr[_d_pedgespanpackage],esi
 mov esi,ds:dword ptr[_errorterm]
 add esi,eax
 mov eax,ds:dword ptr[_d_pdest]
 js LNoLeftEdgeTurnover
 sub esi,ds:dword ptr[_erroradjustdown]
 add eax,ds:dword ptr[_d_pdestextrastep]
 mov ds:dword ptr[_errorterm],esi
 mov ds:dword ptr[_d_pdest],eax
 mov eax,ds:dword ptr[_d_pz]
 mov esi,ds:dword ptr[_d_aspancount]
 add eax,ds:dword ptr[_d_pzextrastep]
 add ecx,ds:dword ptr[_d_sfracextrastep]
 adc ebx,ds:dword ptr[_d_ptexextrastep]
 add esi,ds:dword ptr[_d_countextrastep]
 mov ds:dword ptr[_d_pz],eax
 mov eax,ds:dword ptr[_d_tfracextrastep]
 mov ds:dword ptr[_d_aspancount],esi
 add edx,eax
 jnc LSkip1
 add ebx,ds:dword ptr[_r_affinetridesc+8]
LSkip1:
 add edi,ds:dword ptr[_d_lightextrastep]
 add ebp,ds:dword ptr[_d_ziextrastep]
 mov esi,ds:dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret
LNoLeftEdgeTurnover:
 mov ds:dword ptr[_errorterm],esi
 add eax,ds:dword ptr[_d_pdestbasestep]
 mov ds:dword ptr[_d_pdest],eax
 mov eax,ds:dword ptr[_d_pz]
 mov esi,ds:dword ptr[_d_aspancount]
 add eax,ds:dword ptr[_d_pzbasestep]
 add ecx,ds:dword ptr[_d_sfracbasestep]
 adc ebx,ds:dword ptr[_d_ptexbasestep]
 add esi,ds:dword ptr[_ubasestep]
 mov ds:dword ptr[_d_pz],eax
 mov ds:dword ptr[_d_aspancount],esi
 mov esi,ds:dword ptr[_d_tfracbasestep]
 add edx,esi
 jnc LSkip2
 add ebx,ds:dword ptr[_r_affinetridesc+8]
LSkip2:
 add edi,ds:dword ptr[_d_lightbasestep]
 add ebp,ds:dword ptr[_d_zibasestep]
 mov esi,ds:dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

_L_PDFVertT3:
 push esi
 push edi
 mov eax,ds:dword ptr[0+0+ebx]
 mov edx,ds:dword ptr[_r_refdef+40]
 cmp eax,edx
 jge LNextVert
 mov esi,ds:dword ptr[0+4+ebx]
 mov edx,ds:dword ptr[_r_refdef+44]
 cmp esi,edx
 jge LNextVert
 mov edi,ds:dword ptr[_zspantable+esi*4]
 mov edx,ds:dword ptr[0+20+ebx]
 shr edx,16
 cmp dx,ds:word ptr[edi+eax*2]
 jl LNextVert
 mov ds:word ptr[edi+eax*2],dx
 mov edi,ds:dword ptr[0+12+ebx]
 shr edi,16
 mov edi,ds:dword ptr[_skintable+edi*4]
 mov edx,ds:dword ptr[0+8+ebx]
 shr edx,16
 mov dl,ds:byte ptr[edi+edx]
 or dl,dl
 jz Skip2B  ; color 0 = no draw
 mov edi,ds:dword ptr[0+16+ebx]
 and edi,0FF00h
 and edx,000FFh
 add edi,edx
 mov edx,ds:dword ptr[_acolormap]
 mov dl,ds:byte ptr[edx+edi*1]
 mov edi,ds:dword ptr[_d_scantable+esi*4]
 mov esi,ds:dword ptr[_d_viewbuffer]
 add edi,eax
 mov ds:byte ptr[esi+edi],dl
Skip2B:
LNextVert:
 pop edi
 pop esi
ret

 public _D_PolysetDrawFinalVertsT3
_D_PolysetDrawFinalVertsT3:
 push ebp
 push ebx
 mov ebx,dword ptr[4+8+esp]	;pv1
 call _L_PDFVertT3
 mov ebx,dword ptr[8+8+esp]	;pv2
 call _L_PDFVertT3
 mov ebx,dword ptr[12+8+esp];pv3
 call _L_PDFVertT3
 pop ebx
 pop ebp
 ret

 public _D_DrawNonSubdivT3
_D_DrawNonSubdivT3:
; push ebp
; mov ebp,ds:dword ptr[_r_affinetridesc+24]
 push ebx
; shl ebp,4
 push esi
 mov esi,ds:dword ptr[_r_affinetridesc+16]
 push edi
LNDLoop:
 mov edi,ds:dword ptr[_r_affinetridesc+20]
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
 mov eax,ds:dword ptr[0+4+ecx]
 mov esi,ds:dword ptr[0+0+ecx]
 sub eax,ds:dword ptr[0+4+edx]
 sub esi,ds:dword ptr[0+0+ebx]
 imul eax,esi
 mov esi,ds:dword ptr[0+0+ecx]
 mov edi,ds:dword ptr[0+4+ecx]
 sub esi,ds:dword ptr[0+0+edx]
 sub edi,ds:dword ptr[0+4+ebx]
 imul edi,esi
 sub eax,edi
 jns LNextTri
 mov ds:dword ptr[_d_xdenom],eax
 fild ds:dword ptr[_d_xdenom]
 mov eax,ds:dword ptr[0+0+ecx]
 mov esi,ds:dword ptr[0+4+ecx]
 mov ds:dword ptr[_r_p0+0],eax
 mov ds:dword ptr[_r_p0+4],esi
 mov eax,ds:dword ptr[0+8+ecx]
 mov esi,ds:dword ptr[0+12+ecx]
 mov ds:dword ptr[_r_p0+8],eax
 mov ds:dword ptr[_r_p0+12],esi
 mov eax,ds:dword ptr[0+16+ecx]
 mov esi,ds:dword ptr[0+20+ecx]
 mov ds:dword ptr[_r_p0+16],eax
 mov ds:dword ptr[_r_p0+20],esi
 fdivr ds:dword ptr[float_1]
 mov eax,ds:dword ptr[0+0+edx]
 mov esi,ds:dword ptr[0+4+edx]
 mov ds:dword ptr[_r_p1+0],eax
 mov ds:dword ptr[_r_p1+4],esi
 mov eax,ds:dword ptr[0+8+edx]
 mov esi,ds:dword ptr[0+12+edx]
 mov ds:dword ptr[_r_p1+8],eax
 mov ds:dword ptr[_r_p1+12],esi
 mov eax,ds:dword ptr[0+16+edx]
 mov esi,ds:dword ptr[0+20+edx]
 mov ds:dword ptr[_r_p1+16],eax
 mov ds:dword ptr[_r_p1+20],esi
 mov eax,ds:dword ptr[0+0+ebx]
 mov esi,ds:dword ptr[0+4+ebx]
 mov ds:dword ptr[_r_p2+0],eax
 mov ds:dword ptr[_r_p2+4],esi
 mov eax,ds:dword ptr[0+8+ebx]
 mov esi,ds:dword ptr[0+12+ebx]
 mov ds:dword ptr[_r_p2+8],eax
 mov ds:dword ptr[_r_p2+12],esi
 mov eax,ds:dword ptr[0+16+ebx]
 mov esi,ds:dword ptr[0+20+ebx]
 mov ds:dword ptr[_r_p2+16],eax
 mov edi,ds:dword ptr[_r_affinetridesc+16]
 mov ds:dword ptr[_r_p2+20],esi
 mov eax,ds:dword ptr[0+edi]
 test eax,eax
 jnz LFacesFront
 mov eax,ds:dword ptr[24+ecx]
 mov esi,ds:dword ptr[24+edx]
 mov edi,ds:dword ptr[24+ebx]
 test eax,00020h
 mov eax,ds:dword ptr[_r_affinetridesc+32]
 jz LOnseamDone0
 add ds:dword ptr[_r_p0+8],eax
LOnseamDone0:
 test esi,00020h
 jz LOnseamDone1
 add ds:dword ptr[_r_p1+8],eax
LOnseamDone1:
 test edi,00020h
 jz LOnseamDone2
 add ds:dword ptr[_r_p2+8],eax
LOnseamDone2:
LFacesFront:
 fstp ds:dword ptr[_d_xdenom]
 call near ptr _D_PolysetSetEdgeTable
 call near ptr _D_RasterizeAliasPolySmooth
LNextTri:
 mov esi,ds:dword ptr[_r_affinetridesc+16]
; sub ebp,16
; jnz LNDLoop
 pop edi
 pop esi
 pop ebx
; pop ebp
 add esp,offset (((1024+1 + 1 + ((32 - 1) / 32)) + 1) * 32)
 ret
_TEXT ENDS
 END
