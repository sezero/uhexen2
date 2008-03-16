;
; d_spr8.asm
; x86 assembly-language horizontal 8-bpp sprite span-drawing code.
;
; this file uses NASM syntax.
; $Id: d_spr8.asm,v 1.5 2008-03-16 00:00:22 sezero Exp $
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
 extern fp_1m
 extern fp_1m_minus_1
 extern fp_8
 extern fp_16
 extern fp_64k
 extern fp_64kx64k
 extern izi
 extern izistep
 extern sstep
 extern tstep
 extern advancetable
 extern spr8entryvec_table
 extern reciprocal_table
 extern reciprocal_table_16
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
 extern pz


SEGMENT .text

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

;;;;;;;;;;;;;;;;;;;;;;;;
; D_SpriteDrawSpans
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global D_SpriteDrawSpans
D_SpriteDrawSpans:
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
 fld  dword [d_zistepu]
 fmul  dword [fp_64kx64k]
 fxch st3
 fstp  dword [sdivz8stepu]
 fstp  dword [zi8stepu]
 fstp  dword [tdivz8stepu]
 fistp  dword [izistep]
 mov eax, dword [izistep]
 ror eax,16
 mov ecx, dword [8+ebx]
 mov  dword [izistep],eax
 cmp ecx,0
 jle near LNextSpan
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
 fld st0
 fmul  dword [fp_64kx64k]
 fxch st1
 fdiv st2,st0
 fxch st1
 fistp  dword [izi]
 mov ebp, dword [izi]
 ror ebp,16
 mov eax, dword [4+ebx]
 mov  dword [izi],ebp
 mov ebp, dword [0+ebx]
 imul  dword [d_zrowbytes]
 shl ebp,1
 add eax, dword [d_pzbuffer]
 add eax,ebp
 mov  dword [pz],eax
 mov ebp, dword [d_viewbuffer]
 mov eax, dword [4+ebx]
 push ebx
 mov edx, dword [tadjust]
 mov esi, dword [sadjust]
 mov edi, dword [d_scantable+eax*4]
 add edi,ebp
 mov ebp, dword [0+ebx]
 add edi,ebp
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
 add esi,ebx
 imul eax, dword [cachewidth]
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
 sub ecx,8
 mov ebp, dword [sadjust]
 push ecx
 mov ecx, dword [tadjust]
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
 sar edx,19
 mov ebx, dword [cachewidth]
 sar eax,19
 jz LIsZero
 imul eax,ebx
LIsZero:
 add eax,edx
 mov edx, dword [tfracf]
 mov  dword [advancetable+4],eax
 add eax,ebx
 shl ebp,13
 mov  dword [sstep],ebp
 mov ebx, dword [sfracf]
 shl ecx,13
 mov  dword [advancetable],eax
 mov  dword [tstep],ecx
 mov ecx, dword [pz]
 mov ebp, dword [izi]
 cmp bp, word [ecx]
 jl Lp1
 mov al, byte [esi]
 cmp al,255
 jz Lp1
 mov  word [ecx],bp
 mov  byte [edi],al
Lp1:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [2+ecx]
 jl Lp2
 mov al, byte [esi]
 cmp al,255
 jz Lp2
 mov  word [2+ecx],bp
 mov  byte [1+edi],al
Lp2:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [4+ecx]
 jl Lp3
 mov al, byte [esi]
 cmp al,255
 jz Lp3
 mov  word [4+ecx],bp
 mov  byte [2+edi],al
Lp3:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [6+ecx]
 jl Lp4
 mov al, byte [esi]
 cmp al,255
 jz Lp4
 mov  word [6+ecx],bp
 mov  byte [3+edi],al
Lp4:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [8+ecx]
 jl Lp5
 mov al, byte [esi]
 cmp al,255
 jz Lp5
 mov  word [8+ecx],bp
 mov  byte [4+edi],al
Lp5:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 pop eax
 cmp eax,8
 ja LSetupNotLast2
 dec eax
 jz LFDIVInFlight2
 mov  dword [spancountminus1],eax
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
 push eax
 cmp bp, word [10+ecx]
 jl Lp6
 mov al, byte [esi]
 cmp al,255
 jz Lp6
 mov  word [10+ecx],bp
 mov  byte [5+edi],al
Lp6:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [12+ecx]
 jl Lp7
 mov al, byte [esi]
 cmp al,255
 jz Lp7
 mov  word [12+ecx],bp
 mov  byte [6+edi],al
Lp7:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 cmp bp, word [14+ecx]
 jl Lp8
 mov al, byte [esi]
 cmp al,255
 jz Lp8
 mov  word [14+ecx],bp
 mov  byte [7+edi],al
