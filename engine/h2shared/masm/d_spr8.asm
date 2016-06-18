;
; d_spr8.asm -- for MASM.
; x86 assembly-language horizontal 8-bpp sprite span-drawing code.
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

; externs from ASM-only code
 externdef fp_1m:dword
 externdef fp_1m_minus_1:dword
 externdef fp_8:dword
 externdef fp_16:dword
 externdef fp_64k:dword
 externdef fp_64kx64k:dword
 externdef izi:dword
 externdef izistep:dword
 externdef sstep:dword
 externdef tstep:dword
 externdef advancetable:dword
 externdef spr8entryvec_table:dword
 externdef reciprocal_table:dword
 externdef reciprocal_table_16:dword
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
 externdef pz:dword

_TEXT SEGMENT
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
 mov ebp,2048
 jmp LClampReentry2
LClampHigh2:
 mov ebp,ds:dword ptr[_bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,2048
 jmp LClampReentry3
LClampHigh3:
 mov ecx,ds:dword ptr[_bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,2048
 jmp LClampReentry4
LClampHigh4:
 mov eax,ds:dword ptr[_bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,2048
 jmp LClampReentry5
LClampHigh5:
 mov ebx,ds:dword ptr[_bbextentt]
 jmp LClampReentry5
 align 4
 public _D_SpriteDrawSpans
_D_SpriteDrawSpans:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_sdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov edx,ds:dword ptr[_cacheblock]
 fld ds:dword ptr[_d_tdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov ebx,ds:dword ptr[4+16+esp]
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_8]
 mov ds:dword ptr[pbase],edx
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_64kx64k]
 fxch st(3)
 fstp ds:dword ptr[sdivz8stepu]
 fstp ds:dword ptr[zi8stepu]
 fstp ds:dword ptr[tdivz8stepu]
 fistp ds:dword ptr[izistep]
 mov eax,ds:dword ptr[izistep]
 ror eax,16
 mov ecx,ds:dword ptr[8+ebx]
 mov ds:dword ptr[izistep],eax
 cmp ecx,0
 jle LNextSpan
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
 fld st(0)
 fmul ds:dword ptr[fp_64kx64k]
 fxch st(1)
 fdiv st(2),st(0)
 fxch st(1)
 fistp ds:dword ptr[izi]
 mov ebp,ds:dword ptr[izi]
 ror ebp,16
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[izi],ebp
 mov ebp,ds:dword ptr[0+ebx]
 imul ds:dword ptr[_d_zrowbytes]
 shl ebp,1
 add eax,ds:dword ptr[_d_pzbuffer]
 add eax,ebp
 mov ds:dword ptr[pz],eax
 mov ebp,ds:dword ptr[_d_viewbuffer]
 mov eax,ds:dword ptr[4+ebx]
 push ebx
 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov edi,ds:dword ptr[_d_scantable+eax*4]
 add edi,ebp
 mov ebp,ds:dword ptr[0+ebx]
 add edi,ebp
 cmp ecx,8
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
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
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
 add esi,ebx
 imul eax,ds:dword ptr[_cachewidth]
 add esi,eax
 cmp ecx,8
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
 sub ecx,8
 mov ebp,ds:dword ptr[_sadjust]
 push ecx
 mov ecx,ds:dword ptr[_tadjust]
 add ebp,eax
 add ecx,edx
 mov eax,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp ebp,2048
 jl LClampLow2
 cmp ebp,eax
 ja LClampHigh2
LClampReentry2:
 cmp ecx,2048
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
 sar edx,19
 mov ebx,ds:dword ptr[_cachewidth]
 sar eax,19
 jz LIsZero
 imul eax,ebx
LIsZero:
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],eax
 add eax,ebx
 shl ebp,13
 mov ds:dword ptr[sstep],ebp
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,13
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[tstep],ecx
 mov ecx,ds:dword ptr[pz]
 mov ebp,ds:dword ptr[izi]
 cmp bp,ds:word ptr[ecx]
 jl Lp1
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp1
 mov ds:word ptr[ecx],bp
 mov ds:byte ptr[edi],al
Lp1:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[2+ecx]
 jl Lp2
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp2
 mov ds:word ptr[2+ecx],bp
 mov ds:byte ptr[1+edi],al
Lp2:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[4+ecx]
 jl Lp3
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp3
 mov ds:word ptr[4+ecx],bp
 mov ds:byte ptr[2+edi],al
Lp3:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[6+ecx]
 jl Lp4
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp4
 mov ds:word ptr[6+ecx],bp
 mov ds:byte ptr[3+edi],al
Lp4:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[8+ecx]
 jl Lp5
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp5
 mov ds:word ptr[8+ecx],bp
 mov ds:byte ptr[4+edi],al
Lp5:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 pop eax
 cmp eax,8
 ja LSetupNotLast2
 dec eax
 jz LFDIVInFlight2
 mov ds:dword ptr[spancountminus1],eax
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
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight2:
 push eax
 cmp bp,ds:word ptr[10+ecx]
 jl Lp6
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp6
 mov ds:word ptr[10+ecx],bp
 mov ds:byte ptr[5+edi],al
Lp6:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[12+ecx]
 jl Lp7
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp7
 mov ds:word ptr[12+ecx],bp
 mov ds:byte ptr[6+edi],al
Lp7:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 cmp bp,ds:word ptr[14+ecx]
 jl Lp8
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp8
 mov ds:word ptr[14+ecx],bp
 mov ds:byte ptr[7+edi],al
Lp8:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ds:dword ptr[pz],ecx
 mov ds:dword ptr[izi],ebp
 pop ecx
 cmp ecx,8
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
 mov ebx,ds:dword ptr[_tadjust]
 mov eax,ds:dword ptr[_sadjust]
 add eax,ds:dword ptr[snext]
 add ebx,ds:dword ptr[tnext]
 mov ebp,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp eax,2048
 jl LClampLow4
 cmp eax,ebp
 ja LClampHigh4
LClampReentry4:
 mov ds:dword ptr[snext],eax
 cmp ebx,2048
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
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
LSetEntryvec:
 mov ebx,ds:dword ptr[spr8entryvec_table+ecx*4]
 mov eax,edx
 push ebx
 mov ecx,ebp
 sar ecx,16
 mov ebx,ds:dword ptr[_cachewidth]
 sar edx,16
 jz LIsZeroLast
 imul edx,ebx
LIsZeroLast:
 add edx,ecx
 mov ecx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx,ds:dword ptr[sfracf]
 shl eax,16
 mov ds:dword ptr[advancetable],edx
 mov ds:dword ptr[tstep],eax
 mov ds:dword ptr[sstep],ebp
 mov edx,ecx
 mov ecx,ds:dword ptr[pz]
 mov ebp,ds:dword ptr[izi]
 ret
LNoSteps:
 mov ecx,ds:dword ptr[pz]
 sub edi,7
 sub ecx,14
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
 public Spr8Entry2_8
Spr8Entry2_8:
 sub edi,6
 sub ecx,12
 mov al,ds:byte ptr[esi]
 jmp LLEntry2_8
 public Spr8Entry3_8
Spr8Entry3_8:
 sub edi,5
 sub ecx,10
 jmp LLEntry3_8
 public Spr8Entry4_8
Spr8Entry4_8:
 sub edi,4
 sub ecx,8
 jmp LLEntry4_8
 public Spr8Entry5_8
Spr8Entry5_8:
 sub edi,3
 sub ecx,6
 jmp LLEntry5_8
 public Spr8Entry6_8
Spr8Entry6_8:
 sub edi,2
 sub ecx,4
 jmp LLEntry6_8
 public Spr8Entry7_8
Spr8Entry7_8:
 dec edi
 sub ecx,2
 jmp LLEntry7_8
 public Spr8Entry8_8
Spr8Entry8_8:
 cmp bp,ds:word ptr[ecx]
 jl Lp9
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp9
 mov ds:word ptr[ecx],bp
 mov ds:byte ptr[edi],al
Lp9:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry7_8:
 cmp bp,ds:word ptr[2+ecx]
 jl Lp10
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp10
 mov ds:word ptr[2+ecx],bp
 mov ds:byte ptr[1+edi],al
Lp10:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry6_8:
 cmp bp,ds:word ptr[4+ecx]
 jl Lp11
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp11
 mov ds:word ptr[4+ecx],bp
 mov ds:byte ptr[2+edi],al
Lp11:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry5_8:
 cmp bp,ds:word ptr[6+ecx]
 jl Lp12
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp12
 mov ds:word ptr[6+ecx],bp
 mov ds:byte ptr[3+edi],al
Lp12:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry4_8:
 cmp bp,ds:word ptr[8+ecx]
 jl Lp13
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp13
 mov ds:word ptr[8+ecx],bp
 mov ds:byte ptr[4+edi],al
Lp13:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry3_8:
 cmp bp,ds:word ptr[10+ecx]
 jl Lp14
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp14
 mov ds:word ptr[10+ecx],bp
 mov ds:byte ptr[5+edi],al
Lp14:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LLEntry2_8:
 cmp bp,ds:word ptr[12+ecx]
 jl Lp15
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp15
 mov ds:word ptr[12+ecx],bp
 mov ds:byte ptr[6+edi],al
Lp15:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc esi,ds:dword ptr[advancetable+4+eax*4]
LEndSpan:
 cmp bp,ds:word ptr[14+ecx]
 jl Lp16
 mov al,ds:byte ptr[esi]
 cmp al,255
 jz Lp16
 mov ds:word ptr[14+ecx],bp
 mov ds:byte ptr[7+edi],al
Lp16:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 pop ebx
LNextSpan:
 add ebx,12
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,0
 jg LSpanLoop
 jz LNextSpan
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
