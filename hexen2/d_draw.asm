;
; d_draw.asm
; x86 assembly-language horizontal 8-bpp span-drawing code.
;
; this file uses NASM syntax.
; $Id: d_draw.asm,v 1.4 2008-03-14 10:24:01 sezero Exp $
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
 extern entryvec_table_16
 extern fp_64kx64k
 extern scanList
 extern ZScanCount

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawSpans8
;;;;;;;;;;;;;;;;;;;;;;;;

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
 mov ebp,2048
 jmp LClampReentry2
LClampHigh2:
 mov ebp, dword [bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,2048
 jmp LClampReentry3
LClampHigh3:
 mov ecx, dword [bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,2048
 jmp LClampReentry4
LClampHigh4:
 mov eax, dword [bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,2048
 jmp LClampReentry5
LClampHigh5:
 mov ebx, dword [bbextentt]
 jmp LClampReentry5

 ALIGN 4

 global D_DrawSpans8
D_DrawSpans8:
 push ebp
 push edi
 push esi
 push ebx
 fld  dword [d_sdivzstepu]
 fmul  dword [fp_8]
 mov edx, dword [cacheblock]
 fld  dword [d_tdivzstepu]
 fmul  dword [fp_8]
 mov ebx, dword [4+16+esp]
 fld  dword [d_zistepu]
 fmul  dword [fp_8]
 mov  dword [pbase],edx
 fstp  dword [zi8stepu]
 fstp  dword [tdivz8stepu]
 fstp  dword [sdivz8stepu]
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
 mov edx, dword [tadjust]
 mov esi, dword [sadjust]
 mov edi, dword [d_scantable+eax*4]
 add edi,ecx
 mov ecx, dword [0+ebx]
 add edi,ecx
 mov ecx, dword [8+ebx]
 cmp ecx,8
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
 fadd  dword [zi8stepu]
 fxch st2
 fadd  dword [sdivz8stepu]
 fxch st2
 fld  dword [tdivz8stepu]
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
 cmp ecx,8
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
 mov bl, byte [esi]
 sub ecx,8
 mov ebp, dword [sadjust]
 mov  dword [counttemp],ecx
 mov ecx, dword [tadjust]
 mov  byte [edi],bl
 add ebp,eax
 add ecx,edx
 mov eax, dword [bbextents]
 mov edx, dword [bbextentt]
 cmp ebp,2048
 jl near LClampLow2
 cmp ebp,eax
 ja near LClampHigh2
LClampReentry2:
 cmp ecx,2048
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
 sar eax,19
 jz LZero
 sar edx,19
 mov ebx, dword [cachewidth]
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,19
 mov ebx, dword [cachewidth]
LSetUp1:
 add eax,edx
 mov edx, dword [tfracf]
 mov  dword [advancetable+4],eax
 add eax,ebx
 shl ebp,13
 mov ebx, dword [sfracf]
 shl ecx,13
 mov  dword [advancetable],eax
 mov  dword [tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov al, byte [esi]
 add ebx,ebp
 mov  byte [1+edi],al
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [2+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [3+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 mov ecx, dword [counttemp]
 cmp ecx,8
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
 fadd  dword [zi8stepu]
 fxch st2
 fadd  dword [sdivz8stepu]
 fxch st2
 fld  dword [tdivz8stepu]
 faddp st2,st0
 fld  dword [fp_64k]
 fdiv st0,st1
LFDIVInFlight2:
 mov  dword [counttemp],ecx
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [4+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [5+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [6+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edi,8
 mov  dword [tfracf],edx
 mov edx, dword [snext]
 mov  dword [sfracf],ebx
 mov ebx, dword [tnext]
 mov  dword [s],edx
 mov  dword [t],ebx
 mov ecx, dword [counttemp]
 cmp ecx,8
 mov  byte [-1+edi],al
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
 mov al, byte [esi]
 mov ebx, dword [tadjust]
 mov  byte [edi],al
 mov eax, dword [sadjust]
 add eax, dword [snext]
 add ebx, dword [tnext]
 mov ebp, dword [bbextents]
 mov edx, dword [bbextentt]
 cmp eax,2048
 jl near LClampLow4
 cmp eax,ebp
 ja near LClampHigh4
LClampReentry4:
 mov  dword [snext],eax
 cmp ebx,2048
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
 imul  dword [reciprocal_table-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul  dword [reciprocal_table-8+ecx*4]
LSetEntryvec:
 mov ebx, dword [entryvec_table+ecx*4]
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
 mov al, byte [esi]
 sub edi,7
 jmp LEndSpan
LOnlyOneStep:
 sub eax, dword [s]
 sub ebx, dword [t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec


;;;;;;;;;;;;;;;;;;;;;;;;
; globals Entry*_8
;;;;;;;;;;;;;;;;;;;;;;;;

 global Entry2_8
Entry2_8:
 sub edi,6
 mov al, byte [esi]
 jmp LLEntry2_8

 global Entry3_8
Entry3_8:
 sub edi,5
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 jmp LLEntry3_8

 global Entry4_8
Entry4_8:
 sub edi,4
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LLEntry4_8

 global Entry5_8
Entry5_8:
 sub edi,3
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LLEntry5_8

 global Entry6_8
Entry6_8:
 sub edi,2
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LLEntry6_8

 global Entry7_8
Entry7_8:
 dec edi
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LLEntry7_8

 global Entry8_8
Entry8_8:
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [1+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LLEntry7_8:
 sbb ecx,ecx
 mov  byte [2+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LLEntry6_8:
 sbb ecx,ecx
 mov  byte [3+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LLEntry5_8:
 sbb ecx,ecx
 mov  byte [4+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LLEntry4_8:
 sbb ecx,ecx
 mov  byte [5+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
LLEntry3_8:
 mov  byte [6+edi],al
 mov al, byte [esi]
LLEntry2_8:
LEndSpan:
 fstp st0
 fstp st0
 fstp st0
 mov ebx, dword [pspantemp]
 mov ebx, dword [12+ebx]
 test ebx,ebx
 mov  byte [7+edi],al
 jnz near LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawZSpans
;;;;;;;;;;;;;;;;;;;;;;;;

LClamp:
 mov edx,040000000h
 xor ebx,ebx
 fstp st0
 jmp LZDraw
LClampNeg:
 mov edx,040000000h
 xor ebx,ebx
 fstp st0
 jmp LZDrawNeg

 global D_DrawZSpans
D_DrawZSpans:
 push ebp
 push edi
 push esi
 push ebx
 fld  dword [d_zistepu]
 mov eax, dword [d_zistepu]
 mov esi, dword [4+16+esp]
 test eax,eax
 jz near LFNegSpan
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izistep]
 mov ebx, dword [izistep]
LFSpanLoop:
 fild  dword [4+esi]
 fild  dword [0+esi]
 mov ecx, dword [4+esi]
 mov edi, dword [d_pzbuffer]
 fmul  dword [d_zistepu]
 fxch st1
 fmul  dword [d_zistepv]
 fxch st1
 fadd  dword [d_ziorigin]
 imul ecx, dword [d_zrowbytes]
 faddp st1,st0
 fcom  dword [float_point5]
 add edi,ecx
 mov edx, dword [0+esi]
 add edx,edx
 mov ecx, dword [8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz near LClamp
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izi]
 mov edx, dword [izi]
LZDraw:
 test edi,2
 jz LFMiddle
 mov eax,edx
 add edx,ebx
 shr eax,16
 dec ecx
 mov  word [edi],ax
 add edi,2
LFMiddle:
 push ecx
 shr ecx,1
 jz LFLast
 shr ecx,1
 jnc LFMiddleLoop
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov  dword [edi],eax
 add edi,4
 and ecx,ecx
 jz LFLast
LFMiddleLoop:
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov  dword [edi],eax
 add edx,ebx
 shr ebp,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov  dword [4+edi],ebp
 add edi,8
 dec ecx
 jnz LFMiddleLoop
LFLast:
 pop ecx
 pop esi
 and ecx,1
 jz LFSpanDone
 shr edx,16
 mov  word [edi],dx
LFSpanDone:
 mov esi, dword [12+esi]
 test esi,esi
 jnz near LFSpanLoop
 jmp LFDone
LFNegSpan:
 fmul  dword [FloatMinus2ToThe31nd]
 fistp  dword [izistep]
 mov ebx, dword [izistep]
LFNegSpanLoop:
 fild  dword [4+esi]
 fild  dword [0+esi]
 mov ecx, dword [4+esi]
 mov edi, dword [d_pzbuffer]
 fmul  dword [d_zistepu]
 fxch st1
 fmul  dword [d_zistepv]
 fxch st1
 fadd  dword [d_ziorigin]
 imul ecx, dword [d_zrowbytes]
 faddp st1,st0
 fcom  dword [float_point5]
 add edi,ecx
 mov edx, dword [0+esi]
 add edx,edx
 mov ecx, dword [8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz near LClampNeg
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izi]
 mov edx, dword [izi]
LZDrawNeg:
 test edi,2
 jz LFNegMiddle
 mov eax,edx
 sub edx,ebx
 shr eax,16
 dec ecx
 mov  word [edi],ax
 add edi,2
LFNegMiddle:
 push ecx
 shr ecx,1
 jz LFNegLast
 shr ecx,1
 jnc LFNegMiddleLoop
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov  dword [edi],eax
 add edi,4
 and ecx,ecx
 jz LFNegLast
LFNegMiddleLoop:
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov  dword [edi],eax
 sub edx,ebx
 shr ebp,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov  dword [4+edi],ebp
 add edi,8
 dec ecx
 jnz LFNegMiddleLoop
LFNegLast:
 pop ecx
 pop esi
 and ecx,1
 jz LFNegSpanDone
 shr edx,16
 mov  word [edi],dx
LFNegSpanDone:
 mov esi, dword [12+esi]
 test esi,esi
 jnz near LFNegSpanLoop
LFDone:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawSingleZSpans
;;;;;;;;;;;;;;;;;;;;;;;;

LClamp2:
 mov edx,040000000h
 xor ebx,ebx
 fstp st0
 jmp LZDraw2
LClampNeg2:
 mov edx,040000000h
 xor ebx,ebx
 fstp st0
 jmp LZDrawNeg2

 global D_DrawSingleZSpans
D_DrawSingleZSpans:
 push ebp
 push edi
 push esi
 push ebx
 mov  dword [ZScanCount],0
 fld  dword [d_zistepu]
 mov eax, dword [d_zistepu]
 mov esi, dword [4+16+esp]
 test eax,eax
 jz near LFNegSpan2
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izistep]
 mov ebx, dword [izistep]
LFSpanLoop2:
 fild  dword [4+esi]
 fild  dword [0+esi]
 mov ecx, dword [4+esi]
 mov edi, dword [d_pzbuffer]
 fmul  dword [d_zistepu]
 fxch st1
 fmul  dword [d_zistepv]
 fxch st1
 fadd  dword [d_ziorigin]
 imul ecx, dword [d_zrowbytes]
 faddp st1,st0
 fcom  dword [float_point5]
 add edi,ecx
 mov edx, dword [0+esi]
 add edx,edx
 mov ecx, dword [8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz near LClamp2
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izi]
 mov edx, dword [izi]
LZDraw2:
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov  byte [scanList + ecx - 1], 1
 cmp  word [edi],ax
 jle LZSkip
 mov  byte [scanList + ecx - 1], 0
 add  dword [ZScanCount],1
 LZSkip:
 add edi,2
 dec ecx
 jnz LZDraw2

 pop esi
 jmp LFDone2

LFNegSpan2:
 fmul  dword [FloatMinus2ToThe31nd]
 fistp  dword [izistep]
 mov ebx, dword [izistep]
LFNegSpanLoop2:
 fild  dword [4+esi]
 fild  dword [0+esi]
 mov ecx, dword [4+esi]
 mov edi, dword [d_pzbuffer]
 fmul  dword [d_zistepu]
 fxch st1
 fmul  dword [d_zistepv]
 fxch st1
 fadd  dword [d_ziorigin]
 imul ecx, dword [d_zrowbytes]
 faddp st1,st0
 fcom  dword [float_point5]
 add edi,ecx
 mov edx, dword [0+esi]
 add edx,edx
 mov ecx, dword [8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz near LClampNeg2
 fmul  dword [Float2ToThe31nd]
 fistp  dword [izi]
 mov edx, dword [izi]
LZDrawNeg2:
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov  byte [scanList + ecx - 1], 1
 cmp  word [edi],ax
 jle LZSkip2
 mov  byte [scanList + ecx - 1], 0
 LZSkip2:
 add edi,2
 dec ecx
 jnz near LZDraw2

 pop esi

LFDone2:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

