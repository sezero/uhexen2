;
; d_draw16.asm
; x86 assembly-language horizontal 8-bpp span-drawing code, with 16-pixel
; subdivision.
;
; this file uses NASM syntax.
; $Id: d_draw16.asm,v 1.5 2008-03-16 14:30:46 sezero Exp $
;

%include "asm_nasm.inc"

; underscore prefix handling
; for C-shared symbols:
%ifmacro _sym_prefix
; C-shared externs:
 _sym_prefix d_zistepu
 _sym_prefix d_pzbuffer
 _sym_prefix d_zistepv
 _sym_prefix d_zrowbytes
 _sym_prefix d_ziorigin
 _sym_prefix d_sdivzstepu
 _sym_prefix d_tdivzstepu
 _sym_prefix d_sdivzstepv
 _sym_prefix d_tdivzstepv
 _sym_prefix d_sdivzorigin
 _sym_prefix d_tdivzorigin
 _sym_prefix sadjust
 _sym_prefix tadjust
 _sym_prefix bbextents
 _sym_prefix bbextentt
 _sym_prefix cacheblock
 _sym_prefix d_viewbuffer
 _sym_prefix cachewidth
 _sym_prefix d_scantable
; C-shared globals:
 _sym_prefix D_DrawSpans16
%endif	; _sym_prefix

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

SEGMENT .data

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawSpans16
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

 ALIGN 4

 global D_DrawSpans16
D_DrawSpans16:
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
 mov bl, byte [esi]
 sub ecx,16
 mov ebp, dword [sadjust]
 mov  dword [counttemp],ecx
 mov ecx, dword [tadjust]
 mov  byte [edi],bl
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
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [7+edi],al
 add ebx,ebp
 mov al, byte [esi]
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
 mov  byte [8+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [9+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [10+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [11+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [12+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [13+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 sbb ecx,ecx
 mov  byte [14+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edi,16
 mov  dword [tfracf],edx
 mov edx, dword [snext]
 mov  dword [sfracf],ebx
 mov ebx, dword [tnext]
 mov  dword [s],edx
 mov  dword [t],ebx
 mov ecx, dword [counttemp]
 cmp ecx,16
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
 mov ebx, dword [entryvec_table_16+ecx*4]
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
 sub edi,15
 jmp LEndSpan
LOnlyOneStep:
 sub eax, dword [s]
 sub ebx, dword [t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec

;;;;;;;;;;;;;;;;;;;;;;;;
; globals Entry*_16
;;;;;;;;;;;;;;;;;;;;;;;;

 global Entry2_16, Entry3_16, Entry4_16, Entry5_16
 global Entry6_16, Entry7_16, Entry8_16, Entry9_16
 global Entry10_16, Entry11_16, Entry12_16, Entry13_16
 global Entry14_16, Entry15_16, Entry16_16

Entry2_16:
 sub edi,14
 mov al, byte [esi]
 jmp LEntry2_16
Entry3_16:
 sub edi,13
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 jmp LEntry3_16
Entry4_16:
 sub edi,12
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry4_16
Entry5_16:
 sub edi,11
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry5_16
Entry6_16:
 sub edi,10
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry6_16
Entry7_16:
 sub edi,9
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry7_16
Entry8_16:
 sub edi,8
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry8_16
Entry9_16:
 sub edi,7
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry9_16
Entry10_16:
 sub edi,6
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry10_16
Entry11_16:
 sub edi,5
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry11_16
Entry12_16:
 sub edi,4
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry12_16
Entry13_16:
 sub edi,3
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry13_16
Entry14_16:
 sub edi,2
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry14_16
Entry15_16:
 dec edi
 add edx,eax
 mov al, byte [esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
 jmp LEntry15_16
Entry16_16:
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
LEntry15_16:
 sbb ecx,ecx
 mov  byte [2+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry14_16:
 sbb ecx,ecx
 mov  byte [3+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry13_16:
 sbb ecx,ecx
 mov  byte [4+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry12_16:
 sbb ecx,ecx
 mov  byte [5+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry11_16:
 sbb ecx,ecx
 mov  byte [6+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry10_16:
 sbb ecx,ecx
 mov  byte [7+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry9_16:
 sbb ecx,ecx
 mov  byte [8+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry8_16:
 sbb ecx,ecx
 mov  byte [9+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry7_16:
 sbb ecx,ecx
 mov  byte [10+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry6_16:
 sbb ecx,ecx
 mov  byte [11+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry5_16:
 sbb ecx,ecx
 mov  byte [12+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
 add edx, dword [tstep]
LEntry4_16:
 sbb ecx,ecx
 mov  byte [13+edi],al
 add ebx,ebp
 mov al, byte [esi]
 adc esi, dword [advancetable+4+ecx*4]
LEntry3_16:
 mov  byte [14+edi],al
 mov al, byte [esi]
LEntry2_16:
LEndSpan:
 fstp st0
 fstp st0
 fstp st0
 mov ebx, dword [pspantemp]
 mov ebx, dword [12+ebx]
 test ebx,ebx
 mov  byte [15+edi],al
 jnz near LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