Lp8:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
 add edi,8
 add ecx,16
 mov  dword [tfracf],edx
 mov edx, dword [snext]
 mov  dword [sfracf],ebx
 mov ebx, dword [tnext]
 mov  dword [s],edx
 mov  dword [t],ebx
 mov  dword [pz],ecx
 mov  dword [izi],ebp
 pop ecx
 cmp ecx,8
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
 mov ebx, dword [tadjust]
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
 mov ebx, dword [spr8entryvec_table+ecx*4]
 mov eax,edx
 push ebx
 mov ecx,ebp
 sar ecx,16
 mov ebx, dword [cachewidth]
 sar edx,16
 jz LIsZeroLast
 imul edx,ebx
LIsZeroLast:
 add edx,ecx
 mov ecx, dword [tfracf]
 mov  dword [advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx, dword [sfracf]
 shl eax,16
 mov  dword [advancetable],edx
 mov  dword [tstep],eax
 mov  dword [sstep],ebp
 mov edx,ecx
 mov ecx, dword [pz]
 mov ebp, dword [izi]
 ret
LNoSteps:
 mov ecx, dword [pz]
 sub edi,7
 sub ecx,14
 jmp LEndSpan
LOnlyOneStep:
 sub eax, dword [s]
 sub ebx, dword [t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec

;;;;;;;;;;;;;;;;;;;;;;;;
; globals Spr8Entry?_8
;;;;;;;;;;;;;;;;;;;;;;;;

 global Spr8Entry2_8
Spr8Entry2_8:
 sub edi,6
 sub ecx,12
 mov al, byte [esi]
 jmp LLEntry2_8

 global Spr8Entry3_8
Spr8Entry3_8:
 sub edi,5
 sub ecx,10
 jmp LLEntry3_8

 global Spr8Entry4_8
Spr8Entry4_8:
 sub edi,4
 sub ecx,8
 jmp LLEntry4_8

 global Spr8Entry5_8
Spr8Entry5_8:
 sub edi,3
 sub ecx,6
 jmp LLEntry5_8

 global Spr8Entry6_8
Spr8Entry6_8:
 sub edi,2
 sub ecx,4
 jmp LLEntry6_8

 global Spr8Entry7_8
Spr8Entry7_8:
 dec edi
 sub ecx,2
 jmp LLEntry7_8

 global Spr8Entry8_8
Spr8Entry8_8:
 cmp bp, word [ecx]
 jl Lp9
 mov al, byte [esi]
 cmp al,255
 jz Lp9
 mov  word [ecx],bp
 mov  byte [edi],al
Lp9:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry7_8:
 cmp bp, word [2+ecx]
 jl Lp10
 mov al, byte [esi]
 cmp al,255
 jz Lp10
 mov  word [2+ecx],bp
 mov  byte [1+edi],al
Lp10:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry6_8:
 cmp bp, word [4+ecx]
 jl Lp11
 mov al, byte [esi]
 cmp al,255
 jz Lp11
 mov  word [4+ecx],bp
 mov  byte [2+edi],al
Lp11:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry5_8:
 cmp bp, word [6+ecx]
 jl Lp12
 mov al, byte [esi]
 cmp al,255
 jz Lp12
 mov  word [6+ecx],bp
 mov  byte [3+edi],al
Lp12:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry4_8:
 cmp bp, word [8+ecx]
 jl Lp13
 mov al, byte [esi]
 cmp al,255
 jz Lp13
 mov  word [8+ecx],bp
 mov  byte [4+edi],al
Lp13:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry3_8:
 cmp bp, word [10+ecx]
 jl Lp14
 mov al, byte [esi]
 cmp al,255
 jz Lp14
 mov  word [10+ecx],bp
 mov  byte [5+edi],al
Lp14:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LLEntry2_8:
 cmp bp, word [12+ecx]
 jl Lp15
 mov al, byte [esi]
 cmp al,255
 jz Lp15
 mov  word [12+ecx],bp
 mov  byte [6+edi],al
Lp15:
 add ebp, dword [izistep]
 adc ebp,0
 add edx, dword [tstep]
 sbb eax,eax
 add ebx, dword [sstep]
 adc esi, dword [advancetable+4+eax*4]
LEndSpan:
 cmp bp, word [14+ecx]
 jl Lp16
 mov al, byte [esi]
 cmp al,255
 jz Lp16
 mov  word [14+ecx],bp
 mov  byte [7+edi],al
Lp16:
 fstp st0
 fstp st0
 fstp st0
 pop ebx
LNextSpan:
 add ebx,12
 mov ecx, dword [8+ebx]
 cmp ecx,0
 jg near LSpanLoop
 jz LNextSpan
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

