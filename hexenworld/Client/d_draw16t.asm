;
; d_draw16t.asm
; x86 assembly-language horizontal 8-bpp span-drawing code, with 16-pixel
; subdivision and translucency handling.
;
; this file uses NASM syntax.
; $Id: d_draw16t.asm,v 1.7 2008-03-14 12:01:24 sezero Exp $
;

%idefine offset

; externs from C code
 extern d_zistepu
 extern d_pzbuffer
 extern d_zistepv
 extern d_zrowbytes
 extern d_ziorigin
 extern d_sdivzstepu
 extern d_tdivzstepu
 extern d_sdivzstepv
 extern d_tdivzstepv
 extern d_sdivzorigin
 extern d_tdivzorigin
 extern sadjust
 extern tadjust
 extern bbextents
 extern bbextentt
 extern cacheblock
 extern d_viewbuffer
 extern cachewidth
 extern d_scantable
 extern mainTransTable
 extern scanList
 extern D_DrawSingleZSpans

; externs from ASM-only code
 extern float_point5
 extern Float2ToThe31nd
 extern izistep
 extern izi
 extern FloatMinus2ToThe31nd
 extern float_1
 extern float_particle_z_clip
 extern float_minus_1
 extern float_0
 extern fp_16
 extern fp_64k
 extern fp_1m
 extern fp_1m_minus_1
 extern fp_8
 extern entryvec_table
 extern advancetable
 extern sstep
 extern tstep
 extern pspantemp
 extern counttemp
 extern jumptemp
 extern reciprocal_table
 extern pbase
 extern s
 extern t
 extern sfracf
 extern tfracf
 extern snext
 extern tnext
 extern spancountminus1
 extern zi16stepu
 extern sdivz16stepu
 extern tdivz16stepu
 extern zi8stepu
 extern sdivz8stepu
 extern tdivz8stepu
 extern reciprocal_table_16
 extern entryvec_table_16T
 extern fp_64kx64k

SEGMENT .data
masktemp dw 0

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; D_Draw16StartT
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_Draw16StartT
D_Draw16StartT:
LClampHigh0:
 mov esi, dword [bbextents]
 jmp LClampReentry0
LClampHighOrLow0:
 jg LClampHigh0
 xor esi,esi
 jmp LClampReentry0
LClampHigh1:
 mov edx, dword [bbextentt]
 jmp LClampReentry1
LClampHighOrLow1:
 jg LClampHigh1
 xor edx,edx
 jmp LClampReentry1
LClampLow2:
 mov ebp,4096
 jmp LClampReentry2
