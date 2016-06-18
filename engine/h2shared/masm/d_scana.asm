;
; d_scana.asm -- for MASM
; x86 assembly-language turbulent texture mapping code
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
 externdef _r_turb_s:dword
 externdef _r_turb_t:dword
 externdef _r_turb_pdest:dword
 externdef _r_turb_spancount:dword
 externdef _r_turb_turb:dword
 externdef _r_turb_pbase:dword
 externdef _r_turb_sstep:dword
 externdef _r_turb_tstep:dword
 externdef _mainTransTable:dword
 externdef _scanList:dword

; externs from ASM-only code

_DATA SEGMENT
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_DrawTurbulent8Span
_D_DrawTurbulent8Span:
 push ebp
 push esi
 push edi
 push ebx
 mov esi,ds:dword ptr[_r_turb_s]
 mov ecx,ds:dword ptr[_r_turb_t]
 mov edi,ds:dword ptr[_r_turb_pdest]
 mov ebx,ds:dword ptr[_r_turb_spancount]
Llp:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp,ds:dword ptr[_r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax,ds:dword ptr[ebp+eax*4]
 mov edx,ds:dword ptr[ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp,ds:dword ptr[_r_turb_pbase]
 add edx,eax
 inc edi
 add esi,ds:dword ptr[_r_turb_sstep]
 add ecx,ds:dword ptr[_r_turb_tstep]
 mov dl,ds:byte ptr[ebp+edx*1]
 dec ebx
 mov ds:byte ptr[-1+edi],dl
 jnz Llp
 mov ds:dword ptr[_r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret


 align 4
 public _D_DrawTurbulent8TSpan
_D_DrawTurbulent8TSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi,ds:dword ptr[_r_turb_s]
 mov ecx,ds:dword ptr[_r_turb_t]
 mov edi,ds:dword ptr[_r_turb_pdest]
 mov ebx,ds:dword ptr[_r_turb_spancount]
LlpT:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp,ds:dword ptr[_r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax,ds:dword ptr[ebp+eax*4]
 mov edx,ds:dword ptr[ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp,ds:dword ptr[_r_turb_pbase]
 add edx,eax
 cmp ds:byte ptr[_scanList + ebx - 1], 1
 jnz skip1
 inc edi
 mov dh,ds:byte ptr[ebp+edx*1]
 add esi,ds:dword ptr[_r_turb_sstep]
 mov dl,ds:byte ptr[-1+edi]
 add ecx,ds:dword ptr[_r_turb_tstep]
 mov dl,ds:byte ptr[12345678h + edx]
TranPatch1:

 dec ebx
 mov ds:byte ptr[-1+edi],dl
 ;mov ds:byte ptr[-1+edi],255
 jnz LlpT
skip2:
 mov ds:dword ptr[_r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

skip1:
 inc edi
 dec ebx
 jnz LlpT
 jmp skip2








 align 4
 public _D_DrawTurbulent8TQuickSpan
_D_DrawTurbulent8TQuickSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi,ds:dword ptr[_r_turb_s]
 mov ecx,ds:dword ptr[_r_turb_t]
 mov edi,ds:dword ptr[_r_turb_pdest]
 mov ebx,ds:dword ptr[_r_turb_spancount]
LlpTQ:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp,ds:dword ptr[_r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax,ds:dword ptr[ebp+eax*4]
 mov edx,ds:dword ptr[ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp,ds:dword ptr[_r_turb_pbase]
 add edx,eax
 ;cmp ds:byte ptr[_scanList + ebx - 1], 1
 ;jnz skip1
 inc edi
 mov dh,ds:byte ptr[ebp+edx*1]
 add esi,ds:dword ptr[_r_turb_sstep]
 mov dl,ds:byte ptr[-1+edi]
 add ecx,ds:dword ptr[_r_turb_tstep]
 mov dl,ds:byte ptr[12345678h + edx]
TranPatch2:

 dec ebx
 mov ds:byte ptr[-1+edi],dl
 jnz LlpTQ

 mov ds:dword ptr[_r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

 public _D_DrawTurbulent8TSpanEnd
_D_DrawTurbulent8TSpanEnd:
_TEXT ENDS

_DATA SEGMENT
 align 4
LPatchTable:
 dd TranPatch1-4
 dd TranPatch2-4
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _R_TranPatch7
_R_TranPatch7:
 push ebx
 mov eax,ds:dword ptr[_mainTransTable]
 mov ebx,offset LPatchTable
 mov ecx,2
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
