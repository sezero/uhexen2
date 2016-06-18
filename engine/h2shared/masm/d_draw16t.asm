;
; d_draw16t.asm -- for MASM
; x86 assembly-language horizontal 8-bpp span-drawing code, with 16-pixel
; subdivision and translucency handling.
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
 externdef _d_zistepu:dword
 externdef _d_pzbuffer:dword
 externdef _d_zistepv:dword
 externdef _d_zrowbytes:dword
 externdef _d_ziorigin:dword
 externdef _d_sdivzstepu:dword
 externdef _d_tdivzstepu:dword
 externdef _d_sdivzstepv:dword
 externdef _d_tdivzstepv:dword
 externdef _d_sdivzorigin:dword
 externdef _d_tdivzorigin:dword
 externdef _sadjust:dword
 externdef _tadjust:dword
 externdef _bbextents:dword
 externdef _bbextentt:dword
 externdef _cacheblock:dword
 externdef _d_viewbuffer:dword
 externdef _cachewidth:dword
 externdef _d_scantable:dword
 externdef _mainTransTable:dword
 externdef _scanList:dword
 externdef _D_DrawSingleZSpans:dword

; externs from ASM-only code
 externdef float_point5:dword
 externdef Float2ToThe31nd:dword
 externdef izistep:dword
 externdef izi:dword
 externdef FloatMinus2ToThe31nd:dword
 externdef float_1:dword
 externdef float_particle_z_clip:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef fp_16:dword
 externdef fp_64k:dword
 externdef fp_1m:dword
 externdef fp_1m_minus_1:dword
 externdef fp_8:dword
 externdef entryvec_table:dword
 externdef advancetable:dword
 externdef sstep:dword
 externdef tstep:dword
 externdef pspantemp:dword
 externdef counttemp:dword
 externdef jumptemp:dword
 externdef reciprocal_table:dword
 externdef pbase:dword
 externdef s:dword
 externdef t:dword
 externdef sfracf:dword
 externdef tfracf:dword
 externdef snext:dword
 externdef tnext:dword
 externdef spancountminus1:dword
 externdef zi16stepu:dword
 externdef sdivz16stepu:dword
 externdef tdivz16stepu:dword
 externdef zi8stepu:dword
 externdef sdivz8stepu:dword
 externdef tdivz8stepu:dword
 externdef reciprocal_table_16:dword
 externdef entryvec_table_16T:dword
 externdef fp_64kx64k:dword

_DATA SEGMENT
masktemp dw 0
_DATA ENDS
_TEXT SEGMENT
 public _D_Draw16StartT
_D_Draw16StartT:
LClampHigh0:
 mov esi,ds:dword ptr[_bbextents]
 jmp LClampReentry0
LClampHighOrLow0:
 jg LClampHigh0
 xor esi,esi
 jmp LClampReentry0
LClampHigh1:
 mov edx,ds:dword ptr[_bbextentt]
 jmp LClampReentry1
LClampHighOrLow1:
 jg LClampHigh1
 xor edx,edx
 jmp LClampReentry1
LClampLow2:
 mov ebp,4096
 jmp LClampReentry2