LClampHigh2:
 mov ebp, dword [bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,4096
 jmp LClampReentry3
LClampHigh3:
 mov ecx, dword [bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,4096
 jmp LClampReentry4
LClampHigh4:
 mov eax, dword [bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,4096
 jmp LClampReentry5
LClampHigh5:
 mov ebx, dword [bbextentt]
 jmp LClampReentry5


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawSpans16T
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global D_DrawSpans16T
D_DrawSpans16T:
 push ebp
 push edi
 push esi
 push ebx
 fld  dword [d_sdivzstepu]
 fmul  dword [fp_16]
 mov edx, dword [cacheblock]
 fld  dword [d_tdivzstepu]
 fmul  dword [fp_16]
 mov ebx, dword [4+16+esp]
 fld  dword [d_zistepu]
 fmul  dword [fp_16]
 mov  dword [pbase],edx
 fstp  dword [zi16stepu]
 fstp  dword [tdivz16stepu]
 fstp  dword [sdivz16stepu]
LSpanLoop:
 fild  dword [4+ebx]
 fild  dword [0+ebx]
 fld st1
 fmul  dword [d_sdivzstepv]
 fld st1
 fmul  dword [d_sdivzstepu]
 fld st2
 fmul  dword [d_tdivzstepu]
 fxch st1
 faddp st2,st0
 fxch st1
 fld st3
 fmul  dword [d_tdivzstepv]
 fxch st1
 fadd  dword [d_sdivzorigin]
 fxch st4
 fmul  dword [d_zistepv]
 fxch st1
 faddp st2,st0
 fxch st2
 fmul  dword [d_zistepu]
 fxch st1
 fadd  dword [d_tdivzorigin]
 fxch st2
 faddp st1,st0
 fld  dword [fp_64k]
 fxch st1
 fadd  dword [d_ziorigin]
 fdiv st1,st0
 mov ecx, dword [d_viewbuffer]
 mov eax, dword [4+ebx]
 mov  dword [pspantemp],ebx

 push eax
 push ecx
 push edx
 push ebx

 call D_DrawSingleZSpans		; call near D_DrawSingleZSpans

 pop ebx
 pop edx
 pop ecx
 pop eax

 mov edx, dword [tadjust]
 mov esi, dword [sadjust]
 mov edi, dword [d_scantable+eax*4]
 add edi,ecx
 mov ecx, dword [0+ebx]
 add edi,ecx
 mov ecx, dword [8+ebx]
 cmp ecx,16
 ja LSetupNotLast1
 dec ecx
 jz LCleanup1
 mov  dword [spancountminus1],ecx
 fxch st1
 fld st0
 fmul st0,st4
 fxch st1
 fmul st0,st3
 fxch st1
 fistp  dword [s]
 fistp  dword [t]
 fild  dword [spancountminus1]
 fld  dword [d_tdivzstepu]
 fld  dword [d_zistepu]
 fmul st0,st2
 fxch st1
 fmul st0,st2
 fxch st2
 fmul  dword [d_sdivzstepu]
 fxch st1
 faddp st3,st0
 fxch st1
 faddp st3,st0
 faddp st3,st0
 fld  dword [fp_64k]
 fdiv st0,st1
 jmp LFDIVInFlight1
LCleanup1:
 fxch st1
 fld st0
 fmul st0,st4
 fxch st1
 fmul st0,st3
 fxch st1
 fistp  dword [s]
 fistp  dword [t]
 jmp LFDIVInFlight1

 ALIGN 4

LSetupNotLast1:
 fxch st1
 fld st0
 fmul st0,st4
 fxch st1
 fmul st0,st3
 fxch st1
 fistp  dword [s]
 fistp  dword [t]
 fadd  dword [zi16stepu]
 fxch st2
 fadd  dword [sdivz16stepu]
 fxch st2
 fld  dword [tdivz16stepu]
 faddp st2,st0
 fld  dword [fp_64k]
 fdiv st0,st1
LFDIVInFlight1:
 add esi, dword [s]
 add edx, dword [t]
 mov ebx, dword [bbextents]
 mov ebp, dword [bbextentt]
 cmp esi,ebx
 ja near LClampHighOrLow0
LClampReentry0:
 mov  dword [s],esi
 mov ebx, dword [pbase]
 shl esi,16
 cmp edx,ebp
 mov  dword [sfracf],esi
 ja near LClampHighOrLow1
LClampReentry1:
 mov  dword [t],edx
 mov esi, dword [s]
 shl edx,16
 mov eax, dword [t]
 sar esi,16
 mov  dword [tfracf],edx
 sar eax,16
 mov edx, dword [cachewidth]
 imul eax,edx
 add esi,ebx
 add esi,eax
 cmp ecx,16
 jna near LLastSegment
LNotLastSegment:
 fld st0
 fmul st0,st4
 fxch st1
 fmul st0,st3
 fxch st1
 fistp  dword [snext]
 fistp  dword [tnext]
 mov eax, dword [snext]
 mov edx, dword [tnext]

 xor ebx,ebx
 add bl, byte [scanList + ecx - 1]
 shl ebx,1
 add bl, byte [scanList + ecx - 2]
 shl ebx,1
 add bl, byte [scanList + ecx - 3]
 shl ebx,1
 add bl, byte [scanList + ecx - 4]
 shl ebx,1
 add bl, byte [scanList + ecx - 5]
 shl ebx,1
 add bl, byte [scanList + ecx - 6]
 shl ebx,1
 add bl, byte [scanList + ecx - 7]
 shl ebx,1
 add bl, byte [scanList + ecx - 8]
 shl ebx,1
 add bl, byte [scanList + ecx - 9]
 shl ebx,1
 add bl, byte [scanList + ecx - 10]
 shl ebx,1
 add bl, byte [scanList + ecx - 11]
 shl ebx,1
 add bl, byte [scanList + ecx - 12]
 shl ebx,1
 add bl, byte [scanList + ecx - 13]
 shl ebx,1
 add bl, byte [scanList + ecx - 14]
 shl ebx,1
 add bl, byte [scanList + ecx - 15]
 shl ebx,1
 add bl, byte [scanList + ecx - 16]
 ;mov bx, 8000h
; mov bx, 0ffffh
; mov bx, 0h
 mov [masktemp], bx

 mov bh, byte [esi]
 sub ecx,16
 mov ebp, dword [sadjust]
 mov  dword [counttemp],ecx
 mov ecx, dword [tadjust]

 ;and masktemp, 8000h
 bt word [masktemp], 15
 jnc SkipTran1

 ;rj
 mov bl, byte [edi]
 and ebx, 0ffffh
 mov bl, byte [12345678h + ebx]
TranPatch1:  

 mov  byte [edi],bl

SkipTran1:
 add ebp,eax
 add ecx,edx
 mov eax, dword [bbextents]
 mov edx, dword [bbextentt]
 cmp ebp,4096
 jl near LClampLow2
 cmp ebp,eax
 ja near LClampHigh2
LClampReentry2:
 cmp ecx,4096
 jl near LClampLow3
 cmp ecx,edx
 ja near LClampHigh3
LClampReentry3:
 mov  dword [snext],ebp
 mov  dword [tnext],ecx
 sub ebp, dword [s]
 sub ecx, dword [t]
 mov eax,ecx
 mov edx,ebp
 sar eax,20
 jz LZero
 sar edx,20
 mov ebx, dword [cachewidth]
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,20
 mov ebx, dword [cachewidth]
LSetUp1:
 add eax,edx
 mov edx, dword [tfracf]
 mov  dword [advancetable+4],eax
 add eax,ebx
 shl ebp,12
 mov ebx, dword [sfracf]
 shl ecx,12
 mov  dword [advancetable],eax
 mov  dword [tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 mov ah, byte [esi]

; and masktemp, 4000h
 bt word [masktemp], 14
 jnc SkipTran2

;rj
 mov al, byte [1+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch2:

 mov  byte [1+edi],al

SkipTran2:
 add edx, dword [tstep]
 sbb ecx,ecx
 add ebx,ebp

 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov ah, byte [esi]

; and masktemp, 2000h
 bt word [masktemp], 13
 jnc SkipTran3

;rj
 mov al, byte [2+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch3:

 mov  byte [2+edi],al

SkipTran3:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

; and masktemp, 1000h
 bt word [masktemp], 12
 jnc SkipTran4

;rj
 mov al, byte [3+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch4:

 mov  byte [3+edi],al

SkipTran4:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 
 mov ah, byte [esi]

; and masktemp, 0800h
 bt word [masktemp], 11
 jnc SkipTran5

;rj
 mov al, byte [4+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch5:

 mov  byte [4+edi],al

SkipTran5:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

; and masktemp, 0400h
 bt word [masktemp], 10
 jnc SkipTran6

;rj
 mov al, byte [5+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch6:

 mov  byte [5+edi],al

SkipTran6:
; rj speed test
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

; and masktemp, 0200h
 bt word [masktemp], 9
 jnc SkipTran7

;rj
 mov al, byte [6+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch7:

 mov  byte [6+edi],al



; add ebx,ebp
; adc esi, dword [advancetable+4+ecx*4]
; xor eax, eax
; add edx, dword [tstep]
; mov al, byte [6+edi]
; sbb ecx,ecx
; mov ah, byte [esi]

;rj
; add eax, dword [mainTransTable]
; mov al, byte [eax + mainTransTable]

; mov  byte [6+edi],al













SkipTran7:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

; and masktemp, 0100h
 bt word [masktemp], 8
 jnc SkipTran8

;rj
 mov al, byte [7+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch8:

 mov  byte [7+edi],al

SkipTran8:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 mov ecx, dword [counttemp]
 cmp ecx,16
 ja LSetupNotLast2
 dec ecx
 jz LFDIVInFlight2
 mov  dword [spancountminus1],ecx
 fild  dword [spancountminus1]
 fld  dword [d_zistepu]
 fmul st0,st1
 fld  dword [d_tdivzstepu]
 fmul st0,st2
 fxch st1
 faddp st3,st0
 fxch st1
 fmul  dword [d_sdivzstepu]
 fxch st1
 faddp st3,st0
 fld  dword [fp_64k]
 fxch st1
 faddp st4,st0
 fdiv st0,st1
 jmp LFDIVInFlight2

 ALIGN 4

LSetupNotLast2:
 fadd  dword [zi16stepu]
 fxch st2
 fadd  dword [sdivz16stepu]
 fxch st2
 fld  dword [tdivz16stepu]
 faddp st2,st0
 fld  dword [fp_64k]
 fdiv st0,st1
LFDIVInFlight2:
 mov  dword [counttemp],ecx
 add edx, dword [tstep]
 sbb ecx,ecx

 bt word [masktemp], 7
 jnc SkipTran9

;rj
 mov al, byte [8+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch9:

 mov  byte [8+edi],al

SkipTran9:
 mov ah, byte [esi]

 bt word [masktemp], 6
 jnc SkipTran10

 ;rj
 mov al, byte [9+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch10:

 mov  byte [9+edi],al

SkipTran10:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

 bt word [masktemp], 5
 jnc SkipTran11

 ;rj
 mov al, byte [10+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch11:

 mov  byte [10+edi],al

SkipTran11:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

 bt word [masktemp], 4
 jnc SkipTran12

 ;rj
 mov al, byte [11+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch12:

 mov  byte [11+edi],al

SkipTran12:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

 bt word [masktemp], 3
 jnc SkipTran13

 ;rj
 mov al, byte [12+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch13:

 mov  byte [12+edi],al

SkipTran13:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

 bt word [masktemp], 2
 jnc SkipTran14

 ;rj
 mov al, byte [13+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch14:

 mov  byte [13+edi],al

SkipTran14:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 mov ah, byte [esi]

 bt word [masktemp], 1
 jnc SkipTran15

 ;rj
 mov al, byte [14+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch15:

 mov  byte [14+edi],al

SkipTran15:
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edi,16
 mov  dword [tfracf],edx
 mov edx, dword [snext]
 mov  dword [sfracf],ebx
 mov ebx, dword [tnext]
 mov  dword [s],edx
 mov  dword [t],ebx
 mov ecx, dword [counttemp]


 bt word [masktemp], 0
 jnc SkipTran16


 ;rj
 mov al, byte [-1+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch16:

 mov  byte [-1+edi],al

SkipTran16:
 cmp ecx,16
 ja near LNotLastSegment
LLastSegment:
 test ecx,ecx
 jz near LNoSteps
 fld st0
 fmul st0,st4
 fxch st1
 fmul st0,st3
 fxch st1
 fistp  dword [snext]
 fistp  dword [tnext]
 mov ah, byte [esi]
 mov ebx, dword [tadjust]
 

 cmp  byte [scanList + ecx - 1], 1
 jnz SkipTran17


 ;rj
 mov al, byte [edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch17:

 mov  byte [edi],al

SkipTran17:
 mov eax, dword [sadjust]
 add eax, dword [snext]
 add ebx, dword [tnext]
 mov ebp, dword [bbextents]
 mov edx, dword [bbextentt]
 cmp eax,4096
 jl near LClampLow4
 cmp eax,ebp
 ja near LClampHigh4
LClampReentry4:
 mov  dword [snext],eax
 cmp ebx,4096
 jl near LClampLow5
 cmp ebx,edx
 ja near LClampHigh5
LClampReentry5:
 cmp ecx,1
 je near LOnlyOneStep
 sub eax, dword [s]
 sub ebx, dword [t]
 add eax,eax
 add ebx,ebx
 imul  dword [reciprocal_table_16-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul  dword [reciprocal_table_16-8+ecx*4]
LSetEntryvec:
 mov ebx, dword [entryvec_table_16T+ecx*4]
 mov eax,edx
 mov  dword [jumptemp],ebx
 mov ecx,ebp
 sar edx,16
 mov ebx, dword [cachewidth]
 sar ecx,16
 imul edx,ebx
 add edx,ecx
 mov ecx, dword [tfracf]
 mov  dword [advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx, dword [sfracf]
 shl eax,16
 mov  dword [advancetable],edx
 mov  dword [tstep],eax
 mov edx,ecx
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 jmp dword[jumptemp]
LNoSteps:
 mov ah, byte [esi]
 sub edi,15
 jmp LEndSpan
LOnlyOneStep:
 sub eax, dword [s]
 sub ebx, dword [t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec

;;;;;;;;;;;;;;;;;;;;;;;;
; globals Entry*_16T
;;;;;;;;;;;;;;;;;;;;;;;;

 global Entry2_16T, Entry3_16T, Entry4_16T, Entry5_16T
 global Entry6_16T, Entry7_16T, Entry8_16T, Entry9_16T
 global Entry10_16T, Entry11_16T, Entry12_16T, Entry13_16T
 global Entry14_16T, Entry15_16T, Entry16_16T

Entry2_16T:
 sub edi,14
 mov ah, byte [esi]
 jmp LEntry2_16
Entry3_16T:
 sub edi,13
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 jmp LEntry3_16
Entry4_16T:
 sub edi,12
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry4_16
Entry5_16T:
 sub edi,11
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry5_16
Entry6_16T:
 sub edi,10
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry6_16
Entry7_16T:
 sub edi,9
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry7_16
Entry8_16T:
 sub edi,8
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry8_16
Entry9_16T:
 sub edi,7
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry9_16
Entry10_16T:
 sub edi,6
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry10_16
Entry11_16T:
 sub edi,5
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry11_16
Entry12_16T:
 sub edi,4
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry12_16
Entry13_16T:
 sub edi,3
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry13_16
Entry14_16T:
 sub edi,2
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry14_16
Entry15_16T:
 dec edi
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry15_16
Entry16_16T:
 add edx,eax
 mov ah, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx

 cmp  byte [scanList + 14], 1
 jnz SkipTran18

 ;rj
 mov al, byte [1+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch18:

 mov  byte [1+edi],al

SkipTran18:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry15_16:
 sbb ecx,ecx

 cmp  byte [scanList + 13], 1
 jnz SkipTran19

 ;rj
 mov al, byte [2+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch19:

 mov  byte [2+edi],al

SkipTran19:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry14_16:
 sbb ecx,ecx

 cmp  byte [scanList + 12], 1
 jnz SkipTran20

 ;rj
 mov al, byte [3+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch20:

 mov  byte [3+edi],al

SkipTran20:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry13_16:
 sbb ecx,ecx


 cmp  byte [scanList + 11], 1
 jnz SkipTran21


 ;rj
 mov al, byte [4+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch21:

 mov  byte [4+edi],al

SkipTran21:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry12_16:
 sbb ecx,ecx

 cmp  byte [scanList + 10], 1
 jnz SkipTran22

 ;rj
 mov al, byte [5+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch22:

 mov  byte [5+edi],al

SkipTran22:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry11_16:
 sbb ecx,ecx

 cmp  byte [scanList + 9], 1
 jnz SkipTran23

 ;rj
 mov al, byte [6+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch23:

 mov  byte [6+edi],al

SkipTran23:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry10_16:
 sbb ecx,ecx

 cmp  byte [scanList + 8], 1
 jnz SkipTran24

 ;rj
 mov al, byte [7+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch24:

 mov  byte [7+edi],al

SkipTran24:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry9_16:
 sbb ecx,ecx

 cmp  byte [scanList + 7], 1
 jnz SkipTran25

 ;rj
 mov al, byte [8+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch25:

 mov  byte [8+edi],al

SkipTran25:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry8_16:
 sbb ecx,ecx

 cmp  byte [scanList + 6], 1
 jnz SkipTran26

 ;rj
 mov al, byte [9+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch26:

 mov  byte [9+edi],al

SkipTran26:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry7_16:
 sbb ecx,ecx

 cmp  byte [scanList + 5], 1
 jnz SkipTran27

 ;rj
 mov al, byte [10+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch27:

 mov  byte [10+edi],al

SkipTran27:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry6_16:
 sbb ecx,ecx

 cmp  byte [scanList + 4], 1
 jnz SkipTran28

 ;rj
 mov al, byte [11+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch28:

 mov  byte [11+edi],al

SkipTran28:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry5_16:
 sbb ecx,ecx

 cmp  byte [scanList + 3], 1
 jnz SkipTran29

 ;rj
 mov al, byte [12+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch29:

 mov  byte [12+edi],al

SkipTran29:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry4_16:
 sbb ecx,ecx

 cmp  byte [scanList + 2], 1
 jnz SkipTran30

 ;rj
 mov al, byte [13+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch30:

 mov  byte [13+edi],al

SkipTran30:
 add ebx,ebp
 mov ah, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
LEntry3_16:

 cmp  byte [scanList + 1], 1
 jnz SkipTran31

 ;rj
 mov al, byte [14+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch31:

 mov  byte [14+edi],al

SkipTran31:
 mov ah, byte [esi]
LEntry2_16:
LEndSpan:
 fstp st0
 fstp st0
 fstp st0
 mov ebx, dword [pspantemp]
 mov ebx, dword [12+ebx]

 cmp  byte [scanList + 0], 1
 jnz SkipTran32

 ;rj
 mov al, byte [15+edi]
 and eax, 0ffffh
 mov al, byte [12345678h + eax]
TranPatch32:

 mov  byte [15+edi],al

SkipTran32:
 test ebx,ebx
 jnz near LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_Draw16EndT
;;;;;;;;;;;;;;;;;;;;;;;;

 global D_Draw16EndT
D_Draw16EndT:

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


;;;;;;;;;;;;;;;;;;;;;;;;
; R_TranPatch3
;;;;;;;;;;;;;;;;;;;;;;;;

SEGMENT .text

 ALIGN 4

 global R_TranPatch3
R_TranPatch3:
 push ebx
 mov eax, dword [mainTransTable]
 mov ebx,offset LPatchTable
 mov ecx,32
LPatchLoop:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret

