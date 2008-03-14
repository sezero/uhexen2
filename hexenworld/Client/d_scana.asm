;
; d_scana.asm
; x86 assembly-language turbulent texture mapping code
;
; this file uses NASM syntax.
; $Id: d_scana.asm,v 1.4 2008-03-14 18:55:57 sezero Exp $
;

%idefine offset

; externs from C code
 extern r_turb_s
 extern r_turb_t
 extern r_turb_pdest
 extern r_turb_spancount
 extern r_turb_turb
 extern r_turb_pbase
 extern r_turb_sstep
 extern r_turb_tstep
 extern mainTransTable
 extern scanList

; externs from ASM-only code


SEGMENT .data

SEGMENT .text

;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawTurbulent8Span
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global D_DrawTurbulent8Span
D_DrawTurbulent8Span:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
Llp:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 inc edi
 add esi, dword [r_turb_sstep]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [ebp+edx*1]
 dec ebx
 mov  byte [-1+edi],dl
 jnz Llp
 mov  dword [r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawTurbulent8TSpan
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global D_DrawTurbulent8TSpan
D_DrawTurbulent8TSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
LlpT:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 cmp  byte [scanList + ebx - 1], 1
 jnz skip1
 inc edi
 mov dh, byte [ebp+edx*1]
 add esi, dword [r_turb_sstep]
 mov dl, byte [-1+edi]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [12345678h + edx]
TranPatch1:

 dec ebx
 mov  byte [-1+edi],dl
 ;mov  byte [-1+edi],255
 jnz LlpT
skip2:
 mov  dword [r_turb_pdest],edi
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


;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawTurbulent8TQuickSpan
;;;;;;;;;;;;;;;;;;;;;;;;

 ALIGN 4

 global D_DrawTurbulent8TQuickSpan
D_DrawTurbulent8TQuickSpan:
 push ebp
 push esi
 push edi
 push ebx
 mov esi, dword [r_turb_s]
 mov ecx, dword [r_turb_t]
 mov edi, dword [r_turb_pdest]
 mov ebx, dword [r_turb_spancount]
LlpTQ:
 mov eax,ecx
 mov edx,esi
 sar eax,16
 mov ebp, dword [r_turb_turb]
 sar edx,16
 and eax,128-1
 and edx,128-1
 mov eax, dword [ebp+eax*4]
 mov edx, dword [ebp+edx*4]
 add eax,esi
 sar eax,16
 add edx,ecx
 sar edx,16
 and eax,64-1
 and edx,64-1
 shl edx,6
 mov ebp, dword [r_turb_pbase]
 add edx,eax
 ;cmp  byte [scanList + ebx - 1], 1
 ;jnz skip1
 inc edi
 mov dh, byte [ebp+edx*1]
 add esi, dword [r_turb_sstep]
 mov dl, byte [-1+edi]
 add ecx, dword [r_turb_tstep]
 mov dl, byte [12345678h + edx]
TranPatch2:

 dec ebx
 mov  byte [-1+edi],dl
 jnz LlpTQ

 mov  dword [r_turb_pdest],edi
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret

;;;;;;;;;;;;;;;;;;;;;;;;
; D_DrawTurbulent8TSpanEnd
;;;;;;;;;;;;;;;;;;;;;;;;
 global D_DrawTurbulent8TSpanEnd
D_DrawTurbulent8TSpanEnd:


SEGMENT .data

 ALIGN 4

LPatchTable:
 dd TranPatch1-4
 dd TranPatch2-4

SEGMENT .text

 ALIGN 4

;;;;;;;;;;;;;;;;;;;;;;;;
; R_TranPatch7
;;;;;;;;;;;;;;;;;;;;;;;;

 global R_TranPatch7
R_TranPatch7:
 push ebx
 mov eax, dword [mainTransTable]
 mov ebx,offset LPatchTable
 mov ecx,2
LPatchLoop:
 mov edx, dword [ebx]
 add ebx,4
 mov  dword [edx],eax
 dec ecx
 jnz LPatchLoop
 pop ebx
 ret