LClampHigh2:
 mov ebp,ds:dword ptr[_bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,4096
 jmp LClampReentry3
LClampHigh3:
 mov ecx,ds:dword ptr[_bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,4096
 jmp LClampReentry4
LClampHigh4:
 mov eax,ds:dword ptr[_bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,4096
 jmp LClampReentry5
LClampHigh5:
 mov ebx,ds:dword ptr[_bbextentt]
 jmp LClampReentry5
 align 4
 public _D_DrawSpans16T
_D_DrawSpans16T:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_sdivzstepu]
 fmul ds:dword ptr[fp_16]
 mov edx,ds:dword ptr[_cacheblock]
 fld ds:dword ptr[_d_tdivzstepu]
 fmul ds:dword ptr[fp_16]
 mov ebx,ds:dword ptr[4+16+esp]
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_16]
 mov ds:dword ptr[pbase],edx
 fstp ds:dword ptr[zi16stepu]
 fstp ds:dword ptr[tdivz16stepu]
 fstp ds:dword ptr[sdivz16stepu]
LSpanLoop:
 fild ds:dword ptr[4+ebx]
 fild ds:dword ptr[0+ebx]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepv]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fld st(2)
 fmul ds:dword ptr[_d_tdivzstepu]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(1)
 fld st(3)
 fmul ds:dword ptr[_d_tdivzstepv]
 fxch st(1)
 fadd ds:dword ptr[_d_sdivzorigin]
 fxch st(4)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(2)
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fadd ds:dword ptr[_d_tdivzorigin]
 fxch st(2)
 faddp st(1),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 fdiv st(1),st(0)
 mov ecx,ds:dword ptr[_d_viewbuffer]
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[pspantemp],ebx

 push eax
 push ecx
 push edx
 push ebx

 call near ptr _D_DrawSingleZSpans

 pop ebx
 pop edx
 pop ecx
 pop eax

 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov edi,ds:dword ptr[_d_scantable+eax*4]
 add edi,ecx
 mov ecx,ds:dword ptr[0+ebx]
 add edi,ecx
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,16
 ja LSetupNotLast1
 dec ecx
 jz LCleanup1
 mov ds:dword ptr[spancountminus1],ecx
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_tdivzstepu]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(2)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 faddp st(3),st(0)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
 jmp LFDIVInFlight1
LCleanup1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 jmp LFDIVInFlight1
 align 4
LSetupNotLast1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fadd ds:dword ptr[zi16stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz16stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz16stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight1:
 add esi,ds:dword ptr[s]
 add edx,ds:dword ptr[t]
 mov ebx,ds:dword ptr[_bbextents]
 mov ebp,ds:dword ptr[_bbextentt]
 cmp esi,ebx
 ja LClampHighOrLow0
LClampReentry0:
 mov ds:dword ptr[s],esi
 mov ebx,ds:dword ptr[pbase]
 shl esi,16
 cmp edx,ebp
 mov ds:dword ptr[sfracf],esi
 ja LClampHighOrLow1
LClampReentry1:
 mov ds:dword ptr[t],edx
 mov esi,ds:dword ptr[s]
 shl edx,16
 mov eax,ds:dword ptr[t]
 sar esi,16
 mov ds:dword ptr[tfracf],edx
 sar eax,16
 mov edx,ds:dword ptr[_cachewidth]
 imul eax,edx
 add esi,ebx
 add esi,eax
 cmp ecx,16
 jna LLastSegment
LNotLastSegment:
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov eax,ds:dword ptr[snext]
 mov edx,ds:dword ptr[tnext]

 xor ebx,ebx
 add bl,ds:byte ptr[_scanList + ecx - 1]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 2]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 3]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 4]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 5]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 6]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 7]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 8]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 9]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 10]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 11]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 12]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 13]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 14]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 15]
 shl ebx,1
 add bl,ds:byte ptr[_scanList + ecx - 16]
 ;mov bx, 8000h
; mov bx, 0ffffh
; mov bx, 0h
 mov masktemp, bx

 mov bh,ds:byte ptr[esi]
 sub ecx,16
 mov ebp,ds:dword ptr[_sadjust]
 mov ds:dword ptr[counttemp],ecx
 mov ecx,ds:dword ptr[_tadjust]

 ;and masktemp, 8000h
 bt masktemp, 15
 jnc SkipTran1

 ;rj
 mov bl,ds:byte ptr[edi]
 and ebx, 0ffffh
 mov bl,ds:byte ptr[12345678h + ebx]
TranPatch1:  

 mov ds:byte ptr[edi],bl

SkipTran1:
 add ebp,eax
 add ecx,edx
 mov eax,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp ebp,4096
 jl LClampLow2
 cmp ebp,eax
 ja LClampHigh2
LClampReentry2:
 cmp ecx,4096
 jl LClampLow3
 cmp ecx,edx
 ja LClampHigh3
LClampReentry3:
 mov ds:dword ptr[snext],ebp
 mov ds:dword ptr[tnext],ecx
 sub ebp,ds:dword ptr[s]
 sub ecx,ds:dword ptr[t]
 mov eax,ecx
 mov edx,ebp
 sar eax,20
 jz LZero
 sar edx,20
 mov ebx,ds:dword ptr[_cachewidth]
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,20
 mov ebx,ds:dword ptr[_cachewidth]
LSetUp1:
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],eax
 add eax,ebx
 shl ebp,12
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,12
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 mov ah,ds:byte ptr[esi]

; and masktemp, 4000h
 bt masktemp, 14
 jnc SkipTran2

;rj
 mov al,ds:byte ptr[1+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch2:

 mov ds:byte ptr[1+edi],al

SkipTran2:
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp

 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ah,ds:byte ptr[esi]

; and masktemp, 2000h
 bt masktemp, 13
 jnc SkipTran3

;rj
 mov al,ds:byte ptr[2+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch3:

 mov ds:byte ptr[2+edi],al

SkipTran3:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

; and masktemp, 1000h
 bt masktemp, 12
 jnc SkipTran4

;rj
 mov al,ds:byte ptr[3+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch4:

 mov ds:byte ptr[3+edi],al

SkipTran4:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 
 mov ah,ds:byte ptr[esi]

; and masktemp, 0800h
 bt masktemp, 11
 jnc SkipTran5

;rj
 mov al,ds:byte ptr[4+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch5:

 mov ds:byte ptr[4+edi],al

SkipTran5:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

; and masktemp, 0400h
 bt masktemp, 10
 jnc SkipTran6

;rj
 mov al,ds:byte ptr[5+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch6:

 mov ds:byte ptr[5+edi],al

SkipTran6:
; rj speed test
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

; and masktemp, 0200h
 bt masktemp, 9
 jnc SkipTran7

;rj
 mov al,ds:byte ptr[6+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch7:

 mov ds:byte ptr[6+edi],al



; add ebx,ebp
; adc esi,ds:dword ptr[advancetable+4+ecx*4]
; xor eax, eax
; add edx,ds:dword ptr[tstep]
; mov al,ds:byte ptr[6+edi]
; sbb ecx,ecx
; mov ah,ds:byte ptr[esi]

;rj
; add eax,ds:dword ptr[_mainTransTable]
; mov al,ds:byte ptr[eax + _mainTransTable]

; mov ds:byte ptr[6+edi],al













SkipTran7:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

; and masktemp, 0100h
 bt masktemp, 8
 jnc SkipTran8

;rj
 mov al,ds:byte ptr[7+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch8:

 mov ds:byte ptr[7+edi],al

SkipTran8:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,16
 ja LSetupNotLast2
 dec ecx
 jz LFDIVInFlight2
 mov ds:dword ptr[spancountminus1],ecx
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(1)
 fld ds:dword ptr[_d_tdivzstepu]
 fmul st(0),st(2)
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 faddp st(4),st(0)
 fdiv st(0),st(1)
 jmp LFDIVInFlight2
 align 4
LSetupNotLast2:
 fadd ds:dword ptr[zi16stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz16stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz16stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight2:
 mov ds:dword ptr[counttemp],ecx
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 bt masktemp, 7
 jnc SkipTran9

;rj
 mov al,ds:byte ptr[8+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch9:

 mov ds:byte ptr[8+edi],al

SkipTran9:
 mov ah,ds:byte ptr[esi]

 bt masktemp, 6
 jnc SkipTran10

 ;rj
 mov al,ds:byte ptr[9+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch10:

 mov ds:byte ptr[9+edi],al

SkipTran10:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

 bt masktemp, 5
 jnc SkipTran11

 ;rj
 mov al,ds:byte ptr[10+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch11:

 mov ds:byte ptr[10+edi],al

SkipTran11:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

 bt masktemp, 4
 jnc SkipTran12

 ;rj
 mov al,ds:byte ptr[11+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch12:

 mov ds:byte ptr[11+edi],al

SkipTran12:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

 bt masktemp, 3
 jnc SkipTran13

 ;rj
 mov al,ds:byte ptr[12+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch13:

 mov ds:byte ptr[12+edi],al

SkipTran13:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

 bt masktemp, 2
 jnc SkipTran14

 ;rj
 mov al,ds:byte ptr[13+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch14:

 mov ds:byte ptr[13+edi],al

SkipTran14:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 mov ah,ds:byte ptr[esi]

 bt masktemp, 1
 jnc SkipTran15

 ;rj
 mov al,ds:byte ptr[14+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch15:

 mov ds:byte ptr[14+edi],al

SkipTran15:
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edi,16
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ecx,ds:dword ptr[counttemp]


 bt masktemp, 0
 jnc SkipTran16


 ;rj
 mov al,ds:byte ptr[-1+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch16:

 mov ds:byte ptr[-1+edi],al

SkipTran16:
 cmp ecx,16
 ja LNotLastSegment
LLastSegment:
 test ecx,ecx
 jz LNoSteps
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov ah,ds:byte ptr[esi]
 mov ebx,ds:dword ptr[_tadjust]
 

 cmp ds:byte ptr[_scanList + ecx - 1], 1
 jnz SkipTran17


 ;rj
 mov al,ds:byte ptr[edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch17:

 mov ds:byte ptr[edi],al

SkipTran17:
 mov eax,ds:dword ptr[_sadjust]
 add eax,ds:dword ptr[snext]
 add ebx,ds:dword ptr[tnext]
 mov ebp,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp eax,4096
 jl LClampLow4
 cmp eax,ebp
 ja LClampHigh4
LClampReentry4:
 mov ds:dword ptr[snext],eax
 cmp ebx,4096
 jl LClampLow5
 cmp ebx,edx
 ja LClampHigh5
LClampReentry5:
 cmp ecx,1
 je LOnlyOneStep
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 add eax,eax
 add ebx,ebx
 imul ds:dword ptr[reciprocal_table_16-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul ds:dword ptr[reciprocal_table_16-8+ecx*4]
LSetEntryvec:
 mov ebx,ds:dword ptr[entryvec_table_16T+ecx*4]
 mov eax,edx
 mov ds:dword ptr[jumptemp],ebx
 mov ecx,ebp
 sar edx,16
 mov ebx,ds:dword ptr[_cachewidth]
 sar ecx,16
 imul edx,ebx
 add edx,ecx
 mov ecx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx,ds:dword ptr[sfracf]
 shl eax,16
 mov ds:dword ptr[advancetable],edx
 mov ds:dword ptr[tstep],eax
 mov edx,ecx
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp dword ptr[jumptemp]
LNoSteps:
 mov ah,ds:byte ptr[esi]
 sub edi,15
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
 public Entry2_16T, Entry3_16T, Entry4_16T, Entry5_16T
 public Entry6_16T, Entry7_16T, Entry8_16T, Entry9_16T
 public Entry10_16T, Entry11_16T, Entry12_16T, Entry13_16T
 public Entry14_16T, Entry15_16T, Entry16_16T
Entry2_16T:
 sub edi,14
 mov ah,ds:byte ptr[esi]
 jmp LEntry2_16
Entry3_16T:
 sub edi,13
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp LEntry3_16
Entry4_16T:
 sub edi,12
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry4_16
Entry5_16T:
 sub edi,11
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry5_16
Entry6_16T:
 sub edi,10
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry6_16
Entry7_16T:
 sub edi,9
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry7_16
Entry8_16T:
 sub edi,8
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry8_16
Entry9_16T:
 sub edi,7
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry9_16
Entry10_16T:
 sub edi,6
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry10_16
Entry11_16T:
 sub edi,5
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry11_16
Entry12_16T:
 sub edi,4
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry12_16
Entry13_16T:
 sub edi,3
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry13_16
Entry14_16T:
 sub edi,2
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry14_16
Entry15_16T:
 dec edi
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry15_16
Entry16_16T:
 add edx,eax
 mov ah,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 14], 1
 jnz SkipTran18

 ;rj
 mov al,ds:byte ptr[1+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch18:

 mov ds:byte ptr[1+edi],al

SkipTran18:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry15_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 13], 1
 jnz SkipTran19

 ;rj
 mov al,ds:byte ptr[2+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch19:

 mov ds:byte ptr[2+edi],al

SkipTran19:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry14_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 12], 1
 jnz SkipTran20

 ;rj
 mov al,ds:byte ptr[3+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch20:

 mov ds:byte ptr[3+edi],al

SkipTran20:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry13_16:
 sbb ecx,ecx


 cmp ds:byte ptr[_scanList + 11], 1
 jnz SkipTran21


 ;rj
 mov al,ds:byte ptr[4+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch21:

 mov ds:byte ptr[4+edi],al

SkipTran21:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry12_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 10], 1
 jnz SkipTran22

 ;rj
 mov al,ds:byte ptr[5+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch22:

 mov ds:byte ptr[5+edi],al

SkipTran22:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry11_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 9], 1
 jnz SkipTran23

 ;rj
 mov al,ds:byte ptr[6+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch23:

 mov ds:byte ptr[6+edi],al

SkipTran23:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry10_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 8], 1
 jnz SkipTran24

 ;rj
 mov al,ds:byte ptr[7+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch24:

 mov ds:byte ptr[7+edi],al

SkipTran24:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry9_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 7], 1
 jnz SkipTran25

 ;rj
 mov al,ds:byte ptr[8+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch25:

 mov ds:byte ptr[8+edi],al

SkipTran25:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry8_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 6], 1
 jnz SkipTran26

 ;rj
 mov al,ds:byte ptr[9+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch26:

 mov ds:byte ptr[9+edi],al

SkipTran26:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry7_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 5], 1
 jnz SkipTran27

 ;rj
 mov al,ds:byte ptr[10+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch27:

 mov ds:byte ptr[10+edi],al

SkipTran27:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry6_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 4], 1
 jnz SkipTran28

 ;rj
 mov al,ds:byte ptr[11+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch28:

 mov ds:byte ptr[11+edi],al

SkipTran28:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry5_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 3], 1
 jnz SkipTran29

 ;rj
 mov al,ds:byte ptr[12+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch29:

 mov ds:byte ptr[12+edi],al

SkipTran29:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry4_16:
 sbb ecx,ecx

 cmp ds:byte ptr[_scanList + 2], 1
 jnz SkipTran30

 ;rj
 mov al,ds:byte ptr[13+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch30:

 mov ds:byte ptr[13+edi],al

SkipTran30:
 add ebx,ebp
 mov ah,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
LEntry3_16:

 cmp ds:byte ptr[_scanList + 1], 1
 jnz SkipTran31

 ;rj
 mov al,ds:byte ptr[14+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch31:

 mov ds:byte ptr[14+edi],al

SkipTran31:
 mov ah,ds:byte ptr[esi]
LEntry2_16:
LEndSpan:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 mov ebx,ds:dword ptr[pspantemp]
 mov ebx,ds:dword ptr[12+ebx]

 cmp ds:byte ptr[_scanList + 0], 1
 jnz SkipTran32

 ;rj
 mov al,ds:byte ptr[15+edi]
 and eax, 0ffffh
 mov al,ds:byte ptr[12345678h + eax]
TranPatch32:

 mov ds:byte ptr[15+edi],al

SkipTran32:
 test ebx,ebx
 jnz LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 public _D_Draw16EndT
_D_Draw16EndT:
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
 dd TranPatch12-4
 dd TranPatch13-4
 dd TranPatch14-4
 dd TranPatch15-4
 dd TranPatch16-4
 dd TranPatch17-4
 dd TranPatch18-4
 dd TranPatch19-4
 dd TranPatch20-4
 dd TranPatch21-4
 dd TranPatch22-4
 dd TranPatch23-4
 dd TranPatch24-4
 dd TranPatch25-4
 dd TranPatch26-4
 dd TranPatch27-4
 dd TranPatch28-4
 dd TranPatch29-4
 dd TranPatch30-4
 dd TranPatch31-4
 dd TranPatch32-4
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_TranPatch3
_R_TranPatch3:
 push ebx
 mov eax,ds:dword ptr[_mainTransTable]
 mov ebx,offset LPatchTable
 mov ecx,32
LPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret
_TEXT ENDS
 END
